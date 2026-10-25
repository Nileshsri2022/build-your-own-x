package com.simplekafka.broker;

import static org.junit.jupiter.api.Assertions.assertEquals;

import java.io.File;
import java.nio.file.Files;
import java.util.List;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

public class PartitionTest {

    @Test
    void appendFetchAndRecover(@TempDir File base) throws Exception {
        Partition p = new Partition(base, "orders", 0);
        assertEquals(0, p.append("one".getBytes()));
        assertEquals(1, p.append("two".getBytes()));
        List<Protocol.Record> records = p.fetch(0, 1024);
        assertEquals(2, records.size());
        assertEquals("two", new String(records.get(1).message));
        p.close();

        // Reopen: recovery must find both records, not phantom ones.
        Partition reopened = new Partition(base, "orders", 0);
        assertEquals(2, reopened.getNextOffset());
        assertEquals(1, reopened.fetch(1, 1024).size());
        reopened.close();
    }

    @Test
    void truncateOnRejoin(@TempDir File base) throws Exception {
        Partition p = new Partition(base, "orders", 0);
        p.append("a".getBytes());
        p.append("b".getBytes());
        p.append("c".getBytes());
        p.truncateTo(1);
        assertEquals(1, p.getNextOffset());
        assertEquals(1, p.append("b2".getBytes())); // offset 1 reused
        List<Protocol.Record> records = p.fetch(0, 1024);
        assertEquals(2, records.size());
        assertEquals("b2", new String(records.get(1).message));
        p.close();
    }

    @Test
    void replicateGap(@TempDir File base) throws Exception {
        Partition p = new Partition(base, "orders", 0);
        p.append("a".getBytes());
        // Expected offset 5 is ahead of the tail (1): must not invent a gap.
        assertEquals(-1, p.appendReplicated(5, "x".getBytes()));
        assertEquals(1, p.getNextOffset());
        // Expected offset 0 is behind: truncate, then append.
        assertEquals(0, p.appendReplicated(0, "a2".getBytes()));
        assertEquals("a2", new String(p.fetch(0, 1024).get(0).message));
        p.close();
    }
}
