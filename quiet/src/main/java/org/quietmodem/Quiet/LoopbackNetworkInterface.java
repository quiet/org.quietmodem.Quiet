package org.quietmodem.Quiet;

public class LoopbackNetworkInterface extends NetworkInterface {
    @Override
    protected void initSystem() throws ModemException {
        this.quietSystem.initLoopback();
    }

    @Override
    protected boolean isLoopback() {
        return true;
    }

    public LoopbackNetworkInterface(NetworkInterfaceConfig conf) throws ModemException {
        super(conf);
    }
}
