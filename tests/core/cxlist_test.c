/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"
#include <Muon/Muon.h>


void empty_list_asserts(cxml_list *list){
    CHECK_EQ(list->len, 0);
    CHECK_NULL(list->head);
    CHECK_NULL(list->tail);
    CHECK_EQ(list->head, list->tail);
}

TEST(cxlist, new_cxml_list){
    cxml_list list = new_cxml_list();
    empty_list_asserts(&list);
}

TEST(cxlist, cxml_list_init){
    cxml_list list;
    cxml_list_init(&list);
    empty_list_asserts(&list);
    // should not seg-fault
    cxml_list_init(NULL);
}


TEST(cxlist, cxml_list_insert){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_insert(&list, &data1, true);  // insert at front
    cxml_list_insert(&list, &data2, true);  // insert at front

    CHECK_EQ(list.head->item, (&data2));
    CHECK_EQ(list.tail->item, (&data1));
    CHECK_EQ(list.len, 2);
    CHECK_NOT_NULL(list.head);
    CHECK_NE(list.head->item, list.tail->item);
    CHECK_NE(list.head, list.tail);

    cxml_list_insert(&list, &data1, false); // insert at end
    CHECK_EQ(list.tail->item, (&data1));
    CHECK_EQ(list.len, 3);

    // size doesn't change because NULL will never be inserted.
    cxml_list_insert(&list, NULL, true);
    cxml_list_insert(NULL, &data1, true);
    CHECK_EQ(list.len, 3);

    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_insert_at_index){
    cxml_list list = new_cxml_list();
    struct Data data1, data2, data3, data4;

    cxml_list_insert(&list, &data1, false);
    CHECK_EQ(list.head, list.tail);
    cxml_list_insert(&list, &data2, true);
    cxml_list_insert_at_index(&list, &data3, 1);
    cxml_list_insert_at_index(&list, &data4, 2);

    CHECK_NOT_NULL(list.head);
    CHECK_EQ(list.head->item, ((void*)&data2));
    CHECK_EQ(list.tail->item, ((void*)&data1));
    CHECK_EQ(list.head->next->item, ((void*)&data3));
    CHECK_EQ(list.head->next->next->item, ((void*)&data4));
    CHECK_EQ(list.len, 4);
    CHECK_NE(list.head->item, list.tail->item);
    CHECK_NE(list.head, list.tail);
    // test off index
    cxml_list_insert_at_index(&list, &data4, 5);
    CHECK_EQ(list.len, 4);
    // invalid data | list
    cxml_list_insert_at_index(&list, NULL, 2);
    cxml_list_insert_at_index(NULL, &data4, 2);
    CHECK_EQ(list.len, 4);
    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_append){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    CHECK_EQ(list.len, 2);
    CHECK_EQ(list.head->item, &data1);
    CHECK_EQ(list.tail->item, &data2);

    cxml_list_append(&list, NULL);
    cxml_list_append(NULL, &data1);
    CHECK_EQ(list.len, 2);

    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_extend){
    cxml_list list1 = new_cxml_list(),
              list2 = new_cxml_list();
    CHECK_EQ(list1.len, 0);
    CHECK_EQ(list2.len, 0);

    struct Data data1, data2;

    cxml_list_append(&list1, &data1);
    cxml_list_append(&list1, &data2);
    CHECK_EQ(list1.len, 2);

    cxml_list_extend(&list2, &list1);
    CHECK_EQ(list2.len, 2);
    CHECK_EQ(list2.head->item, &data1);
    CHECK_EQ(list2.tail->item, &data2);

    cxml_list_extend(&list2, NULL);
    CHECK_EQ(list2.len, 2);

    cxml_list_extend(NULL, &list1);
    CHECK_EQ(list2.len, 2);

    cxml_list_free(&list1);
    cxml_list_free(&list2);
}

TEST(cxlist, cxml_list_init_with){
    cxml_list list1 = new_cxml_list(),
            list2 = new_cxml_list();

    struct Data data1, data2;
    cxml_list_append(&list2, &data1);
    cxml_list_append(&list2, &data2);

    CHECK_EQ(list2.len, 2);
    CHECK_EQ(list1.len, 0);

    cxml_list_init_with(&list1, &list2);
    CHECK_EQ(list2.len, 0);
    CHECK_EQ(list1.len, 2);
    CHECK_NOT_NULL(list1.head);
    CHECK_NOT_NULL(list1.tail);
    CHECK_EQ(list1.head->item, &data1);
    CHECK_EQ(list1.tail->item, &data2);
    CHECK_NULL(list2.head);
    CHECK_NULL(list2.tail);
    cxml_list_free(&list1);
    cxml_list_free(&list2);
}

