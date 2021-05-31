/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "xpath/cxxpeval.h"


#define _CXML_MAX_CACHEABLE_SET_SIZE (500000)


/*************************************************************/
extern void query_string(const char *expr);

extern void cxml_set__init_with(cxml_set *recipient, cxml_set *donor);

static bool is_not_prolog_type(_cxml_node_t node_type);

static bool _evaluate_predicate_expr(_cxml_xp_data *res_d);

inline static void
_process_nametest(cxml_elem_node *elem,
                  cxml_xp_nodetest *node_test,
                  cxml_set *acc);

static void
_process_typetest(
        cxml_xp_nodetest* node_test,  /* xpath node_test */
        _cxml_node_t cxml_node_type,   /* cxml node type */
        void* cxml_node,            /* cxml node to be stored*/
        cxml_set* node_set);          /* node-set in which cxml_node would be added */


static void cxml_xp_visit_Predicate(cxml_xp_predicate *node);

/*************************************************************/


inline static cxml_ns_node* _get_xml_namespace(){
    cxml_ns_node *xml_ns = ALLOC(cxml_ns_node, 1);
    cxml_ns_node_init(xml_ns);
    xml_ns->is_global = 1;
    cxml_string_append(&xml_ns->prefix, _CXML_RESERVED_NS_PREFIX_XML,
                       _CXML_RESERVED_NS_PREFIX_XML_LEN);
    cxml_string_append(&xml_ns->uri, _CXML_RESERVED_NS_URI_XML,
                       _CXML_RESERVED_NS_URI_XML_LEN);
    return xml_ns;
}

static bool cmp_expanded_name(cxml_elem_node *elem, cxml_attr_node *attr, cxml_xp_nodetest *node_test){
    cxml_ns_node *check_ns;
    cxml_name *check_name;
    cxml_elem_node *curr;
    if (elem){
        if (!elem->namespace || !elem->name.pname) return false;
        check_ns = elem->namespace;
        check_name = &elem->name;
        curr = elem;
    }
    else{
        if (!attr->namespace || !attr->name.pname) return false;
        check_ns = attr->namespace;
        check_name = &attr->name;
        curr = attr->parent;
    }
    // ensure `namespaces` isn't NULL, if so, ensure we hit the root node
    while (curr->_type != CXML_ROOT_NODE && curr->namespaces == NULL) curr = curr->parent;

    // we need to obtain the namespace bound to the pname/prefix in the nametest.
    // we then obtain the expanded name of the nametest using the namespace found
    // and then compare with the current element's expanded name
    cxml_ns_node *ns = NULL;
    while (curr->_type != CXML_ROOT_NODE)
    {
        cxml_for_each(_ns, curr->namespaces)
        {
            if (!(_unwrap_cxnode(cxml_ns_node, _ns))->is_default
                && cxml_string_lraw_equals(&(_unwrap_cxnode(cxml_ns_node, _ns))->prefix,
                                           node_test->name_test.name.pname,
                                           node_test->name_test.name.pname_len))
            {
                ns = _ns;
                goto cmp;
            }
        }
        curr = curr->parent;
    }

    cmp:
    if (ns == NULL) {
        // check if the namespace referenced was the global namespace 'xml'
        if (memcmp(node_test->name_test.name.pname,
                   _CXML_RESERVED_NS_PREFIX_XML,
                   _CXML_RESERVED_NS_PREFIX_XML_LEN) == 0)
        {
            // check if the namespace is available
            if (!_xpath_parser.xml_namespace)
            {
                _xpath_parser.xml_namespace = _get_xml_namespace();
            }
            ns = _xpath_parser.xml_namespace;
        }else{ // fail if no namespace was found
            int buff_size = node_test->name_test.name.pname_len + 40;
            char buff[buff_size];
            snprintf(buff, buff_size, "Unknown namespace prefix - `%.*s`",
                     node_test->name_test.name.pname_len,
                     node_test->name_test.name.pname);
            _cxml_xp_eval_err(buff);
        }
    }
    // compare with the namespace found
    if (node_test->name_test.t_type == CXML_XP_NAME_TEST_PNAME_LNAME){
        // expanded name -> namespace name (URI) + local name
        // check that URI and local name are equal
        return cxml_string_equals(&ns->uri, &check_ns->uri)
               && cxml_string_llraw_equals(node_test->name_test.name.lname,
                                           check_name->lname,
                                           node_test->name_test.name.lname_len,
                                           check_name->lname_len);
    }else if (node_test->name_test.t_type == CXML_XP_NAME_TEST_PNAME_WILDCARD){
        return ((ns == check_ns) || cxml_string_equals(&ns->uri, &check_ns->uri));
    }
    return false;
}


static void
_get_attrs(cxml_elem_node *node,
           cxml_set *node_set,
           cxml_xp_nodetest *node_test)  // node test
{
    if (node->has_attribute)
    {
        if (node_test == NULL) goto else_; // handles typetest
        cxml_attr_node *attr;
        if (node_test->name_test.t_type == CXML_XP_NAME_TEST_PNAME_WILDCARD)  // @pname: *
        {
            cxml_for_each(key, &node->attributes->keys)
            {
                attr = cxml_table_get(node->attributes, key);
                if (attr->name.pname
                    && cmp_expanded_name(NULL, attr, node_test))
                {
                    cxml_set_add(node_set, attr);
                }
            }
        }
        else{  // * (wildcard)
            else_: ;
            cxml_for_each(key, &node->attributes->keys)
            {
                cxml_set_add(node_set, cxml_table_get(node->attributes, key));
            }
        }
    }
}

// get an attribute of an element if exists.
static void
_get_attr(cxml_elem_node *node,
          cxml_set *node_set,
          cxml_xp_nodetest *node_test)  // node test
{
    if (node->has_attribute)
    {
        cxml_attr_node *attr;
        if (node_test->name_test.t_type == CXML_XP_NAME_TEST_PNAME_LNAME)  // pname:lname
        {
            attr = cxml_table_get(
                    node->attributes,
                    cxml_string_as_raw(&node_test->name_test.name.qname));
            if (attr)
            {
                (attr->name.pname && cmp_expanded_name(NULL, attr, node_test)) ?
                cxml_set_add(node_set, attr) : (void)0;
            } else {
                /* This handles cases where the namespace prefix names don't match but
                 * have exactly matching URis.
                 * For example, consider the xml document:
                 * <foo x:a='1' x:b='2' xmlns:x="some-uri">
                 *     <foo y:a='3' xmlns:y="some-uri"/>
                 * </foo>
                 *  //@x:a must match both x:a='1' and y:a='3' despite both attributes having
                 *  namespaces with unequal prefix names.
                 *  However, the approach used here is quite inefficient.
                 */
                cxml_for_each(key, &node->attributes->keys)
                {
                    attr = cxml_table_get(node->attributes, key);
                    if (cxml_string_llraw_equals(
                            attr->name.lname,
                            node_test->name_test.name.lname,
                            attr->name.lname_len,
                            node_test->name_test.name.lname_len)
                        && cmp_expanded_name(NULL, attr, node_test))
                    {
                        cxml_set_add(node_set, attr);
                        return;
                    }
                }
            }
        }
        else if (node_test->name_test.t_type == CXML_XP_NAME_TEST_WILDCARD_LNAME)   // *:lname
        {
            cxml_for_each(key, &node->attributes->keys)
            {
                attr = cxml_table_get(node->attributes, key);
                if (attr->name.lname
                    && cxml_string_llraw_equals(
                            attr->name.lname,
                            node_test->name_test.name.lname,
                            attr->name.lname_len,
                            node_test->name_test.name.lname_len)
                    )
                {
                    cxml_set_add(node_set, attr);
                }
            }
        }
        else{  // name (unprefixed) -> CXML_XP_NAME_TEST_NAME
            cxml_set_add(node_set, cxml_table_get(
                    node->attributes,
                    cxml_string_as_raw(&node_test->name_test.name.qname)));
        }
    }
}

