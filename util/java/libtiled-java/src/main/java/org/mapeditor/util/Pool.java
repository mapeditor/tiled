package org.mapeditor.util;

import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

public abstract class Pool<T> {
    private final Queue<T> objectQueue = new ConcurrentLinkedQueue<>();

    public final T take() {
        T t = objectQueue.poll();
        return t != null ? t : create();
    }

    public final void recycle(T t) {
        objectQueue.offer(t);
    }

    protected abstract T create();
}
