#include "quiet-jni.h"

void lwip_error_throw_exc(JNIEnv *env) {
    switch(errno) {
        case EBADF:
            throw_error(env, cache.java.socket_exception_klass, "Socket is closed");
            break;
        case EWOULDBLOCK:
            throw_error(env, cache.java.io_exception_klass, "Asynchronous operation would block");
            break;
        case ETIMEDOUT:
            throw_error(env, cache.java.socket_timeout_exception_klass, "Operation reached timeout");
            break;
        case EHOSTUNREACH:
            throw_error(env, cache.java.no_route_exception_klass, "No route to host");
            break;
        default:
            throw_error(env, cache.java.io_exception_klass, "Unspecified IO Error %d", errno);
    }
}

void lwip_android_destroy(quiet_lwip_android *e) {
    if (e->is_loopback) {
        loopback_remove_producer(e->loopback, e->producer);
        loopback_remove_consumer(e->loopback, e->consumer);
    }
    if (e->player != NULL) {
        quiet_opensl_destroy_player(e->player);
    }
    if (e->recorder != NULL) {
        quiet_opensl_destroy_recorder(e->recorder);
    }
    if (e->consumer != NULL) {
        opensl_consumer_destroy(e->consumer);
    }
    if (e->producer != NULL) {
        opensl_producer_destroy(e->producer);
    }
    if (e->interface != NULL) {
        quiet_lwip_destroy(e->interface);
    }
    free(e);
}

void lwip_android_terminate(quiet_lwip_android *e, int urgency) {
    quiet_lwip_close(e->interface);

    if (e->is_loopback) {
        loopback_remove_consumer(e->loopback, e->consumer);
    } else {
        quiet_opensl_stop_recorder(e->recorder);
    }

    if (urgency < 1) {
        return;
    }

    if (e->is_loopback) {
        loopback_remove_producer(e->loopback, e->producer);
    } else {
        // invoke stop on our player. if we have drained the frame buffer then this will
        // smoothly stop sound playback
        quiet_opensl_stop_player(e->player);
    }
}

quiet_lwip_android *lwip_android_create(JNIEnv *env, quiet_lwip_driver_config *conf,
                                        quiet_lwip_ipv4_addr local_address,
                                        quiet_lwip_ipv4_addr netmask,
                                        quiet_lwip_ipv4_addr gateway,
                                        quiet_android_system *sys,
                                        bool is_loopback,
                                        size_t encoder_num_bufs,
                                        size_t encoder_buf_len,
                                        size_t decoder_num_bufs,
                                        size_t decoder_buf_len) {
    quiet_lwip_android *e = malloc(sizeof(quiet_lwip_android));
    e->interface = quiet_lwip_create(conf, local_address, netmask, gateway);
    if (!e->interface) {
        lwip_android_destroy(e);
        throw_error(env, cache.system.init_exc_klass, lwip_error_format);
        return NULL;
    }

    if (is_loopback) {
        // ignore user-supplied buffer length values for loopback
        encoder_num_bufs = 1;
        encoder_buf_len = loopback_buffer_length;
        decoder_num_bufs = 1;
        decoder_buf_len = loopback_buffer_length;
    }

    e->producer = opensl_producer_create(encoder_num_bufs, encoder_buf_len);
    e->producer->produce = quiet_lwip_get_next_audio_packet;
    e->producer->produce_arg = e->interface;

    e->consumer = opensl_consumer_create(decoder_num_bufs, decoder_buf_len);
    e->consumer->consume = quiet_lwip_recv_audio_packet;
    e->consumer->consume_arg = e->interface;

    e->is_loopback = is_loopback;

    if (is_loopback) {
        loopback_add_producer(sys->loopback_sys, e->producer);
        loopback_add_consumer(sys->loopback_sys, e->consumer);
        e->loopback = sys->loopback_sys;
    } else {
        SLresult res = quiet_opensl_create_player(sys->opensl_sys, e->producer, &e->player);
        if (res != SL_RESULT_SUCCESS) {
            lwip_android_destroy(e);
            throw_error(env, cache.system.init_exc_klass, opensl_playback_error_format, res);
            return NULL;
        }

        res = quiet_opensl_create_recorder(sys->opensl_sys, e->consumer, &e->recorder);
        if (res != SL_RESULT_SUCCESS) {
            lwip_android_destroy(e);
            throw_error(env, cache.system.init_exc_klass, opensl_recorder_error_format, res);
            return NULL;
        }
    }

    // now invoke autoip if requested
    // we have to do this after setting up the audio connections
    // so that the autoip process can interact with other devices
    if (!local_address) {
        quiet_lwip_autoip(e->interface);
    }

    return e;
}

