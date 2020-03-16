#include "cache.h"
#include "externals.h"
#include "config.h"

struct CacheEntry *cache = NULL;

void add_cache_element(char *sid, char *dn) {
    struct CacheEntry *entry, *tmp_entry;
    entry = malloc(sizeof (struct CacheEntry));
    entry->session_id = strdup(sid);
    entry->domain = strdup(dn);
    HASH_ADD_KEYPTR(hh, cache, entry->session_id, strlen(entry->session_id), entry);

    if (HASH_COUNT(cache) >= (unsigned int) Config.maxcon) {
        HASH_ITER(hh, cache, entry, tmp_entry) {
            HASH_DELETE(hh, cache, entry);
            free(entry->session_id);
            free(entry->domain);
            free(entry);
            break;
        }
    }
}

char *get_cache_element(char *sid) {
    struct CacheEntry *entry;
    HASH_FIND_STR(cache, sid, entry);
    if (entry) {
        HASH_DELETE(hh, cache, entry);
        HASH_ADD_KEYPTR(hh, cache, entry->session_id, strlen(entry->session_id), entry);
        return entry->domain;
    }
    return NULL;
}

void iterate_cache_elements() {
    struct CacheEntry *s;
    int i = 1;
    for (s = cache; s != NULL; s = s->hh.next) {
        printf("%d: %s = %s\n", i, s->session_id, s->domain);
        i++;
    }
}