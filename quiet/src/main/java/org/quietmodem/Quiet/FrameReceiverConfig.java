package org.quietmodem.Quiet;

import java.io.IOException;
import java.io.InputStream;

public class FrameReceiverConfig {
    private native long nativeOpen(String profiles, String key);
    private native void nativeFree();

    private final long defaultNumBuffers = 3;
    private final long defaultBufferLength = 4096;

    long profile_ptr;
    long numBuffers;
    long bufferLength;
    public FrameReceiverConfig(android.content.Context c, String key) throws IOException {
        profile_ptr = nativeOpen(getDefaultProfiles(c), key);
        numBuffers = defaultNumBuffers;
        bufferLength = defaultBufferLength;
    }

    public FrameReceiverConfig(String profiles, String key) {
        profile_ptr = nativeOpen(profiles, key);
        numBuffers = defaultNumBuffers;
        bufferLength = defaultBufferLength;
    }

    public void setNumBuffers(long numBuffers) {
        this.numBuffers = numBuffers;
    }

    public void setBufferLength(long bufferLength) {
        this.bufferLength = bufferLength;
    }

    public long getNumBuffers() {
        return numBuffers;
    }

    public long getBufferLength() {
        return bufferLength;
    }

    public static String getDefaultProfiles(android.content.Context c) throws IOException {
        InputStream s = c.getResources().openRawResource(R.raw.quiet_profiles);
        byte[] profile_bytes = new byte[s.available()];
        s.read(profile_bytes);
        s.close();
        return new String(profile_bytes);
    }

    @Override
    protected void finalize() throws Throwable {
        nativeFree();
    }

    static {
        QuietInit.init();
    }
}
