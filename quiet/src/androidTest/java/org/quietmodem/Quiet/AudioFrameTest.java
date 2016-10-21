package org.quietmodem.Quiet;

import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;
import android.test.suitebuilder.annotation.SmallTest;

import static org.junit.Assert.*;

import org.junit.Test;
import org.junit.runner.RunWith;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Arrays;

@RunWith(AndroidJUnit4.class)
@SmallTest
public class AudioFrameTest {
    @Test
    public void testTransmitAndReceive() {
        FrameReceiverConfig receiverConfig = null;
        FrameTransmitterConfig transmitterConfig = null;

        try {
            /* if you are running a MainActivity
             * then replace InstrumentationRegistry.getTargetContext()
             * with `this`
             */
            transmitterConfig = new FrameTransmitterConfig(
                    InstrumentationRegistry.getTargetContext(),
                    "audible-7k-channel-0");
            receiverConfig = new FrameReceiverConfig(
                    InstrumentationRegistry.getTargetContext(),
                    "audible-7k-channel-0");
        } catch (IOException e) {
            fail("could not build configs");
        }

        FrameReceiver receiver = null;
        FrameTransmitter transmitter = null;
        try {
            receiver = new FrameReceiver(receiverConfig);
            transmitter = new FrameTransmitter(transmitterConfig);
        } catch (ModemException e) {
            fail("could not set up receiver/transmitter");
        }

        String payload = "Hello, World!";
        try {
            transmitter.send(payload.getBytes());
        } catch (IOException e) {
            fail("error sending on transmitter");
        }

        receiver.setBlocking(1, 0);
        byte[] buf = new byte[80];
        long recvLen = 0;
        try {
            recvLen = receiver.receive(buf);
        } catch (IOException e) {
            fail("error receiving from receiver");
        }

        byte[] recvBuf = Arrays.copyOfRange(buf, 0, (int) recvLen);
        String recvStr = "";
        try {
            recvStr = new String(recvBuf, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            fail("could not decode received buffer to utf-8");
        }
        assertEquals(recvStr, payload);

        transmitter.close();
        receiver.close();
    }

}