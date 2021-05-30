/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"


int empty_str_asserts(cxml_string *str){
    cxml_assert__zero(str->_len)
    cxml_assert__zero(str->_cap)
    cxml_assert__null(str->_raw_chars)
    return 1;
}

int non_empty_str_asserts(cxml_string *str, const char *raw, unsigned len){
    cxml_assert__neq(str->_cap, 0)
    cxml_assert__eq(str->_len, len)
    cxml_assert__not_null(str->_raw_chars)
    cxml_assert__zero(strncmp(raw, str->_raw_chars, len))
    return 1;
}

int str_str_asserts(cxml_string *str1, cxml_string *str2, unsigned len){
    cxml_assert__eq(str1->_len, len)
    cxml_assert__eq(str2->_len, len)
    cxml_assert__eq(str1->_len, str1->_len)
    cxml_assert__eq(str1->_cap, str1->_cap)
    cxml_assert__zero(strncmp(str1->_raw_chars, str2->_raw_chars, len))
    return 1;
}

cts test_cxml_string_init(){
    cxml_string str;
    cxml_string_init(&str);
    cxml_assert__one(empty_str_asserts(&str))
    // should not seg-fault
    cxml_string_init(NULL);
    cxml_pass()
}

extern void cxml_string_from_alloc(cxml_string *str, char **raw, int len);

cts test_cxml_string_from_alloc(){
    int len = 50;
    char *alloc = malloc(len);
    if (!alloc){
        cxml_skip()
    }
    memcpy(alloc, "this is a simple cxml string test", 33);
    cxml_string str = new_cxml_string();
    cxml_string_from_alloc(&str, &alloc, 33);
    cxml_assert__eq(cxml_string_as_raw(&str), alloc)
    cxml_assert__eq(str._len, 33)
    cxml_assert__zero(strcmp(cxml_string_as_raw(&str), alloc))

    cxml_string str2 = new_cxml_string();
    cxml_string_append(&str2, "foo", 3);
    char *tmp = alloc;
    // calling this function on a string with already containing characters,
    // only copies the incoming string (alloc) into the cxml_string object (str2),
    // freeing the incoming string in the process, and allowing it to point to the
    // copied string in the cxml_string object str2. This makes it very unsafe with
    // multiple cxml_strings.
    cxml_string_from_alloc(&str2, &alloc, len);
    // alloc is already freed, and reset to the string in str2.
    cxml_assert__neq(tmp, alloc)
    cxml_assert__not_null(alloc)
    // we can only free str2, as str1, is technically freed,
    // since alloc is freed above (from the call to cxml_string_from_alloc())
    // and freeing str2, also frees alloc, since alloc points to the chars in str2+
    cxml_string_free(&str2);
    cxml_pass()
}

cts test_new_cxml_string(){
    cxml_string str = new_cxml_string();
    cxml_assert__one(empty_str_asserts(&str))
    cxml_pass()
}

cts test_new_alloc_cxml_string(){
    cxml_string *str = new_alloc_cxml_string();
    cxml_assert__one(empty_str_asserts(str))
    FREE(str);
    cxml_pass()
}

cts test_new_cxml_string_s(){
    char *ch = "foobar";
    cxml_string str = new_cxml_string_s(ch);
    cxml_assert__not_null(str._raw_chars)
    cxml_assert__eq(strlen(ch), cxml_string_len(&str))
    cxml_assert__eq(strcmp(cxml_string_as_raw(&str), ch), 0)

    cxml_string str2 = new_cxml_string_s("");
    cxml_assert__zero(str2._len)
    cxml_assert__zero(str2._cap)
    cxml_assert__null(str2._raw_chars)

    cxml_string str3 = new_cxml_string_s(NULL);
    cxml_assert__zero(str3._len)
    cxml_assert__zero(str3._cap)
    cxml_assert__null(str3._raw_chars)

    cxml_string_free(&str);
    cxml_pass()
}

cts test_cxml_string_append(){
    cxml_string str = new_cxml_string();

    cxml_string_append(&str, "this is foo", 11);
    cxml_assert__one(non_empty_str_asserts(&str, "this is foo", 11))

    cxml_string str2 = new_cxml_string();
    cxml_string_append(&str2, NULL, 50);
    cxml_assert__one(empty_str_asserts(&str2));

    cxml_string_free(&str);
    cxml_pass()
}

