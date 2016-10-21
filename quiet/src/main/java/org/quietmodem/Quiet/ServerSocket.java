package org.quietmodem.Quiet;

import java.io.IOException;
import java.net.SocketAddress;
import java.util.Enumeration;

public class ServerSocket {
    private int fd;

    private native int nativeCreate();
    private native void nativeBind(InetSocketAddress inetSocketAddress, int backlog) throws IOException;
    private native void nativeClose();
    private native int nativeAccept() throws IOException;
    private native InetSocketAddress nativeGetLocal() throws IOException;
    private native int nativeGetReceiveBufferSize();
    private native boolean nativeGetReuseAddress();
    private native int nativeGetSoTimeout();
    private native void nativeSetReceiveBufferSize(int size);
    private native void nativeSetReuseAddress(boolean reuse);
    private native void nativeSetSoTimeout(int timeout);

    private final int default_backlog = 1;

    public ServerSocket() throws IOException {
        Enumeration<BaseNetworkInterface> i = BaseNetworkInterface.getNetworkInterfaces();
        if (!i.hasMoreElements()) {
            throw new IOException("no NetworkInterfaces present");
        }
        this.fd = nativeCreate();
    }

    public ServerSocket(int port) throws IOException {
        Enumeration<BaseNetworkInterface> i = BaseNetworkInterface.getNetworkInterfaces();
        if (!i.hasMoreElements()) {
            throw new IOException("no NetworkInterfaces present");
        }
        this.fd = nativeCreate();
        InetSocketAddress inetSocketAddress = new InetSocketAddress(port);
        nativeBind(inetSocketAddress, default_backlog);
    }

    public ServerSocket(int port, int backlog) throws IOException{
        Enumeration<BaseNetworkInterface> i = BaseNetworkInterface.getNetworkInterfaces();
        if (!i.hasMoreElements()) {
            throw new IOException("no NetworkInterfaces present");
        }
        this.fd = nativeCreate();
        InetSocketAddress inetSocketAddress = new InetSocketAddress(port);
        nativeBind(inetSocketAddress, backlog);
    }

    public ServerSocket(int port, int backlog, InetAddress bindAddr) throws IOException {
        Enumeration<BaseNetworkInterface> i = BaseNetworkInterface.getNetworkInterfaces();
        if (!i.hasMoreElements()) {
            throw new IOException("no NetworkInterfaces present");
        }
        this.fd = nativeCreate();
        InetSocketAddress inetSocketAddress = new InetSocketAddress(bindAddr, port);
        nativeBind(inetSocketAddress, backlog);
    }

    public Socket accept() throws IOException {
        return new Socket(nativeAccept());
    }

    public void bind(SocketAddress endpoint) throws IOException {
        nativeBind((InetSocketAddress)endpoint, default_backlog);
    }

    public void bind(SocketAddress endpoint, int backlog) throws IOException {
        nativeBind((InetSocketAddress)endpoint, backlog);
    }

    public void close() {
        nativeClose();
    }

    public InetSocketAddress getLocalSocketAddress() {
        try {
            InetSocketAddress addr = nativeGetLocal();
            if (addr.getPort() == 0) {
                return null;
            }
            return addr;
        } catch (IOException e) {
            return null;
        }
    }

    public InetAddress getInetAddress() {
        try {
            InetSocketAddress addr = nativeGetLocal();
            return addr.getAddress();
        } catch (IOException e) {
            return null;
        }
    }

    public int getLocalPort() {
        try {
            InetSocketAddress addr = nativeGetLocal();
            return addr.getPort();
        } catch (IOException e) {
            return -1;
        }
    }

    public boolean isBound() {
        try {
            InetSocketAddress addr = nativeGetLocal();
            return addr.getPort() != 0;
        } catch (IOException e) {
            return false;
        }
    }

    public boolean isClosed() {
        try {
            InetSocketAddress addr = nativeGetLocal();
            return false;
        } catch (IOException e) {
            return true;
        }
    }

    public int getReceiveBufferSize() {
        return nativeGetReceiveBufferSize();
    }

    public boolean getReuseAddress() {
        return nativeGetReuseAddress();
    }

    public int getSoTimeout() {
        return nativeGetSoTimeout();
    }

    public void setReceiveBufferSize(int size) {
        nativeSetReceiveBufferSize(size);
    }

    public void setReuseAddress(boolean reuse) {
        nativeSetReuseAddress(reuse);
    }

    public void setSoTimeout(int timeout) {
        nativeSetSoTimeout(timeout);
    }

    static {
        QuietInit.init();
        QuietInit.lwipInit();
    }
}
