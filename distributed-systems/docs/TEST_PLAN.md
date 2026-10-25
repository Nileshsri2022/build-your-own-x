# TEST_PLAN — Distributed Systems (Kafka-like system, Java version)

This TEST_PLAN supersedes the earlier Python TEST_PLAN in full.
No `python3 -m unittest`, no `kafka.py`.

## Prerequisites before any test runs

```sh
java -version   # 11+
mvn -version
zkServer start  # or zookeeper-server-start with a config file
```

Checked 2026-10-25: none of these exist in this VM. Installing
them (or moving to a machine that has them) is a precondition for
the develop step. No test result may be claimed until the commands
above actually run.

## Unit tests (JUnit 5, via Maven, no ZooKeeper needed)

```sh
mvn test
```

- Protocol round-trip for every message type in SPEC, including a
  0-byte payload and a 1 MiB payload; a truncated input must raise,
  not return a half message.
- Partition: append 5 records, fetch from offset 0/2/5, fetch past
  the end returns empty, fetch with a max-bytes cap stops mid-log
  without splitting a record.
- Partition recovery: append, close, reopen — next offset resumes,
  all records readable in order.
- Segment roll: force a small segment size in the test, append past
  it, assert a second segment exists and fetches still cross the
  boundary in order.
- Partition assignment: 3 partitions, replication 2, 3 brokers →
  the exact round-robin replica lists from DESIGN, leader = first
  replica.
- Key hashing: same key always maps to the same partition;
  different keys spread (at least 2 distinct partitions over 20 keys).

## Integration tests (scripted, real processes, ZooKeeper running)

Build once, then run the scripted sequence:

```sh
mvn clean package
zkServer start
java -cp target/simple-kafka-1.0-SNAPSHOT.jar com.simplekafka.broker.SimpleKafkaBroker 1 localhost 9091 2181
java -cp target/simple-kafka-1.0-SNAPSHOT.jar com.simplekafka.broker.SimpleKafkaBroker 2 localhost 9092 2181
java -cp target/simple-kafka-1.0-SNAPSHOT.jar com.simplekafka.broker.SimpleKafkaBroker 3 localhost 9093 2181
```

Assert, in order:

- Single-topic flow: create topic, produce 10 messages to partition
  0, consume them back from offset 0, in order, byte-identical.
- Metadata: a broker that is neither controller nor leader answers
  a metadata request with the correct leader for each partition.
- Not-leader behaviour: producing to a follower returns an error
  naming the leader, and the producer's one retry against fresh
  metadata succeeds.
- Consumer groups: consume with a group, stop, restart with the
  same group — it resumes after the last committed offset in
  ZooKeeper, with no duplicates and no gaps.
- Errors: unknown topic, unknown partition, duplicate topic, offset
  before 0 — each returns an error response and leaves the log
  unchanged (asserted by a follow-up fetch).
- Restart: produce, kill a broker, restart it on the same data
  dir, fetch — all records present, offsets unchanged.
- Replication, `acks=all`: produce 20 messages, assert every
  follower's log holds all 20, in order, before the producer reply
  is counted as committed.
- Replication, `acks=1`: produce with a follower paused — the reply
  arrives without waiting for it; and assert the documented
  boundary: a record acknowledged under `acks=1` is *not*
  guaranteed present on a follower that was down (kill the leader
  after, check only that the system stays consistent — it must not
  crash or serve a gap).
- Failover: kill the leader of a partition, wait for ZooKeeper's
  ephemeral-node expiry, produce again — the new leader serves it,
  metadata shows the new leader, and a fetch from offset 0 returns
  the old records plus the new one, in order.
- Controller failover: kill the controller broker, create a new
  topic — a new controller has been elected via ZooKeeper and the
  create succeeds.

Every scripted run must clean up its own brokers and leave no stray
processes behind (kill by explicit PID in a finally block).

## What is NOT tested (and therefore not claimed)

- KRaft, ISR, transactions, exactly-once, compaction, auth/TLS,
  rebalancing, or compatibility with real Kafka clients. No test
  for these exists because the spec excludes them.
- Nothing may be reported as "tested" from this VM until JDK,
  Maven, and ZooKeeper are actually installed and the commands
  above have run.
