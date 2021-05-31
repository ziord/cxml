/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"


int empty_list_asserts(cxml_list *list){
    cxml_assert__zero(list->len)
    cxml_assert__null(list->head)
    cxml_assert__null(list->tail)
    cxml_assert__eq(list->head, list->tail)
    return 1;
}

cts test_new_cxml_list(){
    cxml_list list = new_cxml_list();
    cxml_assert__one(empty_list_asserts(&list))
    cxml_pass()
}

cts test_cxml_list_init(){
    cxml_list list;
    cxml_list_init(&list);
    cxml_assert__one(empty_list_asserts(&list))
    // should not seg-fault
    cxml_list_init(NULL);
    cxml_pass()
}


cts test_cxml_list_insert(){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_insert(&list, &data1, true);  // insert at front
    cxml_list_insert(&list, &data2, true);  // insert at front

    cxml_assert__eq(list.head->item, (&data2))
    cxml_assert__eq(list.tail->item, (&data1))
    cxml_assert__eq(list.len, 2)
    cxml_assert__not_null(list.head)
    cxml_assert__neq(list.head->item, list.tail->item)
    cxml_assert__neq(list.head, list.tail)

    cxml_list_insert(&list, &data1, false); // insert at end
    cxml_assert__eq(list.tail->item, (&data1))
    cxml_assert__eq(list.len, 3)

    // size doesn't change because NULL will never be inserted.
    cxml_list_insert(&list, NULL, true);
    cxml_list_insert(NULL, &data1, true);
    cxml_assert__eq(list.len, 3)

    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_insert_at_index(){
    cxml_list list = new_cxml_list();
    struct Data data1, data2, data3, data4;

    cxml_list_insert(&list, &data1, false);
    cxml_assert__eq(list.head, list.tail)
    cxml_list_insert(&list, &data2, true);
    cxml_list_insert_at_index(&list, &data3, 1);
    cxml_list_insert_at_index(&list, &data4, 2);

    cxml_assert__not_null(list.head)
    cxml_assert__eq(list.head->item, ((void*)&data2))
    cxml_assert__eq(list.tail->item, ((void*)&data1))
    cxml_assert__eq(list.head->next->item, ((void*)&data3))
    cxml_assert__eq(list.head->next->next->item, ((void*)&data4))
    cxml_assert__eq(list.len, 4)
    cxml_assert__neq(list.head->item, list.tail->item)
    cxml_assert__neq(list.head, list.tail)
    // test off index
    cxml_list_insert_at_index(&list, &data4, 5);
    cxml_assert__eq(list.len, 4)
    // invalid data | list
    cxml_list_insert_at_index(&list, NULL, 2);
    cxml_list_insert_at_index(NULL, &data4, 2);
    cxml_assert__eq(list.len, 4)
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_append(){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_assert__two(list.len)
    cxml_assert__eq(list.head->item, &data1)
    cxml_assert__eq(list.tail->item, &data2)

    cxml_list_append(&list, NULL);
    cxml_list_append(NULL, &data1);
    cxml_assert__two(list.len)

    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_extend(){
    cxml_list list1 = new_cxml_list(),
              list2 = new_cxml_list();
    cxml_assert__zero(list1.len)
    cxml_assert__zero(list2.len)

    struct Data data1, data2;

    cxml_list_append(&list1, &data1);
    cxml_list_append(&list1, &data2);
    cxml_assert__two(list1.len)

    cxml_list_extend(&list2, &list1);
    cxml_assert__two(list2.len)
    cxml_assert__eq(list2.head->item, &data1)
    cxml_assert__eq(list2.tail->item, &data2)

    cxml_list_extend(&list2, NULL);
    cxml_assert__two(list2.len)

    cxml_list_extend(NULL, &list1);
    cxml_assert__two(list2.len)

    cxml_list_free(&list1);
    cxml_list_free(&list2);
    cxml_pass()
}

cts test_cxml_list_init_with(){
    cxml_list list1 = new_cxml_list(),
            list2 = new_cxml_list();

    struct Data data1, data2;
    cxml_list_append(&list2, &data1);
    cxml_list_append(&list2, &data2);

    cxml_assert__two(list2.len)
    cxml_assert__zero(list1.len)

    cxml_list_init_with(&list1, &list2);
    cxml_assert__zero(list2.len)
    cxml_assert__two(list1.len)
    cxml_assert__not_null(list1.head)
    cxml_assert__not_null(list1.tail)
    cxml_assert__eq(list1.head->item, &data1)
    cxml_assert__eq(list1.tail->item, &data2)
    cxml_assert__null(list2.head)
    cxml_assert__null(list2.tail)
    cxml_list_free(&list1);
    cxml_list_free(&list2);
    cxml_pass()
}

cts test_cxml_list_qextend(){
    cxml_list list1 = new_cxml_list(),
            list2 = new_cxml_list();

    struct Data data1, data2;
    cxml_list_append(&list2, &data1);
    cxml_list_append(&list2, &data2);

    cxml_assert__two(list2.len)
    cxml_assert__zero(list1.len)

    cxml_list_qextend(&list1, &list2);
    cxml_assert__zero(list2.len)
    cxml_assert__two(list1.len)
    cxml_assert__not_null(list1.head)
    cxml_assert__not_null(list1.tail)
    cxml_assert__eq(list1.head->item, &data1)
    cxml_assert__eq(list1.tail->item, &data2)
    cxml_assert__null(list2.head)
    cxml_assert__null(list2.tail)
    cxml_list_free(&list1);
    cxml_list_free(&list2);
    cxml_pass()
}

cts test_cxml_list_add(){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_add(&list, &data1); // adds to the front of the list
    cxml_assert__eq(list.head, list.tail)
    cxml_assert__eq(list.head->item, &data1)

    cxml_list_add(&list, &data2);
    cxml_assert__eq(list.head->item, &data2)
    cxml_assert__eq(list.tail->item, &data1)
    cxml_assert__neq(list.head, list.tail)

    cxml_list_add(&list, NULL);
    cxml_list_add(NULL, &data1);
    cxml_assert__two(list.len)

    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_search(){
    cxml_list list = new_cxml_list();
    struct Data data1, data2, data3;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_assert__two(list.len)
    // cxml_list_search returns -1 if item isn't found in list, and
    // the index of the item, if found in the list.

    int index = cxml_list_search(&list, cxml_list_cmp_raw_items, &data1);
    cxml_assert__zero(index)
    // search for data not in list
    index = cxml_list_search(&list, cxml_list_cmp_raw_items, &data3);
    cxml_assert__eq(index, -1)
    index = cxml_list_search(&list, cxml_list_cmp_raw_items, NULL);
    cxml_assert__eq(index, -1)
    index = cxml_list_search(NULL, cxml_list_cmp_raw_items, &data2);
    cxml_assert__eq(index, -1)
    index = cxml_list_search(&list, cxml_list_cmp_raw_items, &data2);
    cxml_assert__one(index)
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_search_delete(){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_assert__neq(list.head, list.tail)
    cxml_assert__two(list.len)

    // cxml_list_search_delete returns 0 on error, 1 on success.
    // delete data1
    int index = cxml_list_search_delete(&list, cxml_list_cmp_raw_items, &data1);
    cxml_assert__one(index)
    cxml_assert__one(list.len)
    cxml_assert__eq(list.head, list.tail)

    // search for data1 in list
    index = cxml_list_search(&list, cxml_list_cmp_raw_items, &data1);
    cxml_assert__eq(index, -1)
    index = cxml_list_search_delete(&list, cxml_list_cmp_raw_items, &data1);
    cxml_assert__zero(index)
    // invalid
    index = cxml_list_search_delete(&list, cxml_list_cmp_raw_items, NULL);
    cxml_assert__zero(index)
    // invalid
    index = cxml_list_search_delete(NULL, cxml_list_cmp_raw_items, &data2);
    cxml_assert__zero(index)

    index = cxml_list_search_delete(&list, cxml_list_cmp_raw_items, &data2);
    cxml_assert__one(index)
    cxml_assert__zero(list.len)

    // data2 no longer in list | list is empty
    index = cxml_list_search(&list, cxml_list_cmp_raw_items, &data2);
    cxml_assert__eq(index, -1)
    cxml_assert__zero(list.len)
    cxml_assert__null(list.head)
    cxml_assert__null(list.tail)
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_cmp_str_items(){
    char *data1 = "foo", *data2 = "bar", *data3 = "foo";
    cxml_assert__false( cxml_list_cmp_str_items(&data1, &data2))
    cxml_assert__true(cxml_list_cmp_str_items(&data1, &data1))
    cxml_assert__true(cxml_list_cmp_str_items(&data1, &data3))
    cxml_assert__false(cxml_list_cmp_str_items(&data2, &data3))
    cxml_pass()
}

cts test_cxml_list_cmp_raw_items(){
    struct Data data1, data2;
    cxml_assert__false(cxml_list_cmp_raw_items(&data1, &data2))
    cxml_assert__true(cxml_list_cmp_raw_items(&data1, &data1));
    cxml_pass()
}

int assert_list(cxml_list *list){
    cxml_assert__zero(list->len)
    cxml_assert__null(list->head)
    cxml_assert__null(list->tail)
    cxml_assert__eq(list->head, list->tail)
    return 1;
}

cts test_new_alloc_cxml_list(){
    cxml_list *list1 = new_alloc_cxml_list();
    cxml_list *list2 = new_alloc_cxml_list();
    cxml_assert__one(assert_list(list1));
    cxml_assert__one(assert_list(list2));
    cxml_list_free(list1);
    cxml_list_free(list2);
    FREE(list1);
    FREE(list2);
    cxml_pass()
}

cts test_cxml_list_size(){
    struct Data data;
    cxml_list list = new_cxml_list();
    cxml_assert__zero(cxml_list_size(&list))
    cxml_list_append(&list, &data);
    cxml_assert__one(cxml_list_size(&list))
    cxml_assert__zero(cxml_list_size(NULL))
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_get(){
    struct Data data;
    cxml_list list = new_cxml_list();
    cxml_list_append(&list, &data);
    cxml_assert__one(list.len)
    cxml_assert__eq(cxml_list_get(&list, 0), &data)
    cxml_assert__null(cxml_list_get(&list, 1))
    cxml_assert__null(cxml_list_get(&list, -1))
    cxml_assert__null(cxml_list_get(&list, 5))
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_last(){
    struct Data data1, data2;
    cxml_list list = new_cxml_list();
    cxml_assert__null(cxml_list_last(&list));
    cxml_list_append(&list, &data1);
    cxml_assert__eq(cxml_list_last(&list), &data1);
    cxml_list_append(&list, &data2);
    cxml_assert__eq(cxml_list_last(&list), &data2);
    cxml_assert__null(cxml_list_last(NULL));
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_first(){
    struct Data data1, data2;
    cxml_list list = new_cxml_list();
    cxml_assert__null(cxml_list_first(&list));
    cxml_list_append(&list, &data1);
    cxml_assert__eq(cxml_list_first(&list), &data1);
    cxml_list_append(&list, &data2);
    cxml_assert__eq(cxml_list_first(&list), &data1);
    cxml_assert__null(cxml_list_first(NULL));
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_is_empty(){
    struct Data data;
    cxml_list list = new_cxml_list();
    cxml_assert__true(cxml_list_is_empty(&list));
    cxml_list_append(&list, &data);
    cxml_assert__false(cxml_list_is_empty(&list));
    cxml_assert__true(cxml_list_is_empty(NULL));
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_safe_delete(){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_assert__two(list.len)

    cxml_assert__null(cxml_list_safe_delete(NULL, 0))

    // delete at front
    void *tmp = cxml_list_safe_delete(&list, 0);
    cxml_assert__not_null(tmp)
    cxml_assert__one(list.len)
    cxml_assert__eq(tmp, &data1)
    cxml_assert__eq(list.head, list.tail)

    // search for data1 in list
    tmp = cxml_list_safe_delete(&list, 1);
    cxml_assert__not_null(tmp)
    cxml_assert__zero(list.len)
    cxml_assert__eq(tmp, &data2)
    cxml_assert__null(list.head)
    cxml_assert__null(list.tail)
    cxml_assert__zero(list.len)

    tmp = cxml_list_safe_delete(&list, 1);
    cxml_assert__null(tmp)
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_safe_delete_at_pos(){
    cxml_list list = new_cxml_list();
    struct Data data1, data2, data3;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_list_append(&list, &data3);
    cxml_assert__eq(list.len, 3)

    cxml_assert__null(cxml_list_safe_delete_at_pos(NULL, 0))

    // delete at front
    void *tmp = cxml_list_safe_delete_at_pos(&list, 2);
    cxml_assert__not_null(tmp)
    cxml_assert__two(list.len)
    cxml_assert__eq(tmp, &data3)
    cxml_assert__neq(list.head, list.tail)

    cxml_assert__null(cxml_list_safe_delete_at_pos(&list, 2))
    cxml_assert__null(cxml_list_safe_delete_at_pos(&list, 5))
    cxml_assert__null(cxml_list_safe_delete_at_pos(&list, -1))

    // search for data1 in list
    tmp = cxml_list_safe_delete_at_pos(&list, 0);
    cxml_assert__not_null(tmp)
    cxml_assert__one(list.len)
    cxml_assert__eq(tmp, &data1)

    cxml_assert__null(cxml_list_safe_delete_at_pos(&list, 1))
    tmp = cxml_list_safe_delete_at_pos(&list, 0);
    cxml_assert__not_null(tmp)
    cxml_assert__zero(list.len)
    cxml_assert__eq(tmp, &data2)
    cxml_assert__null(list.head)
    cxml_assert__null(list.tail)
    cxml_assert__zero(list.len)
    // delete from empty list
    cxml_assert__null(cxml_list_safe_delete_at_pos(&list, 0))
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_delete(){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_assert__two(list.len)

    // should not seg-fault
    cxml_list_delete(NULL, 0);

    void *head = list.head;
    // delete at front
    cxml_list_delete(&list, 0);
    cxml_assert__one(list.len)
    cxml_assert__neq(list.head, head)
    cxml_assert__eq(list.head, list.tail)

    cxml_list_delete(&list, 1);
    cxml_assert__zero(list.len)
    cxml_assert__null(list.head)
    cxml_assert__null(list.tail)
    cxml_assert__zero(list.len)

    cxml_list_safe_delete(&list, 0);
    cxml_assert__zero(list.len)
    cxml_list_free(&list);
    cxml_pass()
}

cts test_cxml_list_copy(){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_assert__two(list.len)
    cxml_list list2 = new_cxml_list();
    cxml_list_copy(&list2, &list);
    cxml_assert__eq(list.len, list2.len)
    // check if the items are present in the list
    cxml_assert__zero(cxml_list_search(&list2, cxml_list_cmp_raw_items, &data1))
    cxml_assert__one(cxml_list_search(&list2, cxml_list_cmp_raw_items, &data2))

    // should not seg-fault
    cxml_list_copy(&list2, NULL);
    cxml_list_copy(NULL, &list);

    cxml_list_free(&list);
    cxml_list_free(&list2);
    cxml_pass()
}

int test_cxml_list_delete_at_position(void (*fn)(cxml_list *, int)){
    cxml_list list = new_cxml_list();
    struct Data data1, data2, data3;

    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_list_append(&list, &data3);
    cxml_assert__eq(list.len, 3)

    // should not seg-fault
    fn(NULL, 1);

    cxml_assert__one(cxml_list_search(&list, cxml_list_cmp_raw_items, &data2))
    // delete at front
    fn(&list, 1);
    cxml_assert__two(list.len)
    cxml_assert__eq(cxml_list_search(&list, cxml_list_cmp_raw_items, &data2), -1)

    void *head = list.head;
    fn(&list, 0);
    cxml_assert__one(list.len)
    cxml_assert__neq(list.head, head)
    cxml_assert__eq(list.head, list.tail)

    fn(&list, 1);
    cxml_assert__one(list.len)
    fn(&list, -1);
    cxml_assert__one(list.len)
    fn(&list, 6);
    cxml_assert__one(list.len)
    fn(&list, 0);
    cxml_assert__zero(list.len)
    cxml_assert__null(list.head)
    cxml_assert__null(list.tail)
    cxml_assert__zero(list.len)

    fn(&list, 0);
    cxml_assert__zero(list.len)
    cxml_list_free(&list);
    return 1;
}

cts test_cxml_list_delete_at_pos(){
    cxml_assert__one(test_cxml_list_delete_at_position(cxml_list_delete_at_pos))
    cxml_pass()
}

cts test_cxml_list_delete_at_index(){
    cxml_assert__one(test_cxml_list_delete_at_position(cxml_list_delete_at_index))
    cxml_pass()
}

cts test_cxml_list_free(){
    cxml_list list = new_cxml_list();
    cxml_list *list2 = new_alloc_cxml_list();
    struct Data data1, data2;
    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_list_append(list2, &data1);
    cxml_list_append(list2, &data2);
    cxml_assert__two(list.len)
    cxml_assert__two(list2->len)
    cxml_assert__not_null(list.head)
    cxml_assert__not_null(list.tail)
    cxml_assert__not_null(list2->head)
    cxml_assert__not_null(list2->tail)
    cxml_list_free(&list);
    cxml_assert__zero(list.len)
    cxml_assert__null(list.head)
    cxml_assert__null(list.tail)
    cxml_list_free(list2);
    cxml_assert__zero(list2->len)
    cxml_assert__null(list2->head)
    cxml_assert__null(list2->tail)
    // should not seg-fault
    cxml_list_free(NULL);
    FREE(list2);
    cxml_pass()
}

cts test_cxml_for_each(){
    cxml_list list = new_cxml_list();
    struct Data data1, data2;
    struct Data *ds[] = {&data1, &data2, &data1, &data2};
    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    cxml_list_append(&list, &data1);
    cxml_list_append(&list, &data2);
    int i = 0;
    cxml_for_each(d, &list){
        cxml_assert__eq(d, ds[i])
        i++;
    }
    cxml_list_free(&list);
    cxml_pass()
}


/*
 * cxlist.c suite
 */
void suite_cxlist(){
    cxml_suite(cxlist)
    {
        cxml_add_m_test(27,
                        test_new_cxml_list,
                        test_cxml_list_init,
                        test_cxml_list_insert,
                        test_cxml_list_insert_at_index,
                        test_cxml_list_append,
                        test_cxml_list_extend,
                        test_cxml_list_init_with,
                        test_cxml_list_qextend,
                        test_cxml_list_add,
                        test_cxml_list_search,
                        test_cxml_list_search_delete,
                        test_cxml_list_cmp_str_items,
                        test_cxml_list_cmp_raw_items,
                        test_new_alloc_cxml_list,
                        test_cxml_list_size,
                        test_cxml_list_get,
                        test_cxml_list_last,
                        test_cxml_list_first,
                        test_cxml_list_is_empty,
                        test_cxml_list_safe_delete,
                        test_cxml_list_safe_delete_at_pos,
                        test_cxml_list_delete,
                        test_cxml_list_copy,
                        test_cxml_list_delete_at_pos,
                        test_cxml_list_delete_at_index,
                        test_cxml_list_free,
                        test_cxml_for_each
                        )
        cxml_run_suite()
    }
}
