/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

void empty_str_asserts(cxml_string *str){
    CHECK_EQ(str->_len, 0);
    CHECK_EQ(str->_cap, 0);
    CHECK_EQ(str->_raw_chars, NULL);
}

void non_empty_str_asserts(cxml_string *str, const char *raw, unsigned len){
    CHECK_NE(str->_cap, 0);
    CHECK_EQ(str->_len, len);
    CHECK_NE(str->_raw_chars, NULL);
    CHECK_EQ(strncmp(raw, str->_raw_chars, len), 0);
}

void str_str_asserts(cxml_string *str1, cxml_string *str2, unsigned len){
    CHECK_EQ(str1->_len, len);
    CHECK_EQ(str2->_len, len);
    CHECK_EQ(str1->_len, str1->_len);
    CHECK_EQ(str1->_cap, str1->_cap);
    CHECK_EQ(strncmp(str1->_raw_chars, str2->_raw_chars, len), 0);
}

TEST(cxstr, cxml_string_init){
    cxml_string str;
    cxml_string_init(&str);
    empty_str_asserts(&str);
    // should not seg-fault
    cxml_string_init(NULL);
}

extern void cxml_string_from_alloc(cxml_string *str, char **raw, int len);

TEST(cxstr, cxml_string_from_alloc){
    int len = 50;
    char *alloc = malloc(len);
    if (!alloc){
        exit(1);
    }
    memcpy(alloc, "this is a simple cxml string test", 33);
    cxml_string str = new_cxml_string();
    cxml_string_from_alloc(&str, &alloc, 33);
    CHECK_EQ(cxml_string_as_raw(&str), alloc);
    CHECK_EQ(str._len, 33);
    CHECK_EQ(strcmp(cxml_string_as_raw(&str), alloc), 0);

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
    CHECK_NE(tmp, alloc);
    CHECK_NE(alloc, NULL);
    // we can only free str2, as str1, is technically freed,
    // since alloc is freed above (from the call to cxml_string_from_alloc())
    // and freeing str2, also frees alloc, since alloc points to the chars in str2+
    cxml_string_free(&str2);
}

TEST(cxstr, new_cxml_string){
    cxml_string str = new_cxml_string();
    empty_str_asserts(&str);
}

TEST(cxstr, new_alloc_cxml_string){
    cxml_string *str = new_alloc_cxml_string();
    empty_str_asserts(str);
    FREE(str);
}

TEST(cxstr, new_cxml_string_s){
    char *ch = "foobar";
    cxml_string str = new_cxml_string_s(ch);
    CHECK_NE(str._raw_chars, NULL);
    CHECK_EQ(strlen(ch), cxml_string_len(&str));
    CHECK_EQ(strcmp(cxml_string_as_raw(&str), ch), 0);

    cxml_string str2 = new_cxml_string_s("");
    CHECK_EQ(str2._len, 0);
    CHECK_EQ(str2._cap, 0);
    CHECK_EQ(str2._raw_chars, NULL);

    cxml_string str3 = new_cxml_string_s(NULL);
    CHECK_EQ(str3._len, 0);
    CHECK_EQ(str3._cap, 0);
    CHECK_EQ(str3._raw_chars, NULL);

    cxml_string_free(&str);
}

TEST(cxstr, cxml_string_append){
    cxml_string str = new_cxml_string();

    cxml_string_append(&str, "this is foo", 11);
    CHECK_EQ(non_empty_str_asserts(&str, "this is foo", 11), 1);

    cxml_string str2 = new_cxml_string();
    cxml_string_append(&str2, NULL, 50);
    empty_str_asserts(&str2));

    cxml_string_free(&str);
}

TEST(cxstr, cxml_string_raw_append){
    cxml_string str = new_cxml_string();
    cxml_string str2 = new_cxml_string();

    cxml_string_raw_append(&str, "this is foo");
    CHECK_EQ(non_empty_str_asserts(&str, "this is foo", 11), 1);

    cxml_string_raw_append(&str2, NULL);
    empty_str_asserts(&str2);

    cxml_string_free(&str);
}

