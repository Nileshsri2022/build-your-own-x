package com.simplekafka.broker;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * The tutorial wire protocol, exactly as documented in docs/SPEC.md.
 *
 * <p>Every message starts with one type byte. Fields follow in the fixed
 * order given below; there is no length prefix on the message itself.
 * Strings are UTF-8 with a 2-byte length prefix (DataInput/Output UTF).
 * The two documented extensions are the acks byte on PRODUCE and the
 * length-prefixed JSON body on METADATA_RESPONSE; both are in SPEC.md.
 */
public final class Protocol {
    private Protocol() {}

    public static final byte PRODUCE = 0x01;
    public static final byte FETCH = 0x02;
    public static final byte METADATA = 0x03;
    public static final byte CREATE_TOPIC = 0x04;
    public static final byte PRODUCE_RESPONSE = 0x11;
    public static final byte FETCH_RESPONSE = 0x12;
    public static final byte METADATA_RESPONSE = 0x13;
    public static final byte CREATE_TOPIC_RESPONSE = 0x14;
    public static final byte ERROR = 0x1F;
    public static final byte REPLICATE = 0x21;
    public static final byte REPLICATE_ACK = 0x22;
    public static final byte TOPIC_NOTIFICATION = 0x23;

    public static final byte ACKS_ALL = 0;
    public static final byte ACKS_ONE = 1;

    public static class ProduceRequest {
        public String topic;
        public int partition;
        public byte acks;
        public byte[] message;
    }

    public static class FetchRequest {
        public String topic;
        public int partition;
        public long offset;
        public int maxBytes;
    }

    public static class CreateTopicRequest {
        public String topic;
        public int partitions;
        public int replicationFactor;
    }

    public static class ReplicateRequest {
        public String topic;
        public int partition;
        public long offset;
        public byte[] message;
    }

    public static class Record {
        public long offset;
        public byte[] message;

        public Record(long offset, byte[] message) {
            this.offset = offset;
            this.message = message;
        }
    }

    // ---- requests ----

    public static void writeProduce(DataOutputStream out, String topic, int partition, byte acks,
            byte[] message) throws IOException {
        out.writeByte(PRODUCE);
        out.writeUTF(topic);
        out.writeInt(partition);
        out.writeByte(acks);
        out.writeInt(message.length);
        out.write(message);
    }

    public static ProduceRequest readProduce(DataInputStream in) throws IOException {
        ProduceRequest r = new ProduceRequest();
        r.topic = in.readUTF();
        r.partition = in.readInt();
        r.acks = in.readByte();
        int len = in.readInt();
        r.message = new byte[len];
        in.readFully(r.message);
        return r;
    }

    public static void writeFetch(DataOutputStream out, String topic, int partition, long offset,
            int maxBytes) throws IOException {
        out.writeByte(FETCH);
        out.writeUTF(topic);
        out.writeInt(partition);
        out.writeLong(offset);
        out.writeInt(maxBytes);
    }

    public static FetchRequest readFetch(DataInputStream in) throws IOException {
        FetchRequest r = new FetchRequest();
        r.topic = in.readUTF();
        r.partition = in.readInt();
        r.offset = in.readLong();
        r.maxBytes = in.readInt();
        return r;
    }

    public static void writeMetadata(DataOutputStream out) throws IOException {
        out.writeByte(METADATA);
    }

    public static void writeCreateTopic(DataOutputStream out, String topic, int partitions,
            int replicationFactor) throws IOException {
        out.writeByte(CREATE_TOPIC);
        out.writeUTF(topic);
        out.writeInt(partitions);
        out.writeInt(replicationFactor);
    }

    public static CreateTopicRequest readCreateTopic(DataInputStream in) throws IOException {
        CreateTopicRequest r = new CreateTopicRequest();
        r.topic = in.readUTF();
        r.partitions = in.readInt();
        r.replicationFactor = in.readInt();
        return r;
    }

    public static void writeReplicate(DataOutputStream out, String topic, int partition, long offset,
            byte[] message) throws IOException {
        out.writeByte(REPLICATE);
        out.writeUTF(topic);
        out.writeInt(partition);
        out.writeLong(offset);
        out.writeInt(message.length);
        out.write(message);
    }

    public static ReplicateRequest readReplicate(DataInputStream in) throws IOException {
        ReplicateRequest r = new ReplicateRequest();
        r.topic = in.readUTF();
        r.partition = in.readInt();
        r.offset = in.readLong();
        int len = in.readInt();
        r.message = new byte[len];
        in.readFully(r.message);
        return r;
    }

    // ---- responses ----

    public static void writeProduceResponse(DataOutputStream out, String topic, int partition,
            long offset) throws IOException {
        out.writeByte(PRODUCE_RESPONSE);
        out.writeUTF(topic);
        out.writeInt(partition);
        out.writeLong(offset);
    }

    public static void writeFetchResponse(DataOutputStream out, List<Record> records)
            throws IOException {
        out.writeByte(FETCH_RESPONSE);
        out.writeInt(records.size());
        for (Record r : records) {
            out.writeLong(r.offset);
            out.writeInt(r.message.length);
            out.write(r.message);
        }
    }

    public static List<Record> readFetchResponse(DataInputStream in) throws IOException {
        int count = in.readInt();
        List<Record> records = new ArrayList<>(count);
        for (int i = 0; i < count; i++) {
            long offset = in.readLong();
            int len = in.readInt();
            byte[] message = new byte[len];
            in.readFully(message);
            records.add(new Record(offset, message));
        }
        return records;
    }

    public static void writeMetadataResponse(DataOutputStream out, String json) throws IOException {
        byte[] body = json.getBytes("UTF-8");
        out.writeByte(METADATA_RESPONSE);
        out.writeInt(body.length);
        out.write(body);
    }

    public static String readMetadataResponse(DataInputStream in) throws IOException {
        int len = in.readInt();
        byte[] body = new byte[len];
        in.readFully(body);
        return new String(body, "UTF-8");
    }

    public static void writeCreateTopicResponse(DataOutputStream out, boolean ok)
            throws IOException {
        out.writeByte(CREATE_TOPIC_RESPONSE);
        out.writeBoolean(ok);
    }

    public static void writeError(DataOutputStream out, String message) throws IOException {
        out.writeByte(ERROR);
        out.writeUTF(message);
    }

    public static String readError(DataInputStream in) throws IOException {
        return in.readUTF();
    }

    public static void writeReplicateAck(DataOutputStream out, String topic, int partition,
            long offset) throws IOException {
        out.writeByte(REPLICATE_ACK);
        out.writeUTF(topic);
        out.writeInt(partition);
        out.writeLong(offset);
    }

    /** Reads a REPLICATE_ACK body (topic/partition/offset) after the type byte. */
    public static ReplicateRequest readReplicateAck(DataInputStream in) throws IOException {
        ReplicateRequest r = new ReplicateRequest();
        r.topic = in.readUTF();
        r.partition = in.readInt();
        r.offset = in.readLong();
        return r;
    }

    /** Reads one response-type byte, throwing on EOF (client disconnected). */
    public static byte readType(DataInputStream in) throws IOException {
        return in.readByte();
    }
}
