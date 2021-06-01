/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"
#include <Muon/Muon.h>

void empty_table_asserts(cxml_table *table){
    CHECK_EQ(table->count, 0);
    CHECK_EQ(table->capacity, 0);
    CHECK_NULL(table->entries);
    CHECK_EQ(table->keys.len, 0);
    CHECK_NULL(table->keys.head);
    CHECK_NULL(table->keys.tail);
}

TEST(cxtable, cxml_table_free){
    cxml_table table = new_cxml_table();
    cxml_table_put(&table, "foo", "bar");
    CHECK_EQ(table.count, 1);
    CHECK_EQ(table.capacity, 8);
    for (int i=0; i < 3; i++){
        cxml_table_free(&table);
        empty_table_asserts(&table);
    }
}

TEST(cxtable, new_cxml_table){
    cxml_table table = new_cxml_table();
    empty_table_asserts(&table);
}

TEST(cxtable, new_alloc_cxml_table){
    cxml_table *table = new_alloc_cxml_table();
    empty_table_asserts(table);
    FREE(table);
}

TEST(cxtable, cxml_table_init){
    cxml_table table;
    cxml_table_init(&table);
    empty_table_asserts(&table);
    // should not seg-fault
    cxml_table_init(NULL);
}

TEST(cxtable, cxml_table_put){
    cxml_table table = new_cxml_table();
    // put
    CHECK_EQ(cxml_table_put(&table, "project-1", "cxml test"), 1);
    CHECK_EQ(cxml_table_put(&table, "project-2", "bug fixes"), 1);
    CHECK_EQ(cxml_table_size(&table), 2);
    CHECK_NOT_NULL(cxml_list_first(&table.keys));
    CHECK_NOT_NULL(cxml_list_last(&table.keys));
    CHECK_EQ(cxml_list_size(&table.keys), 2);

    // error
    CHECK_EQ(cxml_table_put(&table, "project-3", NULL), 0);
    CHECK_EQ(cxml_table_put(&table, NULL, "code cleanup"), 0);
    CHECK_EQ(cxml_table_size(&table), 2);

    // update
    CHECK_EQ(cxml_table_put(&table, "project-2", "round up"), 2);
    CHECK_EQ(cxml_table_put(&table, "project-1", "finishes"), 2);
    CHECK_EQ(cxml_table_size(&table), 2);

    // rehash
    CHECK_EQ(cxml_table_put(&table, "test-key-2", "bug fixes"), 1);
    CHECK_EQ(cxml_table_put(&table, "lorem ipsum", "dolor sit amet"), 1);
    CHECK_EQ(cxml_table_put(&table, "adipiscing elit", "adipiscing elit"), 1);
    CHECK_EQ(cxml_table_put(&table, "tristique sem", "1234"), 1);
    CHECK_EQ(cxml_table_put(&table, "a", "a"), 1);
    CHECK_EQ(cxml_table_put(&table, "\0", "Duis a aliquet libero"), 1);
    CHECK_EQ(cxml_table_put(&table, "  in iaculis ", "  vitae efficitur"), 1);
    CHECK_EQ(cxml_table_put(&table, "00000", "11111"), 1);
    CHECK_EQ(cxml_table_put(&table, "0xdeadbeef", "0xaceface"), 1);

    CHECK_EQ(cxml_table_size(&table), 11);
    CHECK_EQ(cxml_list_size(&table.keys), 11);
    CHECK_NOT_NULL(cxml_list_first(&table.keys));
    CHECK_NOT_NULL(cxml_list_last(&table.keys));
    cxml_table_free(&table);
}

TEST(cxtable, cxml_table_put_raw){
    cxml_table table = new_cxml_table();
    struct Data d1, d2, d3, d4, d5, d6, d7, d8;
    // put
    CHECK_EQ(cxml_table_put_raw(&table, &d1, "cxml test"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d2, "bug fixes"), 1);
    CHECK_EQ(cxml_table_size(&table), 2);

    // error
    CHECK_EQ(cxml_table_put_raw(&table, &d2, NULL), 0);
    CHECK_EQ(cxml_table_put_raw(&table, NULL, "code cleanup"), 0);
    CHECK_EQ(cxml_table_size(&table), 2);

    // update
    CHECK_EQ(cxml_table_put_raw(&table, &d2, "round up"), 2);
    CHECK_EQ(cxml_table_put_raw(&table, &d1, &d3), 2);
    CHECK_EQ(cxml_table_size(&table), 2);

    // rehash
    CHECK_EQ(cxml_table_put_raw(&table, &d4, &d4), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d3, &d1), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d5, "adipiscing elit"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d6, "1234"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d7, "a"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, "00000", "Duis a aliquet libero"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, "  in iaculis ", "  vitae efficitur"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d8, "11111"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, "0xdeadbeef", "0xaceface"), 1);

    CHECK_EQ(cxml_table_size(&table), 11);
    cxml_table_free(&table);
}

