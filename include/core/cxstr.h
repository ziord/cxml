/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXSTR_H
#define CXML_CXSTR_H
#include "cxcomm.h"
#include "cxmem.h"


typedef struct {
    char *_raw_chars;
    unsigned int _len;
    unsigned int _cap;
} cxml_string;

void cxml_string_init(cxml_string *str);

cxml_string new_cxml_string();

cxml_string* new_alloc_cxml_string();

cxml_string new_cxml_string_s(const char *raw);

void cxml_string_append(cxml_string *str, const char *raw, unsigned int len);

void cxml_string_raw_append(cxml_string *str, const char *raw);

void cxml_string_str_append(cxml_string *acc, cxml_string *str);

void cxml_string_n_append(cxml_string *str, char raw, int ntimes);

void cxml_string_dcopy(cxml_string *cpy, cxml_string *ori);

bool cxml_string_raw_equals(cxml_string *str, const char *ch_val);

bool cxml_string_cmp_raw_equals(cxml_string *str, const char *ch_val);

bool cxml_string_lraw_equals(cxml_string *str, const char *ch_val, unsigned int ch_val_len);    // u

bool cxml_string_equals(cxml_string *str_1, cxml_string *str_2);    // u

bool cxml_string_llraw_equals(char *str1, char *str2, int str1_len, int str2_len);

bool cxml_string_replace(cxml_string *str, const char *old_str, const char *replacement, cxml_string *into);    // u

void cxml_string_strip_space(cxml_string *str, cxml_string *new_str);

bool cxml_string_startswith(cxml_string *str, const char *s_str);    // u

bool cxml_string_str_startswith(cxml_string *str, cxml_string *s_str);

bool cxml_string_endswith(cxml_string *str, const char *s_str);    // u

bool cxml_string_str_endswith(cxml_string *str, cxml_string *s_str);

bool cxml_string_contains(cxml_string *str, cxml_string *s_str);    // u

bool cxml_string_raw_contains(cxml_string *str, const char *s_str);

int cxml_string_raw_index(cxml_string *str, const char *s_str);    // u

int cxml_string_char_index(cxml_string *str, char ch);

char *cxml_string_as_raw(cxml_string *str);

unsigned int cxml_string_len(cxml_string *str);    // u

void cxml_string_free(cxml_string *str);

/** utf-8 hook **/
bool cxml_string_mb_contains(cxml_string *str, const char *sub_str);    // u

int cxml_string_mb_str_index(cxml_string *str, const char *sub_str);    // u

int cxml_string_mb_index(cxml_string *str, uint32_t ch);    // u

int cxml_string_mb_len(cxml_string *str);    // u

const char *cxml_string_mb_strstr(cxml_string *str, const char *sub_str);

#endif //CXML_CXSTR_H
