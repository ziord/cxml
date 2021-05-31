/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXTABLE_H
#define CXML_CXTABLE_H
#include "cxlist.h"

#define _CXML_HT_INIT_T_CAP         (8)
#define _CXML_HT_GROW_T_CAP(cap)    (cap >= _CXML_HT_INIT_T_CAP ? cap << 1 : _CXML_HT_INIT_T_CAP)
#define _CXML_HT_LOAD_FACTOR        (0.75)
#define _CXML_HT_LOAD_FACTOR_AC     (0.6)

typedef enum {
    CXML_TABLE_HASH_RAW_PTR,
    CXML_TABLE_HASH_STRING
}_cxml_table_hash_t;

typedef struct {
    const char *key;
    void *value;
} _cxml_ht_entry;

typedef struct {
    int count;
    int capacity;
    _cxml_ht_entry *entries;    // store key-value pairs in the table
    cxml_list keys;             // store all keys being added to the table
} cxml_table;

void cxml_table_free(cxml_table *table);

cxml_table new_cxml_table();

cxml_table *new_alloc_cxml_table();

void cxml_table_init(cxml_table *);

int cxml_table_put(cxml_table *table, const char *key, void *value);

int cxml_table_put_raw(cxml_table *set, const void *key, void *value);

void cxml_table_remove(cxml_table *table, const char *key);

void cxml_table_remove_raw(cxml_table *table, const void *key);

void *cxml_table_get(cxml_table *table, const char *key);

void* cxml_table_get_raw(cxml_table *table, const void *key);

bool cxml_table_is_empty(cxml_table *table);

int cxml_table_size(cxml_table *table);

#endif //CXML_CXTABLE_H
