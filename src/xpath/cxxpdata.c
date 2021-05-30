/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "xpath/cxxpdata.h"

void _cxml_xp_data_init(_cxml_xp_data **data) {
    (*data)->type = CXML_XP_DATA_NIL;
}

/*
 * check_type is a flag used when we care about the numerical type of
 * the nodes being converted to strings.
 * Useful when a text node is being converted to a string so it can then be
 * converted to a numerical value, in that case, with check_type set,
 * when _cxml_stringify_node() finds any string with a non-numeric type,
 * it fails fast and stops building the string, because the end result would
 * be a NAN.
 */
static void _cxml_stringify_node(void* root, cxml_string* acc, bool check_type){
    cxml_text_node* text;
    cxml_for_each(child, _cxml__get_node_children(root))
    {
        if (_cxml_node_type(child) == CXML_TEXT_NODE){
            text = child;
            // stop searching immediately we confirm that at least one text node's
            // numeric type is NaN
            if (check_type && text->number_value.type == CXML_NUMERIC_NAN_T){
                break;
            }
            cxml_string_str_append(acc, &text->value);
        }
        else if (_cxml_node_type(child) == CXML_ELEM_NODE){
            _cxml_stringify_node(child, acc, check_type);
        }
    }
}

static void __get_string_val(void* node, cxml_string* acc, bool check_type){
    /*
     * get the string value of a valid cxml node
     */
    if (!node) return;
    switch (_cxml_node_type(node))
    {
        case CXML_TEXT_NODE:
        {
            cxml_string_str_append(acc, &_unwrap_cxtext_node(node)->value);
            break;
        }
        case CXML_ROOT_NODE:
        case CXML_ELEM_NODE:
        {
            _cxml_stringify_node(node, acc, check_type);
            break;
        }
        case CXML_COMM_NODE:
        {
            cxml_string_str_append(acc, &_unwrap_cxcomm_node(node)->value);
            break;
        }
        case CXML_ATTR_NODE:
        {
            cxml_attr_node *attr = node;
            cxml_string_str_append(acc, &attr->value);
            break;
        }
        case CXML_PI_NODE:
        {
            cxml_string_str_append(acc, &_unwrap_cxpi_node(node)->value);
            break;
        }
        // CXML_XHDR_NODE & CXML_DTD_NODE not considered
        default:
            break;
    }
}


/*
 * xpath cxml-node value getters
 */
void _cxml_xp__node_string_val(void* node /*: cxml node*/, cxml_string* acc){
    __get_string_val(node, acc, false);
}

void _cxml_xp__node_num_val(void* node /*: cxml node*/, cxml_number *num){
    /*
     * get the number value of a valid cxml node
     */

    switch(_cxml_get_node_type(node))
    {
        case CXML_TEXT_NODE:
            *num = _unwrap_cxtext_node(node)->number_value;
            break;
        case CXML_ATTR_NODE:
            *num = _unwrap_cxattr_node(node)->number_value;
            break;
        default:
        {
            cxml_string str = new_cxml_string();
            __get_string_val(node, &str, true);
            *num = cxml_literal_to_num(&str);
            cxml_string_free(&str);
        }
        break;
    }
}

void _cxml_xp__node_name_val(void *node /*: cxml node*/, cxml_string *name){
    /*
     * get the name (qname - qualified name) value of a valid cxml node
     */
    cxml_string_init(name);
    switch(_cxml_get_node_type(node))
    {
        case CXML_ELEM_NODE:
            cxml_string_dcopy(name, &_unwrap_cxelem_node(node)->name.qname);
            break;
        case CXML_ATTR_NODE:
            cxml_string_dcopy(name, &_unwrap_cxattr_node(node)->name.qname);
            break;
        case CXML_PI_NODE:
            cxml_string_dcopy(name, &_unwrap_cxpi_node(node)->target);
            break;
        default:
            break;
    }
}


void _cxml_xp__node_lname_val(void *node /*: cxml node*/, cxml_string *name){
    /*
     * get the local name value of a valid cxml node
     */
    cxml_string_init(name);
    switch(_cxml_get_node_type(node))
    {
        case CXML_ELEM_NODE:
            _unwrap_cxelem_node(node)->name.lname ?
            cxml_string_append(name, _unwrap_cxelem_node(node)->name.lname,
                               _unwrap_cxelem_node(node)->name.lname_len) :
            cxml_string_append(name, "", 1);
            break;
        case CXML_ATTR_NODE:
            _unwrap_cxattr_node(node)->name.lname ?
            cxml_string_append(name, _unwrap_cxattr_node(node)->name.lname,
                                   _unwrap_cxattr_node(node)->name.lname_len) :
            cxml_string_append(name, "", 1);
            break;
        default:
            break;
    }
}

