#include "tls.h"

char * get_server_session_id(const char *data, size_t data_len) {
    char tls_content_type;
    char tls_version_major;
    size_t pos = TLS_HEADER_LEN;
    size_t len;

    /* Check that our TCP payload is at least large enough for a TLS header */
    if (data_len < TLS_HEADER_LEN) {
        return NULL;
    }
    if (data[0] & 0x80 && data[2] == 1) {
        return NULL;
    }
    tls_content_type = data[0];
    if (tls_content_type != TLS_HANDSHAKE_CONTENT_TYPE) {
        return NULL;
    }
    tls_version_major = data[1];
    if (tls_version_major < 3) {
        return NULL;
    }
    /* TLS record length */
    len = ((unsigned char) data[3] << 8) +
            (unsigned char) data[4] + TLS_HEADER_LEN;
    data_len = MIN(data_len, len);

    if (pos + 1 > data_len) {
        return NULL;
    }
    if (data[pos] == TLS_HANDSHAKE_TYPE_SERVER_HELLO) {
        pos += 38;
        static char session_id[64];
        bzero(session_id, 64);
        char s[2];
        int len = data[pos];
        int i;
        if (data[pos] != 0x00) {
            for (i = 0; i < len; i++) {
                bzero(s, sizeof (s));
                sprintf(s, "%02x", (unsigned char) data[++pos]);
                strncat(session_id, s, 2);
            }
        }
        return &session_id[0];
    }
    return NULL;
}

char *get_client_session_id(char *data, size_t data_len) {
    char tls_content_type;
    char tls_version_major;
    size_t pos = TLS_HEADER_LEN;
    size_t len;

    /* Check that our TCP payload is at least large enough for a TLS header */
    if (data_len < TLS_HEADER_LEN) {
        return NULL;
    }
    if (data[0] & 0x80 && data[2] == 1) {
        return NULL;
    }
    tls_content_type = data[0];
    if (tls_content_type != TLS_HANDSHAKE_CONTENT_TYPE) {
        return NULL;
    }
    tls_version_major = data[1];
    if (tls_version_major < 3) {
        return NULL;
    }
    /* TLS record length */
    len = ((unsigned char) data[3] << 8) +
            (unsigned char) data[4] + TLS_HEADER_LEN;
    data_len = MIN(data_len, len);

    if (pos + 1 > data_len) {
        return NULL;
    }
    if (data[pos] == TLS_HANDSHAKE_TYPE_CLIENT_HELLO) {
        pos += 38;
        static char session_id[64];
        bzero(session_id, 64);
        char s[2];
        int len = data[pos];
        int i;
        if (data[pos] != 0x00) {
            for (i = 0; i < len; i++) {
                bzero(s, sizeof (s));
                sprintf(s, "%02x", (unsigned char) data[++pos]);
                strncat(session_id, s, 2);
            }
        }
        return &session_id[0];
    }
    return NULL;
}

void clear_session_id(char *data, size_t data_len) {
    char tls_content_type;
    char tls_version_major;
    size_t pos = TLS_HEADER_LEN;
    size_t len;

    /* Check that our TCP payload is at least large enough for a TLS header */
    if (data_len < TLS_HEADER_LEN) {
        return;
    }
    if (data[0] & 0x80 && data[2] == 1) {
        return;
    }
    tls_content_type = data[0];
    if (tls_content_type != TLS_HANDSHAKE_CONTENT_TYPE) {
        return;
    }
    tls_version_major = data[1];
    if (tls_version_major < 3) {
        return;
    }
    /* TLS record length */
    len = ((unsigned char) data[3] << 8) +
            (unsigned char) data[4] + TLS_HEADER_LEN;
    data_len = MIN(data_len, len);

    if (pos + 1 > data_len) {
        return;
    }
    if (data[pos] == TLS_HANDSHAKE_TYPE_CLIENT_HELLO) {
        pos += 38;
        int len = data[pos];
        int i;
        if (data[pos] != 0x00) {
            data[pos] = 0x00;
            for (i = 0; i < len; i++) {
                data[++pos] = 0x00;
            }
        }
    }
}
