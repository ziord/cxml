/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXLRUCACHE_H
#define CXML_CXLRUCACHE_H

#include "cxtable.h"

/*
 * Simple LRU Cache
 */
#define CX_CACHE_MAX_SIZE   (11)
#define voidstr__cast       (void*)

typedef struct{
    cxml_table cache;
}_cxml_lru_cache;

int _cxml_cache_size(_cxml_lru_cache *cache);

void _cxml_cache_init(_cxml_lru_cache *cache);

void *_cxml_cache_put(_cxml_lru_cache *lru_cache, const void *key, void *data);  // _cxml_obj

void *_cxml_cache_get(_cxml_lru_cache *lru_cache, const void *key);

void _cxml_cache_free(_cxml_lru_cache *cache);
#endif //CXML_CXLRUCACHE_H
