package org.quietmodem.Quiet;

import java.util.Enumeration;
import java.util.Vector;

public abstract class BaseNetworkInterface {
    protected abstract void initSystem() throws ModemException;
    protected abstract boolean isLoopback();

    private native long nativeOpen(long sys_ptr, NetworkInterfaceConfig conf, boolean is_loopback) throws ModemException;
    private native void nativeClose();
    private native void nativeTerminate(int urgency);
    private native void nativeFree();

    protected QuietSystem quietSystem;
    private long interface_ptr;

    private static Vector<BaseNetworkInterface> interfaces = new Vector<BaseNetworkInterface>();

    public BaseNetworkInterface(NetworkInterfaceConfig conf) throws ModemException {
        this.quietSystem = QuietSystem.getSystem();
        initSystem();
        this.interface_ptr = nativeOpen(quietSystem.sys_ptr, conf, isLoopback());
        addInterface(this);
    }

    private static synchronized void addInterface(BaseNetworkInterface i) {
        interfaces.add(i);
    }

    private static synchronized void removeInterface(BaseNetworkInterface i) {
        interfaces.remove(i);
    }

    public static synchronized Enumeration<BaseNetworkInterface> getNetworkInterfaces() {
        return interfaces.elements();
    }

    public void close() {
        removeInterface(this);
        this.nativeClose();
    }

    public void terminate(int urgency) {
        this.nativeTerminate(urgency);
    }

    @Override
    protected void finalize() {
        // nativeFree();
    }

    static {
        QuietInit.init();
        QuietInit.lwipInit();
    }
}