cts test_cxml_string_raw_append(){
    cxml_string str = new_cxml_string();
    cxml_string str2 = new_cxml_string();

    cxml_string_raw_append(&str, "this is foo");
    cxml_assert__one(non_empty_str_asserts(&str, "this is foo", 11))

    cxml_string_raw_append(&str2, NULL);
    cxml_assert__one(empty_str_asserts(&str2))

    cxml_string_free(&str);
    cxml_pass()
}

cts test_cxml_string_str_append(){
    char *d = "this is foo";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string(),
                str3 = new_cxml_string();

    cxml_string_str_append(&str2, &str);
    cxml_assert__one(str_str_asserts(&str, &str2, strlen(d)))

    cxml_string_append(&str3, NULL, 50);
    cxml_assert__one(empty_str_asserts(&str3));

    cxml_string_free(&str);
    cxml_string_free(&str2);
    cxml_pass()
}

cts test_cxml_string_n_append(){
    char ch = 'a';
    cxml_string str = new_cxml_string();

    cxml_string_n_append(&str, ch, 5);
    cxml_assert__one(non_empty_str_asserts(&str, "aaaaa", 5))

    cxml_string_free(&str);
    cxml_string_n_append(&str, ch, 0);
    cxml_assert__one(empty_str_asserts(&str))

    cxml_string_n_append(NULL, ch, 10);
    cxml_assert__one(empty_str_asserts(&str))
    cxml_pass()
}

cts test_cxml_string_dcopy(){
    char *d = "this is foo";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string(),
                str3 = new_cxml_string();

    cxml_string_dcopy(&str2, &str);
    cxml_assert__one(str_str_asserts(&str, &str2, strlen(d)))

    cxml_string_dcopy(&str3, NULL);
    cxml_assert__one(empty_str_asserts(&str3))

    cxml_string_free(&str);
    cxml_string_free(&str2);
    cxml_pass()
}

cts test_cxml_string_raw_equals(){
    char *d = "this is foo", *d2 = "not a bar";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string_s(d2);
    cxml_assert__true(cxml_string_raw_equals(&str, d))
    cxml_assert__false(cxml_string_raw_equals(&str, d2))
    cxml_assert__false(cxml_string_raw_equals(&str, NULL))
    cxml_assert__false(cxml_string_raw_equals(NULL, d))
    cxml_assert__true(cxml_string_raw_equals(&str2, d2))
    cxml_assert__false(cxml_string_raw_equals(&str2, d))
    cxml_string_free(&str);
    cxml_string_free(&str2);
    cxml_pass()
}

cts test_cxml_string_cmp_raw_equals(){
    char *d = "this is foo", *d2 = "not a bar";
    cxml_string str = new_cxml_string_s(d),
            str2 = new_cxml_string_s(d2);
    cxml_assert__true(cxml_string_cmp_raw_equals(&str, d))
    cxml_assert__false(cxml_string_cmp_raw_equals(&str, d2))
    cxml_assert__false(cxml_string_cmp_raw_equals(&str, NULL))
    cxml_assert__false(cxml_string_cmp_raw_equals(NULL, d))
    cxml_assert__true(cxml_string_cmp_raw_equals(&str2, d2))
    cxml_assert__false(cxml_string_cmp_raw_equals(&str2, d))
    cxml_string_free(&str);
    cxml_string_free(&str2);
    cxml_pass()
}

cts test_cxml_string_lraw_equals(){
    char *d = "this is foo yet again";
    int len = strlen(d);
    cxml_string str = new_cxml_string_s("this is foo");
    cxml_assert__false(cxml_string_lraw_equals(&str, d, len))
    cxml_assert__true(cxml_string_lraw_equals(&str, d, str._len))
    cxml_assert__false(cxml_string_lraw_equals(&str, d, 0))
    cxml_assert__false(cxml_string_lraw_equals(&str, NULL, 0))
    cxml_assert__false(cxml_string_lraw_equals(NULL, d, 0))
    cxml_string_free(&str);
    cxml_pass()
}

cts test_cxml_string_equals(){
    char *d = "this is foo yet again";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string_s(d),
                str3 = new_cxml_string();
    cxml_assert__true(cxml_string_equals(&str, &str2));
    cxml_assert__false(cxml_string_equals(&str, &str3));
    cxml_assert__false(cxml_string_equals(&str2, &str3));
    cxml_assert__false(cxml_string_equals(&str3, &str2));
    cxml_assert__false(cxml_string_equals(&str2, NULL))
    cxml_assert__false(cxml_string_equals(NULL, &str))
    cxml_string_free(&str);
    cxml_string_free(&str2);
    cxml_pass()
}

