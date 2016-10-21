package org.quietmodem.Quiet;

import java.net.SocketAddress;
import java.net.UnknownHostException;

public class DatagramPacket {
    private byte buf[];
    private int offset;
    private int length;
    private InetSocketAddress addr;

    public DatagramPacket(byte[] buf, int length) {
        this.buf = buf;
        this.length = length;
        this.offset = 0;
    }

    public DatagramPacket(byte[] buf, int length, InetAddress address, int port) {
        this.buf = buf;
        this.length = length;
        this.offset = 0;
        this.addr = new InetSocketAddress(address, port);
    }

    public DatagramPacket(byte[] buf, int offset, int length) {
        this.buf = buf;
        this.offset = offset;
        this.length = length;
    }

    public DatagramPacket(byte[] buf, int offset, int length, InetAddress address, int port) {
        this.buf = buf;
        this.offset = offset;
        this.length = length;
        this.addr = new InetSocketAddress(address, port);
    }

    public DatagramPacket(byte[]buf, int offset, int length, SocketAddress address) {
        this.buf = buf;
        this.offset = offset;
        this.length = length;
        this.addr = (InetSocketAddress)address;
    }

    public DatagramPacket(byte[] buf, int length, SocketAddress address) {
        this.buf = buf;
        this.length = length;
        this.offset = 0;
        this.addr = (InetSocketAddress)address;
    }

    public InetAddress getAddress() {
        InetSocketAddress s = this.addr;
        return s.getAddress();
    }

    public byte[] getData() {
        return this.buf;
    }

    public int getLength() {
        return this.length;
    }

    public int getOffset() {
        return this.offset;
    }

    public int getPort() {
        InetSocketAddress s = this.addr;
        return s.getPort();
    }

    public SocketAddress getSocketAddress() {
        return this.addr;
    }

    public void setAddress(InetAddress iaddr) {
        InetSocketAddress s = this.addr;
        this.addr = new InetSocketAddress(iaddr, s.getPort());
    }

    public void setData(byte[] buf) {
        this.buf = buf;
    }

    public void setData(byte[] buf, int offset, int length) {
        this.buf = buf;
        this.offset = offset;
        this.length = length;
    }

    public void setLength(int length) {
        this.length = length;
    }

    public void setPort(int port) {
        InetSocketAddress s = this.addr;
        this.addr = new InetSocketAddress(s.getAddress(), port);
    }

    public void setSocketAddress(SocketAddress address) {
        this.addr = (InetSocketAddress)address;
    }

    private void setSocketAddress(byte addr[], int port) {
        try {
            InetAddress inetAddress = InetAddress.getByAddress(addr);
            this.addr = new InetSocketAddress(inetAddress, port);
        } catch (UnknownHostException e) {
            return;
        }
    }
}
