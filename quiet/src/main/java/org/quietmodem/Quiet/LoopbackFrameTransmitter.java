package org.quietmodem.Quiet;

public class LoopbackFrameTransmitter extends BaseFrameTransmitter {
    @Override
    protected void initSystem() throws ModemException {
        this.quietSystem.initLoopback();
    }

    @Override
    protected boolean isLoopback() {
        return true;
    }

    public LoopbackFrameTransmitter(FrameTransmitterConfig conf) throws ModemException {
        super(conf);
    }
}