TEST(cxtable, cxml_table_remove){
    cxml_table table = new_cxml_table();
    CHECK_EQ(cxml_table_put(&table, "simple", "bug fixes"), 1);
    CHECK_EQ(cxml_table_put(&table, "lorem ipsum", "dolor sit amet"), 1);
    CHECK_EQ(cxml_table_put(&table, "elite", "adipiscing elit"), 1);
    CHECK_EQ(cxml_table_put(&table, "very good stuff", "1234"), 1);
    CHECK_EQ(cxml_table_size(&table), 4);
    CHECK_EQ(table.capacity, 8);

    // delete all
    cxml_table_remove(&table, "lorem ipsum");
    CHECK_EQ(cxml_table_size(&table), 3);
    cxml_table_remove(&table, "elite");
    CHECK_EQ(cxml_table_size(&table), 2);
    cxml_table_remove(&table, "very good stuff");
    CHECK_EQ(cxml_table_size(&table), 1);
    cxml_table_remove(&table, "simple");
    CHECK_EQ(cxml_table_size(&table), 0);
    CHECK_EQ(table.capacity, 8);

    // put 4 items
    CHECK_EQ(cxml_table_put(&table, "simple", "bug fixes"), 1);
    CHECK_EQ(cxml_table_put(&table, "lorem ipsum", "dolor sit amet"), 1);
    CHECK_EQ(cxml_table_put(&table, "elite", "adipiscing elit"), 1);
    CHECK_EQ(cxml_table_put(&table, "very good stuff", "1234"), 1);
    CHECK_EQ(cxml_table_size(&table), 4);
    // table will not be bloated - capacity still remains the same, even after rehashing
    CHECK_EQ(table.capacity, 8);


    // delete 2 items
    cxml_table_remove(&table, "very good stuff");
    CHECK_EQ(cxml_table_size(&table), 3);
    cxml_table_remove(&table, "simple");
    CHECK_EQ(cxml_table_size(&table), 2);
    CHECK_EQ(table.capacity, 8);

    // delete - error
    cxml_table_remove(&table, "simpless");
    CHECK_EQ(cxml_table_size(&table), 2);
    cxml_table_remove(&table, NULL);
    CHECK_EQ(cxml_table_size(&table), 2);
    cxml_table_remove(&table, "simple");
    CHECK_EQ(cxml_table_size(&table), 2);
    cxml_table_remove(NULL, "simple");
    CHECK_EQ(cxml_table_size(&table), 2);

    CHECK_EQ(cxml_table_put(&table, "xml parser", "a"), 1);
    // rehash occurs here, with capacity smartly preserved - table remains un-bloated.
    // 5 items (3 live, 2 deleted) (+ 1 for current) 8 * .75 -> 6 <- threshold
    CHECK_EQ(cxml_table_put(&table, "foo", "bar"), 1);  // 5 items (3 live, 2 deleted) + current (1 item live, 1);
    CHECK_EQ(cxml_table_put(&table, "powerful", "dissapointing"), 1); // 5 items (live, 1);
    CHECK_EQ(cxml_table_size(&table), 5);
    // threshold exceeded, 5 items (live) + current (1 item live) -> 6 | rehash
    CHECK_EQ(cxml_table_put(&table, "foofoo", "delicious"), 1);
    CHECK_EQ(table.capacity, 16);
    CHECK_EQ(cxml_table_size(&table), 6);

    cxml_table_free(&table);
    CHECK_EQ(cxml_table_put(&table, "simple", "bug fixes"), 1);
    CHECK_EQ(cxml_table_put(&table, "lorem ipsum", "dolor sit amet"), 1);
    CHECK_EQ(cxml_table_put(&table, "elite", "adipiscing elit"), 1);
    CHECK_EQ(cxml_table_put(&table, "very good stuff", "1234"), 1);
    CHECK_EQ(cxml_table_put(&table, "sweet", "bug fixes"), 1);
    CHECK_EQ(table.capacity, 8);
    CHECK_EQ(cxml_table_size(&table), 5);
    // 4 items (live) + 1 (current::to-be-deleted)
    cxml_table_remove(&table, "simple");
    CHECK_EQ(cxml_table_size(&table), 4);
    // 4 items (live) + 1 (deleted) + current (1 item live)
    CHECK_EQ(cxml_table_put(&table, "powerful", "dissapointing"), 1); // 5 items (live, 1);
    // no smart savings, since this is already >= to the savings threshold (_CXML_HT_LOAD_FACTOR_AC)
    CHECK_EQ(table.capacity, 16);
    CHECK_EQ(cxml_table_size(&table), 5);
    cxml_table_free(&table);
}

