/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "xpath/cxxplib.h"
#include <math.h>


/**
 * Core Functions
 **/
static void __cxml_xp_boolean_fn(cxml_xp_functioncall *node,
                                 _cxml_xp_data *(*_pop)(void),
                                 void (*_push)(_cxml_xp_data *));

static void __cxml_xp_true_fn(cxml_xp_functioncall *node,
                              _cxml_xp_data *(*_pop)(void),
                              void (*_push)(_cxml_xp_data *));

static void __cxml_xp_false_fn(cxml_xp_functioncall *node,
                               _cxml_xp_data *(*_pop)(void),
                               void (*_push)(_cxml_xp_data *));

static void __cxml_xp_last_fn(cxml_xp_functioncall *node,
                              _cxml_xp_data *(*_pop)(void),
                              void (*_push)(_cxml_xp_data *));

static void __cxml_xp_count_fn(cxml_xp_functioncall *node,
                               _cxml_xp_data *(*_pop)(void),
                               void (*_push)(_cxml_xp_data *));

static void __cxml_xp_position_fn(cxml_xp_functioncall *node,
                                  _cxml_xp_data *(*_pop)(void),
                                  void (*_push)(_cxml_xp_data *));

static void __cxml_xp_namespace_uri_fn(cxml_xp_functioncall *node,
                                       _cxml_xp_data *(*_pop)(void),
                                       void (*_push)(_cxml_xp_data *));

static void __cxml_xp_not_fn(cxml_xp_functioncall *node,
                             _cxml_xp_data *(*_pop)(void),
                             void (*_push)(_cxml_xp_data *));

static void __cxml_xp_lang_fn(cxml_xp_functioncall *node,
                              _cxml_xp_data *(*_pop)(void),
                              void (*_push)(_cxml_xp_data *));

static void __cxml_xp_string_fn(cxml_xp_functioncall *node,
                                _cxml_xp_data *(*_pop)(void),
                                void (*_push)(_cxml_xp_data *));


static void __cxml_xp_number_fn(cxml_xp_functioncall *node,
                                _cxml_xp_data *(*_pop)(void),
                                void (*_push)(_cxml_xp_data *));

static void __cxml_xp_contains_fn(cxml_xp_functioncall *node,
                                  _cxml_xp_data *(*_pop)(void),
                                  void (*_push)(_cxml_xp_data *));

static void __cxml_xp_sum_fn(cxml_xp_functioncall *node,
                             _cxml_xp_data *(*_pop)(void),
                             void (*_push)(_cxml_xp_data *));

static void __cxml_xp_floor_fn(cxml_xp_functioncall *node,
                               _cxml_xp_data *(*_pop)(void),
                               void (*_push)(_cxml_xp_data *));

static void __cxml_xp_ceiling_fn(cxml_xp_functioncall *node,
                                 _cxml_xp_data *(*_pop)(void),
                                 void (*_push)(_cxml_xp_data *));

static void __cxml_xp_round_fn(cxml_xp_functioncall *node,
                               _cxml_xp_data *(*_pop)(void),
                               void (*_push)(_cxml_xp_data *));

static void __cxml_xp_name_fn(cxml_xp_functioncall *node,
                              _cxml_xp_data *(*_pop)(void),
                              void (*_push)(_cxml_xp_data *));

static void __cxml_xp_lname_fn(cxml_xp_functioncall *node,
                               _cxml_xp_data *(*_pop)(void),
                               void (*_push)(_cxml_xp_data *));

static void __cxml_xp_concat_fn(cxml_xp_functioncall *node,
                                _cxml_xp_data *(*_pop)(void),
                                void (*_push)(_cxml_xp_data *));

static void __cxml_xp_starts_with_fn(cxml_xp_functioncall *node,
                                     _cxml_xp_data *(*_pop)(void),
                                     void (*_push)(_cxml_xp_data *));

static void __cxml_xp_string_length_fn(cxml_xp_functioncall *node,
                                       _cxml_xp_data *(*_pop)(void),
                                       void (*_push)(_cxml_xp_data *));


