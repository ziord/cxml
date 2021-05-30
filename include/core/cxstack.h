/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXSTACK_H
#define CXML_CXSTACK_H

#include "cxlist.h"

typedef struct {
    cxml_list stack;
} _cxml_stack;

void _cxml_stack_init(_cxml_stack *);

void _cxml_stack__push(_cxml_stack *, void *node);

void *_cxml_stack__pop(_cxml_stack *cxstack);

void *_cxml_stack__get(_cxml_stack *cxstack);

bool _cxml_stack_is_empty(_cxml_stack *stack);

int _cxml_stack_size(_cxml_stack *stack);

void _cxml_stack_free(_cxml_stack *stack);
#endif //CXML_CXSTACK_H
