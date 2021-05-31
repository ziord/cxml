/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

int empty_table_asserts(cxml_table *table){
    cxml_assert__zero(table->count)
    cxml_assert__zero(table->capacity)
    cxml_assert__null(table->entries)
    cxml_assert__zero(table->keys.len)
    cxml_assert__null(table->keys.head)
    cxml_assert__null(table->keys.tail)
    return 1;
}

cts test_cxml_table_free(){
    cxml_table table = new_cxml_table();
    cxml_table_put(&table, "foo", "bar");
    cxml_assert__one(table.count)
    cxml_assert__eq(table.capacity, 8)
    for (int i=0; i < 3; i++){
        cxml_table_free(&table);
        cxml_assert__one(empty_table_asserts(&table))
    }
    cxml_pass()
}

cts test_new_cxml_table(){
    cxml_table table = new_cxml_table();
    cxml_assert__one(empty_table_asserts(&table))
    cxml_pass()
}

cts test_new_alloc_cxml_table(){
    cxml_table *table = new_alloc_cxml_table();
    cxml_assert__one(empty_table_asserts(table))
    FREE(table);
    cxml_pass()
}

cts test_cxml_table_init(){
    cxml_table table;
    cxml_table_init(&table);
    cxml_assert__one(empty_table_asserts(&table))
    // should not seg-fault
    cxml_table_init(NULL);
    cxml_pass()
}

cts test_cxml_table_put(){
    cxml_table table = new_cxml_table();
    // put
    cxml_assert__one(cxml_table_put(&table, "project-1", "cxml test"))
    cxml_assert__one(cxml_table_put(&table, "project-2", "bug fixes"))
    cxml_assert__two(cxml_table_size(&table))
    cxml_assert__not_null(cxml_list_first(&table.keys))
    cxml_assert__not_null(cxml_list_last(&table.keys))
    cxml_assert__two(cxml_list_size(&table.keys))

    // error
    cxml_assert__zero(cxml_table_put(&table, "project-3", NULL))
    cxml_assert__zero(cxml_table_put(&table, NULL, "code cleanup"))
    cxml_assert__two(cxml_table_size(&table))

    // update
    cxml_assert__two(cxml_table_put(&table, "project-2", "round up"))
    cxml_assert__two(cxml_table_put(&table, "project-1", "finishes"))
    cxml_assert__two(cxml_table_size(&table))

    // rehash
    cxml_assert__one(cxml_table_put(&table, "test-key-2", "bug fixes"))
    cxml_assert__one(cxml_table_put(&table, "lorem ipsum", "dolor sit amet"))
    cxml_assert__one(cxml_table_put(&table, "adipiscing elit", "adipiscing elit"))
    cxml_assert__one(cxml_table_put(&table, "tristique sem", "1234"))
    cxml_assert__one(cxml_table_put(&table, "a", "a"))
    cxml_assert__one(cxml_table_put(&table, "\0", "Duis a aliquet libero"))
    cxml_assert__one(cxml_table_put(&table, "  in iaculis ", "  vitae efficitur"))
    cxml_assert__one(cxml_table_put(&table, "00000", "11111"))
    cxml_assert__one(cxml_table_put(&table, "0xdeadbeef", "0xaceface"))

    cxml_assert__eq(cxml_table_size(&table), 11)
    cxml_assert__eq(cxml_list_size(&table.keys), 11)
    cxml_assert__not_null(cxml_list_first(&table.keys))
    cxml_assert__not_null(cxml_list_last(&table.keys))
    cxml_table_free(&table);
    cxml_pass()
}

