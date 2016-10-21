package org.quietmodem.Quiet;

import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;
import android.test.suitebuilder.annotation.MediumTest;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.*;
import java.io.IOException;
import java.net.SocketException;

@RunWith(AndroidJUnit4.class)
@MediumTest
public class DatagramTest {
    private static LoopbackNetworkInterface intf;
    @BeforeClass
    public static void setup() {
        FrameReceiverConfig receiverConfig = null;
        FrameTransmitterConfig transmitterConfig = null;

        try {
            transmitterConfig = new FrameTransmitterConfig(InstrumentationRegistry.getTargetContext(), "audible-7k-channel-0");
            receiverConfig = new FrameReceiverConfig(InstrumentationRegistry.getTargetContext(), "audible-7k-channel-0");
        } catch (IOException e) {
            fail("could not build configs");

        }

        NetworkInterfaceConfig conf = null;
        try {
            conf = new NetworkInterfaceConfig(
                    receiverConfig,
                    transmitterConfig,
                    InetAddress.getByName("192.168.0.3"),
                    InetAddress.getByName("255.255.255.0"),
                    InetAddress.getByName("192.168.0.1"));
        } catch (IOException e) {
            fail("could not build network config");
        }

        try {
            intf = new LoopbackNetworkInterface(conf);
        } catch (ModemException e) {
            fail("network interface threw exception");
        }
    }

    @AfterClass
    public static void teardown() {
        intf.close();
    }

    @Test
    public void testFoo() {
        DatagramSocket sSend = null;
        DatagramSocket sRecv = null;
        try {
            sSend = new DatagramSocket(null);
            sRecv = new DatagramSocket(new InetSocketAddress("0.0.0.0", 3334));
        } catch (SocketException e) {
            fail("exception creating DatagramSocket");
        }

        assertFalse(sSend.isConnected());
        assertFalse(sSend.isClosed());
        assertFalse(sSend.isBound());

        try {
            sSend.bind(new InetSocketAddress("0.0.0.0", 3333));
        } catch (SocketException e) {
            fail("exception binding DatagramSocket");
        }

        assertFalse(sSend.isConnected());
        assertFalse(sSend.isClosed());
        assertTrue(sSend.isBound());

        try {
            sSend.setBroadcast(true);
        } catch (SocketException e) {
            fail("exception setting broadcast");
        }


        byte[] send = "Hello, World!".getBytes();
        DatagramPacket p = new DatagramPacket(send, send.length, new InetSocketAddress("192.168.0.255", 3334));

        try {
            sSend.send(p);
        } catch (IOException e) {
            fail("exception sending on DatagramSocket");
        }

        byte[] recv = new byte[800];
        DatagramPacket pRecv = new DatagramPacket(recv, recv.length);
        int recvLen = 0;
        try {
            recvLen = sRecv.receive(pRecv);
        } catch (IOException e) {
            fail("exception receiving on DatagramSocket");
        }
        assertEquals(recvLen, send.length);

        sSend.close();
        assertTrue(sSend.isClosed());
        sRecv.close();
    }
}
