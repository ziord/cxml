/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxlrucache.h"

int _cxml_cache_size(_cxml_lru_cache* cache){
    return cxml_table_size(&cache->cache);
}

void _cxml_cache_init(_cxml_lru_cache* cache){
    cxml_table_init(&cache->cache);
}

static void* _cx_get_lr_added(_cxml_lru_cache* lru_cache){
    // get the least recently added item in the cached nodes
    return cxml_list_get(&lru_cache->cache.keys, 0);
}

void* _cxml_cache_put(_cxml_lru_cache* lru_cache, const void* key, void* data){
    if (_cxml_cache_size(lru_cache) >= CX_CACHE_MAX_SIZE){
        void* removed_key = _cx_get_lr_added(lru_cache);
        void* removed_val = cxml_table_get_raw(&lru_cache->cache, removed_key);
        cxml_table_remove_raw(&lru_cache->cache, removed_key);
        cxml_table_put_raw(&lru_cache->cache, key, data);
        return removed_val;
    }
    cxml_table_put_raw(&lru_cache->cache, key, data);
    return NULL;
}

void* _cxml_cache_get(_cxml_lru_cache* lru_cache, const void* key){
    void* result = cxml_table_get_raw(&lru_cache->cache, key);
    if (result){
        if ((voidstr__cast key) == cxml_list_last(&lru_cache->cache.keys))
        {
            // is key the last added item?
            // no need to push key to most recently accessed position since it's already there
            return result;
        }
        // remove from lru_cache->keys, and append to last to make `key` most recently accessed.
        cxml_list_search_delete(&lru_cache->cache.keys, cxml_list_cmp_raw_items, voidstr__cast key);
        cxml_list_append(&lru_cache->cache.keys, voidstr__cast key);
    }
    return result;
}

void _cxml_cache_free(_cxml_lru_cache* cache){
    // we have no business freeing cached nodes here.
    cxml_table_free(&cache->cache);
}
