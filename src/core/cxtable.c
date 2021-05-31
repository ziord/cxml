/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxtable.h"

// cxml hashtable implementation using open addressing for handling hash-collisions


#define __init_table(__table)               \
    (__table)->count = 0;                   \
    (__table)->capacity = 0;                \
    (__table)->entries = NULL;              \
    cxml_list__init(&(__table)->keys);

extern void cxml_list__init(cxml_list *list);

static int _cxml_table__put(
        cxml_table *table,
        const void *key,
        void *value,
        bool order_key,
        _cxml_table_hash_t hash_type);



void cxml_table_init(cxml_table *table) {
    if (!table) return;
    __init_table(table)
}

cxml_table new_cxml_table(){
    cxml_table table;
    __init_table(&table)
    return table;
}

cxml_table *new_alloc_cxml_table(){
    cxml_table *table = ALLOC(cxml_table, 1);
    __init_table(table)
    return table;
}

uint32_t _cxml__ptr_hash(const void* item){
    unsigned long tmp = (unsigned long)item;
    tmp ^= (tmp >> 20) ^ (tmp >> 12);
    return tmp ^ (tmp >> 7) ^ (tmp >> 4);
}

inline static uint32_t _cxml_table_hash(const char *key) {
    //uses FNV-1a hashing algorithm
    uint32_t hash = 2166136261;
    uint32_t fnv_prime = 16777619;
    for (int i = 0; key[i] != '\0'; i++) {
        hash = hash ^ (*(key + i));
        hash = hash * fnv_prime;
    }
    return hash;
}

static int _cxml_table_find_entry_index(
        cxml_table* table,
        const void* key,
        const _cxml_table_hash_t hash_type)
{
    uint32_t hash = hash_type == CXML_TABLE_HASH_STRING ?
                    _cxml_table_hash(key) : _cxml__ptr_hash(key);
    int deleted = -1, index = (int) (hash & (table->capacity - 1));
    _cxml_ht_entry *entry;
    while (true){
        entry = &table->entries[index];
        if (entry->key == NULL)
        {
            if (entry->value == NULL){ // free entry
                // if we found a deleted entry before a fresh untaken entry,
                // we return the index to the deleted entry, if not, return the
                // index to this fresh untaken entry.
                return deleted != -1 ? deleted : index;
            }
            // we found a deleted entry.
            // if `deleted` hasn't been set yet, then set it to this current index.
            else if (deleted == -1){
                deleted = index;
            }
        }
        else if (entry->key == key
                || ((hash_type == CXML_TABLE_HASH_STRING)
                    && (strcmp(entry->key, key) == 0)))
        {
            return index;
        }
        index = (index + 1) & (table->capacity - 1);
    }
}

static void _cxml_table_rehash(cxml_table *table, _cxml_table_hash_t hash_type) {
    int old_capacity = table->capacity;  /*old capacity*/
    _cxml_ht_entry *old_entries = table->entries;
    int new_capacity;
    // reuse existing capacity if actual table count/usage is less than or equal to
    // 60% (_CXML_HT_LOAD_FACTOR_AC), this minimizes memory wastage.
    if ((table->keys.len + 1) <= (_CXML_HT_LOAD_FACTOR_AC * table->capacity)){
        // re-use capacity
        new_capacity = (table->capacity == 0) ? _CXML_HT_GROW_T_CAP(table->capacity)
                                              : table->capacity;
    }else{
        new_capacity = _CXML_HT_GROW_T_CAP(table->capacity);
    }
    table->capacity = new_capacity;
    table->entries = CALLOC(_cxml_ht_entry, new_capacity);

    if (table->count) {  // does the table contain any entry yet?
        _cxml_ht_entry *entry;
        table->count = 0;  // set count to 0 to prevent insertion compromise
        for (int i = 0; i < old_capacity; i++)
        {
            entry = &old_entries[i];
            if (entry->key != NULL) {
                _cxml_table__put(table, entry->key, entry->value, false, hash_type);
            }
        }
        FREE(old_entries);
    }
}

