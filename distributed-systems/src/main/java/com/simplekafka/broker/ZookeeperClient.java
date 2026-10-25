package com.simplekafka.broker;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import org.apache.zookeeper.CreateMode;
import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.WatchedEvent;
import org.apache.zookeeper.Watcher;
import org.apache.zookeeper.ZooDefs;
import org.apache.zookeeper.ZooKeeper;
import org.apache.zookeeper.data.Stat;

/**
 * Thin ZooKeeper wrapper. Watches are one-shot in ZooKeeper, so callers
 * re-register after every event (see SimpleKafkaBroker).
 */
public class ZookeeperClient implements AutoCloseable {
    private final ZooKeeper zk;

    public ZookeeperClient(String host, int port, Watcher watcher)
            throws IOException, InterruptedException {
        CountDownLatch connected = new CountDownLatch(1);
        Watcher combined = event -> {
            if (event.getState() == Watcher.Event.KeeperState.SyncConnected) {
                connected.countDown();
            }
            if (watcher != null) {
                watcher.process(event);
            }
        };
        this.zk = new ZooKeeper(host + ":" + port, 10000, combined);
        if (!connected.await(10, TimeUnit.SECONDS)) {
            throw new IOException("Timed out connecting to ZooKeeper at " + host + ":" + port);
        }
    }

    public void ensurePath(String path) throws KeeperException, InterruptedException {
        if ("/".equals(path)) {
            return;
        }
        String[] parts = path.split("/");
        StringBuilder current = new StringBuilder();
        for (String part : parts) {
            if (part.isEmpty()) {
                continue;
            }
            current.append('/').append(part);
            String p = current.toString();
            if (zk.exists(p, false) == null) {
                try {
                    zk.create(p, new byte[0], ZooDefs.Ids.OPEN_ACL_UNSAFE,
                            CreateMode.PERSISTENT);
                } catch (KeeperException.NodeExistsException ignored) {
                    // Created concurrently; fine.
                }
            }
        }
    }

    public boolean createEphemeral(String path, String data)
            throws KeeperException, InterruptedException {
        try {
            zk.create(path, data.getBytes(), ZooDefs.Ids.OPEN_ACL_UNSAFE,
                    CreateMode.EPHEMERAL);
            return true;
        } catch (KeeperException.NodeExistsException e) {
            return false;
        }
    }

    public boolean createPersistent(String path, String data)
            throws KeeperException, InterruptedException {
        try {
            zk.create(path, data.getBytes(), ZooDefs.Ids.OPEN_ACL_UNSAFE,
                    CreateMode.PERSISTENT);
            return true;
        } catch (KeeperException.NodeExistsException e) {
            return false;
        }
    }

    public void setData(String path, String data) throws KeeperException, InterruptedException {
        zk.setData(path, data.getBytes(), -1);
    }

    public String getData(String path) throws KeeperException, InterruptedException {
        Stat stat = zk.exists(path, false);
        if (stat == null) {
            return null;
        }
        return new String(zk.getData(path, false, stat));
    }

    public List<String> getChildren(String path, Watcher watcher)
            throws KeeperException, InterruptedException {
        return zk.getChildren(path, watcher);
    }

    public boolean exists(String path, Watcher watcher)
            throws KeeperException, InterruptedException {
        return zk.exists(path, watcher) != null;
    }

    public void delete(String path) throws KeeperException, InterruptedException {
        try {
            zk.delete(path, -1);
        } catch (KeeperException.NoNodeException ignored) {
            // Already gone; fine.
        }
    }

    @Override
    public void close() throws InterruptedException {
        zk.close();
    }
}