struct _cxml_xp_func_LU fn_LU_table[] = {
        // name         arg-count omittable     return-type         function-pointer
        {"boolean",         1,      0,      CXML_XP_RET_BOOLEAN,    __cxml_xp_boolean_fn},
        {"ceiling",         1,      0,      CXML_XP_RET_NUMBER,     __cxml_xp_ceiling_fn},
        {"concat",         -2,      0,      CXML_XP_RET_STRING,     __cxml_xp_concat_fn},  // -2 -> variadic function requiring at least 2 arguments
        {"contains",        2,      0,      CXML_XP_RET_BOOLEAN,    __cxml_xp_contains_fn},
        {"count",           1,      0,      CXML_XP_RET_NUMBER,     __cxml_xp_count_fn},
        {"false",           0,      0,      CXML_XP_RET_BOOLEAN,    __cxml_xp_false_fn},
        {"floor",           0,      0,      CXML_XP_RET_NUMBER,     __cxml_xp_floor_fn},
        {"lang",            1,      0,      CXML_XP_RET_BOOLEAN,    __cxml_xp_lang_fn},
        {"last",            0,      0,      CXML_XP_RET_NUMBER,     __cxml_xp_last_fn},
        {"local-name",      1,      1,      CXML_XP_RET_STRING,     __cxml_xp_lname_fn},
        {"name",            1,      1,      CXML_XP_RET_STRING,     __cxml_xp_name_fn},
        {"namespace-uri",   1,      1,      CXML_XP_RET_STRING,     __cxml_xp_namespace_uri_fn},
        {"not",             1,      0,      CXML_XP_RET_BOOLEAN,    __cxml_xp_not_fn},
        {"number",          1,      1,      CXML_XP_RET_NUMBER,     __cxml_xp_number_fn},
        {"position",        0,      0,      CXML_XP_RET_NUMBER,     __cxml_xp_position_fn},
        {"round",           1,      0,      CXML_XP_RET_NUMBER,     __cxml_xp_round_fn},
        {"starts-with",     2,      0,      CXML_XP_RET_BOOLEAN,    __cxml_xp_starts_with_fn},
        {"string",          1,      1,      CXML_XP_RET_STRING,     __cxml_xp_string_fn},
        {"string-length",   1,      1,      CXML_XP_RET_NUMBER,     __cxml_xp_string_length_fn},
        {"sum",             1,      0,      CXML_XP_RET_NUMBER,     __cxml_xp_sum_fn},
        {"true",            0,      0,      CXML_XP_RET_BOOLEAN,    __cxml_xp_true_fn}
};


// return true if a function found at the given position `pos`
// uses the context node in its computation
bool _is_poisonous_by_computation(int pos){
    switch (pos)
    {
        case 0x07:  // lang()
        case 0x08:  // last()
        case 0xe:  return true; // position()
        default:    return false;
    }
}

// return true if a function found at the given position `pos`
// in the lookup table `fn_LU_table` uses the context node as default argument
bool _is_poisonous_by_default_arg(int pos){
    switch (pos)
    {
        case 0x09:                  // local-name()
        case 0xa:                   // name()
        case 0xb:                   // namespace-uri()
        case 0xd:                   // number()
        case 0x11:                  // string()
        case 0x12:  return true;    // string-length()
        default:    return false;
    }
}


// v, w, x, y, z
static int _cxml_xp_find_fn(const char* n1/*finding n1*/){
    int left = 0, right = (sizeof(fn_LU_table) / sizeof(struct _cxml_xp_func_LU)) - 1;
    int mid, ret;
    while (left <= right){
        mid = (left + right) >> 1;
        ret = strcmp(n1, fn_LU_table[mid].name);
        if (ret==0){
            return mid;
        }else if (ret < 0){  // comes before
            right = mid - 1;
        }else if (ret > 0){ // comes after
            left = mid + 1;
        }
    }
    return -1;
}

static struct _cxml_xp_func_LU_val _cxml_xp_new_fn_LU_val() {
    return (struct _cxml_xp_func_LU_val) {-1, -1, 0};
}


