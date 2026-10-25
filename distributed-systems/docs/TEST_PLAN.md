# TEST_PLAN — Distributed Systems (Kafka-like system)

Runner (stdlib only, no pytest dependency):

```sh
python3 -m unittest discover -s tests -v
```

Every test below must pass before the code-review step, and the
code-review step must pass before any push. A failing test is never
"fixed" by weakening the assertion to match the code — fix the code,
or fix the spec in the open and re-agree it.

## Unit tests (no sockets)

- Frame round-trip for every message type in SPEC, including a
  0-byte payload, a 1 MiB payload, and a truncated frame (must raise,
  not return a half message).
- PartitionLog: append 5 records, fetch from offset 0/2/5, fetch past
  the end returns empty, fetch with a max-bytes cap stops mid-log
  without splitting a record.
- PartitionLog recovery: append, close, reopen — next offset resumes,
  all records readable in order.
- Segment roll: force a small segment size in the test, append past
  it, assert a second segment exists and fetches still cross the
  boundary in order.
- Partition assignment: 3 partitions, replication 2, 3 brokers →
  the exact round-robin replica lists from DESIGN, leader = first
  replica.
- Key hashing: same key always maps to the same partition; different
  keys spread (assert at least 2 distinct partitions over 20 keys).

## Integration tests (real broker processes on localhost)

- Single broker: create topic, produce 10 messages to partition 0,
  consume them back from offset 0, in order, byte-identical.
- Metadata: a second broker (not controller, not leader) answers a
  metadata request with the correct leader for each partition.
- Not-leader behaviour: producing to a follower returns an error
  naming the leader, and the producer's one retry against fresh
  metadata succeeds.
- Consumer groups: consume with `--group demo`, stop, restart the
  consumer with the same group — it resumes after the last committed
  offset, with no duplicates and no gaps.
- Errors: unknown topic, unknown partition, duplicate topic, offset
  before 0 — each returns an error response and leaves the log
  unchanged (asserted by a follow-up fetch).
- Restart: produce, kill the broker, restart it on the same data
  dir, fetch — all records present, offsets unchanged.

## Multi-broker tests (the distributed part)

- Replication, `acks=all`: 3 brokers, produce 20 messages, assert
  every follower's log holds all 20, in order, before the producer
  reply is counted as committed.
- Replication, `acks=1`: produce, assert the reply arrives even
  with a follower paused — and assert the documented boundary: a
  record acknowledged under `acks=1` is *not* guaranteed present on
  a follower that was down (test pauses the follower, kills the
  leader, and checks only that the system stays consistent — it
  must not crash or serve a gap).
- Failover: kill the leader of a partition, wait for the 5 s
  staleness window plus a small margin, produce again — the new
  leader serves it, metadata shows the new leader, and a fetch from
  offset 0 returns the old records plus the new one, in order.
- Controller failover: kill the controller broker, create a new
  topic — the next-lowest live broker has become controller and the
  create succeeds.

## CLI smoke tests

- `kafka.py broker --help`, `produce --help`, `consume --help`
  exit 0 and print usage.
- End-to-end shell run: start 3 brokers, produce one message,
  consume it, stop all brokers — exit codes 0 throughout, no
  stray processes left behind (the test harness kills its own
  brokers in a finally block).

## What is NOT tested (and therefore not claimed)

- Real ZooKeeper, KRaft, ISR, transactions, exactly-once, compaction,
  auth/TLS, rebalancing, or compatibility with real Kafka clients.
  No test for these exists because the spec excludes them.
