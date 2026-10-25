package com.simplekafka.broker;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.File;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.WatchedEvent;
import org.apache.zookeeper.Watcher;

/**
 * A broker: serves clients, owns partition logs, replicates to followers,
 * and coordinates through ZooKeeper (controller election, broker presence,
 * topic metadata). One broker process per id.
 *
 * <p>Run: {@code SimpleKafkaBroker <id> <host> <port> <zkPort> [dataDir]}
 * with ZooKeeper on localhost. Example (tutorial defaults):
 * {@code SimpleKafkaBroker 1 localhost 9091 2181}
 */
public class SimpleKafkaBroker implements Watcher {
    private final int brokerId;
    private final String host;
    private final int port;
    private final String zkHost;
    private final int zkPort;
    private final File dataDir;
    private final Map<String, Map<Integer, Partition>> partitions = new ConcurrentHashMap<>();
    private final ExecutorService pool = Executors.newCachedThreadPool();

    private ZookeeperClient zk;
    private volatile boolean isController = false;

    public SimpleKafkaBroker(int brokerId, String host, int port, String zkHost, int zkPort,
            File dataDir) {
        this.brokerId = brokerId;
        this.host = host;
        this.port = port;
        this.zkHost = zkHost;
        this.zkPort = zkPort;
        this.dataDir = dataDir;
    }

    public void start() throws Exception {
        zk = new ZookeeperClient(zkHost, zkPort, this);
        zk.ensurePath("/brokers");
        zk.ensurePath("/topics");
        zk.ensurePath("/consumer-groups");
        zk.createEphemeral("/brokers/" + brokerId,
                new BrokerInfo(brokerId, host, port).toZkString());
        loadLocalPartitions();
        tryElection();
        watchBrokers();
        watchController();

        // Bind to the configured host (pass "::1" in this sandbox: direct
        // IPv4 loopback TCP is intercepted, IPv6 loopback is not).
        ServerSocket server =
                new ServerSocket(port, 50, java.net.InetAddress.getByName(host));
        System.out.println("Broker " + brokerId + " listening on " + host + ":" + port
                + (isController ? " (controller)" : ""));
        while (true) {
            Socket socket = server.accept();
            pool.submit(() -> {
                try {
                    handle(socket);
                } catch (Exception e) {
                    System.err.println("Connection error: " + e.getMessage());
                } finally {
                    try {
                        socket.close();
                    } catch (IOException ignored) {
                    }
                }
            });
        }
    }

    // ---- ZooKeeper coordination ----

    private void tryElection() throws KeeperException, InterruptedException {
        boolean won = zk.createEphemeral("/controller", String.valueOf(brokerId));
        if (won) {
            isController = true;
            System.out.println("Broker " + brokerId + " became controller");
            // A new controller must promote leaders off dead brokers
            // right away: the /brokers change that killed the old
            // controller fired before this broker won the election, so
            // waiting for the next /brokers change would miss it.
            try {
                promoteDeadLeaders();
            } catch (Exception e) {
                System.err.println("Promotion after election failed: " + e.getMessage());
            }
        }
    }

    private void watchBrokers() throws KeeperException, InterruptedException {
        zk.getChildren("/brokers", this);
    }

    private void watchController() throws KeeperException, InterruptedException {
        zk.exists("/controller", this);
    }

    @Override
    public void process(WatchedEvent event) {
        try {
            if (event.getType() == Watcher.Event.EventType.NodeChildrenChanged
                    && "/brokers".equals(event.getPath())) {
                watchBrokers();
                if (isController) {
                    promoteDeadLeaders();
                }
            } else if (event.getType() == Watcher.Event.EventType.NodeDeleted
                    && "/controller".equals(event.getPath())) {
                isController = false;
                tryElection();
                watchController();
            } else if (event.getType() == Watcher.Event.EventType.NodeCreated
                    && "/controller".equals(event.getPath())) {
                watchController();
            }
        } catch (Exception e) {
            System.err.println("Watcher error: " + e.getMessage());
        }
    }

