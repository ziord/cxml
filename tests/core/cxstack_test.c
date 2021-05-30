/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

extern int empty_list_asserts(cxml_list *list);

// no seg-fault tests, because stack is an internal structure with
// very precise use-case, and not meant to be used by external users.

int empty_stack_asserts(_cxml_stack *stack){
    cxml_assert__one(empty_list_asserts(&stack->stack))
    return 1;
}

cts test__cxml_stack_init(){
    _cxml_stack stack;
    _cxml_stack_init(&stack);
    cxml_assert__one(empty_stack_asserts(&stack))
    cxml_pass()
}

cts test__cxml_stack__push(){
    _cxml_stack stack;
    _cxml_stack_init(&stack);
    struct Data d1, d2, d3;

    _cxml_stack__push(&stack, NULL);
    cxml_assert__zero(cxml_list_size(&stack.stack))

    _cxml_stack__push(&stack, &d1);
    cxml_assert__one(cxml_list_size(&stack.stack))
    cxml_assert__eq(cxml_list_first(&stack.stack), &d1)

    _cxml_stack__push(&stack, &d2);
    cxml_assert__two(cxml_list_size(&stack.stack))
    cxml_assert__eq(cxml_list_first(&stack.stack), &d2)

    _cxml_stack__push(&stack, &d3);
    cxml_assert__eq(cxml_list_size(&stack.stack), 3)
    cxml_assert__eq(cxml_list_first(&stack.stack), &d3)

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
    cxml_assert__eq(d, &d3)
    cxml_assert__two(cxml_list_size(&stack.stack))

    d = _cxml_stack__pop(&stack);
    cxml_assert__eq(d, &d2)
    cxml_assert__one(cxml_list_size(&stack.stack))

    d = _cxml_stack__pop(&stack);
    cxml_assert__eq(d, &d1)
    cxml_assert__zero(cxml_list_size(&stack.stack))

    d = _cxml_stack__pop(&stack);
    cxml_assert__null(d)
    cxml_assert__zero(cxml_list_size(&stack.stack))

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
    cxml_assert__eq(v, &d3)

    _cxml_stack__pop(&stack);
    v = _cxml_stack__get(&stack);
    cxml_assert__eq(v, &d2)

    _cxml_stack__push(&stack, &d2);
    cxml_assert__eq(_cxml_stack__get(&stack), &d2)

    _cxml_stack_free(&stack);
    cxml_pass()
}

cts test__cxml_stack_is_empty(){
    _cxml_stack stack;
    _cxml_stack_init(&stack);
    struct Data d;

    cxml_assert__true(_cxml_stack_is_empty(&stack))

    _cxml_stack__push(&stack, &d);

    cxml_assert__false(_cxml_stack_is_empty(&stack))

    _cxml_stack__pop(&stack);
    cxml_assert__true(_cxml_stack_is_empty(&stack))

    _cxml_stack_free(&stack);
    cxml_pass()
}

cts test__cxml_stack_free(){
    _cxml_stack stack;
    _cxml_stack_init(&stack);
    struct Data d;
    cxml_assert__one(empty_stack_asserts(&stack))

    _cxml_stack__push(&stack, &d);
    cxml_assert__one(cxml_list_size(&stack.stack))

    _cxml_stack_free(&stack);
    cxml_assert__one(empty_stack_asserts(&stack))

    _cxml_stack_free(&stack);
    cxml_assert__one(empty_stack_asserts(&stack))

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
