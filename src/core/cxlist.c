/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxlist.h"


#define __init_list(__list)         \
    (__list)->len = 0;              \
    (__list)->head = NULL;          \
    (__list)->tail = NULL;



void cxml_list_init(cxml_list* list){
    if (!list) return;
    __init_list(list)
}

void cxml_list__init(cxml_list *list){
    list->len = 0;
    list->head = NULL;
    list->tail = NULL;
}

static struct _cxml_list__node* new_node(void* item){
    struct _cxml_list__node* node = ALLOC(struct _cxml_list__node, 1);
    node->item = item;
    node->next = NULL;
    return node;
}

cxml_list* new_alloc_cxml_list(){
    cxml_list* list = ALLOCR(cxml_list, 1, "CXMLFatalError: Not enough memory to allocate list.");
    __init_list(list);
    return list;
}

void cxml_list_insert(cxml_list* list, void* item, bool at_front){
    if (!list || !item) return;
    struct _cxml_list__node *node = new_node(item);
    if (!list->head){
        list->head = list->tail = node;
    }
    else if (!at_front){
        list->tail->next = node;
        list->tail = node;
    }
    else{
        node->next = list->head;
        list->head = node;
    }
    list->len++;
}

void cxml_list_insert_at_index(cxml_list *list, void *item, int index) {
    if (!list || !item) return;
    if ((index < 0) || (index > list->len)){
        return;
    }
    if (list->len == 0 || index == 0){
        cxml_list_insert(list, item, 1);
        return;
    }else if (index == list->len){
        cxml_list_insert(list, item, 0);
        return;
    }
    struct _cxml_list__node* current = list->head;
    struct _cxml_list__node* prev = NULL;
    for (int i=0; i<index; i++){
        prev = current;
        current = current->next;
    }
    struct _cxml_list__node* node = new_node(item);
    // prev, new, curr

    // prev is GUARANTEED to not be NULL because all 0 index insertions
    // are handled in the condition above, and since prev can only be NULL,
    // when inserting at index 0.
    prev->next = node;
    node->next = current;
    list->len++;
}

void cxml_list_append(cxml_list* list, void* item){
    cxml_list_insert(list, item, false);
}

void cxml_list_extend(cxml_list* list1, cxml_list* list2){
    if (!list1 || !list2) return;
    cxml_for_each(item, list2){
        cxml_list_insert(list1, item, false);
    }
}

void cxml_list_init_with(cxml_list* list1, cxml_list* list2){
    if (!list1 || !list2) return;
    list1->head = list2->head;
    list1->tail = list2->tail;
    list1->len = list2->len;
    __init_list(list2);
}

// quick - extend
// extend list1 with list2, invalidating/emptying list2 in the process.
void cxml_list_qextend(cxml_list* list1, cxml_list* list2){
    if (!list1 || !list2) return;
    // x, y, z   *   a, b, c
    if (list2->len){
        if (list1->len){
            list1->tail->next = list2->head;
            list1->tail = list2->tail;
            list1->len += list2->len;
            __init_list(list2);
        }else{
            cxml_list_init_with(list1, list2);
        }
    }
}

void cxml_list_add(cxml_list* list, void* item){
    cxml_list_insert(list, item, true);
}

int cxml_list_search(cxml_list* list, bool (*p_sfun)(void* p1, void* p2), void* fn){
    if (!list) return -1;
    struct _cxml_list__node* curr = list->head;
    bool found = false;
    int index = 0;
    while (curr) {
        if ((*p_sfun)(curr->item, fn)) {
            found = true;
            break;
        }
        curr = curr->next;
        index++;
    }
    return found ? index : -1;
}

int cxml_list_search_delete(cxml_list* list, bool (*p_sfun)(void* p1, void* p2), void* fn){
    if (!list || !list->len) return 0;
    struct _cxml_list__node* prev = NULL;
    struct _cxml_list__node* curr = list->head;
    bool found = false;
    int index = 0;
    while (curr) {
        if ((*p_sfun)(curr->item, fn)) {
            found = true;
            break;
        }
        prev = curr;
        curr = curr->next;
        index++;
    }
    if (found){
        if (index == 0){
            cxml_list_delete(list, 0);
            return 1;
        }else if ((index) == (list->len - 1)){
            cxml_list_delete(list, 1);
            return 1;
        }else{
            if (prev) {
                // x, y, cur, z, m
                prev->next = curr->next;
                FREE(curr);
                list->len--;
                return 1;
            }
        }
    }
    return 0;
}

