package org.quietmodem.Quiet;

import android.content.Context;
import android.media.AudioManager;
import android.os.Build;

import java.io.IOException;
import java.io.InputStream;

public class FrameTransmitterConfig {
    private native long nativeOpen(String profiles, String key);
    private native void nativeFree();

    private final long defaultNumBuffers = 3;
    private final long defaultBufferLength = 4096;
    private final int defaultSampleRate = 44100;

    long profile_ptr;
    long numBuffers;
    long bufferLength;
    int sampleRate;
    public FrameTransmitterConfig(android.content.Context c, String key) throws IOException {
        profile_ptr = nativeOpen(getDefaultProfiles(c), key);
        numBuffers = defaultNumBuffers;
        bufferLength = defaultBufferLength;
        sampleRate = defaultSampleRate;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            AudioManager m = (AudioManager) c.getSystemService(Context.AUDIO_SERVICE);
            String pRate = m.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            sampleRate = Integer.parseInt(pRate);
        }
    }

    public FrameTransmitterConfig(String profiles, String key) {
        profile_ptr = nativeOpen(profiles, key);
        numBuffers = defaultNumBuffers;
        bufferLength = defaultBufferLength;
        sampleRate = defaultSampleRate;
    }

    public static String getDefaultProfiles(android.content.Context c) throws IOException {
        InputStream s = c.getResources().openRawResource(R.raw.quiet_profiles);
        byte[] profile_bytes = new byte[s.available()];
        s.read(profile_bytes);
        s.close();
        return new String(profile_bytes);
    }

    public void setNumBuffers(long numBuffers) {
        this.numBuffers = numBuffers;
    }

    public void setBufferLength(long bufferLength) {
        this.bufferLength = bufferLength;
    }

    public void setSampleRate(int sampleRate) { this.sampleRate = sampleRate; }

    public long getNumBuffers() {
        return numBuffers;
    }

    public long getBufferLength() {
        return bufferLength;
    }

    public int getSampleRate() { return sampleRate; }

    @Override
    protected void finalize() throws Throwable {
        nativeFree();
    }

    static {
        QuietInit.init();
    }
}