TEST(cxstr, cxml_string_str_append){
    char *d = "this is foo";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string(),
                str3 = new_cxml_string();

    cxml_string_str_append(&str2, &str);
    CHECK_EQ(str_str_asserts(&str, &str2, strlen(d)), 1);

    cxml_string_append(&str3, NULL, 50);
    empty_str_asserts(&str3));

    cxml_string_free(&str);
    cxml_string_free(&str2);
}

TEST(cxstr, cxml_string_n_append){
    char ch = 'a';
    cxml_string str = new_cxml_string();

    cxml_string_n_append(&str, ch, 5);
    CHECK_EQ(non_empty_str_asserts(&str, "aaaaa", 5), 1);

    cxml_string_free(&str);
    cxml_string_n_append(&str, ch, 0);
    empty_str_asserts(&str);

    cxml_string_n_append(NULL, ch, 10);
    empty_str_asserts(&str);
}

TEST(cxstr, cxml_string_dcopy){
    char *d = "this is foo";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string(),
                str3 = new_cxml_string();

    cxml_string_dcopy(&str2, &str);
    CHECK_EQ(str_str_asserts(&str, &str2, strlen(d)), 1);

    cxml_string_dcopy(&str3, NULL);
    empty_str_asserts(&str3);

    cxml_string_free(&str);
    cxml_string_free(&str2);
}

TEST(cxstr, cxml_string_raw_equals){
    char *d = "this is foo", *d2 = "not a bar";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string_s(d2);
    CHECK_TRUE(cxml_string_raw_equals(&str, d));
    CHECK_FALSE(cxml_string_raw_equals(&str, d2));
    CHECK_FALSE(cxml_string_raw_equals(&str, NULL));
    CHECK_FALSE(cxml_string_raw_equals(NULL, d));
    CHECK_TRUE(cxml_string_raw_equals(&str2, d2));
    CHECK_FALSE(cxml_string_raw_equals(&str2, d));
    cxml_string_free(&str);
    cxml_string_free(&str2);
}

TEST(cxstr, cxml_string_cmp_raw_equals){
    char *d = "this is foo", *d2 = "not a bar";
    cxml_string str = new_cxml_string_s(d),
            str2 = new_cxml_string_s(d2);
    CHECK_TRUE(cxml_string_cmp_raw_equals(&str, d));
    CHECK_FALSE(cxml_string_cmp_raw_equals(&str, d2));
    CHECK_FALSE(cxml_string_cmp_raw_equals(&str, NULL));
    CHECK_FALSE(cxml_string_cmp_raw_equals(NULL, d));
    CHECK_TRUE(cxml_string_cmp_raw_equals(&str2, d2));
    CHECK_FALSE(cxml_string_cmp_raw_equals(&str2, d));
    cxml_string_free(&str);
    cxml_string_free(&str2);
}

TEST(cxstr, cxml_string_lraw_equals){
    char *d = "this is foo yet again";
    int len = strlen(d);
    cxml_string str = new_cxml_string_s("this is foo");
    CHECK_FALSE(cxml_string_lraw_equals(&str, d, len));
    CHECK_TRUE(cxml_string_lraw_equals(&str, d, str._len));
    CHECK_FALSE(cxml_string_lraw_equals(&str, d, 0));
    CHECK_FALSE(cxml_string_lraw_equals(&str, NULL, 0));
    CHECK_FALSE(cxml_string_lraw_equals(NULL, d, 0));
    cxml_string_free(&str);
}

TEST(cxstr, cxml_string_equals){
    char *d = "this is foo yet again";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string_s(d),
                str3 = new_cxml_string();
    CHECK_TRUE(cxml_string_equals(&str, &str2));;
    CHECK_FALSE(cxml_string_equals(&str, &str3));;
    CHECK_FALSE(cxml_string_equals(&str2, &str3));;
    CHECK_FALSE(cxml_string_equals(&str3, &str2));;
    CHECK_FALSE(cxml_string_equals(&str2, NULL));
    CHECK_FALSE(cxml_string_equals(NULL, &str));
    cxml_string_free(&str);
    cxml_string_free(&str2);
}

