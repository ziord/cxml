/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

int empty_set_asserts(cxml_set *set){
    cxml_assert__zero(set->size)
    cxml_assert__zero(set->capacity)
    cxml_assert__null(set->entries)
    cxml_assert__null(set->items.head)
    cxml_assert__null(set->items.tail)
    cxml_assert__zero(set->items.len)
    return 1;
}


cts test_new_cxml_set(){
    cxml_set set = new_cxml_set();
    cxml_assert__one(empty_set_asserts(&set))
    cxml_pass()
}

cts test_new_alloc_cxml_set(){
    cxml_set *set = new_alloc_cxml_set();
    cxml_assert__one(empty_set_asserts(set))
    FREE(set);
    cxml_pass()
}

cts test_cxml_set_init(){
    cxml_set set;
    cxml_set_init(&set);
    cxml_assert__one(empty_set_asserts(&set))
    // should not seg-fault
    cxml_set_init(NULL);
    cxml_pass()
}

cts test_cxml_set_add(){
    cxml_set set = new_cxml_set();
    struct Data d1, d2, d3, d5, d6;
    struct Data *d4 = ALLOC(struct Data, 1);

    cxml_set_add(&set, &d1);
    cxml_assert__one(cxml_set_size(&set))
    cxml_assert__eq(set.capacity, 8)
    cxml_assert__not_null(cxml_list_first(&set.items))
    cxml_assert__not_null(cxml_list_last(&set.items))
    cxml_assert__not_null(set.entries)
    cxml_assert__one(cxml_list_size(&set.items))
    cxml_set_add(&set, &d1);
    cxml_assert__one(cxml_set_size(&set))
    cxml_assert__eq(set.capacity, 8)

    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_set_add(&set, d4);
    cxml_assert__eq(cxml_set_size(&set), 4)

    // error
    cxml_set_add(&set, NULL);
    cxml_assert__eq(cxml_set_size(&set), 4)
    cxml_set_add(NULL, d4);
    cxml_assert__eq(cxml_set_size(&set), 4)
    cxml_assert__eq(set.capacity, 8)

    // rehash
    cxml_set_add(&set, &d5);
    cxml_assert__eq(set.capacity, 8)
    cxml_set_add(&set, &d6);
    cxml_assert__eq(cxml_set_size(&set), 6)
    cxml_assert__eq(cxml_list_size(&set.items), 6)
    cxml_assert__eq(set.capacity, 16)
    cxml_assert__not_null(set.items.head)
    cxml_assert__not_null(set.items.tail)
    FREE(d4);
    cxml_set_free(&set);

    cxml_pass()
}

cts test_cxml_set_get(){
    cxml_set set = new_cxml_set();
    struct Data d1, d2, d3, d4, d5, d6, d7;
    struct Data *ds[] = {&d1, &d2, &d3, &d4, &d5, &d6, &d7};
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_assert__eq(cxml_set_size(&set), 3)

    cxml_set_add(&set, &d4);
    cxml_set_add(&set, &d5);
    cxml_set_add(&set, &d6);
    cxml_set_add(&set, &d7);
    cxml_assert__eq(cxml_set_size(&set), 7)

    for (int i=0; i<cxml_set_size(&set); i++){
        cxml_assert__eq(cxml_set_get(&set, i), ds[i])
    }

    // errors
    cxml_assert__null(cxml_set_get(&set, -1))
    cxml_assert__null(cxml_set_get(&set, 7))
    cxml_assert__null(cxml_set_get(&set, 8))
    cxml_assert__null(cxml_set_get(NULL, 0))

    cxml_set_free(&set);
    cxml_pass()
}

