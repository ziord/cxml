/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxstack.h"

void _cxml_stack_init(_cxml_stack *cxstack) {
    cxml_list_init(&cxstack->stack);
}

void _cxml_stack__push(_cxml_stack *cxstack, void *node) {
    // add a node to the top of the stack
    cxml_list_add(&cxstack->stack, node);
}

void *_cxml_stack__pop(_cxml_stack *cxstack) {
    // pop a node off the stack
    return cxml_list_safe_delete(&cxstack->stack, false);
}

void *_cxml_stack__get(_cxml_stack *cxstack) {
    // get top-most node on the stack without popping it off the stack
    return cxml_list_get(&cxstack->stack, 0);
}

bool _cxml_stack_is_empty(_cxml_stack *stack) {
    return cxml_list_is_empty(&stack->stack);
}

int _cxml_stack_size(_cxml_stack *stack){
    return cxml_list_size(&stack->stack);
}

void _cxml_stack_free(_cxml_stack *cxstack) {
    cxml_list_free(&cxstack->stack);
}