struct _cxml_xp_func_LU_val _cxml_xp_lookup_fn_name(cxml_string* name, int arity){
    int index = _cxml_xp_find_fn(cxml_string_as_raw(name));
    struct _cxml_xp_func_LU_val ret = _cxml_xp_new_fn_LU_val();
    if (index != -1) {
        if ((fn_LU_table[index].arity == arity)  // check arity
            || ((fn_LU_table[index].arity - fn_LU_table[index].omittable) == arity)     // check if arg is omittable
            || (fn_LU_table[index].arity < 0 && arity >= abs(fn_LU_table[index].arity))) // check if func is variadic
        {
            // function matches perfectly in name and arity
            ret.arity = arity;
            ret.pos = index;
            ret.ret_type = fn_LU_table[index].ret_type;
            return ret;
        }
        ret.arity = fn_LU_table[index].arity;
        ret.pos = -2; // flag for when the function is found but different arity
    }
    return ret;
}


/*
 * Helper functions
 */

static void __name_fn(cxml_xp_functioncall *node,
               _cxml_xp_data *(*_pop)(void),
               void (*_push)(_cxml_xp_data *),
               void (*name_val_fn)(void * /*: cxml node*/, cxml_string *))
{
    if (cxml_list_is_empty(&node->args)){
        _cxml_xp_data *data = _cxml_xp_new_data();
        cxml_string_init(&data->str);
        name_val_fn(_xpath_parser.context.ctx_node, &data->str);
        data->type = CXML_XP_DATA_STRING;
        _push(data);
    }else{
        cxml_for_each(arg, &node->args){
            cxml_xp_visit(arg);
        }
        _cxml_xp_data* res = _pop();
        if (res->type == CXML_XP_DATA_NODESET){
            void *n = cxml_set_get(&res->nodeset, 0);
            _cxml_xp_data_clear(res);
            cxml_string_init(&res->str);
            name_val_fn(n, &res->str);
        }else{
            _cxml_xp_data_clear(res);
        }
        res->type = CXML_XP_DATA_STRING;
        _push(res);
    }
}

static cxml_attr_node *_xml_lang_attr(cxml_elem_node *elem){
    cxml_attr_node *lang;
    do{
        if (elem->attributes){
            lang = cxml_table_get(elem->attributes, "xml:lang");
            if (lang) return lang;
        }
        elem = elem->parent;
    }while (_cxml_node_type(elem) != CXML_ROOT_NODE);
    return NULL;
}

static void _normalize_lang(cxml_string *str, cxml_string *n_str){
    char buff[cxml_string_len(str)];
    for (size_t i=0; i<sizeof(buff); i++){
        buff[i] = tolower((unsigned char)str->_raw_chars[i]);
    }
    cxml_string_append(n_str, buff, sizeof(buff));
}

static void _get_uri(void *node, cxml_string *uri){
    if (_cxml_node_type(node) == CXML_ELEM_NODE){
        cxml_elem_node *elem = node;
        if (elem->namespace){
            cxml_string_str_append(uri, &elem->namespace->uri);
        }
    }else if (_cxml_node_type(node) == CXML_ATTR_NODE){
        cxml_attr_node *attr = node;
        if (attr->namespace){
            cxml_string_str_append(uri, &attr->namespace->uri);
        }
    }
}


/**
 *
 * Node Set Functions
 */

void __cxml_xp_count_fn(cxml_xp_functioncall *node,
                        _cxml_xp_data *(*_pop)(void),
                        void (*_push)(_cxml_xp_data *))
{
    /* count(node-set) -> number
     * The count function returns the number of nodes in the argument node-set.
     */
    cxml_for_each(arg, &node->args){
        cxml_xp_visit(arg);
    }
    _cxml_xp_data* res = _pop();
    if (res->type == CXML_XP_DATA_NODESET){
        int size = cxml_set_size(&res->nodeset);
        _cxml_xp_data_clear(res);
        res->type = CXML_XP_DATA_NUMERIC;
        res->number = new_cxml_number();
        res->number.type = CXML_NUMERIC_DOUBLE_T;
        res->number.dec_val = size;
        _push(res);
    }else{
        _cxml_xp_data_clear(res);
        res->type = CXML_XP_DATA_NUMERIC;
        res->number = new_cxml_number();
        _push(res);
    }
}