    /** Controller only: move leaders off dead brokers onto live replicas. */
    private synchronized void promoteDeadLeaders() throws Exception {
        List<Integer> live = liveBrokerIds();
        List<String> topics = zk.getChildren("/topics", null);
        for (String topic : topics) {
            String data = zk.getData("/topics/" + topic);
            if (data == null) {
                continue;
            }
            TopicMetadata meta = TopicMetadata.parse(data);
            boolean changed = false;
            for (TopicMetadata.PartitionInfo p : meta.partitions) {
                if (!live.contains(p.leader)) {
                    int next = -1;
                    for (int replica : p.followers) {
                        if (live.contains(replica) && replica != p.leader) {
                            next = replica;
                            break;
                        }
                    }
                    if (next < 0) {
                        for (int replica : p.followers) {
                            if (live.contains(replica)) {
                                next = replica;
                                break;
                            }
                        }
                    }
                    if (next >= 0) {
                        System.out.println("Promoting partition " + topic + "/" + p.id
                                + " leader " + p.leader + " -> " + next);
                        p.leader = next;
                        changed = true;
                    }
                }
            }
            if (changed) {
                zk.setData("/topics/" + topic, TopicMetadata.toJson(meta));
            }
        }
    }

    private List<Integer> liveBrokerIds() throws KeeperException, InterruptedException {
        List<Integer> live = new ArrayList<>();
        for (String child : zk.getChildren("/brokers", null)) {
            try {
                live.add(Integer.parseInt(child));
            } catch (NumberFormatException ignored) {
            }
        }
        Collections.sort(live);
        return live;
    }

    private void loadLocalPartitions() throws Exception {
        List<String> topics = zk.getChildren("/topics", null);
        for (String topic : topics) {
            String data = zk.getData("/topics/" + topic);
            if (data == null) {
                continue;
            }
            TopicMetadata meta = TopicMetadata.parse(data);
            for (TopicMetadata.PartitionInfo p : meta.partitions) {
                if (p.leader == brokerId || p.followers.contains(brokerId)) {
                    partition(topic, p.id);
                }
            }
        }
    }

    // ---- partition access ----

    private Partition partition(String topic, int partitionId) throws IOException {
        Map<Integer, Partition> byTopic =
                partitions.computeIfAbsent(topic, k -> new ConcurrentHashMap<>());
        Partition existing = byTopic.get(partitionId);
        if (existing != null) {
            return existing;
        }
        synchronized (byTopic) {
            existing = byTopic.get(partitionId);
            if (existing == null) {
                existing = new Partition(dataDir, topic, partitionId);
                byTopic.put(partitionId, existing);
            }
            return existing;
        }
    }

    private TopicMetadata topicMetadata(String topic) throws Exception {
        String data = zk.getData("/topics/" + topic);
        if (data == null) {
            return null;
        }
        return TopicMetadata.parse(data);
    }

    private BrokerInfo brokerInfo(int id) throws Exception {
        String data = zk.getData("/brokers/" + id);
        if (data == null) {
            return null;
        }
        return BrokerInfo.parse(id, data);
    }

    // ---- request handling ----

    private void handle(Socket socket) throws Exception {
        socket.setSoTimeout(30000);
        DataInputStream in = new DataInputStream(socket.getInputStream());
        DataOutputStream out = new DataOutputStream(socket.getOutputStream());
        while (true) {
            byte type;
            try {
                type = Protocol.readType(in);
            } catch (EOFException e) {
                return; // client closed the connection
            }
            switch (type) {
                case Protocol.PRODUCE:
                    handleProduce(in, out);
                    break;
                case Protocol.FETCH:
                    handleFetch(in, out);
                    break;
                case Protocol.METADATA:
                    handleMetadata(out);
                    break;
                case Protocol.CREATE_TOPIC:
                    handleCreateTopic(in, out);
                    break;
                case Protocol.REPLICATE:
                    handleReplicate(in, out);
                    break;
                default:
                    Protocol.writeError(out, "Unknown request type: " + type);
                    out.flush();
                    return;
            }
            out.flush();
        }
    }

