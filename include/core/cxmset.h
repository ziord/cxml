/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXMSET_H
#define CXML_CXMSET_H

#include "cxtable.h"

typedef struct _cxml_set{
    int size;
    int capacity;
    struct _cxml_set_entry* entries;
    cxml_list items;
}cxml_set;


cxml_set new_cxml_set();

cxml_set* new_alloc_cxml_set();

void cxml_set_init(cxml_set *mset);

void cxml_set_add(cxml_set *mset, const void *item);

void *cxml_set_get(cxml_set *mset, int index);

void cxml_set_remove(cxml_set *mset, const void *item);

void cxml_set_copy(cxml_set *rec, cxml_set *giv);

void cxml_set_extend(cxml_set *rec, cxml_set *giv);

void cxml_set_extend_list(cxml_set *set, cxml_list *list);

int cxml_set_size(cxml_set *mset);

bool cxml_set_is_empty(cxml_set *mset);

void cxml_set_free(cxml_set *mset);

void cxml_set_init_with(cxml_set* recipient, cxml_set* donor);

#endif //CXML_CXMSET_H
