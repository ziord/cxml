/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxstr.h"

#define GROW_CXSTR_CAP(v1, v2)     (v1 && v1 > v2 ? (v1 << 1u) : (v2 << 1u))

#define _set_nul(__str)   __str->_raw_chars[__str->_len] = '\0'

#define __init_str(__str)               \
    (__str)->_len = (__str)->_cap = 0;  \
    (__str)->_raw_chars = NULL;



void cxml_string_init(cxml_string *str) {
    if (!str) return;
    __init_str(str)
}

void cxml_string_from_alloc(cxml_string *str, char **raw, int len) {
    if (!raw) return;
    if (str->_raw_chars == NULL){
        str->_raw_chars = *raw;
        str->_len = str->_cap = len;
    }else{
        cxml_string_append(str, *raw, len);
        FREE(*raw);
        // ensure `raw` still points at the string in `str`
        *raw = str->_raw_chars + (str->_len - len);
    }
}

cxml_string new_cxml_string(){
    cxml_string str;
    __init_str(&str);
    return str;
}

cxml_string* new_alloc_cxml_string(){
    cxml_string *str = ALLOC(cxml_string, 1);
    __init_str(str);
    return str;
}

cxml_string new_cxml_string_s(const char* raw){
    if (!raw) return new_cxml_string();
    cxml_string str;
    __init_str(&str);
    cxml_string_append(&str, raw, (int) strlen(raw));
    return str;
}

// mutating function.
// mutates the original cxml_string object str
void cxml_string_append(cxml_string *str, const char *raw, unsigned int len) {
    if (!str || !raw || !len) return;
    if ((str->_len + len + 1) >= str->_cap) {
        str->_cap = GROW_CXSTR_CAP((str->_cap + 1), (len));
        str->_raw_chars = RALLOC(char, str->_raw_chars, str->_cap);
    }
    memcpy((str->_raw_chars + str->_len), raw, len);
    str->_len += len;
}

// mutating function.
// mutates the original cxml_string object str
void cxml_string_raw_append(cxml_string* str, const char *raw){
    if (!raw) return;
    cxml_string_append(str, raw, (unsigned) strlen(raw));
}

// mutating function.
// mutates the original cxml_string object acc
void cxml_string_str_append(cxml_string *acc, cxml_string *str){
    if (!acc || !str) return;
    cxml_string_append(acc, str->_raw_chars, str->_len);
}

void cxml_string_n_append(cxml_string *str, char raw, int ntimes){
    if (!str || !ntimes) return;
    if (str->_len + ntimes >= str->_cap){
        str->_cap = GROW_CXSTR_CAP((str->_cap + ntimes), (0u));
        str->_raw_chars = RALLOC(char, str->_raw_chars, str->_cap);
    }
    memset(str->_raw_chars + str->_len, raw, ntimes);
    str->_len += ntimes;
}

// non-mutating
// expects an empty string.
void cxml_string_dcopy(cxml_string *cpy, cxml_string *ori){
    if (!cpy || !ori) return;
    cpy->_raw_chars = CALLOC(char, ori->_cap);
    memcpy(cpy->_raw_chars, ori->_raw_chars, ori->_len);
    cpy->_len = ori->_len;
    cpy->_cap = ori->_cap;
}

// non-mutating
bool cxml_string_startswith(cxml_string *str, const char *s_str) {
    if (!str || !s_str) return false;
    unsigned int len = (int)strlen(s_str);
    if (str->_len < len) return false;
    if (str->_cap){
        _set_nul(str);
        return strncmp(str->_raw_chars, s_str, len) == 0;
    }
    return strncmp("", s_str, len) == 0;
}

// non-mutating
bool cxml_string_str_startswith(cxml_string *str, cxml_string *s_str) {
    if (!str || !s_str) return false;
    if (str->_len < s_str->_len) return false;
    if (str->_cap){
        _set_nul(str);
        return strncmp(str->_raw_chars,
                       s_str->_cap ? s_str->_raw_chars : "",
                       s_str->_len) == 0;
    }
    return strncmp("",
                   s_str->_cap ? s_str->_raw_chars : "",
                   s_str->_len) == 0;
}

// non-mutating
static bool cxml_string_endswith_l(cxml_string* str, const char* s_str, unsigned len){
    if (str->_len < len) return false;
    if (!str->_cap){
        if (!len) return true; // "" && ""
        return false;
    }
    _set_nul(str);
    size_t start_offset = str->_len - len;
    return strncmp(str->_raw_chars + start_offset, s_str, len) == 0;
}

// non-mutating
bool cxml_string_endswith(cxml_string *str, const char *s_str) {
    if (!str || !s_str) return false;
    return cxml_string_endswith_l(str, s_str, (int) strlen(s_str));
}

