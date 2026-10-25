#!/bin/bash
# Scripted integration test: 3 brokers, create topic, produce, fetch,
# kill the partition-0 leader, verify promotion and continued serving.
# Requires ZooKeeper on localhost:2181 and compiled classes in target/.
# ZK_LIB: directory of zookeeper jars (default: the ZK install's lib/).
set -u
cd "$(dirname "$0")/.."
ZK_LIB="${ZK_LIB:-$HOME/workspace/tools/apache-zookeeper-3.8.1-bin/lib}"
CP="target/classes:$ZK_LIB/*"
JAVA="${JAVA:-$HOME/jdk17/bin/java}"
if [ ! -x "$JAVA" ]; then JAVA=java; fi
TESTDIR=/tmp/kafka-test
rm -rf "$TESTDIR"
mkdir -p "$TESTDIR"
# Remove any leftover state from a previous run (topic, stale broker
# registrations, stale controller node whose session has not expired).
export JAVA_HOME="${JAVA_HOME:-$HOME/jdk17}"
ZKCLI="$HOME/workspace/tools/apache-zookeeper-3.8.1-bin/bin/zkCli.sh"
"$ZKCLI" -server "[::1]:2181" deleteall /topics/orders >/dev/null 2>&1 || true
"$ZKCLI" -server "[::1]:2181" deleteall /brokers >/dev/null 2>&1 || true
"$ZKCLI" -server "[::1]:2181" delete /controller >/dev/null 2>&1 || true
cleanup() {
  kill $B1 $B2 $B3 2>/dev/null || true
}
trap cleanup EXIT
# Host is ::1, not localhost: in this sandbox, Java TCP to 127.0.0.1 is
# intercepted ("Other TCP connections is turned off"), while ::1 works.
# On a normal machine, localhost works the same way; ::1 is equivalent.
"$JAVA" -cp "$CP" com.simplekafka.broker.SimpleKafkaBroker 1 ::1 9091 2181 "$TESTDIR/broker-1" > "$TESTDIR/broker-1.log" 2>&1 & B1=$!
"$JAVA" -cp "$CP" com.simplekafka.broker.SimpleKafkaBroker 2 ::1 9092 2181 "$TESTDIR/broker-2" > "$TESTDIR/broker-2.log" 2>&1 & B2=$!
"$JAVA" -cp "$CP" com.simplekafka.broker.SimpleKafkaBroker 3 ::1 9093 2181 "$TESTDIR/broker-3" > "$TESTDIR/broker-3.log" 2>&1 & B3=$!
# Brokers need time for ZooKeeper registration (the ZK client cycles
# through localhost addresses, some of which are intercepted in this
# sandbox). Wait until all three report listening, up to 30s.
for i in $(seq 1 30); do
  if grep -q "listening" "$TESTDIR/broker-1.log" 2>/dev/null \
    && grep -q "listening" "$TESTDIR/broker-2.log" 2>/dev/null \
    && grep -q "listening" "$TESTDIR/broker-3.log" 2>/dev/null; then
    break
  fi
  sleep 1
done
"$JAVA" -cp "$CP:target/test-classes" com.simplekafka.broker.IntegrationDriver phase1 9091 || exit 1
# Follower log must exist on broker 2 (replica of partition 0).
ls "$TESTDIR/broker-2/orders/0/"*.log >/dev/null || { echo "FAIL: no follower log on broker 2"; exit 1; }
echo "OK: follower log exists on broker 2"
kill $B1
# Wait for ZooKeeper to expire broker 1's session (10s timeout) and drop
# its ephemeral /brokers/1 node; until then it still looks live.
for i in $(seq 1 30); do
  if ! "$HOME/workspace/tools/apache-zookeeper-3.8.1-bin/bin/zkCli.sh" -server "[::1]:2181" ls /brokers 2>/dev/null | grep -q "\[1"; then
    break
  fi
  sleep 1
done
sleep 3
"$JAVA" -cp "$CP:target/test-classes" com.simplekafka.broker.IntegrationDriver phase2 9092 || exit 1
echo "INTEGRATION PASS"