void __cxml_xp_last_fn(cxml_xp_functioncall *node,
                       _cxml_xp_data *(*_)(void),
                       void (*_push)(_cxml_xp_data *))
{
    /* last() -> number
     * The last function returns a number equal to the context size from the
     * expression evaluation context
     */
    (void)node;
    (void)_;
    _cxml_xp_data *data = _cxml_xp_new_data();
    data->type = CXML_XP_DATA_NUMERIC;
    data->number.type = CXML_NUMERIC_DOUBLE_T;
    // returns the last context position which equals the context size
    data->number.dec_val = _xpath_parser.context.ctx_size;
    _push(data);
}

void __cxml_xp_position_fn(cxml_xp_functioncall* node,
                           _cxml_xp_data *(*_)(void),
                           void (*_push)(_cxml_xp_data *))
{
    /* position() -> number
     * The position function returns a number equal to the context
     * position from the expression evaluation context.
     */
    (void)node;
    (void)_;
    _cxml_xp_data *data = _cxml_xp_new_data();
    data->type = CXML_XP_DATA_NUMERIC;
    data->number.type = CXML_NUMERIC_DOUBLE_T;
    // get the position of the context node.
    data->number.dec_val = _xpath_parser.context.ctx_pos;
    _push(data);
}

void __cxml_xp_namespace_uri_fn(cxml_xp_functioncall *node,
                        _cxml_xp_data *(*_pop)(void),
                        void (*_push)(_cxml_xp_data *))
{
    /* namespace-uri(node-set?) -> string
     * The namespace-uri function returns the namespace URI of the expanded-name
     * of the node in the argument node-set that is first in document order.
     * If the argument node-set is empty, the first node has no expanded-name,
     * or the namespace URI of the expanded-name is null, an empty string is returned.
     * If the argument is omitted, it defaults to a node-set with the context node as its only member
     */
    cxml_string uri = new_cxml_string();
    if (cxml_list_is_empty(&node->args)){
        _get_uri(_xpath_parser.context.ctx_node, &uri);
    }else{
        cxml_for_each(arg, &node->args){
            cxml_xp_visit(arg);
        }
        _cxml_xp_data *d = _pop();
        if (d->type == CXML_XP_DATA_NODESET && !cxml_set_is_empty(&d->nodeset)){
            _get_uri(cxml_set_get(&d->nodeset, 0), &uri);
        }
    }
    _cxml_xp_data *d = _cxml_xp_new_data();
    d->type = CXML_XP_DATA_STRING;
    d->str = uri;
    _push(d);
}


/**
 *
 * Boolean Functions
 */
void __cxml_xp_boolean_fn(cxml_xp_functioncall *node,
                          _cxml_xp_data *(*_pop)(void),
                          void (*_push)(_cxml_xp_data *))
{
    /* boolean(object) -> boolean
     * The boolean function converts its argument to a boolean.
     */
    cxml_for_each(arg, &node->args){
        cxml_xp_visit(arg);
    }
    _cxml_xp_data* res = _pop();
    bool ret;
    _cxml_xp_data_to_boolean(res, &ret);
    _cxml_xp_data_clear(res);
    res->type = CXML_XP_DATA_BOOLEAN;
    res->boolean = ret;
    _push(res);
}

void __cxml_xp_true_fn(cxml_xp_functioncall *node,
                       _cxml_xp_data *(*_)(void),
                       void (*_push)(_cxml_xp_data *))
{
    /* true() -> boolean
     * The true function returns true.
     */
    (void)node;
    (void)_;
    _cxml_xp_data *data = _cxml_xp_new_data();
    data->type = CXML_XP_DATA_BOOLEAN;
    data->boolean = true;
    _push(data);
}