cts test_cxml_table_put_raw(){
    cxml_table table = new_cxml_table();
    struct Data d1, d2, d3, d4, d5, d6, d7, d8;
    // put
    cxml_assert__one(cxml_table_put_raw(&table, &d1, "cxml test"))
    cxml_assert__one(cxml_table_put_raw(&table, &d2, "bug fixes"))
    cxml_assert__two(cxml_table_size(&table))

    // error
    cxml_assert__zero(cxml_table_put_raw(&table, &d2, NULL))
    cxml_assert__zero(cxml_table_put_raw(&table, NULL, "code cleanup"))
    cxml_assert__two(cxml_table_size(&table))

    // update
    cxml_assert__two(cxml_table_put_raw(&table, &d2, "round up"))
    cxml_assert__two(cxml_table_put_raw(&table, &d1, &d3))
    cxml_assert__two(cxml_table_size(&table))

    // rehash
    cxml_assert__one(cxml_table_put_raw(&table, &d4, &d4))
    cxml_assert__one(cxml_table_put_raw(&table, &d3, &d1))
    cxml_assert__one(cxml_table_put_raw(&table, &d5, "adipiscing elit"))
    cxml_assert__one(cxml_table_put_raw(&table, &d6, "1234"))
    cxml_assert__one(cxml_table_put_raw(&table, &d7, "a"))
    cxml_assert__one(cxml_table_put_raw(&table, "00000", "Duis a aliquet libero"))
    cxml_assert__one(cxml_table_put_raw(&table, "  in iaculis ", "  vitae efficitur"))
    cxml_assert__one(cxml_table_put_raw(&table, &d8, "11111"))
    cxml_assert__one(cxml_table_put_raw(&table, "0xdeadbeef", "0xaceface"))

    cxml_assert__eq(cxml_table_size(&table), 11)
    cxml_table_free(&table);
    cxml_pass()
}

cts test_cxml_table_remove(){
    cxml_table table = new_cxml_table();
    cxml_assert__one(cxml_table_put(&table, "simple", "bug fixes"))
    cxml_assert__one(cxml_table_put(&table, "lorem ipsum", "dolor sit amet"))
    cxml_assert__one(cxml_table_put(&table, "elite", "adipiscing elit"))
    cxml_assert__one(cxml_table_put(&table, "very good stuff", "1234"))
    cxml_assert__eq(cxml_table_size(&table), 4)
    cxml_assert__eq(table.capacity, 8)

    // delete all
    cxml_table_remove(&table, "lorem ipsum");
    cxml_assert__eq(cxml_table_size(&table), 3)
    cxml_table_remove(&table, "elite");
    cxml_assert__eq(cxml_table_size(&table), 2)
    cxml_table_remove(&table, "very good stuff");
    cxml_assert__eq(cxml_table_size(&table), 1)
    cxml_table_remove(&table, "simple");
    cxml_assert__zero(cxml_table_size(&table))
    cxml_assert__eq(table.capacity, 8)

    // put 4 items
    cxml_assert__one(cxml_table_put(&table, "simple", "bug fixes"))
    cxml_assert__one(cxml_table_put(&table, "lorem ipsum", "dolor sit amet"))
    cxml_assert__one(cxml_table_put(&table, "elite", "adipiscing elit"))
    cxml_assert__one(cxml_table_put(&table, "very good stuff", "1234"))
    cxml_assert__eq(cxml_table_size(&table), 4)
    // table will not be bloated - capacity still remains the same, even after rehashing
    cxml_assert__eq(table.capacity, 8)


    // delete 2 items
    cxml_table_remove(&table, "very good stuff");
    cxml_assert__eq(cxml_table_size(&table), 3)
    cxml_table_remove(&table, "simple");
    cxml_assert__eq(cxml_table_size(&table), 2)
    cxml_assert__eq(table.capacity, 8)

    // delete - error
    cxml_table_remove(&table, "simpless");
    cxml_assert__eq(cxml_table_size(&table), 2)
    cxml_table_remove(&table, NULL);
    cxml_assert__eq(cxml_table_size(&table), 2)
    cxml_table_remove(&table, "simple");
    cxml_assert__eq(cxml_table_size(&table), 2)
    cxml_table_remove(NULL, "simple");
    cxml_assert__eq(cxml_table_size(&table), 2)

    cxml_assert__one(cxml_table_put(&table, "xml parser", "a"))
    // rehash occurs here, with capacity smartly preserved - table remains un-bloated.
    // 5 items (3 live, 2 deleted) (+ 1 for current) 8 * .75 -> 6 <- threshold
    cxml_assert__one(cxml_table_put(&table, "foo", "bar"))  // 5 items (3 live, 2 deleted) + current (1 item live)
    cxml_assert__one(cxml_table_put(&table, "powerful", "dissapointing")) // 5 items (live)
    cxml_assert__eq(cxml_table_size(&table), 5)
    // threshold exceeded, 5 items (live) + current (1 item live) -> 6 | rehash
    cxml_assert__one(cxml_table_put(&table, "foofoo", "delicious"))
    cxml_assert__eq(table.capacity, 16)
    cxml_assert__eq(cxml_table_size(&table), 6)

    cxml_table_free(&table);
    cxml_assert__one(cxml_table_put(&table, "simple", "bug fixes"))
    cxml_assert__one(cxml_table_put(&table, "lorem ipsum", "dolor sit amet"))
    cxml_assert__one(cxml_table_put(&table, "elite", "adipiscing elit"))
    cxml_assert__one(cxml_table_put(&table, "very good stuff", "1234"))
    cxml_assert__one(cxml_table_put(&table, "sweet", "bug fixes"))
    cxml_assert__eq(table.capacity, 8)
    cxml_assert__eq(cxml_table_size(&table), 5)
    // 4 items (live) + 1 (current::to-be-deleted)
    cxml_table_remove(&table, "simple");
    cxml_assert__eq(cxml_table_size(&table), 4)
    // 4 items (live) + 1 (deleted) + current (1 item live)
    cxml_assert__one(cxml_table_put(&table, "powerful", "dissapointing")) // 5 items (live)
    // no smart savings, since this is already >= to the savings threshold (_CXML_HT_LOAD_FACTOR_AC)
    cxml_assert__eq(table.capacity, 16)
    cxml_assert__eq(cxml_table_size(&table), 5)
    cxml_table_free(&table);
    cxml_pass()
}

