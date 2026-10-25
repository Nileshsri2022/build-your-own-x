# DESIGN — Distributed Systems (Kafka-like system, Java version)

This DESIGN supersedes the earlier Python DESIGN in full. No
`kafka.py`, no heartbeat-file registry, no length-prefixed frames.

## Layout (tutorial structure, kept)

```
pom.xml                              # Java 11, zookeeper 3.8.1,
                                     # maven-shade-plugin, main class
                                     # com.simplekafka.broker.SimpleKafkaBroker
src/main/java/com/simplekafka/
  broker/
    BrokerInfo.java                  # id, host, port
    Protocol.java                    # message codes + encode/decode
    Partition.java                   # segment log, index, recovery
    ZookeeperClient.java             # ZK connection, ephemeral nodes,
                                     # children/node watches, callbacks
    SimpleKafkaBroker.java           # TCP server, controller role,
                                     # produce/fetch/metadata/replicate
  client/
    SimpleKafkaClient.java           # bootstrap, metadata, send, fetch
    SimpleKafkaProducer.java         # create-if-missing, random/key partition
    SimpleKafkaConsumer.java         # explicit offset + group resume
src/test/java/com/simplekafka/
  broker/ProtocolTest.java
  broker/PartitionTest.java
  # integration runs are scripted, not JUnit (see TEST_PLAN)
```

## Classes, in dependency order

| Class | Responsibility |
|---|---|
| `Protocol` | Message codes and encode/decode for every type in SPEC. Pure bytes in, bytes out — unit-testable without a server or ZooKeeper. |
| `Partition` | Append, fetch-from-offset, segment roll at 1 MiB, index lookup, recovery on open (recompute next offset from the last segment, reject a gap). One read-write lock per partition; writers serialised. |
| `ZookeeperClient` | Connects to localhost:2181. Creates ephemeral nodes under `/brokers/<id>` (broker presence) and `/controller` (election: first creator wins, others watch). Watches children of `/brokers` and the `/controller` node, firing callbacks on change. Also stores consumer-group offsets under `/consumer-groups/<group>/<topic>/<partition>` as persistent nodes. |
| `SimpleKafkaBroker` | Threaded (NIO, as in the tutorial) TCP server. Dispatches by type: CREATE_TOPIC (controller only; others forward to the controller), PRODUCE (leader only, others forward to the leader), FETCH (leader only, others forward to the leader), METADATA (any broker), REPLICATE (follower appends, replies REPLICATE_ACK). Holds the controller role when it owns `/controller`. |
| `BrokerInfo` | Value object: broker id, host, port. |
| `SimpleKafkaClient` | Low-level client: connect to a bootstrap broker, metadata, send, fetch, create-topic. Retries once against fresh metadata on a "not leader" error. |
| `SimpleKafkaProducer` | On top of the client: create the topic if missing (3 partitions, replication 2 by default), pick a partition — explicit, key-hashed (SHA-256), or random — send, return the offset. |
| `SimpleKafkaConsumer` | On top of the client: poll from an explicit offset, advancing it per batch; group mode reads/writes the committed offset in ZooKeeper. |

## Flows

### Create topic

```
client -> any broker: CREATE_TOPIC
broker (not controller) -> controller: forward (tutorial behaviour)
controller: assign replicas round-robin, leader = first replica,
  write topic metadata, notify replicas (TOPIC_NOTIFICATION)
client <- CREATE_TOPIC_RESPONSE ok=1
```

### Produce, acks=all (tutorial behaviour)

```
producer -> leader: PRODUCE
leader: append to its Partition, assign offset
leader -> each follower: REPLICATE (offset, payload)
follower: append, REPLICATE_ACK
leader: once every live follower acked (or timeout -> ERROR)
leader -> producer: PRODUCE_RESPONSE offset
```

With `acks=1` (extension) the leader replies immediately after its
own append; followers still receive the record, but the producer is
not told to wait for them.

### Fetch

```
consumer -> leader: FETCH (topic, partition, offset, max_bytes)
leader: read from Partition from offset, stop at max_bytes
leader -> consumer: FETCH_RESPONSE records in offset order
```

Fetching from a follower is forwarded to the leader. Fetching
past the high-water mark returns zero records, not an error.
Fetching before offset 0 is an error.

### Failover (ZooKeeper-driven)

```
broker 2 (leader of partition 0) dies
  -> its /brokers/2 ephemeral node disappears
controller (watching /brokers) promotes the next live replica
  -> new leader, metadata updated
producer/consumer: next request is forwarded by any broker to the new leader
controller dies
  -> its /controller ephemeral node disappears
  -> watching brokers re-run the election; first creator wins
follower rejoining: truncate records beyond the new leader's
  high-water mark, then resume replication from there
```

Data written with `acks=all` before the failure is not lost in this
flow, because every live follower had acked it. Data written with
`acks=1` may be lost if the leader dies before replicating — that
is the documented trade-off of `acks=1`, and a test asserts exactly
that boundary rather than pretending otherwise.

## Data layout

```
data/broker-<id>/
  <topic>/
    <partition>/
      00000000000000000000.log
      00000000000000000000.index
      00000000000000001024.log   # after a 1 MiB roll
      ...
```

Topic metadata lives in the brokers' in-memory state, coordinated
through ZooKeeper (broker/controller nodes), as in the tutorial —
not in a shared `topics.json` file. That file was part of the
superseded Python design and must not reappear.

## Failure handling (decided now, not during debugging)

- Malformed frame / unknown type: close that connection, log, keep
  serving others. No partial append: a record is appended only after
  its full request is read and validated.
- Follower timeout on `acks=all`: producer gets an error; the leader
  keeps the record (it is already in its log) but does not report it
  as committed — the next produce to that partition must still work.
- Broker restart: reopen logs, recompute offsets, re-register its
  ephemeral `/brokers/<id>` node, rejoin as follower until the
  controller promotes it.
- Two brokers with the same id: the second to start exits with an
  error, it does not steal the first's ephemeral node.
- ZooKeeper down: brokers log the loss and refuse new
  controller/leader decisions until it returns; already-running
  leaders keep serving their existing partitions, they do not
  invent a new controller on their own.
