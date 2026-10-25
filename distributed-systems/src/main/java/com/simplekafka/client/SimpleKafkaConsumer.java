package com.simplekafka.client;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.List;

import com.simplekafka.broker.Protocol;
import com.simplekafka.broker.ZookeeperClient;

/**
 * Consumer. Explicit offsets by default (tutorial behaviour); with a
 * group name, offsets are committed to ZooKeeper at
 * /consumer-groups/&lt;group&gt;/&lt;topic&gt;-&lt;partition&gt; and
 * resumed from there on the next run.
 *
 * <p>Run: {@code SimpleKafkaConsumer <host> <port> <topic> <partition>
 * [startOffset] [group]} — polls in a loop and prints messages.
 */
public class SimpleKafkaConsumer {
    private final SimpleKafkaClient client;
    private final String zkHost;
    private final int zkPort;

    public SimpleKafkaConsumer(String host, int port) {
        this(host, port, "localhost", 2181);
    }

    public SimpleKafkaConsumer(String host, int port, String zkHost, int zkPort) {
        this.client = new SimpleKafkaClient(host, port);
        this.zkHost = zkHost;
        this.zkPort = zkPort;
    }

    public List<Protocol.Record> poll(String topic, int partition, long offset, int maxBytes)
            throws IOException {
        return client.fetch(topic, partition, offset, maxBytes);
    }

    public List<String> pollStrings(String topic, int partition, long offset, int maxBytes)
            throws IOException {
        List<Protocol.Record> records = poll(topic, partition, offset, maxBytes);
        java.util.ArrayList<String> out = new java.util.ArrayList<>();
        for (Protocol.Record r : records) {
            out.add(new String(r.message, StandardCharsets.UTF_8));
        }
        return out;
    }

    public long groupOffset(String group, String topic, int partition) throws Exception {
        try (ZookeeperClient zk = new ZookeeperClient(zkHost, zkPort, null)) {
            String path = "/consumer-groups/" + group + "/" + topic + "-" + partition;
            String data = zk.getData(path);
            return data == null ? 0 : Long.parseLong(data.trim());
        }
    }

    public void commitGroupOffset(String group, String topic, int partition, long offset)
            throws Exception {
        try (ZookeeperClient zk = new ZookeeperClient(zkHost, zkPort, null)) {
            zk.ensurePath("/consumer-groups/" + group);
            String path = "/consumer-groups/" + group + "/" + topic + "-" + partition;
            if (zk.getData(path) == null) {
                zk.createPersistent(path, String.valueOf(offset));
            } else {
                zk.setData(path, String.valueOf(offset));
            }
        }
    }

    /** Polls once from the group's committed offset and commits the new one. */
    public List<String> pollGroup(String group, String topic, int partition, int maxBytes)
            throws Exception {
        long offset = groupOffset(group, topic, partition);
        List<Protocol.Record> records = poll(topic, partition, offset, maxBytes);
        if (!records.isEmpty()) {
            long next = records.get(records.size() - 1).offset + 1;
            commitGroupOffset(group, topic, partition, next);
        }
        java.util.ArrayList<String> out = new java.util.ArrayList<>();
        for (Protocol.Record r : records) {
            out.add(new String(r.message, StandardCharsets.UTF_8));
        }
        return out;
    }

    public static void main(String[] args) throws Exception {
        if (args.length < 4) {
            System.err.println(
                    "Usage: SimpleKafkaConsumer <host> <port> <topic> <partition> "
                            + "[startOffset] [group]");
            System.exit(1);
        }
        SimpleKafkaConsumer consumer = new SimpleKafkaConsumer(args[0], Integer.parseInt(args[1]));
        String topic = args[2];
        int partition = Integer.parseInt(args[3]);
        long offset = args.length >= 5 ? Long.parseLong(args[4]) : 0;
        String group = args.length >= 6 ? args[5] : null;
        if (group != null) {
            offset = consumer.groupOffset(group, topic, partition);
            System.out.println("Group " + group + " resuming at offset " + offset);
        }
        while (true) {
            List<Protocol.Record> records = consumer.poll(topic, partition, offset, 1024 * 1024);
            for (Protocol.Record r : records) {
                System.out.println("Offset " + r.offset + ": "
                        + new String(r.message, StandardCharsets.UTF_8));
                offset = r.offset + 1;
            }
            if (group != null && !records.isEmpty()) {
                consumer.commitGroupOffset(group, topic, partition, offset);
            }
            Thread.sleep(100);
        }
    }
}
