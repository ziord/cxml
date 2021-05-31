/*
 * An extract of Jeff Bezanson's UTF-8 manipulation routines modified and adapted for use in cxml.
 * Distributed as part of cxml under the terms of the MIT license.
 */


#include "utils/cxutf8hook.h"

static const u_int32_t offsetsFromUTF8[6] = {
        0x00000000UL, 0x00003080UL, 0x000E2080UL,
        0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

/* charnum => byte offset */
int u8_offset(const char *str, int charnum) {
    int offs = 0;

    while (charnum > 0 && str[offs]) {
        (void) (isutf(str[++offs]) || isutf(str[++offs]) ||
                isutf(str[++offs]) || ++offs);
        charnum--;
    }
    return offs;
}

bool is_valid_utf8_start(const char *raw){
    int c = 0, next;
    if (raw[c] >= 0) return true;
    // 110xxxxx 10xxxxxx
    if (((unsigned char)raw[c] & 0xe0) == 0xc0){
        next = 1;
    }
    // 1110xxxx 10xxxxxx 10xxxxxx
    else if (((unsigned char)raw[c] & 0xf0) == 0xe0){
        next = 2;
    }
    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    else if (((unsigned char)raw[c] & 0xf8) == 0xf0){
        next = 3;
    }else{
        return false;
    }
    while (next--){
        if (raw[++c] >= 0 || ((unsigned char)raw[c] & 0xc0) != 0x80) return false;
    }
    return true;
}

/* number of characters */
int u8_strlen(char *s) {
    int count = 0;
    int i = 0;

    while (u8_nextchar(s, &i) != 0)
        count++;

    return count;
}

/* reads the next utf-8 sequence out of a string, updating an index */
u_int32_t u8_nextchar(const char *s, int *i) {
    u_int32_t ch = 0;
    int sz = 0;

    do {
        ch <<= 6;
        ch += (unsigned char) s[(*i)++];
        sz++;
    } while (s[*i] && !isutf(s[*i]));
    ch -= offsetsFromUTF8[sz - 1];

    return ch;
}

char *u8_strchr(char *s, u_int32_t ch, int *charn) {
    int i = 0, lasti = 0;
    u_int32_t c;

    *charn = 0;
    while (s[i]) {
        c = u8_nextchar(s, &i);
        if (c == ch) {
            return &s[lasti];
        }
        lasti = i;
        (*charn)++;
    }
    return NULL;
}
