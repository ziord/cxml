/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxgrptable.h"

#define     __init_grp_table(__table)                   \
    (__table)->capacity = (__table)->count = 0;         \
    cxml_list__init(&(__table)->keys);                  \
    (__table)->entries = NULL;


struct _cxml_gt_entry{
    const void *key;
    cxml_list group;
};

extern uint32_t _cxml__ptr_hash(const void* item);

extern void cxml_list__init(cxml_list *list);

static void _cxml_grp_table__put(
        cxml_grp_table *table,
        const void *key,
        void *value,
        bool order_key);



void cxml_grp_table_init(cxml_grp_table* table){
    if (!table) return;
    __init_grp_table(table)
}

cxml_grp_table new_cxml_grp_table(){
    cxml_grp_table table;
    __init_grp_table(&table)
    return table;
}

static int _cxml_grp_table_find_entry_index(cxml_grp_table* table, const void* key){
    uint32_t hash = _cxml__ptr_hash(key);
    int deleted = -1, index = (int) (hash & (table->capacity - 1));
    struct _cxml_gt_entry *entry;
    while (true){
        entry = &table->entries[index];
        if (entry->key == NULL)
        {
            if (cxml_list_is_empty(&entry->group)){ // free entry
                // if we found a deleted entry before a fresh unused entry,
                // we return the index to the deleted entry, if not, return the
                // index to this fresh unused entry.
                return deleted != -1 ? deleted : index;
            }
            // we found a deleted entry.
            // if `deleted` hasn't been set yet, then set it to this current index.
            else if (deleted == -1){
                deleted = index;
            }
        }
        else if (entry->key == key){
            return index;
        }
        index = (index + 1) & (table->capacity - 1);
    }
}

static void _cxml_grp_table_rehash(cxml_grp_table *table) {
    int old_capacity = table->capacity;  /*old capacity*/
    struct _cxml_gt_entry *old_entries = table->entries;
    int new_capacity;
    // reuse existing capacity if actual table count/usage is less than or equal to
    // 60% (_CXML_HT_LOAD_FACTOR_AC), this minimizes memory wastage.
    if ((cxml_list_size(&table->keys) + 1) <= (_CXML_HT_LOAD_FACTOR_AC * table->capacity)){
        // re-use capacity
        new_capacity = (table->capacity == 0) ? _CXML_HT_GROW_T_CAP(table->capacity)
                                              : table->capacity;
    }else{
        new_capacity = _CXML_HT_GROW_T_CAP(table->capacity);
    }
    table->capacity = new_capacity;
    table->entries = CALLOC(struct _cxml_gt_entry, new_capacity);

    if (table->count) {  // does the table contain any entry yet?
        struct _cxml_gt_entry *entry;
        table->count = 0;
        for (int i = 0; i < old_capacity; i++)
        {
            entry = &old_entries[i];
            if (entry->key != NULL) {
                cxml_for_each(val, &entry->group){
                    _cxml_grp_table__put(table, entry->key, val, false);
                }
                cxml_list_free(&entry->group);
            }
        }
        FREE(old_entries);
    }
}

static void _cxml_grp_table__put(
        cxml_grp_table *table,
        const void *key,
        void *value,
        bool order_key)
{
    if ((table->count + 1) >= (_CXML_HT_LOAD_FACTOR * table->capacity)){
        _cxml_grp_table_rehash(table);
    }
    int index = _cxml_grp_table_find_entry_index(table, key);
    if (table->entries[index].key == NULL){  // new insert operation
        // could be new entry or deleted entry
        bool is_unused_entry = cxml_list_is_empty(&table->entries[index].group);
        table->entries[index].key = key;
        cxml_list_append(&table->entries[index].group, value);
        // key shouldn't be ordered (added to the list) if it is already present.
        order_key ? cxml_list_append(&table->keys, (void*)key) : (void)0;
        // increment only if it's a new/unused entry
        is_unused_entry ? table->count++ : 0;
    }else{  // update operation
        cxml_list_append(&table->entries[index].group, value);
    }
}

void cxml_grp_table_put(
        cxml_grp_table *table,
        const void *key,
        void *value)
{
    if (table == NULL || key == NULL || value == NULL) return;
    _cxml_grp_table__put(table, key, value, true);
}

void cxml_grp_table_get(
        cxml_grp_table *table,
        const void *key,
        cxml_list *val)
{
    if (table == NULL || key == NULL || val == NULL) return;
    int index = _cxml_grp_table_find_entry_index(table, key);
    if (table->entries[index].key != NULL){
        // get() copies the items in the list into the list passed as an argument, instead
        // of handling over the original list, since that can be freed during re-hashing.
        cxml_list_extend(val, &table->entries[index].group);
    }
}

void cxml_grp_table_remove(cxml_grp_table *table, const void *key) {
    if (table == NULL || key == NULL) return;
    int index = _cxml_grp_table_find_entry_index(table, key);
    if (table->entries[index].key != NULL){
        // only "nullify" the key, leave the value field, this
        // indicates a tombstone or deleted entry.
        table->entries[index].key = NULL;
        cxml_list_search_delete(&table->keys, cxml_list_cmp_raw_items, (void*)key);

        // no need to reduce the count, since this helps in rehashing the table.
        // we minimize memory wastage using the actual count to measure
        // what would be allocated (see _cxml_grp_table_rehash()).
    }
}

bool cxml_grp_table_is_empty(cxml_grp_table *table){
    return (!table) || cxml_list_size(&table->keys) == 0;
}

int cxml_grp_table_size(cxml_grp_table *table){
    if (!table) return 0;
    return cxml_list_size(&table->keys);
}

void cxml_grp_table_free(cxml_grp_table *table) {
    struct _cxml_gt_entry *entry;
    for (int i=0; i<table->capacity; i++){
        entry = &table->entries[i];
        // free all lists
        if (cxml_list_size(&entry->group)) {
            cxml_list_free(&entry->group);
        }
    }
    FREE(table->entries);
    cxml_list_free(&table->keys);
    __init_grp_table(table)
}