TEST(cxtable, cxml_table_remove_raw){
    cxml_table table = new_cxml_table();
    struct Data d1, d2, d3, d4, d5, d6, d7, d8;
    CHECK_EQ(cxml_table_put_raw(&table, &d1, "bug fixes"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d2, "dolor sit amet"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d3, "adipiscing elit"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d4, "1234"), 1);
    CHECK_EQ(cxml_table_size(&table), 4);
    CHECK_EQ(table.capacity, 8);

    // delete all
    cxml_table_remove_raw(&table, &d2);
    CHECK_EQ(cxml_table_size(&table), 3);
    cxml_table_remove_raw(&table, &d3);
    CHECK_EQ(cxml_table_size(&table), 2);
    cxml_table_remove_raw(&table, &d1);
    CHECK_EQ(cxml_table_size(&table), 1);
    cxml_table_remove_raw(&table, &d4);
    CHECK_EQ(cxml_table_size(&table), 0);
    CHECK_EQ(table.capacity, 8);

    // put 4 items
    CHECK_EQ(cxml_table_put_raw(&table, &d1, "bug fixes"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d2, "dolor sit amet"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d3, "adipiscing elit"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d4, "1234"), 1);
    CHECK_EQ(cxml_table_size(&table), 4);
    // table will not be bloated - capacity still remains the same, even after rehashing
    CHECK_EQ(table.capacity, 8);


    // delete 2 items
    cxml_table_remove_raw(&table, &d1);
    CHECK_EQ(cxml_table_size(&table), 3);
    cxml_table_remove_raw(&table, &d3);
    CHECK_EQ(cxml_table_size(&table), 2);
    CHECK_EQ(table.capacity, 8);

    // delete - error
    cxml_table_remove_raw(&table, "simpless");
    CHECK_EQ(cxml_table_size(&table), 2);
    cxml_table_remove_raw(&table, NULL);
    CHECK_EQ(cxml_table_size(&table), 2);
    cxml_table_remove_raw(NULL, &d1);
    CHECK_EQ(cxml_table_size(&table), 2);
    cxml_table_remove_raw(&table, &d5); // d5 not in table
    CHECK_EQ(cxml_table_size(&table), 2);
    cxml_table_remove_raw(&table, &d3); // d3 not in table -> already deleted
    CHECK_EQ(cxml_table_size(&table), 2);
    cxml_table_remove_raw(&table, &d1); // d1 not in table -> already deleted
    CHECK_EQ(cxml_table_size(&table), 2);

    CHECK_EQ(cxml_table_put_raw(&table, &d6, "a"), 1);
    // rehash occurs here, with capacity smartly preserved - table remains un-bloated.
    // 5 items (3 live, 2 deleted) (+ 1 for current) 8 * .75 -> 6 <- threshold
    CHECK_EQ(cxml_table_put_raw(&table, &d7, "bar"), 1);  // 5 items (3 live, 2 deleted) + current (1 item live, 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d8, &d1), 1); // 5 items (live, 1);
    CHECK_EQ(cxml_table_size(&table), 5);
    // threshold exceeded, 5 items (live) + current (1 item live) -> 6 | rehash
    CHECK_EQ(cxml_table_put_raw(&table, &d3, &d2), 1);
    CHECK_EQ(table.capacity, 16);
    CHECK_EQ(cxml_table_size(&table), 6);

    cxml_table_free(&table);
    CHECK_EQ(cxml_table_put_raw(&table, &d2, &d2), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d5, &d2), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d6, &d6), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d7, &d1), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d3, "bug fixes"), 1);
    CHECK_EQ(table.capacity, 8);
    CHECK_EQ(cxml_table_size(&table), 5);
    // 4 items (live) + 1 (current::to-be-deleted)
    cxml_table_remove_raw(&table, &d2);
    CHECK_EQ(cxml_table_size(&table), 4);
    // 4 items (live) + 1 (deleted) + current (1 item live)
    CHECK_EQ(cxml_table_put_raw(&table, &d8, "dissapointing"), 1); // 5 items (live, 1);
    // no smart savings, since this is already >= to the savings threshold (_CXML_HT_LOAD_FACTOR_AC)
    CHECK_EQ(table.capacity, 16);
    CHECK_EQ(cxml_table_size(&table), 5);
    cxml_table_free(&table);
}

