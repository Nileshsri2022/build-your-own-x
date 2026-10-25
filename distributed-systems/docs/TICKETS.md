# TICKETS — Distributed Systems (ask-matt: to-spec → to-tickets)

Work in ticket order. A ticket is done only when its tests pass.
Blocking edges are real: do not start a ticket whose blockers are
not done.

## T1 — Protocol frames (blocks T2, T3, T4, T5)

- Implement `Frame`/`Protocol` encode/decode for every type in SPEC.
- Tests: frame round-trips, truncated frame raises, 0-byte and
  1 MiB payloads.
- No sockets, no broker in this ticket.

## T2 — Partition log (blocks T3, T4, T5)

- Implement `Segment`/`PartitionLog`: append, fetch, segment roll,
  index, recovery, gap rejection.
- Tests: append/fetch, recovery, segment roll, key hashing helper
  (pure function, lives here so T3 can use it).

## T3 — Single broker + producer/consumer (blocks T4, T5)

- Implement `Broker` (CREATE_TOPIC, PRODUCE, FETCH, METADATA),
  `Producer`, `Consumer` (explicit offset + group), CLI.
- One broker only; replication and controller code are not in this
  ticket, even as stubs that pretend to work.
- Tests: single-broker produce/consume, metadata, not-leader error
  path is deferred — a single broker *is* the leader here — errors,
  restart, CLI smoke.

## T4 — Registry and controller (blocks T5)

- Implement `Registry` heartbeats/pruning and the controller role:
  round-robin assignment, topics.json, controller election by
  lowest live id.
- Tests: assignment lists, controller election after the controller
  dies, duplicate broker id exits with an error.

## T5 — Replication and failover (blocks T6)

- Implement REPLICATE/REPLICATE_ACK, `acks=1` vs `acks=all`,
  leader promotion, follower truncation on rejoin.
- Tests: multi-broker replication, acks boundary, leader failover,
  controller failover with a new topic create.

## T6 — Code review and push prep (blocks push)

- Two-axis review (Standards + Spec) of the full diff: every SPEC
  claim traced to a test, every deliberate divergence still listed
  in SPEC, no test weakened to match code.
- Full suite green: `python3 -m unittest discover -s tests -v`.
- Update this project's README run section from "planned" to
  verified, with the exact commands run.
- Push is a separate step after this ticket, on your per-project
  approval — this ticket does not itself push anything.

## Out of tickets (not in this build)

ZooKeeper/KRaft, ISR, transactions, exactly-once, compaction,
auth/TLS, rebalancing, real-Kafka client compatibility. If you want
any of these later, they become new tickets with a spec change first.
