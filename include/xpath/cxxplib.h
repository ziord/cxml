/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXXPLIB_H
#define CXML_CXXPLIB_H

#include "cxxpast.h"
#include "cxxpdata.h"
#include "cxxpvisitors.h"
#include "cxxpparser.h"

struct _cxml_xp_func_LU {
    const char *name;
    const int arity;
    const int omittable;
    _cxml_xp_ret_t ret_type;
    void (*fn)(cxml_xp_functioncall *,
               _cxml_xp_data *(*_pop)(void),
               void (*_push)(_cxml_xp_data *));
};

struct _cxml_xp_func_LU_val{
    int pos;
    int arity;
    _cxml_xp_ret_t ret_type; // return type
};

#endif //CXML_CXXPLIB_H
