package com.simplekafka.broker;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * A durable, append-only log for one topic partition.
 *
 * <p>On disk: {@code <base-offset>.log} holds records as {@code uint32 len +
 * payload}; {@code <base-offset>.index} holds {@code uint64 offset + uint64
 * position} per record. Segments roll at 1 MiB. Recovery scans the log and
 * rebuilds the index when it disagrees, so a crash mid-append cannot leave
 * a phantom record: only fully written records are counted.
 */
public class Partition {
    public static final long SEGMENT_SIZE = 1024L * 1024L;

    private final File dir;
    private final String topic;
    private final int partitionId;
    private final ReentrantReadWriteLock lock = new ReentrantReadWriteLock();

    private File currentLogFile;
    private File currentIndexFile;
    private FileChannel logChannel;
    private FileChannel indexChannel;
    private long baseOffset;
    private long nextOffset;
    private long currentSegmentSize;

    public Partition(File baseDir, String topic, int partitionId) throws IOException {
        this.topic = topic;
        this.partitionId = partitionId;
        this.dir = new File(baseDir, topic + File.separator + partitionId);
        if (!dir.mkdirs() && !dir.isDirectory()) {
            throw new IOException("Cannot create partition dir: " + dir);
        }
        recover();
    }

    private void recover() throws IOException {
        File[] logs = dir.listFiles((d, name) -> name.endsWith(".log"));
        long maxNext = 0;
        if (logs != null) {
            java.util.Arrays.sort(logs, (a, b) -> Long.compare(baseOf(a), baseOf(b)));
            for (File log : logs) {
                long base = baseOf(log);
                long count = countRecords(log);
                // Rebuild the index for this segment from the log itself.
                writeIndex(new File(dir, base + ".index"), base, log);
                maxNext = Math.max(maxNext, base + count);
            }
        }
        if (logs == null || logs.length == 0) {
            openSegment(0);
        } else {
            File last = logs[logs.length - 1];
            long base = baseOf(last);
            long count = countRecords(last);
            openSegment(base);
            // Position the channels at the end of the last segment.
            logChannel.position(logChannel.size());
            indexChannel.position(indexChannel.size());
            currentSegmentSize = logChannel.size();
            nextOffset = base + count;
            // If the last segment is full, roll now so the next append lands
            // in a fresh segment.
            if (currentSegmentSize >= SEGMENT_SIZE) {
                closeChannels();
                openSegment(nextOffset);
            }
        }
        if (maxNext > nextOffset) {
            nextOffset = maxNext;
        }
    }

    private static long baseOf(File log) {
        String name = log.getName();
        return Long.parseLong(name.substring(0, name.length() - ".log".length()));
    }

    private static long countRecords(File log) throws IOException {
        long count = 0;
        try (RandomAccessFile raf = new RandomAccessFile(log, "r")) {
            while (raf.getFilePointer() < raf.length()) {
                long remaining = raf.length() - raf.getFilePointer();
                if (remaining < 4) {
                    break; // torn tail: partial length, not a record
                }
                int len = raf.readInt();
                if (len < 0 || remaining - 4 < len) {
                    break; // torn tail: partial payload, not a record
                }
                raf.skipBytes(len);
                count++;
            }
        }
        return count;
    }

    private static void writeIndex(File indexFile, long base, File log) throws IOException {
        try (RandomAccessFile raf = new RandomAccessFile(log, "r");
                FileChannel idx = new FileChannelWrapper(indexFile)) {
            long position = 0;
            long offset = base;
            while (raf.getFilePointer() < raf.length()) {
                long remaining = raf.length() - raf.getFilePointer();
                if (remaining < 4) {
                    break;
                }
                int len = raf.readInt();
                if (len < 0 || remaining - 4 < len) {
                    break;
                }
                ByteBuffer buf = ByteBuffer.allocate(16);
                buf.putLong(offset);
                buf.putLong(position);
                buf.flip();
                while (buf.hasRemaining()) {
                    idx.write(buf);
                }
                raf.skipBytes(len);
                position += 4L + len;
                offset++;
            }
        }
    }

    /** Small helper so writeIndex can use try-with-resources on a channel. */
    private static class FileChannelWrapper extends FileChannel {
        private final RandomAccessFile raf;

        FileChannelWrapper(File file) throws IOException {
            this.raf = new RandomAccessFile(file, "rw");
            this.raf.setLength(0);
        }

        @Override
        protected void implCloseChannel() throws IOException {
            raf.close();
        }

