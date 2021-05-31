/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"
#include "core/cxgrptable.h"

int empty_grp_table_asserts(cxml_grp_table *table){
    CHECK_EQ(table->count, 0)
    CHECK_EQ(table->capacity, 0)
    CHECK_EQ(table->entries, NULL)
    CHECK_EQ(table->keys.len, 0)
    CHECK_EQ(table->keys.head, NULL)
    CHECK_EQ(table->keys.tail, NULL)
    return 1;
}

cts test_cxml_grp_table_init(){
    cxml_grp_table table;
    cxml_grp_table_init(&table);
    CHECK_EQ(empty_grp_table_asserts(&table), 1)
    // should not seg-fault
    cxml_grp_table_init(NULL);
    cxml_pass()
}

cts test_new_cxml_grp_table(){
    cxml_grp_table table = new_cxml_grp_table();
    CHECK_EQ(empty_grp_table_asserts(&table), 1)
    cxml_pass()
}

cts test_cxml_grp_table_put(){
    cxml_grp_table table = new_cxml_grp_table();
    struct Data parent1, parent2, parent3, child1, child2;

    cxml_grp_table_put(&table, &parent1, &child1);
    cxml_grp_table_put(&table, &parent1, &child2);
    cxml_grp_table_put(&table, &parent2, &child1);
    cxml_grp_table_put(&table, &parent2, &child2);
    CHECK_EQ(cxml_grp_table_size(&table), 2)

    cxml_grp_table_put(&table, &parent3, NULL);
    CHECK_EQ(cxml_grp_table_size(&table), 2)

    cxml_grp_table_put(&table, NULL, &child1);
    CHECK_EQ(cxml_grp_table_size(&table), 2)

    cxml_grp_table_put(NULL, &parent3, &child1);
    CHECK_EQ(cxml_grp_table_size(&table), 2)
    CHECK_EQ(table.capacity, 8)

    cxml_grp_table_put(&table, "test-1", &child1);
    cxml_grp_table_put(&table, "test-2", &child2);
    cxml_grp_table_put(&table, "test-3", &child1);
    cxml_grp_table_put(&table, "test-4", &child2);
    cxml_grp_table_put(&table, "test-5", &child2);
    CHECK_EQ(cxml_grp_table_size(&table), 7)
    CHECK_EQ(table.capacity, 16)

    CHECK_EQ(cxml_list_size(&table.keys), 7)
    CHECK_NE(cxml_list_first(&table.keys), NULL)
    CHECK_NE(cxml_list_last(&table.keys), NULL)
    cxml_grp_table_free(&table);
    cxml_pass()
}

extern int empty_list_asserts(cxml_list *list);

cts test_cxml_grp_table_get(){
    cxml_grp_table table = new_cxml_grp_table();
    struct Data parent1, parent2, parent3,
            parent4, parent5, parent6, child1, child2;

    cxml_grp_table_put(&table, &parent1, &child1);
    cxml_grp_table_put(&table, &parent1, &child1);
    cxml_grp_table_put(&table, &parent1, &child1);
    cxml_grp_table_put(&table, &parent2, &child2);
    cxml_grp_table_put(&table, &parent2, &child2);
    cxml_grp_table_put(&table, &parent2, &child2);
    cxml_grp_table_put(&table, &parent2, &child2);

    cxml_list value = new_cxml_list();
    cxml_grp_table_get(&table, &parent1, &value);
    CHECK_EQ(cxml_list_size(&value), 3)

    {
        cxml_for_each(node, &value){
            CHECK_EQ(node, &child1)
        }
    }

    cxml_list_free(&value);
    cxml_grp_table_get(&table, &parent2, &value);
    CHECK_EQ(cxml_list_size(&value), 4)

    {
        cxml_for_each(node, &value){
            CHECK_EQ(node, &child2)
        }
    }

    cxml_list_free(&value);

    // errors
    cxml_grp_table_get(&table, NULL, &value);
    CHECK_EQ(empty_list_asserts(&value), 1)

    cxml_grp_table_get(&table, &parent1, NULL);
    cxml_grp_table_get(NULL, &parent1, &value);
    CHECK_EQ(empty_list_asserts(&value), 1)

    // rehash
    char *tmp = "stuff";
    cxml_grp_table_put(&table, &parent3, tmp);
    cxml_grp_table_put(&table, &parent4, &child1);
    cxml_grp_table_put(&table, &parent5, tmp);
    cxml_grp_table_put(&table, &parent5, tmp);
    cxml_grp_table_put(&table, &parent5, tmp);
    cxml_grp_table_put(&table, &parent5, tmp);
    cxml_grp_table_put(&table, &parent5, tmp);
    cxml_grp_table_put(&table, &parent6, &child2);
    cxml_grp_table_put(&table, &parent6, &child2);
    cxml_grp_table_put(&table, "testing", &child2);
    cxml_grp_table_put(&table, "eight", &child2);
    CHECK_EQ(cxml_grp_table_size(&table), 8)

    cxml_grp_table_get(&table, &parent5, &value);
    CHECK_EQ(cxml_list_size(&value), 5)

    {
        cxml_for_each(node, &value){
            CHECK_EQ(node, tmp)
        }
    }
    cxml_list_free(&value);
    cxml_grp_table_free(&table);

    cxml_pass()
}

