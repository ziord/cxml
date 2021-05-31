/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXXPDATA_H
#define CXML_CXXPDATA_H

#include "core/cxmset.h"
#include "cxxpast.h"

// xpath (1.0) has 4 data types: node-set (nodes), string, boolean, numeric

/*
 *  xpath data types
 */
typedef enum _cxml_xp_data_t{
    CXML_XP_DATA_NIL,
    CXML_XP_DATA_NUMERIC,
    CXML_XP_DATA_STRING,
    CXML_XP_DATA_BOOLEAN,
    CXML_XP_DATA_NODESET
}_cxml_xp_data_t;

/** xpath data **/
typedef struct {
    _cxml_xp_data_t type;
    union{
        _Bool       boolean;
        cxml_string str;
        cxml_set    nodeset;
        cxml_number number;
    };
}_cxml_xp_data;

void _cxml_xp_data_init(_cxml_xp_data **data);

_cxml_xp_data *_cxml_xp_new_data();

void _cxml_xp_data_free(_cxml_xp_data *data);

void _cxml_xp_data_clear(_cxml_xp_data *data);

void _cxml_xp__node_string_val(void* node, cxml_string* acc);

void _cxml_xp__node_num_val(void* node, cxml_number *num);

void _cxml_xp__node_name_val(void *node, cxml_string *name);

void _cxml_xp__node_lname_val(void *node /*: cxml node*/, cxml_string *name);

void _cxml_xp_data_to_numeric(_cxml_xp_data *data, cxml_number *num);

void _cxml_xp_data_to_string(_cxml_xp_data *node, cxml_string *str);

void _cxml_xp_data_to_boolean(_cxml_xp_data *node, bool *bol);

#endif //CXML_CXXPDATA_H