void __cxml_xp_false_fn(cxml_xp_functioncall *node,
                        _cxml_xp_data *(*_)(void),
                        void (*_push)(_cxml_xp_data *))
{
    /* false() -> boolean
     * The false function returns false.
     */
    (void)node;
    (void)_;
    _cxml_xp_data *data = _cxml_xp_new_data();
    data->type = CXML_XP_DATA_BOOLEAN;
    data->boolean = false;
    _push(data);
}

void __cxml_xp_not_fn(cxml_xp_functioncall *node,
                      _cxml_xp_data *(*_pop)(void),
                      void (*_push)(_cxml_xp_data *))
{
    /* not(boolean) -> boolean
     *  The not function returns true if its argument is false, and false otherwise
     */
    cxml_for_each(arg, &node->args){
        cxml_xp_visit(arg);
    }
    _cxml_xp_data* res = _pop();
    bool ret;
    _cxml_xp_data_to_boolean(res, &ret);
    _cxml_xp_data_clear(res);
    res->type = CXML_XP_DATA_BOOLEAN;
    res->boolean = !ret;
    _push(res);
}

void __cxml_xp_lang_fn(cxml_xp_functioncall *node,
                       _cxml_xp_data *(*_pop)(void),
                       void (*_push)(_cxml_xp_data *))
{
    /* lang(string) -> boolean
     *  The lang function returns true or false depending on whether the language
     *  of the context node as specified by xml:lang attributes is the same as or
     *  is a sublanguage of the language specified by the argument string.
     */
    cxml_for_each(arg, &node->args){
        cxml_xp_visit(arg);
    }
    _cxml_xp_data* res = _pop();
    bool ret = 0;
    if (res->type == CXML_XP_DATA_STRING
        && _cxml_node_type(_xpath_parser.context.ctx_node) == CXML_ELEM_NODE)
    {
        // we need to find xml:lang attribute
        cxml_attr_node *lang = _xml_lang_attr(_xpath_parser.context.ctx_node);
        if (lang){
            cxml_string l_str = new_cxml_string();
            cxml_string r_str = new_cxml_string();
            _normalize_lang(&lang->value, &l_str);
            _normalize_lang(&res->str, &r_str);
            if (cxml_string_str_startswith(&l_str, &r_str)){
                char *raw = cxml_string_as_raw(&l_str);
                unsigned len = cxml_string_len(&r_str);
                if (*(raw + len) == '\0' || *(raw + len) == '-') ret = 1;
            }
            cxml_string_free(&l_str);
            cxml_string_free(&r_str);
        }
    }
    _cxml_xp_data_clear(res);
    res->type = CXML_XP_DATA_BOOLEAN;
    res->boolean = ret;
    _push(res);
}


/**
 *
 * String Functions
 */

void __cxml_xp_string_fn(cxml_xp_functioncall *node,
                         _cxml_xp_data *(*_pop)(void),
                         void (*_push)(_cxml_xp_data *))
{
    /* string(object?) -> string
     * The string function converts an object to a string.
     * If the argument is omitted, it defaults to a node-set with
     * the context node as its only member.
     */
    cxml_string str = new_cxml_string();
    if (cxml_list_is_empty(&node->args)){
        _cxml_xp__node_string_val(&_xpath_parser.context.ctx_node, &str);
        _cxml_xp_data *data = _cxml_xp_new_data();
        data->type = CXML_XP_DATA_STRING;
        data->str = str;
        _push(data);
    }else{
        cxml_for_each(arg, &node->args){
            cxml_xp_visit(arg);
        }
        _cxml_xp_data* res = _pop();
        // already a string?
        if (res->type == CXML_XP_DATA_STRING){
            // just push
            goto push;
        }
        _cxml_xp_data_to_string(res, &str);
        _cxml_xp_data_clear(res);
        res->type = CXML_XP_DATA_STRING;
        res->str = str;
    push:
        _push(res);
    }
}