cts test_cxml_set_remove(){
    cxml_set set = new_cxml_set();
    struct Data d1, d2, d3, d4, d5, d6, d7, d8;
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_set_add(&set, &d4);
    cxml_assert__eq(cxml_set_size(&set), 4)

    // delete all
    cxml_set_remove(&set, &d1);
    cxml_assert__eq(cxml_set_size(&set), 3)
    cxml_set_remove(&set, &d4);
    cxml_assert__eq(cxml_set_size(&set), 2)
    cxml_set_remove(&set, &d2);
    cxml_assert__eq(cxml_set_size(&set), 1)
    cxml_set_remove(&set, &d3);
    cxml_assert__zero(cxml_set_size(&set))
    cxml_assert__eq(set.capacity, 8)

    // put 4 items
    cxml_set_add(&set, &d5);
    cxml_set_add(&set, &d6);
    cxml_set_add(&set, &d7);
    cxml_set_add(&set, &d8);
    cxml_assert__eq(cxml_set_size(&set), 4)
    // set will not be bloated - capacity still remains the same, even after rehashing
    cxml_assert__eq(set.capacity, 8)

    // delete 2 items
    cxml_set_remove(&set, &d6);
    cxml_assert__eq(cxml_set_size(&set), 3)
    cxml_set_remove(&set, &d7);
    cxml_assert__eq(cxml_set_size(&set), 2)
    cxml_assert__eq(set.capacity, 8)

    // delete - error
    cxml_set_remove(&set, "simpless");
    cxml_assert__eq(cxml_set_size(&set), 2)
    cxml_set_remove(&set, NULL);
    cxml_assert__eq(cxml_set_size(&set), 2)
    cxml_set_remove(&set, &d6);
    cxml_assert__eq(cxml_set_size(&set), 2)
    cxml_set_remove(NULL, "simple");
    cxml_assert__eq(cxml_set_size(&set), 2)

    cxml_set_add(&set, &d1);
    cxml_assert__eq(cxml_list_size(&set.items), 3);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);

    cxml_assert__eq(cxml_set_size(&set), 5)
    cxml_set_add(&set, "foodie");
    cxml_assert__eq(set.capacity, 16)
    cxml_assert__eq(cxml_set_size(&set), 6)

    cxml_set_free(&set);
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_set_add(&set, &d4);
    cxml_set_add(&set, &d5);
    cxml_assert__eq(set.capacity, 8)
    cxml_assert__eq(cxml_set_size(&set), 5)
    // 4 items (live) + 1 (current::to-be-deleted)
    cxml_set_remove(&set, "simple");
    cxml_assert__eq(cxml_set_size(&set), 5)
    // 4 items (live) + 1 (deleted) + current (1 item live)
    cxml_set_add(&set, &d6);
    // no smart savings, since this is already >= to the savings threshold (_CXML_HT_LOAD_FACTOR_AC)
    cxml_assert__eq(set.capacity, 16)
    cxml_assert__eq(cxml_set_size(&set), 6)
    cxml_set_free(&set);
    cxml_pass()
}

cts test_cxml_set_copy(){
    cxml_set set = new_cxml_set(),
             set2 = new_cxml_set();
    struct Data d1, d2, d3, d4, d5, d6, d7, d8;
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_set_add(&set, &d4);
    cxml_set_add(&set, &d5);
    cxml_set_add(&set, &d6);
    cxml_set_add(&set, &d7);
    cxml_set_add(&set, &d8);
    cxml_assert__eq(cxml_set_size(&set), 8)
    cxml_assert__one(empty_set_asserts(&set2))
    cxml_set_copy(&set2, &set);
    for (int i=0; i<cxml_set_size(&set2); i++){
        cxml_assert__eq(cxml_list_get(&set.items, i), cxml_list_get(&set2.items, i))
    }

    // should not seg-fault
    cxml_set_copy(&set2, NULL);
    cxml_set_copy(NULL, &set);

    cxml_set_free(&set);
    cxml_set_free(&set2);

    cxml_pass()
}

