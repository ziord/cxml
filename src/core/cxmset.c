/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxmset.h"
/*
 * simplistic set implementation for storing xpath nodes supporting only set addition
 */

struct _cxml_set_entry{
    uint32_t hash;
    const void* item;
};

#define __init_mset(__set)                      \
    (__set)->entries = NULL;                    \
    cxml_list__init(&(__set)->items);           \
    (__set)->size = (__set)->capacity = 0;

extern void cxml_list__init(cxml_list *list);

extern uint32_t _cxml__ptr_hash(const void* item);

void _cxml_set__add(cxml_set *mset, const void *item, bool order_key);


void cxml_set_init(cxml_set* mset){
    if (!mset) return;
    __init_mset(mset)
}

cxml_set new_cxml_set(){
    cxml_set mset;
    __init_mset(&mset)
    return mset;
}

cxml_set* new_alloc_cxml_set(){
    cxml_set *mset = ALLOC(cxml_set, 1);
    __init_mset(mset);
    return mset;
}

static int _cxml_set_find_entry_index(cxml_set* mset, const void* key, const uint32_t *_hash){
    uint32_t hash = _hash ? *_hash : _cxml__ptr_hash(key);
    int deleted = -1, index = (int) (hash & (mset->capacity - 1));
    struct _cxml_set_entry *entry;
    while (true){
        entry = &mset->entries[index];
        if (entry->hash == 0)
        {
            if (entry->item == NULL){ // free entry
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
        else if (entry->item == key)
        {
            return index;
        }
        index = (index + 1) & (mset->capacity - 1);
    }
}

static void _cxml_set_rehash(cxml_set *mset) {
    int old_capacity = mset->capacity;  /*old capacity*/
    struct _cxml_set_entry *old_entries = mset->entries;
    int new_capacity;
    // reuse existing capacity if actual mset count/usage is less than or equal to
    // 60% (_CXML_HT_LOAD_FACTOR_AC), this minimizes memory wastage.
    if ((cxml_list_size(&mset->items) + 1) <= (_CXML_HT_LOAD_FACTOR_AC * mset->capacity)){
        // re-use capacity
        new_capacity = (mset->capacity == 0) ? _CXML_HT_GROW_T_CAP(mset->capacity) : mset->capacity;
    }else {
        new_capacity = _CXML_HT_GROW_T_CAP(mset->capacity);
    }
    mset->capacity = new_capacity;
    mset->entries = CALLOC(struct _cxml_set_entry, new_capacity);

    if (cxml_list_size(&mset->items)) {  // does the mset contain any entry yet?
        mset->size = 0;
        for (int i = 0; i < old_capacity; i++)
        {
            // entry = &old_entries[i];
            if (old_entries[i].hash != 0) {
                _cxml_set__add(mset, old_entries[i].item, false);
            }
        }
    }
    FREE(old_entries);
}

void _cxml_set__add(cxml_set *mset, const void *item, bool order_key){
    if ((mset->size + 1) >= (_CXML_HT_LOAD_FACTOR * mset->capacity)){
        _cxml_set_rehash(mset);
    }
    uint32_t hash = _cxml__ptr_hash(item);
    int index = _cxml_set_find_entry_index(mset, item, &hash);
    if (mset->entries[index].hash == 0){  // new insert operation
        // could be new entry or deleted entry
        bool is_unused_entry = mset->entries[index].item == NULL;
        mset->entries[index].hash = hash;
        mset->entries[index].item = item;
        // key shouldn't be ordered (added to the list) if it is already present.
        order_key ? cxml_list_append(&mset->items, (void*)item) : (void)0;
        // increment only if it's a new/unused entry
        is_unused_entry ? mset->size++ : 0;
    }
}

void cxml_set_add(cxml_set *mset, const void *item){
    if (mset == NULL || item == NULL) return;
    _cxml_set__add(mset, item, true);
}

void* cxml_set_get(cxml_set* mset, int index){
    if (mset == NULL) return NULL;
    // get items stored in the set by index
    return cxml_list_get(&mset->items, index);
}

void cxml_set_remove(cxml_set *mset, const void *item){
    if (mset == NULL || item == NULL) return;
    int index = _cxml_set_find_entry_index(mset, item, NULL);
    if (mset->entries[index].hash != 0){
        // only "nullify" the hash, leave the value field, this
        // indicates a tombstone or deleted entry.
        mset->entries[index].hash = 0;
        cxml_list_search_delete(&mset->items, cxml_list_cmp_raw_items, (void*)item);

        // no need to reduce the count, since this helps in rehashing the table.
        // we minimize memory wastage using the actual count to measure
        // what would be allocated (see _cxml_set_rehash()).
    }
}

// expects `rec` to be empty
void cxml_set_copy(cxml_set *rec, cxml_set *giv){
    if (!rec || !giv) return;
    __init_mset(rec);
    cxml_for_each(item, &giv->items){
        cxml_set_add(rec, item);
    }
}

void cxml_set_extend(cxml_set *rec, cxml_set *giv){
    if (!rec || !giv) return;
    cxml_for_each(item, &giv->items){
        cxml_set_add(rec, item);
    }
}

void cxml_set_extend_list(cxml_set *set, cxml_list *list){
    if (!set || !list) return;
    cxml_for_each(item, list){
        cxml_set_add(set, item);
    }
}

int cxml_set_size(cxml_set* mset){
    if (!mset) return 0;
    return cxml_list_size(&mset->items);
}

bool cxml_set_is_empty(cxml_set* mset){
    return (!mset) || cxml_list_size(&mset->items) == 0;
}

void cxml_set_free(cxml_set* mset){
    if (!mset) return;
    if (mset->capacity){
        cxml_list_free(&mset->items);
        FREE(mset->entries);
    }
    __init_mset(mset);
}

void cxml_set__init_with(cxml_set* recipient, cxml_set* donor){
    recipient->capacity = donor->capacity;
    recipient->size = donor->size;
    recipient->entries = donor->entries;
    cxml_list_init_with(&recipient->items, &donor->items);
    __init_mset(donor);
}

void cxml_set_init_with(cxml_set* recipient, cxml_set* donor){
    if (!recipient || !donor) return;
    recipient->capacity = donor->capacity;
    recipient->size = donor->size;
    recipient->entries = donor->entries;
    cxml_list_init_with(&recipient->items, &donor->items);
    __init_mset(donor);
}
