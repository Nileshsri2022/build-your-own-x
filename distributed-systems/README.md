# Distributed Systems — Build Your Own Kafka-like System

Status: **docs only, awaiting your agreement. No code, no tests, no push yet.**

A Kafka-like distributed messaging system in Python 3 (standard library
only), built from the Build Your Own X entry for Distributed Systems:

- Source list: https://github.com/codecrafters-io/build-your-own-x
  (section: Build your own `Distributed Systems`)
- Tutorial followed: _Building Your Own Kafka-like System From Scratch_
  — https://github.com/buildthingsuseful/build-your-own-kafka
  (Java, 2,952 lines: broker, protocol, partition log, ZooKeeper
  client, producer, consumer)

This build is **not a port of that Java code**. It follows the same
architecture and the same tutorial stages, in Python, with the
deliberate divergences listed in `docs/SPEC.md` — most importantly, no
Java, no Maven, and no real ZooKeeper server. Coordination is a small
registry on disk (see `docs/DESIGN.md`).

## What it will be

- 3 brokers on localhost by default: ids 1, 2, 3 on ports 9091, 9092, 9093
- Topics split into partitions; each partition is an append-only log
  with segments and an offset index, persisted on disk
- A controller (lowest live broker id) assigns partitions, picks the
  leader, and re-elects a leader when one dies
- Leader replicates to followers; `acks=all` waits for follower acks
- Producer: create topic if missing, send to a chosen partition or a
  key-hashed / random partition
- Consumer: fetch from an explicit offset, or by consumer group with a
  committed offset that survives a restart

## Planned run (not working yet — code is not written)

```sh
# one terminal per broker
python3 kafka.py broker --id 1 --port 9091 --data-dir data/broker-1
python3 kafka.py broker --id 2 --port 9092 --data-dir data/broker-2
python3 kafka.py broker --id 3 --port 9093 --data-dir data/broker-3

# produce and consume
python3 kafka.py produce --bootstrap 127.0.0.1:9091 --topic test-topic "hello"
python3 kafka.py consume --bootstrap 127.0.0.1:9091 --topic test-topic --partition 0 --offset 0
python3 kafka.py consume --bootstrap 127.0.0.1:9091 --topic test-topic --group demo
```

## Docs (ask-matt: grill-with-docs → spec → tickets)

- `docs/SPEC.md` — what the tutorial asks, the wire protocol, and
  every deliberate divergence
- `docs/DESIGN.md` — modules, data layout, replication and failover
- `docs/TEST_PLAN.md` — the tests that must pass before any push
- `docs/TICKETS.md` — build order, with blocking edges
- `docs/GLOSSARY.md` — the shared vocabulary for this project

## Out of scope, honestly

Real ZooKeeper / KRaft, ISR management, transactions, exactly-once
delivery, log compaction, auth/TLS, rebalancing existing topics when a
broker joins, and wire compatibility with the Java tutorial's clients
(our framing adds a length prefix — see SPEC). Those are not claimed.

## Next step

Your agreement on these docs triggers, in order: code (TDD, one ticket
at a time), test run, code review against SPEC, then push to
`Nileshsri2022/somecoolprojects` — this project only, as its own
`distributed-systems/` directory.
