# GLOSSARY — Distributed Systems, Java version (ask-matt: grill-with-docs paper trail)

Supersedes the Python glossary: "registry" and "lowest live id"
entries below are replaced by ZooKeeper entries.

- **Topic**: a named stream of messages, e.g. `test-topic`.
- **Partition**: one ordered shard of a topic, numbered from 0. A
  topic's partitions together hold all its messages.
- **Offset**: a message's position inside its partition, starting
  at 0, increasing by 1 per message. Dense — no gaps in a committed
  log.
- **Broker**: one server process that stores some partitions and
  serves produce/fetch requests for them.
- **Bootstrap broker**: any broker a client first contacts to learn
  metadata; it need not be the leader of anything the client wants.
- **Leader**: the one replica of a partition that accepts produces
  and serves fetches for it.
- **Follower**: a replica that is not the leader; it receives
  records from the leader by replication.
- **Replica**: any copy of a partition, leader or follower.
- **Replication factor**: how many replicas each partition has
  (default 2).
- **Controller**: the one broker that owns the ZooKeeper
  `/controller` ephemeral node, assigns partitions, and elects new
  leaders. A role, not a separate process.
- **ZooKeeper**: the coordination server (localhost:2181) the
  tutorial uses. Brokers register as ephemeral nodes under
  `/brokers/<id>`; the controller is an ephemeral node under
  `/controller`.
- **Ephemeral node**: a ZooKeeper node that disappears automatically
  when its creator's session ends — how broker/controller death is
  detected, with no heartbeat files.
- **Watch**: a ZooKeeper notification subscription: brokers watch
  `/brokers` children and the `/controller` node, and re-run
  election/promotion when they change.
- **Maven**: the Java build tool; `mvn clean package` builds the
  shaded jar. The tutorial targets Java 11.
- **Segment**: one log file of up to 1 MiB, named by its base
  offset, plus its `.index` file mapping offsets to byte positions.
- **High-water mark**: the highest offset that is committed on a
  partition (for `acks=all`: present on every live follower).
- **Producer**: a client that sends messages to a topic.
- **Consumer**: a client that fetches messages from a partition.
- **Consumer group**: a named set of consumers sharing one committed
  offset per (group, topic, partition), so a restart resumes.
- **acks**: producer durability setting. `1` = leader append only.
  `all` = leader append plus every live follower's ack.