    private void handleProduce(DataInputStream in, DataOutputStream out) throws Exception {
        Protocol.ProduceRequest req = Protocol.readProduce(in);
        TopicMetadata meta = topicMetadata(req.topic);
        if (meta == null) {
            Protocol.writeError(out, "Unknown topic: " + req.topic);
            return;
        }
        TopicMetadata.PartitionInfo p = meta.partition(req.partition);
        if (p == null) {
            Protocol.writeError(out, "Unknown partition: " + req.partition);
            return;
        }
        if (p.leader != brokerId) {
            // Proxy to the leader so any bootstrap broker can serve a
            // produce; the client does not need the leader's address.
            BrokerInfo leader = brokerInfo(p.leader);
            if (leader == null) {
                Protocol.writeError(out, "Leader " + p.leader + " is down");
                return;
            }
            forwardProduce(leader, req, out);
            return;
        }
        long offset = partition(req.topic, req.partition).append(req.message);
        if (req.acks == Protocol.ACKS_ALL) {
            replicate(req.topic, req.partition, offset, req.message, p.followers);
        } else {
            // acks=1: reply now; replication still happens, in the background.
            pool.submit(() -> {
                try {
                    replicate(req.topic, req.partition, offset, req.message, p.followers);
                } catch (Exception e) {
                    System.err.println("Background replication failed: " + e.getMessage());
                }
            });
        }
        Protocol.writeProduceResponse(out, req.topic, req.partition, offset);
    }

    private void replicate(String topic, int partition, long offset, byte[] message,
            List<Integer> followers) throws Exception {
        for (int followerId : followers) {
            if (followerId == brokerId) {
                continue;
            }
            BrokerInfo follower = brokerInfo(followerId);
            if (follower == null) {
                continue; // dead follower: leader keeps serving; see SPEC
            }
            replicateTo(topic, partition, follower, message);
        }
        // Note: replicateTo sends the full catch-up range, ending at the
        // record the leader just appended, so the offset argument is the
        // tail. The per-follower loop above is replaced below.
    }

    /**
     * Sends a follower every record it is missing, then the new record.
     * The follower reports its own tail by replying with an error-style
     * REPLICATE_ACK carrying its next offset when it cannot append.
     */
    private void replicateTo(String topic, int partition, BrokerInfo follower, byte[] message)
            throws Exception {
        // First, ask the follower to append the newest record; if it is
        // behind, it will fail to append and we catch up from its tail.
        // The follower's appendReplicated returns -1 for a gap, which the
        // broker encodes as an ERROR with "gap:<nextOffset>".
        Partition local = partition(topic, partition);
        long tail = local.getNextOffset() - 1;
        for (int attempt = 0; attempt < 1000; attempt++) {
            // Determine the record to send: the follower's tail is unknown
            // until it tells us, so start from the newest record and let
            // the follower's gap reply drive the catch-up loop below.
            // To keep this simple and correct, we send from the follower's
            // last known position, tracked via a probe: send the newest
            // record; on gap, fetch the follower's next offset from the
            // error and replay from there.
            try (Socket socket = new Socket(follower.getHost(), follower.getPort())) {
                socket.setSoTimeout(5000);
                DataOutputStream out = new DataOutputStream(socket.getOutputStream());
                DataInputStream in = new DataInputStream(socket.getInputStream());
                Protocol.writeReplicate(out, topic, partition, tail, messageFor(local, tail));
                out.flush();
                byte response = Protocol.readType(in);
                if (response == Protocol.REPLICATE_ACK) {
                    Protocol.readReplicateAck(in);
                    return;
                }
                if (response == Protocol.ERROR) {
                    String error = Protocol.readError(in);
                    if (error.startsWith("gap:")) {
                        long followerNext = Long.parseLong(error.substring(4));
                        List<Protocol.Record> missing = local.fetch(followerNext,
                                Integer.MAX_VALUE);
                        for (Protocol.Record r : missing) {
                            sendOneReplicate(follower, topic, partition, r);
                        }
                        return;
                    }
                    throw new IOException("Replication failed: " + error);
                }
                throw new IOException("Unexpected replication response: " + response);
            }
        }
        throw new IOException("Replication catch-up did not converge for follower "
                + follower.getId());
    }

    private byte[] messageFor(Partition local, long offset) throws IOException {
        List<Protocol.Record> records = local.fetch(offset, Integer.MAX_VALUE);
        for (Protocol.Record r : records) {
            if (r.offset == offset) {
                return r.message;
            }
        }
        throw new IOException("No record at offset " + offset);
    }