cts test_cxml_table_remove_raw(){
    cxml_table table = new_cxml_table();
    struct Data d1, d2, d3, d4, d5, d6, d7, d8;
    cxml_assert__one(cxml_table_put_raw(&table, &d1, "bug fixes"))
    cxml_assert__one(cxml_table_put_raw(&table, &d2, "dolor sit amet"))
    cxml_assert__one(cxml_table_put_raw(&table, &d3, "adipiscing elit"))
    cxml_assert__one(cxml_table_put_raw(&table, &d4, "1234"))
    cxml_assert__eq(cxml_table_size(&table), 4)
    cxml_assert__eq(table.capacity, 8)

    // delete all
    cxml_table_remove_raw(&table, &d2);
    cxml_assert__eq(cxml_table_size(&table), 3)
    cxml_table_remove_raw(&table, &d3);
    cxml_assert__eq(cxml_table_size(&table), 2)
    cxml_table_remove_raw(&table, &d1);
    cxml_assert__eq(cxml_table_size(&table), 1)
    cxml_table_remove_raw(&table, &d4);
    cxml_assert__eq(cxml_table_size(&table), 0)
    cxml_assert__eq(table.capacity, 8)

    // put 4 items
    cxml_assert__one(cxml_table_put_raw(&table, &d1, "bug fixes"))
    cxml_assert__one(cxml_table_put_raw(&table, &d2, "dolor sit amet"))
    cxml_assert__one(cxml_table_put_raw(&table, &d3, "adipiscing elit"))
    cxml_assert__one(cxml_table_put_raw(&table, &d4, "1234"))
    cxml_assert__eq(cxml_table_size(&table), 4)
    // table will not be bloated - capacity still remains the same, even after rehashing
    cxml_assert__eq(table.capacity, 8)


    // delete 2 items
    cxml_table_remove_raw(&table, &d1);
    cxml_assert__eq(cxml_table_size(&table), 3)
    cxml_table_remove_raw(&table, &d3);
    cxml_assert__eq(cxml_table_size(&table), 2)
    cxml_assert__eq(table.capacity, 8)

    // delete - error
    cxml_table_remove_raw(&table, "simpless");
    cxml_assert__eq(cxml_table_size(&table), 2)
    cxml_table_remove_raw(&table, NULL);
    cxml_assert__eq(cxml_table_size(&table), 2)
    cxml_table_remove_raw(NULL, &d1);
    cxml_assert__eq(cxml_table_size(&table), 2)
    cxml_table_remove_raw(&table, &d5); // d5 not in table
    cxml_assert__eq(cxml_table_size(&table), 2)
    cxml_table_remove_raw(&table, &d3); // d3 not in table -> already deleted
    cxml_assert__eq(cxml_table_size(&table), 2)
    cxml_table_remove_raw(&table, &d1); // d1 not in table -> already deleted
    cxml_assert__eq(cxml_table_size(&table), 2)

    cxml_assert__one(cxml_table_put_raw(&table, &d6, "a"))
    // rehash occurs here, with capacity smartly preserved - table remains un-bloated.
    // 5 items (3 live, 2 deleted) (+ 1 for current) 8 * .75 -> 6 <- threshold
    cxml_assert__one(cxml_table_put_raw(&table, &d7, "bar"))  // 5 items (3 live, 2 deleted) + current (1 item live)
    cxml_assert__one(cxml_table_put_raw(&table, &d8, &d1)) // 5 items (live)
    cxml_assert__eq(cxml_table_size(&table), 5)
    // threshold exceeded, 5 items (live) + current (1 item live) -> 6 | rehash
    cxml_assert__one(cxml_table_put_raw(&table, &d3, &d2))
    cxml_assert__eq(table.capacity, 16)
    cxml_assert__eq(cxml_table_size(&table), 6)

    cxml_table_free(&table);
    cxml_assert__one(cxml_table_put_raw(&table, &d2, &d2))
    cxml_assert__one(cxml_table_put_raw(&table, &d5, &d2))
    cxml_assert__one(cxml_table_put_raw(&table, &d6, &d6))
    cxml_assert__one(cxml_table_put_raw(&table, &d7, &d1))
    cxml_assert__one(cxml_table_put_raw(&table, &d3, "bug fixes"))
    cxml_assert__eq(table.capacity, 8)
    cxml_assert__eq(cxml_table_size(&table), 5)
    // 4 items (live) + 1 (current::to-be-deleted)
    cxml_table_remove_raw(&table, &d2);
    cxml_assert__eq(cxml_table_size(&table), 4)
    // 4 items (live) + 1 (deleted) + current (1 item live)
    cxml_assert__one(cxml_table_put_raw(&table, &d8, "dissapointing")) // 5 items (live)
    // no smart savings, since this is already >= to the savings threshold (_CXML_HT_LOAD_FACTOR_AC)
    cxml_assert__eq(table.capacity, 16)
    cxml_assert__eq(cxml_table_size(&table), 5)
    cxml_table_free(&table);
    cxml_pass()
}