void __cxml_xp_contains_fn(cxml_xp_functioncall *node,
                           _cxml_xp_data *(*_pop)(void),
                           void (*_push)(_cxml_xp_data *))
{
    /* contains(string, string) -> boolean
     * The contains function returns true if the first argument string contains the second
       argument string, and otherwise returns false.
     */
    cxml_for_each(arg, &node->args){
        cxml_xp_visit(arg);
    }
    _cxml_xp_data *right = _pop();
    _cxml_xp_data *left = _pop();
    cxml_string l_str = new_cxml_string(),
                r_str = new_cxml_string();
    bool free_l_str = 1, free_r_str = 1;
    // already a string?
    if (left->type == CXML_XP_DATA_STRING){
        // use it's `str` val
        l_str = left->str;
        free_l_str = 0;
    }else{
        // if not, convert to a string
        _cxml_xp_data_to_string(left, &l_str);
    }
    // already a string?
    if (right->type == CXML_XP_DATA_STRING){
        // use it's `str` val
        r_str = right->str;
        free_r_str = 0;
    }else{
        // if not, convert to a string
        _cxml_xp_data_to_string(right, &r_str);
    }
    bool contains = cxml_string_contains(&l_str, &r_str);
    _cxml_xp_data_clear(left);
    _cxml_xp_data_clear(right);
    free_l_str ? cxml_string_free(&l_str) : (void)0;
    free_r_str ? cxml_string_free(&r_str) : (void)0;
    left->type = CXML_XP_DATA_BOOLEAN;
    left->boolean = contains;
    _push(left);
}

void __cxml_xp_name_fn(cxml_xp_functioncall* node,
                       _cxml_xp_data *(*_pop)(void),
                       void (*_push)(_cxml_xp_data *))
{
    /*
     * name(node-set?) -> string
     * The name function returns a string containing a QName representing the expanded-name
     * of the node in the argument node-set that is first in document order.
     * If the argument node-set is empty or the first node has no expanded-name,
     * an empty string is returned. If the argument is omitted,
     * it defaults to a node-set with the context node as its only member.
     */
    __name_fn(node, _pop, _push, _cxml_xp__node_name_val);
}


void __cxml_xp_lname_fn(cxml_xp_functioncall *node,
                        _cxml_xp_data *(*_pop)(void),
                        void (*_push)(_cxml_xp_data *))
{
    /*
     * local-name(node-set?) -> string
     * The local-name function returns the local part of the expanded-name of the node in
     * the argument node-set that is first in document order. If the argument node-set is
     * empty or the first node has no expanded-name, an empty string is returned.
     * If the argument is omitted, it defaults to a node-set with the context node as its only member.
    */
    __name_fn(node, _pop, _push, _cxml_xp__node_lname_val);
}

void __cxml_xp_concat_fn(cxml_xp_functioncall* node,
                       _cxml_xp_data *(*_pop)(void),
                       void (*_push)(_cxml_xp_data *))
{
    /*
     * concat(string, string, string*) -> string
     * The concat function returns the concatenation of its arguments.
     */
    cxml_for_each(arg, &node->args)
    {
        cxml_xp_visit(arg);
    }
    cxml_string conc = new_cxml_string();
    int size = cxml_list_size(&node->args);
    cxml_list v_args = new_cxml_list();
    while (size > 0){
        // reverse the order of the visited arguments to ensure its in the right order
        cxml_list_insert(&v_args, _pop(), true);
        size--;
    }
    cxml_for_each(v_arg, &v_args)
    {
        _cxml_xp_data_to_string(v_arg, &conc);
    }
    cxml_list_free(&v_args);
    _cxml_xp_data *res = _cxml_xp_new_data();
    res->type = CXML_XP_DATA_STRING;
    res->str = conc;
    _push(res);
}

