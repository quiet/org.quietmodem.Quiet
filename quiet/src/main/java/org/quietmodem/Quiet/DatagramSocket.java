package org.quietmodem.Quiet;

import java.io.IOException;
import java.net.SocketAddress;
import java.net.SocketException;

public class DatagramSocket {
    private native int nativeCreate() throws SocketException;
    private native void nativeBind(InetSocketAddress inetSocketAddress) throws SocketException;
    private native void nativeClose();
    private native void nativeConnect(InetSocketAddress inetSocketAddress) throws SocketException;
    private native void nativeDisconnect();
    private native int nativeReceive(DatagramPacket datagramPacket) throws IOException;
    private native int nativeSend(DatagramPacket datagramPacket) throws IOException;
    private native boolean nativeGetBroadcast() throws SocketException;
    private native int nativeGetReceiveBufferSize() throws SocketException;
    private native boolean nativeGetReuseAddress() throws SocketException;
    private native int nativeGetSendBufferSize() throws SocketException;
    private native int nativeGetSoTimeout() throws SocketException;
    private native int nativeGetTrafficClass() throws SocketException;
    private native void nativeSetBroadcast(boolean broadcast) throws SocketException;
    private native void nativeSetReceiveBufferSize(int size) throws SocketException;
    private native void nativeSetReuseAddress(boolean reuse) throws SocketException;
    private native void nativeSetSendBufferSize(int size) throws SocketException;
    private native void nativeSetSoTimeout(int timeout) throws SocketException;
    private native void nativeSetTrafficClass(int tos) throws SocketException;
    private native InetSocketAddress nativeGetLocal() throws IOException;
    private native InetSocketAddress nativeGetRemote() throws  IOException;


    private int fd;

    public DatagramSocket() throws SocketException {
        InetSocketAddress socketAddress = new InetSocketAddress(0);
        fd = nativeCreate();
        nativeBind(socketAddress);
    }

    public DatagramSocket(int port) throws SocketException {
        InetSocketAddress socketAddress = new InetSocketAddress(port);
        fd = nativeCreate();
        nativeBind(socketAddress);
    }

    public DatagramSocket(int port, InetAddress addr) throws SocketException {
        InetSocketAddress socketAddress = new InetSocketAddress(addr, port);
        fd = nativeCreate();
        nativeBind(socketAddress);
    }

    public DatagramSocket(SocketAddress bindaddr) throws SocketException {
        fd = nativeCreate();
        if (bindaddr != null) {
            nativeBind((InetSocketAddress) bindaddr);
        }
    }

    public void bind(SocketAddress addr) throws SocketException {
        nativeBind((InetSocketAddress) addr);
    }

    public void close() {
        nativeClose();
    }

    public void connect(InetAddress address, int port) {
        InetSocketAddress socketAddress = new InetSocketAddress(address, port);
        try {
            nativeConnect(socketAddress);
        } catch (IOException e) {
            // we have to swallow this error in order to be compatible
            //   with java.net.DatagramSocket
            // TODO decide to break compat? this is obviously bad
        }
    }

    public void connect(SocketAddress address) throws SocketException {
        nativeConnect((InetSocketAddress) address);
    }

    public void disconnect() {
        nativeDisconnect();
    }

    public int send(DatagramPacket datagramPacket) throws IOException {
        return nativeSend(datagramPacket);
    }

    public int receive(DatagramPacket datagramPacket) throws IOException {
        return nativeReceive(datagramPacket);
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

    public boolean getBroadcast() throws SocketException {
        return nativeGetBroadcast();
    }

    public int getReceiveBufferSize() throws SocketException {
        return nativeGetReceiveBufferSize();
    }

    public boolean getReuseAddress() throws SocketException {
        return nativeGetReuseAddress();
    }

    public int getSendBufferSize() throws SocketException {
        return nativeGetSendBufferSize();
    }

    public int getSoTimeout() throws SocketException {
        return nativeGetSoTimeout();
    }

    public int getTrafficClass() throws SocketException {
        return nativeGetTrafficClass();
    }

    public void setBroadcast(boolean broadcast) throws SocketException {
        nativeSetBroadcast(broadcast);
    }

    public void setReceiveBufferSize(int size) throws SocketException {
        nativeSetReceiveBufferSize(size);
    }

    public void setReuseAddress(boolean reuse) throws SocketException {
        nativeSetReuseAddress(reuse);
    }

    public void setSendBufferSize(int size) throws SocketException {
        nativeSetSendBufferSize(size);
    }

    public void setSoTimeout(int timeout) throws SocketException {
        nativeSetSoTimeout(timeout);
    }

    public void setTrafficClass(int tos) throws SocketException {
        nativeSetTrafficClass(tos);
    }

    static {
        QuietInit.init();
        QuietInit.lwipInit();
    }
}