cts test_cxml_string_llraw_equals(){
    char *d = "this is foo yet again", *d2 = "this is foo";
    int len1 = strlen(d), len2 = strlen(d2);
    cxml_assert__false(cxml_string_llraw_equals(d, d2, len1, len2))
    cxml_assert__true(cxml_string_llraw_equals(d, d2, len2, len2))
    cxml_assert__false(cxml_string_llraw_equals(d, d2, len1, len1))
    cxml_assert__false(cxml_string_llraw_equals(d, d2, len1, 0))
    cxml_assert__false(cxml_string_llraw_equals(d, d2, 0, len2))
    cxml_assert__false(cxml_string_llraw_equals(d, NULL, len1, len2))
    cxml_assert__false(cxml_string_llraw_equals(NULL, d2, len1, len2))
    cxml_pass()
}

cts test_cxml_string_replace(){
    char *d = "this is foo yet again";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string();

    cxml_assert__true(cxml_string_replace(&str, "i", "---", NULL));
    char *r = "th---s ---s foo yet aga---n";
    cxml_assert__one(non_empty_str_asserts(&str, r, strlen(r)));

    cxml_assert__true(cxml_string_replace(&str, "---", "", NULL));
    r = "ths s foo yet agan";
    cxml_assert__one(non_empty_str_asserts(&str, r, strlen(r)));

    cxml_assert__false(cxml_string_replace(&str, "o", NULL, NULL));
    cxml_assert__one(non_empty_str_asserts(&str, r, strlen(r)));

    cxml_assert__false(cxml_string_replace(&str, NULL, "xyz", NULL));
    cxml_assert__one(non_empty_str_asserts(&str, r, strlen(r)));

    cxml_assert__true(cxml_string_replace(&str, "o", "^^^", &str2));
    cxml_assert__one(non_empty_str_asserts(&str, r, strlen(r)));
    char *r2 = "ths s f^^^^^^ yet agan";
    cxml_assert__one(non_empty_str_asserts(&str2, r2, strlen(r2)));

    cxml_string_free(&str);
    cxml_string_free(&str2);
    cxml_pass()
}

cts test_cxml_string_strip_space(){
    char *d = "\t\r\nthis is foo yet again       ",
         *d2 = " this is foo yet again\r\r\t\n",
         *d3 = "\r\n\t this is foo yet again",
         *d4 = "this  is   foo",
         *d5 = "";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string();

    cxml_string_strip_space(&str, &str2);
    char *r = "this is foo yet again";
    cxml_assert__one(non_empty_str_asserts(&str2, r, strlen(r)))
    cxml_string_free(&str);
    cxml_string_free(&str2);

    cxml_string_strip_space(&str, &str2);
    cxml_assert__one(empty_str_asserts(&str))
    cxml_assert__one(empty_str_asserts(&str2))

    cxml_string_strip_space(NULL, &str2);
    cxml_assert__one(empty_str_asserts(&str2))

    str2 = new_cxml_string_s(d2);
    cxml_string_strip_space(&str2, &str);
    cxml_assert__one(non_empty_str_asserts(&str, r, strlen(r)))
    cxml_string_free(&str);
    cxml_string_free(&str2);

    str = new_cxml_string_s(d3);
    cxml_string_strip_space(&str, &str2);
    cxml_assert__one(non_empty_str_asserts(&str2, r, strlen(r)))
    cxml_string_free(&str);
    cxml_string_free(&str2);

    str2 = new_cxml_string_s(d4);
    cxml_string_strip_space(&str2, &str);
    cxml_assert__eq(str2._len, str._len)
    cxml_assert__eq(str2._cap, str._cap)
    cxml_assert__zero(strncmp(str._raw_chars, str2._raw_chars, str._len))
    cxml_string_free(&str);
    cxml_string_free(&str2);

    str = new_cxml_string_s(d5);
    cxml_string_strip_space(&str, &str2);
    cxml_assert__zero(str._len)
    cxml_assert__zero(str2._len)
    cxml_assert__zero(str._cap)
    cxml_assert__zero(str2._cap)
    cxml_string_free(&str);
    cxml_string_free(&str2);
    cxml_pass()
}