void __cxml_xp_starts_with_fn(cxml_xp_functioncall* node,
                         _cxml_xp_data *(*_pop)(void),
                         void (*_push)(_cxml_xp_data *))
{
    /* starts-with(string, string) -> boolean
     * The starts-with function returns true if the first argument string starts with
     * the second argument string, and otherwise returns false.
     */
    cxml_for_each(arg, &node->args)
    {
        cxml_xp_visit(arg);
    }
    cxml_string first = new_cxml_string(),
                second = new_cxml_string();
    _cxml_xp_data *sec = _pop();
    _cxml_xp_data *fst = _pop();
    bool free_first = 1, free_second = 1;
    // already a string?
    if (fst->type == CXML_XP_DATA_STRING){
        // use it's `str` val
        first = fst->str;
        free_first = 0;
    }else{
        // if not, convert to a string
        _cxml_xp_data_to_string(fst, &first);
    }
    // already a string?
    if (sec->type == CXML_XP_DATA_STRING){
        // use it's `str` val
        second = sec->str;
        free_second = 0;
    }else{
        // if not, convert to a string
        _cxml_xp_data_to_string(fst, &first);
    }
    bool startswith = cxml_string_str_startswith(&first, &second);
    _cxml_xp_data_clear(fst);
    _cxml_xp_data_clear(sec);
    free_first ? cxml_string_free(&first) : (void)0;
    free_second ? cxml_string_free(&second) : (void)0;
    fst->type = CXML_XP_DATA_BOOLEAN;
    fst->boolean = startswith;
    _push(fst);
}

extern bool is_valid_utf8_start(const char *raw);

inline static int _len(cxml_string *str){
    char *raw = cxml_string_as_raw(str);
    if (!raw) return 0;
    // for len, only utf-8 is actually supported (for now)
    if (is_valid_utf8_start(raw)) return cxml_string_mb_len(str);
    else return (int)cxml_string_len(str);
}

void __cxml_xp_string_length_fn(cxml_xp_functioncall *node,
                           _cxml_xp_data *(*_pop)(void),
                           void (*_push)(_cxml_xp_data *))
{
    /* string-length(string?) -> number
     * The string-length returns the number of characters in the string.
     * If the argument is omitted, it defaults to the context node converted to a string,
     * in other words the string-value of the context node.
     */
    _cxml_xp_data *d;
    int len;
    if (cxml_list_is_empty(&node->args)){
        d = _cxml_xp_new_data();
        cxml_string tmp = new_cxml_string();
        _cxml_xp__node_string_val(_xpath_parser.context.ctx_node, &tmp);
        len = _len(&tmp);
        cxml_string_free(&tmp);
    }else{
        cxml_for_each(arg, &node->args) {
            cxml_xp_visit(arg);
        }
        d = _pop();
        if (d->type == CXML_XP_DATA_STRING){
            len = _len(&d->str);
        }else{
            cxml_string tmp = new_cxml_string();
            _cxml_xp_data_to_string(d, &tmp);
            len = _len(&tmp);
            cxml_string_free(&tmp);
        }
    }
    _cxml_xp_data_clear(d);
    d->type = CXML_XP_DATA_NUMERIC;
    d->number.dec_val = len;
    _push(d);
}

/**
 *
 * Number Functions
 */

void __cxml_xp_number_fn(cxml_xp_functioncall* node,
                         _cxml_xp_data *(*_pop)(void),
                         void (*_push)(_cxml_xp_data *))
{
    /* number(object?) -> number
     * The number function converts its argument to a number.
     * If the argument is omitted, it defaults to a node-set with
     * the context node as its only member.
     */
    if (cxml_list_is_empty(&node->args)){
        _cxml_xp_data *d = _cxml_xp_new_data();
        d->type = CXML_XP_DATA_NUMERIC;
        _cxml_xp__node_num_val(&_xpath_parser.context.ctx_node, &d->number);
        _push(d);
    }else{
        cxml_for_each(arg, &node->args){
            cxml_xp_visit(arg);
        }
        _cxml_xp_data* res = _pop();
        cxml_number num = new_cxml_number();
        _cxml_xp_data_to_numeric(res, &num);
        _cxml_xp_data_clear(res);
        res->type = CXML_XP_DATA_NUMERIC;
        res->number = num;
        _push(res);
    }
}

