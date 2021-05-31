/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

// no seg-fault tests, because lru-cache is an internal structure with
// very precise use-case, and not meant to be used by external users.

int empty_lrucache_asserts(_cxml_lru_cache *cache){
    cxml_assert__zero(cache->cache.capacity)
    cxml_assert__zero(cache->cache.count)
    cxml_assert__null(cache->cache.entries)
    cxml_assert__true(cxml_list_is_empty(&cache->cache.keys))
    return 1;
}

cts test__cxml_cache_size(){
    struct Data k, v;
    _cxml_lru_cache cache;
    _cxml_cache_init(&cache);

    _cxml_cache_put(&cache, &k, &v);
    cxml_assert__one(_cxml_cache_size(&cache))

    _cxml_cache_put(&cache, &k, &v);
    cxml_assert__one(_cxml_cache_size(&cache))

    _cxml_cache_put(&cache, &v, &v);
    cxml_assert__two(_cxml_cache_size(&cache))

    _cxml_cache_put(&cache, &v, &v);
    cxml_assert__two(_cxml_cache_size(&cache))

    _cxml_cache_free(&cache);

    cxml_pass()
}

cts test__cxml_cache_init(){
    _cxml_lru_cache cache;
    _cxml_cache_init(&cache);
    cxml_assert__one(empty_lrucache_asserts(&cache))
    cxml_pass()
}

cts test__cxml_cache_put(){
    struct Data k, k2, k3, v, v2, v3;
    _cxml_lru_cache cache;
    _cxml_cache_init(&cache);

    _cxml_cache_put(&cache, &k, &v);
    _cxml_cache_put(&cache, &k2, &v2);
    cxml_assert__two(_cxml_cache_size(&cache))
    cxml_assert__eq(cxml_list_first(&cache.cache.keys), &k)
    cxml_assert__eq(cxml_list_last(&cache.cache.keys), &k2)

    _cxml_cache_put(&cache, &k3, &v3);
    cxml_assert__eq(cxml_list_last(&cache.cache.keys), &k3)
    cxml_assert__eq(_cxml_cache_size(&cache), 3)

    _cxml_cache_free(&cache);
    cxml_pass()
}

cts test__cxml_cache_get(){
    struct Data k, k2, k3, v, v2, v3;
    _cxml_lru_cache cache;
    _cxml_cache_init(&cache);

    _cxml_cache_put(&cache, &k, &v);
    _cxml_cache_put(&cache, &k2, &v2);

    cxml_assert__eq(cxml_list_first(&cache.cache.keys), &k)

    void *d = _cxml_cache_get(&cache, &k);
    cxml_assert__eq(d, &v)
    // &k is made recently accessed item.
    cxml_assert__eq(cxml_list_last(&cache.cache.keys), &k)

    _cxml_cache_put(&cache, &k3, &v3);
    cxml_assert__eq(cxml_list_last(&cache.cache.keys), &k3)

    d = _cxml_cache_get(&cache, &k2);
    cxml_assert__eq(d, &v2)
    // &k2 is made recently accessed item.
    cxml_assert__eq(cxml_list_last(&cache.cache.keys), &k2)
    // &k is currently at the top of the underlying list,
    // since it's the least recently used
    cxml_assert__eq(cxml_list_first(&cache.cache.keys), &k)
    _cxml_cache_free(&cache);

    cxml_pass()
}

cts test__cxml_cache_free(){
    struct Data k, v;
    _cxml_lru_cache cache;
    _cxml_cache_init(&cache);

    _cxml_cache_put(&cache, &k, &v);
    cxml_assert__one(_cxml_cache_size(&cache))

    _cxml_cache_put(&cache, &v, &k);
    cxml_assert__two(_cxml_cache_size(&cache))
    _cxml_cache_free(&cache);

    cxml_assert__one(empty_lrucache_asserts(&cache));
    cxml_pass()
}


void suite_cxlrucache(){
    cxml_suite(cxstack)
    {
        cxml_add_m_test(5,
                        test__cxml_cache_size,
                        test__cxml_cache_init,
                        test__cxml_cache_put,
                        test__cxml_cache_get,
                        test__cxml_cache_free
        )
        cxml_run_suite()
    }
}
