#include "quiet-jni.h"

jvm_pointer jvm_opaque_pointer(void *p) { return (jvm_pointer)(intptr_t)p; }

void *recover_pointer(jvm_pointer p) { return (void *)(intptr_t)p; }

void throw_error(JNIEnv *env, jclass exc_class, const char *err_fmt, ...) {
    char *err_msg;
    va_list err_args;
    va_start(err_args, err_fmt);
    vasprintf(&err_msg, err_fmt, err_args);
    va_end(err_args);
    (*env)->ThrowNew(env, exc_class, err_msg);
    free(err_msg);
}

void convert_stereoopensl2monofloat(const opensl_sample_t *stereo_buf, float *mono_f,
                                    size_t num_frames, unsigned int num_channels) {
    for (size_t i = 0; i < num_frames; i++) {
        // just skip every other sample e.g. ignore samples from right channel
#ifdef QUIET_JNI_USE_FLOAT
        mono_f[i] = stereo_buf[i * num_channels];
#else
        mono_f[i] = stereo_buf[i * num_channels] / (float)(SHRT_MAX);
#endif
    }
}

void convert_monofloat2stereoopensl(const float *mono_f, opensl_sample_t *stereo_buf,
                                    size_t num_frames) {
    for (size_t i = 0; i < num_frames; i++) {
        float temp = mono_f[i];
        temp = (temp > 1.0f) ? 1.0f : temp;
        temp = (temp < -1.0f) ? -1.0f : temp;
        // just skip every other sample e.g. leave the right channel empty
#ifdef QUIET_JNI_USE_FLOAT
        stereo_buf[i * num_playback_channels] = temp;
#else
        stereo_buf[i * num_playback_channels] = temp * SHRT_MAX;
#endif
    }
}

const char *opensl_engine_error_format = "failed to initialize opensl engine, opensl error code=%04d";
const char *opensl_playback_error_format = "failed to initialize opensl playback, opensl error code=%04d";
const char *opensl_recorder_error_format = "failed to initialize opensl recorder, opensl error code=%04d";
const char *encoder_profile_error_format = "invalid quiet encoder profile, quiet error code=%04d";
const char *decoder_profile_error_format = "invalid quiet decoder profile, quiet error code=%04d";
const char *encoder_error_format = "failed to initialize quiet encoder, quiet error code=%04d";
const char *decoder_error_format = "failed to initialize quiet decoder, quiet error code=%04d";
const char *lwip_error_format = "failed to initialize quiet-lwip";

void android_system_destroy(quiet_android_system *android_sys) {
    if (android_sys->opensl_sys) {
        quiet_opensl_system_destroy(android_sys->opensl_sys);
    }
    if (android_sys->loopback_sys) {
        quiet_loopback_system_destroy(android_sys->loopback_sys);
    }
    free(android_sys);
}

quiet_android_system *android_system_create(JNIEnv *env) {
    return calloc(1, sizeof(quiet_android_system));
}

