#include "quiet-jni.h"

ssize_t quiet_android_playback_callback(void *enc_v, float *buf, size_t num_frames) {
    quiet_encoder *e = (quiet_encoder *)enc_v;
    return quiet_encoder_emit(e, buf, num_frames);
}

void android_encoder_destroy(quiet_android_encoder *enc) {
    if (enc->is_loopback) {
        // loopback_remove_producer works even if producer isn't
        // present so this should be a safe call always
        loopback_remove_producer(enc->loopback, enc->producer);
    }
    if (enc->player) {
        quiet_opensl_destroy_player(enc->player);
    }
    if (enc->enc) {
        quiet_encoder_destroy(enc->enc);
    }
    if (enc->producer) {
        opensl_producer_destroy(enc->producer);
    }
    free(enc);
}

void android_encoder_terminate(quiet_android_encoder *enc, int urgency) {
    // close quiet's frame buffer. this will stop new frames from
    // being written and will cause the callback to see EOF once it has
    // drained the buffer. the callback will stop the player once it
    // has drained its own queue
    //
    // this process also works with the loopback which will detect EOF
    // and eject our producer
    quiet_encoder_close(enc->enc);

    if (urgency < 1) {
        return;
    }

    if (enc->is_loopback) {
        loopback_remove_producer(enc->loopback, enc->producer);
    } else {
        // invoke stop on our player. if we have drained the frame buffer then this will
        // smoothly stop sound playback
        quiet_opensl_stop_player(enc->player);
    }
}

quiet_android_encoder *android_encoder_create(JNIEnv *env, const quiet_encoder_options *opt,
                                              quiet_android_system *sys, bool is_loopback,
                                              size_t num_bufs, size_t buf_len) {
    quiet_android_encoder *e = calloc(1, sizeof(quiet_android_encoder));
    e->enc = quiet_encoder_create(opt, 44100);
    if (!e->enc) {
        android_encoder_destroy(e);
        throw_error(env, cache.system.init_exc_klass, encoder_error_format, quiet_get_last_error());
        return NULL;
    }
    if (is_loopback) {
        // ignore user-supplied buffer lengths for loopback
        num_bufs = 1;
        buf_len = loopback_buffer_length;
    }
    e->producer = opensl_producer_create(num_bufs, buf_len);
    e->producer->produce = quiet_android_playback_callback;
    e->producer->produce_arg = e->enc;
    e->is_loopback = is_loopback;

    if (is_loopback) {
        loopback_add_producer(sys->loopback_sys, e->producer);
        e->loopback = sys->loopback_sys;
    } else {
        SLresult res = quiet_opensl_create_player(sys->opensl_sys, e->producer, &e->player);
        if (res != SL_RESULT_SUCCESS) {
            android_encoder_destroy(e);
            throw_error(env, cache.system.init_exc_klass, opensl_playback_error_format, res);
            return NULL;
        }
    }

    return e;
}

