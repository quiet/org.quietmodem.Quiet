package org.quietmodem.Quiet;

import java.io.IOException;
import java.io.OutputStream;

public class SocketOutputStream extends OutputStream {
    private native void nativeClose() throws IOException;
    private native void nativeWrite(byte[] buf, int off, int len) throws IOException;

    private int fd;

    SocketOutputStream(int fd) {
        this.fd = fd;
    }

    @Override
    public void close() throws IOException {
        nativeClose();
    }

    @Override
    public void flush() throws IOException {
        return;
    }

    @Override
    public void write(byte[] b) throws IOException {
        nativeWrite(b, 0, b.length);
    }

    @Override
    public void write(byte[] b, int off, int len) throws IOException {
        nativeWrite(b, off, len);
    }

    @Override
    public void write(int b) throws IOException {
        byte buf[] = new byte[]{(byte)b};
        nativeWrite(buf, 0, 1);
    }

    static {
        QuietInit.init();
        QuietInit.lwipInit();
    }
}