#include "quiet-jni.h"

static const size_t ipv4_len = (4 * 3) + 3 + 1;  // 4 octets and 3 dots and a nul

JNIEXPORT jstring JNICALL Java_org_quietmodem_Quiet_InetAddress_byte2str(
        JNIEnv *env, jclass inet_klass, jbyteArray addr) {

    jbyte *buf = (*env)->GetByteArrayElements(env, addr, NULL);
    size_t buf_len = (*env)->GetArrayLength(env, addr);
    char *addr_s = malloc(ipv4_len);
    jstring ret = NULL;

    if (buf_len != 4) {
        (*env)->ThrowNew(env, cache.java.unknown_host_exception_klass, "IPv4 address must be 4 bytes in length");
        goto inet_address_byte2str_dealloc;
    }
    ip_addr_t ip_addr;
    memcpy(&ip_addr.addr, buf, 4);
    char *res = inet_ntoa_r(ip_addr, addr_s, ipv4_len);

    if (!res) {
        (*env)->ThrowNew(env, cache.java.unknown_host_exception_klass, "Could not convert address to string");
        goto inet_address_byte2str_dealloc;
    }

    ret = (*env)->NewStringUTF(env, addr_s);
inet_address_byte2str_dealloc:
    (*env)->ReleaseByteArrayElements(env, addr, buf, JNI_ABORT);
    free(addr_s);
    return ret;
}

JNIEXPORT jbyteArray JNICALL Java_org_quietmodem_Quiet_InetAddress_str2byte(
        JNIEnv *env, jclass inet_klass, jstring j_query) {

    const char *query = (*env)->GetStringUTFChars(env, j_query, 0);
    jbyteArray dest = (*env)->NewByteArray(env, 4);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    struct addrinfo *ret;
    int error = lwip_getaddrinfo(query, NULL, &hints, &ret);

    if (error) {
        (*env)->ThrowNew(env, cache.java.unknown_host_exception_klass, "Address could not be resolved");
        goto inet_address_str2byte_dealloc;
    }

    // just take the first result since lwip only returns one
    if (ret[0].ai_addrlen == sizeof(struct sockaddr_in)) {
        struct sockaddr_in addr = *(struct sockaddr_in *)(ret[0].ai_addr);
        (*env)->SetByteArrayRegion(env, dest, 0, 4, (jbyte *)(&addr.sin_addr.s_addr));
    }

    lwip_freeaddrinfo(ret);
inet_address_str2byte_dealloc:
    (*env)->ReleaseStringUTFChars(env, j_query, query);
    return dest;

}