cts test_cxml_table_get(){
    cxml_table table = new_cxml_table();
    char *v;
    // put
    cxml_assert__one(cxml_table_put(&table, "project-1", "cxml test"))
    cxml_assert__one(cxml_table_put(&table, "project-2", "bug fixes"))

    cxml_assert__not_null((v = cxml_table_get(&table, "project-1")))
    cxml_assert__zero(strcmp(v, "cxml test"))
    cxml_assert__not_null((v = cxml_table_get(&table, "project-2")))
    cxml_assert__zero(strcmp(v, "bug fixes"))

    // error
    cxml_assert__null(cxml_table_get(&table, "project"))
    cxml_assert__null(cxml_table_get(&table, NULL))
    cxml_assert__null(cxml_table_get(&table, "code cleanup"))
    cxml_assert__null(cxml_table_get(NULL, "code cleanup"))

    // update
    cxml_assert__two(cxml_table_put(&table, "project-2", "round up"))
    cxml_assert__two(cxml_table_put(&table, "project-1", "finishes"))

    cxml_assert__not_null((v = cxml_table_get(&table, "project-1")))
    cxml_assert__zero(strcmp(v, "finishes"))
    cxml_assert__not_null((v = cxml_table_get(&table, "project-2")))
    cxml_assert__zero(strcmp(v, "round up"))

    // rehash
    cxml_assert__one(cxml_table_put(&table, "test-key-2", "bug fixes"))
    cxml_assert__one(cxml_table_put(&table, "lorem ipsum", "dolor sit amet"))
    cxml_assert__one(cxml_table_put(&table, "adipiscing elit", "adipiscing elit"))
    cxml_assert__one(cxml_table_put(&table, "tristique sem", "1234"))
    cxml_assert__one(cxml_table_put(&table, "a", "a"))
    cxml_assert__one(cxml_table_put(&table, "", "Duis a aliquet libero"))
    cxml_assert__one(cxml_table_put(&table, "  in iaculis ", "  vitae efficitur"))
    cxml_assert__eq(cxml_table_size(&table), 9)

    cxml_assert__not_null((v = cxml_table_get(&table, "test-key-2")))
    cxml_assert__zero(strcmp(v, "bug fixes"))
    cxml_assert__not_null((v = cxml_table_get(&table, "adipiscing elit")))
    cxml_assert__zero(strcmp(v, "adipiscing elit"))
    cxml_assert__not_null((v = cxml_table_get(&table, "lorem ipsum")))
    cxml_assert__zero(strcmp(v, "dolor sit amet"))
    cxml_assert__not_null((v = cxml_table_get(&table, "tristique sem")))
    cxml_assert__zero(strcmp(v, "1234"))
    cxml_assert__not_null((v = cxml_table_get(&table, "a")))
    cxml_assert__zero(strcmp(v, "a"))
    cxml_assert__not_null((v = cxml_table_get(&table, "\0")))
    cxml_assert__zero(strcmp(v, "Duis a aliquet libero"))
    cxml_assert__not_null((v = cxml_table_get(&table, "  in iaculis ")))
    cxml_assert__zero(strcmp(v, "  vitae efficitur"))
    cxml_assert__not_null((v = cxml_table_get(&table, "")))
    cxml_assert__zero(strcmp(v, "Duis a aliquet libero"))

    cxml_assert__null(cxml_table_get(NULL, "  in iaculis"))
    cxml_assert__null(cxml_table_get(&table, "12345"))

    cxml_assert__one(cxml_table_put(&table, "खुशियों की बरसातें", "Jerry"));
    cxml_assert__not_null((v = cxml_table_get(&table, "खुशियों की बरसातें")))
    cxml_assert__zero(strcmp(v, "Jerry"))

    cxml_table_free(&table);
    cxml_pass()
}