inline static void
_process_nametest(cxml_elem_node *elem,
                  cxml_xp_nodetest *node_test,
                  cxml_set *acc)
{
    switch(node_test->name_test.t_type)
    {
        case CXML_XP_NAME_TEST_NAME:           // '//nm'
            if (cxml_string_equals(&elem->name.qname,
                                   &node_test->name_test.name.qname))
            {
                cxml_set_add(acc, elem);
            }
            break;
        case CXML_XP_NAME_TEST_PNAME_WILDCARD:  // '//pn:*'
        case CXML_XP_NAME_TEST_PNAME_LNAME:   // '//pn:ln'
            if (cmp_expanded_name(elem, NULL, node_test)){
                cxml_set_add(acc, elem);
            }
            break;
        case CXML_XP_NAME_TEST_WILDCARD:       // '//*'
            cxml_set_add(acc, elem);
            break;
        case CXML_XP_NAME_TEST_WILDCARD_LNAME:  // '//*:ln'
            if (cxml_string_llraw_equals(elem->name.lname,
                                         node_test->name_test.name.lname,
                                         elem->name.lname_len,
                                         node_test->name_test.name.lname_len))
            {
                cxml_set_add(acc, elem);
            }
            break;
        default:
            break;
    }
}

static void
_process_typetest(
        cxml_xp_nodetest* node_test,  /* xpath node_test */
        _cxml_node_t cxml_node_type,   /* cxml node type */
        void* cxml_node,            /* cxml node to be stored*/
        cxml_set* node_set)          /* node-set in which cxml_node would be added */
{
    switch (node_test->type_test.t_type)
    {
        case CXML_XP_TYPE_TEST_NODE: // like a wildcard, captures any node
            is_not_prolog_type(cxml_node_type) ? // do not capture xml header & dtd
            cxml_set_add(node_set, cxml_node) : (void)0;
            break;
        case CXML_XP_TYPE_TEST_TEXT:
            (cxml_node_type == CXML_TEXT_NODE) ?
            cxml_set_add(node_set, cxml_node) : (void)0;
            break;
        case CXML_XP_TYPE_TEST_COMMENT:
            (cxml_node_type == CXML_COMM_NODE) ?
            cxml_set_add(node_set, cxml_node) : (void)0;
            break;
        case CXML_XP_TYPE_TEST_PI:
            if (cxml_node_type == CXML_PI_NODE)
            {
                if (node_test->type_test.has_target) // type-test has target?
                {
                    // check if the proc node has the same target as the type-test target
                    cxml_string_equals(
                            &node_test->type_test.target,
                            &_unwrap_cxpi_node(cxml_node)->target) ?
                    cxml_set_add(node_set, cxml_node) : (void)0;
                }else{
                    cxml_set_add(node_set, cxml_node);
                }
            }
            break;
        default:
            break;
    }
}

inline static void
_save_attr_if_matches(cxml_elem_node *elem,
                      cxml_xp_nodetest *node_test,
                      cxml_set *acc)
{
    // '//@nm | //@pn:ln | //@*:ln | //@pn:* | //@* | //@nt()
    if (node_test->t_type == CXML_XP_NODE_TEST_NAMETEST){
        switch (node_test->name_test.t_type)
        {
            case CXML_XP_NAME_TEST_NAME:
            case CXML_XP_NAME_TEST_PNAME_LNAME:
            case CXML_XP_NAME_TEST_WILDCARD_LNAME:
                _get_attr(elem, acc, node_test);  // handle any test involving lnames
                break;
            case CXML_XP_NAME_TEST_WILDCARD:
            case CXML_XP_NAME_TEST_PNAME_WILDCARD:
                _get_attrs(elem, acc, node_test); // handle any test ending with a wildcard
                break;
            default:
                break;
        }
    }else{
        if (node_test->type_test.t_type == CXML_XP_TYPE_TEST_NODE){
            _get_attrs(elem, acc, NULL);
        }
    }
}

static void
_cxml_xp_transfer_nodeset(cxml_set *old, cxml_set *new){
    cxml_set_free(old);
    cxml_set__init_with(old, new);
}

static void _cxml_xp__e_push(_cxml_xp_data *val){
    // add an element node to the top of the stack
    _cxml_stack__push(&_xpath_parser.acc_stack, val);
}

static _cxml_xp_data * _cxml_xp__e_pop(){
    // pop an element node off the result accumulator stack
    return _cxml_stack__pop(&_xpath_parser.acc_stack);
}

static bool is_not_prolog_type(_cxml_node_t node_type){
    return (node_type != CXML_XHDR_NODE && node_type != CXML_DTD_NODE);
}

_cxml_xp_data *_cxml_xp_new_data() {
    _cxml_xp_data *res = ALLOC(_cxml_xp_data, 1);
    // store for later de-allocation
    cxml_list_append(&_xpath_parser.data_nodes, res);
    _cxml_xp_data_init(&res);
    return res;
}

static void
add_parent(void* _node, cxml_set* acc){
#define _add(_n_set, _val)                         \
{                                                  \
       cxml_set_add(_n_set, _val);                 \
       break;                                      \
}

    switch(_cxml_node_type(_node))
    {
        case CXML_ELEM_NODE:
            _add(acc, _unwrap_cxnode(cxml_elem_node, _node)->parent)
        case CXML_TEXT_NODE:
            _add(acc, _unwrap_cxnode(cxml_text_node, _node)->parent)
        case CXML_COMM_NODE:
            _add(acc, _unwrap_cxnode(cxml_comm_node, _node)->parent)
        case CXML_ATTR_NODE:
            _add(acc, _unwrap_cxnode(cxml_attr_node, _node)->parent)
        case CXML_PI_NODE:
            _add(acc, _unwrap_cxnode(cxml_pi_node, _node)->parent)
        default: break;
    }
#undef _add
}

static void* _get_parent(void* _node){
    if (!_node) return NULL;
    switch(_cxml_node_type(_node))
    {
        case CXML_ELEM_NODE:
            return _unwrap_cxelem_node(_node)->parent;
        case CXML_TEXT_NODE:
            return _unwrap_cxtext_node(_node)->parent;
        case CXML_COMM_NODE:
            return _unwrap_cxcomm_node(_node)->parent;
        case CXML_ATTR_NODE:
            return _unwrap_cxattr_node(_node)->parent;
        case CXML_PI_NODE:
            return _unwrap_cxpi_node(_node)->parent;
        default: return NULL;
    }
}

/*

 * / is short for /child::node()/.

    Use / to select a node's immediate children.

* // is short for /descendant-or-self::node()/.

    Use // to select a node, its children, its grandchildren, and so on recursively.

inspect children, grandchildren, etc.
. -> select current
.. -> select parent
* -> select any element
@ -> select attribute
name -> select element with name
node-type() -> selects node type (node(), comment(), etc.)
 */

static void
_recursive_find_all(
        /* root elem / root node*/
        void* root,
        /*  '//.'  | '//..'  <- 1 (self), 2 (parent), 0 (none) */
        cxml_xp_abbrev_step_t abbrev_step_type,
        /* node test to be performed */
        cxml_xp_nodetest* node_test,
        /* result accumulator */
        cxml_set* acc)
{
    cxml_for_each(child, _cxml__get_node_children(root))
    {
        if (node_test == NULL)  // '//.'  | '//..' -> abbreviated step (no node-test)
        {
            if (abbrev_step_type == CXML_XP_ABBREV_STEP_TSELF){  // '//.'
                // exclude xml header and dtd nodes
                is_not_prolog_type(_cxml_node_type(child)) ? cxml_set_add(acc, child) : (void)0;
            }
            else if (abbrev_step_type == CXML_XP_ABBREV_STEP_TPARENT) // '//..'
            {
                add_parent(child, acc);
            }
        }
        // is it a pure node-test (without attributes)?
        else if (!node_test->has_attr_axis)  // '//nm  | // * | //nt()'
        {
            // name-test
            if ((node_test->t_type == CXML_XP_NODE_TEST_NAMETEST)
                && (_cxml_node_type(child) == CXML_ELEM_NODE))
            {
                _process_nametest(child, node_test, acc);
            }
            // type-test
            else if (node_test->t_type == CXML_XP_NODE_TEST_TYPETEST) // //nt() -> node-type
            {
                _process_typetest(node_test, _cxml_node_type(child), child, acc);
            }
        }
        // '//@nm | //@* | //@nt()'
        else
        {
            if (_cxml_node_type(child) == CXML_ELEM_NODE)
            {
                _save_attr_if_matches(child, node_test, acc);
            }
        }
        if (_cxml_node_type(child) == CXML_ELEM_NODE)
        {
            _recursive_find_all(
                    child,
                    abbrev_step_type,
                    node_test, acc);
        }
    }
}