bool cxml_string_str_endswith(cxml_string *str, cxml_string *s_str){
    if (!str || !s_str) return false;
    return cxml_string_endswith_l(str, s_str->_raw_chars, s_str->_len);
}


char *cxml_string_as_raw(cxml_string *str) {
    if (!str) return NULL;
    if (str->_cap) {
        _set_nul(str);
        return str->_raw_chars;
    }
    return NULL;
}

// check for equality using only the length of str arg
bool cxml_string_raw_equals(cxml_string* str, const char* ch_val){
    if (!str || !ch_val || (!str->_len && ch_val[0])) return false;
    return strncmp(ch_val, str->_raw_chars, str->_len) == 0;
}

// check for equality using the length of both args
bool cxml_string_cmp_raw_equals(cxml_string* str, const char* ch_val){
    if (!str || !ch_val) return false;
    unsigned len = strlen(ch_val);
    if (len != str->_len) return false;
    return strncmp(ch_val, str->_raw_chars, len) == 0;
}

// check for equality using the length of str, and
// the length of `ch_val` provided by the third arg.
bool cxml_string_lraw_equals(cxml_string* str, const char* ch_val, unsigned ch_val_len){
    if (!str || str->_len != ch_val_len){
        return false;
    }
    return cxml_string_raw_equals(str, ch_val);
}

// check for equality between two cxml_string objects
bool cxml_string_equals(cxml_string* str_1, cxml_string* str_2){
    if (!str_1 || !str_2 || str_1->_len != str_2->_len){
        return false;
    }
    return strncmp(str_1->_raw_chars, str_2->_raw_chars, str_1->_len) == 0;
}

// check for equality using the both length
// provided by the third and fourth argument.
bool cxml_string_llraw_equals(char *str1, char *str2, int str1_len, int str2_len){
    // compare two char* objects
    if (!str1 || !str2 || str1_len != str2_len) return 0;
    return memcmp(str1, str2, str1_len) == 0;
}

// check if s_str is a substring of str
bool cxml_string_contains(cxml_string *str, cxml_string *s_str) {
    if (!str || !s_str) return 0;
    if (s_str->_len == 0) return 1;  // all strings contains ""
    if (str->_cap){
        _set_nul(str);
        return strstr(str->_raw_chars, cxml_string_as_raw(s_str)) != NULL;
    }
    return strstr("", cxml_string_as_raw(s_str)) != NULL;
}

// non-mutating
bool cxml_string_raw_contains(cxml_string *str, const char *s_str) {
    if (!str || !s_str) return 0;
    if (str->_cap){
        _set_nul(str);
        return strstr(str->_raw_chars, s_str) != NULL;
    }
    return strstr("", s_str) != NULL;
}

// non-mutating
int cxml_string_raw_index(cxml_string *str, const char *s_str) {
    if (!str || !s_str) return -1;
    char *ptr;
    if (str->_cap){
        _set_nul(str);
        ptr = strstr(str->_raw_chars, s_str);
    }else{
        ptr = strstr("", s_str);
    }
    if (ptr) return (int) (ptr - str->_raw_chars);
    return -1;
}

// non-mutating
int cxml_string_char_index(cxml_string *str, char ch) {
    if (!str || ch == '\0') return -1;
    char *ptr;
    if (str->_cap){
        _set_nul(str);
        ptr = strchr(str->_raw_chars, ch);
    }else{
        ptr = strchr("", ch);
    }
    if (ptr) return (int) (ptr - str->_raw_chars);
    return -1;
}

static void cxstr_init_with(cxml_string* str1, cxml_string* str2){
    if (!str1 || !str2) return;
    str1->_raw_chars = str2->_raw_chars;
    str1->_len = str2->_len;
    str1->_cap = str2->_cap;
}