cts test_cxml_table_get_raw(){
    cxml_table table = new_cxml_table();
    struct Data d1, d2, d3, d4, d5, d6, d7, d8;
    char *k1 = "abc", *k2 = "step-up", *k3 = "\0", *k4 = "";
    char *c;
    void *v;
    // put
    cxml_assert__one(cxml_table_put_raw(&table, &d1, "cxml test"))
    cxml_assert__one(cxml_table_put_raw(&table, &d2, "bug fixes"))

    cxml_assert__not_null((v = cxml_table_get_raw(&table, &d1)))
    cxml_assert__zero(strcmp(v, "cxml test"))
    cxml_assert__not_null((v = cxml_table_get_raw(&table, &d2)))
    cxml_assert__zero(strcmp(v, "bug fixes"))

    // error
    cxml_assert__null(cxml_table_get_raw(&table, &d3))
    cxml_assert__null(cxml_table_get_raw(&table, "project"))
    cxml_assert__null(cxml_table_get_raw(&table, NULL))
    cxml_assert__null(cxml_table_get_raw(&table, "code cleanup"))
    cxml_assert__null(cxml_table_get_raw(NULL, "code cleanup"))

    // update
    cxml_assert__two(cxml_table_put_raw(&table, &d2, "round up"))
    cxml_assert__two(cxml_table_put_raw(&table, &d1, "finishes"))

    cxml_assert__not_null((v = cxml_table_get_raw(&table, &d1)))
    cxml_assert__zero(strcmp(v, "finishes"))
    cxml_assert__not_null((v = cxml_table_get_raw(&table, &d2)))
    cxml_assert__zero(strcmp(v, "round up"))

    // rehash
    cxml_assert__one(cxml_table_put_raw(&table, &d3, &d3))
    cxml_assert__one(cxml_table_put_raw(&table, &d4, &d1))
    cxml_assert__one(cxml_table_put_raw(&table, &d5, &d5))
    cxml_assert__one(cxml_table_put_raw(&table, &d6, &d2))
    cxml_assert__one(cxml_table_put_raw(&table, k1, &d7))
    cxml_assert__one(cxml_table_put_raw(&table, k3, &d3))
    cxml_assert__one(cxml_table_put_raw(&table, k2, "UP!"))
    cxml_assert__one(cxml_table_put_raw(&table, k4, ""))
    cxml_assert__two(cxml_table_put_raw(&table, k4, "foo"))
    cxml_assert__eq(cxml_table_size(&table), 10)

    cxml_assert__not_null((v = cxml_table_get_raw(&table, &d3)))
    cxml_assert__eq(v, &d3)
    cxml_assert__eq(((struct Data*)v), &d3)
    cxml_assert__not_null((v = cxml_table_get_raw(&table, &d4)))
    cxml_assert__eq(v, &d1)
    cxml_assert__not_null((v = cxml_table_get_raw(&table, &d5)))
    cxml_assert__eq(v, &d5)
    cxml_assert__not_null((v = cxml_table_get_raw(&table, &d6)))
    cxml_assert__eq(v, &d2)
    cxml_assert__not_null((v = cxml_table_get_raw(&table, k1)))
    cxml_assert__eq(v, &d7)
    cxml_assert__not_null((v = cxml_table_get_raw(&table, k3)))
    cxml_assert__eq(v, &d3)
    cxml_assert__not_null((c = cxml_table_get_raw(&table, k4)))
    cxml_assert__zero(strcmp(c, "foo"))
    cxml_assert__not_null((v = cxml_table_get_raw(&table, k1)))
    cxml_assert__eq(v, &d7)
    cxml_assert__two(cxml_table_put_raw(&table, k1, &d8))
    cxml_assert__not_null((v = cxml_table_get_raw(&table, k1)))
    cxml_assert__eq(v, &d8)
    cxml_assert__not_null((c = cxml_table_get_raw(&table, k2)))
    cxml_assert__zero(strcmp(c, "UP!"))

    cxml_assert__null(cxml_table_get_raw(NULL, "  in iaculis"))
    cxml_assert__null(cxml_table_get_raw(&table, &d8))
    cxml_table_free(&table);
    cxml_pass()
}