static void
_find_all(
        cxml_xp_nodetest* node_test,
        void* root,
        cxml_set* acc)
{

    cxml_list* children = _cxml__get_node_children(root);

    // in a wildcard check (name-test name is NULL), all children of root is selected
    // when root is specified (i.e. not NULL), then all matching children of root is selected.
    if (cxml_list_size(children))
    {
        // capture child element nodes with matching names.
        cxml_for_each(node, children)
        {
            if (node_test->t_type == CXML_XP_NODE_TEST_NAMETEST)
            {
                if (_cxml_node_type(node) == CXML_ELEM_NODE){
                    _process_nametest(node, node_test, acc);
                }
            }
            else
            {
                // check and capture nodes with matching type
                _process_typetest(node_test, _cxml_node_type(node), node, acc);
            }
        }
    }
}


//***********************************************//
/*
 * /.
 * /..
 * /@nm | /@* | /@nt()
 * /nm  | / * | /nt()
 *
 *
 * //.
 * //..
 * //@nm | //@* | //@nt()
 * //nm  | // * | //nt()
 *
 * .
 * ..
 * @nm | @* | @nt()
 * nm  | *  | nt()
 */
//***********************************************//

static void _set_state_1(){
    // `/.`
    // '/' refers to root node. '.' selects the context node, hence,
    // we save the root node when the result set is empty
    if (cxml_set_is_empty(&_xpath_parser.nodeset)){
        cxml_set_add(&_xpath_parser.nodeset, _xpath_parser.root_node);
    }
    // if not empty, no need to do perform any operation, as `/.` selects the
    // exact same nodes in the accumulating nodeset (`_xpath_parser.nodeset`)
}

