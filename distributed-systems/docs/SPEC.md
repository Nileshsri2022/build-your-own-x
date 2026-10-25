# SPEC — Distributed Systems (Kafka-like system)

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
- Tutorial defaults carried over: 3 partitions and replication factor
  2 in the example producer, broker ids 1/2/3 on ports 9091/9092/9093,
  consumer takes a topic, a partition, and a starting offset.

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
   duplicating records within one fetch. A consumer group option
   stores the committed offset per (group, topic, partition) so a
   later run resumes where the previous one stopped.
5. **Metadata.** Any broker answers a metadata request with the list
   of topics, partitions, and the current leader and followers of
   each partition, so clients can talk to the leader.
6. **Replication.** The leader forwards each appended record to every
   follower of that partition. With `acks=1` the producer gets its
   reply after the leader append. With `acks=all` (default) it waits
   until every live follower acks, or times out with an error and the
   record is not reported as committed.
7. **Controller and failover.** Exactly one controller at a time: the
   lowest live broker id. The controller assigns partitions
   round-robin across brokers at topic creation and, when a leader
   dies, promotes the next live replica to leader. A follower that
   rejoins truncates any records beyond the new leader's high-water
   mark before resuming replication.
8. **Errors, not silence.** Unknown topic, unknown partition, offset
   out of range, duplicate topic, and follower-timeout all return an
   explicit error response and leave the log unchanged.

## Wire protocol

TCP, one connection per request batch, big-endian throughout. This is
the tutorial's message-type design, with one deliberate change:
**every frame is length-prefixed**, so TCP cannot merge or split two
messages into one read.

Frame: `uint32 frame_len | uint8 type | payload`
(`frame_len` counts the bytes after itself.)

| Type | Code | Payload |
|---|---|---|
| PRODUCE | 0x01 | `uint16 topic_len, topic, uint32 partition, uint32 msg_len, msg` |
| FETCH | 0x02 | `uint16 topic_len, topic, uint32 partition, uint64 offset, uint32 max_bytes` |
| METADATA | 0x03 | (none) |
| CREATE_TOPIC | 0x04 | `uint16 topic_len, topic, uint32 partitions, uint16 replication` |
| REPLICATE | 0x21 | `uint16 topic_len, topic, uint32 partition, uint64 offset, uint32 msg_len, msg` |
| REPLICATE_ACK | 0x22 | `uint16 topic_len, topic, uint32 partition, uint64 offset` |
| PRODUCE_RESPONSE | 0x11 | `uint64 offset` |
| FETCH_RESPONSE | 0x12 | `uint32 count, then per record: uint64 offset, uint32 msg_len, msg` |
| METADATA_RESPONSE | 0x13 | topics, partitions, leader, followers (JSON body) |
| CREATE_TOPIC_RESPONSE | 0x14 | `uint8 ok` |
| ERROR_RESPONSE | 0x1F | `uint16 msg_len, msg` |

A broker that receives a request for a partition it does not lead
forwards nothing silently: it replies with an error naming the leader,
and the client retries against the leader from metadata.

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
- New topic: 3 partitions, replication factor 2 (tutorial producer
  defaults), unless the create call says otherwise
- Segment size: 1 MiB; fetch cap: 1 MiB
- `acks`: `all` by default; `1` is the only other accepted value
- Heartbeat: brokers write a heartbeat every 2 s; a broker with no
  heartbeat for 5 s counts as dead for election purposes
- Data layout: `<data-dir>/<topic>/<partition>/` holds the segments;
  `<data-dir>/groups.json` holds committed consumer-group offsets;
  `<data-dir>/registry/` holds broker heartbeat files

## Deliberate divergences from the tutorial (state them, don't hide them)

1. **Python 3 stdlib only, not Java/Maven.** Matches every other
   somecoolprojects build and needs no extra install.
2. **No real ZooKeeper server.** The tutorial's controller election
   uses a ZooKeeper ephemeral node. Ours uses heartbeat files in a
   shared registry directory plus lowest-live-id election. Same
   observable behaviour for a local 3-broker cluster; it is not a
   ZooKeeper replacement and must not be described as one.
3. **Length-prefixed frames.** The tutorial's encoder writes type +
   fields with no frame length. Ours adds a `uint32` length prefix.
   Consequence: our clients and brokers only talk to each other,
   not to the tutorial's Java clients.
4. **Consumer groups added.** The tutorial consumer takes an explicit
   offset. Ours keeps that, and adds a `--group` mode with a committed
   offset, because a Kafka-like system without resume is half a demo.
5. **Single file, `kafka.py`.** Broker, protocol, log, producer,
   consumer, and CLI live in one stdlib-only file, following the
   somecoolprojects single-file convention. Tests live in `tests/`.

## Non-goals

Real ZooKeeper / KRaft, ISR sets, transactions, exactly-once
semantics, log compaction, authentication, TLS, rebalancing an
existing topic when a broker joins later, and compatibility with real
Kafka clients. If a test needs any of these, the spec is wrong, not
the test — fix the spec first, in the open, before changing code.
