/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

int empty_set_asserts(cxml_set *set){
    CHECK_EQ(set->size, 0);
    CHECK_EQ(set->capacity, 0);
    CHECK_EQ(set->entries, NULL);
    CHECK_EQ(set->items.head, NULL);
    CHECK_EQ(set->items.tail, NULL);
    CHECK_EQ(set->items.len, 0);
    return 1;
}


cts test_new_cxml_set(){
    cxml_set set = new_cxml_set();
    CHECK_EQ(empty_set_asserts(&set), 1);
    cxml_pass()
}

cts test_new_alloc_cxml_set(){
    cxml_set *set = new_alloc_cxml_set();
    CHECK_EQ(empty_set_asserts(set), 1);
    FREE(set);
    cxml_pass()
}

cts test_cxml_set_init(){
    cxml_set set;
    cxml_set_init(&set);
    CHECK_EQ(empty_set_asserts(&set), 1);
    // should not seg-fault
    cxml_set_init(NULL);
    cxml_pass()
}

cts test_cxml_set_add(){
    cxml_set set = new_cxml_set();
    struct Data d1, d2, d3, d5, d6;
    struct Data *d4 = ALLOC(struct Data, 1);

    cxml_set_add(&set, &d1);
    CHECK_EQ(cxml_set_size(&set), 1);
    CHECK_EQ(set.capacity, 8);
    CHECK_NE(cxml_list_first(&set.items), NULL);
    CHECK_NE(cxml_list_last(&set.items), NULL);
    CHECK_NE(set.entries, NULL);
    CHECK_EQ(cxml_list_size(&set.items), 1);
    cxml_set_add(&set, &d1);
    CHECK_EQ(cxml_set_size(&set), 1);
    CHECK_EQ(set.capacity, 8);

    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_set_add(&set, d4);
    CHECK_EQ(cxml_set_size(&set), 4);

    // error
    cxml_set_add(&set, NULL);
    CHECK_EQ(cxml_set_size(&set), 4);
    cxml_set_add(NULL, d4);
    CHECK_EQ(cxml_set_size(&set), 4);
    CHECK_EQ(set.capacity, 8);

    // rehash
    cxml_set_add(&set, &d5);
    CHECK_EQ(set.capacity, 8);
    cxml_set_add(&set, &d6);
    CHECK_EQ(cxml_set_size(&set), 6);
    CHECK_EQ(cxml_list_size(&set.items), 6);
    CHECK_EQ(set.capacity, 16);
    CHECK_NE(set.items.head, NULL);
    CHECK_NE(set.items.tail, NULL);
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
    CHECK_EQ(cxml_set_size(&set), 3);

    cxml_set_add(&set, &d4);
    cxml_set_add(&set, &d5);
    cxml_set_add(&set, &d6);
    cxml_set_add(&set, &d7);
    CHECK_EQ(cxml_set_size(&set), 7);

    for (int i=0; i<cxml_set_size(&set); i++){
        CHECK_EQ(cxml_set_get(&set, i), ds[i]);
    }

    // errors
    CHECK_EQ(cxml_set_get(&set, -1), NULL);
    CHECK_EQ(cxml_set_get(&set, 7), NULL);
    CHECK_EQ(cxml_set_get(&set, 8), NULL);
    CHECK_EQ(cxml_set_get(NULL, 0), NULL);

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
    CHECK_EQ(cxml_set_size(&set), 4);

    // delete all
    cxml_set_remove(&set, &d1);
    CHECK_EQ(cxml_set_size(&set), 3);
    cxml_set_remove(&set, &d4);
    CHECK_EQ(cxml_set_size(&set), 2);
    cxml_set_remove(&set, &d2);
    CHECK_EQ(cxml_set_size(&set), 1);
    cxml_set_remove(&set, &d3);
    CHECK_EQ(cxml_set_size(&set), 0);
    CHECK_EQ(set.capacity, 8);

    // put 4 items
    cxml_set_add(&set, &d5);
    cxml_set_add(&set, &d6);
    cxml_set_add(&set, &d7);
    cxml_set_add(&set, &d8);
    CHECK_EQ(cxml_set_size(&set), 4);
    // set will not be bloated - capacity still remains the same, even after rehashing
    CHECK_EQ(set.capacity, 8);

    // delete 2 items
    cxml_set_remove(&set, &d6);
    CHECK_EQ(cxml_set_size(&set), 3);
    cxml_set_remove(&set, &d7);
    CHECK_EQ(cxml_set_size(&set), 2);
    CHECK_EQ(set.capacity, 8);

    // delete - error
    cxml_set_remove(&set, "simpless");
    CHECK_EQ(cxml_set_size(&set), 2);
    cxml_set_remove(&set, NULL);
    CHECK_EQ(cxml_set_size(&set), 2);
    cxml_set_remove(&set, &d6);
    CHECK_EQ(cxml_set_size(&set), 2);
    cxml_set_remove(NULL, "simple");
    CHECK_EQ(cxml_set_size(&set), 2);

    cxml_set_add(&set, &d1);
    CHECK_EQ(cxml_list_size(&set.items), 3);;
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);

    CHECK_EQ(cxml_set_size(&set), 5);
    cxml_set_add(&set, "foodie");
    CHECK_EQ(set.capacity, 16);
    CHECK_EQ(cxml_set_size(&set), 6);

    cxml_set_free(&set);
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_set_add(&set, &d4);
    cxml_set_add(&set, &d5);
    CHECK_EQ(set.capacity, 8);
    CHECK_EQ(cxml_set_size(&set), 5);
    // 4 items (live) + 1 (current::to-be-deleted)
    cxml_set_remove(&set, "simple");
    CHECK_EQ(cxml_set_size(&set), 5);
    // 4 items (live) + 1 (deleted) + current (1 item live)
    cxml_set_add(&set, &d6);
    // no smart savings, since this is already >= to the savings threshold (_CXML_HT_LOAD_FACTOR_AC)
    CHECK_EQ(set.capacity, 16);
    CHECK_EQ(cxml_set_size(&set), 6);
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
    CHECK_EQ(cxml_set_size(&set), 8);
    CHECK_EQ(empty_set_asserts(&set2), 1);
    cxml_set_copy(&set2, &set);
    for (int i=0; i<cxml_set_size(&set2); i++){
        CHECK_EQ(cxml_list_get(&set.items, i), cxml_list_get(&set2.items, i));
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
    CHECK_EQ(cxml_set_size(&set), 8);
    CHECK_EQ(empty_set_asserts(&set2), 1);
    cxml_set_extend(&set2, &set);
    for (int i=0; i<cxml_set_size(&set2); i++){
        CHECK_EQ(cxml_list_get(&set.items, i), cxml_list_get(&set2.items, i));
    }

    // should not seg-fault
    cxml_set_extend(&set2, NULL);
    cxml_set_extend(NULL, &set);

    // empty copy
    cxml_set_free(&set);
    CHECK_EQ(cxml_set_size(&set), 0);
    cxml_set_extend(&set2, &set);
    CHECK_EQ(cxml_set_size(&set2), 8);

    cxml_set_free(&set2);
    CHECK_EQ(cxml_set_size(&set2), 0);
    cxml_set_extend(&set2, &set);
    CHECK_EQ(cxml_set_size(&set), 0);

    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    cxml_set_add(&set2, &d1);
    cxml_set_add(&set2, &d2);
    cxml_set_add(&set2, &d3);
    CHECK_EQ(cxml_set_size(&set), 3);
    CHECK_EQ(cxml_set_size(&set2), 3);
    // no changes
    cxml_set_extend(&set2, &set);
    CHECK_EQ(cxml_set_size(&set), 3);
    CHECK_EQ(cxml_set_size(&set2), 3);

    cxml_set_add(&set, &d4);
    cxml_set_add(&set, &d5);
    cxml_set_add(&set, &d7);
    cxml_set_add(&set, &d8);
    CHECK_EQ(cxml_set_size(&set), 7);
    CHECK_EQ(cxml_set_size(&set2), 3);
    cxml_set_extend(&set2, &set);
    CHECK_EQ(cxml_set_size(&set2), 7);
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
    CHECK_EQ(empty_set_asserts(&set), 1);

    cxml_set_extend_list(&set, &list);
    CHECK_EQ(cxml_set_size(&set), 8);
    for (int i=0; i<8; i++){
        CHECK_EQ(cxml_list_get(&set.items, i), ds[i]);
    }
    // no change
    cxml_set_extend_list(&set, &list);
    CHECK_EQ(cxml_set_size(&set), 8);

    // errors -
    cxml_set_extend_list(&set, NULL);
    cxml_set_extend_list(NULL, &list);

    cxml_set_free(&set);
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    CHECK_EQ(cxml_set_size(&set), 3);
    cxml_set_extend_list(&set, &list);
    CHECK_EQ(cxml_set_size(&set), 8);

    cxml_set_free(&set);
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_set_size(){
    cxml_set set = new_cxml_set();
    CHECK_EQ(cxml_set_size(&set), 0);
    struct Data d1, d2, d3;
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    CHECK_EQ(cxml_set_size(&set), 3);
    CHECK_EQ(cxml_set_size(NULL), 0);
    cxml_set_free(&set);
    CHECK_EQ(cxml_set_size(&set), 0);
    cxml_pass()
}

cts test_cxml_set_is_empty(){
    cxml_set set = new_cxml_set();
    CHECK_TRUE(cxml_set_is_empty(&set));
    struct Data d1, d2, d3;
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    CHECK_FALSE(cxml_set_is_empty(&set));
    CHECK_TRUE(cxml_set_is_empty(NULL));
    cxml_set_free(&set);
    CHECK_TRUE(cxml_set_is_empty(&set));
    cxml_pass()
}

cts test_cxml_set_free(){
    cxml_set set = new_cxml_set();
    CHECK_EQ(empty_set_asserts(&set), 1);
    struct Data d1, d2, d3;
    cxml_set_add(&set, &d1);
    cxml_set_add(&set, &d2);
    cxml_set_add(&set, &d3);
    CHECK_EQ(cxml_set_size(&set), 3);
    CHECK_EQ(set.size, 3);
    CHECK_EQ(set.capacity, 8);
    CHECK_EQ(cxml_set_size(NULL), 0);
    cxml_set_free(&set);
    CHECK_EQ(empty_set_asserts(&set), 1);
    CHECK_EQ(set.size, 0);
    CHECK_EQ(set.capacity, 0);
    CHECK_EQ(set.entries, NULL);
    cxml_pass()
}

cts test_cxml_set__init_with(){
    cxml_set set = new_cxml_set(), set2 = new_cxml_set();
    cxml_set_add(&set2, "foo");
    cxml_set_add(&set2, "bar");
    CHECK_EQ(set2.size, 2);
    CHECK_EQ(set2.capacity, 8);
    CHECK_NE(set2.entries, NULL);
    CHECK_NE(set2.items.head, NULL);
    CHECK_NE(set2.items.tail, NULL);

    CHECK_EQ(empty_set_asserts(&set), 1);
    cxml_set_init_with(&set, &set2);
    CHECK_EQ(set2.capacity, 0);
    CHECK_EQ(set2.size, 0);
    CHECK_EQ(set2.entries, NULL);
    CHECK_EQ(set2.items.head, NULL);
    CHECK_EQ(set2.items.tail, NULL);

    CHECK_EQ(set.size, 2);
    CHECK_EQ(set.capacity, 8);
    CHECK_NE(set.entries, NULL);
    CHECK_NE(set.items.head, NULL);
    CHECK_NE(set.items.tail, NULL);

    cxml_set_free(&set);
    cxml_pass()
}

cts test_cxml_set_init_with(){
    cxml_set set = new_cxml_set(), set2 = new_cxml_set();
    cxml_set_add(&set2, "foo");
    cxml_set_add(&set2, "bar");
    CHECK_EQ(set2.size, 2);
    CHECK_EQ(set2.capacity, 8);
    CHECK_NE(set2.entries, NULL);
    CHECK_NE(set2.items.head, NULL);
    CHECK_NE(set2.items.tail, NULL);

    CHECK_EQ(empty_set_asserts(&set), 1);
    cxml_set_init_with(&set, &set2);
    CHECK_EQ(set2.capacity, 0);
    CHECK_EQ(set2.size, 0);
    CHECK_EQ(set2.entries, NULL);
    CHECK_EQ(set2.items.head, NULL);
    CHECK_EQ(set2.items.tail, NULL);

    CHECK_EQ(set.size, 2);
    CHECK_EQ(set.capacity, 8);
    CHECK_NE(set.entries, NULL);
    CHECK_NE(set.items.head, NULL);
    CHECK_NE(set.items.tail, NULL);

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