// general comparison function for str
bool cxml_list_cmp_str_items(void* item1, void* item2){
    // this function is unsafe, as it doesn't check if either arguments are NULL
    // it is only used internally where data is GUARANTEED to not be NULL
    return ((item1 == item2) || (strcmp(item1, item2) == 0));
}

// general comparison function for raw pointers
bool cxml_list_cmp_raw_items(void* itm1, void* itm2){
    // this function is unsafe, as it doesn't check if either arguments are NULL
    // it is only used internally where data is GUARANTEED to not be NULL
    return (itm1 == itm2);
}

static struct _cxml_list__node* _cxml_list__remove(cxml_list* list, bool at_last){
    if (!list || !list->head) return NULL;
    struct _cxml_list__node* _node = NULL;
    if (list->len == 1){
        _node = list->head;
        list->head = list->tail = NULL;
    }
    else{
        if (!at_last){
            _node = list->head;
            list->head = list->head->next;
        }
        else{
            struct _cxml_list__node* prev = NULL, *curr = list->head;
            while (curr != list->tail){
                prev = curr;
                curr = curr->next;
            }
            prev->next = NULL;
            list->tail = prev;
            _node = curr;
        }
    }
    list->len--;
    return _node;
}

int cxml_list_size(cxml_list* list){
    if (!list) return 0;
    return list->len;
}

void* cxml_list_get(cxml_list* list, int index){
    if (!list || list->len == 0 || (index >= list->len) || index < 0){
        return NULL;
    }
    if (index == 0){
        return list->head->item;
    }else if (index == (list->len - 1)){
        return list->tail->item;
    }
    struct _cxml_list__node* current = list->head;
    for (int i=0; i<index; i++){
        current = current->next;
    }
    return current->item;
}

void* cxml_list_last(cxml_list *list){
    if (!list || !list->len) return NULL;
    return list->tail->item;
}

void* cxml_list_first(cxml_list *list){
    if (!list || !list->len) return NULL;
    return list->head->item;
}

bool cxml_list_is_empty(cxml_list* list){
    return (!list) || (list->len == 0);
}

cxml_list new_cxml_list(){
    cxml_list list;
    __init_list(&list);
    return list;
}

// should be used when allocated data is stored in the list
void *cxml_list_safe_delete(cxml_list *list, bool at_last) {
    struct _cxml_list__node* node = _cxml_list__remove(list, at_last);
    if (node){
        void* item = node->item;
        FREE(node);
        return item;
    }
    return NULL;
}

struct _cxml_list__node* _cxml_list_remove_at_pos(cxml_list* list, int index){
    if (!list || !list->len || index >= list->len || index < 0){
        return NULL;
    }
    if (index == 0){
        return _cxml_list__remove(list, 0);
    }
    else if (index == list->len - 1){
        return _cxml_list__remove(list, 1);
    }
    else {
        struct _cxml_list__node *current = list->head;
        struct _cxml_list__node *prev = NULL;
        for (int i = 0; i < index; i++) {
            prev = current;
            current = current->next;
        }
        // x, y, cur, z, m
        prev->next = current->next;
        list->len--;
        return current;
    }
}

void* cxml_list_safe_delete_at_pos(cxml_list* list, int index){
    struct _cxml_list__node* node = _cxml_list_remove_at_pos(list, index);
    if (node){
        void* item = node->item;
        FREE(node);
        return item;
    }
    return NULL;
}

void cxml_list_delete(cxml_list* list, bool at_last){
    struct _cxml_list__node* node = _cxml_list__remove(list, at_last);
    FREE(node);
}

void cxml_list_copy(cxml_list *cpy, cxml_list *ori){
    if (!cpy || !ori) return;
    cxml_for_each(item, ori){
        cxml_list_insert(cpy, item, false);
    }
}

void cxml_list_delete_at_pos(cxml_list* list, int index){
    FREE(_cxml_list_remove_at_pos(list, index));
}

void cxml_list_delete_at_index(cxml_list* list, int index){
    FREE(_cxml_list_remove_at_pos(list, index));
}

void cxml_list_free(cxml_list* list){
    if (!list) return;
    struct _cxml_list__node *curr;
    while (list->head){
        curr = list->head;
        list->head = list->head->next;
        FREE(curr);
    }
    __init_list(list)
}