// mutating function.
// mutates the original cxml_string object str
bool cxml_string_replace(cxml_string *str, const char *old_str, const char *replacement, cxml_string *into) {
    /*
     * old_str represents the substring to be replaced.
     * old_str_len represents the length of the substring to be replaced.
     *
     * Caution: Any pointer previously pointing to the `_raw_chars` of `str`
     * (e.g. char *raw = cxml_string_as_raw(str);)
     * could be invalidated after `cxml_string_replace()` is called and executed on `str`.
     * If this is a concern, `into` argument MUST be passed to receive the transformed string.
     * If `into` argument is provided, and no replacement was made, `into` is left unmodified.
     */

    // find the string, replace, move.

    // sanitary checks
    if (!str || !old_str || !replacement || old_str == replacement)
        return 0;

    // add '\0' to the str, to prevent errors.
    _set_nul(str);

    size_t old_str_len = strlen(old_str);

    if (old_str_len == 0) return 0;

    size_t new_str_len = strlen(replacement);

    cxml_string new_str = new_cxml_string();

    char* container = str->_raw_chars;

    char* s_ptr = strstr(container, old_str);

    // tmp_len - number of bytes before the string to be replaced in container (str->_raw_chars)
    // acc_len - accumulation of the entire tmp_len computed.

    unsigned tmp_len = 0, acc_len = 0;

    while (s_ptr){
        tmp_len = (s_ptr - container);

        if (tmp_len > 0){ // replacement is found after first byte downwards..
            cxml_string_append(&new_str, container, tmp_len);
        }
        cxml_string_append(&new_str, replacement, new_str_len);
        container = container + tmp_len + old_str_len;
        acc_len += tmp_len + old_str_len;
        s_ptr = strstr(container, old_str);
    }
     /*
      * after replacement, we could end up in either of the two states:
      * 1.) s-----*-----*------*c-------x
      * 2.) s-----*-----*------*cx
      * s -> start, * -> to be replaced, x -> end, c -> current position of container
      */
    // helps to check if the string was actually replaced or not.
    if (new_str._cap && acc_len <= str->_len){
        // was entire string replaced ? if not, append the remaining chars in the container
        // else do not append since there'd be nothing left (from the original string) to append
        // (len of original string must equal accumulated len -> acc_len, if fully replaced)
        if ((str->_len - acc_len) != 0) {
            cxml_string_append(&new_str, container, (int) (str->_len - acc_len));
        }
        if (!into){
            FREE(str->_raw_chars);
            cxstr_init_with(str, &new_str);
        }else{
            cxstr_init_with(into, &new_str);
        }
        return 1;
    }
    cxml_string_free(&new_str);
    return 0;
}

// non-mutating
void cxml_string_strip_space(cxml_string *str, cxml_string* new_str) {

    if (!cxml_string_len(str) || !new_str) return;
    // left
    unsigned left_count = 0, right_count = 0, len = str->_len;

    for (unsigned i = 0; i < len && isspace((unsigned char) str->_raw_chars[i]); i++){
        left_count++;
    }
    // means cxml_string contains only spaces.
    if (left_count == str->_len) return;
    for (unsigned i = len - 1; i && isspace((unsigned char) str->_raw_chars[i]); i--){
        right_count++;
    }
    char* raw_left = str->_raw_chars + left_count;
    cxml_string_append(new_str, raw_left, (str->_len) - (left_count + right_count));
}

// non-mutating
unsigned int cxml_string_len(cxml_string* str){
    if (!str) return 0;
    return str->_len;
}

void cxml_string_free(cxml_string *str) {
    if (!str) return;
    if (str->_cap){
        FREE(str->_raw_chars);
    }
    __init_str(str);
}


/** utf-8/multibyte string support functions**/
#include "utils/cxutf8hook.h"

static const char *_cxstr_mb_strstr(cxml_string *str, const char *sub_str, int *index);

bool cxml_string_mb_contains(cxml_string* str, const char* sub_str){
    return _cxstr_mb_strstr(str, sub_str, NULL) != NULL;
}

static bool is_match(const char* s, const char* c, size_t len){
    return memcmp(s, c, len) == 0;
}

int  cxml_string_mb_str_index(cxml_string* str, const char* sub_str){
    int index;
    return _cxstr_mb_strstr(str, sub_str, &index) == NULL ? -1 : index;
}

const char*  cxml_string_mb_strstr(cxml_string* str, const char* sub_str){
    return _cxstr_mb_strstr(str, sub_str, NULL);
}

static const char*  _cxstr_mb_strstr(cxml_string* str, const char* sub_str, int* index){
    if (!str || !sub_str) return NULL;
    char* s = str->_raw_chars;
    int i = 0, j = 0, k;
    if (!index) index = &k;
    *index = 0;
    size_t len =  strlen(sub_str);
    if (len == 0) return sub_str;  // catch empty substring checks ""
    if (len > str->_len) return NULL;
    uint32_t tmp = u8_nextchar(sub_str, &i);
    char* i_ptr;
    while ((i_ptr = u8_strchr(s, tmp, &i)) != NULL){
        *index += i;
        if (is_match(i_ptr, sub_str, len)){
            return i_ptr;
        }
        // get byte offset for 1 unicode character
        j = u8_offset(i_ptr, 1);
        // adjust input str by current found substr and byte offset
        s = i_ptr + j;
        // adjust index by 1 multi-byte character
        *index += 1;
    }
    return NULL;
}

int  cxml_string_mb_index(cxml_string* str, uint32_t ch){
    if (!str) return -1;
    int chn;
    char* i_ptr = u8_strchr(cxml_string_as_raw(str), ch, &chn);
    if (i_ptr) return chn;
    return -1;
}

int cxml_string_mb_len(cxml_string* str){
    if (!str) return 0;
    return u8_strlen(cxml_string_as_raw(str));
}