TEST(cxtable, cxml_table_get){
    cxml_table table = new_cxml_table();
    char *v;
    // put
    CHECK_EQ(cxml_table_put(&table, "project-1", "cxml test"), 1);
    CHECK_EQ(cxml_table_put(&table, "project-2", "bug fixes"), 1);

    CHECK_NOT_NULL((v = cxml_table_get(&table, "project-1")));
    CHECK_EQ(strcmp(v, "cxml test"), 0);
    CHECK_NOT_NULL((v = cxml_table_get(&table, "project-2")));
    CHECK_EQ(strcmp(v, "bug fixes"), 0);

    // error
    CHECK_NULL(cxml_table_get(&table, "project"));
    CHECK_NULL(cxml_table_get(&table, NULL));
    CHECK_NULL(cxml_table_get(&table, "code cleanup"));
    CHECK_NULL(cxml_table_get(NULL, "code cleanup"));

    // update
    CHECK_EQ(cxml_table_put(&table, "project-2", "round up"), 2);
    CHECK_EQ(cxml_table_put(&table, "project-1", "finishes"), 2);

    CHECK_NOT_NULL((v = cxml_table_get(&table, "project-1")));
    CHECK_EQ(strcmp(v, "finishes"), 0);
    CHECK_NOT_NULL((v = cxml_table_get(&table, "project-2")));
    CHECK_EQ(strcmp(v, "round up"), 0);

    // rehash
    CHECK_EQ(cxml_table_put(&table, "test-key-2", "bug fixes"), 1);
    CHECK_EQ(cxml_table_put(&table, "lorem ipsum", "dolor sit amet"), 1);
    CHECK_EQ(cxml_table_put(&table, "adipiscing elit", "adipiscing elit"), 1);
    CHECK_EQ(cxml_table_put(&table, "tristique sem", "1234"), 1);
    CHECK_EQ(cxml_table_put(&table, "a", "a"), 1);
    CHECK_EQ(cxml_table_put(&table, "", "Duis a aliquet libero"), 1);
    CHECK_EQ(cxml_table_put(&table, "  in iaculis ", "  vitae efficitur"), 1);
    CHECK_EQ(cxml_table_size(&table), 9);

    CHECK_NOT_NULL((v = cxml_table_get(&table, "test-key-2")));
    CHECK_EQ(strcmp(v, "bug fixes"), 0);
    CHECK_NOT_NULL((v = cxml_table_get(&table, "adipiscing elit")));
    CHECK_EQ(strcmp(v, "adipiscing elit"), 0);
    CHECK_NOT_NULL((v = cxml_table_get(&table, "lorem ipsum")));
    CHECK_EQ(strcmp(v, "dolor sit amet"), 0);
    CHECK_NOT_NULL((v = cxml_table_get(&table, "tristique sem")));
    CHECK_EQ(strcmp(v, "1234"), 0);
    CHECK_NOT_NULL((v = cxml_table_get(&table, "a")));
    CHECK_EQ(strcmp(v, "a"), 0);
    CHECK_NOT_NULL((v = cxml_table_get(&table, "\0")));
    CHECK_EQ(strcmp(v, "Duis a aliquet libero"), 0);
    CHECK_NOT_NULL((v = cxml_table_get(&table, "  in iaculis ")));
    CHECK_EQ(strcmp(v, "  vitae efficitur"), 0);
    CHECK_NOT_NULL((v = cxml_table_get(&table, "")));
    CHECK_EQ(strcmp(v, "Duis a aliquet libero"), 0);

    CHECK_NULL(cxml_table_get(NULL, "  in iaculis"));
    CHECK_NULL(cxml_table_get(&table, "12345"));

    CHECK_EQ(cxml_table_put(&table, "खुशियों की बरसातें", "Jerry"), 1);
    CHECK_NOT_NULL((v = cxml_table_get(&table, "खुशियों की बरसातें")));
    CHECK_EQ(strcmp(v, "Jerry"), 0);

    cxml_table_free(&table);
}