        @Override
        public int read(ByteBuffer dst) throws IOException {
            return raf.getChannel().read(dst);
        }

        @Override
        public long read(ByteBuffer[] dsts, int offset, int length) throws IOException {
            return raf.getChannel().read(dsts, offset, length);
        }

        @Override
        public int write(ByteBuffer src) throws IOException {
            return raf.getChannel().write(src);
        }

        @Override
        public long write(ByteBuffer[] srcs, int offset, int length) throws IOException {
            return raf.getChannel().write(srcs, offset, length);
        }

        @Override
        public long position() throws IOException {
            return raf.getChannel().position();
        }

        @Override
        public FileChannel position(long newPosition) throws IOException {
            raf.getChannel().position(newPosition);
            return this;
        }

        @Override
        public long size() throws IOException {
            return raf.getChannel().size();
        }

        @Override
        public FileChannel truncate(long size) throws IOException {
            raf.getChannel().truncate(size);
            return this;
        }

        @Override
        public void force(boolean metaData) throws IOException {
            raf.getChannel().force(metaData);
        }

        @Override
        public long transferTo(long position, long count, java.nio.channels.WritableByteChannel target)
                throws IOException {
            return raf.getChannel().transferTo(position, count, target);
        }

        @Override
        public long transferFrom(java.nio.channels.ReadableByteChannel src, long position,
                long count) throws IOException {
            return raf.getChannel().transferFrom(src, position, count);
        }

        @Override
        public int read(ByteBuffer dst, long position) throws IOException {
            return raf.getChannel().read(dst, position);
        }

        @Override
        public int write(ByteBuffer src, long position) throws IOException {
            return raf.getChannel().write(src, position);
        }

        @Override
        public java.nio.MappedByteBuffer map(MapMode mode, long position, long size)
                throws IOException {
            return raf.getChannel().map(mode, position, size);
        }

        @Override
        public java.nio.channels.FileLock lock(long position, long size, boolean shared)
                throws IOException {
            return raf.getChannel().lock(position, size, shared);
        }

        @Override
        public java.nio.channels.FileLock tryLock(long position, long size, boolean shared)
                throws IOException {
            return raf.getChannel().tryLock(position, size, shared);
        }
    }

    private void openSegment(long base) throws IOException {
        baseOffset = base;
        currentLogFile = new File(dir, base + ".log");
        currentIndexFile = new File(dir, base + ".index");
        logChannel = new RandomAccessFile(currentLogFile, "rw").getChannel();
        indexChannel = new RandomAccessFile(currentIndexFile, "rw").getChannel();
        currentSegmentSize = logChannel.size();
        if (nextOffset < base) {
            nextOffset = base;
        }
    }

    private void closeChannels() throws IOException {
        if (logChannel != null) {
            logChannel.close();
        }
        if (indexChannel != null) {
            indexChannel.close();
        }
    }

    public long getNextOffset() {
        lock.readLock().lock();
        try {
            return nextOffset;
        } finally {
            lock.readLock().unlock();
        }
    }

    /** Appends at the tail and returns the assigned offset. */
    public long append(byte[] message) throws IOException {
        lock.writeLock().lock();
        try {
            if (currentSegmentSize + 4L + message.length > SEGMENT_SIZE) {
                closeChannels();
                openSegment(nextOffset);
            }
            long position = currentSegmentSize;
            long assigned = nextOffset;
            ByteBuffer record = ByteBuffer.allocate(4 + message.length);
            record.putInt(message.length);
            record.put(message);
            record.flip();
            while (record.hasRemaining()) {
                logChannel.write(record);
            }
            ByteBuffer indexEntry = ByteBuffer.allocate(16);
            indexEntry.putLong(assigned);
            indexEntry.putLong(position);
            indexEntry.flip();
            while (indexEntry.hasRemaining()) {
                indexChannel.write(indexEntry);
            }
            logChannel.force(false);
            indexChannel.force(false);
            currentSegmentSize += 4L + message.length;
            nextOffset++;
            return assigned;
        } finally {
            lock.writeLock().unlock();
        }
    }

    /**
     * Appends a replicated record, enforcing the expected offset.
     *
     * @return the offset the record was written at, or -1 when the caller
     *         must catch up first (expected offset ahead of our tail, or
     *         the record is already present).
     */
    public long appendReplicated(long expectedOffset, byte[] message) throws IOException {
        lock.writeLock().lock();
        try {
            if (expectedOffset < nextOffset) {
                // Divergent tail (leader changed): truncate, then append.
                truncateLocked(expectedOffset);
            }
            if (expectedOffset != nextOffset) {
                return -1;
            }
            return append(message);
        } finally {
            lock.writeLock().unlock();
        }
    }