JNIEXPORT jvm_pointer JNICALL Java_org_quietmodem_Quiet_BaseFrameTransmitter_nativeOpen(
    JNIEnv *env, jobject This, jvm_pointer j_sys, jobject conf, jboolean is_loopback) {
    quiet_android_system *sys = (quiet_android_system *)recover_pointer(j_sys);
    jvm_pointer j_profile = (*env)->GetLongField(env, conf, cache.encoder_profile.ptr);
    quiet_encoder_options *opt = (quiet_encoder_options *)recover_pointer(j_profile);
    size_t num_bufs = (*env)->GetLongField(env, conf, cache.encoder_profile.num_bufs);
    size_t buf_len = (*env)->GetLongField(env, conf, cache.encoder_profile.buf_len);

    quiet_android_encoder *enc = android_encoder_create(env, opt, sys, is_loopback, num_bufs, buf_len);

    return jvm_opaque_pointer(enc);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameTransmitter_nativeTerminate(
        JNIEnv *env, jobject This, jint urgency) {
    jvm_pointer j_enc = (*env)->GetLongField(env, This, cache.encoder.ptr);
    quiet_android_encoder *enc = (quiet_android_encoder *)recover_pointer(j_enc);
    android_encoder_terminate(enc, urgency);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameTransmitter_nativeClose(
    JNIEnv *env, jobject This) {
    jvm_pointer j_enc = (*env)->GetLongField(env, This, cache.encoder.ptr);
    quiet_android_encoder *enc = (quiet_android_encoder *)recover_pointer(j_enc);
    android_encoder_terminate(enc, 0);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameTransmitter_nativeFree(
        JNIEnv *env, jobject This) {
    jvm_pointer j_enc = (*env)->GetLongField(env, This, cache.encoder.ptr);
    quiet_android_encoder *enc = (quiet_android_encoder *)recover_pointer(j_enc);
    android_encoder_destroy(enc);
}

JNIEXPORT jlong JNICALL Java_org_quietmodem_Quiet_BaseFrameTransmitter_nativeSend(
    JNIEnv *env, jobject This, jbyteArray frame,
    jlong offset, jlong len) {
    jvm_pointer j_enc = (*env)->GetLongField(env, This, cache.encoder.ptr);
    quiet_android_encoder *enc = (quiet_android_encoder *)recover_pointer(j_enc);

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

    ssize_t nsent = quiet_encoder_send(enc->enc, buf + offset, len);
    (*env)->ReleaseByteArrayElements(env, frame, buf, JNI_ABORT);

    if (nsent == 0) {
        throw_error(env, cache.java.eof_exception_klass, "EOF");
    } else if (nsent < 0) {
        quiet_error err = quiet_get_last_error();
        nsent = 0;
        switch(err) {
            case quiet_msg_size:
                throw_error(env, cache.java.io_exception_klass, "Message too long");
                break;
            case quiet_would_block:
                throw_error(env, cache.java.io_exception_klass, "Asynchronous operation would block");
                break;
            case quiet_timedout:
                throw_error(env, cache.java.socket_timeout_exception_klass, "Timed out");
                break;
            default:
                throw_error(env, cache.java.io_exception_klass, "Unspecified I/O Error");
                break;
        }
    }

    return nsent;
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameTransmitter_nativeSetBlocking(
        JNIEnv *env, jobject This, jlong sec, jlong nano) {
    jvm_pointer j_enc = (*env)->GetLongField(env, This, cache.encoder.ptr);
    quiet_android_encoder *enc = (quiet_android_encoder *)recover_pointer(j_enc);

    quiet_encoder_set_blocking(enc->enc, sec, nano);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_BaseFrameTransmitter_nativeSetNonblocking(
        JNIEnv *env, jobject This) {
    jvm_pointer j_enc = (*env)->GetLongField(env, This, cache.encoder.ptr);
    quiet_android_encoder *enc = (quiet_android_encoder *)recover_pointer(j_enc);

    quiet_encoder_set_nonblocking(enc->enc);
}

JNIEXPORT jlong JNICALL Java_org_quietmodem_Quiet_BaseFrameTransmitter_nativeClampFrameLen(
        JNIEnv *env, jobject This, jlong sample_length) {
    jvm_pointer j_enc = (*env)->GetLongField(env, This, cache.encoder.ptr);
    quiet_android_encoder *enc = (quiet_android_encoder *)recover_pointer(j_enc);

    return quiet_encoder_clamp_frame_len(enc->enc, sample_length);
}

JNIEXPORT jlong JNICALL Java_org_quietmodem_Quiet_BaseFrameTransmitter_nativeGetFrameLength(
        JNIEnv *env, jobject This) {
    jvm_pointer j_enc = (*env)->GetLongField(env, This, cache.encoder.ptr);
    quiet_android_encoder *enc = (quiet_android_encoder *)recover_pointer(j_enc);

    return quiet_encoder_get_frame_len(enc->enc);
}

