#include "quiet-jni.h"

void quiet_android_record_callback(void *dec_v, const float *buf, size_t num_frames) {
    quiet_decoder *d = (quiet_decoder *)dec_v;
    quiet_decoder_consume(d, buf, num_frames);
}

void android_decoder_destroy(quiet_android_decoder *dec) {
    if (dec->is_loopback) {
        loopback_remove_consumer(dec->loopback, dec->consumer);
    }
    if (dec->recorder) {
        quiet_opensl_destroy_recorder(dec->recorder);
    }
    if (dec->dec) {
        quiet_decoder_destroy(dec->dec);
    }
    if (dec->consumer) {
        opensl_consumer_destroy(dec->consumer);
    }
    free(dec);
}

void android_decoder_terminate(quiet_android_decoder *dec) {
    quiet_decoder_close(dec->dec);
    if (dec->is_loopback) {
        loopback_remove_consumer(dec->loopback, dec->consumer);
    } else {
        quiet_opensl_stop_recorder(dec->recorder);
    }
}

quiet_android_decoder *android_decoder_create(JNIEnv *env, const quiet_decoder_options *opt,
                                              quiet_android_system *sys, bool is_loopback,
                                              size_t num_bufs, size_t buf_len) {
    quiet_android_decoder *d = calloc(1, sizeof(quiet_android_decoder));
    d->dec = quiet_decoder_create(opt, 44100);
    if (!d->dec) {
        android_decoder_destroy(d);
        throw_error(env, cache.system.init_exc_klass, decoder_error_format, quiet_get_last_error());
        return NULL;
    }
    if (is_loopback) {
        // ignore user-requested buffer sizes for loopback
        num_bufs = 1;
        buf_len = loopback_buffer_length;
    }
    d->consumer = opensl_consumer_create(num_bufs, buf_len);
    d->consumer->consume = quiet_android_record_callback;
    d->consumer->consume_arg = d->dec;
    d->is_loopback = is_loopback;

    if (is_loopback) {
        loopback_add_consumer(sys->loopback_sys, d->consumer);
        d->loopback = sys->loopback_sys;
    } else {
        SLresult res = quiet_opensl_create_recorder(sys->opensl_sys, d->consumer, &d->recorder);
        if (res != SL_RESULT_SUCCESS) {
            android_decoder_destroy(d);
            throw_error(env, cache.system.init_exc_klass, opensl_recorder_error_format, res);
            return NULL;
        }
    }

    return d;
}