TEST(cxstr, cxml_string_llraw_equals){
    char *d = "this is foo yet again", *d2 = "this is foo";
    int len1 = strlen(d), len2 = strlen(d2);
    CHECK_FALSE(cxml_string_llraw_equals(d, d2, len1, len2));
    CHECK_TRUE(cxml_string_llraw_equals(d, d2, len2, len2));
    CHECK_FALSE(cxml_string_llraw_equals(d, d2, len1, len1));
    CHECK_FALSE(cxml_string_llraw_equals(d, d2, len1, 0));
    CHECK_FALSE(cxml_string_llraw_equals(d, d2, 0, len2));
    CHECK_FALSE(cxml_string_llraw_equals(d, NULL, len1, len2));
    CHECK_FALSE(cxml_string_llraw_equals(NULL, d2, len1, len2));
}

TEST(cxstr, cxml_string_replace){
    char *d = "this is foo yet again";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string();

    CHECK_TRUE(cxml_string_replace(&str, "i", "---", NULL));;
    char *r = "th---s ---s foo yet aga---n";
    CHECK_EQ(non_empty_str_asserts(&str, r, strlen(r))), 1;;

    CHECK_TRUE(cxml_string_replace(&str, "---", "", NULL));;
    r = "ths s foo yet agan";
    CHECK_EQ(non_empty_str_asserts(&str, r, strlen(r))), 1;;

    CHECK_FALSE(cxml_string_replace(&str, "o", NULL, NULL));;
    CHECK_EQ(non_empty_str_asserts(&str, r, strlen(r))), 1;;

    CHECK_FALSE(cxml_string_replace(&str, NULL, "xyz", NULL));;
    CHECK_EQ(non_empty_str_asserts(&str, r, strlen(r))), 1;;

    CHECK_TRUE(cxml_string_replace(&str, "o", "^^^", &str2));;
    CHECK_EQ(non_empty_str_asserts(&str, r, strlen(r))), 1;;
    char *r2 = "ths s f^^^^^^ yet agan";
    CHECK_EQ(non_empty_str_asserts(&str2, r2, strlen(r2))), 1;;

    cxml_string_free(&str);
    cxml_string_free(&str2);
}

TEST(cxstr, cxml_string_strip_space){
    char *d = "\t\r\nthis is foo yet again       ",
         *d2 = " this is foo yet again\r\r\t\n",
         *d3 = "\r\n\t this is foo yet again",
         *d4 = "this  is   foo",
         *d5 = "";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string();

    cxml_string_strip_space(&str, &str2);
    char *r = "this is foo yet again";
    CHECK_EQ(non_empty_str_asserts(&str2, r, strlen(r)), 1);
    cxml_string_free(&str);
    cxml_string_free(&str2);

    cxml_string_strip_space(&str, &str2);
    empty_str_asserts(&str);
    empty_str_asserts(&str2);

    cxml_string_strip_space(NULL, &str2);
    empty_str_asserts(&str2);

    str2 = new_cxml_string_s(d2);
    cxml_string_strip_space(&str2, &str);
    CHECK_EQ(non_empty_str_asserts(&str, r, strlen(r)), 1);
    cxml_string_free(&str);
    cxml_string_free(&str2);

    str = new_cxml_string_s(d3);
    cxml_string_strip_space(&str, &str2);
    CHECK_EQ(non_empty_str_asserts(&str2, r, strlen(r)), 1);
    cxml_string_free(&str);
    cxml_string_free(&str2);

    str2 = new_cxml_string_s(d4);
    cxml_string_strip_space(&str2, &str);
    CHECK_EQ(str2._len, str._len);
    CHECK_EQ(str2._cap, str._cap);
    CHECK_EQ(strncmp(str._raw_chars, str2._raw_chars, str._len), 0);
    cxml_string_free(&str);
    cxml_string_free(&str2);

    str = new_cxml_string_s(d5);
    cxml_string_strip_space(&str, &str2);
    CHECK_EQ(str._len, 0);
    CHECK_EQ(str2._len, 0);
    CHECK_EQ(str._cap, 0);
    CHECK_EQ(str2._cap, 0);
    cxml_string_free(&str);
    cxml_string_free(&str2);
}

