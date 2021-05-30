/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXDTABLE_H
#define CXML_CXDTABLE_H

#include "cxtable.h"


typedef struct{
    int capacity;
    int count;
    struct _cxml_gt_entry* entries;
    cxml_list keys;
}cxml_grp_table;

void cxml_grp_table_init(cxml_grp_table *table);

cxml_grp_table new_cxml_grp_table();

void cxml_grp_table_put(cxml_grp_table *table, const void *key, void *value);

void cxml_grp_table_get(cxml_grp_table *table, const void *key, cxml_list *val);

void cxml_grp_table_remove(cxml_grp_table *table, const void *key);

bool cxml_grp_table_is_empty(cxml_grp_table *table);

int cxml_grp_table_size(cxml_grp_table *table);

void cxml_grp_table_free(cxml_grp_table *table);

#endif //CXML_CXDTABLE_H
