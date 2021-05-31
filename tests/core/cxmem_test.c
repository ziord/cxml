/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

TEST(cxmem, cxml_allocate){
    char *foo = _cxml_allocate(10);
    CHECK_NE(foo, NULL);
    free(foo);

    foo = ALLOC(char, 10);
    CHECK_NE(foo, NULL);
    free(foo)
}

TEST(cxmem, cxml_callocate){
    char *foo = _cxml_callocate(1, 10);
    CHECK_NE(foo, NULL);
    free(foo);

    foo = CALLOC(char, 10);
    CHECK_NE(foo, NULL);
    free(foo);
}

TEST(cxmem, cxml_rallocate){
    char *foo = malloc(3);
    foo = _cxml_rallocate(foo, 10);
    CHECK_NE(foo, NULL);

    foo = RALLOC(char, foo, 13);
    CHECK_NE(foo, NULL);
    free(foo);
}

TEST(cxmem, cxml_allocate_r){
    char *foo = _cxml_allocate_r(10, "not enough memory");
    CHECK_NE(foo, NULL);
    free(foo);

    foo = ALLOCR(char, 10, "not enough memory");
    CHECK_NE(foo, NULL);
    free(foo);
}


TEST(cxmem, cxml_callocate_r){
    char *foo = _cxml_callocate_r(1, 10, "not enough memory");
    CHECK_NE(foo, NULL);
    free(foo);

    foo = CALLOCR(char, 10, "not enough memory");
    CHECK_NE(foo, NULL);
    free(foo);
}

TEST(cxmem, cxml_rallocate_r){
    char *foo = malloc(3);
    if (!foo){
        cxml_skip()
    }
    foo = _cxml_rallocate_r(foo, 10, "not enough memory");
    CHECK_NE(foo, NULL);

    foo = RALLOCR(char, foo, 13, "not enough memory");
    CHECK_NE(foo, NULL);
    free(foo);
}