cts test_cxml_grp_table_remove(){
    cxml_grp_table table = new_cxml_grp_table();
    struct Data parent1, parent2, parent3,
            parent4, parent5, parent6, parent7, child1, child2;
    char *tmp = "stuff";

    cxml_grp_table_put(&table, &parent1, &child1);
    cxml_grp_table_put(&table, &parent1, &child1);
    cxml_grp_table_put(&table, &parent1, &child1);
    cxml_grp_table_put(&table, &parent2, &child2);
    cxml_grp_table_put(&table, &parent2, &child2);
    cxml_grp_table_put(&table, &parent2, &child2);
    cxml_grp_table_put(&table, &parent2, &child2);
    cxml_grp_table_put(&table, &parent3, tmp);
    cxml_grp_table_put(&table, &parent4, &child1);

    CHECK_EQ(cxml_grp_table_size(&table), 4)

    // delete all
    cxml_grp_table_remove(&table, &parent4);
    CHECK_EQ(cxml_grp_table_size(&table), 3)
    cxml_grp_table_remove(&table, &parent3);
    CHECK_EQ(cxml_grp_table_size(&table), 2)
    cxml_grp_table_remove(&table, &parent3);  // already deleted.
    CHECK_EQ(cxml_grp_table_size(&table), 2)
    cxml_grp_table_remove(&table, &parent2);
    CHECK_EQ(cxml_grp_table_size(&table), 1)
    cxml_grp_table_remove(&table, &parent1);
    CHECK_EQ(cxml_grp_table_size(&table), 0)

    // put 4
    cxml_grp_table_put(&table, &parent5, tmp);
    cxml_grp_table_put(&table, &parent6, &child2);
    cxml_grp_table_put(&table, &parent6, &child2);
    cxml_grp_table_put(&table, tmp, &child2);
    cxml_grp_table_put(&table, &parent7, &child2);
    cxml_grp_table_put(&table, tmp, &child2);
    CHECK_EQ(cxml_grp_table_size(&table), 4)

    // delete 2
    cxml_grp_table_remove(&table, tmp);
    CHECK_EQ(cxml_grp_table_size(&table), 3)
    cxml_grp_table_remove(&table, tmp);  // already deleted.
    CHECK_EQ(cxml_grp_table_size(&table), 3)
    cxml_grp_table_remove(&table, &parent6);
    CHECK_EQ(cxml_grp_table_size(&table), 2)

    // error
    cxml_grp_table_remove(&table, NULL);
    CHECK_EQ(cxml_grp_table_size(&table), 2)
    cxml_grp_table_remove(NULL, &parent7);
    CHECK_EQ(cxml_grp_table_size(&table), 2)
    cxml_grp_table_remove(&table, &parent3);  // not in
    CHECK_EQ(cxml_grp_table_size(&table), 2)
    cxml_grp_table_remove(&table, "FoobarIsland");  // not in
    CHECK_EQ(cxml_grp_table_size(&table), 2)

    // remove
    cxml_grp_table_put(&table, &parent1, &child1);
    // rehash occurs here, with capacity smartly preserved - table remains un-bloated.
    // 5 items (3 live, 2 deleted) (+ 1 for current) 8 * .75 -> 6 <- threshold
    cxml_grp_table_put(&table, &parent2, &child2);
    cxml_grp_table_put(&table, &child1, &child2);
    CHECK_EQ(cxml_grp_table_size(&table), 5)

    // threshold exceeded, 5 items (live) + current (1 item live) -> 6 | rehash
    cxml_grp_table_put(&table, &parent3, &child1);
    CHECK_EQ(table.capacity, 16)
    CHECK_EQ(cxml_grp_table_size(&table), 6)

    cxml_grp_table_free(&table);
    cxml_grp_table_put(&table, &parent1, &child1);
    cxml_grp_table_put(&table, &parent1, &child1);
    cxml_grp_table_put(&table, &parent2, &child2);
    cxml_grp_table_put(&table, &parent2, &child2);
    cxml_grp_table_put(&table, &parent3, tmp);
    cxml_grp_table_put(&table, &parent4, &child1);
    cxml_grp_table_put(&table, &parent5, tmp);
    CHECK_EQ(table.capacity, 8)
    CHECK_EQ(cxml_grp_table_size(&table), 5)
    // 4 items (live) + 1 (current::to-be-deleted)

    cxml_grp_table_remove(&table, &parent2);
    CHECK_EQ(cxml_grp_table_size(&table), 4)

    // 4 items (live) + 1 (deleted) + current (1 item live)
    cxml_grp_table_put(&table, tmp, tmp);
    // no smart savings, since this is already >= to the savings threshold (_CXML_HT_LOAD_FACTOR_AC)
    CHECK_EQ(table.capacity, 16)
    CHECK_EQ(cxml_grp_table_size(&table), 5)
    cxml_grp_table_free(&table);
    cxml_pass()
}