cts test_cxml_string_startswith(){
    char *d = "this is foo yet again";
    cxml_string str = new_cxml_string_s(d);
    cxml_assert__true(cxml_string_startswith(&str, "this"));
    cxml_assert__false(cxml_string_startswith(&str, NULL));
    cxml_assert__false(cxml_string_startswith(NULL, "this"));
    cxml_assert__true(cxml_string_startswith(&str, "t"));
    cxml_assert__true(cxml_string_startswith(&str, ""));
    cxml_string_free(&str);
    cxml_assert__true(cxml_string_startswith(&str, ""));
    cxml_pass()
}

cts test_cxml_string_str_startswith(){
    char *d = "this is foo yet again",
         *d2 = "t",
         *d3 = "this is f00";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string_s(d2);
    cxml_assert__true(cxml_string_str_startswith(&str, &str2));
    cxml_assert__false(cxml_string_str_startswith(&str, NULL));
    cxml_assert__false(cxml_string_str_startswith(NULL, &str2));

    cxml_assert__true(cxml_string_str_startswith(&str, &str2))

    cxml_string_free(&str2);
    str2 = new_cxml_string_s(d3);
    cxml_assert__false(cxml_string_str_startswith(&str, &str2))

    cxml_string_free(&str2);
    cxml_assert__true(cxml_string_str_startswith(&str, &str2))  // empty ""
    cxml_string_free(&str);
    cxml_pass()
}

cts test_cxml_string_endswith(){
    char *d = "this is foo yet again";
    cxml_string str = new_cxml_string_s(d);
    cxml_assert__true(cxml_string_endswith(&str, "again"));
    cxml_assert__false(cxml_string_endswith(&str, NULL));
    cxml_assert__false(cxml_string_endswith(NULL, "again"));
    cxml_assert__true(cxml_string_endswith(&str, "n"));
    cxml_assert__true(cxml_string_endswith(&str, ""));
    cxml_string_free(&str);
    cxml_assert__true(cxml_string_endswith(&str, ""));
    cxml_pass()
}

cts test_cxml_string_str_endswith(){
    char *d = "this is foo yet again",
            *d2 = "n",
            *d3 = "this is f00";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string_s(d2);
    cxml_assert__true(cxml_string_str_endswith(&str, &str2))
    cxml_assert__false(cxml_string_str_endswith(&str, NULL))
    cxml_assert__false(cxml_string_str_endswith(NULL, &str2))

    cxml_assert__true(cxml_string_str_endswith(&str, &str2))

    cxml_string_free(&str2);
    str2 = new_cxml_string_s(d3);
    cxml_assert__false(cxml_string_str_endswith(&str, &str2))

    cxml_string_free(&str2);
    cxml_assert__true(cxml_string_str_endswith(&str, &str2))  // empty ""
    cxml_string_free(&str);
    cxml_pass()
}

cts test_cxml_string_contains(){
    char *d = "this is foo yet again",
         *d2 = "n",
         *d3 = "this is f00";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string_s(d2);

    cxml_assert__true(cxml_string_contains(&str, &str))
    cxml_assert__true(cxml_string_contains(&str2, &str2))
    cxml_assert__true(cxml_string_contains(&str, &str2))
    cxml_string_free(&str2);

    str2 = new_cxml_string_s(d3);
    cxml_assert__false(cxml_string_contains(&str, &str2))
    cxml_assert__false(cxml_string_contains(&str, NULL))
    cxml_assert__false(cxml_string_contains(NULL, &str2))
    cxml_string_free(&str2);

    cxml_assert__true(cxml_string_contains(&str, &str2))  // "" in d
    cxml_string_free(&str2);

    cxml_string_free(&str);
    cxml_assert__true(cxml_string_contains(&str, &str2))
    cxml_assert__true(cxml_string_contains(&str2, &str))
    cxml_assert__true(cxml_string_contains(&str, &str))
    cxml_assert__true(cxml_string_contains(&str2, &str2))

    cxml_pass()
}

cts test_cxml_string_raw_contains(){
    char *d = "this is foo yet again",
         *d3 = "this is f00";
    cxml_string str = new_cxml_string_s(d);

    cxml_assert__true(cxml_string_raw_contains(&str, d))
    cxml_string_free(&str);

    cxml_assert__true(cxml_string_raw_contains(&str, ""))  // "" in d
    cxml_assert__false(cxml_string_raw_contains(&str, NULL))
    cxml_assert__false(cxml_string_raw_contains(NULL, d3))

    str = new_cxml_string_s(d);
    cxml_assert__true(cxml_string_raw_contains(&str, ""))  // "" in d
    cxml_string_free(&str);

    str = new_cxml_string_s(d3);
    cxml_assert__true(cxml_string_raw_contains(&str, d3))
    cxml_string_free(&str);

    cxml_pass()
}

