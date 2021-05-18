package org.quietmodem.Quiet;

import android.content.Context;
import android.media.AudioDeviceInfo;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.os.Build;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;

public class FrameReceiverConfig {
    private native long nativeOpen(String profiles, String key);
    private native void nativeFree();

    private final long defaultNumBuffers = 3;
    private final long defaultBufferLength = 4096;
    private final int defaultSampleRate = 44100;

    long profile_ptr;
    long numBuffers;
    long bufferLength;
    int sampleRate;

    public FrameReceiverConfig(android.content.Context c, String key) throws IOException {
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

    public FrameReceiverConfig(String profiles, String key) {
        profile_ptr = nativeOpen(profiles, key);
        numBuffers = defaultNumBuffers;
        bufferLength = defaultBufferLength;
        sampleRate = defaultSampleRate;
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

    public static String getDefaultProfiles(android.content.Context c) throws IOException {
        InputStream s = c.getResources().openRawResource(R.raw.quiet_profiles);
        byte[] profile_bytes = new byte[s.available()];
        s.read(profile_bytes);
        s.close();
        return new String(profile_bytes);
    }

    public static void printAudioDeviceInfo(android.content.Context c) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            AudioManager m = (AudioManager) c.getSystemService(Context.AUDIO_SERVICE);
            String pRate = m.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            int sampleRate = Integer.parseInt(pRate);
            Log.w("FrameReceiverConfig", pRate);
            AudioDeviceInfo[] devices = m.getDevices(AudioManager.GET_DEVICES_ALL);
            for (int i = 0; i < devices.length; i++) {
                Log.w("FrameReceiverConfig", String.format("%s %d %d", devices[i].getProductName().toString(), devices[i].getId(), devices[i].getType()));
                int[] rates = devices[i].getSampleRates();
                for (int j = 0; j < rates.length; j++) {
                    Log.w("FrameReceiverConfig", String.format("%d", rates[j]));
                }
                int[] channels = devices[i].getChannelCounts();
                for (int j = 0; j < channels.length; j++) {
                    Log.w("FrameReceiverConfig", String.format("%d", channels[j]));
                }
            }
            Log.w("FrameReceiverConfig", String.format("near ultrasound %s %s", m.getProperty(AudioManager.PROPERTY_SUPPORT_MIC_NEAR_ULTRASOUND), m.getProperty(AudioManager.PROPERTY_SUPPORT_SPEAKER_NEAR_ULTRASOUND)));
        }
    }

    @Override
    protected void finalize() throws Throwable {
        nativeFree();
    }

    static {
        QuietInit.init();
    }
}
