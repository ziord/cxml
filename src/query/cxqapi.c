/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "query/cxqapi.h"


/********************************
 *                              *
 * Utility routines/functions   *
 ********************************
 */

/*
 * check if a node is a valid root or not
 * A root is valid only if it's an element node, or a root node (document node)
 */
inline static bool _is_valid_root(void *root){
    if (!root){
        return false;
    }
    return (_cxml_node_type(root) == CXML_ELEM_NODE || _cxml_node_type(root) == CXML_ROOT_NODE);
}

/*
 * Obtain the root element
 */
inline static cxml_elem_node* _get_root_element(void *root){
    if (_cxml_node_type(root) == CXML_ELEM_NODE){
       return root;
    }else if (_cxml_node_type(root) == CXML_ROOT_NODE){
        return _unwrap__cxnode(root, root)->root_element;
    }
    return NULL;
}

/*
 * Concatenate all text nodes present in an element/root node's children/descendants into `acc` string.
 * if `concat` is present, it is used as the concatenation string between text nodes,
 * if not present, the text nodes are concatenated together with no concatenation string between.
 */
inline static void _stringify_node(
        cxml_list *children,
        cxml_string *acc)
{
    cxml_for_each(child, children)
    {
        if ( _cxml_node_type(child) == CXML_TEXT_NODE){
            cxml_string_str_append(acc, &_unwrap_cxnode(cxml_text_node, child)->value);
        }
        else if (_cxml_node_type(child) == CXML_ELEM_NODE){
            _stringify_node(&_unwrap_cxnode(cxml_elem_node, child)->children, acc);
        }
    }
}

inline static void _gather_nodes(
        cxml_list *nodes,
        _cxml_node_t type,
        cxml_list *gather);

inline static void _stringify(
        cxml_list *children,
        const char *concat,
        size_t concat_len,
        cxml_string *acc)
{
    if (!concat){
        _stringify_node(children, acc);
    }else{
        cxml_list texts = new_cxml_list();
        _gather_nodes(children, CXML_TEXT_NODE, &texts);
        int i = 0, j = cxml_list_size(&texts) - 1;
        cxml_for_each(text, &texts)
        {
            cxml_string_str_append(acc, &_unwrap_cxnode(cxml_text_node, text)->value);
            if (i != j){
                cxml_string_append(acc, concat, concat_len);
            }
            i++;
        }
    }
}

/*
 * Find the comment node whose string value matches `value`
 * `match_specific` if true, ensures exact matches, and if false
 * ensures partial matches.
 */
static cxml_comm_node *_find_matching_comment(
        cxml_elem_node *elem,
        cxml_string *value,
        bool match_specific)
{
    if (match_specific){
        cxml_for_each(child, &elem->children)
        {
            if ((*(_cxml_node_t *)(child) == CXML_COMM_NODE)
                && cxml_string_equals(&_unwrap_cxnode(cxml_comm_node, child)->value, value))
            {
                return child;
            }
        }
    }else{
        cxml_for_each(child, &elem->children)
        {
            if ((*(_cxml_node_t *)(child) == CXML_COMM_NODE)
                && cxml_string_contains(&_unwrap_cxnode(cxml_comm_node, child)->value, value))
            {
                return child;
            }
        }
    }
    return NULL;
}

/*
 * Find the text node whose string value matches `value`
 * `match_specific` if true, ensures exact matches, and if false
 * ensures partial matches.
 */
static cxml_text_node* _find_matching_text(
        cxml_elem_node *elem,
        cxml_string *value,
        bool match_specific)
{
    if (match_specific){
        cxml_for_each(child, &elem->children){
            if ((_cxml_node_type(child) == CXML_TEXT_NODE)
                && cxml_string_equals(&_unwrap_cxnode(cxml_text_node, child)->value, value))
            {
                return child;
            }
        }
    }else{
        cxml_for_each(child, &elem->children){
            if ((_cxml_node_type(child) == CXML_TEXT_NODE)
                && cxml_string_contains(&_unwrap_cxnode(cxml_text_node, child)->value, value))
            {
                return child;
            }
        }
    }
    return NULL;
}

/*
 * Determine if `element` satisfies all rigid query sub-expressions in the
 * _cxml_query object's `q_r_list` (rigid query list) of query expressions
 */
static bool _elem_matches_rigid_query(cxml_elem_node *element, _cxml_query *q_obj){
    int match_count = 0;
    cxml_attr_node *attr;
    cxml_for_each(expr, &q_obj->q_r_list)
    {
        if (((_cxml_q*)expr)->q_attr)
        {
            if (((_cxml_q *) expr)->q_attr->flags & _CXQ_MATCH_KEY_ONLY){
                if (cxml_table_get(
                        element->attributes,
                        cxml_string_as_raw(((_cxml_q *) expr)->q_attr->key)))
                {
                    match_count++;
                } else {
                    return NULL;
                }
            }
            else if ((attr = cxml_table_get(
                    element->attributes,
                    cxml_string_as_raw(((_cxml_q *) expr)->q_attr->key))))
            {
                if ((((_cxml_q*)expr)->q_attr->flags & _CXQ_MATCH_EXACT)
                    && (cxml_string_equals(&attr->value, ((_cxml_q *) expr)->q_attr->value)))
                {
                    match_count++;
                }
                else if ((((_cxml_q*)expr)->q_attr->flags & _CXQ_MATCH_PARTIAL)
                         && (cxml_string_contains(&attr->value, ((_cxml_q *) expr)->q_attr->value)))
                {
                    match_count++;
                }
                else {
                    return NULL;
                }
            }
        }
        else if (((_cxml_q*)expr)->q_text)
        {
            if (((_cxml_q*)expr)->q_text->flags & _CXQ_MATCH_ANY){
                if (element->has_text) match_count++; else return NULL;
            }
            else if (((_cxml_q*)expr)->q_text->flags & _CXQ_MATCH_EXACT){
                if (_find_matching_text(
                        element,
                        ((_cxml_q *) expr)->q_text->text,
                        true))
                {
                    match_count++;
                }else{
                    return NULL;
                }
            }
            else if (((_cxml_q*)expr)->q_text->flags & _CXQ_MATCH_PARTIAL){
                if (_find_matching_text(
                        element,
                        ((_cxml_q *) expr)->q_text->text,
                        false))
                {
                    match_count++;
                } else{
                    return NULL;
                }
            }
        }
        else if (((_cxml_q*)expr)->q_comm)
        {
            if (((_cxml_q*)expr)->q_comm->flags & _CXQ_MATCH_ANY){
                if (element->has_comment) match_count++; else return NULL;
            }
            else if (((_cxml_q*)expr)->q_comm->flags & _CXQ_MATCH_EXACT){
                if (_find_matching_comment(
                        element,
                        ((_cxml_q *) expr)->q_comm->comment,
                        true))
                {
                    match_count++;
                }else{
                    return NULL;
                }
            }
            else if (((_cxml_q*)expr)->q_comm->flags & _CXQ_MATCH_PARTIAL){
                if (_find_matching_comment(
                        element,
                        ((_cxml_q *) expr)->q_comm->comment,
                        false))
                {
                    match_count++;
                }else{
                    return NULL;
                }
            }
        }
    }
    return (match_count == cxml_list_size(&q_obj->q_r_list));
}

/*
 * Determine if `element` satisfies at lease one optional query sub-expression in the
 * _cxml_query object's `q_o_list` (optional query list) of query expressions
 */
