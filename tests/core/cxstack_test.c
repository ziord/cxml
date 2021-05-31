/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

extern int empty_list_asserts(cxml_list *list);

// no seg-fault tests, because stack is an internal structure with
// very precise use-case, and not meant to be used by external users.

int empty_stack_asserts(_cxml_stack *stack){
    CHECK_EQ(empty_list_asserts(&stack->stack), 1);
    return 1;
}

cts test__cxml_stack_init(){
    _cxml_stack stack;
    _cxml_stack_init(&stack);
    CHECK_EQ(empty_stack_asserts(&stack), 1);
    cxml_pass()
}

cts test__cxml_stack__push(){
    _cxml_stack stack;
    _cxml_stack_init(&stack);
    struct Data d1, d2, d3;

    _cxml_stack__push(&stack, NULL);
    CHECK_EQ(cxml_list_size(&stack.stack), 0);

    _cxml_stack__push(&stack, &d1);
    CHECK_EQ(cxml_list_size(&stack.stack), 1);
    CHECK_EQ(cxml_list_first(&stack.stack), &d1);

    _cxml_stack__push(&stack, &d2);
    CHECK_EQ(cxml_list_size(&stack.stack), 2);
    CHECK_EQ(cxml_list_first(&stack.stack), &d2);

    _cxml_stack__push(&stack, &d3);
    CHECK_EQ(cxml_list_size(&stack.stack), 3);
    CHECK_EQ(cxml_list_first(&stack.stack), &d3);

    _cxml_stack_free(&stack);
    cxml_pass()
}

cts test__cxml_stack__pop(){
    _cxml_stack stack;
    _cxml_stack_init(&stack);
    struct Data d1, d2, d3;

    _cxml_stack__push(&stack, &d1);
    _cxml_stack__push(&stack, &d2);
    _cxml_stack__push(&stack, &d3);

    void *d = _cxml_stack__pop(&stack);
    CHECK_EQ(d, &d3);
    CHECK_EQ(cxml_list_size(&stack.stack), 2);

    d = _cxml_stack__pop(&stack);
    CHECK_EQ(d, &d2);
    CHECK_EQ(cxml_list_size(&stack.stack), 1);

    d = _cxml_stack__pop(&stack);
    CHECK_EQ(d, &d1);
    CHECK_EQ(cxml_list_size(&stack.stack), 0);

    d = _cxml_stack__pop(&stack);
    CHECK_EQ(d, NULL);
    CHECK_EQ(cxml_list_size(&stack.stack), 0);

    _cxml_stack_free(&stack);
    cxml_pass()
}

cts test__cxml_stack__get(){
    _cxml_stack stack;
    _cxml_stack_init(&stack);
    struct Data d1, d2, d3;

    _cxml_stack__push(&stack, &d1);
    _cxml_stack__push(&stack, &d2);
    _cxml_stack__push(&stack, &d3);

    void *v = _cxml_stack__get(&stack);
    CHECK_EQ(v, &d3);

    _cxml_stack__pop(&stack);
    v = _cxml_stack__get(&stack);
    CHECK_EQ(v, &d2);

    _cxml_stack__push(&stack, &d2);
    CHECK_EQ(_cxml_stack__get(&stack), &d2);

    _cxml_stack_free(&stack);
    cxml_pass()
}

cts test__cxml_stack_is_empty(){
    _cxml_stack stack;
    _cxml_stack_init(&stack);
    struct Data d;

    CHECK_TRUE(_cxml_stack_is_empty(&stack));

    _cxml_stack__push(&stack, &d);

    CHECK_FALSE(_cxml_stack_is_empty(&stack));

    _cxml_stack__pop(&stack);
    CHECK_TRUE(_cxml_stack_is_empty(&stack));

    _cxml_stack_free(&stack);
    cxml_pass()
}

cts test__cxml_stack_free(){
    _cxml_stack stack;
    _cxml_stack_init(&stack);
    struct Data d;
    CHECK_EQ(empty_stack_asserts(&stack), 1);

    _cxml_stack__push(&stack, &d);
    CHECK_EQ(cxml_list_size(&stack.stack), 1);

    _cxml_stack_free(&stack);
    CHECK_EQ(empty_stack_asserts(&stack), 1);

    _cxml_stack_free(&stack);
    CHECK_EQ(empty_stack_asserts(&stack), 1);

    cxml_pass()
}

void suite_cxstack(){
    cxml_suite(cxstack)
    {
        cxml_add_m_test(6,
                        test__cxml_stack_init,
                        test__cxml_stack__push,
                        test__cxml_stack__pop,
                        test__cxml_stack__get,
                        test__cxml_stack_is_empty,
                        test__cxml_stack_free
        )
        cxml_run_suite()
    }
}
