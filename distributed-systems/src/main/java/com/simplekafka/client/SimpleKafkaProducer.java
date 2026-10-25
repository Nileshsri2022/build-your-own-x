package com.simplekafka.client;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.List;

import com.simplekafka.broker.PartitionAssignment;
import com.simplekafka.broker.Protocol;
import com.simplekafka.broker.TopicMetadata;

/**
 * Producer. Default acks=all (tutorial behaviour); acks=1 may lose a
 * record if the leader dies before replicating (see SPEC.md).
 *
 * <p>Run: {@code SimpleKafkaProducer <host> <port> <topic> [partition message]}
 * With no message, sends 10 demo messages like the tutorial.
 */
public class SimpleKafkaProducer {
    private final SimpleKafkaClient client;
    private byte acks = Protocol.ACKS_ALL;

    public SimpleKafkaProducer(String host, int port) {
        this.client = new SimpleKafkaClient(host, port);
    }

    public void setAcks(byte acks) {
        this.acks = acks;
    }

    public long send(String topic, int partition, String message) throws IOException {
        return client.produce(topic, partition, message.getBytes(StandardCharsets.UTF_8), acks);
    }

    public long send(String topic, String message) throws IOException {
        return send(topic, 0, message);
    }

    /** Keyed send: partition = hash(key) mod partitionCount. */
    public long sendWithKey(String topic, String key, String message) throws IOException {
        int partitions = partitionCount(topic);
        int partition = PartitionAssignment.partitionForKey(key, partitions);
        return send(topic, partition, message);
    }

    public int partitionCount(String topic) throws IOException {
        List<TopicMetadata> topics = client.getMetadata();
        for (TopicMetadata t : topics) {
            if (t.name.equals(topic)) {
                return t.partitions.size();
            }
        }
        throw new IOException("Unknown topic: " + topic);
    }

    public void createTopic(String topic, int partitions, int replicationFactor)
            throws IOException {
        client.createTopic(topic, partitions, replicationFactor);
    }

    public static void main(String[] args) throws Exception {
        if (args.length < 3) {
            System.err.println(
                    "Usage: SimpleKafkaProducer <host> <port> <topic> [partition message | message]");
            System.exit(1);
        }
        SimpleKafkaProducer producer = new SimpleKafkaProducer(args[0], Integer.parseInt(args[1]));
        String topic = args[2];
        if (args.length >= 5) {
            int partition = Integer.parseInt(args[3]);
            long offset = producer.send(topic, partition, args[4]);
            System.out.println("Produced to " + topic + "/" + partition + " at offset " + offset);
        } else if (args.length == 4) {
            long offset = producer.send(topic, args[3]);
            System.out.println("Produced to " + topic + "/0 at offset " + offset);
        } else {
            for (int i = 0; i < 10; i++) {
                String message = "Message " + i + " - " + System.currentTimeMillis();
                long offset = producer.send(topic, 0, message);
                System.out.println("Sent: " + message + " (offset " + offset + ")");
                Thread.sleep(1000);
            }
        }
    }
}
