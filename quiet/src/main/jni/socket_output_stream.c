#include "quiet-jni.h"

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_SocketOutputStream_nativeWrite(
    JNIEnv *env, jobject This, jbyteArray j_buf,
    jlong offset, jlong len) {

    jint lwip_fd = (*env)->GetIntField(env, This, cache.output_stream.fd);
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

    int nsent = lwip_write(lwip_fd, buf + offset, len);
    (*env)->ReleaseByteArrayElements(env, j_buf, buf, JNI_ABORT);

    if (nsent == 0) {
        throw_error(env, cache.java.eof_exception_klass, "EOF");
    } else if (nsent < 0) {
        lwip_error_throw_exc(env);
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_SocketOutputStream_nativeClose(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.output_stream.fd);

    int res = lwip_shutdown(lwip_fd, SHUT_WR);

    if (res < 0) {
        lwip_error_throw_exc(env);
    }
}
