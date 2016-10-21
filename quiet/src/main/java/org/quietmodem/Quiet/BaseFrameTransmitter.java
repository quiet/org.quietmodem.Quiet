package org.quietmodem.Quiet;

import java.io.IOException;

public abstract class BaseFrameTransmitter {
    private native long nativeOpen(long sys_ptr, FrameTransmitterConfig conf, boolean is_loopback) throws ModemException;
    private native void nativeClose();
    private native void nativeTerminate(int urgency);
    private native long nativeSend(byte[] frame, long offset, long length) throws IOException;
    private native void nativeSetBlocking(long sec, long nano);
    private native void nativeSetNonblocking();
    private native long nativeGetFrameLength();
    private native void nativeFree();

    protected QuietSystem quietSystem;
    private long enc_ptr;

    protected abstract void initSystem() throws ModemException;
    protected abstract boolean isLoopback();

    public BaseFrameTransmitter(FrameTransmitterConfig conf) throws ModemException {
        this.quietSystem = QuietSystem.getSystem();
        this.initSystem();
        this.enc_ptr = nativeOpen(this.quietSystem.sys_ptr, conf, isLoopback());
    }

    public long send(byte[] frame) throws IOException {
        return this.nativeSend(frame, 0, frame.length);
    }

    public long send(byte[] frame, long length) throws IOException {
        return this.nativeSend(frame, 0, length);
    }

    public long send(byte[] frame, long offset, long length) throws IOException {
        return this.nativeSend(frame, offset, length);
    }

    public void setBlocking(long seconds, long nano) {
        this.nativeSetBlocking(seconds, nano);
    }

    public void setNonBlocking() {
        this.nativeSetNonblocking();
    }

    public long getFrameLength() {
        return this.nativeGetFrameLength();
    }

    public void close() {
        this.nativeClose();
    }

    public void terminate(int urgency) {
        this.nativeTerminate(urgency);
    }

    @Override
    protected void finalize() {
        nativeFree();
    }

    static {
        QuietInit.init();
    }
}