cts test_cxml_set_extend(){
    cxml_set set = new_cxml_set(),
             set2 = new_cxml_set();
    struct Data d1, d2, d3, d4, d5, d6, d7, d8;
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_set_add(&set, &d4);
    cxml_set_add(&set, &d5);
    cxml_set_add(&set, &d6);
    cxml_set_add(&set, &d7);
    cxml_set_add(&set, &d8);
    cxml_assert__eq(cxml_set_size(&set), 8)
    cxml_assert__one(empty_set_asserts(&set2))
    cxml_set_extend(&set2, &set);
    for (int i=0; i<cxml_set_size(&set2); i++){
        cxml_assert__eq(cxml_list_get(&set.items, i), cxml_list_get(&set2.items, i))
    }

    // should not seg-fault
    cxml_set_extend(&set2, NULL);
    cxml_set_extend(NULL, &set);

    // empty copy
    cxml_set_free(&set);
    cxml_assert__zero(cxml_set_size(&set))
    cxml_set_extend(&set2, &set);
    cxml_assert__eq(cxml_set_size(&set2), 8)

    cxml_set_free(&set2);
    cxml_assert__zero(cxml_set_size(&set2))
    cxml_set_extend(&set2, &set);
    cxml_assert__zero(cxml_set_size(&set))

    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_set_add(&set2, &d1);
    cxml_set_add(&set2, &d2);
    cxml_set_add(&set2, &d3);
    cxml_assert__eq(cxml_set_size(&set), 3)
    cxml_assert__eq(cxml_set_size(&set2), 3)
    // no changes
    cxml_set_extend(&set2, &set);
    cxml_assert__eq(cxml_set_size(&set), 3)
    cxml_assert__eq(cxml_set_size(&set2), 3)

    cxml_set_add(&set, &d4);
    cxml_set_add(&set, &d5);
    cxml_set_add(&set, &d7);
    cxml_set_add(&set, &d8);
    cxml_assert__eq(cxml_set_size(&set), 7)
    cxml_assert__eq(cxml_set_size(&set2), 3)
    cxml_set_extend(&set2, &set);
    cxml_assert__eq(cxml_set_size(&set2), 7)
    cxml_set_free(&set);
    cxml_set_free(&set2);
    cxml_pass()
}

