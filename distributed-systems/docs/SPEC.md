# SPEC — Distributed Systems (Kafka-like system, Java version)

Source, read before coding:

- Build Your Own X list:
  https://github.com/codecrafters-io/build-your-own-x
  — section "Build your own `Distributed Systems`", exactly one entry.
- Tutorial: _Building Your Own Kafka-like System From Scratch_
  — https://github.com/buildthingsuseful/build-your-own-kafka
  plus its linked Medium walkthrough. The repo's README stages are:
  1. project structure, 2. core protocol layer, 3. ZooKeeper
  integration, 4. storage layer, 5. broker, 6. client library,
  7. higher-level producer/consumer APIs, 8. test the system.
- Tutorial defaults carried over: Java 11, Maven, package
  `com.simplekafka`, artifact `simple-kafka-1.0-SNAPSHOT.jar`,
  ZooKeeper client 3.8.1, 3 partitions and replication factor 2 in
  the example producer, broker ids 1/2/3 on ports 9091/9092/9093
  with ZooKeeper on 2181, consumer takes a topic, a partition, and
  a starting offset.

This SPEC supersedes the earlier Python SPEC in full. Nothing in
the Python version (stdlib-only, heartbeat-file registry,
length-prefixed frames, `kafka.py`) applies to this build.

## What the system must do

1. **Topics and partitions.** `CREATE_TOPIC topic partitions replication`
   creates a topic. A topic is split into numbered partitions
   (0..n-1). Each partition is an ordered, append-only log. Messages
   are addressed by (topic, partition, offset), offset starting at 0
   and increasing by 1 per message.
2. **Durable log.** Messages survive a broker restart: on startup the
   broker reopens existing segments and resumes at the next offset.
   Segments roll at 1 MiB (tutorial default). Each segment is
   `<base-offset>.log` plus `<base-offset>.index`.
3. **Produce.** A producer connects to any broker (bootstrap), learns
   metadata, and sends a message to a partition. If no partition is
   given, a key is hashed (SHA-256, first 8 bytes) to a partition;
   with no key, the partition is chosen at random — matching the
   tutorial producer's random-partition behaviour for keyless sends.
   The reply carries the assigned offset.
4. **Fetch / consume.** A consumer fetches from a (topic, partition,
   offset) with a max-bytes cap, in offset order, without skipping or
   duplicating records within one fetch. A consumer-group mode
   stores the committed offset per (group, topic, partition) in
   ZooKeeper, so a later run resumes where the previous one stopped.
5. **Metadata.** Any broker answers a metadata request with the list
   of topics, partitions, and the current leader and followers of
   each partition, so clients can talk to the leader.
6. **Replication.** The leader forwards each appended record to every
   follower of that partition and waits for REPLICATE_ACK, as in the
   tutorial. With `acks=1` (extension, see below) the producer gets
   its reply after the leader append. With `acks=all` (default,
   tutorial behaviour) it waits until every live follower acks, or
   times out with an error and the record is not reported as
   committed.
7. **Controller and failover.** Exactly one controller at a time,
   elected through a ZooKeeper ephemeral node (`/controller`): the
   first broker to create it wins; the others watch it and re-run
   the election when it disappears. The controller assigns
   partitions round-robin across brokers at topic creation and,
   when a leader dies (its `/brokers/<id>` ephemeral node
   disappears), promotes the next live replica to leader. A
   follower that rejoins truncates any records beyond the new
   leader's high-water mark before resuming replication.
8. **Errors, not silence.** Unknown topic, unknown partition, offset
   out of range, duplicate topic, and follower-timeout all return an
   explicit error response and leave the log unchanged.

## Wire protocol

TCP, big-endian, following the tutorial's `Protocol` class exactly
— **no length prefix**. Framing works because the reader reads the
type byte first, then exactly the fields that type defines, using
the length fields inside variable-length parts. Message codes:

| Type | Code | Payload |
|---|---|---|
| PRODUCE | 0x01 | `uint16 topic_len, topic, uint32 partition, uint32 msg_len, msg` |
| FETCH | 0x02 | `uint16 topic_len, topic, uint32 partition, uint64 offset, uint32 max_bytes` |
| METADATA | 0x03 | (none) |
| CREATE_TOPIC | 0x04 | `uint16 topic_len, topic, uint32 partitions, uint16 replication` |
| REPLICATE | 0x21 | `uint16 topic_len, topic, uint32 partition, uint64 offset, uint32 msg_len, msg` |
| REPLICATE_ACK | 0x22 | `uint16 topic_len, topic, uint32 partition, uint64 offset` |
| TOPIC_NOTIFICATION | 0x23 | `uint16 topic_len, topic` |
| PRODUCE_RESPONSE | 0x11 | `uint64 offset` |
| FETCH_RESPONSE | 0x12 | `uint32 count, then per record: uint64 offset, uint32 msg_len, msg` |
| METADATA_RESPONSE | 0x13 | topics, partitions, leader, followers |
| CREATE_TOPIC_RESPONSE | 0x14 | `uint8 ok` |
| ERROR_RESPONSE | 0x1F | `uint16 msg_len, msg` |

A broker that receives a request for a partition it does not lead
forwards it to the leader (produce/fetch) or to the controller
(create-topic), and relays the response — so any bootstrap broker
can serve any request and the client never needs the leader's
address. This forwarding is a deliberate simplification over the
tutorial's error-and-retry flow; a dead leader/controller is the
only case that returns an error. Broker presence nodes in
ZooKeeper store `host|port` (not `host:port`) so IPv6 hosts like
`::1` survive. In this sandbox, broker/client hosts are `::1`,
not `localhost`: Java TCP to 127.0.0.1 is intercepted here, while
IPv6 loopback works (see README).

## Log record format (on disk)

Per segment, records are appended as:

`uint32 payload_len | payload bytes`

The index entry for a record is:

`uint64 offset | uint64 byte_position_in_log`

Offsets are dense per partition (0, 1, 2, …). No gaps are allowed in
a committed log: a gap found on recovery is a corruption error, not
something to paper over.

## Defaults

- Brokers: ids 1, 2, 3; host 127.0.0.1; ports 9091, 9092, 9093
- ZooKeeper: localhost:2181
- New topic: 3 partitions, replication factor 2 (tutorial producer
  defaults), unless the create call says otherwise
- Segment size: 1 MiB; fetch cap: 1 MiB
- `acks`: `all` by default; `1` is the only other accepted value

## Deliberate extensions beyond the tutorial (state them, don't hide them)

1. **Consumer groups.** The tutorial consumer takes an explicit
   offset. This build keeps that, and adds a group mode whose
   committed offset lives under a ZooKeeper node
   (`/consumer-groups/<group>/<topic>/<partition>`), because a
   Kafka-like system without resume is half a demo.
2. **`acks=1` option.** The tutorial's leader waits for follower
   acks before replying (i.e. `acks=all`). `acks=1` is added as an
   explicit, documented trade-off: faster, but a record may be lost
   if the leader dies before replicating.
3. **Tests.** The tutorial has no test suite. This build adds one
   (see TEST_PLAN): JUnit unit tests plus scripted multi-broker
   integration runs. No test is weakened to match code.

Everything else — language, package, message codes, ZooKeeper
coordination, segment format, broker CLI shape — follows the
tutorial, and is not a divergence.

## Non-goals

KRaft, ISR sets, transactions, exactly-once semantics, log
compaction, authentication, TLS, rebalancing an existing topic when
a broker joins later, and compatibility with real Kafka clients.
If a test needs any of these, the spec is wrong, not the test —
fix the spec first, in the open, before changing code.

## Prerequisites (verification gap, stated plainly)

Checked 2026-10-25 in this VM: `java`, `mvn`, and any ZooKeeper
server are all absent. The Java build therefore cannot be compiled
or tested here until JDK 11+, Maven, and a ZooKeeper server are
installed, or the work moves to a machine that has them. This is a
precondition for the develop step, not a detail to discover during
it.