TEST(cxlist, cxml_list_qextend){
    cxml_list list1 = new_cxml_list(),
            list2 = new_cxml_list();

    struct Data data1, data2;
    cxml_list_append(&list2, &data1);
    cxml_list_append(&list2, &data2);

    CHECK_EQ(list2.len, 2);
    CHECK_EQ(list1.len, 0);

    cxml_list_qextend(&list1, &list2);
    CHECK_EQ(list2.len, 0);
    CHECK_EQ(list1.len, 2);
    CHECK_NOT_NULL(list1.head);
    CHECK_NOT_NULL(list1.tail);
    CHECK_EQ(list1.head->item, &data1);
    CHECK_EQ(list1.tail->item, &data2);
    CHECK_NULL(list2.head);
    CHECK_NULL(list2.tail);
    cxml_list_free(&list1);
    cxml_list_free(&list2);
}

TEST(cxlist, cxml_list_add){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_add(&list, &data1); // adds to the front of the list
    CHECK_EQ(list.head, list.tail);
    CHECK_EQ(list.head->item, &data1);

    cxml_list_add(&list, &data2);
    CHECK_EQ(list.head->item, &data2);
    CHECK_EQ(list.tail->item, &data1);
    CHECK_NE(list.head, list.tail);

    cxml_list_add(&list, NULL);
    cxml_list_add(NULL, &data1);
    CHECK_EQ(list.len, 2);

    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_search){
    cxml_list list = new_cxml_list();
    struct Data data1, data2, data3;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    CHECK_EQ(list.len, 2);
    // cxml_list_search returns -1 if item isn't found in list, and
    // the index of the item, if found in the list.

    int index = cxml_list_search(&list, cxml_list_cmp_raw_items, &data1);
    CHECK_EQ(index, 0);
    // search for data not in list
    index = cxml_list_search(&list, cxml_list_cmp_raw_items, &data3);
    CHECK_EQ(index, -1);
    index = cxml_list_search(&list, cxml_list_cmp_raw_items, NULL);
    CHECK_EQ(index, -1);
    index = cxml_list_search(NULL, cxml_list_cmp_raw_items, &data2);
    CHECK_EQ(index, -1);
    index = cxml_list_search(&list, cxml_list_cmp_raw_items, &data2);
    CHECK_EQ(index, 1);
    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_search_delete){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    CHECK_NE(list.head, list.tail);
    CHECK_EQ(list.len, 2);

    // cxml_list_search_delete returns 0 on error, 1 on success.
    // delete data1
    int index = cxml_list_search_delete(&list, cxml_list_cmp_raw_items, &data1);
    CHECK_EQ(index, 1);
    CHECK_EQ(list.len, 1);
    CHECK_EQ(list.head, list.tail);

    // search for data1 in list
    index = cxml_list_search(&list, cxml_list_cmp_raw_items, &data1);
    CHECK_EQ(index, -1);
    index = cxml_list_search_delete(&list, cxml_list_cmp_raw_items, &data1);
    CHECK_EQ(index, 0);
    // invalid
    index = cxml_list_search_delete(&list, cxml_list_cmp_raw_items, NULL);
    CHECK_EQ(index, 0);
    // invalid
    index = cxml_list_search_delete(NULL, cxml_list_cmp_raw_items, &data2);
    CHECK_EQ(index, 0);

    index = cxml_list_search_delete(&list, cxml_list_cmp_raw_items, &data2);
    CHECK_EQ(index, 1);
    CHECK_EQ(list.len, 0);

    // data2 no longer in list | list is empty
    index = cxml_list_search(&list, cxml_list_cmp_raw_items, &data2);
    CHECK_EQ(index, -1);
    CHECK_EQ(list.len, 0);
    CHECK_NULL(list.head);
    CHECK_NULL(list.tail);
    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_cmp_str_items){
    char *data1 = "foo", *data2 = "bar", *data3 = "foo";
    CHECK_FALSE( cxml_list_cmp_str_items(&data1, &data2));
    CHECK_TRUE(cxml_list_cmp_str_items(&data1, &data1));
    CHECK_TRUE(cxml_list_cmp_str_items(&data1, &data3));
    CHECK_FALSE(cxml_list_cmp_str_items(&data2, &data3));
}

TEST(cxlist, cxml_list_cmp_raw_items){
    struct Data data1, data2;
    CHECK_FALSE(cxml_list_cmp_raw_items(&data1, &data2));
    CHECK_TRUE(cxml_list_cmp_raw_items(&data1, &data1));
}

