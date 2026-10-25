package com.simplekafka.broker;

import java.util.ArrayList;
import java.util.List;

/**
 * Topic metadata as stored in ZooKeeper at /topics/&lt;topic&gt; and as
 * returned by METADATA_RESPONSE. JSON is built and parsed by hand (no
 * extra dependency) for this one fixed shape:
 * {@code {"name":"t","partitions":[{"id":0,"leader":1,"followers":[2]}]}}
 * Topic names are restricted to [A-Za-z0-9._-] so no escaping is needed.
 */
public class TopicMetadata {
    public static class PartitionInfo {
        public int id;
        public int leader;
        public List<Integer> followers = new ArrayList<>();

        public PartitionInfo(int id, int leader, List<Integer> followers) {
            this.id = id;
            this.leader = leader;
            this.followers = followers;
        }
    }

    public String name;
    public List<PartitionInfo> partitions = new ArrayList<>();

    public static String toJson(TopicMetadata t) {
        StringBuilder sb = new StringBuilder();
        sb.append("{\"name\":\"").append(t.name).append("\",\"partitions\":[");
        for (int i = 0; i < t.partitions.size(); i++) {
            PartitionInfo p = t.partitions.get(i);
            if (i > 0) {
                sb.append(',');
            }
            sb.append("{\"id\":").append(p.id).append(",\"leader\":").append(p.leader)
                    .append(",\"followers\":[");
            for (int j = 0; j < p.followers.size(); j++) {
                if (j > 0) {
                    sb.append(',');
                }
                sb.append(p.followers.get(j));
            }
            sb.append("]}");
        }
        sb.append("]}");
        return sb.toString();
    }

    public static String allToJson(List<TopicMetadata> topics) {
        StringBuilder sb = new StringBuilder();
        sb.append("{\"topics\":[");
        for (int i = 0; i < topics.size(); i++) {
            if (i > 0) {
                sb.append(',');
            }
            sb.append(toJson(topics.get(i)));
        }
        sb.append("]}");
        return sb.toString();
    }

    /** Parses the single-topic shape produced by {@link #toJson}. */
    public static TopicMetadata parse(String json) {
        TopicMetadata t = new TopicMetadata();
        t.name = stringField(json, "\"name\":\"");
        int partitionsStart = json.indexOf("\"partitions\":[", 0);
        if (partitionsStart < 0) {
            return t;
        }
        int i = partitionsStart + "\"partitions\":[".length();
        while (i < json.length()) {
            int idPos = json.indexOf("\"id\":", i);
            if (idPos < 0) {
                break;
            }
            int id = intAfter(json, idPos + "\"id\":".length());
            int leaderPos = json.indexOf("\"leader\":", idPos);
            int leader = intAfter(json, leaderPos + "\"leader\":".length());
            int followersPos = json.indexOf("\"followers\":[", leaderPos);
            int close = json.indexOf(']', followersPos);
            List<Integer> followers = new ArrayList<>();
            String list = json.substring(followersPos + "\"followers\":[".length(), close).trim();
            if (!list.isEmpty()) {
                for (String part : list.split(",")) {
                    followers.add(Integer.parseInt(part.trim()));
                }
            }
            t.partitions.add(new PartitionInfo(id, leader, followers));
            i = close + 1;
            if (i >= json.length() || json.charAt(i) == ']') {
                break;
            }
        }
        return t;
    }

    /** Parses the {"topics":[...]} shape produced by {@link #allToJson}. */
    public static List<TopicMetadata> parseAll(String json) {
        List<TopicMetadata> topics = new ArrayList<>();
        int i = 0;
        while (true) {
            int namePos = json.indexOf("\"name\":\"", i);
            if (namePos < 0) {
                break;
            }
            // Find the matching closing "]}" for this topic object: the
            // partitions array ends with "]}" followed by "," or "]}".
            int partitionsPos = json.indexOf("\"partitions\":[", namePos);
            int depth = 0;
            int j = partitionsPos + "\"partitions\":[".length();
            int end = -1;
            for (; j < json.length(); j++) {
                char c = json.charAt(j);
                if (c == '[') {
                    depth++;
                } else if (c == ']') {
                    depth--;
                    if (depth == -1) {
                        // end of partitions array; the topic object closes
                        // with '}' right after.
                        end = j + 1;
                        break;
                    }
                }
            }
            if (end < 0) {
                break;
            }
            topics.add(parse(json.substring(namePos - 1, end + 1)));
            i = end + 1;
        }
        return topics;
    }

    private static String stringField(String json, String marker) {
        int start = json.indexOf(marker);
        if (start < 0) {
            return "";
        }
        start += marker.length();
        int end = json.indexOf('"', start);
        return json.substring(start, end);
    }

    private static int intAfter(String json, int pos) {
        int end = pos;
        while (end < json.length()
                && (Character.isDigit(json.charAt(end)) || json.charAt(end) == '-')) {
            end++;
        }
        return Integer.parseInt(json.substring(pos, end));
    }

    public PartitionInfo partition(int id) {
        for (PartitionInfo p : partitions) {
            if (p.id == id) {
                return p;
            }
        }
        return null;
    }
}