    /** Drops every record at or after {@code offset}. Used on rejoin. */
    public void truncateTo(long offset) throws IOException {
        lock.writeLock().lock();
        try {
            truncateLocked(offset);
        } finally {
            lock.writeLock().unlock();
        }
    }

    private void truncateLocked(long offset) throws IOException {
        if (offset >= nextOffset) {
            return;
        }
        if (offset <= 0) {
            closeChannels();
            for (File f : dir.listFiles()) {
                f.delete();
            }
            nextOffset = 0;
            openSegment(0);
            return;
        }
        // Find the segment containing offset, truncate it, drop later ones.
        File[] logs = dir.listFiles((d, name) -> name.endsWith(".log"));
        if (logs == null) {
            return;
        }
        java.util.Arrays.sort(logs, (a, b) -> Long.compare(baseOf(a), baseOf(b)));
        closeChannels();
        for (File log : logs) {
            long base = baseOf(log);
            long count = countRecords(log);
            if (base + count <= offset) {
                continue; // whole segment survives
            }
            if (base >= offset) {
                log.delete();
                new File(dir, base + ".index").delete();
                continue;
            }
            // Segment is cut in the middle: find the byte position of offset.
            long position = positionOf(log, base, offset);
            try (RandomAccessFile raf = new RandomAccessFile(log, "rw")) {
                raf.setLength(position);
            }
            File idx = new File(dir, base + ".index");
            try (RandomAccessFile raf = new RandomAccessFile(idx, "rw")) {
                raf.setLength((offset - base) * 16L);
            }
        }
        // Reopen the segment that now holds the tail.
        File[] remaining = dir.listFiles((d, name) -> name.endsWith(".log"));
        if (remaining == null || remaining.length == 0) {
            openSegment(offset);
            nextOffset = offset;
        } else {
            java.util.Arrays.sort(remaining, (a, b) -> Long.compare(baseOf(a), baseOf(b)));
            File last = remaining[remaining.length - 1];
            openSegment(baseOf(last));
            logChannel.position(logChannel.size());
            indexChannel.position(indexChannel.size());
            currentSegmentSize = logChannel.size();
            nextOffset = offset;
        }
    }

    private static long positionOf(File log, long base, long targetOffset) throws IOException {
        try (RandomAccessFile raf = new RandomAccessFile(log, "r")) {
            long offset = base;
            while (raf.getFilePointer() < raf.length() && offset < targetOffset) {
                int len = raf.readInt();
                raf.skipBytes(len);
                offset++;
            }
            return raf.getFilePointer();
        }
    }

    /** Reads records from {@code offset}, up to {@code maxBytes} of payload. */
    public List<Protocol.Record> fetch(long offset, int maxBytes) throws IOException {
        lock.readLock().lock();
        try {
            List<Protocol.Record> result = new ArrayList<>();
            if (offset >= nextOffset) {
                return result;
            }
            int total = 0;
            File[] logs = dir.listFiles((d, name) -> name.endsWith(".log"));
            if (logs == null) {
                return result;
            }
            java.util.Arrays.sort(logs, (a, b) -> Long.compare(baseOf(a), baseOf(b)));
            for (File log : logs) {
                long base = baseOf(log);
                long count = countRecords(log);
                if (base + count <= offset) {
                    continue;
                }
                try (RandomAccessFile raf = new RandomAccessFile(log, "r")) {
                    long current = base;
                    while (raf.getFilePointer() < raf.length()) {
                        int len = raf.readInt();
                        byte[] message = new byte[len];
                        raf.readFully(message);
                        if (current >= offset) {
                            if (total + len > maxBytes && !result.isEmpty()) {
                                return result;
                            }
                            result.add(new Protocol.Record(current, message));
                            total += len;
                        }
                        current++;
                    }
                }
            }
            return result;
        } finally {
            lock.readLock().unlock();
        }
    }

    public String getTopic() {
        return topic;
    }

    public int getPartitionId() {
        return partitionId;
    }

    public void close() throws IOException {
        lock.writeLock().lock();
        try {
            closeChannels();
        } finally {
            lock.writeLock().unlock();
        }
    }
}
