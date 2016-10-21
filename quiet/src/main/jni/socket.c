#include "quiet-jni.h"

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_Socket_nativeCreate(
        JNIEnv *env, jobject This) {
    int fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed to create new socket");
    }
    return fd;
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_Socket_nativeBind(
        JNIEnv *env, jobject This, jobject inet_socket_address) {
    jobject inet_addr = (*env)->GetObjectField(env, inet_socket_address, cache.inet_socket_address.inet_address);
    jbyteArray j_addr = (jbyteArray)((*env)->GetObjectField(env, inet_addr, cache.inet_address.addr_bytes));
    jint port = (*env)->GetIntField(env, inet_socket_address, cache.inet_socket_address.port);
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons((short)port);
    (*env)->GetByteArrayRegion(env, j_addr, 0, 4, (jbyte *)&saddr.sin_addr.s_addr);

    int res = lwip_bind(lwip_fd, (struct sockaddr *)&saddr, sizeof(saddr));

    if (res < 0) {
        throw_error(env, cache.java.bind_exception_klass, "failed to bind to address");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_Socket_nativeClose(
        JNIEnv *env, jobject This) {
    jint socket_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int res = lwip_close(socket_fd);

    if (res < 0) {
        lwip_error_throw_exc(env);
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_Socket_nativeConnect(
        JNIEnv *env, jobject This, jobject inet_socket_address) {
    jobject inet_addr = (*env)->GetObjectField(env, inet_socket_address, cache.inet_socket_address.inet_address);
    jbyteArray j_addr = (jbyteArray)((*env)->GetObjectField(env, inet_addr, cache.inet_address.addr_bytes));
    jint port = (*env)->GetIntField(env, inet_socket_address, cache.inet_socket_address.port);
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons((short)port);
    (*env)->GetByteArrayRegion(env, j_addr, 0, 4, (jbyte *)&saddr.sin_addr.s_addr);

    int res = lwip_connect(lwip_fd, (struct sockaddr *)&saddr, sizeof(saddr));

    if (res < 0) {
        lwip_error_throw_exc(env);
    }
}

JNIEXPORT jobject JNICALL Java_org_quietmodem_Quiet_Socket_nativeGetLocal(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
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

JNIEXPORT jobject JNICALL Java_org_quietmodem_Quiet_Socket_nativeGetRemote(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    struct sockaddr_in local;
    socklen_t local_len = sizeof(local);
    int res = lwip_getpeername(lwip_fd, (struct sockaddr *)&local, &local_len);
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

JNIEXPORT jboolean JNICALL Java_org_quietmodem_Quiet_Socket_nativeGetKeepAlive(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int keepalive;
    socklen_t len = sizeof(keepalive);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jboolean)(keepalive);
}

JNIEXPORT jboolean JNICALL Java_org_quietmodem_Quiet_Socket_nativeGetOOBInline(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int oobinline;
    socklen_t len = sizeof(oobinline);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_OOBINLINE, &oobinline, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jboolean)(oobinline);
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_Socket_nativeGetReceiveBufferSize(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int buffer_size;
    socklen_t len = sizeof(buffer_size);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_RCVBUF, &buffer_size, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jint)(buffer_size);
}

JNIEXPORT jboolean JNICALL Java_org_quietmodem_Quiet_Socket_nativeGetReuseAddress(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int reuse;
    socklen_t len = sizeof(reuse);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jboolean)(reuse);
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_Socket_nativeGetSendBufferSize(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int buffer_size;
    socklen_t len = sizeof(buffer_size);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_SNDBUF, &buffer_size, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jint)(buffer_size);
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_Socket_nativeGetSoLinger(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int linger;
    socklen_t len = sizeof(linger);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_LINGER, &linger, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jint)(linger);
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_Socket_nativeGetSoTimeout(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int timeout;
    socklen_t len = sizeof(timeout);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jint)(timeout);
}

JNIEXPORT jboolean JNICALL Java_org_quietmodem_Quiet_Socket_nativeGetTcpNoDelay(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int nodelay;
    socklen_t len = sizeof(nodelay);
    int res = lwip_getsockopt(lwip_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jboolean)(nodelay);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_Socket_nativeSetKeepAlive(
        JNIEnv *env, jobject This, jboolean j_keepalive) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int keepalive = (int)j_keepalive;
    socklen_t len = sizeof(keepalive);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_Socket_nativeSetOOBInline(
        JNIEnv *env, jobject This, jboolean j_oobinline) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int oobinline = (int)j_oobinline;
    socklen_t len = sizeof(oobinline);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_OOBINLINE, &oobinline, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_Socket_nativeSetReceiveBufferSize(
        JNIEnv *env, jobject This, jint buffer_size) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    socklen_t len = sizeof(buffer_size);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_RCVBUF, &buffer_size, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_Socket_nativeSetReuseAddress(
        JNIEnv *env, jobject This, jboolean j_reuse) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int reuse = (int)j_reuse;
    socklen_t len = sizeof(reuse);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_Socket_nativeSetSendBufferSize(
        JNIEnv *env, jobject This, jint buffer_size) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    socklen_t len = sizeof(buffer_size);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_SNDBUF, &buffer_size, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_Socket_nativeSetSoLinger(
        JNIEnv *env, jobject This, jint linger) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    socklen_t len = sizeof(linger);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_LINGER, &linger, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_Socket_nativeSetSoTimeout(
        JNIEnv *env, jobject This, jint timeout) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    socklen_t len = sizeof(timeout);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_Socket_nativeSetTcpNoDelay(
        JNIEnv *env, jobject This, jboolean j_nodelay) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.socket.fd);
    int nodelay = (int)j_nodelay;
    socklen_t len = sizeof(nodelay);
    int res = lwip_setsockopt(lwip_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}
