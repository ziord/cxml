/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxmem.h"

#define _CXML_FATAL_ERROR   "CXMLFatalError... Not enough memory.\n"

_CX_ATR_NORETURN void cxml_error(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

void* _cxml_allocate_r(size_t len, char* fmt, ...){
    void* ptr = malloc(len);
    if (ptr == NULL){
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* _cxml_callocate_r(size_t nitems, size_t size, char* fmt, ...){
    void* ptr = calloc(nitems, size);
    if (ptr == NULL){
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* _cxml_rallocate_r(void* ptr, size_t len, char* fmt, ...){
    void* new_ptr = realloc(ptr, len);
    if (new_ptr == NULL){
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        exit(EXIT_FAILURE);;
    }
    return new_ptr;
}

void* _cxml_allocate(size_t len){
    return _cxml_allocate_r(len, _CXML_FATAL_ERROR);
}

void* _cxml_callocate(size_t nitems, size_t size){
    return _cxml_callocate_r(nitems, size, _CXML_FATAL_ERROR);
}

void* _cxml_rallocate(void* ptr, size_t len){
    return _cxml_rallocate_r(ptr, len, _CXML_FATAL_ERROR);
}
