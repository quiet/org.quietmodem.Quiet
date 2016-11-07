package org.quietmodem.Quiet;

import java.io.IOException;

public abstract class BaseFrameReceiver {
    private native long nativeOpen(long sys_ptr, FrameReceiverConfig conf, boolean is_loopback) throws ModemException;
    private native void nativeClose();
    private native void nativeTerminate(int urgency);
    private native long nativeRecv(byte[] frame, long offset, long length) throws IOException;
    private native void nativeSetBlocking(long seconds, long nano);
    private native void nativeSetNonblocking();
    private native void nativeFree();
    private native void nativeEnableStats();
    private native void nativeDisableStats();
    private native void nativeStatsSetBlocking(long seconds, long nano);
    private native void nativeStatsSetNonblocking();
    private native FrameStats nativeRecvStats() throws IOException;

    protected QuietSystem quietSystem;
    private long dec_ptr;

    protected abstract void initSystem() throws ModemException;
    protected abstract boolean isLoopback();

    public BaseFrameReceiver(FrameReceiverConfig conf) throws ModemException {
        this.quietSystem = QuietSystem.getSystem();
        initSystem();
        this.dec_ptr = nativeOpen(this.quietSystem.sys_ptr, conf, isLoopback());
    }

    public long receive(byte[] frame) throws IOException {
        return this.nativeRecv(frame, 0, frame.length);
    }

    public long receive(byte[] frame, long length) throws IOException {
        return this.nativeRecv(frame, 0, length);
    }

    public long receive(byte[] frame, long offset, long length) throws IOException {
        return this.nativeRecv(frame, offset, length);
    }

    public void setBlocking(long seconds, long nano) {
        this.nativeSetBlocking(seconds, nano);
    }

    public void setNonBlocking() {
        this.nativeSetNonblocking();
    }

    public void close() {
        this.nativeClose();
    }

    public void terminate(int urgency) {
        this.nativeTerminate(urgency);
    }

    public void enableStats() { nativeEnableStats(); }
    public void disableStats() { nativeDisableStats(); }
    public void statsSetBlocking(long seconds, long nano) { nativeStatsSetBlocking(seconds, nano); }
    public void statsSetNonblocking() { nativeStatsSetNonblocking(); }
    public synchronized FrameStats receiveStats() throws IOException { return nativeRecvStats(); }

    @Override
    protected void finalize() {
        /* it's unfortunate to reach for finalize() in general
         * however it seems excusable here because this just frees memory
         * this is *not* the same operation as close()
         */
        nativeFree();
    }

    static {
        QuietInit.init();
    }
}
