#include "quiet-jni.h"

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeCreate(
        JNIEnv *env, jobject This) {
    int fd = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed to create new socket");
    }
    return fd;
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeBind(
        JNIEnv *env, jobject This, jobject inet_socket_address) {
    jobject inet_addr = (*env)->GetObjectField(env, inet_socket_address, cache.inet_socket_address.inet_address);
    jbyteArray j_addr = (jbyteArray)((*env)->GetObjectField(env, inet_addr, cache.inet_address.addr_bytes));
    jint port = (*env)->GetIntField(env, inet_socket_address, cache.inet_socket_address.port);
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons((short)port);
    (*env)->GetByteArrayRegion(env, j_addr, 0, 4, (jbyte *)&saddr.sin_addr.s_addr);

    int res = lwip_bind(lwip_fd, (struct sockaddr *)&saddr, sizeof(saddr));

    if (res < 0) {
        throw_error(env, cache.java.bind_exception_klass, "failed to bind to address");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeClose(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);

    lwip_close(lwip_fd);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeConnect(
        JNIEnv *env, jobject This, jobject inet_socket_address) {
    jobject inet_addr = (*env)->GetObjectField(env, inet_socket_address, cache.inet_socket_address.inet_address);
    jbyteArray j_addr = (jbyteArray)((*env)->GetObjectField(env, inet_addr, cache.inet_address.addr_bytes));
    jint port = (*env)->GetIntField(env, inet_socket_address, cache.inet_socket_address.port);
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons((short)port);
    (*env)->GetByteArrayRegion(env, j_addr, 0, 4, (jbyte *)&saddr.sin_addr.s_addr);

    int res = lwip_connect(lwip_fd, (struct sockaddr *)&saddr, sizeof(saddr));

    if (res < 0) {
        throw_error(env, cache.java.connect_exception_klass, "failed to connect socket");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeDisconnect(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);

    struct sockaddr_in saddr;
    saddr.sin_family = AF_UNSPEC;

    lwip_connect(lwip_fd, (struct sockaddr *)&saddr, sizeof(saddr));
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeReceive(
        JNIEnv *env, jobject This, jobject datagram_packet) {
    jbyteArray j_buf = (*env)->GetObjectField(env, datagram_packet, cache.datagram_packet.buf);
    jlong offset = (*env)->GetIntField(env, datagram_packet, cache.datagram_packet.offset);
    jlong len = (*env)->GetIntField(env, datagram_packet, cache.datagram_packet.length);
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
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

    struct sockaddr_in recv_from;
    socklen_t recv_from_len = sizeof(recv_from);
    ssize_t nrecv = lwip_recvfrom(lwip_fd, buf + offset, len, 0, (struct sockaddr *)&recv_from, &recv_from_len);
    if (nrecv == 0) {
        throw_error(env, cache.java.eof_exception_klass, "EOF");
    } else if (nrecv < 0) {
        lwip_error_throw_exc(env);
    } else {
        jint port = ntohs(recv_from.sin_port);
        jbyteArray addr_buf = (*env)->NewByteArray(env, 4);
        (*env)->SetByteArrayRegion(env, addr_buf, 0, 4, (jbyte *)(&recv_from.sin_addr.s_addr));
        (*env)->CallVoidMethod(env, datagram_packet, cache.datagram_packet.set_socket_addr, addr_buf, port);
        (*env)->SetIntField(env, datagram_packet, cache.datagram_packet.offset, offset + len);
    }

    (*env)->ReleaseByteArrayElements(env, j_buf, buf, 0);

    return nrecv;
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeSend(
    JNIEnv *env, jobject This, jobject datagram_packet) {
    jbyteArray j_buf = (*env)->GetObjectField(env, datagram_packet, cache.datagram_packet.buf);
    jlong offset = (*env)->GetIntField(env, datagram_packet, cache.datagram_packet.offset);
    jlong len = (*env)->GetIntField(env, datagram_packet, cache.datagram_packet.length);
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    jbyte *buf = (*env)->GetByteArrayElements(env, j_buf, NULL);
    size_t buf_len = (*env)->GetArrayLength(env, j_buf);
    jobject inet_socket_address = (*env)->GetObjectField(env, datagram_packet, cache.datagram_packet.inet_socket_address);
    jobject inet_addr = (*env)->GetObjectField(env, inet_socket_address, cache.inet_socket_address.inet_address);
    jbyteArray j_addr = (jbyteArray)((*env)->GetObjectField(env, inet_addr, cache.inet_address.addr_bytes));
    jint port = (*env)->GetIntField(env, inet_socket_address, cache.inet_socket_address.port);

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

    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));

    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons((short)port);
    (*env)->GetByteArrayRegion(env, j_addr, 0, 4, (jbyte *)&remote_addr.sin_addr.s_addr);

    int res = lwip_sendto(lwip_fd, buf + offset, len, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));

    if (res == 0) {
        throw_error(env, cache.java.eof_exception_klass, "EOF");
    } else if (res < 0) {
        lwip_error_throw_exc(env);
    }

    (*env)->ReleaseByteArrayElements(env, j_buf, buf, JNI_ABORT);

    return res;
}

JNIEXPORT jobject JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeGetLocal(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
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

JNIEXPORT jobject JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeGetRemote(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
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

JNIEXPORT jboolean JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeGetBroadcast(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    int broadcast;
    socklen_t len = sizeof(broadcast);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jboolean)(broadcast);
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeGetReceiveBufferSize(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    int buffer_size;
    socklen_t len = sizeof(buffer_size);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_RCVBUF, &buffer_size, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jint)(buffer_size);
}

JNIEXPORT jboolean JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeGetReuseAddress(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    int reuse;
    socklen_t len = sizeof(reuse);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jboolean)(reuse);
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeGetSendBufferSize(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    int buffer_size;
    socklen_t len = sizeof(buffer_size);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_SNDBUF, &buffer_size, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jint)(buffer_size);
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeGetSoTimeout(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    int timeout;
    socklen_t len = sizeof(timeout);
    int res = lwip_getsockopt(lwip_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jint)(timeout);
}

JNIEXPORT jint JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeGetTrafficClass(
        JNIEnv *env, jobject This) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    int class;
    socklen_t len = sizeof(class);
    int res = lwip_getsockopt(lwip_fd, IPPROTO_IP, IP_TOS, &class, &len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed get socket option");
    }

    return (jint)(class);
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeSetBroadcast(
        JNIEnv *env, jobject This, jboolean j_broadcast) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    int broadcast = (int)(j_broadcast);
    socklen_t len = sizeof(broadcast);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeSetReceiveBufferSize(
        JNIEnv *env, jobject This, jint size) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    socklen_t len = sizeof(size);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_RCVBUF, &size, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeSetReuseAddress(
        JNIEnv *env, jobject This, jboolean j_reuse) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    int reuse = (int)(j_reuse);
    socklen_t len = sizeof(reuse);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeSetSendBufferSize(
        JNIEnv *env, jobject This, jint size) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    socklen_t len = sizeof(size);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_SNDBUF, &size, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeSetSoTimeout(
        JNIEnv *env, jobject This, jint timeout) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    socklen_t len = sizeof(timeout);
    int res = lwip_setsockopt(lwip_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}

JNIEXPORT void JNICALL Java_org_quietmodem_Quiet_DatagramSocket_nativeSetTrafficClass(
        JNIEnv *env, jobject This, jint class) {
    jint lwip_fd = (*env)->GetIntField(env, This, cache.datagram.fd);
    socklen_t len = sizeof(class);
    int res = lwip_setsockopt(lwip_fd, IPPROTO_IP, IP_TOS, &class, len);

    if (res < 0) {
        throw_error(env, cache.java.socket_exception_klass, "failed set socket option");
    }
}