cts test_cxml_table_is_empty(){
    cxml_table table = new_cxml_table();
    cxml_assert__true(cxml_table_is_empty(&table))
    cxml_assert__true(cxml_table_is_empty(NULL))
    cxml_table_put(&table, "--cxml--", "C XML Minimalistic Library");
    cxml_assert__false(cxml_table_is_empty(&table));
    cxml_table_free(&table);
    cxml_pass()
}

cts test_cxml_table_size(){
    cxml_table table = new_cxml_table();
    cxml_assert__zero(cxml_table_size(&table))
    cxml_assert__zero(cxml_table_size(NULL))
    cxml_table_put(&table, "cxml", "xml library for C");
    cxml_assert__one(cxml_table_size(&table));
    cxml_table_free(&table);
    cxml_pass()
}

void suite_cxtable(){
    cxml_suite(cxtable)
    {
        cxml_add_m_test(12,
                        test_cxml_table_free,
                        test_new_cxml_table,
                        test_new_alloc_cxml_table,
                        test_cxml_table_init,
                        test_cxml_table_put,
                        test_cxml_table_put_raw,
                        test_cxml_table_remove,
                        test_cxml_table_remove_raw,
                        test_cxml_table_get,
                        test_cxml_table_get_raw,
                        test_cxml_table_is_empty,
                        test_cxml_table_size
                )
        cxml_run_suite()
    }
}
