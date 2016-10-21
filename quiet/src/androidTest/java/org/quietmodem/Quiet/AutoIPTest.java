package org.quietmodem.Quiet;

import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;
import android.test.suitebuilder.annotation.LargeTest;
import android.util.Log;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.*;
import java.io.IOException;
import java.net.SocketException;

@RunWith(AndroidJUnit4.class)
@LargeTest
public class AutoIPTest {
    private static LoopbackNetworkInterface intf;
    @BeforeClass
    public static void setup() {
        FrameReceiverConfig receiverConfig = null;
        FrameTransmitterConfig transmitterConfig = null;

        try {
            transmitterConfig = new FrameTransmitterConfig(InstrumentationRegistry.getTargetContext(),
                    "audible-7k-channel-0");
            receiverConfig = new FrameReceiverConfig(InstrumentationRegistry.getTargetContext(),
                    "audible-7k-channel-0");
        } catch (IOException e) {
            fail("could not build configs");

        }

        NetworkInterfaceConfig conf = new NetworkInterfaceConfig(
                    receiverConfig,
                    transmitterConfig);

        try {
            intf = new LoopbackNetworkInterface(conf);
        } catch (ModemException e) {
            fail("network interface threw exception");
        }

        /* XXX we have to sleep to let the autoip
         * process resolve. we should fix this later
         * a callback on ip settle maybe?
         */
        try {
            Thread.sleep(10000, 0);
        } catch (InterruptedException e) {
        }
    }

    @AfterClass
    public static void teardown() {
        intf.close();
    }

    @Test
    public void testFoo() {
        DatagramSocket s = null;
        try {
            s = new DatagramSocket(null);
        } catch (SocketException e) {
            fail("exception creating DatagramSocket");
        }

        assertFalse(s.isConnected());
        assertFalse(s.isClosed());
        assertFalse(s.isBound());

        try {
            s.bind(new InetSocketAddress("0.0.0.0", 3333));
        } catch (SocketException e) {
            fail("exception binding DatagramSocket");
        }

        assertFalse(s.isConnected());
        assertFalse(s.isClosed());
        assertTrue(s.isBound());

        try {
            s.setBroadcast(true);
        } catch (SocketException e) {
            fail("exception setting broadcast");
        }

        Thread listener = new Thread(new Runnable() {
            @Override
            public void run() {
                DatagramSocket s = null;
                try {
                    s = new DatagramSocket(null);
                } catch (SocketException e) {
                    fail("exception creating DatagramSocket");
                }
                try {
                    s.bind(new InetSocketAddress("0.0.0.0", 3334));
                } catch (SocketException e) {
                    fail("exception binding DatagramSocket");
                }
                byte[] recv = new byte[800];
                DatagramPacket pRecv = new DatagramPacket(recv, recv.length);
                int recvLen = 0;
                try {
                    recvLen = s.receive(pRecv);
                } catch (IOException e) {
                    fail("exception receiving on DatagramSocket");
                }
                assertEquals(recvLen, "Hello, World!".length());
                s.close();
            }
        });

        listener.start();

        try {
            Thread.sleep(10, 0);
        } catch (InterruptedException e) {

        }

        byte[] send = "Hello, World!".getBytes();
        DatagramPacket p = new DatagramPacket(send, send.length, new InetSocketAddress("169.254.255.255", 3334));

        try {
            s.send(p);
        } catch (IOException e) {
            fail("exception sending on DatagramSocket");
        }

        try {
            listener.join();
        } catch (InterruptedException e) {

        }
        s.close();
        assertTrue(s.isClosed());
    }
}
