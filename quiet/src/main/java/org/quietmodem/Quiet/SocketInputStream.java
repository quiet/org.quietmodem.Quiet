package org.quietmodem.Quiet;

import java.io.IOException;
import java.io.InputStream;

public class SocketInputStream extends InputStream {
    private native int nativeAvailable() throws IOException;
    private native void nativeClose() throws IOException;
    private native int nativeRead(byte[] buf, int off, int len) throws IOException;

    private int fd;

    SocketInputStream(int fd) {
        this.fd = fd;
    }

    @Override
    public int available() throws IOException {
        return nativeAvailable();
    }

    @Override
    public void close() throws IOException {
        nativeClose();
    }

    @Override
    public boolean markSupported() {
        return false;
    }

    @Override
    public int read() throws IOException {
        byte buf[] = new byte[1];
        nativeRead(buf, 0, 1);
        return (int)buf[0];
    }

    @Override
    public int read(byte[] b) throws IOException {
        return nativeRead(b, 0, b.length);
    }

    @Override
    public int read(byte[] b, int off, int len) throws IOException {
        return nativeRead(b, 0, len);
    }

    static {
        QuietInit.init();
        QuietInit.lwipInit();
    }
}
