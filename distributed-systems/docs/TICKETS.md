# TICKETS — Distributed Systems, Java version (ask-matt: to-spec → to-tickets)

This TICKETS file supersedes the earlier Python TICKETS file in
full. Ticket order now follows the tutorial's stages 1–8.

Work in ticket order. A ticket is done only when its tests pass.
Blocking edges are real: do not start a ticket whose blockers are
not done.

Prerequisite for all tickets (not a ticket itself): JDK 11+,
Maven, and a ZooKeeper server. Checked 2026-10-25: none present in
this VM. Resolve that before T1's tests can run.

## T1 — Project structure + protocol (blocks T2, T3, T4, T5, T6)

- `pom.xml` (Java 11, zookeeper 3.8.1, shade plugin), package
  `com.simplekafka`, directory layout from DESIGN.
- `Protocol` encode/decode for every type in SPEC, tutorial framing
  (no length prefix).
- Tests: protocol round-trips, truncated input raises, 0-byte and
  1 MiB payloads. Run: `mvn test`.

## T2 — ZooKeeper client + controller election (blocks T4, T5, T6)

- `ZookeeperClient`: connect, ephemeral nodes, children/node
  watches with callbacks, persistent nodes for consumer-group
  offsets.
- Controller election: create `/controller` ephemeral node, first
  creator wins, watchers re-elect on disappearance.
- Tests: election (first wins, second watches), re-election after
  the controller's session ends, broker presence via
  `/brokers/<id>` ephemeral nodes. Requires a running ZooKeeper
  server; these tests do not run in a VM without one.

## T3 — Storage layer (blocks T4, T5, T6)

- `Partition`: append, fetch, segment roll at 1 MiB, index,
  recovery, gap rejection. `BrokerInfo` value object.
- Tests: append/fetch, recovery, segment roll, key-hash helper
  (pure function, lives here so T5 can use it).

## T4 — Broker (blocks T5, T6)

- `SimpleKafkaBroker`: NIO TCP server, CREATE_TOPIC (controller
  only, forward otherwise), PRODUCE (leader only), FETCH (leader
  only), METADATA (any broker), REPLICATE/REPLICATE_ACK, leader
  promotion on broker-death watch, follower truncation on rejoin.
- Tests: scripted single-broker produce/consume, metadata,
  not-leader error + retry, errors, restart. Multi-broker
  replication is T5's, not this ticket's — a single broker *is*
  the leader here.

## T5 — Client library (blocks T6)

- `SimpleKafkaClient`: bootstrap, metadata, send, fetch,
  create-topic, one retry on "not leader".
- Tests: client against a live single broker (produce/fetch/
  metadata round-trips), retry after a metadata change.

## T6 — Producer / consumer APIs (blocks T7)

- `SimpleKafkaProducer`: create-if-missing (3 partitions,
  replication 2), explicit / key-hashed / random partition, offset
  reply, `acks` handling.
- `SimpleKafkaConsumer`: explicit-offset poll, group mode with
  ZooKeeper committed offsets.
- Tests: multi-broker replication (`acks=all` and the `acks=1`
  boundary), leader failover, controller failover with a new
  topic create, group resume.

## T7 — Code review and push prep (blocks push)

- Two-axis review (Standards + Spec) of the full diff: every SPEC
  claim traced to a test, every extension beyond the tutorial
  still listed in SPEC, no test weakened to match code.
- Full suite green: `mvn test` plus the scripted integration runs
  in TEST_PLAN, with JDK/Maven/ZooKeeper actually present.
- Update this project's README run section from "planned" to
  verified, with the exact commands run.
- Push is a separate step after this ticket, on your per-project
  approval — this ticket does not itself push anything.

## Out of tickets (not in this build)

KRaft, ISR, transactions, exactly-once, compaction, auth/TLS,
rebalancing, real-Kafka client compatibility. If you want any of
these later, they become new tickets with a spec change first.
