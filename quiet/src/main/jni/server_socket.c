#include "quiet-jni.h"

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_ServerSocket_nativeCreate(
        JNIEnv *env, jobject This) {
    int fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed to create new socket");
    }
    return fd;
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_ServerSocket_nativeBind(
        JNIEnv *env, jobject This, jobject inet_socket_address, jint backlog) {
    jobject inet_addr = (*env)->GetObjectField(env, inet_socket_address, cache.inet_socket_address.inet_address);
    jbyteArray j_addr = (jbyteArray)((*env)->GetObjectField(env, inet_addr, cache.inet_address.addr_bytes));
    jint port = (*env)->GetIntField(env, inet_socket_address, cache.inet_socket_address.port);
    jint lwip_fd = (*env)->GetIntField(env, This, cache.server_socket.fd);

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons((short)port);
    (*env)->GetByteArrayRegion(env, j_addr, 0, 4, (jbyte *)&saddr.sin_addr.s_addr);

    int res = lwip_bind(lwip_fd, (struct sockaddr *)&saddr, sizeof(saddr));

    if (res < 0) {
        throw_error(env, cache.java.bind_exception_klass, "failed to bind to address");
    }

    res = lwip_listen(lwip_fd, backlog);

    if (res < 0) {
        throw_error(env, cache.java.bind_exception_klass, "failed to listen on address");
    }
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_ServerSocket_nativeAccept(
        JNIEnv *env, jobject This) {
    jint socket_fd = (*env)->GetIntField(env, This, cache.server_socket.fd);
    struct sockaddr_in recv_from;
    socklen_t recv_from_len = sizeof(recv_from);
    int fd = lwip_accept(socket_fd, (struct sockaddr *)&recv_from, &recv_from_len);

    if (fd < 0) {
        lwip_error_throw_exc(env);
    }

    return fd;
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_ServerSocket_nativeClose(
        JNIEnv *env, jobject This) {
    jint socket_fd = (*env)->GetIntField(env, This, cache.server_socket.fd);
    lwip_close(socket_fd);
}

JNIEXPORT jobject JNICALL Java_org_quietmodem_Quiet_ServerSocket_nativeGetLocal(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.server_socket.fd);
    struct sockaddr_in local;
    socklen_t local_len = sizeof(local);
    int res = lwip_getsockname(lwip_fd, (struct sockaddr *)&local, &local_len);
    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "Socket is closed");
        return NULL;
    } else {
        jint port = ntohs(local.sin_port);
        jbyteArray addr_buf = (*env)->NewByteArray(env, 4);
        (*env)->SetByteArrayRegion(env, addr_buf, 0, 4, (jbyte *)(&local.sin_addr.s_addr));
        jobject inet_socket_addr = (*env)->NewObject(env, cache.inet_socket_address.klass, cache.inet_socket_address.ctor_bytes, addr_buf, port);
        return inet_socket_addr;
    }
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_ServerSocket_nativeGetReceiveBufferSize(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.server_socket.fd);
    int buffer_size;
    socklen_t len = sizeof(buffer_size);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_RCVBUF, &buffer_size, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jint)(buffer_size);
}

JNIEXPORT jboolean JNICALL Java_org_quietmodem_Quiet_ServerSocket_nativeGetReuseAddress(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.server_socket.fd);
    int reuse;
    socklen_t len = sizeof(reuse);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jboolean)(reuse);
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_ServerSocket_nativeGetSoTimeout(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.server_socket.fd);
    int timeout;
    socklen_t len = sizeof(timeout);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jint)(timeout);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_ServerSocket_nativeSetReceiveBufferSize(
        JNIEnv *env, jobject This, jint size) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.server_socket.fd);
    socklen_t len = sizeof(size);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_RCVBUF, &size, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_ServerSocket_nativeSetReuseAddress(
        JNIEnv *env, jobject This, jboolean j_reuse) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.server_socket.fd);
    int reuse = (int)(j_reuse);
    socklen_t len = sizeof(reuse);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_ServerSocket_nativeSetSoTimeout(
        JNIEnv *env, jobject This, jint timeout) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.server_socket.fd);
    socklen_t len = sizeof(timeout);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}