java_cache cache;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    // classes
    jclass localUnknownHost =
        (*env)->FindClass(env, "java/net/UnknownHostException");

    cache.java.unknown_host_exception_klass = (jclass)((*env)->NewGlobalRef(env, localUnknownHost));
    (*env)->DeleteLocalRef(env, localUnknownHost);

    jclass localTimeout =
        (*env)->FindClass(env, "java/net/SocketTimeoutException");

    cache.java.socket_timeout_exception_klass = (jclass)((*env)->NewGlobalRef(env, localTimeout));
    (*env)->DeleteLocalRef(env, localTimeout);

    jclass localInterrupted =
        (*env)->FindClass(env, "java/io/InterruptedIOException");

    cache.java.interrupted_io_exception_klass = (jclass)((*env)->NewGlobalRef(env, localInterrupted));
    (*env)->DeleteLocalRef(env, localInterrupted);

    jclass localEOF =
        (*env)->FindClass(env, "java/io/EOFException");

    cache.java.eof_exception_klass = (jclass)((*env)->NewGlobalRef(env, localEOF));
    (*env)->DeleteLocalRef(env, localEOF);

    jclass localBind =
        (*env)->FindClass(env, "java/net/BindException");

    cache.java.bind_exception_klass = (jclass)((*env)->NewGlobalRef(env, localBind));
    (*env)->DeleteLocalRef(env, localBind);

    jclass localConnect =
        (*env)->FindClass(env, "java/net/ConnectException");

    cache.java.connect_exception_klass = (jclass)((*env)->NewGlobalRef(env, localConnect));
    (*env)->DeleteLocalRef(env, localConnect);

    jclass localSocket =
        (*env)->FindClass(env, "java/net/SocketException");

    cache.java.socket_exception_klass = (jclass)((*env)->NewGlobalRef(env, localSocket));
    (*env)->DeleteLocalRef(env, localSocket);

    jclass localIO =
        (*env)->FindClass(env, "java/io/IOException");

    cache.java.io_exception_klass = (jclass)((*env)->NewGlobalRef(env, localIO));
    (*env)->DeleteLocalRef(env, localIO);

    jclass localIllegalArg =
        (*env)->FindClass(env, "java/lang/IllegalArgumentException");
    cache.java.illegal_arg_klass = (jclass)((*env)->NewGlobalRef(env, localIllegalArg));
    (*env)->DeleteLocalRef(env, localIllegalArg);

    jclass localNoRoute =
        (*env)->FindClass(env, "java/net/NoRouteToHostException");
    cache.java.no_route_exception_klass = (jclass)((*env)->NewGlobalRef(env, localNoRoute));
    (*env)->DeleteLocalRef(env, localNoRoute);

    jclass localSystemClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/QuietSystem");

    cache.system.klass = (jclass)((*env)->NewGlobalRef(env, localSystemClass));
    (*env)->DeleteLocalRef(env, localSystemClass);

    jclass localSystemInitExcClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/ModemException");
    cache.system.init_exc_klass = (jclass)((*env)->NewGlobalRef(env, localSystemInitExcClass));
    (*env)->DeleteLocalRef(env, localSystemInitExcClass);

    jclass localInetAddressClass = (*env)->FindClass(env, "org/quietmodem/Quiet/InetAddress");
    cache.inet_address.klass = (jclass)((*env)->NewGlobalRef(env, localInetAddressClass));
    (*env)->DeleteLocalRef(env, localInetAddressClass);

    jclass localInetSocketAddressClass = (*env)->FindClass(env, "org/quietmodem/Quiet/InetSocketAddress");
    cache.inet_socket_address.klass = (jclass)((*env)->NewGlobalRef(env, localInetSocketAddressClass));
    (*env)->DeleteLocalRef(env, localInetSocketAddressClass);

    jclass localDatagramPacketClass = (*env)->FindClass(env, "org/quietmodem/Quiet/DatagramPacket");
    cache.datagram_packet.klass = (jclass)((*env)->NewGlobalRef(env, localDatagramPacketClass));
    (*env)->DeleteLocalRef(env, localDatagramPacketClass);

    jclass localEncoderProfileClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/FrameTransmitterConfig");
    cache.encoder_profile.klass = (jclass)((*env)->NewGlobalRef(env, localEncoderProfileClass));
    (*env)->DeleteLocalRef(env, localEncoderProfileClass);

    jclass localDecoderProfileClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/FrameReceiverConfig");
    cache.decoder_profile.klass = (jclass)((*env)->NewGlobalRef(env, localDecoderProfileClass));
    (*env)->DeleteLocalRef(env, localDecoderProfileClass);

    jclass localEncoderClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/BaseFrameTransmitter");

    cache.encoder.klass = (jclass)((*env)->NewGlobalRef(env, localEncoderClass));
    (*env)->DeleteLocalRef(env, localEncoderClass);

    jclass localDecoderClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/BaseFrameReceiver");

    cache.decoder.klass = (jclass)((*env)->NewGlobalRef(env, localDecoderClass));
    (*env)->DeleteLocalRef(env, localDecoderClass);

    jclass localComplexClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/Complex");
    cache.complex.klass = (jclass)((*env)->NewGlobalRef(env, localComplexClass));
    (*env)->DeleteLocalRef(env, localComplexClass);

    jclass localFrameStatsClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/FrameStats");
    cache.frame_stats.klass = (jclass)((*env)->NewGlobalRef(env, localFrameStatsClass));
    (*env)->DeleteLocalRef(env, localFrameStatsClass);

    jclass localNetworkInterfaceConfClass = (*env)->FindClass(
        env, "org/quietmodem/Quiet/NetworkInterfaceConfig");

    cache.network_interface_config.klass =
        (jclass)((*env)->NewGlobalRef(env, localNetworkInterfaceConfClass));
    (*env)->DeleteLocalRef(env, localNetworkInterfaceConfClass);

    jclass localNetworkInterfaceClass = (*env)->FindClass(
        env, "org/quietmodem/Quiet/BaseNetworkInterface");

    cache.network_interface.klass =
        (jclass)((*env)->NewGlobalRef(env, localNetworkInterfaceClass));
    (*env)->DeleteLocalRef(env, localNetworkInterfaceClass);

    jclass localDatagramClass = (*env)->FindClass(
        env, "org/quietmodem/Quiet/DatagramSocket");

    cache.datagram.klass =
        (jclass)((*env)->NewGlobalRef(env, localDatagramClass));
    (*env)->DeleteLocalRef(env, localDatagramClass);

    jclass localServerSocketClass = (*env)->FindClass(
        env, "org/quietmodem/Quiet/ServerSocket");

    cache.server_socket.klass =
        (jclass)((*env)->NewGlobalRef(env, localServerSocketClass));
    (*env)->DeleteLocalRef(env, localServerSocketClass);

    jclass localSocketClass = (*env)->FindClass(
        env, "org/quietmodem/Quiet/Socket");

    cache.socket.klass =
        (jclass)((*env)->NewGlobalRef(env, localSocketClass));
    (*env)->DeleteLocalRef(env, localSocketClass);

    jclass localInputStreamClass = (*env)->FindClass(
        env, "org/quietmodem/Quiet/SocketInputStream");

    cache.input_stream.klass =
        (jclass)((*env)->NewGlobalRef(env, localInputStreamClass));
    (*env)->DeleteLocalRef(env, localInputStreamClass);

    jclass localOutputStreamClass = (*env)->FindClass(
        env, "org/quietmodem/Quiet/SocketOutputStream");

    cache.output_stream.klass =
        (jclass)((*env)->NewGlobalRef(env, localOutputStreamClass));
    (*env)->DeleteLocalRef(env, localOutputStreamClass);

    // fields
    cache.java.socket_timeout_bytes =
        (*env)->GetFieldID(env, cache.java.socket_timeout_exception_klass, "bytesTransferred", "I");

    cache.java.interrupted_bytes =
        (*env)->GetFieldID(env, cache.java.interrupted_io_exception_klass, "bytesTransferred", "I");

    cache.system.ptr =
        (*env)->GetFieldID(env, cache.system.klass, "sys_ptr", "J");

    cache.inet_address.addr_bytes =
        (*env)->GetFieldID(env, cache.inet_address.klass, "address", "[B");

    cache.inet_socket_address.inet_address =
        (*env)->GetFieldID(env, cache.inet_socket_address.klass, "addr", "Lorg/quietmodem/Quiet/InetAddress;");

    cache.inet_socket_address.ctor_bytes =
        (*env)->GetMethodID(env, cache.inet_socket_address.klass, "<init>", "([BI)V");

    cache.datagram_packet.inet_socket_address =
        (*env)->GetFieldID(env, cache.datagram_packet.klass, "addr", "Lorg/quietmodem/Quiet/InetSocketAddress;");

    cache.datagram_packet.buf =
        (*env)->GetFieldID(env, cache.datagram_packet.klass, "buf", "[B");

    cache.datagram_packet.offset =
        (*env)->GetFieldID(env, cache.datagram_packet.klass, "offset", "I");

    cache.datagram_packet.length =
        (*env)->GetFieldID(env, cache.datagram_packet.klass, "length", "I");

    cache.datagram_packet.set_socket_addr =
        (*env)->GetMethodID(env, cache.datagram_packet.klass, "setSocketAddress", "([BI)V");

    cache.inet_socket_address.port =
        (*env)->GetFieldID(env, cache.inet_socket_address.klass, "port", "I");

    cache.encoder_profile.ptr =
        (*env)->GetFieldID(env, cache.encoder_profile.klass, "profile_ptr", "J");

    cache.encoder_profile.num_bufs =
        (*env)->GetFieldID(env, cache.encoder_profile.klass, "numBuffers", "J");

    cache.encoder_profile.buf_len =
        (*env)->GetFieldID(env, cache.encoder_profile.klass, "bufferLength", "J");

    cache.decoder_profile.ptr =
        (*env)->GetFieldID(env, cache.decoder_profile.klass, "profile_ptr", "J");

    cache.decoder_profile.num_bufs =
        (*env)->GetFieldID(env, cache.decoder_profile.klass, "numBuffers", "J");

    cache.decoder_profile.buf_len =
        (*env)->GetFieldID(env, cache.decoder_profile.klass, "bufferLength", "J");

    cache.encoder.ptr =
        (*env)->GetFieldID(env, cache.encoder.klass, "enc_ptr", "J");

    cache.decoder.ptr =
        (*env)->GetFieldID(env, cache.decoder.klass, "dec_ptr", "J");

    cache.complex.ctor =
        (*env)->GetMethodID(env, cache.complex.klass, "<init>", "(FF)V");

    cache.frame_stats.ctor =
        (*env)->GetMethodID(env, cache.frame_stats.klass, "<init>", "([Lorg/quietmodem/Quiet/Complex;FFZ)V");

    cache.network_interface_config.encoder_profile =
        (*env)->GetFieldID(env, cache.network_interface_config.klass, "transmitterConfig", "Lorg/quietmodem/Quiet/FrameTransmitterConfig;");

    cache.network_interface_config.decoder_profile =
        (*env)->GetFieldID(env, cache.network_interface_config.klass, "receiverConfig", "Lorg/quietmodem/Quiet/FrameReceiverConfig;");

    cache.network_interface_config.local_address =
        (*env)->GetFieldID(env, cache.network_interface_config.klass, "localAddress", "Lorg/quietmodem/Quiet/InetAddress;");

    cache.network_interface_config.netmask =
        (*env)->GetFieldID(env, cache.network_interface_config.klass, "netmask", "Lorg/quietmodem/Quiet/InetAddress;");

    cache.network_interface_config.gateway =
        (*env)->GetFieldID(env, cache.network_interface_config.klass, "gateway", "Lorg/quietmodem/Quiet/InetAddress;");

    cache.network_interface_config.hardware_address =
        (*env)->GetFieldID(env, cache.network_interface_config.klass, "hardwareAddress", "[B");

    cache.network_interface.ptr = (*env)->GetFieldID(
        env, cache.network_interface.klass, "interface_ptr", "J");

    cache.datagram.fd = (*env)->GetFieldID(env, cache.datagram.klass, "fd", "I");

    cache.server_socket.fd = (*env)->GetFieldID(env, cache.server_socket.klass, "fd", "I");

    cache.socket.fd = (*env)->GetFieldID(env, cache.socket.klass, "fd", "I");

    cache.input_stream.fd = (*env)->GetFieldID(env, cache.input_stream.klass, "fd", "I");

    cache.output_stream.fd = (*env)->GetFieldID(env, cache.output_stream.klass, "fd", "I");

    return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL
