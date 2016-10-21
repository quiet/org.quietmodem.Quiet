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
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.SocketException;
import java.util.Arrays;

@RunWith(AndroidJUnit4.class)
@MediumTest
public class SocketTest {
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
        Thread listener = new Thread(new Runnable() {
            @Override
            public void run() {
                ServerSocket s = null;
                try {
                    s = new ServerSocket(5678);
                } catch (IOException e) {
                    fail("exception creating ServerSocket");
                }

                byte[] recv = new byte[800];

                Socket peer = null;
                try {
                    peer = s.accept();
                } catch (IOException e) {
                    fail("accept failed");
                }

                InputStream is = peer.getInputStream();
                OutputStream os = peer.getOutputStream();

                int recvLen = 0;
                try {
                    recvLen = is.read(recv);
                } catch (IOException e) {
                    fail("read failed");
                }

                try {
                    os.write(recv, 0, recvLen);
                } catch (IOException e) {
                    fail("write failed");
                }

                try {
                    peer.close();
                    s.close();
                } catch (IOException e) {
                    fail("close failed");
                }
            }
        });
        listener.start();

        // let listener set up
        try {
            Thread.sleep(10, 0);
        } catch (InterruptedException e) {

        }

        Socket s = null;
        try {
            s = new Socket();
            s.bind(new InetSocketAddress("192.168.0.3", 0));
        } catch (IOException e) {
            fail("new socket failed");
        }

        try {
            s.connect(new InetSocketAddress("192.168.0.3", 5678));
        } catch (IOException e) {
            fail(e.getMessage());
            fail("connect failed");
        }

        byte[] send = "Hello, World!".getBytes();
        InputStream is = s.getInputStream();
        OutputStream os = s.getOutputStream();

        try {
            os.write(send);
        } catch (IOException e) {
            fail("exception sending on Socket");
        }

        byte[] recv = new byte[800];
        int recvLen = 0;
        try {
            recvLen = is.read(recv);
        } catch (IOException e) {
            fail("exception receiving on DatagramSocket");
        }
        assertEquals(recvLen, send.length);
        assertTrue(Arrays.equals(send, Arrays.copyOfRange(recv, 0, recvLen)));

        try {
            listener.join();
        } catch (InterruptedException e) {

        }

        try {
            s.close();
        } catch (IOException e) {
            fail("close failed");
        }
        assertTrue(s.isClosed());
    }
}