cts test_cxml_grp_table_is_empty(){
    cxml_grp_table table = new_cxml_grp_table();
    struct Data parent, child1, child2;

    CHECK_TRUE(cxml_grp_table_is_empty(&table))

    cxml_grp_table_put(&table, &parent, &child1);
    cxml_grp_table_put(&table, &parent, &child2);
    CHECK_FALSE(cxml_grp_table_is_empty(&table))
    CHECK_TRUE(cxml_grp_table_is_empty(NULL))

    cxml_grp_table_free(&table);
    cxml_pass()
}

cts test_cxml_grp_table_size(){
    cxml_grp_table table = new_cxml_grp_table();
    struct Data parent1, parent2, child1, child2;

    cxml_grp_table_put(&table, &parent1, &child1);
    cxml_grp_table_put(&table, &parent1, &child2);
    CHECK_EQ(cxml_grp_table_size(&table), 1)

    cxml_grp_table_put(&table, &parent2, &child1);
    cxml_grp_table_put(&table, &parent2, &child2);
    CHECK_EQ(cxml_grp_table_size(&table), 2)

    cxml_grp_table_put(&table, &parent2, NULL);
    cxml_grp_table_put(&table, NULL, &child2);
    cxml_grp_table_put(NULL, &parent2, &child2);
    CHECK_EQ(cxml_grp_table_size(&table), 2)

    cxml_grp_table_free(&table);
    cxml_pass()
}

cts test_cxml_grp_table_free(){
    cxml_grp_table table = new_cxml_grp_table();
    struct Data parent, child1, child2;

    cxml_grp_table_put(&table, &parent, &child1);
    cxml_grp_table_put(&table, &parent, &child2);

    cxml_grp_table_free(&table);
    CHECK_EQ(empty_grp_table_asserts(&table), 1)

    cxml_grp_table_free(&table);
    CHECK_EQ(empty_grp_table_asserts(&table), 1)
    cxml_pass()
}


void suite_cxgrptable() {
    cxml_suite(cxgrptable)
    {
        cxml_add_m_test(8,
                        test_cxml_grp_table_init,
                        test_new_cxml_grp_table,
                        test_cxml_grp_table_put,
                        test_cxml_grp_table_get,
                        test_cxml_grp_table_remove,
                        test_cxml_grp_table_is_empty,
                        test_cxml_grp_table_size,
                        test_cxml_grp_table_free
        )
        cxml_run_suite()
    }
}