int assert_list(cxml_list *list){
    CHECK_EQ(list->len, 0);
    CHECK_NULL(list->head);
    CHECK_NULL(list->tail);
    CHECK_EQ(list->head, list->tail);
    return 1;
}

TEST(cxlist, new_alloc_cxml_list){
    cxml_list *list1 = new_alloc_cxml_list();
    cxml_list *list2 = new_alloc_cxml_list();
    CHECK_EQ(assert_list(list1), 1);
    CHECK_EQ(assert_list(list2), 1);
    cxml_list_free(list1);
    cxml_list_free(list2);
    FREE(list1);
    FREE(list2);
}

TEST(cxlist, cxml_list_size){
    struct Data data;
    cxml_list list = new_cxml_list();
    CHECK_EQ(cxml_list_size(&list), 0);
    cxml_list_append(&list, &data);
    CHECK_EQ(cxml_list_size(&list), 1);
    CHECK_EQ(cxml_list_size(NULL), 0);
    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_get){
    struct Data data;
    cxml_list list = new_cxml_list();
    cxml_list_append(&list, &data);
    CHECK_EQ(list.len, 1);
    CHECK_EQ(cxml_list_get(&list, 0), &data);
    CHECK_NULL(cxml_list_get(&list, 1));
    CHECK_NULL(cxml_list_get(&list, -1));
    CHECK_NULL(cxml_list_get(&list, 5));
    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_last){
    struct Data data1, data2;
    cxml_list list = new_cxml_list();
    CHECK_NULL(cxml_list_last(&list));
    cxml_list_append(&list, &data1);
    CHECK_EQ(cxml_list_last(&list), &data1);
    cxml_list_append(&list, &data2);
    CHECK_EQ(cxml_list_last(&list), &data2);
    CHECK_NULL(cxml_list_last(NULL));
    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_first){
    struct Data data1, data2;
    cxml_list list = new_cxml_list();
    CHECK_NULL(cxml_list_first(&list));
    cxml_list_append(&list, &data1);
    CHECK_EQ(cxml_list_first(&list), &data1);
    cxml_list_append(&list, &data2);
    CHECK_EQ(cxml_list_first(&list), &data1);
    CHECK_NULL(cxml_list_first(NULL));
    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_is_empty){
    struct Data data;
    cxml_list list = new_cxml_list();
    CHECK_TRUE(cxml_list_is_empty(&list));
    cxml_list_append(&list, &data);
    CHECK_FALSE(cxml_list_is_empty(&list));
    CHECK_TRUE(cxml_list_is_empty(NULL));
    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_safe_delete){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    CHECK_EQ(list.len, 2);

    CHECK_NULL(cxml_list_safe_delete(NULL, 0));

    // delete at front
    void *tmp = cxml_list_safe_delete(&list, 0);
    CHECK_NOT_NULL(tmp);
    CHECK_EQ(list.len, 1);
    CHECK_EQ(tmp, &data1);
    CHECK_EQ(list.head, list.tail);

    // search for data1 in list
    tmp = cxml_list_safe_delete(&list, 1);
    CHECK_NOT_NULL(tmp);
    CHECK_EQ(list.len, 0);
    CHECK_EQ(tmp, &data2);
    CHECK_NULL(list.head);
    CHECK_NULL(list.tail);
    CHECK_EQ(list.len, 0);

    tmp = cxml_list_safe_delete(&list, 1);
    CHECK_NULL(tmp);
    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_safe_delete_at_pos){
    cxml_list list = new_cxml_list();
    struct Data data1, data2, data3;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_list_append(&list, &data3);
    CHECK_EQ(list.len, 3);

    CHECK_NULL(cxml_list_safe_delete_at_pos(NULL, 0));

    // delete at front
    void *tmp = cxml_list_safe_delete_at_pos(&list, 2);
    CHECK_NOT_NULL(tmp);
    CHECK_EQ(list.len, 2);
    CHECK_EQ(tmp, &data3);
    CHECK_NE(list.head, list.tail);

    CHECK_NULL(cxml_list_safe_delete_at_pos(&list, 2));
    CHECK_NULL(cxml_list_safe_delete_at_pos(&list, 5));
    CHECK_NULL(cxml_list_safe_delete_at_pos(&list, -1));

    // search for data1 in list
    tmp = cxml_list_safe_delete_at_pos(&list, 0);
    CHECK_NOT_NULL(tmp);
    CHECK_EQ(list.len, 1);
    CHECK_EQ(tmp, &data1);

    CHECK_NULL(cxml_list_safe_delete_at_pos(&list, 1));
    tmp = cxml_list_safe_delete_at_pos(&list, 0);
    CHECK_NOT_NULL(tmp);
    CHECK_EQ(list.len, 0);
    CHECK_EQ(tmp, &data2);
    CHECK_NULL(list.head);
    CHECK_NULL(list.tail);
    CHECK_EQ(list.len, 0);
    // delete from empty list
    CHECK_NULL(cxml_list_safe_delete_at_pos(&list, 0));
    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_delete){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    CHECK_EQ(list.len, 2);

    // should not seg-fault
    cxml_list_delete(NULL, 0);

    void *head = list.head;
    // delete at front
    cxml_list_delete(&list, 0);
    CHECK_EQ(list.len, 1);
    CHECK_NE(list.head, head);
    CHECK_EQ(list.head, list.tail);

    cxml_list_delete(&list, 1);
    CHECK_EQ(list.len, 0);
    CHECK_NULL(list.head);
    CHECK_NULL(list.tail);
    CHECK_EQ(list.len, 0);

    cxml_list_safe_delete(&list, 0);
    CHECK_EQ(list.len, 0);
    cxml_list_free(&list);
}

