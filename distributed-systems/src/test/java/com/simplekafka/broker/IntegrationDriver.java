package com.simplekafka.broker;

import java.util.List;

import com.simplekafka.client.SimpleKafkaConsumer;
import com.simplekafka.client.SimpleKafkaProducer;

/** Scripted integration driver, run by tests/integration.sh in two phases. */
public class IntegrationDriver {
    private static void check(boolean ok, String message) {
        if (!ok) {
            throw new AssertionError(message);
        }
        System.out.println("OK: " + message);
    }

    public static void main(String[] args) throws Exception {
        String phase = args[0];
        int port = Integer.parseInt(args[1]);
        // ::1, not localhost: see tests/integration.sh for why.
        SimpleKafkaProducer producer = new SimpleKafkaProducer("::1", port);
        SimpleKafkaConsumer consumer = new SimpleKafkaConsumer("::1", port);
        if ("phase1".equals(phase)) {
            producer.createTopic("orders", 3, 2);
            System.out.println("Created topic orders (3 partitions, replication 2)");
            for (int i = 1; i <= 5; i++) {
                producer.send("orders", 0, "order-" + i);
            }
            for (int i = 1; i <= 3; i++) {
                producer.send("orders", 1, "x-" + i);
            }
            List<String> p0 = consumer.pollStrings("orders", 0, 0, 1024 * 1024);
            check(p0.size() == 5, "partition 0 has 5 messages, got " + p0.size());
            check("order-1".equals(p0.get(0)), "first message is order-1");
            List<String> p1 = consumer.pollStrings("orders", 1, 0, 1024 * 1024);
            check(p1.size() == 3, "partition 1 has 3 messages, got " + p1.size());
            System.out.println("PHASE1 PASS");
        } else if ("phase2".equals(phase)) {
            // Leader for partition 0 was broker 1; after its death the
            // controller should have promoted broker 2.
            List<TopicMetadata> meta =
                    new com.simplekafka.client.SimpleKafkaClient("::1", port).getMetadata();
            int leader = -1;
            for (TopicMetadata t : meta) {
                if ("orders".equals(t.name)) {
                    leader = t.partition(0).leader;
                }
            }
            check(leader == 2, "partition 0 leader promoted to 2, got " + leader);
            producer.send("orders", 0, "order-6");
            List<String> p0 = consumer.pollStrings("orders", 0, 0, 1024 * 1024);
            check(p0.size() == 6, "partition 0 has 6 messages after failover, got " + p0.size());
            check("order-6".equals(p0.get(5)), "last message is order-6");
            System.out.println("PHASE2 PASS");
        } else {
            throw new IllegalArgumentException("Unknown phase: " + phase);
        }
    }
}
