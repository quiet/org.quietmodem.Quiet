package org.quietmodem.Quiet;

public class FrameTransmitter extends BaseFrameTransmitter {
    @Override
    protected void initSystem() throws ModemException {
        this.quietSystem.initOpenSL();
    }

    @Override
    protected boolean isLoopback() {
        return false;
    }

    public FrameTransmitter(FrameTransmitterConfig conf) throws ModemException {
        super(conf);
    }
}
