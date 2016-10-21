package org.quietmodem.Quiet;

public class LoopbackFrameReceiver extends BaseFrameReceiver {
    @Override
    protected void initSystem() throws ModemException {
        this.quietSystem.initLoopback();
    }

    @Override
    protected boolean isLoopback() {
        return true;
    }

    public LoopbackFrameReceiver(FrameReceiverConfig conf) throws ModemException {
        super(conf);
    }
}