Java_org_quietmodem_Quiet_QuietInit_nativeLWIPInit(JNIEnv *env, jclass klass) {
    quiet_lwip_init();
}

JNIEXPORT jvm_pointer JNICALL
Java_org_quietmodem_Quiet_QuietSystem_nativeOpen(JNIEnv *env,
        jobject This) {
    return jvm_opaque_pointer(android_system_create(env));
}

JNIEXPORT void JNICALL
Java_org_quietmodem_Quiet_QuietSystem_nativeClose(JNIEnv *env, jobject This) {
    jvm_pointer j_sys = (*env)->GetLongField(env, This, cache.system.ptr);
    quiet_android_system *sys = (quiet_android_system *)recover_pointer(j_sys);
    android_system_destroy(sys);
}

JNIEXPORT void JNICALL
Java_org_quietmodem_Quiet_QuietSystem_nativeOpenOpenSL(JNIEnv *env,
                                                          jobject This) {
    jvm_pointer j_sys = (*env)->GetLongField(env, This, cache.system.ptr);
    quiet_android_system *sys = (quiet_android_system *)recover_pointer(j_sys);
    SLresult res = quiet_opensl_system_create(&sys->opensl_sys);

    if (res != SL_RESULT_SUCCESS) {
        quiet_opensl_system_destroy(sys->opensl_sys);
        throw_error(env, cache.system.init_exc_klass, opensl_engine_error_format, res);
        return;
    }
}

JNIEXPORT void JNICALL
Java_org_quietmodem_Quiet_QuietSystem_nativeOpenLoopback(JNIEnv *env,
        jobject This) {
    jvm_pointer j_sys = (*env)->GetLongField(env, This, cache.system.ptr);
    quiet_android_system *sys = (quiet_android_system *)recover_pointer(j_sys);
    quiet_loopback_system_create(&sys->loopback_sys);
}
