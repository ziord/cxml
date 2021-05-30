/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXSCOPE_H
#define CXML_CXSCOPE_H

#include "core/cxtable.h"

struct _cxml_scope_table{
    /* symbols/items in the scope */
    cxml_table symbols;
    /* outer/enclosing scope */
    struct _cxml_scope_table *enclosing_scope;
};

void _cxml_scope_table_init(struct _cxml_scope_table *table);

void _cxml_scope_table_free(struct _cxml_scope_table *scope);

struct _cxml_scope_table * _cxml_scope_table_new();

void* _cxml_scope_table_lookup(struct _cxml_scope_table *table, const char *name);

int _cxml_scope_table_insert(struct _cxml_scope_table *table, const char *name, void *symbol);

#endif //CXML_CXSCOPE_H