void __cxml_xp_sum_fn(cxml_xp_functioncall* node,
                      _cxml_xp_data *(*_pop)(void),
                      void (*_push)(_cxml_xp_data *))
{
    /* sum(node-set) -> number
     * The sum function returns the sum, for each node in the argument node-set, of the result
       of converting the string-values of the node to a number.
     */
    cxml_for_each(arg, &node->args){
        cxml_xp_visit(arg);
    }
    _cxml_xp_data* res = _pop();
    cxml_number num = new_cxml_number();
    if (res->type != CXML_XP_DATA_NODESET){
        goto push;
    }
    double acc_d = 0;
    bool nan_flag = 0;
    cxml_for_each(_node, &res->nodeset.items)
    {
        _cxml_xp__node_num_val(_node, &num);
        if (num.type == CXML_NUMERIC_DOUBLE_T){
            acc_d += num.dec_val;
        }else{
            nan_flag = 1;
            break;
        }
    }
    if (!nan_flag){
        num.type = CXML_NUMERIC_DOUBLE_T;
        num.dec_val = acc_d;
    }
    push:
    _cxml_xp_data_clear(res);
    res->type = CXML_XP_DATA_NUMERIC;
    res->number = num;
    _push(res);
}

void __cxml_xp_floor_fn(cxml_xp_functioncall *node,
                            _cxml_xp_data *(*_pop)(void),
                            void (*_push)(_cxml_xp_data *))
{
    /*
     * floor(number) -> number
     * The floor function returns the largest (closest to positive infinity)
     * number that is not greater than the argument and that is an integer.
     */
    cxml_for_each(arg, &node->args){
        cxml_xp_visit(arg);
    }
    cxml_number number;
    _cxml_xp_data *d = _pop();
    if (d->type != CXML_XP_DATA_NUMERIC){
        _cxml_xp_data_to_numeric(d, &number);
    }else{
        number = d->number;
    }
    _cxml_xp_data_clear(d);
    d->type = CXML_XP_DATA_NUMERIC;
    if (number.type != CXML_NUMERIC_NAN_T){
        d->number.type = CXML_NUMERIC_DOUBLE_T;
        d->number.dec_val = floor(number.dec_val);
    }
    _push(d);
}

void __cxml_xp_ceiling_fn(cxml_xp_functioncall *node,
                        _cxml_xp_data *(*_pop)(void),
                        void (*_push)(_cxml_xp_data *))
{
    /*
     * ceiling(number) -> number
     * The ceiling function returns the smallest (closest to negative infinity)
     * number that is not less than the argument and that is an integer.
     */
    cxml_for_each(arg, &node->args){
        cxml_xp_visit(arg);
    }
    cxml_number number;
    _cxml_xp_data *d = _pop();
    if (d->type != CXML_XP_DATA_NUMERIC){
        _cxml_xp_data_to_numeric(d, &number);
    }else{
        number = d->number;
    }
    _cxml_xp_data_clear(d);
    d->type = CXML_XP_DATA_NUMERIC;
    if (number.type != CXML_NUMERIC_NAN_T){
        d->number.type = CXML_NUMERIC_DOUBLE_T;
        d->number.dec_val = ceil(number.dec_val);
    }
    _push(d);
}

void __cxml_xp_round_fn(cxml_xp_functioncall *node,
                        _cxml_xp_data *(*_pop)(void),
                        void (*_push)(_cxml_xp_data *))
{
    /*
     * round(number) -> number
     * The round function returns the number that is closest to the argument and that is an integer.
     * If the argument is NaN, then NaN is returned.
     * If the argument is less than zero, but greater than or equal to -0.5, then negative zero is returned.
     */
    cxml_for_each(arg, &node->args){
        cxml_xp_visit(arg);
    }
    cxml_number number;
    _cxml_xp_data *d = _pop();
    if (d->type != CXML_XP_DATA_NUMERIC){
        _cxml_xp_data_to_numeric(d, &number);
    }else{
        number = d->number;
    }
    _cxml_xp_data_clear(d);
    d->type = CXML_XP_DATA_NUMERIC;
    if (number.type != CXML_NUMERIC_NAN_T){
        d->number.type = CXML_NUMERIC_DOUBLE_T;
        if (number.dec_val < 0 && number.dec_val >= -0.5){
            d->number.dec_val = 0;
        }else{
            d->number.dec_val = round(number.dec_val);
        }
    }
    _push(d);
}