cts test_cxml_string_raw_index(){
    cxml_string str = new_cxml_string_s("this is foo yet again");
    cxml_assert__eq(cxml_string_raw_index(&str, "is"), 2)
    cxml_assert__eq(cxml_string_raw_index(&str, "oo"), 9)
    cxml_assert__eq(cxml_string_raw_index(&str, "in"), 19)
    cxml_assert__eq(cxml_string_raw_index(&str, "g"), 17)
    cxml_assert__eq(cxml_string_raw_index(&str, "x"), -1)
    cxml_assert__eq(cxml_string_raw_index(&str, "\0"), 0)
    cxml_assert__eq(cxml_string_raw_index(NULL, "a"), -1)
    cxml_assert__eq(cxml_string_raw_index(&str, NULL), -1)
    cxml_string_free(&str);
    cxml_pass()
}

cts test_cxml_string_char_index(){
    cxml_string str = new_cxml_string_s("this is foo yet again");
    cxml_assert__eq(cxml_string_char_index(&str, 'y'), 12)
    cxml_assert__eq(cxml_string_char_index(&str, 'i'), 2)
    cxml_assert__eq(cxml_string_char_index(&str, 'n'), 20)
    cxml_assert__eq(cxml_string_char_index(&str, '\0'), -1)
    cxml_assert__eq(cxml_string_char_index(&str, 'x'), -1)
    cxml_assert__eq(cxml_string_char_index(&str, '"'), -1)
    cxml_assert__eq(cxml_string_char_index(NULL, 'a'), -1)
    cxml_assert__eq(cxml_string_char_index(&str, 0), -1)
    cxml_string_free(&str);
    cxml_pass()
}

cts test_cxml_string_as_raw(){
    char *d = "this is foo yet again";
    cxml_string str = new_cxml_string_s(d);
    cxml_assert__zero(strncmp(d, cxml_string_as_raw(&str), strlen(d)))
    cxml_assert__null(cxml_string_as_raw(NULL))
    cxml_string_free(&str);
    // returns null for empty strings, to prevent freeing stack allocated strings.
    cxml_assert__null(cxml_string_as_raw(&str))
    cxml_pass()
}

cts test_cxml_string_len(){
    char *d = "this is foo yet again",
         *d2 = "";

    cxml_string str = new_cxml_string_s(d);
    cxml_assert__eq(cxml_string_len(&str), strlen(d))
    cxml_string_free(&str);

    str = new_cxml_string_s(d2);
    cxml_assert__one(empty_str_asserts(&str))
    cxml_assert__eq(cxml_string_len(&str), strlen(d2))

    cxml_assert__zero(cxml_string_len(NULL))
    cxml_string_free(&str);
    cxml_pass()
}

cts test_cxml_string_free(){
    char *d = "this is foo yet again";
    cxml_string str = new_cxml_string_s(d);
    cxml_assert__true(non_empty_str_asserts(&str, d, strlen(d)));
    cxml_string_free(&str);

    cxml_assert__true(empty_str_asserts(&str));
    cxml_string_free(&str);
    cxml_pass()
}

/** utf-8 hook **/
cts test_cxml_string_mb_contains(){
    char *d = "इस नए साल खुशियों की बरसातें हों",
         *d2 = "साल",
         *d3 = "बरसातें this is f00";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string_s(d3);

    cxml_assert__true(cxml_string_mb_contains(&str, d))
    cxml_assert__true(cxml_string_mb_contains(&str, d2))

    cxml_assert__false(cxml_string_mb_contains(&str, d3))
    cxml_assert__false(cxml_string_mb_contains(&str2, d2))
    cxml_assert__false(cxml_string_mb_contains(&str, NULL))
    cxml_assert__false(cxml_string_mb_contains(NULL, d2))
    cxml_string_free(&str2);

    cxml_assert__true(cxml_string_mb_contains(&str, ""))  // "" in d
    cxml_string_free(&str);

    cxml_pass()
}

