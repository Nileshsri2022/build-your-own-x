# Distributed Systems — Build Your Own Kafka-like System

Status: **code implemented and tested, 2026-10-25. Not yet pushed —
push needs your per-push approval.**

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

## What it is

- 3 brokers: ids 1, 2, 3 on ports 9091, 9092, 9093
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
  key-hashed partition
- Consumer: fetch from an explicit offset, or by consumer group with
  a committed offset stored in ZooKeeper
- Any bootstrap broker can serve any request: it proxies produce /
  fetch to the partition leader and create-topic to the controller.
  This is a deliberate simplification over the tutorial, documented
  in SPEC — the client never needs the leader's address.

## Code

- `pom.xml` — Java 11 target, ZooKeeper 3.8.1, JUnit 5.10.2 (test)
- `src/main/java/com/simplekafka/broker/` — BrokerInfo, Protocol,
  Partition, PartitionAssignment, TopicMetadata, ZookeeperClient,
  SimpleKafkaBroker
- `src/main/java/com/simplekafka/client/` — SimpleKafkaClient,
  SimpleKafkaProducer, SimpleKafkaConsumer
- `src/test/java/com/simplekafka/broker/` — ProtocolTest,
  PartitionTest, IntegrationDriver (scripted, not JUnit)
- `tests/integration.sh` — 3-broker scripted test (see TEST_PLAN)

## Verified 2026-10-25 in this VM

- Unit tests: **7/7 pass** (protocol round-trip, metadata JSON,
  assignment/key routing, partition append/fetch/recover/truncate,
  replication gap)
- Integration: **PASS** — 3 brokers, create topic (3 partitions,
  replication 2), produce 5 + 3 messages, fetch, follower log
  present on broker 2, kill broker 1, leader promoted to broker 2,
  produce + fetch after failover (6 messages, last is order-6)
- Toolchain here: JDK 17 toolchain compiling `--release 11`
  (bytecode runs on Java 11), Maven 3.9.9, ZooKeeper 3.8.1.
  Maven Central does **not** work from this VM's Java (the egress
  proxy's CONNECT tunnel fails for Java's HttpClient, even via a
  local relay), so the build here used `javac` + the ZooKeeper
  install's lib jars directly, and the shade jar was not built.
  On a normal machine with Maven Central access, `mvn clean package`
  is the canonical build and produces the shaded jar below.

## Run (normal machine)

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

In this sandbox, use `::1` instead of `localhost` for broker and
client hosts: Java TCP to 127.0.0.1 is intercepted here ("Other TCP
connections is turned off"), while IPv6 loopback works. See
`tests/integration.sh`, which already uses `::1`. Broker nodes in
ZooKeeper store `host|port` (not `host:port`) so `::1` survives.

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

Your per-push approval, then push this `distributed-systems/`
directory (docs + code + tests, excluding `target/` and broker
data) to `Nileshsri2022/build-your-own-x` — the repo where the
docs already live — as one commit.