TEST(cxlist, cxml_list_copy){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    CHECK_EQ(list.len, 2);
    cxml_list list2 = new_cxml_list();
    cxml_list_copy(&list2, &list);
    CHECK_EQ(list.len, list2.len);
    // check if the items are present in the list;
    CHECK_EQ(cxml_list_search(&list2, cxml_list_cmp_raw_items, &data1), 0);
    CHECK_EQ(cxml_list_search(&list2, cxml_list_cmp_raw_items, &data2), 1);

    // should not seg-fault
    cxml_list_copy(&list2, NULL);
    cxml_list_copy(NULL, &list);

    cxml_list_free(&list);
    cxml_list_free(&list2);
}

int test_cxml_list_delete_at_position(void (*fn)(cxml_list *, int)){
    cxml_list list = new_cxml_list();
    struct Data data1, data2, data3;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_list_append(&list, &data3);
    CHECK_EQ(list.len, 3);

    // should not seg-fault
    fn(NULL, 1);

    CHECK_EQ(cxml_list_search(&list, cxml_list_cmp_raw_items, &data2), 1);
    // delete at front
    fn(&list, 1);
    CHECK_EQ(list.len, 2);
    CHECK_EQ(cxml_list_search(&list, cxml_list_cmp_raw_items, &data2), -1);

    void *head = list.head;
    fn(&list, 0);
    CHECK_EQ(list.len, 1);
    CHECK_NE(list.head, head);
    CHECK_EQ(list.head, list.tail);

    fn(&list, 1);
    CHECK_EQ(list.len, 1);
    fn(&list, -1);
    CHECK_EQ(list.len, 1);
    fn(&list, 6);
    CHECK_EQ(list.len, 1);
    fn(&list, 0);
    CHECK_EQ(list.len, 0);
    CHECK_NULL(list.head);
    CHECK_NULL(list.tail);
    CHECK_EQ(list.len, 0);

    fn(&list, 0);
    CHECK_EQ(list.len, 0);
    cxml_list_free(&list);
    return 1;
}

TEST(cxlist, cxml_list_delete_at_pos){
    CHECK_EQ(test_cxml_list_delete_at_position(cxml_list_delete_at_pos), 1);
}

TEST(cxlist, cxml_list_delete_at_index){
    CHECK_EQ(test_cxml_list_delete_at_position(cxml_list_delete_at_index), 1);
}

TEST(cxlist, cxml_list_free){
    cxml_list list = new_cxml_list();
    cxml_list *list2 = new_alloc_cxml_list();
    struct Data data1, data2;
    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_list_append(list2, &data1);
    cxml_list_append(list2, &data2);
    CHECK_EQ(list.len, 2);
    CHECK_EQ(list2->len, 2);
    CHECK_NOT_NULL(list.head);
    CHECK_NOT_NULL(list.tail);
    CHECK_NOT_NULL(list2->head);
    CHECK_NOT_NULL(list2->tail);
    cxml_list_free(&list);
    CHECK_EQ(list.len, 0);
    CHECK_NULL(list.head);
    CHECK_NULL(list.tail);
    cxml_list_free(list2);
    CHECK_EQ(list2->len, 0);
    CHECK_NULL(list2->head);
    CHECK_NULL(list2->tail);
    // should not seg-fault
    cxml_list_free(NULL);
    FREE(list2);
}

TEST(cxlist, cxml_for_each){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;
    struct Data *ds[] = {&data1, &data2, &data1, &data2};
    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    int i = 0;
    cxml_for_each(d, &list){
        CHECK_EQ(d, ds[i]);
        i++;
    }
    cxml_list_free(&list);
}
