#include "tls.h"

/* Parse a TLS packet for the Server Name Indication extension in the client
 * hello handshake, returning the first servername found (pointer to static
 * array)
 *
 * Returns:
 * >=0 - length of the hostname and updates *hostname
 * caller is responsible for freeing *hostname
 * -1 - Incomplete request
 * -2 - No Host header included in this request
 * -3 - Invalid hostname pointer
 * -4 - Malloc failure
 * < -4 - Invalid TLS client hello
 */
int tls_fetch_domain(const char *data, size_t data_len, char *hostname) {
    char tls_content_type;
    char tls_version_major;
    char tls_version_minor;
    size_t pos = TLS_HEADER_LEN;
    size_t len;
    if (hostname == NULL)
        return -3;
    /* Check that our TCP payload is at least large enough for a TLS header */
    if (data_len < TLS_HEADER_LEN)
        return -1;
    /* SSL 2.0 compatible Client Hello
     *
     * High bit of first byte (length) and content type is Client Hello
     *
     * See RFC5246 Appendix E.2
     */
    if (data[0] & 0x80 && data[2] == 1) {
        logger(1, "%s: Received SSL 2.0 Client Hello which can not support SNI.", __FUNCTION__);
        return -2;
    }
    tls_content_type = data[0];
    if (tls_content_type != TLS_HANDSHAKE_CONTENT_TYPE) {
        logger(1, "%s: Request did not begin with TLS handshake.", __FUNCTION__);
        return -5;
    }
    tls_version_major = data[1];
    tls_version_minor = data[2];
    if (tls_version_major < 3) {
        logger(1, "%s: Received SSL %d.%d handshake which which can not support SNI.", __FUNCTION__,
                tls_version_major, tls_version_minor);
        return -2;
    }
    /* TLS record length */
    len = ((unsigned char) data[3] << 8) +
            (unsigned char) data[4] + TLS_HEADER_LEN;
    data_len = MIN(data_len, len);
    /* Check we received entire TLS record length */
    if (data_len < len)
        return -1;
    /*
     * Handshake
     */
    if (pos + 1 > data_len) {
        return -5;
    }
    if (data[pos] != TLS_HANDSHAKE_TYPE_CLIENT_HELLO) {
        logger(1, "%s: Not a client hello", __FUNCTION__);
        return -5;
    }
    /* Skip past fixed length records:
    1 Handshake Type
    3 Length
    2 Version (again)
    32 Random
    to Session ID Length
     */
    pos += 38;

    /* Session ID */
    if (pos + 1 > data_len)
        return -5;
    len = (unsigned char) data[pos];
    pos += 1 + len;
    /* Cipher Suites */
    if (pos + 2 > data_len)
        return -5;
    len = ((unsigned char) data[pos] << 8) + (unsigned char) data[pos + 1];
    pos += 2 + len;
    /* Compression Methods */
    if (pos + 1 > data_len)
        return -5;
    len = (unsigned char) data[pos];
    pos += 1 + len;
    if (pos == data_len && tls_version_major == 3 && tls_version_minor == 0) {
        logger(1, "%s: Received SSL 3.0 handshake without extensions", __FUNCTION__);
        return -2;
    }
    /* Extensions */
    if (pos + 2 > data_len)
        return -5;
    len = ((unsigned char) data[pos] << 8) + (unsigned char) data[pos + 1];
    pos += 2;
    if (pos + len > data_len)
        return -5;
    return parse_extensions(data + pos, len, hostname);
}

int parse_extensions(const char *data, size_t data_len, char *hostname) {
    size_t pos = 0;
    size_t len;
    /* Parse each 4 bytes for the extension header */
    while (pos + 4 <= data_len) {
        /* Extension Length */
        len = ((unsigned char) data[pos + 2] << 8) +
                (unsigned char) data[pos + 3];
        /* Check if it's a server name extension */
        if (data[pos] == 0x00 && data[pos + 1] == 0x00) {
            if (pos + 4 + len > data_len)
                return -5;
            return parse_server_name_extension(data + pos + 4, len, hostname);
        }
        pos += 4 + len; /* Advance to the next extension header */
    }
    /* Check we ended where we expected to */
    if (pos != data_len)
        return -5;
    return -2;
}

int parse_server_name_extension(const char *data, size_t data_len, char *hostname) {
    size_t pos = 2; /* skip server name list length */
    size_t len;
    while (pos + 3 < data_len) {
        len = ((unsigned char) data[pos + 1] << 8) +
                (unsigned char) data[pos + 2];
        if (pos + 3 + len > data_len)
            return -5;
        switch (data[pos]) { /* name type */
            case 0x00: /* host_name */
                if (hostname == NULL) {
                    logger(0, "Error: %s", strerror(errno));
                    return -4;
                }
                strncpy(hostname, data + pos + 3, len);
                hostname[len] = '\0';
                return len;
            default:
                logger(1, "%s: Unknown server name extension name type: %d", __FUNCTION__,
                        data[pos]);
        }
        pos += 3 + len;
    }
    /* Check we ended where we expected to */
    if (pos != data_len)
        return -5;
    return -2;
}
