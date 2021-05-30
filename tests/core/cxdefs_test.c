/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

int empty_elem_asserts(cxml_elem_node *node){
    cxml_assert__true(node->is_self_enclosing)
    cxml_assert__eq(node->_type, CXML_ELEM_NODE)
    cxml_assert__null(node->parent)
    cxml_assert__false(node->is_namespaced)
    cxml_assert__false(node->has_attribute)
    cxml_assert__false(node->has_child)
    cxml_assert__false(node->has_text)
    cxml_assert__false(node->has_parent)
    cxml_assert__false(node->has_comment)
    cxml_assert__null(node->attributes)
    cxml_assert__null(node->namespace)
    cxml_assert__null(node->namespaces)
    cxml_assert__zero(node->pos)
    cxml_assert__zero(cxml_list_size(&node->children))
    cxml_assert__zero(cxml_string_len(&node->name.qname))
    return 1;
}

int empty_text_asserts(cxml_text_node *node){
    cxml_assert__eq(node->_type, CXML_TEXT_NODE)
    cxml_assert__zero(cxml_string_len(&node->value))
    cxml_assert__zero(node->pos)
    cxml_assert__null(node->parent)
    cxml_assert__false(node->has_entity)
    cxml_assert__false(node->is_cdata)
    cxml_assert__eq(node->number_value.type, CXML_NUMERIC_NAN_T)
    cxml_assert__zero(node->number_value.dec_val)
    return 1;
}

int empty_attr_asserts(cxml_attr_node *node){
    cxml_assert__eq(node->_type, CXML_ATTR_NODE)
    cxml_assert__zero(cxml_string_len(&node->name.qname))
    cxml_assert__zero(cxml_string_len(&node->value))
    cxml_assert__zero(node->pos)
    cxml_assert__null(node->parent)
    cxml_assert__null(node->namespace)
    cxml_assert__eq(node->number_value.type, CXML_NUMERIC_NAN_T)
    cxml_assert__zero(node->number_value.dec_val)
    return 1;
}

int empty_ns_asserts(cxml_ns_node *node){
    cxml_assert__eq(node->_type, CXML_NS_NODE)
    cxml_assert__zero(cxml_string_len(&node->prefix))
    cxml_assert__zero(cxml_string_len(&node->uri))
    cxml_assert__zero(node->pos)
    cxml_assert__null(node->parent)
    cxml_assert__false(node->is_global)
    cxml_assert__false(node->is_default)
    return 1;
}

int empty_root_asserts(cxml_root_node *node){
    cxml_assert__eq(node->_type, CXML_ROOT_NODE)
    cxml_assert__false(node->is_well_formed)
    cxml_assert__false(node->has_child)
    cxml_assert__null(node->root_element)
    cxml_assert__null(node->namespaces)
    cxml_assert__zero(node->pos)
    cxml_assert__zero(cxml_list_size(&node->children))
    cxml_assert__zero(cxml_string_len(&node->name))
    return 1;
}

int empty_comm_asserts(cxml_comm_node *node){
    cxml_assert__eq(node->_type, CXML_COMM_NODE)
    cxml_assert__zero(cxml_string_len(&node->value))
    cxml_assert__zero(node->pos)
    cxml_assert__null(node->parent)
    return 1;
}

int empty_pi_asserts(cxml_pi_node *node){
    cxml_assert__eq(node->_type, CXML_PI_NODE)
    cxml_assert__zero(cxml_string_len(&node->target))
    cxml_assert__zero(cxml_string_len(&node->value))
    cxml_assert__zero(node->pos)
    cxml_assert__null(node->parent)
    return 1;
}

int empty_dtd_asserts(cxml_dtd_node *node){
    cxml_assert__eq(node->_type, CXML_DTD_NODE)
    cxml_assert__zero(cxml_string_len(&node->value))
    cxml_assert__null(node->parent)
    return 1;
}

int empty_xhdr_asserts(cxml_xhdr_node *node){
    cxml_assert__eq(node->_type, CXML_XHDR_NODE)
    cxml_assert__null(node->attributes.entries)
    cxml_assert__zero(node->attributes.count)
    cxml_assert__zero(node->attributes.capacity)
    cxml_assert__true(cxml_table_is_empty(&node->attributes))
    return 1;
}

cts test_cxml_elem_node_init(){
    cxml_elem_node node;
    cxml_elem_node_init(&node);
    cxml_assert__one(empty_elem_asserts(&node))

    // no seg-fault
    cxml_elem_node_init(NULL);
    cxml_pass()
}

cts test_cxml_text_node_init(){
    cxml_text_node node;
    cxml_text_node_init(&node);
    cxml_assert__one(empty_text_asserts(&node))
    // no seg-fault
    cxml_text_node_init(NULL);
    cxml_pass()
}

cts test_cxml_attr_node_init(){
    cxml_attr_node node;
    cxml_attr_node_init(&node);
    cxml_assert__one(empty_attr_asserts(&node))
    // no seg-fault
    cxml_attr_node_init(NULL);
    cxml_pass()
}

cts test_cxml_ns_node_init(){
    cxml_ns_node node;
    cxml_ns_node_init(&node);
    cxml_assert__one(empty_ns_asserts(&node))
    // no seg-fault
    cxml_ns_node_init(NULL);
    cxml_pass()
}