TEST(cxstr, cxml_string_startswith){
    char *d = "this is foo yet again";
    cxml_string str = new_cxml_string_s(d);
    CHECK_TRUE(cxml_string_startswith(&str, "this"));;
    CHECK_FALSE(cxml_string_startswith(&str, NULL));;
    CHECK_FALSE(cxml_string_startswith(NULL, "this"));;
    CHECK_TRUE(cxml_string_startswith(&str, "t"));;
    CHECK_TRUE(cxml_string_startswith(&str, ""));;
    cxml_string_free(&str);
    CHECK_TRUE(cxml_string_startswith(&str, ""));;
}

TEST(cxstr, cxml_string_str_startswith){
    char *d = "this is foo yet again",
         *d2 = "t",
         *d3 = "this is f00";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string_s(d2);
    CHECK_TRUE(cxml_string_str_startswith(&str, &str2));;
    CHECK_FALSE(cxml_string_str_startswith(&str, NULL));;
    CHECK_FALSE(cxml_string_str_startswith(NULL, &str2));;

    CHECK_TRUE(cxml_string_str_startswith(&str, &str2));

    cxml_string_free(&str2);
    str2 = new_cxml_string_s(d3);
    CHECK_FALSE(cxml_string_str_startswith(&str, &str2));

    cxml_string_free(&str2);
    CHECK_TRUE(cxml_string_str_startswith(&str, &str2))  // empty "";
    cxml_string_free(&str);
}

TEST(cxstr, cxml_string_endswith){
    char *d = "this is foo yet again";
    cxml_string str = new_cxml_string_s(d);
    CHECK_TRUE(cxml_string_endswith(&str, "again"));;
    CHECK_FALSE(cxml_string_endswith(&str, NULL));;
    CHECK_FALSE(cxml_string_endswith(NULL, "again"));;
    CHECK_TRUE(cxml_string_endswith(&str, "n"));;
    CHECK_TRUE(cxml_string_endswith(&str, ""));;
    cxml_string_free(&str);
    CHECK_TRUE(cxml_string_endswith(&str, ""));;
}

TEST(cxstr, cxml_string_str_endswith){
    char *d = "this is foo yet again",
            *d2 = "n",
            *d3 = "this is f00";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string_s(d2);
    CHECK_TRUE(cxml_string_str_endswith(&str, &str2));
    CHECK_FALSE(cxml_string_str_endswith(&str, NULL));
    CHECK_FALSE(cxml_string_str_endswith(NULL, &str2));

    CHECK_TRUE(cxml_string_str_endswith(&str, &str2));

    cxml_string_free(&str2);
    str2 = new_cxml_string_s(d3);
    CHECK_FALSE(cxml_string_str_endswith(&str, &str2));

    cxml_string_free(&str2);
    CHECK_TRUE(cxml_string_str_endswith(&str, &str2))  // empty "";
    cxml_string_free(&str);
}

TEST(cxstr, cxml_string_contains){
    char *d = "this is foo yet again",
         *d2 = "n",
         *d3 = "this is f00";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string_s(d2);

    CHECK_TRUE(cxml_string_contains(&str, &str));
    CHECK_TRUE(cxml_string_contains(&str2, &str2));
    CHECK_TRUE(cxml_string_contains(&str, &str2));
    cxml_string_free(&str2);

    str2 = new_cxml_string_s(d3);
    CHECK_FALSE(cxml_string_contains(&str, &str2));
    CHECK_FALSE(cxml_string_contains(&str, NULL));
    CHECK_FALSE(cxml_string_contains(NULL, &str2));
    cxml_string_free(&str2);

    CHECK_TRUE(cxml_string_contains(&str, &str2))  // "" in d;
    cxml_string_free(&str2);

    cxml_string_free(&str);
    CHECK_TRUE(cxml_string_contains(&str, &str2));
    CHECK_TRUE(cxml_string_contains(&str2, &str));
    CHECK_TRUE(cxml_string_contains(&str, &str));
    CHECK_TRUE(cxml_string_contains(&str2, &str2));

}

