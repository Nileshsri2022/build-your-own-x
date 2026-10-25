package com.simplekafka.broker;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertArrayEquals;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.util.List;

import org.junit.jupiter.api.Test;

public class ProtocolTest {

    @Test
    void produceRoundTrip() throws Exception {
        ByteArrayOutputStream bytes = new ByteArrayOutputStream();
        DataOutputStream out = new DataOutputStream(bytes);
        Protocol.writeProduce(out, "orders", 1, Protocol.ACKS_ALL, "hello".getBytes());
        out.flush();
        DataInputStream in = new DataInputStream(new ByteArrayInputStream(bytes.toByteArray()));
        assertEquals(Protocol.PRODUCE, Protocol.readType(in));
        Protocol.ProduceRequest req = Protocol.readProduce(in);
        assertEquals("orders", req.topic);
        assertEquals(1, req.partition);
        assertEquals(Protocol.ACKS_ALL, req.acks);
        assertArrayEquals("hello".getBytes(), req.message);
    }

    @Test
    void fetchResponseRoundTrip() throws Exception {
        ByteArrayOutputStream bytes = new ByteArrayOutputStream();
        DataOutputStream out = new DataOutputStream(bytes);
        Protocol.writeFetchResponse(out,
                List.of(new Protocol.Record(3, "a".getBytes()),
                        new Protocol.Record(4, "b".getBytes())));
        out.flush();
        DataInputStream in = new DataInputStream(new ByteArrayInputStream(bytes.toByteArray()));
        assertEquals(Protocol.FETCH_RESPONSE, Protocol.readType(in));
        List<Protocol.Record> records = Protocol.readFetchResponse(in);
        assertEquals(2, records.size());
        assertEquals(3, records.get(0).offset);
        assertEquals("b", new String(records.get(1).message));
    }

    @Test
    void metadataJsonRoundTrip() throws Exception {
        TopicMetadata t = new TopicMetadata();
        t.name = "orders";
        t.partitions.add(new TopicMetadata.PartitionInfo(0, 1, List.of(1, 2)));
        t.partitions.add(new TopicMetadata.PartitionInfo(1, 2, List.of(2, 1)));
        String json = TopicMetadata.allToJson(List.of(t));
        List<TopicMetadata> parsed = TopicMetadata.parseAll(json);
        assertEquals(1, parsed.size());
        assertEquals("orders", parsed.get(0).name);
        assertEquals(2, parsed.get(0).partitions.size());
        assertEquals(2, parsed.get(0).partitions.get(1).leader);
        assertEquals(List.of(2, 1), parsed.get(0).partitions.get(1).followers);
    }

    @Test
    void assignmentAndKeyRouting() {
        List<List<Integer>> assignment =
                PartitionAssignment.assign(3, 2, List.of(1, 2, 3));
        assertEquals(List.of(1, 2), assignment.get(0));
        assertEquals(List.of(2, 3), assignment.get(1));
        assertEquals(List.of(3, 1), assignment.get(2));
        int p = PartitionAssignment.partitionForKey("order-42", 3);
        assertEquals(Math.floorMod("order-42".hashCode(), 3), p);
    }
}