    private void sendOneReplicate(BrokerInfo follower, String topic, int partition,
            Protocol.Record record) throws IOException {
        try (Socket socket = new Socket(follower.getHost(), follower.getPort())) {
            socket.setSoTimeout(5000);
            DataOutputStream out = new DataOutputStream(socket.getOutputStream());
            DataInputStream in = new DataInputStream(socket.getInputStream());
            Protocol.writeReplicate(out, topic, partition, record.offset, record.message);
            out.flush();
            byte response = Protocol.readType(in);
            if (response == Protocol.REPLICATE_ACK) {
                Protocol.readReplicateAck(in);
                return;
            }
            if (response == Protocol.ERROR) {
                throw new IOException("Replication failed: " + Protocol.readError(in));
            }
            throw new IOException("Unexpected replication response: " + response);
        }
    }

    private void forwardProduce(BrokerInfo leader, Protocol.ProduceRequest req,
            DataOutputStream clientOut) throws IOException {
        try (Socket socket = new Socket(leader.getHost(), leader.getPort())) {
            socket.setSoTimeout(10000);
            DataOutputStream out = new DataOutputStream(socket.getOutputStream());
            DataInputStream in = new DataInputStream(socket.getInputStream());
            Protocol.writeProduce(out, req.topic, req.partition, req.acks, req.message);
            out.flush();
            byte type = Protocol.readType(in);
            if (type == Protocol.PRODUCE_RESPONSE) {
                String topic = in.readUTF();
                int partition = in.readInt();
                long offset = in.readLong();
                Protocol.writeProduceResponse(clientOut, topic, partition, offset);
            } else if (type == Protocol.ERROR) {
                Protocol.writeError(clientOut, Protocol.readError(in));
            } else {
                Protocol.writeError(clientOut, "Unexpected leader response: " + type);
            }
        }
    }

    private void forwardCreateTopic(BrokerInfo controller, Protocol.CreateTopicRequest req,
            DataOutputStream clientOut) throws IOException {
        try (Socket socket = new Socket(controller.getHost(), controller.getPort())) {
            socket.setSoTimeout(10000);
            DataOutputStream out = new DataOutputStream(socket.getOutputStream());
            DataInputStream in = new DataInputStream(socket.getInputStream());
            Protocol.writeCreateTopic(out, req.topic, req.partitions, req.replicationFactor);
            out.flush();
            byte type = Protocol.readType(in);
            if (type == Protocol.CREATE_TOPIC_RESPONSE) {
                Protocol.writeCreateTopicResponse(clientOut, in.readBoolean());
            } else if (type == Protocol.ERROR) {
                Protocol.writeError(clientOut, Protocol.readError(in));
            } else {
                Protocol.writeError(clientOut, "Unexpected controller response: " + type);
            }
        }
    }

    private void forwardFetch(BrokerInfo leader, Protocol.FetchRequest req,
            DataOutputStream clientOut) throws IOException {
        try (Socket socket = new Socket(leader.getHost(), leader.getPort())) {
            socket.setSoTimeout(10000);
            DataOutputStream out = new DataOutputStream(socket.getOutputStream());
            DataInputStream in = new DataInputStream(socket.getInputStream());
            Protocol.writeFetch(out, req.topic, req.partition, req.offset, req.maxBytes);
            out.flush();
            byte type = Protocol.readType(in);
            if (type == Protocol.FETCH_RESPONSE) {
                Protocol.writeFetchResponse(clientOut, Protocol.readFetchResponse(in));
            } else if (type == Protocol.ERROR) {
                Protocol.writeError(clientOut, Protocol.readError(in));
            } else {
                Protocol.writeError(clientOut, "Unexpected leader response: " + type);
            }
        }
    }

    private void handleFetch(DataInputStream in, DataOutputStream out) throws Exception {
        Protocol.FetchRequest req = Protocol.readFetch(in);
        TopicMetadata meta = topicMetadata(req.topic);
        if (meta == null) {
            Protocol.writeError(out, "Unknown topic: " + req.topic);
            return;
        }
        TopicMetadata.PartitionInfo p = meta.partition(req.partition);
        if (p == null) {
            Protocol.writeError(out, "Unknown partition: " + req.partition);
            return;
        }
        if (p.leader != brokerId) {
            BrokerInfo leader = brokerInfo(p.leader);
            if (leader == null) {
                Protocol.writeError(out, "Leader " + p.leader + " is down");
                return;
            }
            forwardFetch(leader, req, out);
            return;
        }
        List<Protocol.Record> records =
                partition(req.topic, req.partition).fetch(req.offset, req.maxBytes);
        Protocol.writeFetchResponse(out, records);
    }