TEST(cxstr, cxml_string_raw_contains){
    char *d = "this is foo yet again",
         *d3 = "this is f00";
    cxml_string str = new_cxml_string_s(d);

    CHECK_TRUE(cxml_string_raw_contains(&str, d));
    cxml_string_free(&str);

    CHECK_TRUE(cxml_string_raw_contains(&str, ""))  // "" in d;
    CHECK_FALSE(cxml_string_raw_contains(&str, NULL));
    CHECK_FALSE(cxml_string_raw_contains(NULL, d3));

    str = new_cxml_string_s(d);
    CHECK_TRUE(cxml_string_raw_contains(&str, ""))  // "" in d;
    cxml_string_free(&str);

    str = new_cxml_string_s(d3);
    CHECK_TRUE(cxml_string_raw_contains(&str, d3));
    cxml_string_free(&str);

}

TEST(cxstr, cxml_string_raw_index){
    cxml_string str = new_cxml_string_s("this is foo yet again");
    CHECK_EQ(cxml_string_raw_index(&str, "is"), 2);
    CHECK_EQ(cxml_string_raw_index(&str, "oo"), 9);
    CHECK_EQ(cxml_string_raw_index(&str, "in"), 19);
    CHECK_EQ(cxml_string_raw_index(&str, "g"), 17);
    CHECK_EQ(cxml_string_raw_index(&str, "x"), -1);
    CHECK_EQ(cxml_string_raw_index(&str, "\0"), 0);
    CHECK_EQ(cxml_string_raw_index(NULL, "a"), -1);
    CHECK_EQ(cxml_string_raw_index(&str, NULL), -1);
    cxml_string_free(&str);
}

TEST(cxstr, cxml_string_char_index){
    cxml_string str = new_cxml_string_s("this is foo yet again");
    CHECK_EQ(cxml_string_char_index(&str, 'y'), 12);
    CHECK_EQ(cxml_string_char_index(&str, 'i'), 2);
    CHECK_EQ(cxml_string_char_index(&str, 'n'), 20);
    CHECK_EQ(cxml_string_char_index(&str, '\0'), -1);
    CHECK_EQ(cxml_string_char_index(&str, 'x'), -1);
    CHECK_EQ(cxml_string_char_index(&str, '"'), -1);
    CHECK_EQ(cxml_string_char_index(NULL, 'a'), -1);
    CHECK_EQ(cxml_string_char_index(&str, 0), -1);
    cxml_string_free(&str);
}

TEST(cxstr, cxml_string_as_raw){
    char *d = "this is foo yet again";
    cxml_string str = new_cxml_string_s(d);
    CHECK_EQ(strncmp(d, cxml_string_as_raw(&str), strlen(d)), 0);
    CHECK_EQ(cxml_string_as_raw(NULL), NULL);
    cxml_string_free(&str);
    // returns null for empty strings, to prevent freeing stack allocated strings.
    CHECK_EQ(cxml_string_as_raw(&str), NULL);
}

TEST(cxstr, cxml_string_len){
    char *d = "this is foo yet again",
         *d2 = "";

    cxml_string str = new_cxml_string_s(d);
    CHECK_EQ(cxml_string_len(&str), strlen(d));
    cxml_string_free(&str);

    str = new_cxml_string_s(d2);
    empty_str_asserts(&str);
    CHECK_EQ(cxml_string_len(&str), strlen(d2));

    CHECK_EQ(cxml_string_len(NULL), 0);
    cxml_string_free(&str);
}

