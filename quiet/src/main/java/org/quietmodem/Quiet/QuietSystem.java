package org.quietmodem.Quiet;

public class QuietSystem {
    private static QuietSystem instance = null;

    private native long nativeOpen() throws ModemException;
    private native void nativeClose();
    private native void nativeOpenOpenSL() throws ModemException;
    private native void nativeOpenLoopback() throws ModemException;

    final long sys_ptr;
    private boolean init_opensl;
    private boolean init_loopback;

    protected QuietSystem() throws ModemException {
        this.sys_ptr = nativeOpen();
        this.init_opensl = false;
        this.init_loopback = false;
    }

    synchronized static QuietSystem getSystem() throws ModemException {
        if (instance == null) {
            instance = new QuietSystem();
        }
        return instance;
    }

    private synchronized static void deleteSystem() {
        instance = null;
    }

    public synchronized void close() {
        nativeClose();
        deleteSystem();
    }

    synchronized void initOpenSL() throws ModemException {
        if (!this.init_opensl) {
            nativeOpenOpenSL();
            this.init_opensl = true;
        }
    }

    synchronized void initLoopback() throws ModemException {
        if (!this.init_loopback) {
            nativeOpenLoopback();
            this.init_loopback = true;
        }
    }

    static {
        QuietInit.init();
    }
}