JNIEXPORT jvm_pointer JNICALL Java_org_quietmodem_Quiet_BaseNetworkInterface_nativeOpen(
    JNIEnv *env, jobject This, jvm_pointer j_sys, jobject j_conf, jboolean is_loopback) {
    quiet_android_system *sys = (quiet_android_system *)recover_pointer(j_sys);

    quiet_lwip_android *l = NULL;

    quiet_lwip_driver_config *conf = malloc(sizeof(quiet_lwip_driver_config));

    jobject j_enc_profile = (*env)->GetObjectField(env, j_conf, cache.network_interface_config.encoder_profile);
    jvm_pointer j_enc_profile_ptr = (*env)->GetLongField(env, j_enc_profile, cache.encoder_profile.ptr);
    conf->encoder_opt = (quiet_encoder_options *)recover_pointer(j_enc_profile_ptr);
    conf->encoder_rate = 44100;
    size_t encoder_num_bufs = (*env)->GetLongField(env, j_enc_profile, cache.encoder_profile.num_bufs);
    size_t encoder_buf_len = (*env)->GetLongField(env, j_enc_profile, cache.encoder_profile.buf_len);

    jobject j_dec_profile = (*env)->GetObjectField(env, j_conf, cache.network_interface_config.decoder_profile);
    jvm_pointer j_dec_profile_ptr = (*env)->GetLongField(env, j_dec_profile, cache.decoder_profile.ptr);
    conf->decoder_opt = (quiet_decoder_options *)recover_pointer(j_dec_profile_ptr);
    conf->decoder_rate = 44100;
    size_t decoder_num_bufs = (*env)->GetLongField(env, j_dec_profile, cache.decoder_profile.num_bufs);
    size_t decoder_buf_len = (*env)->GetLongField(env, j_dec_profile, cache.decoder_profile.buf_len);

    jbyteArray j_hardware_addr = (jbyteArray)((*env)->GetObjectField(env, j_conf, cache.network_interface_config.hardware_address));
    size_t hardware_addr_len = (*env)->GetArrayLength(env, j_hardware_addr);
    if (hardware_addr_len != 6) {
        throw_error(env, cache.system.init_exc_klass, "hardware address must be exactly 6 bytes");
        goto network_interface_native_open_dealloc;
    }
    (*env)->GetByteArrayRegion(env, j_hardware_addr, 0, 6, (jbyte *)conf->hardware_addr);

    uint32_t local_addr, netmask, gateway;
    jobject j_local_addr = (*env)->GetObjectField(env, j_conf, cache.network_interface_config.local_address);
    jobject j_netmask = (*env)->GetObjectField(env, j_conf, cache.network_interface_config.netmask);
    jobject j_gateway = (*env)->GetObjectField(env, j_conf, cache.network_interface_config.gateway);
    jbyteArray j_local_addr_bytes = (jbyteArray)((*env)->GetObjectField(env, j_local_addr, cache.inet_address.addr_bytes));
    jbyteArray j_netmask_bytes = (jbyteArray)((*env)->GetObjectField(env, j_netmask, cache.inet_address.addr_bytes));
    jbyteArray j_gateway_bytes = (jbyteArray)((*env)->GetObjectField(env, j_gateway, cache.inet_address.addr_bytes));
    (*env)->GetByteArrayRegion(env, j_local_addr_bytes, 0, 4, (jbyte *)&local_addr);
    (*env)->GetByteArrayRegion(env, j_netmask_bytes, 0, 4, (jbyte *)&netmask);
    (*env)->GetByteArrayRegion(env, j_gateway_bytes, 0, 4, (jbyte *)&gateway);

    uint8_t *addr_bytes = (uint8_t *)&local_addr;

    l = lwip_android_create(env, conf, local_addr, netmask, gateway, sys, is_loopback,
                            encoder_num_bufs, encoder_buf_len, decoder_num_bufs, decoder_buf_len);

    // l could be NULL and we may have exception thrown -- lwip_android_create handles this

network_interface_native_open_dealloc:
    if (conf != NULL) {
        free(conf);
    }

    return jvm_opaque_pointer(l);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseNetworkInterface_nativeTerminate(
        JNIEnv *env, jobject This, jint urgency) {
    jvm_pointer j_intf = (*env)->GetLongField(env, This, cache.network_interface.ptr);
    quiet_lwip_android *e = (quiet_lwip_android *)recover_pointer(j_intf);
    lwip_android_terminate(e, urgency);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseNetworkInterface_nativeClose(
        JNIEnv *env, jobject This) {
    jvm_pointer j_intf = (*env)->GetLongField(env, This, cache.network_interface.ptr);
    quiet_lwip_android *e = (quiet_lwip_android *)recover_pointer(j_intf);
    lwip_android_terminate(e, 0);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseNetworkInterface_nativeFree(
        JNIEnv *env, jobject This) {
    jvm_pointer j_intf = (*env)->GetLongField(env, This, cache.network_interface.ptr);
    quiet_lwip_android *e = (quiet_lwip_android *)recover_pointer(j_intf);
    free(e);
}
