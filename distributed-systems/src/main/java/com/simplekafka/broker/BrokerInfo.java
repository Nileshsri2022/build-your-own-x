package com.simplekafka.broker;

/** Value object: one broker's identity and address. */
public class BrokerInfo {
    private final int id;
    private final String host;
    private final int port;

    public BrokerInfo(int id, String host, int port) {
        this.id = id;
        this.host = host;
        this.port = port;
    }

    public int getId() {
        return id;
    }

    public String getHost() {
        return host;
    }

    public int getPort() {
        return port;
    }

    /**
     * Encoded as "host|port" in ZooKeeper broker nodes (|, not :, so
     * IPv6 hosts like ::1 survive the encoding).
     */
    public String toZkString() {
        return host + "|" + port;
    }

    public static BrokerInfo parse(int id, String value) {
        int sep = value.lastIndexOf('|');
        if (sep >= 0) {
            return new BrokerInfo(id, value.substring(0, sep),
                    Integer.parseInt(value.substring(sep + 1)));
        }
        // Legacy "host:port" form (IPv4/hostname only).
        String[] parts = value.split(":");
        return new BrokerInfo(id, parts[0], Integer.parseInt(parts[1]));
    }

    @Override
    public String toString() {
        return host + ":" + port;
    }
}
