package org.quietmodem.Quiet;

import java.net.UnknownHostException;
import java.util.Arrays;

public class InetAddress {
    private static native String byte2str(byte[] addr) throws UnknownHostException;
    private static native byte[] str2byte(String addr) throws UnknownHostException;

    private byte[] address;
    private String address_string;
    private String hostname;
    private String fqdn;

    public byte[] getAddress() {
        return Arrays.copyOf(address, address.length);
    }

    public String getCanonicalHostName() {
        return fqdn;
    }

    public String getHostAddress() {
        return address_string;
    }

    public String getHostName() {
        return hostname;
    }

    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }
        if (!(obj instanceof InetAddress)) {
            return false;
        }
        InetAddress i = (InetAddress)obj;
        byte[] otherAddr = i.getAddress();
        if (otherAddr.length != this.address.length) {
            return false;
        }
        return Arrays.equals(otherAddr, this.address);
    }

    public int hashCode() {
        int a, b, c, d;
        a = this.address[0];
        b = this.address[1];
        c = this.address[2];
        d = this.address[3];
        return (a << 24) | (b << 16) | (c << 8) | d;
    }

    public String toString() {
        return "InetAddress[hostname=" + hostname + ",address=" + address_string + "]";
    }

    private InetAddress(byte[] address, String address_string, String hostname) {
        this.address = address;
        this.address_string = address_string;
        this.hostname = hostname;
        this.fqdn = hostname;
    }

    public static InetAddress getByAddress(String host, byte[] addr) throws UnknownHostException {
        throw new UnknownHostException();
    }

    public static InetAddress getByName(String host) throws UnknownHostException {
        if (host == null) {
            return InetAddress.getLoopbackAddress();
        }
        byte addr[] = str2byte(host);
        String address_string = byte2str(addr);
        return new InetAddress(addr, address_string, host);
    }

    public static InetAddress[] getAllByName(String host) throws UnknownHostException {
        throw new UnknownHostException();
    }

    public static InetAddress getLoopbackAddress() {
        String loopback = "127.0.0.1";
        byte loopback_byte[] = new byte[]{127, 0, 0, 1};
        return new InetAddress(loopback_byte, loopback, loopback);
    }

    static InetAddress getEmptyAddress() {
        String empty = "0.0.0.0";
        byte empty_byte[] = new byte[]{0, 0, 0, 0};
        return new InetAddress(empty_byte, empty, empty);
    }

    public static InetAddress getByAddress(byte[] addr) throws UnknownHostException {
        if (addr.length != 4) {
            throw new UnknownHostException();
        }
        String address_string = byte2str(addr);
        return new InetAddress(addr, address_string, address_string);
    }

    public static InetAddress getLocalHost() throws UnknownHostException {
        throw new UnknownHostException();
    }

    static {
        QuietInit.init();
        QuietInit.lwipInit();
    }
}
