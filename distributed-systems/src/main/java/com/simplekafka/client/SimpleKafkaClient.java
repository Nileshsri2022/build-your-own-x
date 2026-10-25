package com.simplekafka.client;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.List;

import com.simplekafka.broker.Protocol;
import com.simplekafka.broker.TopicMetadata;

/** One-request-per-connection client, as in the tutorial. */
public class SimpleKafkaClient {
    private final String host;
    private final int port;

    public SimpleKafkaClient(String host, int port) {
        this.host = host;
        this.port = port;
    }

    public String getHost() {
        return host;
    }

    public int getPort() {
        return port;
    }

    private static class Response {
        byte type;
        DataInputStream in;
    }

    /** Sends a request and returns the raw response stream; caller closes. */
    @FunctionalInterface
    interface RequestWriter {
        void write(DataOutputStream out) throws IOException;
    }

    public void createTopic(String topic, int partitions, int replicationFactor)
            throws IOException {
        try (Socket socket = new Socket(host, port)) {
            socket.setSoTimeout(10000);
            DataOutputStream out = new DataOutputStream(socket.getOutputStream());
            DataInputStream in = new DataInputStream(socket.getInputStream());
            Protocol.writeCreateTopic(out, topic, partitions, replicationFactor);
            out.flush();
            byte type = Protocol.readType(in);
            if (type == Protocol.CREATE_TOPIC_RESPONSE) {
                if (!in.readBoolean()) {
                    throw new IOException("Create topic failed");
                }
                return;
            }
            throw error(type, in);
        }
    }

    public List<TopicMetadata> getMetadata() throws IOException {
        try (Socket socket = new Socket(host, port)) {
            socket.setSoTimeout(10000);
            DataOutputStream out = new DataOutputStream(socket.getOutputStream());
            DataInputStream in = new DataInputStream(socket.getInputStream());
            Protocol.writeMetadata(out);
            out.flush();
            byte type = Protocol.readType(in);
            if (type == Protocol.METADATA_RESPONSE) {
                return TopicMetadata.parseAll(Protocol.readMetadataResponse(in));
            }
            throw error(type, in);
        }
    }

    public TopicMetadata.PartitionInfo partitionInfo(String topic, int partition)
            throws IOException {
        for (TopicMetadata t : getMetadata()) {
            if (t.name.equals(topic)) {
                TopicMetadata.PartitionInfo p = t.partition(partition);
                if (p != null) {
                    return p;
                }
            }
        }
        throw new IOException("Unknown topic/partition: " + topic + "/" + partition);
    }

    /** Produces to the leader, retrying once after a leader-change error. */
    public long produce(String topic, int partition, byte[] message, byte acks)
            throws IOException {
        IOException lastError = null;
        for (int attempt = 0; attempt < 2; attempt++) {
            // Validate the topic/partition exists first; the broker then
            // proxies the request to the current leader, so the client
            // does not need the leader's address.
            partitionInfo(topic, partition);
            try (Socket socket = new Socket(host, port)) {
                socket.setSoTimeout(10000);
                DataOutputStream out = new DataOutputStream(socket.getOutputStream());
                DataInputStream in = new DataInputStream(socket.getInputStream());
                Protocol.writeProduce(out, topic, partition, acks, message);
                out.flush();
                byte type = Protocol.readType(in);
                if (type == Protocol.PRODUCE_RESPONSE) {
                    in.readUTF();
                    in.readInt();
                    return in.readLong();
                }
                lastError = error(type, in);
            }
        }
        throw lastError;
    }

    public List<Protocol.Record> fetch(String topic, int partition, long offset, int maxBytes)
            throws IOException {
        try (Socket socket = new Socket(host, port)) {
            socket.setSoTimeout(10000);
            DataOutputStream out = new DataOutputStream(socket.getOutputStream());
            DataInputStream in = new DataInputStream(socket.getInputStream());
            Protocol.writeFetch(out, topic, partition, offset, maxBytes);
            out.flush();
            byte type = Protocol.readType(in);
            if (type == Protocol.FETCH_RESPONSE) {
                return Protocol.readFetchResponse(in);
            }
            throw error(type, in);
        }
    }

    private static IOException error(byte type, DataInputStream in) throws IOException {
        if (type == Protocol.ERROR) {
            return new IOException(Protocol.readError(in));
        }
        return new IOException("Unexpected response type: " + type);
    }
}