/*
 * xpath data conversion functions
 */
void _cxml_xp_data_to_numeric(_cxml_xp_data *data, cxml_number *num){
    switch(data->type)
    {
        case CXML_XP_DATA_NUMERIC:
        {
            *num = data->number;
            break;
        }
        case CXML_XP_DATA_STRING:
        {
            *num = cxml_literal_to_num(&data->str);
            break;
        }
        case CXML_XP_DATA_BOOLEAN:
        {
            // cast boolean to numeric (number)
            num->dec_val = data->boolean ? 1.0 : 0.0;
            num->type = CXML_NUMERIC_DOUBLE_T;
            break;
        }
        case CXML_XP_DATA_NODESET:
        {
            if (cxml_set_size(&data->nodeset)){
                // for a node set, we convert the first node in
                // the node set in document order to a cxml_number
                _cxml_xp__node_num_val(cxml_set_get(&data->nodeset, 0), num);
            }
            break;
        }
        default: break;
    }
}

void _cxml_xp_data_to_string(_cxml_xp_data *data, cxml_string *str) {
#define _BUFF_SIZE  100
    switch (data->type)
    {
        case CXML_XP_DATA_NUMERIC:
        {
            cxml_number* num = &data->number;
            char buff[_BUFF_SIZE] = {0};
            int chars;
            if (num->type == CXML_NUMERIC_NAN_T){
                cxml_string_append(str, "NaN", 3);
            }else{
                chars = snprintf(buff, _BUFF_SIZE, "%lf", num->dec_val);
                cxml_string_append(str, buff, chars);
            }
            break;
        }
        case CXML_XP_DATA_STRING:
        {
            cxml_string_str_append(str, &data->str);
            break;
        }
        case CXML_XP_DATA_BOOLEAN:
        {
            if (data->boolean){
                cxml_string_append(str, "true", 4);
            }else{
                cxml_string_append(str, "false", 5);
            }
            break;
        }
        case CXML_XP_DATA_NODESET:
        {
            // for a node set, we obtain the string value of the first node in
            // the node set in document order.
            if (cxml_set_size(&data->nodeset)){
                _cxml_xp__node_string_val(cxml_set_get(&data->nodeset, 0), str);
            }
            break;
        }
        default: break;
    }
#undef _BUFF_SIZE
}

void _cxml_xp_data_to_boolean(_cxml_xp_data *data, bool *bol){
    switch (data->type)
    {
        case CXML_XP_DATA_NUMERIC:
        {
            cxml_number *num = &data->number;
            if (num->type == CXML_NUMERIC_NAN_T){
                *bol = 0;
            }else{
                *bol = (num->dec_val != 0) ? 1 : 0;
            }
            break;
        }
        case CXML_XP_DATA_STRING:
        {
            *bol = cxml_string_len(&data->str) ? 1 : 0;
            break;
        }
        case CXML_XP_DATA_BOOLEAN:
        {
            *bol = data->boolean;
            break;
        }
        case CXML_XP_DATA_NODESET:
        {
            *bol = cxml_set_size(&data->nodeset) ? 1 : 0;
            break;
        }
        default:
            break;
    }
}


/*
 * xpath data management functions
 */
void _cxml_xp_data_free(_cxml_xp_data *data){
    if (data->type == CXML_XP_DATA_STRING){
        cxml_string_free(&data->str);
    }else if (data->type == CXML_XP_DATA_NODESET){
        cxml_set_free(&data->nodeset);
    }
    FREE(data);
}

void _cxml_xp_data_clear(_cxml_xp_data *data){
    // clear resets the contents of data for re-use.
    // no freeing.
    switch(data->type)
    {
        case CXML_XP_DATA_STRING:
        {
            cxml_string_free(&data->str);
            break;
        }
        case CXML_XP_DATA_NODESET:
        {
            cxml_set_free(&data->nodeset);
            break;
        }
        case CXML_XP_DATA_NUMERIC:
        {
            if (data->number.type == CXML_NUMERIC_DOUBLE_T){
                data->number.dec_val = 0;
            }
            data->number.type = CXML_NUMERIC_NAN_T;
            break;
        }
        case CXML_XP_DATA_BOOLEAN:
        {
            data->boolean = 0;
            break;
        }
        default: break;
    }
    _cxml_xp_data_init(&data);
}

