/*
 * An extract of Jeff Bezanson's UTF-8 manipulation routines modified and adapted for use in cxml.
 * Distributed as part of cxml under the terms of the MIT license.
 */

#ifndef CXML_CXUTF8HOOK_H
#define CXML_CXUTF8HOOK_H

#include "core/cxcomm.h"
#include <stdarg.h>


/* is c the start of a utf8 sequence? */
#define isutf(c) (((c)&0xC0)!=0x80)

/* character number to byte offset */
int u8_offset(const char *str, int charnum);

/* return next character, updating an index variable */
u_int32_t u8_nextchar(const char *s, int *i);

/* return a pointer to the first occurrence of ch in s, or NULL if not
   found. character index of found character returned in *charn. */
char *u8_strchr(char *s, u_int32_t ch, int *charn);

/* count the number of characters in a UTF-8 string */
int u8_strlen(char *s);

#endif //CXML_CXUTF8HOOK_H
