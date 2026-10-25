# DESIGN — Distributed Systems (Kafka-like system)

Ponytail rule for this build: smallest code that fully satisfies
SPEC, nothing speculative. One file, `kafka.py`, stdlib only.

## Modules (all inside kafka.py, in dependency order)

| Piece | Responsibility |
|---|---|
| `Frame` / `Protocol` | Length-prefixed encode/decode for every message type in SPEC. No socket logic here — pure bytes in, bytes out, so it is unit-testable without a server. |
| `Segment` / `PartitionLog` | Append, fetch-from-offset, segment roll at 1 MiB, index lookup, recovery on open (recompute next offset from the last segment, reject a gap). One lock per partition; readers do not block each other, writers are serialised. |
| `Registry` | Broker heartbeats: each broker writes `<registry>/broker-<id>.json` (id, host, port, timestamp) every 2 s and prunes files older than 5 s. Live set = files that survive pruning. Controller = lowest live id. No network calls in the registry — it is a shared directory, which is why this design is local-cluster only. |
| `Broker` | Threaded TCP server. Dispatches by message type: CREATE_TOPIC (controller only, others reply with the controller's address), PRODUCE (leader only), FETCH (leader only, followers serve only replication traffic), METADATA (any broker), REPLICATE (follower appends, replies REPLICATE_ACK). |
| `Controller` (role, not a process) | Runs inside whichever broker is currently controller. Assigns partitions round-robin at topic creation: partition p's replicas are brokers `[p % n, (p+1) % n, …]` up to the replication factor, leader = first replica. On leader death it promotes the next live replica and tells followers the new leader. Topic metadata is kept in `<data-dir>/topics.json`, written by the controller only. |
| `Producer` | Bootstrap → metadata → send to the leader. Retries once against fresh metadata when told "not leader". Key → partition by SHA-256; no key → random partition. |
| `Consumer` | Fetch loop from an explicit offset, or from a group's committed offset (read from the leader's `groups.json` via the broker, committed after a successful fetch batch). |
| CLI | `kafka.py broker --id --port --data-dir`, `kafka.py produce --bootstrap --topic [--partition|--key] [--acks] <message>`, `kafka.py consume --bootstrap --topic --partition --offset` or `--group`. No other subcommands in v1. |

## Flows

### Create topic

```
client -> any broker: CREATE_TOPIC
broker (not controller) -> client: ERROR "controller is broker X"
client -> controller: CREATE_TOPIC
controller: assign replicas round-robin, write topics.json
controller -> each replica broker: (in-process for local brokers,
  REPLICATE-style notification in the design; v1 uses the shared
  topics.json + a metadata refresh, not a second wire protocol)
client <- CREATE_TOPIC_RESPONSE ok=1
```

Note the honesty point: because all brokers in v1 share one data
directory root in tests and a topics.json file, "notification" is a
metadata refresh, not a distributed consensus round. A broker that
misses a refresh answers with stale metadata and the client retries
— that retry is part of the spec, not a bug to hide.

### Produce, acks=all

```
producer -> leader: PRODUCE
leader: append to its PartitionLog, assign offset
leader -> each follower: REPLICATE (offset, payload)
follower: append, REPLICATE_ACK
leader: once every live follower acked (or timeout -> ERROR)
leader -> producer: PRODUCE_RESPONSE offset
```

With `acks=1` the leader replies immediately after its own append;
followers still receive the record, but the producer is not told to
wait for them.

### Fetch

```
consumer -> leader: FETCH (topic, partition, offset, max_bytes)
leader: read from PartitionLog from offset, stop at max_bytes
leader -> consumer: FETCH_RESPONSE records in offset order
```

Fetching from a follower is an error naming the leader. Fetching past
the high-water mark returns zero records, not an error. Fetching
before offset 0 or with a negative offset is an error.

### Failover

```
broker 2 (leader of partition 0) stops heartbeating
controller (lowest live id) notices after 5 s
controller: new leader = next live replica, update topics.json
producer/consumer: next request gets ERROR "not leader",
  refreshes metadata, retries against the new leader
follower rejoining: truncate records beyond the new leader's
  high-water mark, then resume replication from there
```

Data written with `acks=all` before the failure is not lost in this
flow, because every live follower had acked it. Data written with
`acks=1` may be lost if the leader dies before replicating — that is
the documented trade-off of `acks=1`, and a test asserts exactly that
boundary rather than pretending otherwise.

## Data layout

```
data/broker-1/
  topics.json            # controller-written, all brokers read
  groups.json            # committed consumer-group offsets
  registry/
    broker-1.json        # heartbeat: id, host, port, timestamp
    broker-2.json
    broker-3.json
  <topic>/
    <partition>/
      00000000000000000000.log
      00000000000000000000.index
      00000000000000001024.log   # after a 1 MiB roll
      ...
```

## Failure handling (decided now, not during debugging)

- Malformed frame / unknown type: close that connection, log, keep
  serving others. No partial append: a record is appended only after
  its full frame is read and validated.
- Follower timeout on `acks=all`: producer gets an error; the leader
  keeps the record (it is already in its log) but does not report it
  as committed — the next produce to that partition must still work.
- Broker restart: reopen logs, recompute offsets, re-register in the
  registry, rejoin as follower until the controller promotes it.
- Two brokers with the same id: the second to start exits with an
  error, it does not steal the first's heartbeat file.
