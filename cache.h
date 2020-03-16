#ifndef CACHE_H
#define	CACHE_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <string.h>
#include <uthash.h>

    struct CacheEntry {
        char *session_id;
        char *domain;
        UT_hash_handle hh;
    };
    
#ifdef	__cplusplus
}
#endif

#endif	/* CACHE_H */