cts test_cxml_set_extend_list(){
    cxml_set set = new_cxml_set();
    struct Data d1, d2, d3, d4, d5, d6, d7, d8;
    struct Data *ds[] = {&d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8};
    cxml_list list = new_cxml_list();
    for (int i = 0; i < 8; i++){
        cxml_list_append(&list, ds[i]);
    }
    cxml_assert__one(empty_set_asserts(&set))

    cxml_set_extend_list(&set, &list);
    cxml_assert__eq(cxml_set_size(&set), 8)
    for (int i=0; i<8; i++){
        cxml_assert__eq(cxml_list_get(&set.items, i), ds[i])
    }
    // no change
    cxml_set_extend_list(&set, &list);
    cxml_assert__eq(cxml_set_size(&set), 8)

    // errors -
    cxml_set_extend_list(&set, NULL);
    cxml_set_extend_list(NULL, &list);

    cxml_set_free(&set);
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_assert__eq(cxml_set_size(&set), 3)
    cxml_set_extend_list(&set, &list);
    cxml_assert__eq(cxml_set_size(&set), 8)

    cxml_set_free(&set);
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_set_size(){
    cxml_set set = new_cxml_set();
    cxml_assert__zero(cxml_set_size(&set))
    struct Data d1, d2, d3;
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_assert__eq(cxml_set_size(&set), 3)
    cxml_assert__zero(cxml_set_size(NULL))
    cxml_set_free(&set);
    cxml_assert__zero(cxml_set_size(&set))
    cxml_pass()
}

cts test_cxml_set_is_empty(){
    cxml_set set = new_cxml_set();
    cxml_assert__true(cxml_set_is_empty(&set))
    struct Data d1, d2, d3;
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_assert__false(cxml_set_is_empty(&set))
    cxml_assert__true(cxml_set_is_empty(NULL))
    cxml_set_free(&set);
    cxml_assert__true(cxml_set_is_empty(&set))
    cxml_pass()
}

cts test_cxml_set_free(){
    cxml_set set = new_cxml_set();
    cxml_assert__one(empty_set_asserts(&set))
    struct Data d1, d2, d3;
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_assert__eq(cxml_set_size(&set), 3)
    cxml_assert__eq(set.size, 3)
    cxml_assert__eq(set.capacity, 8)
    cxml_assert__zero(cxml_set_size(NULL))
    cxml_set_free(&set);
    cxml_assert__one(empty_set_asserts(&set))
    cxml_assert__zero(set.size)
    cxml_assert__zero(set.capacity)
    cxml_assert__null(set.entries)
    cxml_pass()
}

cts test_cxml_set__init_with(){
    cxml_set set = new_cxml_set(), set2 = new_cxml_set();
    cxml_set_add(&set2, "foo");
    cxml_set_add(&set2, "bar");
    cxml_assert__eq(set2.size, 2)
    cxml_assert__eq(set2.capacity, 8)
    cxml_assert__not_null(set2.entries)
    cxml_assert__not_null(set2.items.head)
    cxml_assert__not_null(set2.items.tail)

    cxml_assert__one(empty_set_asserts(&set))
    cxml_set_init_with(&set, &set2);
    cxml_assert__zero(set2.capacity)
    cxml_assert__zero(set2.size)
    cxml_assert__null(set2.entries)
    cxml_assert__null(set2.items.head)
    cxml_assert__null(set2.items.tail)

    cxml_assert__eq(set.size, 2)
    cxml_assert__eq(set.capacity, 8)
    cxml_assert__not_null(set.entries)
    cxml_assert__not_null(set.items.head)
    cxml_assert__not_null(set.items.tail)

    cxml_set_free(&set);
    cxml_pass()
}

cts test_cxml_set_init_with(){
    cxml_set set = new_cxml_set(), set2 = new_cxml_set();
    cxml_set_add(&set2, "foo");
    cxml_set_add(&set2, "bar");
    cxml_assert__eq(set2.size, 2)
    cxml_assert__eq(set2.capacity, 8)
    cxml_assert__not_null(set2.entries)
    cxml_assert__not_null(set2.items.head)
    cxml_assert__not_null(set2.items.tail)

    cxml_assert__one(empty_set_asserts(&set))
    cxml_set_init_with(&set, &set2);
    cxml_assert__zero(set2.capacity)
    cxml_assert__zero(set2.size)
    cxml_assert__null(set2.entries)
    cxml_assert__null(set2.items.head)
    cxml_assert__null(set2.items.tail)

    cxml_assert__eq(set.size, 2)
    cxml_assert__eq(set.capacity, 8)
    cxml_assert__not_null(set.entries)
    cxml_assert__not_null(set.items.head)
    cxml_assert__not_null(set.items.tail)

    // should not seg-fault
    cxml_set_init_with(&set, NULL);
    cxml_set_init_with(NULL, &set2);

    cxml_set_free(&set);
    cxml_pass()
}

void suite_cxmset() {
    cxml_suite(cxmset)
    {
        cxml_add_m_test(14,
                        test_new_cxml_set,
                        test_new_alloc_cxml_set,
                        test_cxml_set_init,
                        test_cxml_set_add,
                        test_cxml_set_get,
                        test_cxml_set_remove,
                        test_cxml_set_copy,
                        test_cxml_set_extend,
                        test_cxml_set_extend_list,
                        test_cxml_set_size,
                        test_cxml_set_is_empty,
                        test_cxml_set_free,
                        test_cxml_set__init_with,
                        test_cxml_set_init_with
                )
        cxml_run_suite()
    }
}