TEST(cxstr, cxml_string_free){
    char *d = "this is foo yet again";
    cxml_string str = new_cxml_string_s(d);
    CHECK_TRUE(non_empty_str_asserts(&str, d, strlen(d)));;
    cxml_string_free(&str);

    CHECK_TRUE(empty_str_asserts(&str));;
    cxml_string_free(&str);
}

/** utf-8 hook **/
TEST(cxstr, cxml_string_mb_contains){
    char *d = "इस नए साल खुशियों की बरसातें हों",
         *d2 = "साल",
         *d3 = "बरसातें this is f00";
    cxml_string str = new_cxml_string_s(d),
                str2 = new_cxml_string_s(d3);

    CHECK_TRUE(cxml_string_mb_contains(&str, d));
    CHECK_TRUE(cxml_string_mb_contains(&str, d2));

    CHECK_FALSE(cxml_string_mb_contains(&str, d3));
    CHECK_FALSE(cxml_string_mb_contains(&str2, d2));
    CHECK_FALSE(cxml_string_mb_contains(&str, NULL));
    CHECK_FALSE(cxml_string_mb_contains(NULL, d2));
    cxml_string_free(&str2);

    CHECK_TRUE(cxml_string_mb_contains(&str, ""))  // "" in d;
    cxml_string_free(&str);

}

TEST(cxstr, cxml_string_mb_str_index){
    cxml_string str = new_cxml_string_s("thisनए isबरसातेंfoo yet होंagain");
    CHECK_EQ(cxml_string_mb_str_index(&str, "नए"), 4);
    CHECK_EQ(cxml_string_mb_str_index(&str, "हों"), 24);
    CHECK_EQ(cxml_string_mb_str_index(&str, "बरसातेंf"), 9);
    CHECK_EQ(cxml_string_mb_str_index(&str, "y"), 20);
    CHECK_EQ(cxml_string_mb_str_index(&str, "ăåå"), -1);
    CHECK_EQ(cxml_string_mb_str_index(&str, "\0"), 0);
    CHECK_EQ(cxml_string_mb_str_index(NULL, "a"), -1);
    CHECK_EQ(cxml_string_mb_str_index(&str, NULL), -1);
    cxml_string_free(&str);
}

TEST(cxstr, cxml_string_mb_index){
    cxml_string str = new_cxml_string_s("Ģğhķăls åâv än băåå");
    CHECK_EQ(cxml_string_mb_index(&str, L'å'), 8);
    CHECK_EQ(cxml_string_mb_index(&str, L'ğ'), 1);
    CHECK_EQ(cxml_string_mb_index(&str, L'â'), 9);
    CHECK_EQ(cxml_string_mb_index(&str, L'b'), 15);
    CHECK_EQ(cxml_string_mb_index(&str, L'z'), -1);
    CHECK_EQ(cxml_string_mb_index(&str, L'\0'), -1);
    CHECK_EQ(cxml_string_mb_index(NULL, L'a'), -1);
    cxml_string_free(&str);
}

TEST(cxstr, cxml_string_mb_len){
    char *d = "इस नए साल खुशियों की बरसातें हों";
    cxml_string str = new_cxml_string_s(d);
    CHECK_TRUE(non_empty_str_asserts(&str, d, strlen(d)));
    CHECK_EQ(cxml_string_mb_len(&str), 32);
    CHECK_NE(cxml_string_mb_len(&str), (int)strlen(d));
    CHECK_EQ(cxml_string_mb_len(NULL), 0);
    cxml_string_free(&str);
}

TEST(cxstr, cxml_string_mb_strstr){
    char *d = "इस नए साल खुशियों की बरसातें हों";
    cxml_string str = new_cxml_string_s(d);
    CHECK_NE(cxml_string_mb_strstr(&str, d), NULL);
    CHECK_NE(cxml_string_mb_strstr(&str, "इस"), NULL);
    CHECK_EQ(cxml_string_mb_strstr(NULL, "बरसातें"), NULL);
    CHECK_EQ(cxml_string_mb_strstr(&str, NULL), NULL);
    CHECK_EQ(cxml_string_mb_strstr(&str, "शुरू"), NULL);
    cxml_string_free(&str);
}