cts test_cxml_string_mb_str_index(){
    cxml_string str = new_cxml_string_s("thisनए isबरसातेंfoo yet होंagain");
    cxml_assert__eq(cxml_string_mb_str_index(&str, "नए"), 4)
    cxml_assert__eq(cxml_string_mb_str_index(&str, "हों"), 24)
    cxml_assert__eq(cxml_string_mb_str_index(&str, "बरसातेंf"), 9)
    cxml_assert__eq(cxml_string_mb_str_index(&str, "y"), 20)
    cxml_assert__eq(cxml_string_mb_str_index(&str, "ăåå"), -1)
    cxml_assert__eq(cxml_string_mb_str_index(&str, "\0"), 0)
    cxml_assert__eq(cxml_string_mb_str_index(NULL, "a"), -1)
    cxml_assert__eq(cxml_string_mb_str_index(&str, NULL), -1)
    cxml_string_free(&str);
    cxml_pass()
}

cts test_cxml_string_mb_index(){
    cxml_string str = new_cxml_string_s("Ģğhķăls åâv än băåå");
    cxml_assert__eq(cxml_string_mb_index(&str, L'å'), 8)
    cxml_assert__eq(cxml_string_mb_index(&str, L'ğ'), 1)
    cxml_assert__eq(cxml_string_mb_index(&str, L'â'), 9)
    cxml_assert__eq(cxml_string_mb_index(&str, L'b'), 15)
    cxml_assert__eq(cxml_string_mb_index(&str, L'z'), -1)
    cxml_assert__eq(cxml_string_mb_index(&str, L'\0'), -1)
    cxml_assert__eq(cxml_string_mb_index(NULL, L'a'), -1)
    cxml_string_free(&str);
    cxml_pass()
}

cts test_cxml_string_mb_len(){
    char *d = "इस नए साल खुशियों की बरसातें हों";
    cxml_string str = new_cxml_string_s(d);
    cxml_assert__true(non_empty_str_asserts(&str, d, strlen(d)))
    cxml_assert__eq(cxml_string_mb_len(&str), 32)
    cxml_assert__neq(cxml_string_mb_len(&str), (int)strlen(d))
    cxml_assert__zero(cxml_string_mb_len(NULL))
    cxml_string_free(&str);
    cxml_pass()
}

cts test_cxml_string_mb_strstr(){
    char *d = "इस नए साल खुशियों की बरसातें हों";
    cxml_string str = new_cxml_string_s(d);
    cxml_assert__not_null(cxml_string_mb_strstr(&str, d))
    cxml_assert__not_null(cxml_string_mb_strstr(&str, "इस"))
    cxml_assert__null(cxml_string_mb_strstr(NULL, "बरसातें"))
    cxml_assert__null(cxml_string_mb_strstr(&str, NULL))
    cxml_assert__null(cxml_string_mb_strstr(&str, "शुरू"))
    cxml_string_free(&str);
    cxml_pass()
}

void suite_cxstr() {
    cxml_suite(cxstr)
    {
        cxml_add_m_test(33,
                        test_cxml_string_init,
                        test_cxml_string_from_alloc,
                        test_new_cxml_string,
                        test_new_alloc_cxml_string,
                        test_new_cxml_string_s,
                        test_cxml_string_append,
                        test_cxml_string_raw_append,
                        test_cxml_string_str_append,
                        test_cxml_string_n_append,
                        test_cxml_string_dcopy,
                        test_cxml_string_raw_equals,
                        test_cxml_string_cmp_raw_equals,
                        test_cxml_string_lraw_equals,
                        test_cxml_string_equals,
                        test_cxml_string_llraw_equals,
                        test_cxml_string_replace,
                        test_cxml_string_strip_space,
                        test_cxml_string_startswith,
                        test_cxml_string_str_startswith,
                        test_cxml_string_endswith,
                        test_cxml_string_str_endswith,
                        test_cxml_string_contains,
                        test_cxml_string_raw_contains,
                        test_cxml_string_raw_index,
                        test_cxml_string_char_index,
                        test_cxml_string_as_raw,
                        test_cxml_string_len,
                        test_cxml_string_free,
                        test_cxml_string_mb_contains,
                        test_cxml_string_mb_str_index,
                        test_cxml_string_mb_index,
                        test_cxml_string_mb_len,
                        test_cxml_string_mb_strstr
        )
        cxml_run_suite()
//        cxml_suite_report()
    }
}