// return 2 on update insert, 1 on new insert, 0 on failed insert
int _cxml_table__put(
        cxml_table *table,
        const void *key,
        void *value,
        bool order_key,
        _cxml_table_hash_t hash_type)
{
    if ((table->count + 1) >= (_CXML_HT_LOAD_FACTOR * table->capacity)){
        _cxml_table_rehash(table, hash_type);
    }
    int index = _cxml_table_find_entry_index(table, key, hash_type);
    if (table->entries[index].key == NULL){  // new insert operation
        // could be new entry or deleted entry
        bool is_unused_entry = table->entries[index].value == NULL;
        table->entries[index].key = key;
        table->entries[index].value = value;
        // key shouldn't be ordered (added to the list) if it is already present.
        order_key ? cxml_list_append(&table->keys, (void*)key) : (void)0;
        // increment only if it's a new/unused entry
        is_unused_entry ? table->count++ : 0;
        return 1;
    }else{  // update operation
        table->entries[index].value = value;
        return 2;
    }
}

/*
 * extra care should be taken when using the functions: cxml_table_put(), and
 * cxml_table_put_raw().
 * Only one of these functions must be used with a particular table throughout
 * the table's lifetime.
 * Mixed usage is not advised.
 */
int cxml_table_put(
        cxml_table *table,
        const char *key,
        void *value)
{
    if (table == NULL || key == NULL || value == NULL) return 0;
    return _cxml_table__put(table, key, value, true, CXML_TABLE_HASH_STRING);
}

int cxml_table_put_raw(
        cxml_table *table,
        const void *key,
        void *value)
{
    if (table == NULL || key == NULL || value == NULL) return 0;
    return _cxml_table__put(table, key, value, true, CXML_TABLE_HASH_RAW_PTR);
}

void* cxml_table_get(cxml_table *table, const char *key){
    if (table == NULL || key == NULL || cxml_table_is_empty(table)) return NULL;
    int index = _cxml_table_find_entry_index(table, key, CXML_TABLE_HASH_STRING);
    if (table->entries[index].key != NULL){
        return table->entries[index].value;
    }
    return NULL;
}

void* cxml_table_get_raw(cxml_table *table, const void *key){
    if (table == NULL || key == NULL || cxml_table_is_empty(table)) return NULL;
    int index = _cxml_table_find_entry_index(table, key, CXML_TABLE_HASH_RAW_PTR);
    if (table->entries[index].key != NULL){
        return table->entries[index].value;
    }
    return NULL;
}

inline static void _cxml_table__remove(
        cxml_table *table,
        const void *key,
        _cxml_table_hash_t hash_type)
{
    int index = _cxml_table_find_entry_index(table, key, hash_type);
    if (table->entries[index].key != NULL){
        // only "nullify" the key, leave the value field, this
        // indicates a tombstone or deleted entry.
        table->entries[index].key = NULL;
        cxml_list_search_delete(&table->keys,
                                hash_type == CXML_TABLE_HASH_STRING
                                   ? cxml_list_cmp_str_items
                                   : cxml_list_cmp_raw_items,
                                (void*)key);

        // no need to reduce the count, since this helps in rehashing the table.
        // we minimize memory wastage using the actual count to measure
        // what would be allocated (see _cxml_table_rehash()).
    }
}

void cxml_table_remove(cxml_table *table, const char *key){
    if (table == NULL || key == NULL) return;
    _cxml_table__remove(table, key, CXML_TABLE_HASH_STRING);
}

void cxml_table_remove_raw(cxml_table *table, const void *key){
    if (table == NULL || key == NULL) return;
    _cxml_table__remove(table, key, CXML_TABLE_HASH_RAW_PTR);
}

bool cxml_table_is_empty(cxml_table *table){
    return (!table) || cxml_list_size(&table->keys) == 0;
}

int cxml_table_size(cxml_table *table){
    if (!table) return 0;
    return cxml_list_size(&table->keys);
}

void cxml_table_free(cxml_table *table) {
    if (table->capacity > 0) {
        FREE(table->entries);
    }
    cxml_list_free(&table->keys);
    __init_table(table)
}