JNIEXPORT jvm_pointer JNICALL Java_org_quietmodem_Quiet_BaseFrameReceiver_nativeOpen(
    JNIEnv *env, jobject This, jvm_pointer j_sys, jobject conf, jboolean is_loopback) {
    quiet_android_system *sys = (quiet_android_system *)recover_pointer(j_sys);
    jvm_pointer j_profile = (*env)->GetLongField(env, conf, cache.decoder_profile.ptr);
    quiet_decoder_options *opt = (quiet_decoder_options *)recover_pointer(j_profile);
    size_t num_bufs = (*env)->GetLongField(env, conf, cache.decoder_profile.num_bufs);
    size_t buf_len = (*env)->GetLongField(env, conf, cache.decoder_profile.buf_len);

    quiet_android_decoder *dec = android_decoder_create(env, opt, sys, is_loopback, num_bufs, buf_len);

    return jvm_opaque_pointer(dec);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameReceiver_nativeClose(
        JNIEnv *env, jobject This) {
    jvm_pointer j_dec = (*env)->GetLongField(env, This, cache.decoder.ptr);
    quiet_android_decoder *dec = (quiet_android_decoder *)recover_pointer(j_dec);
    android_decoder_terminate(dec);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameReceiver_nativeFree(
    JNIEnv *env, jobject This) {
    jvm_pointer j_dec = (*env)->GetLongField(env, This, cache.decoder.ptr);
    quiet_android_decoder *dec = (quiet_android_decoder *)recover_pointer(j_dec);
    android_decoder_destroy(dec);
}

JNIEXPORT jlong JNICALL Java_org_quietmodem_Quiet_BaseFrameReceiver_nativeRecv(
    JNIEnv *env, jobject This, jbyteArray frame,
    jlong offset, jlong len) {
    jvm_pointer j_dec = (*env)->GetLongField(env, This, cache.decoder.ptr);
    quiet_android_decoder *dec = (quiet_android_decoder *)recover_pointer(j_dec);

    jbyte *buf = (*env)->GetByteArrayElements(env, frame, NULL);
    size_t buf_len = (*env)->GetArrayLength(env, frame);

    if (offset < 0) {
        offset = 0;
    }
    if (offset >= buf_len) {
        offset = buf_len - 1;
    }
    if (len < 0) {
        len = 0;
    }
    if (len > (buf_len - offset)) {
        len = buf_len - offset;
    }

    ssize_t nrecv = quiet_decoder_recv(dec->dec, buf + offset, len);

    if (nrecv == 0) {
        throw_error(env, cache.java.eof_exception_klass, "EOF");
    } else if (nrecv < 0) {
        quiet_error err = quiet_get_last_error();
        nrecv = 0;
        switch(err) {
            case quiet_would_block:
                throw_error(env, cache.java.io_exception_klass, "Asynchronous operation would block");
                break;
            case quiet_timedout:
                throw_error(env, cache.java.socket_timeout_exception_klass, "Timed out");
                break;
            default:
                throw_error(env, cache.java.io_exception_klass, "Unspecified I/O Error %d", err);
                break;
        }
    }

    (*env)->ReleaseByteArrayElements(env, frame, buf, 0);

    return nrecv;
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameReceiver_nativeSetBlocking(
        JNIEnv *env, jobject This, jlong seconds, jlong nano) {
    jvm_pointer j_dec = (*env)->GetLongField(env, This, cache.decoder.ptr);
    quiet_android_decoder *dec = (quiet_android_decoder *)recover_pointer(j_dec);
    quiet_decoder_set_blocking(dec->dec, seconds, nano);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameReceiver_nativeSetNonblocking(
        JNIEnv *env, jobject This) {
    jvm_pointer j_dec = (*env)->GetLongField(env, This, cache.decoder.ptr);
    quiet_android_decoder *dec = (quiet_android_decoder *)recover_pointer(j_dec);
    quiet_decoder_set_nonblocking(dec->dec);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameReceiver_nativeEnableStats(
        JNIEnv *env, jobject This) {
    jvm_pointer j_dec = (*env)->GetLongField(env, This, cache.decoder.ptr);
    quiet_android_decoder *dec = (quiet_android_decoder *)recover_pointer(j_dec);
    quiet_decoder_enable_stats(dec->dec);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameReceiver_nativeDisableStats(
        JNIEnv *env, jobject This) {
    jvm_pointer j_dec = (*env)->GetLongField(env, This, cache.decoder.ptr);
    quiet_android_decoder *dec = (quiet_android_decoder *)recover_pointer(j_dec);
    quiet_decoder_disable_stats(dec->dec);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameReceiver_nativeStatsSetBlocking(
        JNIEnv *env, jobject This, jlong seconds, jlong nano) {
    jvm_pointer j_dec = (*env)->GetLongField(env, This, cache.decoder.ptr);
    quiet_android_decoder *dec = (quiet_android_decoder *)recover_pointer(j_dec);
    quiet_decoder_set_stats_blocking(dec->dec, seconds, nano);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameReceiver_nativeStatsSetNonblocking(
        JNIEnv *env, jobject This) {
    jvm_pointer j_dec = (*env)->GetLongField(env, This, cache.decoder.ptr);
    quiet_android_decoder *dec = (quiet_android_decoder *)recover_pointer(j_dec);
    quiet_decoder_set_stats_nonblocking(dec->dec);
}

JNIEXPORT jobject JNICALL Java_org_quietmodem_Quiet_BaseFrameReceiver_nativeRecvStats(
        JNIEnv *env, jobject This) {
    jvm_pointer j_dec = (*env)->GetLongField(env, This, cache.decoder.ptr);
    quiet_android_decoder *dec = (quiet_android_decoder *)recover_pointer(j_dec);
    const quiet_decoder_frame_stats *stats = quiet_decoder_recv_stats(dec->dec);

    if (!stats) {
        quiet_error err = quiet_get_last_error();
        switch (err) {
            case quiet_success:
                throw_error(env, cache.java.eof_exception_klass, "EOF");
                return NULL;
            case quiet_would_block:
                throw_error(env, cache.java.io_exception_klass, "Asynchronous operation would block");
                return NULL;
            case quiet_timedout:
                throw_error(env, cache.java.socket_timeout_exception_klass, "Timed out");
                return NULL;
            default:
                throw_error(env, cache.java.io_exception_klass, "Unspecified I/O Error %d", err);
                return NULL;
        }
    }

    jobject symbol_array = (*env)->NewObjectArray(env, stats->num_symbols, cache.complex.klass, NULL);

    for (size_t i = 0; i < stats->num_symbols; i++) {
        jobject symbol = (*env)->NewObject(env, cache.complex.klass, cache.complex.ctor, stats->symbols[i].real, stats->symbols[i].imaginary);
        (*env)->SetObjectArrayElement(env, symbol_array, i, symbol);
        (*env)->DeleteLocalRef(env, symbol);
    }

    jobject frame_stats = (*env)->NewObject(env, cache.frame_stats.klass, cache.frame_stats.ctor, symbol_array, stats->received_signal_strength_indicator, stats->error_vector_magnitude, stats->checksum_passed);

    return frame_stats;
}
