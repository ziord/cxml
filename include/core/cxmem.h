/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXMEM_H
#define CXML_CXMEM_H

#include "cxcomm.h"
#include <stdarg.h>

#define ALLOC(type, length)                     _cxml_allocate((sizeof (type) * length))
#define CALLOC(type, length)                     _cxml_callocate((length), sizeof(type))
#define RALLOC(type, ptr, length)               _cxml_rallocate(ptr, (sizeof(type) * length))

#define ALLOCR(type, length, ...)               _cxml_allocate_r((sizeof (type) * length), __VA_ARGS__)
#define CALLOCR(type, length, ...)               _cxml_callocate_r((length), sizeof(type), __VA_ARGS__)
#define RALLOCR(type, ptr, length, ...)         _cxml_rallocate_r(ptr, (sizeof(type) * length), __VA_ARGS__)

#define FREE(ptr)                               (free(ptr))

#if defined(__STDC__)
    #if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
        #define _CX_ATR_NORETURN _Noreturn
    #else
        #define _CX_ATR_NORETURN  /*Nothing*/
    #endif
#else
    #define _CX_ATR_NORETURN  /*Nothing*/
#endif


#if defined(__GNUC__)
    #define GCC_VERSION                 (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
    #define _CX_ATR_MALLOC              __attribute__((malloc))
    #define _CX_ATR_FMT(n, m)           __attribute__((format(printf, n, m)))
    #if GCC_VERSION >= 20500
        #undef _CX_ATR_NORETURN
        #define _CX_ATR_NORETURN        __attribute__((noreturn))
    #endif
#else
    #define _CX_ATR_MALLOC              /*Nothing*/
    #define _CX_ATR_FMT(n, m)           /*Nothing*/
    #define _CX_ATR_PURE                /*Nothing*/
#endif

_CX_ATR_NORETURN
void cxml_error(char *errfmt, ...) _CX_ATR_FMT(1, 2);

void* _cxml_allocate(size_t len) _CX_ATR_MALLOC;

void* _cxml_callocate(size_t nitems, size_t size) _CX_ATR_MALLOC;

void* _cxml_rallocate(void* ptr, size_t len);

void* _cxml_allocate_r(size_t len, char* fmt, ...)
_CX_ATR_MALLOC _CX_ATR_FMT(2, 3);

void* _cxml_callocate_r(size_t nitems, size_t size, char* fmt, ...)
_CX_ATR_MALLOC _CX_ATR_FMT(3, 4);

void* _cxml_rallocate_r(void* ptr, size_t len, char* fmt, ...) _CX_ATR_FMT(3, 4);
#endif
