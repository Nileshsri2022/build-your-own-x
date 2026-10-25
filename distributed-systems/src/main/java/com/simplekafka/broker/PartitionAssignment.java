package com.simplekafka.broker;

import java.util.ArrayList;
import java.util.List;

/** Pure helpers for replica assignment and key routing; unit-tested. */
public final class PartitionAssignment {
    private PartitionAssignment() {}

    /**
     * Round-robin replicas: partition p starts at broker (p % brokerCount)
     * and takes {@code replicationFactor} brokers, wrapping around.
     */
    public static List<List<Integer>> assign(int partitions, int replicationFactor,
            List<Integer> brokerIds) {
        List<List<Integer>> result = new ArrayList<>();
        if (brokerIds.isEmpty()) {
            return result;
        }
        int factor = Math.min(replicationFactor, brokerIds.size());
        for (int p = 0; p < partitions; p++) {
            List<Integer> replicas = new ArrayList<>();
            for (int i = 0; i < factor; i++) {
                replicas.add(brokerIds.get((p + i) % brokerIds.size()));
            }
            result.add(replicas);
        }
        return result;
    }

    /** Partition for a key: hash(key) mod partitionCount, always >= 0. */
    public static int partitionForKey(String key, int partitionCount) {
        return Math.floorMod(key.hashCode(), partitionCount);
    }
}