TEST(cxtable, cxml_table_get_raw){
    cxml_table table = new_cxml_table();
    struct Data d1, d2, d3, d4, d5, d6, d7, d8;
    char *k1 = "abc", *k2 = "step-up", *k3 = "\0", *k4 = "";
    char *c;
    void *v;
    // put
    CHECK_EQ(cxml_table_put_raw(&table, &d1, "cxml test"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d2, "bug fixes"), 1);

    CHECK_NOT_NULL((v = cxml_table_get_raw(&table, &d1)));
    CHECK_EQ(strcmp(v, "cxml test"), 0);
    CHECK_NOT_NULL((v = cxml_table_get_raw(&table, &d2)));
    CHECK_EQ(strcmp(v, "bug fixes"), 0);

    // error
    CHECK_NULL(cxml_table_get_raw(&table, &d3));
    CHECK_NULL(cxml_table_get_raw(&table, "project"));
    CHECK_NULL(cxml_table_get_raw(&table, NULL));
    CHECK_NULL(cxml_table_get_raw(&table, "code cleanup"));
    CHECK_NULL(cxml_table_get_raw(NULL, "code cleanup"));

    // update
    CHECK_EQ(cxml_table_put_raw(&table, &d2, "round up"), 2);
    CHECK_EQ(cxml_table_put_raw(&table, &d1, "finishes"), 2);

    CHECK_NOT_NULL((v = cxml_table_get_raw(&table, &d1)));
    CHECK_EQ(strcmp(v, "finishes"), 0);
    CHECK_NOT_NULL((v = cxml_table_get_raw(&table, &d2)));
    CHECK_EQ(strcmp(v, "round up"), 0);

    // rehash
    CHECK_EQ(cxml_table_put_raw(&table, &d3, &d3), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d4, &d1), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d5, &d5), 1);
    CHECK_EQ(cxml_table_put_raw(&table, &d6, &d2), 1);
    CHECK_EQ(cxml_table_put_raw(&table, k1, &d7), 1);
    CHECK_EQ(cxml_table_put_raw(&table, k3, &d3), 1);
    CHECK_EQ(cxml_table_put_raw(&table, k2, "UP!"), 1);
    CHECK_EQ(cxml_table_put_raw(&table, k4, ""), 1);
    CHECK_EQ(cxml_table_put_raw(&table, k4, "foo"), 2);
    CHECK_EQ(cxml_table_size(&table), 10);

    CHECK_NOT_NULL((v = cxml_table_get_raw(&table, &d3)));
    CHECK_EQ(v, &d3);
    CHECK_EQ(((struct Data*)v), &d3);
    CHECK_NOT_NULL((v = cxml_table_get_raw(&table, &d4)));
    CHECK_EQ(v, &d1);
    CHECK_NOT_NULL((v = cxml_table_get_raw(&table, &d5)));
    CHECK_EQ(v, &d5);
    CHECK_NOT_NULL((v = cxml_table_get_raw(&table, &d6)));
    CHECK_EQ(v, &d2);
    CHECK_NOT_NULL((v = cxml_table_get_raw(&table, k1)));
    CHECK_EQ(v, &d7);
    CHECK_NOT_NULL((v = cxml_table_get_raw(&table, k3)));
    CHECK_EQ(v, &d3);
    CHECK_NOT_NULL((c = cxml_table_get_raw(&table, k4)));
    CHECK_EQ(strcmp(c, "foo"), 0);
    CHECK_NOT_NULL((v = cxml_table_get_raw(&table, k1)));
    CHECK_EQ(v, &d7);
    CHECK_EQ(cxml_table_put_raw(&table, k1, &d8), 2);
    CHECK_NOT_NULL((v = cxml_table_get_raw(&table, k1)));
    CHECK_EQ(v, &d8);
    CHECK_NOT_NULL((c = cxml_table_get_raw(&table, k2)));
    CHECK_EQ(strcmp(c, "UP!"), 0);

    CHECK_NULL(cxml_table_get_raw(NULL, "  in iaculis"));
    CHECK_NULL(cxml_table_get_raw(&table, &d8));
    cxml_table_free(&table);
}

TEST(cxtable, cxml_table_is_empty){
    cxml_table table = new_cxml_table();
    CHECK_TRUE(cxml_table_is_empty(&table));
    CHECK_TRUE(cxml_table_is_empty(NULL));
    cxml_table_put(&table, "--cxml--", "C XML Minimalistic Library");
    CHECK_FALSE(cxml_table_is_empty(&table));
    cxml_table_free(&table);
}

TEST(cxtable, cxml_table_size){
    cxml_table table = new_cxml_table();
    CHECK_EQ(cxml_table_size(&table), 0);
    CHECK_EQ(cxml_table_size(NULL), 0);
    cxml_table_put(&table, "cxml", "xml library for C");
    CHECK_EQ(cxml_table_size(&table), 1);
    cxml_table_free(&table);
}