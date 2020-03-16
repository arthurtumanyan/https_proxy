#ifndef TLS_H
#define	TLS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/socket.h>
#include <errno.h>
#include "externals.h"    

#define SERVER_NAME_LEN 256
#define TLS_HEADER_LEN 5
#define TLS_HANDSHAKE_CONTENT_TYPE 0x16
#define TLS_HANDSHAKE_TYPE_CLIENT_HELLO 0x01
#define TLS_HANDSHAKE_TYPE_SERVER_HELLO 0x02
#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* TLS_H */

