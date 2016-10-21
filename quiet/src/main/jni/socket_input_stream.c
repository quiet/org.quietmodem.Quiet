#include "quiet-jni.h"

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_SocketInputStream_nativeRead(
        JNIEnv *env, jobject This, jbyteArray j_buf,
        jlong offset, jlong len) {

    jint lwip_fd = (*env)->GetIntField(env, This, cache.input_stream.fd);
    jbyte *buf = (*env)->GetByteArrayElements(env, j_buf, NULL);
    size_t buf_len = (*env)->GetArrayLength(env, j_buf);

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

    ssize_t nrecv = lwip_read(lwip_fd, buf + offset, len);

    (*env)->ReleaseByteArrayElements(env, j_buf, buf, 0);

    if (nrecv < 0) {
        lwip_error_throw_exc(env);
    } else if (nrecv == 0) {
        nrecv = -1;
    }

    return (jint)nrecv;
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_SocketInputStream_nativeClose(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.input_stream.fd);

    int res = lwip_shutdown(lwip_fd, SHUT_RD);

    if (res < 0) {
        lwip_error_throw_exc(env);
    }
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_SocketInputStream_nativeAvailable(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.input_stream.fd);

    uint16_t avail;
    int res = lwip_ioctl(lwip_fd, FIONREAD, (void *)&avail);

    if (res < 0) {
        lwip_error_throw_exc(env);
    }

    return (jint)(avail);
}
