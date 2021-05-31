/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXXPRESOLVER_H
#define CXML_CXXPRESOLVER_H

#include "cxxpdata.h"


/*
 * err
 */
_CX_ATR_NORETURN void _cxml_xp_eval_err(const char* cause);


void
_cxml_xp_resolve_arithmetic_operation(
        _cxml_xp_data* left,
        _cxml_xp_data* right,
        void (*_push)(_cxml_xp_data *),
        cxml_xp_op op);


void
_cxml_xp_resolve_and_or_operation(
        _cxml_xp_data* left,
        _cxml_xp_data* right,
        void (*_push)(_cxml_xp_data *),
        cxml_xp_op op);

void
_cxml_xp_resolve_relative_operation(
        _cxml_xp_data* left,
        _cxml_xp_data* right,
        void (*_push)(_cxml_xp_data *),
        cxml_xp_op op);

void
_cxml_xp_resolve_pipe_operation(
        _cxml_xp_data* left,
        _cxml_xp_data* right,
        void (*_nodeset_sorter)(cxml_list *nodes),
        void (*_push)(_cxml_xp_data *));


#endif //CXML_CXXPRESOLVER_H
