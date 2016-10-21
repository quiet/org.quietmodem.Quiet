package org.quietmodem.Quiet;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Enumeration;

public class Socket {
    private native int nativeCreate() throws SocketException;
    private native void nativeBind(InetSocketAddress inetSocketAddress) throws IOException;
    private native void nativeClose() throws IOException;
    private native void nativeConnect(InetSocketAddress inetSocketAddress) throws IOException;
    private native InetSocketAddress nativeGetLocal() throws IOException;
    private native InetSocketAddress nativeGetRemote() throws IOException;
    private native boolean nativeGetKeepAlive() throws SocketException;
    private native boolean nativeGetOOBInline() throws SocketException;
    private native int nativeGetReceiveBufferSize() throws SocketException;
    private native boolean nativeGetReuseAddress() throws SocketException;
    private native int nativeGetSendBufferSize() throws SocketException;
    private native int nativeGetSoLinger() throws SocketException;
    private native int nativeGetSoTimeout() throws SocketException;
    private native boolean nativeGetTcpNoDelay() throws SocketException;
    private native void nativeSetKeepAlive(boolean keepalive) throws SocketException;
    private native void nativeSetOOBInline(boolean oobinline) throws SocketException;
    private native void nativeSetReceiveBufferSize(int size) throws SocketException;
    private native void nativeSetReuseAddress(boolean reuse) throws SocketException;
    private native void nativeSendBufferSize(int size) throws SocketException;
    private native void nativeSetSoLinger(int linger) throws SocketException;
    private native void nativeSetSoTimeout(int timeout) throws SocketException;
    private native void nativeSetTcpNoDelay(boolean nodelay) throws SocketException;

    private int fd;
    private SocketInputStream inputStream;
    private SocketOutputStream outputStream;

    private void init(int fd) {
        this.fd = fd;
        this.inputStream = new SocketInputStream(fd);
        this.outputStream = new SocketOutputStream(fd);
    }

    /* this constructor is for use by c bindings */
    Socket(int fd) {
        init(fd);
    }

    public Socket() throws IOException {
        Enumeration<BaseNetworkInterface> i = BaseNetworkInterface.getNetworkInterfaces();
        if (!i.hasMoreElements()) {
            throw new IOException("no NetworkInterfaces present");
        }
        init(nativeCreate());
    }

    public Socket(InetAddress address, int port) throws IOException {
        Enumeration<BaseNetworkInterface> i = BaseNetworkInterface.getNetworkInterfaces();
        if (!i.hasMoreElements()) {
            throw new IOException("no NetworkInterfaces present");
        }
        init(nativeCreate());
        bind(new InetSocketAddress(address, port));
    }

    public Socket(InetAddress remoteAddress, int remotePort, InetAddress localAddress, int localPort) throws IOException {
        Enumeration<BaseNetworkInterface> i = BaseNetworkInterface.getNetworkInterfaces();
        if (!i.hasMoreElements()) {
            throw new IOException("no NetworkInterfaces present");
        }
        init(nativeCreate());
        bind(new InetSocketAddress(localAddress, localPort));
        connect(new InetSocketAddress(remoteAddress, remotePort));
    }

    public Socket(String host, int port) throws UnknownHostException, IOException {
        Enumeration<BaseNetworkInterface> i = BaseNetworkInterface.getNetworkInterfaces();
        if (!i.hasMoreElements()) {
            throw new IOException("no NetworkInterfaces present");
        }
        InetAddress inetAddress = InetAddress.getByName(host);
        init(nativeCreate());
        connect(new InetSocketAddress(inetAddress, port));
    }

    public Socket(String remoteHost, int remotePort, InetAddress localAddress, int localPort) throws UnknownHostException, IOException {
        Enumeration<BaseNetworkInterface> i = BaseNetworkInterface.getNetworkInterfaces();
        if (!i.hasMoreElements()) {
            throw new IOException("no NetworkInterfaces present");
        }
        InetAddress inetAddress = InetAddress.getByName(remoteHost);
        init(nativeCreate());
        bind(new InetSocketAddress(localAddress, localPort));
        connect(new InetSocketAddress(inetAddress, remotePort));
    }

    public void bind(SocketAddress bindpoint) throws IOException {
        nativeBind((InetSocketAddress) bindpoint);
    }

    public void close() throws IOException {
        nativeClose();
    }

    public void connect(SocketAddress endpoint) throws IOException {
        nativeConnect((InetSocketAddress) endpoint);
    }

    public InputStream getInputStream() {
        return this.inputStream;
    }

    public OutputStream getOutputStream() {
        return this.outputStream;
    }

    public void shutdownInput() throws IOException {
        this.inputStream.close();
    }

    public void shutdownOutput() throws IOException {
        this.outputStream.close();
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

    public boolean isConnected() {
        try {
            InetSocketAddress addr = nativeGetRemote();
            return addr.getPort() != 0;
        } catch (IOException e) {
            return false;
        }
    }

    public InetSocketAddress getRemoteSocketAddress() {
        try {
            return nativeGetRemote();
        } catch (IOException e) {
            return null;
        }
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
        InetSocketAddress addr = getRemoteSocketAddress();
        if (addr == null) {
            return null;
        }
        return addr.getAddress();
    }

    public InetAddress getLocalAddress() {
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

    public int getPort() {
        InetSocketAddress addr = getRemoteSocketAddress();
        if (addr == null) {
            return -1;
        }
        return addr.getPort();
    }

    public boolean getKeepAlive() throws SocketException { return nativeGetKeepAlive(); }

    public boolean getOOBInline() throws SocketException { return nativeGetOOBInline(); }

    public int getReceiveBufferSize() throws SocketException { return nativeGetReceiveBufferSize(); }

    public boolean getReuseAddress() throws SocketException { return nativeGetReuseAddress(); }

    public int getSendBufferSize() throws SocketException { return nativeGetSendBufferSize(); }

    public int getSoLinger() throws SocketException { return nativeGetSoLinger(); }

    public int getSoTimeout() throws SocketException { return nativeGetSoTimeout(); }

    public boolean getTcpNoDelay() throws SocketException { return nativeGetTcpNoDelay(); }

    public void setKeepAlive(boolean keepAlive) throws SocketException { nativeSetKeepAlive(keepAlive); }

    public void setOOBInline(boolean oobInline) throws SocketException { nativeSetOOBInline(oobInline); }

    public void setReceiveBufferSize(int size) throws SocketException { nativeSetReceiveBufferSize(size); }

    public void setReuseAddress(boolean reuse) throws SocketException { nativeSetReuseAddress(reuse); }

    public void setSendBufferSize(int size) throws SocketException { nativeSetReceiveBufferSize(size); }

    public void setSoLinger(int linger) throws SocketException { nativeSetSoLinger(linger); }

    public void setSoTimeout(int timeout) throws SocketException { nativeSetSoTimeout(timeout); }

    public void SetTcpNoDelay(boolean nodelay) throws SocketException { nativeSetTcpNoDelay(nodelay); }

    static {
        QuietInit.init();
        QuietInit.lwipInit();
    }
}