static void _set_state_2(){
    // `/..`  -> captures parent of context node
    if (cxml_set_size(&_xpath_parser.nodeset)){
        cxml_set node_set = new_cxml_set();
        // store parent nodes into node_set,
        cxml_for_each(_node, &_xpath_parser.nodeset.items){
            add_parent(_node, &node_set);
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_3(cxml_xp_step* node){
    /*
     *  `/@nm` -> selects specific attribute in an elem node if present
     *  `/@pn:ln` -> selects specific attribute (under the given namespace)
     *               in an elem node if present
     *  `/@*:ln` -> selects specific attribute in an elem node
     *              (whether or not the attribute has a namespace), if present
     *  when the accumulating nodeset (_xpath_parser.nodeset) is empty,
     *  /@name | /@* selects nothing.
     *  since this would attempt selecting an attribute from the context node (root node)
     */
     // if step node's current state is a "wildcard / name", then the local name should be used
     // when obtaining the attribute. Otherwise, the qualified name should be used, as this
     // matches when the node-test has no prefix name, and also when the node-test is fully prefixed
     // e.g. for a name: foo:bar -> qname == foo:bar
     //      for a name: foobar  -> qname == foobar

    if (cxml_set_size(&_xpath_parser.nodeset)){
        cxml_set node_set = new_cxml_set();
        // iterate over existing nodes (result) and get their attributes
        // only when they are elements
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            if (_cxml_node_type(_node) == CXML_ELEM_NODE){
                _get_attr(_node, &node_set, node->node_test);
            }
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}


static void _set_state_4(){
     /*
      * `/@*` -> selects all attributes in an elem node if it has any.
      * when the accumulating nodeset (_xpath_parser.nodeset)
      * is empty, /@name | /@* selects nothing.
      * since this would attempt selecting an attribute from the current (doc) node
      */
    if (cxml_set_size(&_xpath_parser.nodeset)){
        cxml_set node_set = new_cxml_set();
        // iterate over existing nodes (result) and get their attributes
        // only when they are elements
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            if (_cxml_node_type(_node) == CXML_ELEM_NODE){
                _get_attrs(_node, &node_set, NULL);
            }
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_4b(cxml_xp_step *node){
    /*
     * `/@pn:*` -> selects all attributes in an elem node that has a namespace 'pn', if it has any.
     * when the accumulating nodeset (_xpath_parser.nodeset)
     * is empty, /@name | /@* selects nothing.
     * since this would attempt selecting an attribute from the current (doc) node
     */
    if (cxml_set_size(&_xpath_parser.nodeset)){
        cxml_set node_set = new_cxml_set();
        // iterate over existing nodes (result) and get their attributes
        // only when they are elements
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            if (_cxml_node_type(_node) == CXML_ELEM_NODE){
                _get_attrs(_node, &node_set, node->node_test);
            }
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_5(cxml_xp_step* node){
    // `/@nt()` -> acts like a wildcard
    // (@node()-> selects all attributes in an elem node if it has any)
    if (cxml_set_size(&_xpath_parser.nodeset)) {
        cxml_set node_set = new_cxml_set();
        // the other node types (for node test) are invalid for an '@'
        if (node->node_test->type_test.t_type == CXML_XP_TYPE_TEST_NODE)
        {
            // node() acts like '*' if it immediately follows an '@'
            // when result set is empty, /@name | /@* selects nothing
            cxml_for_each(_node, &_xpath_parser.nodeset.items)
            {
                if (_cxml_node_type(_node) == CXML_ELEM_NODE) {
                    // state 5. -> '/@node()'
                    _get_attrs(_node, &node_set, NULL);
                }
            }
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_6(cxml_xp_step* node){
    /*
     * `/nm`
     * `/pn:ln`
     *  '/'*:ln'
     * when the accumulating nodeset (_xpath_parser.nodeset) is empty,
     * '/' refers to the root node, /nm captures element children of root node,
     * if such node's name matches nm. However, since the context node
     * is the root node, and since the root node has only one element child
     * which is the root element, then it's optimal to search/check directly
     * instead of calling _find_all().
     */
    if (cxml_set_is_empty(&_xpath_parser.nodeset))
    {
        // capture root element if result list is empty
        _process_nametest(
                _xpath_parser.root_element,
                node->node_test,
                &_xpath_parser.nodeset);
    }else{
        cxml_set node_set = new_cxml_set();
        // find & capture matching child element nodes from each context/current node
        // if such node is an element
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            if ((_cxml_node_type(_node) ==  CXML_ELEM_NODE)
                || (_cxml_node_type(_node) == CXML_ROOT_NODE))
            {
                _find_all(node->node_test, _node, &node_set);
            }
        }
        // update result list with current accumulated node results
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_7(cxml_xp_step* step){
     /*
      * `/ *`    -> captures element node children of context node.
      * `/pn:*`  -> captures element node children of context node
      *             under the available namespace prefix.

     * when the accumulating nodeset (_xpath_parser.nodeset) is empty,
     * '/' refers to the root node - the context node
     * in this case. However, since the context node is the root node,
     * and since the root node has only one element child which is the
     * root element, then it's optimal to match (since it's a wildcard search)
     * directly instead of calling _find_all().
     */

    // which is the root element.
    if (cxml_set_is_empty(&_xpath_parser.nodeset)){
        // * acts like a wildcard, capturing the root element at top level
        _process_nametest(_xpath_parser.root_element,
                          step->node_test, &_xpath_parser.nodeset);
    }else{
        cxml_set node_set = new_cxml_set();
        // get element nodes
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            if ((_cxml_node_type(_node) ==  CXML_ELEM_NODE)
                || (_cxml_node_type(_node) == CXML_ROOT_NODE))
            {
                _find_all(step->node_test, _node, &node_set);
            }
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}


static void _set_state_8(cxml_xp_step *node) {
    // `/nt()` -> selects children of the context node matching
    // the particular node-type.
    if (cxml_set_is_empty(&_xpath_parser.nodeset)) {
        // node() acts like a wildcard, capturing the root element at top level
        _find_all(node->node_test, _xpath_parser.root_node, &_xpath_parser.nodeset);
    } else {
        cxml_set node_set = new_cxml_set();
        // get element nodes
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            // find all matching nodes in element's children
            if (_cxml_node_type(_node) == CXML_ELEM_NODE
                || _cxml_node_type(_node) == CXML_ROOT_NODE)
            {
                _find_all(node->node_test, _node, &node_set);
            }
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_9(cxml_xp_step* step){
    // `//.` -> captures all nodes including context node

    // capture all elements, the element's children, grandchildren, etc.
    if (cxml_list_is_empty(&_xpath_parser.nodeset.items)){
        // `.` selects the context node., so we select the context node first (root_node)
        cxml_set_add(&_xpath_parser.nodeset, _xpath_parser.root_node);
        // then we select its descendants
        _recursive_find_all(
                _xpath_parser.root_node,
                CXML_XP_ABBREV_STEP_TSELF,
                step->node_test, &_xpath_parser.nodeset);
    }else{
        cxml_set node_set = new_cxml_set();
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            // select current node if not xml header
            if (is_not_prolog_type(_cxml_node_type(_node))) cxml_set_add(&node_set, _node);

            // select children, grandchildren, etc.. only when an element/doc node
            if ((_cxml_node_type(_node) == CXML_ELEM_NODE)
                || (_cxml_node_type(_node) == CXML_ROOT_NODE))
            {
                _recursive_find_all(
                        _node,
                        CXML_XP_ABBREV_STEP_TSELF,
                        step->node_test, &node_set);
            }
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_10(cxml_xp_step* step){
    /*
     * `//..` -> captures parents of all nodes including context node
     */

    // capture parent of every node, the node's children, grandchildren, etc.
    if (cxml_set_is_empty(&_xpath_parser.nodeset)){
        // the context node (root_node) has no parent,
        // hence no need to bother adding anything for it.
        // Instead, we capture every parent of its descandants.
        _recursive_find_all(
                _xpath_parser.root_node,
                CXML_XP_ABBREV_STEP_TPARENT,
                step->node_test, &_xpath_parser.nodeset);
    }else{
        cxml_set node_set = new_cxml_set();
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            // select current node's parent
            // also select parents of the descendant of current node,
            // only when current node is an element/doc node
            add_parent(_node, &node_set);
            // check if its an elem/root node so that the parents of every of its
            // descendants can be captured as well.
            if ((_cxml_node_type(_node) == CXML_ELEM_NODE)
                || (_cxml_node_type(_node) == CXML_ROOT_NODE))
            {
                _recursive_find_all(
                        _node,
                        CXML_XP_ABBREV_STEP_TPARENT,
                        step->node_test, &node_set);
            }
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_11(cxml_xp_step* node){
     /*
      * capture all attributes matching `nm` of every node (if has any),
      * the node's children, grandchildren, etc.
      * //@nm  -> capture all
      * //@pn:ln -> capture all if under namespace with prefix `pn`
      * //@*:ln -> capture all irrespective of namespace
      *
      */
    if (cxml_set_is_empty(&_xpath_parser.nodeset)){
        _recursive_find_all(
                _xpath_parser.root_node,
                CXML_XP_ABBREV_STEP_TNIL,
                node->node_test, &_xpath_parser.nodeset);
    }else{
        cxml_set node_set = new_cxml_set();
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            /*
             * we can only match/select attributes of elements, and their descendants
             * or the root node and its descendants (specifically the root node's descendants
             * since the root node has no attribute).
             * attributes cannot be set on any other nodes, not even on an attribute node itself,
             * hence unlike _set_state_9() or _set_state_10() where we set/select the context node
             * or the parent of the context node respectively, we cannot select the context node here,
             * since it's an element and not an attribute,
             * instead we can only select the attributes of the context node and its descendants
             * if and only if the context node is an element node or the root node
             */
            if (_cxml_node_type(_node) == CXML_ELEM_NODE){
                _get_attr(_node, &node_set, node->node_test);
                goto find;
            }else if (_cxml_node_type(_node) == CXML_ROOT_NODE){
                goto find;
            }else{
                continue;
            }
            find:
            _recursive_find_all(
                    _node,
                    CXML_XP_ABBREV_STEP_TNIL,
                    node->node_test, &node_set);
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_12(cxml_xp_step* step){
     /*
      * `//@*`    -> capture attribute nodes of (element/root) context node and its descendants
      * `//@pn:*` -> capture attribute nodes of (element/root) context node and its descendants
      *              if such nodes have a matching prefix name
      * capture all attributes of every node (if has any),
      * the node's children, grandchildren, etc.
      */

    if (cxml_set_is_empty(&_xpath_parser.nodeset)){
        _recursive_find_all(
                _xpath_parser.root_node, CXML_XP_ABBREV_STEP_TNIL,
                step->node_test, &_xpath_parser.nodeset);
    }else{
        cxml_set node_set = new_cxml_set();
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            if (_cxml_node_type(_node) == CXML_ELEM_NODE){
                _get_attrs(_node, &node_set, step->node_test);
                goto find;
            }else if (_cxml_node_type(_node) == CXML_ROOT_NODE){
                goto find;
            }else{
                continue;
            }
            find:
            _recursive_find_all(
                    _node, CXML_XP_ABBREV_STEP_TNIL,
                    step->node_test, &node_set);
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_13(cxml_xp_step *node) {
     /*
      * '//@nt()'
      * capture all attributes of every node (if has any, and nt == node()),
      * the node's children, grandchildren, etc.
      */

    if (cxml_set_is_empty(&_xpath_parser.nodeset)) {
        _recursive_find_all(
                _xpath_parser.root_node, CXML_XP_ABBREV_STEP_TNIL,
                node->node_test, &_xpath_parser.nodeset);
    } else {
        cxml_set node_set = new_cxml_set();
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            if (_cxml_node_type(_node) == CXML_ELEM_NODE) {
                // get all attributes of context node only when
                // type-test/ node-type is node()
                if (node->node_test->type_test.t_type == CXML_XP_TYPE_TEST_NODE) {
                    _get_attrs(_node, &node_set, NULL);
                }
                _recursive_find_all(
                        _node, CXML_XP_ABBREV_STEP_TNIL,
                        node->node_test, &node_set);
            }
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_14(cxml_xp_step* node){
    // `//nm` -> selects all the nm descendants of the context node
    if (cxml_set_is_empty(&_xpath_parser.nodeset)) {
        _recursive_find_all(
                _xpath_parser.root_node, CXML_XP_ABBREV_STEP_TNIL,
                node->node_test, &_xpath_parser.nodeset);
    }else{
        cxml_set node_set = new_cxml_set();
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            // selects nm descendants if element node or root node
            if ((_cxml_node_type(_node) == CXML_ELEM_NODE)
                || (_cxml_node_type(_node) == CXML_ROOT_NODE))
            {
                _recursive_find_all(
                        _node, CXML_XP_ABBREV_STEP_TNIL,
                        node->node_test, &node_set);
            }
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_15(cxml_xp_step* node){
    // `//*` -> selects all the element descendants of the context node

    if (cxml_set_is_empty(&_xpath_parser.nodeset)) {
        _recursive_find_all(
                _xpath_parser.root_node, CXML_XP_ABBREV_STEP_TNIL,
                node->node_test, &_xpath_parser.nodeset);
    }else{
        cxml_set node_set = new_cxml_set();
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            // selects element descendants if element node or root node
            if ((_cxml_node_type(_node) == CXML_ELEM_NODE)
                || (_cxml_node_type(_node) == CXML_ROOT_NODE))
            {
                _recursive_find_all(
                        _node, CXML_XP_ABBREV_STEP_TNIL,
                        node->node_test, &node_set);
            }
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_16(cxml_xp_step* node){
     /*
      * `//nt()`
      * selects all children of the context node matching the
      * particular node-type.
      * nt() could be: node(), text(), comment(), or processing-instruction()
      */

    if (cxml_set_is_empty(&_xpath_parser.nodeset)) {
        _recursive_find_all(
                _xpath_parser.root_node, CXML_XP_ABBREV_STEP_TNIL,
                node->node_test, &_xpath_parser.nodeset);
    }else{
        cxml_set node_set = new_cxml_set();
        cxml_for_each(_node, &_xpath_parser.nodeset.items)
        {
            // selects nt() descendants if element node or root node
            if ((_cxml_node_type(_node) == CXML_ELEM_NODE)
                || (_cxml_node_type(_node) == CXML_ROOT_NODE))
            {
                _recursive_find_all(
                        _node, CXML_XP_ABBREV_STEP_TNIL,
                        node->node_test, &node_set);
            }
        }
        _cxml_xp_transfer_nodeset(&_xpath_parser.nodeset, &node_set);
    }
}

static void _set_state_17(){
    /*
     * `.` -> selects the context node
     * when the accumulating nodeset (_xpath_parser.nodeset) is empty,
     * the context node is the root_node, hence,
     * we save the root node when the result set is empty
     */
    if (cxml_set_is_empty(&_xpath_parser.nodeset)){
        cxml_set_add(&_xpath_parser.nodeset, _xpath_parser.root_node);
    }
    /*
     * In a regular step expression (without predicates) if not empty,
      * then this state would never be matched, as the non-empty
      * state is captured by other state functions, specifically - _set_state_1()

      * However, in step expressions involving predicates,
      * if the nodeset is not empty, this state could be entered/matched.
      * And since '.' selects the context node, this would only just
      * select all the nodes in the current nodeset, hence so effects whatsoever,
      * Making the current state exactly satisfiable by _set_state_1()
     */
}

static void _set_state_18(){
    /*
     * `..` ->  selects the parent of the context node
    * when the accumulating nodeset (_xpath_parser.nodeset) is empty,
    * the context node is the root_node which has no parent,
    * In a regular step expression (without predicates), when the nodeset
    * is not empty, this state will never be matched, since the
    * path would have to be expressed as `path_expr/..`
    * (here, `path_expr` is any valid xpath sub expression)
    * `..` will never be captured since '/..' is captured in _set_state_2()

    * *//* do nothing *//*

    * However, when the nodeset is not empty, this state would only ever be entered/matched
    * via a predicate filter.
    * In a predicate filter, this would lead to the exact state satisfied by _set_state_2()
    * i.e. `/..`
     */
    _set_state_2();
}

static void _set_state_19(cxml_xp_step* node){
    /*
     * `@nm` -> selects the nm attribute of the context node.
     * when the accumulating nodeset (_xpath_parser.nodeset) is empty,
     * the context node is the root_node which has no attribute,
     * however, when result is not empty, there are 2 possible cases:
     *      (1.)-when the step expression is regular (without predicates),
     *      (2.)-when the step expression isn't regular (with predicates).
     * In (1.) above:
     * this state will never be matched, since the
     * path would have to be expressed as `path_expr/@nm`
     * (here, `path_expr` is any valid xpath sub expression)
     * `@nm` will never be captured since '/@nm' is captured in _set_state_3()

     * *//* do nothing *//*

     * However, in (2.) above:
     * this state could be matched/entered due to predicate filters. When this happens
     * this leads to the exact state satisfied by _set_state_3() (`/@nm`)
     */
    _set_state_3(node);
}

static void _set_state_20(){
     /*
      * `@*` ->  selects all the attributes of the context node
      * `@pn:*` ->  selects all the attributes of the context node under
      *             the given prefixed namespace
     * when the accumulating nodeset (_xpath_parser.nodeset) is empty,
     * the context node is the root_node which has no attribute,
     * however, when result is not empty, there are 2 possible cases:
     *      (1.)-when the step expression is regular (without predicates),
     *      (2.)-when the step expression isn't regular (with predicates).
     * In (1.) above:
     * this state will never be matched, since the
     * path would have to be expressed as `path_expr/@nm`
     * (here, `path_expr` is any valid xpath sub expression)
     * `@*` will never be captured since '/@*' is captured in _set_state_4()

     * However, in (2.) above:
     * this state could be matched/entered due to predicate filters. When this happens
     * this leads to the exact state satisfied by _set_state_4() (`/@*`)
     */
    _set_state_4();
}

static void _set_state_21(cxml_xp_step* node){
     /*
      * `@nt()` -> invalid if no result node-set is empty, but valid when not.
      * when result node-set isn't empty,
      * however, when the accumulating nodeset (_xpath_parser.nodeset)
      * is not empty, there are 2 possible cases:
      *      (1.)-when the step expression is regular (without predicates),
      *      (2.)-when the step expression isn't regular (with predicates).
      * In case (1.), this state would never be matched, as the non-empty
      * state is captured in _set_state_5().

      * *//* do nothing *//*

      * However, in case (2.), this could be matched due to predicate filters.
      * When this happens this leads to the exact state satisfied
      * by _set_state_5() (`/@nt()`)
     */
    _set_state_5(node);
}

static void _set_state_22(cxml_xp_step* node){
     /*
      * `nm` -> selects the nm element children of the context node
      * 'pn:ln' ->
      * '*:ln' ->
      * when the accumulating nodeset (_xpath_parser.nodeset) is empty,
      * the context node is the root node, and since the
      * root node has only one element child which is the root element, then
      * it's optimal to check directly instead of calling _find_all().

      * however, when the accumulating nodeset (_xpath_parser.nodeset)
      * is not empty, there are 2 possible cases:
      *      (1.)-when the step expression is regular (without predicates),
      *      (2.)-when the step expression isn't regular (with predicates).
      * In case (1.), this state would never be matched. as the non-empty
      * state is captured in _set_state_6().

      * *//* do nothing *//*

      * However, in case (2.), this could be matched due to predicate filters.
      * When this happens this leads to the exact state satisfied by
      * _set_state_6() (`/nm`)
     */
    _set_state_6(node);
}

static void _set_state_23(cxml_xp_step* step){
      /*
       * `*` -> selects all element children of the context node
       * `pn:*` -> selects all element children of the context node under
       *            the given namesapce
       * when the accumulating nodeset (_xpath_parser.nodeset) is empty, the
       * context node is the root node, * selects the element child of the
       * root node, which is the root element, as explained in _set_state_7()

       * however, when the accumulating nodeset (_xpath_parser.nodeset)
       * is not empty, there are 2 possible cases:
       *      (1.)-when the step expression is regular (without predicates),
       *      (2.)-when the step expression isn't regular (with predicates).
       * In case (1.), this state would never be matched, as the non-empty
       * state is captured in _set_state_7().
       *
       * However, in case (2.), this could be matched due to predicate filters.
       * for example //'*[/'*][*]
       * When this happens this leads to the exact state satisfied by
     */
    // _set_state_7() (`/*`)
    _set_state_7(step);
}

static void _set_state_24(cxml_xp_step* node){
     /*
      * `nt()` ->  selects children of the context node matching
      * the particular node-type.
      * when the accumulating nodeset (_xpath_parser.nodeset) is empty, the
      * selection starts from the root node

      * however, when the accumulating nodeset (_xpath_parser.nodeset)
      * is not empty, there are 2 possible cases:
      *      (1.)-when the step expression is regular (without predicates),
      *      (2.)-when the step expression isn't regular (with predicates).
      * In case (1.), this state would never be matched, as the non-empty
      * state is captured in _set_state_8().
      * *//* do nothing *//*
      *
      * However, in case (2.), this could be matched due to predicate filters.
      * When this happens this leads to the exact state satisfied by
      * _set_state_8() (`/ *`)
     */
    _set_state_8(node);
}


static void cxml_xp_visit_Step(cxml_xp_step* node){
    if (node->abbrev_step == 1){  // '.' -> state_1
        switch(node->path_spec)
        {
            case 1:  _set_state_1(); break; // '/.'
            case 2:  _set_state_9(node); break;  // '//.'
            default: _set_state_17(); break;   // '.'
        }
    }
    else if (node->abbrev_step == 2) // '..' -> state_2
    {
        switch(node->path_spec)
        {
            case 1:  _set_state_2(); break;  // '/..'
            case 2:  _set_state_10(node); break;  // '//..'
            default: _set_state_18(); break;  // '..'
        }
    }
    else if (node->has_attr_axis)  // '@'
    {
        if (node->node_test->t_type == CXML_XP_NODE_TEST_NAMETEST){
            if (node->node_test->name_test.t_type == CXML_XP_NAME_TEST_WILDCARD){ // '@*'
                switch(node->path_spec)
                {
                    case 1:  _set_state_4(); break; // '/@*'
                    case 2:  _set_state_12(node); break;  // '//@*'
                    default: _set_state_20(); break;  // '@*'
                }
            }
            else if (node->node_test->name_test.t_type == CXML_XP_NAME_TEST_PNAME_WILDCARD){
                switch(node->path_spec)
                {
                    case 1:  _set_state_4b(node); break; // '/@pn:*'
                    case 2:  _set_state_12(node); break;  // '//@pn:*'
                    default: _set_state_20(); break;  // '@pn:*'
                }
            }
            else{  // '@nm'
                switch(node->path_spec)
                {
                    case 1:  _set_state_3(node); break;  // '/@nm'  | '/@pn:ln'  | '/*:ln'
                    case 2:  _set_state_11(node); break;  // '//@nm | '//@pn:ln' | '//*:ln'
                    default: _set_state_19(node); break;  // '@nm'  | '@pn:ln'   | '*:ln'
                }
            }
        }
        else{      // '@nt()'
            switch(node->path_spec)
            {
                case 1:  _set_state_5(node); break;  // '/@nt()'
                case 2:  _set_state_13(node); break;  // '//@nt()'
                default: _set_state_21(node); break;  // '@nt()'
            }
        }
    }
    else{  // nm  |  * | nt()
        if (node->node_test->t_type == CXML_XP_NODE_TEST_NAMETEST)
        {
            if ((node->node_test->name_test.t_type == CXML_XP_NAME_TEST_WILDCARD)
                || (node->node_test->name_test.t_type == CXML_XP_NAME_TEST_PNAME_WILDCARD))  // '*' | pname:*
            {
                switch(node->path_spec)
                {
                    case 1:  _set_state_7(node); break;  // '/*'   | '/pn:*'
                    case 2:  _set_state_15(node); break;  // '//*' | '//pn:*'
                    default: _set_state_23(node); break; // '*'    | 'pn:*'
                }
            }
            else{  // 'nm'
                switch(node->path_spec)
                {
                    case 1:  _set_state_6(node); break;  // '/nm'   | '/pn:ln'  | '/*:ln'
                    case 2:   _set_state_14(node); break; // '//nm' | '//pn:ln' | '//*:ln'
                    default:  _set_state_22(node); break; // 'nm'   | 'pn:ln'   | '*:ln'
                }
            }
        }
        else{  // `nt()`
            switch(node->path_spec)
            {
                case 1:  _set_state_8(node); break;  // '/nt()'
                case 2:  _set_state_16(node); break; // '//nt()'
                default: _set_state_24(node); break; // 'nt()'
            }
        }
    }
    // track the resulting node-set by storing it's state into is_empty_nodeset
    // for further processing in visit_Path()
     _xpath_parser.is_empty_nodeset = cxml_set_is_empty(&_xpath_parser.nodeset);

     /*
      * only push to the stack when the step node has predicates and when the nodeset produced isn't empty.
      * this is a simple optimization. Since the data pushed to the stack in this function
      * would only be used "immediately" by predicates (in cxml_xp_visit_Predicate()),
      * but if there are no predicates, the data is only useful after the entire path node
      * has been completely evaluated.
      * Thus, it is optimal to not push to the result stack when evaluating a step
      * that has no predicate node. We instead, delay the push, to be performed in cxml_xp_visit_Path()
      */
    if (cxml_list_size(&node->predicates))  // has predicates?
    {
        // do not proceed to cxml_xp_visit_Predicate() when the nodeset produced is empty,
        // as the outcome/result would always be an empty nodeset.
        if (_xpath_parser.is_empty_nodeset){
            _cxml_xp_data* data = _cxml_xp_new_data();
            data->type = CXML_XP_DATA_NODESET;
            cxml_set_init(&data->nodeset);
            _cxml_xp__e_push(data);
            return;
        }
        // create a new data object
        _cxml_xp_data* data = _cxml_xp_new_data();
        data->type = CXML_XP_DATA_NODESET;
        cxml_set_copy(&data->nodeset, &_xpath_parser.nodeset);

        // push the data of the evaluated step node to the stack
        _cxml_xp__e_push(data);

         /*
          * if there are predicates, empty the accumulating _xpath_parser.nodeset,
          * to prevent result collisions when it is used in processing potential step axes
          * in the predicate node(s).

          * We do not do this when the step node has no predicate(s) to prevent the accumulating node-set
          * from being reset, (since the nodes in the accumulating node-set would be used in
          * processing following steps - if the result produced by a previous step is a non-empty node-set, that is),
          * thereby compromising the _set_state_x() functions' logic.
          * Here, _xpath_parser.nodeset is the accumulating node-set, we still need to clean it up though,
          * after path node has been completely evaluated in cxml_xp_visit_Path(), to prevent collision
          * when a new path needs to be evaluated, since it's always used in evaluating path nodes.
          */
         cxml_set_free(&_xpath_parser.nodeset);
        _cxml_xp_ret_t o_flag;
         cxml_for_each(pred, &node->predicates)
         {
             o_flag = 1;
             cxml_xp_ovisit(((cxml_xp_predicate*)pred)->expr_node, &o_flag);
             // if the optimization flag is poison, or a number,
             // this indicates that the expression isn't (truthy/Falsy) optimizable.
             if (o_flag == _CXML_XP_PS_POISON || o_flag == CXML_XP_RET_NUMBER){
                 cxml_xp_visit_Predicate(pred);
             }else{
                 _cxml_xp__e_pop();
                 cxml_xp_visit(((cxml_xp_predicate*)pred)->expr_node);
                 bool always_true = _evaluate_predicate_expr(_cxml_xp__e_pop());
                 if (always_true){
                     /*
                      * always_true means that the result of the evaluated predicate expression
                      * is always true irrespective of the context node.
                      * This implies that when each of the node in the nodeset (of the) data
                      * popped from the stack is evaluated, they'll all pass the predicate
                      * test, and hence will be filtered into the final result.
                      * Hence, we just copy the nodeset into the accumulating nodeset,
                      * (should in case there's more evaluation to be done after the predicate
                      * has been evaluated), and push the original nodeset data on the stack.
                      */
                     cxml_set_copy(&_xpath_parser.nodeset, &data->nodeset);
                 }else{
                     /*
                      * the reverse is the case if the condition doesn't hold true.
                      * However, this means that the result of the evaluated predicate expression
                      * is always false irrespective of the context node.
                      * This implies that the predicate expression would produce an empty
                      * nodeset since the expression is always false, thus, no node from the
                      * nodeset `data` (selected above) evaluated against the predicate
                      * expression would be selected; no node would be filtered into the final result,
                      * since they'll all fail the predicate test (it's always false).
                      */
                     cxml_set_free(&data->nodeset);
                     // the accumulating nodeset must reflect the result of the evaluated predicate node
                     cxml_set_free(&_xpath_parser.nodeset);
                 }
                 _cxml_xp__e_push(data);
                 _xpath_parser.is_empty_nodeset = cxml_set_is_empty(&_xpath_parser.nodeset);
             }
        }
    }
}

// Step | '/' Step | '//' Step
void cxml_xp_visit_Path(cxml_xp_astnode *node){
    cxml_xp_path* path = node->wrapped_node.path;
    bool should_cache = false;
    cxml_xp_step *step_node = cxml_list_get(&path->steps, 0);
    if (path->from_predicate)
    {
        if (step_node->path_spec == 1 || step_node->path_spec == 2)
        {
            // check if the result of the expression exists in the cache
            cxml_list *cached_nodeset = _cxml_cache_get(
                    &_xpath_parser.lru_cache, node);

            // if it exists, re-use the result, push to the stack, and
            // free the accumulating nodeset
            if (cached_nodeset){
                _cxml_xp_data* data = _cxml_xp_new_data();
                data->type = CXML_XP_DATA_NODESET;
                cxml_set_init(&data->nodeset);
                cxml_set_extend_list(&data->nodeset, cached_nodeset);
                _cxml_xp__e_push(data);
                cxml_set_free(&_xpath_parser.nodeset);
                return;
            }
             /*
              * we only cache the results of path expressions that begins
              * with a "/" or "//" because the results would always be the same
              * regardless of the context node (independent of the context node),
              * unlike xpath expressions beginning with a "." or a "name" which
              * produces different results depending on the context node
              */
            should_cache = true;
            /*
             * When evaluating the predicate expr field of a predicate node:
                - Absolute paths should start empty, ('/')
                - Descendant-or-self paths should start empty ('//')
               i.e. the context node shouldn't be
               added to the accumulating nodeset, this ensures that the evaluation
               of the step nodes begins from the root node (where appropriate).
            */
            cxml_set_free(&_xpath_parser.nodeset);
        }
    }
    /*
     * if the accumulating nodeset is empty and a context node exists,
     * chances are that a path node was previously evaluated which emptied
     * the accumulating nodeset (since the accumulating nodeset is always emptied
     * after a path node is completely evaluated).
     * So we add the context node to the accumulating nodeset so that evaluation begins
     * from the context node, this prevents us from obtaining false results when
     * each step node contained in this current path node is being evaluated.
     */
    if (step_node->path_spec == 0  // '.' | `name`
        && cxml_set_is_empty(&_xpath_parser.nodeset)
        && _xpath_parser.context.ctx_node)
    {
        cxml_set_add(&_xpath_parser.nodeset, _xpath_parser.context.ctx_node);
    }
    bool has_no_predicate = 0;
    cxml_for_each(step, &path->steps)
    {
        cxml_xp_visit_Step(step);
        has_no_predicate = cxml_list_is_empty(&((cxml_xp_step* )step)->predicates);
        // stop evaluating if the result of evaluating a step is an empty node-set
        if (_xpath_parser.is_empty_nodeset){
            break;
        }
    }
    /*
     * no predicate? push accumulating nodeset on stack
     * since the accumulating node-set would only be pushed on stack in
     * cxml_xp_visit_Step() when the step node has predicate(s) as explained in the comments
     * in cxml_xp_visit_Step(), which means that if the final step
     * has no predicate, the result obtained from its evaluation will never be pushed to the stack,
     * so we have to do it explicitly here.
     * Note that a step node can evaluate to an empty result, which is still a valid result.
     */
    if (has_no_predicate){
        _cxml_xp_data* data = _cxml_xp_new_data();
        data->type = CXML_XP_DATA_NODESET;
        if (_xpath_parser.is_empty_nodeset){
            cxml_set_init(&data->nodeset);
        }else{
            cxml_set_copy(&data->nodeset, &_xpath_parser.nodeset);
        }
        _cxml_xp__e_push(data);
    }
    if (should_cache && (cxml_set_size(&_xpath_parser.nodeset) < _CXML_MAX_CACHEABLE_SET_SIZE))
    {
        // cache the result
        // copy `_xpath_parser.nodeset`, and cache result
        cxml_list * cached_nodeset = new_alloc_cxml_list();
        // store the nodeset in alloc_set_list for tracking, and eventually freeing.
        cxml_list_append(&_xpath_parser.alloc_set_list, cached_nodeset);
        // copy the accumulating nodeset into the cache
        cxml_list_copy(cached_nodeset, &_xpath_parser.nodeset.items);
        // store the nodeset in the cache, but check if an item was popped off the cache.
        void* removed = _cxml_cache_put(&_xpath_parser.lru_cache, node, cached_nodeset);
        if (removed){
            // all allocated lists will be freed on destruction,
            // for now, we just free this current list's contents
            cxml_list_free(removed);
        }
    }
     /*
      * we need to clear the accumulating node-set (the _xpath_parser.nodeset)
      * after the path node has been completely evaluated, to prevent result collision
      * when a new path node needs to be evaluated, since it's a global object always
      * used in evaluating path nodes.
      */
    cxml_set_free(&_xpath_parser.nodeset);
}

bool _evaluate_predicate_expr(_cxml_xp_data *res_d) {
     /*
      * A PredicateExpr is evaluated by evaluating the Expr and converting the result to a boolean.
      * If the result is a number, the result will be converted to true if the number is equal to
      * the context position and will be converted to false otherwise;
      * if the result is not a number, then the result will be converted as if by a call
      * to the boolean function.
      */

    bool ret = 0;
    if (res_d->type == CXML_XP_DATA_NUMERIC)
    {
        if (res_d->number.type == CXML_NUMERIC_DOUBLE_T)
        {
            return cxml_number_is_d_equal(
                    (double)_xpath_parser.context.ctx_pos,
                    res_d->number.dec_val);
        }
    }
    else if (res_d->type == CXML_XP_DATA_BOOLEAN){
        return res_d->boolean;
    }
    else{
        _cxml_xp_data_to_boolean(res_d, &ret);
    }
    return ret;
}

static void _partition_nodeset(cxml_list* nodelist, cxml_list* gather){
    /*
     * group nodes into lists where each list contains only nodes
     * having the same parent - in document order.
     * This helps partition nodesets, where nodes in each partition
     * has 2 properties: size & position.
     */
    cxml_grp_table table = new_cxml_grp_table();
    struct _cx_obj_node obj;
    cxml_for_each(node, nodelist)
    {
        if (_cxml_node_type(node) != CXML_ROOT_NODE){
            cxml_grp_table_put(&table, _get_parent(node), node);
        }else{
            cxml_grp_table_put(&table, &obj, node);
        }
    }
    cxml_list *parts;
    cxml_for_each(key, &table.keys)
    {
        parts = new_alloc_cxml_list();
        cxml_grp_table_get(&table, key, parts);
        cxml_list_append(gather, parts);
    }
    cxml_grp_table_free(&table);
}


static void _free_partitioned_nodeset(cxml_list* list){
    cxml_for_each(nodeset_list, list)
    {
        cxml_list_free(nodeset_list);
        FREE(nodeset_list);
    }
    cxml_list_free(list);
}


static void _sort_nodeset_by_pos(cxml_list *nodes){
#define __sort()                                        \
cxml_for_each(node, nodes){                             \
    arr[i++] = node;                                    \
}                                                       \
qsort(arr, len, sizeof(void *), _cxml_cmp_node);        \
cxml_list_free(nodes);                                  \
for (int j = 0; j < i; j++){                            \
    cxml_list_append(nodes, arr[j]);                    \
}

    int len = cxml_list_size(nodes);
    if (len <= 1) return;
    int i = 0;
    if (len > _CXML_MAX_STACK_ALLOCATABLE_SIZE){
        void **arr = CALLOC(void*, len);
        __sort()
        FREE(arr);
    }else{
        void *arr[len];
        __sort()
    }
#undef __sort
}

void cxml_xp_visit_Predicate(cxml_xp_predicate* node){
    /*
     A PredicateExpr is evaluated by evaluating the Expr and converting the result to a boolean.
     If the result is a number, the result will be converted to true if the number is equal to
     the context position and will be converted to false otherwise;
     if the result is not a number, then the result will be converted as if by a call
     to the boolean function.

     * A predicate filters a node-set with respect to an axis to produce a new node-set.
     * For each node in the node-set to be filtered, the PredicateExpr is evaluated with that
     * node as the *context node*, with the number of nodes in the node-set as the *context size*,
     * and with the proximity position of the node in the node-set (in document order) with respect to the axis as
     * the *context position*; if PredicateExpr evaluates to true for that node, the node is included
     * in the new node-set; otherwise, it is not included
     */
    _cxml_xp_data *data = _cxml_xp__e_pop();  // nodeset

     /*
      * get result of PredicateExpr evaluation
      * compute (filter) result and push back to stack.

      * separate the nodeset's items into clusters/strata such that
      * each cluster/stratum only contains items with the same parents, and
      * each cluster becomes a context: i.e. has its own context size,
      * items contained within the cluster have their own context position,
      * and each of those items becomes the context node.
      */
    cxml_list partitions = new_cxml_list(),  // store list partitions
            filtered = new_cxml_list();   // store filtered node

    _partition_nodeset(&data->nodeset.items, &partitions);
    // clear data object for re-use
    _cxml_xp_data_clear(data);
    data->type = CXML_XP_DATA_NODESET;

    int ctx_size, ctx_pos;
    struct _cxml_xp_context_state ctx;
    cxml_for_each(partition, &partitions)
    {
        ctx_size = cxml_list_size(partition);
        ctx_pos = 0;
        cxml_for_each(ctx_node, (cxml_list*)partition)
        {
            ctx_pos++;
            cxml_set_add(&_xpath_parser.nodeset, ctx_node);

            _cxml_xp_push_context(&_xpath_parser.ctx_stack,
                    &_xpath_parser.context,
                    &ctx, ctx_node, ctx_pos, ctx_size);  // push new context state

            cxml_xp_visit(node->expr_node);
            // evaluate res_d against each node (as context node)
            // if the context node passes the predicate expression test, then
            // add it to the list of successfully filtered nodes.
            if (_evaluate_predicate_expr(_cxml_xp__e_pop())){
                cxml_list_append(&filtered, ctx_node);
            }
            _cxml_xp_pop_context(&_xpath_parser.ctx_stack,
                    &_xpath_parser.context);  // reset context to initial state
            cxml_set_free(&_xpath_parser.nodeset);
        }
    }
    /*
     * push the evaluated predicate node to stack, update the
     * is_empty_nodeset flag (used by the Path node in determining when
     * to stop visiting a failed step node - a step node producing an
     * empty nodeset), and also update the accumulating nodeset.
     */

    // sort the evaluated nodes in document order
    _sort_nodeset_by_pos(&filtered);
    // free partitioned nodesets
    _free_partitioned_nodeset(&partitions);
    // transfer the result set to the accumulating nodeset and
    // into the data nodeset to be pushed on the stack
    cxml_for_each(_node, &filtered){
        cxml_set_add(&data->nodeset, _node);
        cxml_set_add(&_xpath_parser.nodeset, _node);
    }
    // push final evaluated result on the stack
    _cxml_xp__e_push(data);

    cxml_list_free(&filtered);

    // update is_empty_nodeset flag to be used in visit_Path() after the
    // Predicate node has been completely evaluated.
    _xpath_parser.is_empty_nodeset = cxml_set_is_empty(&_xpath_parser.nodeset);
}

static void cxml_xp_visit_Num(cxml_xp_num *node) {
    cxml_number num = cxml_literal_to_num(&node->val);
    _cxml_xp_data* data = _cxml_xp_new_data();
    data->number = num;
    data->type = CXML_XP_DATA_NUMERIC;
    _cxml_xp__e_push(data);
}

static void cxml_xp_visit_String(cxml_xp_string *node) {
    _cxml_xp_data* data = _cxml_xp_new_data();
    cxml_string_dcopy(&data->str, &node->str);
    data->type = CXML_XP_DATA_STRING;
    _cxml_xp__e_push(data);
}

// from cxxplib.c
extern struct _cxml_xp_func_LU fn_LU_table[];

static void cxml_xp_visit_FunctionCall(cxml_xp_functioncall* node){
    // lookup_func_name()
    // invoke func()
    // visit args
    // evaluate func
    // push result to stack
    (*fn_LU_table[node->pos].fn)(node, _cxml_xp__e_pop, _cxml_xp__e_push);
}

static void cxml_xp_visit_BinaryOp(cxml_xp_binaryop* node){
    cxml_xp_visit(node->l_node);
    cxml_xp_visit(node->r_node);
    _cxml_xp_data* right = _cxml_xp__e_pop();
    _cxml_xp_data* left = _cxml_xp__e_pop();
    switch(node->op){
        case CXML_XP_OP_PLUS:
        case CXML_XP_OP_MINUS:
        case CXML_XP_OP_MULT:
        case CXML_XP_OP_DIV:
        case CXML_XP_OP_MOD:
            _cxml_xp_resolve_arithmetic_operation(left, right, _cxml_xp__e_push, node->op);
            break;
        case CXML_XP_OP_EQ:
        case CXML_XP_OP_LEQ:
        case CXML_XP_OP_GEQ:
        case CXML_XP_OP_NEQ:
        case CXML_XP_OP_GT:
        case CXML_XP_OP_LT:
            _cxml_xp_resolve_relative_operation(left, right, _cxml_xp__e_push, node->op);
            break;
        case CXML_XP_OP_AND:
        case CXML_XP_OP_OR:
            _cxml_xp_resolve_and_or_operation(left, right, _cxml_xp__e_push, node->op);
            break;
        case CXML_XP_OP_PIPE:
            _cxml_xp_resolve_pipe_operation(left, right, _sort_nodeset_by_pos, _cxml_xp__e_push);
            break;
        default:
            break;
    }
}

static void cxml_xp_visit_UnaryOp(cxml_xp_unaryop *node){
    cxml_xp_op op = node->op;
    cxml_xp_visit(node->node);
    _cxml_xp_data *r_node = _cxml_xp__e_pop();
    cxml_number num = new_cxml_number();
    _cxml_xp_data_to_numeric(r_node, &num);
    if (op == CXML_XP_OP_MINUS){
        if(num.type == CXML_NUMERIC_DOUBLE_T)
        {
            num.dec_val = -num.dec_val;
        }
    }
    _cxml_xp_data_clear(r_node);

    r_node->type = CXML_XP_DATA_NUMERIC;
    r_node->number = num;
    _cxml_xp__e_push(r_node);
}

void cxml_xp_visit(cxml_xp_astnode * ast_node){  // generic cxml_xp_visit
    switch(ast_node->wrapped_type){
        case CXML_XP_AST_UNARYOP_NODE:
            cxml_xp_visit_UnaryOp(ast_node->wrapped_node.unary);
            break;
        case CXML_XP_AST_BINOP_NODE:
            cxml_xp_visit_BinaryOp(ast_node->wrapped_node.binary);
            break;
        case CXML_XP_AST_PREDICATE_NODE:
            cxml_xp_visit_Predicate(ast_node->wrapped_node.predicate);
            break;
        case CXML_XP_AST_FUNCTION_CALL_NODE:
            cxml_xp_visit_FunctionCall(ast_node->wrapped_node.func_call);
            break;
        case CXML_XP_AST_NUM_NODE:
            cxml_xp_visit_Num(ast_node->wrapped_node.num);
            break;
        case CXML_XP_AST_STR_LITERAL_NODE:
            cxml_xp_visit_String(ast_node->wrapped_node.str_literal);
            break;
        case CXML_XP_AST_STEP_NODE:
            cxml_xp_visit_Step(ast_node->wrapped_node.step);
            break;
        case CXML_XP_AST_PATH_NODE:
            cxml_xp_visit_Path(ast_node);
            break;
        default: break;
    }
}

cxml_set* cxml_xp_eval_expr(){
    if (!_xpath_parser.root_node || !_xpath_parser.root_element) return NULL;
    // don't pop the node off the stack since it's needed when calling cxml_xp_free_ast_nodes()
    cxml_xp_astnode* node = _cxml_stack__get(&_xpath_parser.ast_stack);
    cxml_xp_visit(node);
    _cxml_xp_data *d =  _cxml_xp__e_pop();
    cxml_set *set = ALLOC(cxml_set, 1);
    cxml_set__init_with(set, &d->nodeset);
    d->nodeset = new_cxml_set();
    // free all data
    _cxml_xpath_parser_free();
    return set;
}


static void _set_root_elem(){
    if (_xpath_parser.root_node->root_element){
        _xpath_parser.root_element = _xpath_parser.root_node->root_element;
        return;
    }
    cxml_for_each(node, &_xpath_parser.root_node->children)
    {
        if (_cxml_node_type(node) == CXML_ELEM_NODE){
            _xpath_parser.root_element = node;
            break;
        }
    }
    if (!_xpath_parser.root_element){
        cxml_error("CXML XPath Error: Bad root node, no root element found.");
    }
}


static void _create_virtual_root_node(void *root){
    /*
     * create a virtual root node when an element is passed
     * instead of a root node
     */
    cxml_root_node *root_node = create_root_node();
    // save root_element in root_node
    cxml_list_append(&root_node->children, root);
    _xpath_parser.root_element = root;
    _xpath_parser.root_node = root_node;
    _xpath_parser.root_node->root_element = root;
}

static void _set_roots(void *root){
    if (!root){
        cxml_error("CXML XPath Error: Root cannot be NULL.\n");
    }
    if (_cxml_node_type(root) == CXML_ROOT_NODE){
        _xpath_parser.root_node = root;
        _set_root_elem();
    }else if (_cxml_node_type(root) == CXML_ELEM_NODE){
        _create_virtual_root_node(root);
    }else{
        cxml_error("Unknown root type.\n");
    }
}


void cxml_xp_debug_expr(){
    // don't pop the node off the stack since it's needed
    // when calling cxml_xp_free_ast_nodes()
    cxml_xp_astnode* node = _cxml_stack__get(&_xpath_parser.ast_stack);
    if (!node) return;
    cxml_xp_dvisit(node);
}


/*
 * XMLQuery  ::=     QueryString
 */
// external interface/front end
cxml_set * cxml_xpath(void * root, const char *expr) {
    if (!root || !expr) return NULL;
    query_string(expr);
    _set_roots(root);
    cxml_set *nodeset = cxml_xp_eval_expr();
    return nodeset;
}

