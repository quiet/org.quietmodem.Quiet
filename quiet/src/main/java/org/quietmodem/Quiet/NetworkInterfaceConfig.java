package org.quietmodem.Quiet;

import java.util.Random;

public class NetworkInterfaceConfig {
    FrameReceiverConfig receiverConfig;
    FrameTransmitterConfig transmitterConfig;;
    InetAddress localAddress;
    InetAddress netmask;
    InetAddress gateway;
    byte[] hardwareAddress;

    public NetworkInterfaceConfig(FrameReceiverConfig receiverConfig,
                                  FrameTransmitterConfig transmitterConfig) {
        this.receiverConfig = receiverConfig;
        this.transmitterConfig = transmitterConfig;
        this.localAddress = InetAddress.getEmptyAddress();
        this.netmask = InetAddress.getEmptyAddress();
        this.gateway = InetAddress.getEmptyAddress();
        this.hardwareAddress = getRandomHardwareAddress();
    }

    public NetworkInterfaceConfig(FrameReceiverConfig receiverConfig,
                                  FrameTransmitterConfig transmitterConfig,
                                  byte[] hardwareAddress) {
        this.receiverConfig = receiverConfig;
        this.transmitterConfig = transmitterConfig;
        this.localAddress = InetAddress.getEmptyAddress();
        this.netmask = InetAddress.getEmptyAddress();
        this.gateway = InetAddress.getEmptyAddress();
        this.hardwareAddress = hardwareAddress;
    }

    public NetworkInterfaceConfig(FrameReceiverConfig receiverConfig,
                                  FrameTransmitterConfig transmitterConfig,
                                  InetAddress localAddress,
                                  InetAddress netmask,
                                  InetAddress gateway) {
        this.receiverConfig = receiverConfig;
        this.transmitterConfig = transmitterConfig;
        this.localAddress = localAddress;
        this.netmask = netmask;
        this.gateway = gateway;
        this.hardwareAddress = getRandomHardwareAddress();
    }

    public NetworkInterfaceConfig(FrameReceiverConfig receiverConfig,
                                  FrameTransmitterConfig transmitterConfig,
                                  InetAddress localAddress,
                                  InetAddress netmask,
                                  InetAddress gateway,
                                  byte[] hardwareAddress) {
        this.receiverConfig = receiverConfig;
        this.transmitterConfig = transmitterConfig;
        this.localAddress = localAddress;
        this.netmask = netmask;
        this.gateway = gateway;
        this.hardwareAddress = hardwareAddress;
    }

    public FrameReceiverConfig getReceiverConfig() { return receiverConfig; }

    public FrameTransmitterConfig getTransmitterConfig() { return transmitterConfig; }

    public InetAddress getLocalAddress() { return localAddress; }

    public InetAddress getNetmask() { return netmask; }

    public InetAddress getGateway() { return gateway; }

    public byte[] getHardwareAddress() { return hardwareAddress; }

    public void setReceiverConfig(FrameReceiverConfig conf) { this.receiverConfig = conf; }

    public void setTransmitterConfig(FrameTransmitterConfig conf) { this.transmitterConfig = conf; }

    public void setLocalAddress(InetAddress localAddress) {
        this.localAddress = localAddress;
    }

    public void setNetmask(InetAddress netmask) {
        this.netmask = netmask;
    }

    public void setGateway(InetAddress gateway) {
        this.gateway = gateway;
    }

    public void setHardwareAddress(byte[] hardwareAddress) {
        if (hardwareAddress.length != 6) {
            throw new IllegalArgumentException("Hardware Address must be 6 bytes");
        }
        this.hardwareAddress = hardwareAddress;
    }

    private static final Random rand = new Random();
    public static byte[] getRandomHardwareAddress() {
        byte[] mac = new byte[6];
        // we choose an address with a locally administered address
        // there are actually quite a few more choices but we probably
        // don't really need to use the entire range
        mac[0] = 2;
        // the top nibble of this address can still be random
        mac[0] |= (byte)((rand.nextInt(16)) << 4);
        for (int i = 1; i < 6; i++) {
            mac[i] = (byte) rand.nextInt(256);
        }
        return mac;
    }
}
