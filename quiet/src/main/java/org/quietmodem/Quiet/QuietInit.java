package org.quietmodem.Quiet;

public class QuietInit {
    private static native void nativeLWIPInit();

    private static boolean has_init = false;
    private static boolean has_lwip_init = false;

    static void init() {
        if (has_init) {
            return;
        }
        has_init = true;
        System.loadLibrary("complex");
        System.loadLibrary("fec");
        System.loadLibrary("jansson");
        System.loadLibrary("liquid");
        System.loadLibrary("quiet");
        System.loadLibrary("quiet_lwip");
        System.loadLibrary("quiet-jni");
    }

    static void lwipInit() {
        if (has_lwip_init) {
            return;
        }
        has_lwip_init = true;
        nativeLWIPInit();
    }
}
