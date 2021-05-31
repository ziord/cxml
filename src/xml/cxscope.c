/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "xml/cxscope.h"

void _cxml_scope_table_init(struct _cxml_scope_table *table){
    table->enclosing_scope = NULL;
    cxml_table_init(&table->symbols);
}

void _cxml_scope_table_free(struct _cxml_scope_table *table){
    if (!table) return ;
    cxml_table_free(&table->symbols);
    FREE(table);
}

struct _cxml_scope_table * _cxml_scope_table_new(){
    struct _cxml_scope_table *scope = ALLOC(struct _cxml_scope_table, 1);
    _cxml_scope_table_init(scope);
    return scope;
}

void* _cxml_scope_table_lookup(struct _cxml_scope_table *table, const char *name){
    if (!table) return NULL;
    void *val = cxml_table_get(&table->symbols, name);
    if (val == NULL) return _cxml_scope_table_lookup(table->enclosing_scope, name);
    return val;
}

int _cxml_scope_table_insert(struct _cxml_scope_table *table, const char *name, void *symbol){
    if (!table) return -1;
    return cxml_table_put(&table->symbols, name, symbol);
}
