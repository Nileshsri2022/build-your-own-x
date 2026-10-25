# Distributed Systems — Build Your Own Kafka-like System

Status: **docs only, revised to Java, awaiting your agreement to develop.
No code, no tests, no code push yet.**

A Kafka-like distributed messaging system in **Java 11 + Maven**,
built from the Build Your Own X entry for Distributed Systems:

- Source list: https://github.com/codecrafters-io/build-your-own-x
  (section: Build your own `Distributed Systems`)
- Tutorial followed: _Building Your Own Kafka-like System From Scratch_
  — https://github.com/buildthingsuseful/build-your-own-kafka
  (Java, 2,952 lines: broker, protocol, partition log, ZooKeeper
  client, producer, consumer)

This version follows the tutorial's language, package structure, and
architecture closely — including real ZooKeeper for controller
election — instead of the earlier Python proposal. That earlier
Python version is superseded; do not build from it.

## What it will be

- 3 brokers on localhost by default: ids 1, 2, 3 on ports 9091, 9092, 9093
- A ZooKeeper server on localhost:2181, as the tutorial requires
- Topics split into partitions; each partition is an append-only log
  with segments and an offset index, persisted on disk
- A controller elected through a ZooKeeper ephemeral node, which
  assigns partitions, picks the leader, and re-elects a leader when
  one dies
- Leader replicates to followers; producer replies follow the
  tutorial's replication-ack flow, with an `acks` extension
  documented in SPEC
- Producer: create topic if missing, send to a chosen partition or a
  key-hashed / random partition
- Consumer: fetch from an explicit offset, or by consumer group with
  a committed offset stored in ZooKeeper

## Prerequisites (checked 2026-10-25 — none are installed in this VM)

- JDK 11 or later (`java`, `javac`)
- Maven (`mvn`)
- A ZooKeeper server (`zkServer start`, or
  `zookeeper-server-start` with a config file)

Code cannot be compiled or tested here until those are installed.
At develop time, either install them (with your approval) or run
the build/tests on a machine that already has them. Do not claim
the Java build verified until `mvn clean package` and the test
plan have actually run.

## Planned run (not working yet — code is not written)

```sh
mvn clean package

# ZooKeeper first, as the tutorial requires
zkServer start

# one terminal per broker: <id> <host> <port> <zookeeper-port>
java -cp target/simple-kafka-1.0-SNAPSHOT.jar com.simplekafka.broker.SimpleKafkaBroker 1 localhost 9091 2181
java -cp target/simple-kafka-1.0-SNAPSHOT.jar com.simplekafka.broker.SimpleKafkaBroker 2 localhost 9092 2181
java -cp target/simple-kafka-1.0-SNAPSHOT.jar com.simplekafka.broker.SimpleKafkaBroker 3 localhost 9093 2181

# produce and consume (tutorial-style positional args)
java -cp target/simple-kafka-1.0-SNAPSHOT.jar com.simplekafka.client.SimpleKafkaProducer localhost 9091 test-topic
java -cp target/simple-kafka-1.0-SNAPSHOT.jar com.simplekafka.client.SimpleKafkaConsumer localhost 9091 test-topic 0
```

## Docs (ask-matt: grill-with-docs → spec → tickets)

- `docs/SPEC.md` — what the tutorial asks, the wire protocol, and
  every deliberate extension beyond the tutorial
- `docs/DESIGN.md` — classes, data layout, replication and failover
- `docs/TEST_PLAN.md` — the tests that must pass before any push
- `docs/TICKETS.md` — build order, following the tutorial's stages,
  with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Out of scope, honestly

KRaft (the tutorial is ZooKeeper-based, so this build is too), ISR
management, transactions, exactly-once delivery, log compaction,
auth/TLS, rebalancing existing topics when a broker joins, and
compatibility with real Kafka clients (the tutorial's protocol is
its own, not Kafka's wire protocol). Those are not claimed.

## Next step

Your agreement on these revised Java docs triggers, in order: code
(TDD, one ticket at a time), test run, code review against SPEC,
then push — this project only, as its own `distributed-systems/`
directory. The code's destination (build-your-own-x vs
somecoolprojects) still needs confirming at develop time.