static bool _elem_matches_optional_query(void *child, _cxml_query *q_obj){
    cxml_attr_node *attr;
    cxml_for_each(expr, &q_obj->q_o_list)
    {
        // match by attribute
        if (((_cxml_q*)expr)->q_attr)
        {
            if (((_cxml_q*)expr)->q_attr->flags & _CXQ_MATCH_KEY_ONLY)
            {
                if (cxml_table_get(_unwrap__cxnode(elem, child)->attributes,
                                   cxml_string_as_raw(((_cxml_q *) expr)->q_attr->key)))
                {
                    return true;
                }
            }
            else if ((attr = cxml_table_get(_unwrap__cxnode(elem, child)->attributes,
                                           cxml_string_as_raw(((_cxml_q *) expr)->q_attr->key))))
            {
                if ((((_cxml_q*)expr)->q_attr->flags & _CXQ_MATCH_EXACT)
                    && (cxml_string_equals(&attr->value, ((_cxml_q *) expr)->q_attr->value)))
                {
                    return true;
                }
                else if ((((_cxml_q*)expr)->q_attr->flags & _CXQ_MATCH_PARTIAL)
                         && (cxml_string_contains(&attr->value, ((_cxml_q *) expr)->q_attr->value)))
                {
                    return true;
                }
            }
        }
        // match by text
        else if (((_cxml_q*)expr)->q_text)
        {
            if (((_cxml_q*)expr)->q_text->flags & _CXQ_MATCH_ANY)
            {
                if (_unwrap__cxnode(elem, child)->has_text) return child;
            }
            else if (((_cxml_q*)expr)->q_text->flags & _CXQ_MATCH_EXACT)
            {
                if (_find_matching_text(child, ((_cxml_q *) expr)->q_text->text, true))
                {
                    return true;
                }
            }
            else if (((_cxml_q*)expr)->q_text->flags & _CXQ_MATCH_PARTIAL)
            {
                if (_find_matching_text(child, ((_cxml_q *) expr)->q_text->text, false))
                {
                    return true;
                }
            }
        }
        // match by comment
        else if (((_cxml_q*)expr)->q_comm)
        {
            if (((_cxml_q*)expr)->q_comm->flags & _CXQ_MATCH_ANY)
            {
                if (_unwrap__cxnode(elem, child)->has_comment) return child;
            }
            else if (((_cxml_q*)expr)->q_comm->flags & _CXQ_MATCH_EXACT)
            {
                if (_find_matching_comment(child, ((_cxml_q *) expr)->q_comm->comment, true))
                {
                    return true;
                }
            }
            else if (((_cxml_q*)expr)->q_comm->flags & _CXQ_MATCH_PARTIAL)
            {
                if (_find_matching_comment(child, ((_cxml_q *) expr)->q_comm->comment, false))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

/*
 * Obtain the next cxml object that is a direct sibling of `node` and
 * has the same type as `node`
 */
inline static void *_get_next_node(
        void *node,
        _cxml_node_t type,
        cxml_list *siblings)
{
    bool at_index = 0;
    cxml_for_each(sib, siblings)
    {
        if (_cxml_node_type(sib) == type)
        {
            if (node == sib)
            {
                at_index = 1;
            }
            else if (at_index){
                return sib;
            }
        }
    }
    return NULL;
}

inline static void *_get_prev_node(
        void *node,
        _cxml_node_t type,
        cxml_list *siblings)
{
    void *prev = NULL;
    cxml_for_each(sib, siblings)
    {
        if (_cxml_node_type(sib) == type)
        {
            if (node == sib)
            {
                return prev;
            }
            prev = sib;
        }
    }
    return NULL;
}

/*
 * Obtain the descendants of all element nodes in the list `nodes`
 * The descendants of a node are the children of the node and the
 * descendants of the children of the node
 */
inline static void _descendants(cxml_list *nodes, cxml_list *acc){
    cxml_for_each(child, nodes)
    {
        if (_cxml_node_type(child) == CXML_ELEM_NODE){
            cxml_list_append(acc, child);
            _descendants(&_unwrap_cxnode(cxml_elem_node, child)->children, acc);
        }else{
            cxml_list_append(acc, child);
        }
    }
}

inline static void set_parent_field(void *child, void *parent){
    switch (_cxml_node_type(child))
    {
        case CXML_TEXT_NODE:
            _unwrap__cxnode(text, child)->parent = parent;
            break;
        case CXML_ELEM_NODE:
            _unwrap__cxnode(elem, child)->parent = parent;
            break;
        case CXML_COMM_NODE:
            _unwrap__cxnode(comm, child)->parent = parent;
            break;
        case CXML_ATTR_NODE:
            _unwrap__cxnode(attr, child)->parent = parent;
            break;
        case CXML_PI_NODE:
            _unwrap__cxnode(pi, child)->parent = parent;
            break;
        case CXML_NS_NODE:
            _unwrap__cxnode(ns, child)->parent = parent;
            break;
        case CXML_XHDR_NODE:
            _unwrap__cxnode(xhdr, child)->parent = parent;
            break;
        case CXML_DTD_NODE:
            _unwrap__cxnode(dtd, child)->parent = parent;
            break;
        default: return;
    }
}

inline static void update_parent_fields_after_add(void *child, cxml_elem_node *parent){
    parent->has_child = true;
    parent->is_self_enclosing = false;
    switch (_cxml_node_type(child))
    {
        case CXML_TEXT_NODE:
            parent->has_text = true;
            break;
        case CXML_ELEM_NODE:
            _unwrap__cxnode(elem, child)->has_parent = true;
            break;
        case CXML_COMM_NODE:
            parent->has_comment = true;
            break;
        case CXML_ATTR_NODE:
            parent->has_attribute = true;
            break;
        default:
            break;
    }
}

inline static void update_parent_fields_after_delete(cxml_elem_node *parent){
    // has_text | has_comment | has_child | has_attribute | is_self_enclosing
    parent->is_self_enclosing = cxml_list_is_empty(&parent->children);
    parent->is_namespaced = (bool)parent->namespace;
    parent->has_attribute = cxml_table_size(parent->attributes);

    if (parent->is_self_enclosing){
        parent->has_child = false;
        parent->has_comment = false;
        parent->has_text = false;
        return;
    }

    parent->has_child = true;
    // setting this to false so that it can be reset in the loop below
    parent->has_comment = false;
    parent->has_text = false;

    // we need a better way to update these fields.
    int checks = 0;
    cxml_for_each(node, &parent->children)
    {
        if (checks == 2) return;
        if ((_cxml_node_type(node) == CXML_TEXT_NODE) && !checks){
            parent->has_text = true;
            checks++;
        }else if ((_cxml_node_type(node) == CXML_COMM_NODE) && !checks){
            parent->has_comment = true;
            checks++;
        }
    }
}

inline static void _update_parent(void *parent){
    if (!parent) return;
    if (_cxml_node_type(parent) == CXML_ELEM_NODE)
    {
        update_parent_fields_after_delete(parent);
    }
    else if (_cxml_node_type(parent) == CXML_ROOT_NODE)
    {
        _unwrap__cxnode(root, parent)->has_child = (
                cxml_list_size(&_unwrap__cxnode(root, parent)->children));
        if (!_unwrap__cxnode(root, parent)->has_child){
            _unwrap__cxnode(root, parent)->root_element = NULL;
        }
    }
}

/*
 * Set `parent` node as the parent of `child` node
 */
inline static int link_child_to_parent(void *child, void *parent, const int *index){
    if ((_cxml_node_type(parent) != CXML_ELEM_NODE)
      && (_cxml_node_type(parent) != CXML_ROOT_NODE)) return 0;

    // first, add the child.
    // second, update the parent's respective fields (has_text, has_child, etc.)
    // third, update the child's `parent` field
    cxml_list *children = _cxml__get_node_children(parent);

    if (_cxml_node_type(parent) == CXML_ELEM_NODE){
        if (!index){
            cxml_list_append(children, child);
        }else{
            cxml_list_insert_at_index(children, child, *index);
        }
        update_parent_fields_after_add(child, parent);
    }else{
        // if node is a cxml_root_node, ensure we do not add
        // an extra element if root node already has a root element
        if (_cxml_node_type(child) == CXML_ELEM_NODE){
            if (_unwrap__cxnode(root, parent)->root_element) return 0;
            _unwrap__cxnode(root, parent)->root_element = child;
        }
        if (!index){
            cxml_list_append(children, child);
        }else{
            cxml_list_insert_at_index(children, child, *index);
        }
        _unwrap__cxnode(root, parent)->has_child = true;
    }
    set_parent_field(child, parent);
    return 1;
}

/*
 * Obtain the cxml_literal_t type of a cxml_string object
 */
inline static cxml_literal_t _get_literal_type(cxml_string *v){
    int ret;
    char *raw = cxml_string_as_raw(v);
    int len = _cxml_int_cast cxml_string_len(v);
    if ((ret = _cxml_is_integer(raw, len))){
        return ret == 1 ? CXML_INTEGER_LITERAL : CXML_XINTEGER_LITERAL;
    }else if (_cxml_is_double(raw, len)){
        return CXML_DOUBLE_LITERAL;
    }else{
        return CXML_STRING_LITERAL;
    }
}

/*
 * Find all elements in root (including root itself) by name that matches
 * the tag name specified in the _cxml_query object `q_obj`
 *
 * This is used when the query object doesn't contain any rigid query sub-expressions
 */
static void _cxml__find_all_by_name(
        _cxml_query *q_obj,
        cxml_elem_node *root,
        cxml_list *children,
        cxml_list *acc)
{
    if (root && cxml_string_equals(&root->name.qname, &q_obj->q_name))
    {
        cxml_list_append(acc, root);
    }
    cxml_for_each(child, children)
    {
        if (_cxml_node_type(child) == CXML_ELEM_NODE)
        {
            if (cxml_string_equals(
                    &_unwrap_cxnode(cxml_elem_node, child)->name.qname,
                    &q_obj->q_name))
            {
                cxml_list_append(acc, child);
            }
            _cxml__find_all_by_name(q_obj, NULL, _cxml__get_node_children(child), acc);
        }
    }
}

/*
 * Find all elements in root (including root itself) that satisfies
 * any of the query criteria (rigid or optional) contained in
 * the _cxml_query object `q_obj`
 *
 * This is used when the query object contains both rigid query sub-expression(s)
 * and optional query sub-expression(s) or only rigid query sub-expression(s)
 */
static void _cxml__find_all_by_any(
        _cxml_query *q_obj,
        cxml_elem_node *root,
        cxml_list *children,
        cxml_list *acc)
{
    if (root && cxml_string_equals(&root->name.qname, &q_obj->q_name)){
        if (_elem_matches_rigid_query(root, q_obj)){
            cxml_list_append(acc, root);
        }else if (_elem_matches_optional_query(root, q_obj)){
            cxml_list_append(acc, root);
        }
    }
    cxml_for_each(child, children)
    {
        if (_cxml_node_type(child) == CXML_ELEM_NODE)
        {
            if (cxml_string_equals(
                    &_unwrap_cxnode(cxml_elem_node, child)->name.qname,
                    &q_obj->q_name))
            {
                if (_elem_matches_rigid_query(child, q_obj)){
                    cxml_list_append(acc, child);
                }else if (_elem_matches_optional_query(child, q_obj)){
                    cxml_list_append(acc, child);
                }
            }
            _cxml__find_all_by_any(q_obj, NULL, _cxml__get_node_children(child), acc);
        }
    }
}

/*
 * Find the first element in root (including root itself) by name that matches
 * the tag name specified in the _cxml_query object `q_obj`
 *
 * This is used when the query object doesn't contain any rigid query sub-expressions
 */
static cxml_element_node *_cxml__find_by_name(
        _cxml_query *q_obj,
        cxml_elem_node *root,
        cxml_list *children)
{

    if (root && cxml_string_equals(&root->name.qname, &q_obj->q_name))
    {
        return root;
    }
    cxml_elem_node *elem = NULL;
    cxml_for_each(child, children)
    {
        if (_cxml_node_type(child) == CXML_ELEM_NODE)
        {
            if (cxml_string_equals(&_unwrap_cxnode(cxml_elem_node, child)->name.qname,
                                   &q_obj->q_name))
            {
                return child;
            }
            elem = _cxml__find_by_name(q_obj, NULL, _cxml__get_node_children(child));
            if (elem) return elem;
        }
    }
    return elem;
}

/*
 * Find the first element in root (including root itself) that satisfies
 * any of the query criteria (rigid or optional) contained in
 * the _cxml_query object `q_obj`
 *
 * This is used when the query object contains both rigid query sub-expression(s)
 * and optional query sub-expression(s) or only rigid query sub-expression(s)
 */
static cxml_element_node *_cxml__find_by_any(
        _cxml_query *q_obj,
        cxml_elem_node *root,
        cxml_list *children)
{
    cxml_elem_node *elem = NULL;
    if (root && cxml_string_equals(&root->name.qname, &q_obj->q_name)){
        if (_elem_matches_rigid_query(root, q_obj)){
            return root;
        }else if (_elem_matches_optional_query(root, q_obj)){
            return root;
        }
    }
    cxml_for_each(child, children)
    {
        if (_cxml_node_type(child) == CXML_ELEM_NODE)
        {
            if (cxml_string_equals(&_unwrap__cxnode(elem, child)->name.qname, &q_obj->q_name)){
                if (_elem_matches_rigid_query(child, q_obj)){
                    return child;
                }else if (_elem_matches_optional_query(child, q_obj)){
                    return child;
                }
            }
            elem = _cxml__find_by_any(q_obj, NULL, _cxml__get_node_children(child));
            if (elem) return elem;
        }
    }
    return elem;
}

/*
 * Helper function for finding all elements that satisfies the given query criteria
 */
static void _cxml__find_all(_cxml_query  *q_obj, void *root, cxml_list *acc){
    cxml_elem_node *root_elem = _get_root_element(root);
    if (!root_elem) return;
    // if no rigid, use only tag name as match
    if (cxml_list_is_empty(&q_obj->q_r_list)){
        _cxml__find_all_by_name(q_obj, root_elem, &root_elem->children, acc);
        return;
    }
    // find any criteria specified
    _cxml__find_all_by_any(q_obj, root_elem, &root_elem->children, acc);
}

/*
 * Helper function for finding the first element that satisfies the given query criteria
 */
static cxml_element_node *_cxml__find(_cxml_query  *q_obj, void *root){
    cxml_elem_node *root_elem = _get_root_element(root);
    if (!root_elem) return NULL;
    // if no rigid, use only tag name as match
    if (cxml_list_is_empty(&q_obj->q_r_list)){
        return _cxml__find_by_name(q_obj, root_elem, &root_elem->children);
    }
    // find any criteria specified
    return _cxml__find_by_any(q_obj, root_elem, &root_elem->children);
}

/*
 * Gather all cxml node objects of type `type` into list cxml_list `gather`
 */
inline static void _gather_nodes(cxml_list *nodes, _cxml_node_t type, cxml_list *gather){
    cxml_for_each(node, nodes)
    {
        if (_cxml_node_type(node) == type){
            cxml_list_append(gather, node);
        }
        if (_cxml_node_type(node) == CXML_ELEM_NODE){
            _gather_nodes(_cxml__get_node_children(node), type, gather);
        }
    }
}

/*
 * Delete a valid cxml node object `node`, and disassociate it from its siblings
 */
inline static int _delete_cxml_node(void *child, cxml_list *children, void *parent) {
    int ret = cxml_list_search_delete(children, cxml_list_cmp_raw_items, child);
    cxml_node_free(child);
    _update_parent(parent);
    return ret;
}

/*
 * Drop/remove a valid cxml node object `node`, and disassociate it from its siblings
 */
inline static int _drop_cxml_node(void *child, cxml_list *children, void *parent){
    if (cxml_list_search_delete(children, cxml_list_cmp_raw_items, child))
    {
        _cxml_unset_parent(child);
        _update_parent(parent);
        return 1;
    }
    return 0;
}

/*
 * Drop/remove a valid cxml node object `node`, and disassociate it from its siblings,
 * appending `node` to an accumulator list
 */
inline static int _drop_cxml_node_into(void *child, cxml_list *children, cxml_list *acc){
    if (cxml_list_search_delete(children, cxml_list_cmp_raw_items, child))
    {
        _update_parent(_cxml_node_parent(child));
        _cxml_unset_parent(child);
        cxml_list_append(acc, child);
        return 1;
    }
    return 0;
}

/*
 * Delete all nodes in `nodes`, that has its type equal to `type`,
 * and disassociate each from its siblings
 */
inline static int _delete_cxml_nodes(cxml_list *nodes, _cxml_node_t type){
    cxml_list tmp = new_cxml_list();
    cxml_for_each(node, nodes)
    {
        if (_cxml_node_type(node) == type){
            cxml_list_append(&tmp, node);
        }
    }
    cxml_for_each(obj, &tmp)
    {
        cxml_list_search_delete(nodes, cxml_list_cmp_raw_items, obj);
        _update_parent(_cxml_node_parent(obj));
        cxml_node_free(obj);
    }
    int size = cxml_list_size(&tmp);
    cxml_list_free(&tmp);
    return size;
}

/*
 * Drop/remove all nodes in `nodes`, that has its type equal to `type`,
 * and disassociate each from its siblings
 */
inline static int _drop_cxml_nodes(cxml_list *nodes, _cxml_node_t type, cxml_list *acc){
    int size = cxml_list_size(acc);
    cxml_for_each(node, nodes)
    {
        if (_cxml_node_type(node) == type){
            cxml_list_append(acc, node);
        }
    }

    cxml_for_each(obj, acc)
    {
        if (cxml_list_search_delete(nodes, cxml_list_cmp_raw_items, obj)){
            _update_parent(_cxml_node_parent(obj));
            _cxml_unset_parent(obj);
        }
    }
    return size != cxml_list_size(acc);
}

/*
 * Delete all nodes in `nodes`, that has its type equal to `type`,
 * and disassociate each from its siblings - recursively
 * means, that it'll delete a cxml object node matching the given
 * type, even if the node isn't a direct child of the ancestor node
 * whose children is contained in the list `nodes`.
 */
static int _delete_cxml_nodes_recursive(cxml_list *nodes, _cxml_node_t type){
    cxml_list tmp = new_cxml_list();
    // this gathers elements from `nodes` in document order, i.e.
    // from top to bottom, assuming nodes is sorted in document order.
    // However, if nodes is sorted in a non-document order, then this could
    // lead to errors.
    _gather_nodes(nodes, type, &tmp);
    void *par;
    cxml_for_each(obj, &tmp)
    {
        par = _cxml_node_parent(obj);
        // only proceed to delete if we're certain parent hasn't been deleted yet,
        // because if parent has been deleted, then the node `obj` would have been
        // deleted as well.
        if (par != NULL){
            _delete_cxml_node(obj, _cxml__get_node_children(par), par);
            // NULLify potential parent node
            obj = NULL;
        }
    }
    int size = cxml_list_size(&tmp);
    cxml_list_free(&tmp);
    return size;
}

/*
 * Drop/remove all nodes in `nodes`, that has its type equal to `type`,
 * and disassociate each from its siblings - recursively
 * means, that it'll delete a cxml object node matching the given
 * type, even if the node isn't a direct child of the ancestor node
 * whose children is contained in the list `nodes`.
 */
static int _drop_cxml_nodes_recursive(cxml_list *nodes, _cxml_node_t type, cxml_list *acc){
    cxml_list tmp = new_cxml_list();
    _gather_nodes(nodes, type, &tmp);
    void *par;
    cxml_for_each(obj, &tmp)
    {
        if ((par=_cxml_node_parent(obj))){
            _drop_cxml_node_into(obj, _cxml__get_node_children(par), acc);
        }
    }
    int size = cxml_list_size(&tmp);
    cxml_list_free(&tmp);
    return size;
}


/*
 * Find a node in `nodes`, that has its type equal to `type`
 */
inline static void * _get_node(cxml_list *nodes, _cxml_node_t type){
    cxml_for_each(node, nodes)
    {
        if (_cxml_node_type(node) == type){
            return node;
        }
    }
    return NULL;
}

/*
 * check if a uri is empty, by checking if it contains only spaces
 */
inline static bool _cxml_is_empty_URI(const char *uri, int length){
    for (int i = 0; i < length; i++){
        if (!isspace((unsigned char)uri[i]))
        {
            return 0;
        }
    }
    return 1;
}

/*
 * remove the namespace prefix part of a name
 */
inline static void _remove_ns_prefix(cxml_name *name){
    // remove the prefix part + ':'
    char *raw = cxml_string_as_raw(&name->qname);
    memmove(raw, (raw + name->pname_len + 1), name->lname_len);
    name->pname = NULL;
    name->pname_len = 0;
    // reset local name to point at the updated string
    name->lname = cxml_string_as_raw(&name->qname);
    name->qname._len = name->lname_len;
}

/*
 * helper function for setting the prefix name of a namespace node.
 */
inline static int set_namespace_prefix_helper(
        cxml_namespace_node *ns,
        const char *prefix, bool force_set)
{
    int len = _cxml_int_cast strlen(prefix);
    if (!len) return 0;
    /*
     * The prefix xmlns is used only to declare namespace bindings and is by definition bound
     * to the namespace name http://www.w3.org/2000/xmlns/. It MUST NOT be declared.
     */
    if (cxml_string_llraw_equals((void*)prefix, _CXML_RESERVED_NS_PREFIX_XMLNS,
                                 len, _CXML_RESERVED_NS_PREFIX_XMLNS_LEN))
    {
        return 0;
    }
    /*
     * The prefix xml is by definition bound to the namespace name
     * http://www.w3.org/XML/1998/namespace. It MAY, but need not, be declared,
     * and MUST NOT be bound to any other namespace name. Other prefixes MUST NOT
     * be bound to this namespace name, and it MUST NOT be declared as the default namespace.
     */
    if (cxml_string_llraw_equals((void *) prefix, _CXML_RESERVED_NS_PREFIX_XML,
                                 len, _CXML_RESERVED_NS_PREFIX_XML_LEN)
        && !cxml_string_lraw_equals(&ns->uri,
                                    _CXML_RESERVED_NS_URI_XML,
                                    _CXML_RESERVED_NS_URI_XML_LEN)
        && !force_set) // should setting be allowed after this violation has occurred?
    {
        return -1;
    }
    cxml_string_free(&ns->prefix);
    cxml_string_append(&ns->prefix, prefix, len);
    return 1;
}


/*********************************
 *                               *
 * selection methods/functions   *
 *********************************
 */

/*
 * Returns the first *element* child of `root` that matches the given query criteria.
 *
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
cxml_element_node* cxml_find(void *root, const char *query){
    if (!query || !_is_valid_root(root)) return NULL;
    _cxml_query  *q_obj = cxq_parse_query(query);
    cxml_elem_node *elem = _cxml__find(q_obj, root);
    cxq_free_query(q_obj);
    return elem;
}


/*
 * Obtains all *element* 'childs' of `root` that matches the given query criteria.
 *
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
void cxml_find_all(void *root, const char *query, cxml_list *acc){
    if (!acc || !query || !_is_valid_root(root)) return;
    _cxml_query  *q_obj = cxq_parse_query(query);
    _cxml__find_all(q_obj, root, acc);
    cxq_free_query(q_obj);
}

/*
 * Obtains all children of the first *element* that matches the given query criteria.
 *
 * `root` is the node from which the search starts.
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
void cxml_find_children(void *root, const char *query, cxml_list *acc){
    if (!acc || !query || !_is_valid_root(root)) return;
    _cxml_query  *q_obj = cxq_parse_query(query);
    cxml_elem_node *elem = _cxml__find(q_obj, root);
    if (elem){
        cxml_list_copy(acc, &elem->children);
    }
}

/*
 * Obtains all children of `node`.
 *
 * `node` can be a cxml_root_node, or a cxml_elem_node object.
 */
void cxml_children(void *node, cxml_list *acc){
    if (!acc || !_is_valid_root(node)) return;
    cxml_list_copy(acc,  _cxml__get_node_children(node));
}

/*
 * Obtains the first child (of any type) of `node`.
 *
 * `node` can be a cxml_root_node, or a cxml_elem_node object.
 */
void *cxml_first_child(void *node){
    if (!_is_valid_root(node)) return NULL;
    return cxml_list_get(_cxml__get_node_children(node), 0);
}

/*
 * Obtains the first child, of the *element* that matches the given query criteria `.
 *
 * `root` is the node from which the search starts.
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
void *cxml_find_first_child(void *root, const char *query){
    return cxml_first_child(cxml_find(root, query));
}

/*
 * Obtains the next element child that is a sibling of `node`.
 * i.e. the element child after `node` which has the same parent as `node`
 */
cxml_elem_node *cxml_next_element(cxml_elem_node *node) {
    if (!node || node->_type != CXML_ELEM_NODE) return NULL;
    return _get_next_node(node, CXML_ELEM_NODE, _cxml__get_node_children(node->parent));
}

/*
 * Obtains the previous element child that is a sibling of `node`.
 * i.e. the element child before `node` which has the same parent as `node`
 */
cxml_elem_node *cxml_previous_element(cxml_elem_node *node) {
    if (!node || node->_type != CXML_ELEM_NODE) return NULL;
    return _get_prev_node(node, CXML_ELEM_NODE, _cxml__get_node_children(node->parent));
}

/*
 * Obtains the next comment child that is a sibling of `node`.
 * i.e. the comment child after `node` which has the same parent as `node`
 */
cxml_comm_node *cxml_next_comment(cxml_comm_node *node) {
    if (!node || node->_type != CXML_COMM_NODE) return NULL;
    return _get_next_node(node, CXML_COMM_NODE, _cxml__get_node_children(node->parent));
}

/*
 * Obtains the previous comment child that is a sibling of `node`.
 * i.e. the comment child before `node` which has the same parent as `node`
 */
cxml_comm_node *cxml_previous_comment(cxml_comm_node *node) {
    if (!node || node->_type != CXML_COMM_NODE) return NULL;
    return _get_prev_node(node, CXML_COMM_NODE, _cxml__get_node_children(node->parent));
}

/*
 * Obtains the next text child that is a sibling of `node`.
 * i.e. the text child after `node` which has the same parent as `node`
 */
cxml_text_node *cxml_next_text(cxml_text_node *node) {
    if (!node || node->_type != CXML_TEXT_NODE) return NULL;
    return _get_next_node(node, CXML_TEXT_NODE, _cxml__get_node_children(node->parent));
}

/*
 * Obtains the previous text child that is a sibling of `node`.
 * i.e. the text child before `node` which has the same parent as `node`
 */
cxml_text_node *cxml_previous_text(cxml_text_node *node) {
    if (!node || node->_type != CXML_TEXT_NODE) return NULL;
    return _get_prev_node(node, CXML_TEXT_NODE, _cxml__get_node_children(node->parent));
}

/*
 * Obtains the parent of `node`.
 *
 * `node` is any valid cxml_*_node
 */
void * cxml_parent(void *node){
    return _cxml_get_node_parent(node);
}

/*
 * Obtains the parent of the *element* that matches the given query criteria.
 *
 * `root` is the node from which the search starts.
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
void *cxml_find_parent(void *root, const char *query){
    return _cxml_get_node_parent(cxml_find(root, query));
}

/*
 * Obtains the parent(s) of `node`, the parent of that parent, and so on.
 */
void cxml_ancestors(void *node, cxml_list *acc){
    if (!node || !acc) return;
    void *par = node;
    do{
        par = _cxml_get_node_parent(par);
        cxml_list_append(acc, par);  // doesn't append NULL
    }while (par && _cxml_get_node_type(par) != CXML_ROOT_NODE);
}

/*
 * Obtains the parent(s) of the element that matches the given query criteria,
 * the parent of that parent, and so on.
 *
 * `root` is the node from which the search starts.
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
void cxml_find_ancestors(void *root, const char *query, cxml_list *acc){
    cxml_ancestors(cxml_find(root, query), acc);
}

/*
 * Obtains the child(ren) of `node`, the child of that child, and so on..
 */
void cxml_descendants(void *node, cxml_list *acc){
    if (!_is_valid_root(node) || !acc) return;
    _descendants(_cxml__get_node_children(node), acc);
}

/*
 * Obtains the child(ren) of the element that matches the given query criteria,
 * the child of that child, and so on.
 *
 * `root` is the node from which the search starts.
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
void cxml_find_descendants(void *root, const char *query, cxml_list *acc){
    if (!acc || !query || !root) return;
    cxml_elem_node *elem = cxml_find(root, query);
    if (elem){
        _descendants(_cxml__get_node_children(elem), acc);
    }
}

/*
 * Obtains the next sibling of `node`.
 * The next sibling is the immediate object after `node`
 * that has the same parent as `node` irrespective of its type.
 */
void *cxml_next_sibling(void *node){
    void *par = _cxml_get_node_parent(node);
    if (!par) return NULL;
    bool at_index = 0;
    cxml_for_each(child, _cxml__get_node_children(par))
    {
        if (child == node){
            at_index = 1;
        }else if (at_index){
            return child;
        }
    }
    return NULL;
}

/*
 * Obtains the next sibling of `node`.
 * The next sibling is the immediate object after `node`
 * that has the same parent as `node` irrespective of its type.
 */
void* (*cxml_next)(void *node)  = cxml_next_sibling;

/*
 * Obtains the previous sibling of `node`.
 * The previous sibling is the immediate object before `node`
 * that has the same parent as `node` irrespective of its type.
 */
void *cxml_previous_sibling(void *node){
    void *par = _cxml_get_node_parent(node);
    if (!par) return NULL;
    void *prev = NULL;
    cxml_for_each(child, _cxml__get_node_children(par))
    {
        if (child == node){
            return prev;
        }
        prev = child;
    }
    return NULL;
}

/*
 * Obtains the previous sibling of `node`.
 * The previous sibling is the immediate object before `node`
 * that has the same parent as `node` irrespective of its type.
 */
void* (*cxml_previous)(void *node) = cxml_previous_sibling;

/*
 * Obtains the next sibling of the element that matches the given query criteria.
 * The next sibling is the immediate object after the matched element,
 * or that has the same parent as the matched element irrespective of its type.
 *
 * `root` is the node from which the search starts.
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
void *cxml_find_next_sibling(void *root, const char *query){
    return cxml_next_sibling(cxml_find(root, query));
}

/*
 * Obtains the previous sibling of the element that matches the given query criteria.
 * The previous sibling is the immediate object before the matched element,
 * or that has the same parent as the matched element irrespective of its type.
 *
 * `root` is the node from which the search starts.
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
void *cxml_find_previous_sibling(void *root, const char *query){
    return cxml_previous_sibling(cxml_find(root, query));
}

/*
 * Obtains the siblings of `node`.
 * The siblings of `node` are cxml objects that share the same parent
 * as `node` irrespective of their types.
 */
void cxml_siblings(void *node, cxml_list *acc){
    if (!acc) return;
    void *par = _cxml_get_node_parent(node);
    if (!par) return;
    cxml_for_each(child, _cxml__get_node_children(par))
    {
        if (child != node){
            cxml_list_append(acc, child);
        }
    }
}

/*
 * Obtains the siblings of the element that matches the given query criteria.
 * The siblings of the matched element are cxml objects that share the same parent
 * as the matched element irrespective of their types.
 *
 * `root` is the node from which the search starts.
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
void cxml_find_siblings(void *root, const char *query, cxml_list *acc){
    cxml_siblings(cxml_find(root, query), acc);
}

/*
 * Obtains a cxml_attr_node from element `node` whose name matches the given name/key.
 */
cxml_attr_node *cxml_get_attribute(cxml_elem_node *node, const char *name_key){
    if (!node || !name_key || !node->has_attribute) return NULL;
    return cxml_table_get(node->attributes, name_key);
}


/*
 * Obtains a cxml_attr_node whose name matches the given key, from an element
 * that matches the given query criteria.
 *
 * `root` is the node from which the search starts.
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
cxml_attr_node *cxml_find_attribute(void *root,
                                    const char *query,
                                    const char *name_key)
{
    return cxml_get_attribute(cxml_find(root, query), name_key);
}

/*
 * Obtains all cxml_attr_nodes in element `node`.
 */
void cxml_attributes(cxml_elem_node *node, cxml_list *acc){
    if (!node || !acc || !node->has_attribute) return;
    cxml_for_each(attr_name, &node->attributes->keys)
    {
        cxml_list_append(acc, cxml_table_get(node->attributes, attr_name));
    }
}

/*
 * alias for cxml_attributes()
 */
void (*cxml_attribute_list)(cxml_elem_node *, cxml_list *) = cxml_attributes;

/*
 * Obtains all cxml_attr_nodes from an element that matches the given query criteria.
 *
 * `root` is the node from which the search starts.
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
void cxml_find_attributes(void *root, const char *query, cxml_list *acc){
    cxml_attributes(cxml_find(root, query), acc);
}

/*
 * alias for cxml_find_attributes()
 */
void (*cxml_find_attribute_list)(void *, const char *, cxml_list *) = cxml_find_attributes;


/*
 * Obtains all text nodes in `root` concatenating them together with concat,
 * and returning the result as an allocated cxml_string* - to be used/freed by the user.
 * If concat is NULL, the default concatenation parameter is nothing.
 *
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
cxml_string *cxml_text_as_cxml_string(void *root, const char *concat){
    if (!root
        || (_cxml_node_type(root) != CXML_ROOT_NODE && _cxml_node_type(root) != CXML_ELEM_NODE)
        ) return NULL;
    cxml_string *text_str = new_alloc_cxml_string();
    _stringify(_cxml__get_node_children(root), concat, (concat ? strlen(concat) : 0), text_str);
    return text_str;
}

/*
 * Obtains all text nodes in `root` concatenating them together with concat,
 * and returning the result as raw char* - to be used/freed by the user.
 * If concat is NULL, the default concatenation parameter is nothing.
 *
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
char *cxml_text(void *root, const char *concat){
    if (!root
        || (_cxml_node_type(root) != CXML_ROOT_NODE && _cxml_node_type(root) != CXML_ELEM_NODE)
        ) return NULL;
    cxml_string text_str = new_cxml_string();
    _stringify(_cxml__get_node_children(root), concat, (concat ? strlen(concat) : 0), &text_str);
    return cxml_string_as_raw(&text_str);
}

/*
 * Obtains the name of `node`
 *
 * CAUTION: returned name must be freed by user
 */
char *cxml_get_name(void *node){
    cxml_string name = new_cxml_string();
    // CAUTION: returned name must be freed by user
    switch(_cxml_get_node_type(node))
    {
        case CXML_ELEM_NODE:
            cxml_string_str_append(&name, &_unwrap__cxnode(elem, node)->name.qname);
            return cxml_string_as_raw(&name);
        case CXML_ATTR_NODE:
            cxml_string_str_append(&name, &_unwrap__cxnode(attr, node)->name.qname);
            return cxml_string_as_raw(&name);
        case CXML_ROOT_NODE:
            cxml_string_str_append(&name, &_unwrap__cxnode(root, node)->name);
            return cxml_string_as_raw(&name);
        default:
            return NULL;
    }
}

/*
 * Obtains the namespace to which `node` is bound.
 *
 * `node` must be an element, or attribute
 *
 * CAUTION: returned namespace should only be freed with cxml_delete_namespace()
 */
cxml_ns_node *cxml_get_bound_namespace(void *node){
    switch(_cxml_get_node_type(node))
    {
        case CXML_ELEM_NODE:
            return _unwrap__cxnode(elem, node)->namespace;
        case CXML_ATTR_NODE:
            return _unwrap__cxnode(attr, node)->namespace;
        default:
            return NULL;
    }
}

/*
 * Obtains the comments in `root`, and stores them in the list `acc`
 *
 * `root` can be a cxml_root_node, or a cxml_elem_node object.
 */
void cxml_get_comments(void *root, cxml_list *acc, bool recursive){
    if (!root
        || !acc
        || (_cxml_node_type(root) != CXML_ROOT_NODE && _cxml_node_type(root) != CXML_ELEM_NODE)
        ) return;
    if (!recursive){
        cxml_for_each(child, _cxml__get_node_children(root))
        {
            if (_cxml_node_type(child) == CXML_COMM_NODE){
                cxml_list_append(acc, child);
            }
        }
    }else{
        _gather_nodes(_cxml__get_node_children(root), CXML_COMM_NODE, acc);
    }
}


/*********************************
 *                               *
 *  creation methods/functions   *
 *********************************
 */

/**General**/
/*
 * Create a cxml node, given the node type
 */
void *cxml_create_node(cxml_node_t type){
#define __create_node(__type)                                   \
{                                                               \
cxml_##__type##_node* node = ALLOC(cxml_##__type##_node, 1);    \
cxml_##__type##_node_init(node);                                \
return node;                                                    \
}
    switch (type)
    {
        case CXML_TEXT_NODE:
            __create_node(text)
        case CXML_ELEM_NODE:
            __create_node(elem)
        case CXML_COMM_NODE:
            __create_node(comm)
        case CXML_ATTR_NODE:
            __create_node(attr)
        case CXML_ROOT_NODE:
            __create_node(root)
        case CXML_PI_NODE:
            __create_node(pi)
        case CXML_NS_NODE:
            __create_node(ns)
        case CXML_XHDR_NODE:
            __create_node(xhdr)
        case CXML_DTD_NODE:
            __create_node(dtd)
        default:
            return NULL;
    }
#undef __create_node
}

extern bool _cxml__is_alpha(char ch);

/*
 * Set/Update the name of a cxml node.
 *
 * `node` can be an element or an attribute
 */
int cxml_set_name(void *node, const char *pname, const char *lname){
    if (!node) return 0;
    cxml_name *name = NULL;
    bool attr = 0;
    cxml_table *attrs = NULL;
    if (_cxml_node_type(node) == CXML_ELEM_NODE){
        name = &_unwrap__cxnode(elem, node)->name;
    }else if (_cxml_node_type(node) == CXML_ATTR_NODE){
        attr = 1;
        name = &_unwrap__cxnode(attr, node)->name;
    }else{
        return 0;
    }

    if (pname)
    {
        // prefix must be a valid identifier
        if (!_cxml__is_alpha(*pname)) return 0;
        int lname_len;
        if (!lname)
        {
            // prefix name cannot exist without a local name
            if (!name->lname || !name->lname_len){
                return 0;
            }else{
                lname = name->lname;
                lname_len = name->lname_len;
            }
        }else{
            lname_len = _cxml_int_cast strlen(lname);
            // local name cannot be empty
            if (!lname_len) return 0;
        }
        int pname_len = _cxml_int_cast strlen(pname);
        // cannot declare a prefix named "xmlns"
        if (cxml_string_llraw_equals(
                (void*)pname, _CXML_RESERVED_NS_PREFIX_XMLNS,
                pname_len, _CXML_RESERVED_NS_PREFIX_XMLNS_LEN))
        {
            return 0;
        }
        // prefix name cannot be empty, and local name must be a valid identifier
        if (!pname_len || !_cxml__is_alpha(*lname))
        {
            return 0;
        }
        // no name must have the prefix 'xmlns'
        if ((_cxml_node_type(node) == CXML_ELEM_NODE)
            && cxml_string_llraw_equals((void *) pname, _CXML_RESERVED_NS_PREFIX_XMLNS,
                                        pname_len, _CXML_RESERVED_NS_PREFIX_XMLNS_LEN))
        {
            return 0;
        }
        cxml_string tmp = new_cxml_string();
        // create qname, update pname, and lname accordingly
        name->pname_len = pname_len;
        cxml_string_append(&tmp, pname, name->pname_len);
        cxml_string_append(&tmp, ":", 1);
        name->lname_len = lname_len;
        cxml_string_append(&tmp, lname, name->lname_len);
        // update the attribute (key-access) in its parent's `attributes` table field, if it has a parent.
        if (attr && _unwrap_cxnode(cxml_attr_node, node)->parent)
        {
            cxml_table_remove(_unwrap__cxnode(attr, node)->parent->attributes,
                              cxml_string_as_raw(&name->qname));
            attrs = _unwrap__cxnode(attr, node)->parent->attributes;
        }
        cxml_string_free(&name->qname);
        name->qname = tmp;
        name->pname = cxml_string_as_raw(&name->qname);
        name->lname = name->pname + name->pname_len + 1;
        // update the attribute in the table if attribute
        attrs ? cxml_table_put(attrs, cxml_string_as_raw(&name->qname), node) : 0;
        return 1;
    }
    if (lname){
        int lname_len = _cxml_int_cast strlen(lname);
        if (!lname_len || !_cxml__is_alpha(*lname)) { return 0; }
        // update the attribute (key-access) in its parent's `attributes` table field, if it has a parent.
        if (attr && _unwrap_cxnode(cxml_attr_node, node)->parent)
        {
            cxml_table_remove(_unwrap__cxnode(attr, node)->parent->attributes,
                              cxml_string_as_raw(&name->qname));
            attrs = _unwrap__cxnode(attr, node)->parent->attributes;
        }
        // zero out lname chars-space in order to set the new lname
        memset(cxml_string_as_raw(&name->qname) +
               (name->pname_len ? name->pname_len + 1 : 0), 0, name->lname_len);
        // remove old lname_len
        name->qname._len -= name->lname_len;
        // update lname_len, and lname
        name->lname_len = lname_len;
        // add lname to qname
        cxml_string_append(&name->qname, lname, name->lname_len);
        name->lname = name->pname_len ?  // ensure we start after the prefix if prefix is available
                      (cxml_string_as_raw(&name->qname) + name->pname_len + 1) : // + 1 for ':'
                      cxml_string_as_raw(&name->qname);
        // update the attribute in the table if attribute
        attrs ? cxml_table_put(attrs, cxml_string_as_raw(&name->qname), node) : 0;
        return 1;
    }
    return 0;
}


/*
 * Add `child` as a child of `parent`
 * `parent` is automatically set as the parent of `child` cxml node.
 * If `child` is an element node, and `parent` is a cxml_root_node object,
 * `child` is automatically set as the root element.
 *
 * `child` is any valid cxml node, and `parent` must be an element, or root node
 */
// for elem, and root
int cxml_add_child(void *parent, void *child){
    if (!parent || !child) return 0;
    return link_child_to_parent(child, parent, NULL);
}

/*
 * Set the namespace of `node` to `ns`. i.e.
 * `node` becomes bound to the namespace `ns`.
 *
 * `node` can be an element node or an attribute node.
 */
int cxml_set_namespace(void *node, cxml_namespace_node *ns){
    if (!node || !ns || ns->_type != CXML_NS_NODE) return 0;
    cxml_name *name;
    switch (_cxml_node_type(node))
    {
        case CXML_ELEM_NODE:
            name = &_unwrap_cxnode(cxml_elem_node, node)->name;
            break;
        case CXML_ATTR_NODE:
            name = &_unwrap_cxnode(cxml_attr_node, node)->name;
            break;
        default: return 0;
    }
    if (ns->is_default){
        // cannot set a default namespace on a node with a prefixed name
        if (name->pname) return 0;
    }else{
        if (!name->pname) return 0;
        // node's prefix name must be same as namespace prefix
        if (!cxml_string_lraw_equals(&ns->prefix, name->pname, name->pname_len)) return 0;
    }
    if (_cxml_node_type(node) == CXML_ELEM_NODE){
        _unwrap_cxnode(cxml_elem_node, node)->namespace = ns;
        _unwrap_cxnode(cxml_elem_node, node)->is_namespaced = 1;
    }else{
        _unwrap_cxnode(cxml_attr_node, node)->namespace = ns;
    }
    return 1;
}

/*
 * Adds the namespace `ns` to the `node`'s `namespaces` list
 */
int cxml_add_namespace(cxml_element_node *node, cxml_namespace_node *ns){
    if (!node || !ns || node->_type != CXML_ELEM_NODE
        || ns->_type != CXML_NS_NODE) return 0;
    cxml_list *namespaces = _unwrap_cxnode(cxml_elem_node, node)->namespaces ?
                            _unwrap_cxnode(cxml_elem_node, node)->namespaces : new_alloc_cxml_list();
    cxml_list_append(namespaces, ns);
    ns->parent = node;
    _unwrap_cxnode(cxml_elem_node, node)->namespaces = namespaces;
    return 1;
}

/*
 * Adds the attribute `attr` to element `elem` table of attributes
 * Does not validate that the attribute is unique using the attribute's expanded name.
 */
int cxml_add_attribute(cxml_element_node *elem, cxml_attribute_node *attr){
    if (!elem || !attr || elem->_type != CXML_ELEM_NODE
        || attr->_type != CXML_ATTR_NODE) return 0;
    int ret = 0;
    if (!elem->attributes){
        elem->attributes = new_alloc_cxml_table();
        cxml_table_put(elem->attributes, cxml_string_as_raw(&attr->name.qname), attr);
        goto assign;
    }
    // try to give the new attribute a position:
    // the problem with this is that, when a union xpath operation (for example: //@* | //*)
    // returns a nodeset containing attributes and other node types, the sorting might be
    // impaired because these newly added attributes might have assumed the position of already existing nodes
    // in the document tree. Of course this doesn't matter unless the user is really interested in
    // the precision of the ordering of the nodes in the nodeset returned.
    // Setting `pos` ensures that attributes are ordered when their parents are printed.
    cxml_attr_node *last = cxml_table_get(elem->attributes, cxml_list_last(&elem->attributes->keys));

    if (!attr->pos) attr->pos = (last->pos + 1);

    if ((ret=cxml_table_put(elem->attributes, cxml_string_as_raw(&attr->name.qname), attr))){
        assign:
        attr->parent = elem;
        elem->has_attribute = 1;
    }
    return ret;
}

/*
 * Sets the value of the attribute `node`
 */
int cxml_set_attribute_value(cxml_attribute_node *node, const char *value){
    if (!node || !value || node->_type != CXML_ATTR_NODE) return 0;
    cxml_string_free(&node->value);
    cxml_string_append(&node->value, value, _cxml_int_cast strlen(value));
    cxml_set_literal(&node->number_value, _get_literal_type(&node->value), &node->value);
    return 1;
}

/*
 * Sets the name/key of the attribute `node`
 * The name/key can be a cxml_string* object (cs_key) or a char* object (rw_key)
 */
int cxml_set_attribute_name(cxml_attribute_node *node,
                            const char *pname,
                            const char *lname)
{
    return cxml_set_name(node, pname, lname);
}

int cxml_set_attribute_data(
        cxml_attribute_node *node,
        const char *pname,
        const char *lname,
        const char *value)
{
    if (!node || !value || node->_type != CXML_ATTR_NODE) return 0;
    if (cxml_set_name(node, pname, lname)){
        cxml_string_free(&node->value);
        cxml_string_append(&node->value, value, _cxml_int_cast strlen(value));
        cxml_set_literal(&node->number_value, _get_literal_type(&node->value), &node->value);
        return 1;
    }
    return 0;
}

/*
 * Sets the text value of a cxml_text_node `text`
 */
int cxml_set_text_value(cxml_text_node *text, const char *value, bool is_cdata){
    if (!text || !value || text->_type != CXML_TEXT_NODE) return 0;
    cxml_string_free(&text->value);
    cxml_string_append(&text->value, value, _cxml_int_cast strlen(value));
    cxml_set_literal(&text->number_value, _get_literal_type(&text->value), &text->value);
    text->is_cdata = is_cdata;
    return 1;
}

/*
 * Sets the text value of a cxml_text_node `text`
 */
int cxml_set_comment_value(cxml_comment_node *comment, const char *value){
    if (!comment || !value || comment->_type != CXML_COMM_NODE) return 0;
    cxml_string_free(&comment->value);
    cxml_string_append(&comment->value, value, _cxml_int_cast strlen(value));
    return 1;
}

/*
 * Sets the target of a processing instruction node
 */
int cxml_set_pi_target(cxml_pi_node *pi, const char *target){
    if (!pi || !target || pi->_type != CXML_PI_NODE) return 0;
    size_t len = strlen(target);
    if (!len) return 0;
    cxml_string_free(&pi->target);
    cxml_string_append(&pi->target, target, len);
    return 1;
}

/*
 * Sets the string value of a processing instruction node
 */
int cxml_set_pi_value(cxml_pi_node *pi, const char *value){
    if (!pi || !value || pi->_type != CXML_PI_NODE) return 0;
    cxml_string_free(&pi->value);
    cxml_string_append(&pi->value, value, _cxml_int_cast strlen(value));
    return 1;
}

/*
 * Sets the target and string value of a processing instruction node
 */
int cxml_set_pi_data(cxml_pi_node *pi, const char *target, const char *value){
    if (!pi || !target || !value || pi->_type != CXML_PI_NODE) return 0;
    cxml_string_free(&pi->target);
    cxml_string_free(&pi->value);
    cxml_string_append(&pi->target, target, _cxml_int_cast strlen(target));
    cxml_string_append(&pi->value, value, _cxml_int_cast strlen(value));
    return 1;
}


/*
 * Sets the prefix value of a namespace node
 */
int cxml_set_namespace_prefix(cxml_namespace_node *ns, const char *prefix){
    if (!ns || !prefix || ns->_type != CXML_NS_NODE) return 0;
    int ret = set_namespace_prefix_helper(ns, prefix, false);
    return  ret && ret != -1;
}

/*
 * Sets the namespace name or URI value of a namespace node
 */
int cxml_set_namespace_uri(cxml_namespace_node *ns, const char *uri){
    if (!ns || !uri || ns->_type != CXML_NS_NODE) return 0;
    int len = _cxml_int_cast strlen(uri);
    if (!len) return 0;
    /*
     * Other prefixes MUST NOT be bound to this namespace name, and it MUST NOT be declared
     * as the default namespace.
     */
    if (cxml_string_llraw_equals((void*)uri, _CXML_RESERVED_NS_URI_XMLNS,
                              len, _CXML_RESERVED_NS_URI_XMLNS_LEN))
    {
        return 0;
    }

    // namespace with the prefix xml, must have a matching uri
    // namespace with a different prefix, must not use the uri corresponding to the xml uri
    if (cxml_string_lraw_equals(&ns->prefix, _CXML_RESERVED_NS_PREFIX_XML,
                                _CXML_RESERVED_NS_PREFIX_XML_LEN) ==
        !cxml_string_llraw_equals((void *) uri, _CXML_RESERVED_NS_URI_XML,
                                  len, _CXML_RESERVED_NS_URI_XML_LEN))
    {
        return 0;
    }else{
        ns->is_global = true;
    }
    // no namespace with a prefix can have an empty URI
    if (cxml_string_len(&ns->prefix) && _cxml_is_empty_URI(uri, len)){
        return 0;
    }
    cxml_string_free(&ns->uri);
    cxml_string_append(&ns->uri, uri, _cxml_int_cast strlen(uri));
    return 1;
}

/*
 * Sets the prefix value and namespace name (or URI value) of a namespace node
 */
int cxml_set_namespace_data(cxml_namespace_node *ns, const char *prefix, const char *uri){
    if (!ns || !prefix || !uri || ns->_type != CXML_NS_NODE) return 0;
    if (!set_namespace_prefix_helper(ns, prefix, true)){
        return 0;
    }
    // if prefix is 'xml', this will be set still, because force_set is set to true.
    // we do not need to worry if the URI matches, because that will be handled
    // in the call to cxml_set_namespace_uri() below:
    return cxml_set_namespace_uri(ns, uri);
}

/*
 * Sets the name of the root node (cxml_root_node object)
 */
int cxml_set_root_node_name(cxml_root_node *root, char *name){
    if (!root || !name || root->_type != CXML_ROOT_NODE) return 0;
    size_t len = strlen(name);
    if (!len) return 0;
    cxml_string_free(&root->name);
    cxml_string_append(&root->name, name, len);
    return 1;
}

/*
 * Sets the root element of cxml_root_node `root`
 */
int cxml_set_root_element(cxml_root_node *root, cxml_element_node *node){
    if (!root || !node || root->root_element
        || root->_type != CXML_ROOT_NODE
        || node->_type != CXML_ELEM_NODE) return 0;
    root->root_element = node;
    root->has_child = true;
    return 1;
}


/*********************************
 *                               *
 * update methods/functions      *
 *********************************
 */
 /*
  * Insert `ins` before `node` irrespective of their types.
  * `node` and `ins` cannot be a cxml_root_node object
  */
int cxml_insert_before(void *node, void *ins){
    if (!node || !ins || _cxml_node_type(ins) == CXML_ROOT_NODE) return 0;
    void *parent = _cxml_get_node_parent(node);
    if (!parent) return 0;
    cxml_list *children = _cxml__get_node_children(parent);
    int index = cxml_list_search(children, cxml_list_cmp_raw_items, node);
    if (index != -1){
        link_child_to_parent(ins, parent, &index);
        return 1;
    }
    return 0;
}

/*
 * Insert `ins` after `node` irrespective of their types.
 * `node` and `ins` cannot be a cxml_root_node object
 */
int cxml_insert_after(void *node, void *ins){
    if (!node || !ins || _cxml_node_type(ins) == CXML_ROOT_NODE) return 0;
    void *parent = _cxml_get_node_parent(node);
    if (!parent) return 0;
    cxml_list *children = _cxml__get_node_children(parent);
    int index = cxml_list_search(children, cxml_list_cmp_raw_items, node);
    if (index != -1){
        index++;
        link_child_to_parent(ins, parent, &index);
        return 1;
    }
    return 0;
}


/*********************************
 *                               *
 * delete methods/functions      *
 *********************************
 */
/*
 * Delete an element node, disassociating it from its parent, and siblings
 */
int cxml_delete_element(cxml_element_node *elem){
    if (!elem || elem->_type != CXML_ELEM_NODE) return 0;
    void *par = elem->parent;
    if (cxml_list_search_delete(_cxml__get_node_children(par),
                            cxml_list_cmp_raw_items, elem))
    {
        cxml_elem_node_free(elem);
        _update_parent(par);
        return 1;
    }
    return 0;
}

/*
 * Drop/remove an element node, disassociating it from its parent, and siblings
 */
int cxml_drop_element(cxml_element_node *elem){
    if (!elem || !elem->parent || elem->_type != CXML_ELEM_NODE) return 0;
    return _drop_cxml_node(elem, _cxml__get_node_children(elem->parent), elem->parent);
}

/*
 * Find the first element node matching the given query, and drop/remove the element, i.e.
 * disassociate the element from its parent, and siblings, without freeing the element.
 * The element returned will no longer be associated with any node in the document.
 */
cxml_element_node* cxml_drop_element_by_query(void *root, const char *query){
    cxml_elem_node *elem = cxml_find(root, query);
    if (!elem || !elem->parent || elem->_type != CXML_ELEM_NODE) return NULL;
    if (cxml_list_search_delete(_cxml__get_node_children(elem->parent),
                                cxml_list_cmp_raw_items, elem))
    {
        _update_parent(elem->parent);
        elem->parent = NULL;
        elem->has_parent = false;
        return elem;
    }
    return NULL;
}

/*
 * Delete all elements that are direct children of `root`,
 * the children of those children, and so forth.
 * `root` is the cxml node from which search begins.
 * `root` can be a cxml_element_node or a cxml_root_node.
 */
int cxml_delete_elements(void *root){
    if (!root) return 0;
    return _delete_cxml_nodes(_cxml__get_node_children(root), CXML_ELEM_NODE);
}

/*
 * Delete all elements satisfying the given query criteria, from the root tree.
 * Recursive by default - means, that it'll delete an element satisfying the query
 * criteria, even if the element isn't a direct child of `root`.
 * `root` is the cxml node from which search begins.
 * `root` can be a cxml_element_node or a cxml_root_node.
 */
int cxml_delete_elements_by_query(void *root, const char *query){
    if (!root || !query) return 0;
    cxml_list all = new_cxml_list();
    cxml_find_all(root, query, &all);
    if (cxml_list_is_empty(&all)) return 0;
    void *par;
    cxml_for_each(elem, &all)
    {
        par = _cxml_node_parent(elem);
        if (par != NULL){
            _delete_cxml_node(elem, _cxml__get_node_children(par), par);
            elem = NULL;
        }
    }
    cxml_list_free(&all);
    return 1;
}

/*
 * Drop/remove all elements that are direct children of `root`.
 * If `recursive` is true, this deletes an element even
 * if the element isn't a direct child of `root`.
 * `root` is the cxml node from which search begins.
 * `root` can be a cxml_element_node or a cxml_root_node.
 */
int cxml_drop_elements(void *root, cxml_list *acc){
    if (!root || !acc) return 0;
    return _drop_cxml_nodes(_cxml__get_node_children(root), CXML_ELEM_NODE, acc);
}

/*
 * Drop/remove all elements satisfying the given query criteria, from the root tree.
 * Recursive by default - means, that it'll drop an element satisfying the query
 * criteria, even if the element isn't a direct child of `root`.
 * `root` is the cxml node from which search begins.
 * `root` can be a cxml_element_node or a cxml_root_node.
 */
int cxml_drop_elements_by_query(void *root, const char *query, cxml_list *acc){
    if (!root || !query || !acc) return 0;
    cxml_list all = new_cxml_list();
    cxml_find_all(root, query, &all);
    if (cxml_list_is_empty(&all)) return 0;
    cxml_for_each(elem, &all)
    {
        // inlining
        if (_unwrap__cxnode(elem, elem)->parent && cxml_list_search_delete(
                _cxml__get_node_children(_unwrap__cxnode(elem, elem)->parent),
                cxml_list_cmp_raw_items, elem))
        {
            _update_parent(_unwrap__cxnode(elem, elem)->parent);
            _unwrap__cxnode(elem, elem)->parent = NULL;
            cxml_list_append(acc, elem);
        }
    }
    cxml_list_free(&all);
    return 1;
}

/*
 * Delete a comment node, disassociating it from its parent, and siblings
 */
int cxml_delete_comment(cxml_comment_node *comm){
    if (!comm || comm->_type != CXML_COMM_NODE) return 0;
    // if comment's parent is NULL, just free comment
    if (!comm->parent){
        cxml_comm_node_free(comm);
        return 1;
    }
    return _delete_cxml_node(comm, _cxml__get_node_children(comm->parent), comm->parent);
}

/*
 * Drop/remove a comment node, disassociating it from its parent, and siblings
 */
int cxml_drop_comment(cxml_comment_node *comm){
    if (!comm || !comm->parent || comm->_type != CXML_COMM_NODE) return 0;
    return _drop_cxml_node(comm, _cxml__get_node_children(comm->parent), comm->parent);
}

/*
 * Delete a text node, disassociating it from its parent, and siblings
 */
int cxml_delete_text(cxml_text_node *text){
    if (!text || text->_type != CXML_TEXT_NODE) return 0;
    // if text's parent is NULL, just free text
    if (!text->parent){
        cxml_text_node_free(text);
        return 1;
    }
    return _delete_cxml_node(text, _cxml__get_node_children(text->parent), text->parent);
}

/*
 * Drop/remove a text node, disassociating it from its parent, and siblings
 */
int cxml_drop_text(cxml_text_node *text){
    if (!text || text->_type != CXML_TEXT_NODE) return 0;
    return _drop_cxml_node(text, _cxml__get_node_children(text->parent), text->parent);
}

/*
 * Delete attribute node, disassociating it from its parent, and siblings
 */
int cxml_delete_attribute(cxml_attribute_node *attr){
    if (!attr || !attr->parent || attr->_type != CXML_ATTR_NODE) return 0;
    int curr_size = cxml_table_size(_unwrap__cxnode(elem, attr->parent)->attributes);
    cxml_table_remove(attr->parent->attributes,
                      cxml_string_as_raw(&attr->name.qname));
    if (curr_size != cxml_table_size(attr->parent->attributes)){
        cxml_attr_node_free(attr);
        return 1;
    }
    return 0;
}

/*
 * Drop/remove an attribute node, disassociating it from its parent, and siblings
 */
int cxml_drop_attribute(cxml_attribute_node *attr){
    if (!attr || !attr->parent || attr->_type != CXML_ATTR_NODE) return 0;
    int curr_size = cxml_table_size(_unwrap__cxnode(elem, attr->parent)->attributes);
    cxml_table_remove(attr->parent->attributes,
                      cxml_string_as_raw(&attr->name.qname));
    return curr_size != cxml_table_size(attr->parent->attributes);
}

/*
 * Eliminate the binding of an element to its namespace, if any.
 */
int cxml_unbind_element(cxml_elem_node *element){
    if (!element
        || _cxml_node_type(element) != CXML_ELEM_NODE
        || !element->is_namespaced
        || !element->namespace
        ) return 0;
    if (element->name.pname
        && cxml_string_raw_equals(&element->namespace->prefix, element->name.pname))
    {
        _remove_ns_prefix(&element->name);
    }
    element->namespace = NULL;
    element->is_namespaced = false;
    return 1;
}

/*
 * Eliminate the binding of an attribute to its namespace, if any.
 */
int cxml_unbind_attribute(cxml_attr_node *attr){
    if (!attr
        || _cxml_node_type(attr) != CXML_ATTR_NODE
        || !attr->namespace
        ) return 0;
    if (attr->name.pname
        && cxml_string_raw_equals(&attr->namespace->prefix, attr->name.pname))
    {
        // update attr in its parent
        cxml_table_remove(attr->parent->attributes, cxml_string_as_raw(&attr->name.qname));
        _remove_ns_prefix(&attr->name);
        cxml_table_put(attr->parent->attributes, cxml_string_as_raw(&attr->name.qname), attr);
    }
    attr->namespace = NULL;
    return 1;
}


/*
 * Delete processing instruction node, disassociating it from its parent, and siblings
 */
int cxml_delete_pi(cxml_pi_node *pi){
    if (!pi || pi->_type != CXML_PI_NODE) return 0;
    // if pi's parent is NULL, just free pi
    if (!pi->parent){
        cxml_pi_node_free(pi);
        return 1;
    }
    return _delete_cxml_node(pi, _cxml__get_node_children(pi->parent), pi->parent);
}

/*
 * Drop/remove processing instruction node, disassociating it from its parent, and siblings
 */
int cxml_drop_pi(cxml_pi_node *pi){
    if (!pi || !pi->parent || pi->_type != CXML_PI_NODE) return 0;
    return _drop_cxml_node(pi, _cxml__get_node_children(pi->parent), pi->parent);
}

/*
 * Delete dtd node, disassociating it from its parent, and siblings
 */
int cxml_delete_dtd(cxml_dtd_node *dtd){
    if (!dtd || dtd->_type != CXML_DTD_NODE) return 0;
    // if dtd's parent is NULL, just free dtd
    if (!dtd->parent){
        cxml_dtd_node_free(dtd);
        return 1;
    }
    return _delete_cxml_node(dtd, _cxml__get_node_children(dtd->parent), dtd->parent);
}

/*
 * Drop/remove dtd node, disassociating it from its parent, and siblings
 */
int cxml_drop_dtd(cxml_dtd_node *dtd){
    if (!dtd || !dtd->parent || dtd->_type != CXML_DTD_NODE) return 0;
    return _drop_cxml_node(dtd, _cxml__get_node_children(dtd->parent), dtd->parent);
}

/*
 * Delete xml header node
 */
int cxml_delete_xml_hdr(cxml_xhdr_node *xhdr){
    if (!xhdr || xhdr->_type != CXML_XHDR_NODE) return 0;
    // if xhdr's parent is NULL, just free xhdr
    if (!xhdr->parent){
        cxml_xhdr_node_free(xhdr);
        return 1;
    }
    return _delete_cxml_node(xhdr, _cxml__get_node_children(xhdr->parent), xhdr->parent);
}

/*
 * Drop/remove xml header node
 */
int cxml_drop_xml_hdr(cxml_xhdr_node *xhdr){
    if (!xhdr || !xhdr->parent || xhdr->_type != CXML_XHDR_NODE) return 0;
    return _drop_cxml_node(xhdr, _cxml__get_node_children(xhdr->parent), xhdr->parent);
}


/*
 * Delete all nodes that is a direct or indirect descendant of the element `node`
 */
int cxml_delete_descendants(cxml_element_node *node){
    if (!node) return 0;
    cxml_for_each(child, &node->children)
    {
        cxml_node_free(child);
    }
    cxml_list_free(&node->children);
    node->has_child = 0;
    node->has_text = 0;
    node->has_comment = 0;
    node->is_self_enclosing = 1;
    return 1;
}

/*
 * Drop/empty all nodes that is a direct or indirect descendant of the element `node`
 */
int cxml_drop_descendants(cxml_element_node *node, cxml_list *acc){
    if (!node || !acc || node->_type != CXML_ELEM_NODE) return 0;
    cxml_for_each(child, &node->children)
    {
        _cxml_unset_parent(child);
    }
    cxml_list_extend(acc, &node->children);
    cxml_list_free(&node->children);
    node->has_child = 0;
    node->has_text = 0;
    node->has_comment = 0;
    node->is_self_enclosing = 1;
    return 1;
}

/* Deletes a cxml node's parent i.e. the parent of `node`
 * Caution should be taken when deleting a cxml node's parent.
 * This is because it has a side effect of disassociating the parent
 * from all of its children, and also, freeing the parent's `children` list
 * containing the siblings of `node`, but NOT the children themselves.
 * The parent is deleted however.
 */
int cxml_delete_parent(void *node){
    void *parent = _cxml_get_node_parent(node);
    if (!parent) return 0;
    cxml_list *children = _cxml__get_node_children(parent);
    cxml_for_each(child, children)
    {
        _cxml_unset_parent(child);
    }
    cxml_list_free(children);
    if (_cxml_node_type(parent) != CXML_ROOT_NODE){
        void *par_par = _cxml_node_parent(parent);
        // if parent's parent is NULL, just free parent
        if (!par_par){
            cxml_node_free(parent);
            return 1;
        }
        // delete the parent node from its own parent node's `children` list
        // and free the parent node
        return _delete_cxml_node(parent, _cxml__get_node_children(_cxml_node_parent(parent)), par_par);
    }else{
        cxml_root_node_free(parent);
        return 1;
    }
}

/*
 * Delete all comment nodes that are direct children of `root`
 * if recursive is true, it deletes all comment nodes whose
 * ancestor is `root` even if the comment node isn't a direct child of `root`.
 *
 * `root` can only be an element, or root node
 */
int cxml_delete_comments(void *root, bool recursive){
    if (!root
        || (_cxml_node_type(root) != CXML_ELEM_NODE && _cxml_node_type(root) != CXML_ROOT_NODE)
        ) return 0;
    if (!recursive){
        return _delete_cxml_nodes(_cxml__get_node_children(root), CXML_COMM_NODE);
    }
    return _delete_cxml_nodes_recursive(_cxml__get_node_children(root), CXML_COMM_NODE);
}

/*
 * Drop/remove all comment nodes that are direct children of `root`
 * if recursive is true, it drops all comment nodes whose
 * ancestor is `root` even if the comment node isn't a direct child of `root`.
 * `root` can only be an element, or root node
 */
int cxml_drop_comments(void *root, bool recursive, cxml_list *acc){
    if (!root
        || !acc
        || (_cxml_node_type(root) != CXML_ELEM_NODE && _cxml_node_type(root) != CXML_ROOT_NODE)
        ) return 0;
    if (!recursive){
        return _drop_cxml_nodes(_cxml__get_node_children(root), CXML_COMM_NODE, acc);
    }
    return _drop_cxml_nodes_recursive(_cxml__get_node_children(root), CXML_COMM_NODE, acc);
}

/*
 * Delete all text nodes that are direct children of `root`
 * if recursive is true, it deletes all text nodes whose
 * ancestor is `root` even if the text node isn't a direct child of `root`.
 * `root` can only be an element, or root node
 */
int cxml_delete_texts(void *root, bool recursive){
    if (!root
        || (_cxml_node_type(root) != CXML_ELEM_NODE && _cxml_node_type(root) != CXML_ROOT_NODE)
        ) return 0;
    if (!recursive){
        return _delete_cxml_nodes(_cxml__get_node_children(root), CXML_TEXT_NODE);
    }
    return _delete_cxml_nodes_recursive(_cxml__get_node_children(root), CXML_TEXT_NODE);
}

/*
 * Drop/remove all text nodes that are direct children of `root`
 * if recursive is true, it drops all text nodes whose
 * ancestor is `root` even if the text node isn't a direct child of `root`.
 * `root` can only be an element, or root node
 */
int cxml_drop_texts(void *root, bool recursive, cxml_list *acc){
    if (!root
        || !acc
        || (_cxml_node_type(root) != CXML_ELEM_NODE && _cxml_node_type(root) != CXML_ROOT_NODE)
        ) return 0;
    if (!recursive){
        return _drop_cxml_nodes(_cxml__get_node_children(root), CXML_TEXT_NODE, acc);
    }
    return _drop_cxml_nodes_recursive(_cxml__get_node_children(root), CXML_TEXT_NODE, acc);
}

/*
 * Delete all processing instruction nodes that are direct children of `root`
 * if recursive is true, it deletes all pi nodes whose
 * ancestor is `root` even if the pi node isn't a direct child of `root`.
 * `root` can only be an element, or root node
 */
int cxml_delete_pis(void *root, bool recursive){
    if (!root
        || (_cxml_node_type(root) != CXML_ELEM_NODE && _cxml_node_type(root) != CXML_ROOT_NODE)
        ) return 0;
    if (!recursive){
        return _delete_cxml_nodes(_cxml__get_node_children(root), CXML_PI_NODE);
    }
    return _delete_cxml_nodes_recursive(_cxml__get_node_children(root), CXML_PI_NODE);
}

/*
 * Drop/remove all processing instruction nodes that are direct children of `root`
 * if recursive is true, it drops all pi nodes whose
 * ancestor is `root` even if the pi node isn't a direct child of `root`.
 * `root` can only be an element, or root node
 */
int cxml_drop_pis(void *root, bool recursive, cxml_list *acc){
    if (!root
        || !acc
        || (_cxml_node_type(root) != CXML_ELEM_NODE
          && _cxml_node_type(root) != CXML_ROOT_NODE)
        ) return 0;
    if (!recursive){
        return _drop_cxml_nodes(_cxml__get_node_children(root), CXML_PI_NODE, acc);
    }
    return _drop_cxml_nodes_recursive(_cxml__get_node_children(root), CXML_PI_NODE, acc);
}

/*
 * Delete all prolog nodes in `root`.
 * The prolog nodes are `cxml_dtd_node` and `cxml_xml_hdr_node` node objects.
 * This function only deletes the nodes linearly, that is, it is not recursive.
 * Hence, if the `cxml_dtd_node` and `cxml_xml_hdr_node` nodes are not direct
 * children of root, then nothing would be deleted.
 * Ideally, `root` should be a cxml_root_node object.
 * However, a cxml_element_node object is also accepted.
 */
int cxml_delete_prolog(void *root){
    if (!root
        || (_cxml_node_type(root) != CXML_ELEM_NODE
           && _cxml_node_type(root) != CXML_ROOT_NODE)
        ) return 0;
    cxml_list tmp = new_cxml_list();
    _cxml_node_t type;
    cxml_list *children = _cxml__get_node_children(root);
    cxml_for_each(node, children)
    {
        type = _cxml_node_type(node);
        if (type == CXML_XHDR_NODE || type == CXML_DTD_NODE)
        {
            cxml_list_append(&tmp, node);
        }
    }
    cxml_for_each(obj, &tmp)
    {
        cxml_list_search_delete(children, cxml_list_cmp_raw_items, obj);
        cxml_node_free(obj);
    }
    _update_parent(root);
    int size = cxml_list_size(&tmp);
    cxml_list_free(&tmp);
    return size;
}

/*
 * Drop/remove all prolog nodes in `root`.
 * The prolog nodes are `cxml_dtd_node` and `cxml_xml_hdr_node` node objects.
 * This function only deletes the nodes linearly, that is, it is not recursive.
 * Hence, if the `cxml_dtd_node` and `cxml_xml_hdr_node` nodes are not direct
 * children of root, then nothing would be deleted.
 * Ideally, `root` should be a cxml_root_node object.
 * However, a cxml_element_node object is also accepted.
 */
int cxml_drop_prolog(void *root, cxml_list *acc){
    if (!root
        || !acc
        || (_cxml_node_type(root) != CXML_ELEM_NODE
            && _cxml_node_type(root) != CXML_ROOT_NODE)
        ) return 0;
    _cxml_node_t type;
    int size = cxml_list_size(acc);
    cxml_list *children = _cxml__get_node_children(root);
    cxml_for_each(node, children)
    {
        type = _cxml_node_type(node);
        if (type == CXML_XHDR_NODE || type == CXML_DTD_NODE)
        {
            cxml_list_append(acc, node);
            _cxml_unset_parent(node);
        }
    }
    cxml_for_each(obj, acc)
    {
        cxml_list_search_delete(children, cxml_list_cmp_raw_items, obj);
    }
    _update_parent(root);
    return size != cxml_list_size(acc);
}

/*
 * Delete the root node (cxml_root_node object)
 */
int cxml_delete_document(cxml_root_node *node){
    if (!node || node->_type != CXML_ROOT_NODE) return 0;
    cxml_root_node_free(node);
    return 1;
}

/*
 * Delete any valid cxml node object
 */
int cxml_delete(void *node){
    if (!node) return 0;
    switch (_cxml_node_type(node))
    {
        case CXML_PI_NODE:      return cxml_delete_pi(node);
        case CXML_DTD_NODE:     return cxml_delete_dtd(node);
        case CXML_COMM_NODE:    return cxml_delete_comment(node);
        case CXML_ELEM_NODE:    return cxml_delete_element(node);
        case CXML_ATTR_NODE:    return cxml_delete_attribute(node);
        case CXML_TEXT_NODE:    return cxml_delete_text(node);
        case CXML_ROOT_NODE:    return cxml_delete_document(node);
        case CXML_XHDR_NODE:    return cxml_delete_xml_hdr(node);
        default:                return 0;
    }
}

/*
 * Delete/free any valid cxml node
 */
void cxml_clear(void *node){
    cxml_node_free(node);
}


/*************************************
 *                                   *
 * general utility methods/functions *
 *************************************
 */

/*
 * Obtain the number value of a cxml_text_node/cxml_attr_node
 */
double cxml_get_number(void *node){
    if (!node
        || (_cxml_node_type(node) != CXML_TEXT_NODE
         && _cxml_node_type(node) != CXML_ATTR_NODE))
        return 0;
    if (_cxml_node_type(node) == CXML_ATTR_NODE){
        return _unwrap__cxnode(attr, node)->number_value.dec_val;
    }else{
        return _unwrap__cxnode(text, node)->number_value.dec_val;
    }
}

/*
 * Check that the document is well-formed
 */
bool cxml_is_well_formed(cxml_root_node *root){
    if (!root || root->_type != CXML_ROOT_NODE) {
        return false;
    }
    return root->is_well_formed;
}

/*
 * Obtain the type of cxml node that `node` is
 */
cxml_node_t cxml_get_node_type(void *node){
    return _cxml_get_node_type(node);
}

/*
 * Obtain the dtd node if present
 */
cxml_dtd_node *cxml_get_dtd_node(cxml_root_node *node){
    if (!node || node->_type != CXML_ROOT_NODE) return NULL;
    return _get_node(&node->children, CXML_DTD_NODE);
}

/*
 * Obtain the xml header prolog if present
 */
cxml_xhdr_node *cxml_get_xml_hdr_node(cxml_root_node *node){
    if (!node || node->_type != CXML_ROOT_NODE) return NULL;
    return _get_node(&node->children, CXML_XHDR_NODE);
}

/*
 * Obtain the root element if present
 */
cxml_elem_node *cxml_get_root_element(cxml_root_node *node){
    if (!node || node->_type != CXML_ROOT_NODE) return NULL;
    return node->root_element;
}