cts test_cxml_root_node_init(){
    cxml_root_node node;
    cxml_root_node_init(&node);
    cxml_assert__one(empty_root_asserts(&node))

    // no seg-fault
    cxml_root_node_init(NULL);
    cxml_pass()
}

cts test_cxml_comm_node_init(){
    cxml_comm_node node;
    cxml_comm_node_init(&node);
    cxml_assert__one(empty_comm_asserts(&node))

    // no seg-fault
    cxml_comm_node_init(NULL);
    cxml_pass()
}

cts test_cxml_pi_node_init(){
    cxml_pi_node node;
    cxml_pi_node_init(&node);
    cxml_assert__one(empty_pi_asserts(&node))

    // no seg-fault
    cxml_pi_node_init(NULL);
    cxml_pass()
}

cts test_cxml_dtd_node_init(){
    cxml_dtd_node node;
    cxml_dtd_node_init(&node);
    cxml_assert__one(empty_dtd_asserts(&node))

    // no seg-fault
    cxml_dtd_node_init(NULL);
    cxml_pass()
}

cts test_cxml_xhdr_node_init(){
    cxml_xhdr_node node;
    cxml_xhdr_node_init(&node);
    cxml_assert__one(empty_xhdr_asserts(&node))

    // no seg-fault
    cxml_xhdr_node_init(NULL);
    cxml_pass()
}

cts test_cxml_name_init(){
    cxml_name name;
    cxml_name_init(&name);
    cxml_assert__zero(cxml_string_len(&name.qname))
    cxml_assert__zero(name.pname_len)
    cxml_assert__zero(name.lname_len)
    cxml_assert__null(name.lname)
    cxml_assert__null(name.pname)
    cxml_pass()
}

cts test__cxml_get_node_type(){
    cxml_elem_node *node = ALLOC(cxml_elem_node, 1);
    cxml_elem_node_init(node);
    cxml_elem_node_init(node);
    cxml_assert__eq(_cxml_get_node_type(node), CXML_ELEM_NODE)
    cxml_assert__eq(_cxml_get_node_type(NULL), 0xff)
    FREE(node);
    cxml_pass()
}

cts test__cxml_get_node_pos(){
    cxml_elem_node node;
    cxml_elem_node_init(&node);
    cxml_assert__zero(_cxml_get_node_pos(&node))
    node.pos = 5;
    cxml_assert__eq(_cxml_get_node_pos(&node), 5)
    cxml_pass()
}

cts test__cxml_get_node_parent(){
    cxml_elem_node node, parent;
    cxml_elem_node_init(&node);
    cxml_elem_node_init(&parent);
    node.parent = &parent;
    cxml_assert__eq(_cxml_get_node_parent(&node), &parent)
    cxml_assert__null(_cxml_get_node_parent(&parent))
    cxml_assert__null(_cxml_get_node_parent(NULL))
    cxml_pass()
}

cts test__cxml_node_parent(){
    cxml_elem_node node, parent;
    cxml_elem_node_init(&node);
    cxml_elem_node_init(&parent);
    node.parent = &parent;
    cxml_assert__eq(_cxml_node_parent(&node), &parent)
    cxml_assert__null(_cxml_node_parent(&parent))
    cxml_pass()
}

cts test__cxml_unset_parent(){
    cxml_elem_node node, parent;
    cxml_elem_node_init(&node);
    cxml_elem_node_init(&parent);
    node.parent = &parent;
    cxml_assert__eq(_cxml_node_parent(&node), &parent)
    _cxml_unset_parent(&node);
    cxml_assert__null(_cxml_node_parent(&node))
    cxml_pass()
}

cts test__cxml_cmp_node(){
    cxml_elem_node *node1 = ALLOC(cxml_elem_node, 1);
    cxml_elem_node *node2 = ALLOC(cxml_elem_node, 1);
    cxml_elem_node_init(node1);
    cxml_elem_node_init(node2);
    cxml_assert__zero(_cxml_cmp_node(&node1, &node2))
    node2->pos = 2;

    // node2 comes after node1 -> +2
    cxml_assert__two(_cxml_cmp_node(&node2, &node1))

    // node1 comes before node2 -> -2
    cxml_assert__eq(_cxml_cmp_node(&node1, &node2), -2)

    cxml_assert__zero(_cxml_cmp_node(&node2, &node2))
    cxml_assert__zero(_cxml_cmp_node(&node1, &node1))

    cxml_elem_node_free(node1);
    cxml_elem_node_free(node2);
    cxml_pass()
}


void suite_cxdefs(){
    cxml_suite(cxdefs)
    {
        cxml_add_m_test(16,
                        test_cxml_elem_node_init,
                        test_cxml_text_node_init,
                        test_cxml_attr_node_init,
                        test_cxml_ns_node_init,
                        test_cxml_root_node_init,
                        test_cxml_comm_node_init,
                        test_cxml_pi_node_init,
                        test_cxml_dtd_node_init,
                        test_cxml_xhdr_node_init,
                        test_cxml_name_init,
                        test__cxml_get_node_type,
                        test__cxml_get_node_pos,
                        test__cxml_get_node_parent,
                        test__cxml_node_parent,
                        test__cxml_unset_parent,
                        test__cxml_cmp_node
                        )
        cxml_run_suite()
    }
}
