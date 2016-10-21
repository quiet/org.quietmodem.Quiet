package org.quietmodem.Quiet;

public class FrameReceiver extends BaseFrameReceiver {
    @Override
    protected void initSystem() throws ModemException {
        this.quietSystem.initOpenSL();
    }

    @Override
    protected boolean isLoopback() {
        return false;
    }

    public FrameReceiver(FrameReceiverConfig conf) throws ModemException {
        super(conf);
    }
}
