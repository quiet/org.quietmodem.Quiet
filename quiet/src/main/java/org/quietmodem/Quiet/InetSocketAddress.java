package org.quietmodem.Quiet;

import java.net.SocketAddress;
import java.net.UnknownHostException;

public class InetSocketAddress extends SocketAddress {
    private InetAddress addr;
    private int port;
    private boolean is_resolved;

    public InetSocketAddress(InetAddress addr, int port) {
        if (addr == null) {
            try {
                addr = InetAddress.getByAddress(new byte[]{0, 0, 0, 0});
            } catch (UnknownHostException e) {
                addr = InetAddress.getLoopbackAddress();
            }
        }
        this.is_resolved = true;
        this.addr = addr;
        if (port >= 0 && port <= 65535) {
            this.port = port;
        } else {
            throw new IllegalArgumentException("port out of range");
        }
    }

    public InetSocketAddress(int port) {
        InetAddress addr;
        try {
            addr = InetAddress.getByAddress(new byte[]{0, 0, 0, 0});
        } catch (UnknownHostException e) {
            addr = InetAddress.getLoopbackAddress();
        }
        this.addr = addr;
        this.is_resolved = true;
        if (port >= 0 && port <= 65535) {
            this.port = port;
        } else {
            throw new IllegalArgumentException("port out of range");
        }
    }

    public InetSocketAddress(String hostname, int port) {
        if (hostname == null) {
            throw new IllegalArgumentException("hostname must not be null");
        }
        try {
            this.addr = InetAddress.getByName(hostname);
            this.is_resolved = true;
        } catch (UnknownHostException e) {
            this.addr = InetAddress.getLoopbackAddress();
            this.is_resolved = false;
        }
        if (port >= 0 && port <= 65535) {
            this.port = port;
        } else {
            throw new IllegalArgumentException("port out of range");
        }

    }

    public static InetSocketAddress createUnresolved(String host, int port) {
        return new InetSocketAddress(host, port);
    }

    public InetAddress getAddress() {
        return this.addr;
    }

    public String getHostName() {
        return this.addr.getHostName();
    }

    public String getHostString() {
        return this.addr.getHostAddress();
    }

    public int getPort() {
        return this.port;
    }

    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }
        if (!(obj instanceof InetSocketAddress)) {
            return false;
        }
        InetSocketAddress i = (InetSocketAddress)obj;
        if (!this.getAddress().equals(i.getAddress())) {
            return false;
        }
        return this.getPort() == i.getPort();
    }

    public int hashCode() {
        int hc = this.getAddress().hashCode();
        return hc + this.getPort();
    }

    public boolean isUnresolved() {
        return false;
    }

    public String toString() {
        return "InetSocketAddress[host=" + getHostName() + ",address=" + getHostString() + ",port=" + getPort() + "]";
    }

    private InetSocketAddress(byte[] addrBytes, int port) {
        try {
            this.addr = InetAddress.getByAddress(addrBytes);
        } catch (UnknownHostException e) {
        }
        if (port >= 0 && port <= 65535) {
            this.port = port;
        } else {
            throw new IllegalArgumentException("port out of range");
        }
    }

}
