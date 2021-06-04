/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXLIST_H
#define CXML_CXLIST_H

#include "cxcomm.h"
#include "cxmem.h"

#define cxml_for_each(_node, __list)                                                    \
void *_node = NULL;                                                                     \
for(struct _cxml_list__node *__00prev00##_node = NULL,                                  \
                            *__00current00##_node = (__list)->head;                     \
    __00current00##_node ? (_node = (__00current00##_node)->item) : 0;                  \
    __00prev00##_node    = __00current00##_node,                                        \
    __00current00##_node = (((void)__00prev00##_node ), (__00current00##_node)->next)   \
)

// shorter (and perhaps cleaner?)
#define cxml_for    cxml_for_each


struct _cxml_list__node{
    void *item;
    struct _cxml_list__node *next;
};

typedef struct _cxml_list{
    int len;
    struct _cxml_list__node *head;
    struct _cxml_list__node *tail;
}cxml_list;

void cxml_list_init(cxml_list* list);

void cxml_list_insert(cxml_list* list, void* item, bool at_front);

void cxml_list_insert_at_index(cxml_list *list, void *item, int index);

void cxml_list_append(cxml_list* list, void* item);

void cxml_list_extend(cxml_list* list1, cxml_list* list2);

void cxml_list_init_with(cxml_list* list1, cxml_list* list2);

void cxml_list_qextend(cxml_list* list1, cxml_list* list2);

void cxml_list_add(cxml_list* list, void* item);

int cxml_list_search(cxml_list* list, bool (*p_sfun)(void* p1, void* p2), void* fn);

int cxml_list_search_delete(cxml_list* list, bool (*p_sfun)(void* p1, void* p2), void* fn);

bool cxml_list_cmp_str_items(void* item1, void* item2);

bool cxml_list_cmp_raw_items(void* item1, void* item2);

cxml_list new_cxml_list();

cxml_list *new_alloc_cxml_list();

int cxml_list_size(cxml_list* list);

void* cxml_list_get(cxml_list* list, int index);

void* cxml_list_last(cxml_list *list);

void* cxml_list_first(cxml_list *list);

bool cxml_list_is_empty(cxml_list* list);

void *cxml_list_safe_delete(cxml_list *list, bool at_last);

void* cxml_list_safe_delete_at_pos(cxml_list* list, int index);

void cxml_list_delete(cxml_list* list, bool at_last);

void cxml_list_copy(cxml_list *cpy, cxml_list *ori);

void cxml_list_delete_at_pos(cxml_list* list, int index);

void cxml_list_delete_at_index(cxml_list* list, int index);

void cxml_list_free(cxml_list* list);


#endif //CXML_CXLIST_H