    private void handleMetadata(DataOutputStream out) throws Exception {
        List<TopicMetadata> topics = new ArrayList<>();
        for (String topic : zk.getChildren("/topics", null)) {
            String data = zk.getData("/topics/" + topic);
            if (data != null) {
                topics.add(TopicMetadata.parse(data));
            }
        }
        Protocol.writeMetadataResponse(out, TopicMetadata.allToJson(topics));
    }

    private void handleCreateTopic(DataInputStream in, DataOutputStream out) throws Exception {
        Protocol.CreateTopicRequest req = Protocol.readCreateTopic(in);
        if (!req.topic.matches("[A-Za-z0-9._-]+")) {
            Protocol.writeError(out, "Invalid topic name: " + req.topic);
            return;
        }
        if (!isController) {
            // Proxy to the controller, like produce/fetch proxy to the
            // leader, so any bootstrap broker can create a topic.
            String controllerId = zk.getData("/controller");
            if (controllerId == null) {
                Protocol.writeError(out, "No controller");
                return;
            }
            BrokerInfo controller = brokerInfo(Integer.parseInt(controllerId.trim()));
            if (controller == null) {
                // Stale controller node (its broker died, session not
                // yet expired): try to take over, else ask to retry.
                tryElection();
                if (!isController) {
                    Protocol.writeError(out, "Controller is down, retry");
                    return;
                }
                // Fall through to local creation below.
            } else {
                forwardCreateTopic(controller, req, out);
                return;
            }
        }
        if (zk.getData("/topics/" + req.topic) != null) {
            Protocol.writeError(out, "Topic already exists: " + req.topic);
            return;
        }
        List<Integer> live = liveBrokerIds();
        if (live.isEmpty()) {
            Protocol.writeError(out, "No live brokers");
            return;
        }
        if (req.replicationFactor > live.size()) {
            Protocol.writeError(out, "Replication factor exceeds live brokers");
            return;
        }
        TopicMetadata meta = new TopicMetadata();
        meta.name = req.topic;
        List<List<Integer>> assignment =
                PartitionAssignment.assign(req.partitions, req.replicationFactor, live);
        for (int i = 0; i < assignment.size(); i++) {
            List<Integer> replicas = assignment.get(i);
            meta.partitions.add(new TopicMetadata.PartitionInfo(i, replicas.get(0), replicas));
            // Leader creates its partition now; followers create theirs
            // lazily on first replicate.
            if (replicas.get(0) == brokerId) {
                partition(req.topic, i);
            }
        }
        zk.createPersistent("/topics/" + req.topic, TopicMetadata.toJson(meta));
        Protocol.writeCreateTopicResponse(out, true);
    }

    private void handleReplicate(DataInputStream in, DataOutputStream out) throws Exception {
        Protocol.ReplicateRequest req = Protocol.readReplicate(in);
        Partition p = partition(req.topic, req.partition);
        long written = p.appendReplicated(req.offset, req.message);
        if (written < 0) {
            Protocol.writeError(out, "gap:" + p.getNextOffset());
            return;
        }
        Protocol.writeReplicateAck(out, req.topic, req.partition, written);
    }

    public static void main(String[] args) throws Exception {
        if (args.length < 4) {
            System.err.println(
                    "Usage: SimpleKafkaBroker <id> <host> <port> <zkPort> [dataDir]");
            System.exit(1);
        }
        int id = Integer.parseInt(args[0]);
        String host = args[1];
        int port = Integer.parseInt(args[2]);
        int zkPort = Integer.parseInt(args[3]);
        File dataDir = args.length >= 5 ? new File(args[4]) : new File("data/broker-" + id);
        new SimpleKafkaBroker(id, host, port, "localhost", zkPort, dataDir).start();
    }
}
