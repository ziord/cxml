/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"
#include <Muon/Muon.h>

void empty_elem_asserts(cxml_elem_node *node){
    CHECK_TRUE(node->is_self_enclosing);
    CHECK_EQ(node->_type, CXML_ELEM_NODE);
    CHECK_NULL(node->parent);
    CHECK_FALSE(node->is_namespaced);
    CHECK_FALSE(node->has_attribute);
    CHECK_FALSE(node->has_child);
    CHECK_FALSE(node->has_text);
    CHECK_FALSE(node->has_parent);
    CHECK_FALSE(node->has_comment);
    CHECK_NULL(node->attributes);
    CHECK_NULL(node->namespace);
    CHECK_NULL(node->namespaces);
    CHECK_EQ(node->pos, 0);
    CHECK_EQ(cxml_list_size(&node->children), 0);
    CHECK_EQ(cxml_string_len(&node->name.qname), 0);
}

void empty_text_asserts(cxml_text_node *node){
    CHECK_EQ(node->_type, CXML_TEXT_NODE);
    CHECK_EQ(cxml_string_len(&node->value), 0);
    CHECK_EQ(node->pos, 0);
    CHECK_NULL(node->parent);
    CHECK_FALSE(node->has_entity);
    CHECK_FALSE(node->is_cdata);
    CHECK_EQ(node->number_value.type, CXML_NUMERIC_NAN_T);
    CHECK_EQ(node->number_value.dec_val, 0);
}

void empty_attr_asserts(cxml_attr_node *node){
    CHECK_EQ(node->_type, CXML_ATTR_NODE);
    CHECK_EQ(cxml_string_len(&node->name.qname), 0);
    CHECK_EQ(cxml_string_len(&node->value), 0);
    CHECK_EQ(node->pos, 0);
    CHECK_NULL(node->parent);
    CHECK_NULL(node->namespace);
    CHECK_EQ(node->number_value.type, CXML_NUMERIC_NAN_T);
    CHECK_EQ(node->number_value.dec_val, 0);
}

void empty_ns_asserts(cxml_ns_node *node){
    CHECK_EQ(node->_type, CXML_NS_NODE);
    CHECK_EQ(cxml_string_len(&node->prefix), 0);
    CHECK_EQ(cxml_string_len(&node->uri), 0);
    CHECK_EQ(node->pos, 0);
    CHECK_NULL(node->parent);
    CHECK_FALSE(node->is_global);
    CHECK_FALSE(node->is_default);
}

void empty_root_asserts(cxml_root_node *node){
    CHECK_EQ(node->_type, CXML_ROOT_NODE);
    CHECK_FALSE(node->is_well_formed);
    CHECK_FALSE(node->has_child);
    CHECK_NULL(node->root_element);
    CHECK_NULL(node->namespaces);
    CHECK_EQ(node->pos, 0);
    CHECK_EQ(cxml_list_size(&node->children), 0);
    CHECK_EQ(cxml_string_len(&node->name), 0);
}

void empty_comm_asserts(cxml_comm_node *node){
    CHECK_EQ(node->_type, CXML_COMM_NODE);
    CHECK_EQ(cxml_string_len(&node->value), 0);
    CHECK_EQ(node->pos, 0);
    CHECK_NULL(node->parent);
}

void empty_pi_asserts(cxml_pi_node *node){
    CHECK_EQ(node->_type, CXML_PI_NODE);
    CHECK_EQ(cxml_string_len(&node->target), 0);
    CHECK_EQ(cxml_string_len(&node->value), 0);
    CHECK_EQ(node->pos, 0);
    CHECK_NULL(node->parent);
}

void empty_dtd_asserts(cxml_dtd_node *node){
    CHECK_EQ(node->_type, CXML_DTD_NODE);
    CHECK_EQ(cxml_string_len(&node->value), 0);
    CHECK_NULL(node->parent);
}

void empty_xhdr_asserts(cxml_xhdr_node *node){
    CHECK_EQ(node->_type, CXML_XHDR_NODE);
    CHECK_NULL(node->attributes.entries);
    CHECK_EQ(node->attributes.count, 0);
    CHECK_EQ(node->attributes.capacity, 0);
    CHECK_TRUE(cxml_table_is_empty(&node->attributes));
}

TEST(cxdefs, cxml_elem_node_init){
    cxml_elem_node node;
    cxml_elem_node_init(&node);
    empty_elem_asserts(&node);

    // no seg-fault
    cxml_elem_node_init(NULL);
}

TEST(cxdefs, cxml_text_node_init){
    cxml_text_node node;
    cxml_text_node_init(&node);
    empty_text_asserts(&node);
    // no seg-fault
    cxml_text_node_init(NULL);
}

TEST(cxdefs, cxml_attr_node_init){
    cxml_attr_node node;
    cxml_attr_node_init(&node);
    empty_attr_asserts(&node);
    // no seg-fault
    cxml_attr_node_init(NULL);
}

TEST(cxdefs, cxml_ns_node_init){
    cxml_ns_node node;
    cxml_ns_node_init(&node);
    empty_ns_asserts(&node);
    // no seg-fault
    cxml_ns_node_init(NULL);
}

TEST(cxdefs, cxml_root_node_init){
    cxml_root_node node;
    cxml_root_node_init(&node);
    empty_root_asserts(&node);

    // no seg-fault
    cxml_root_node_init(NULL);
}

TEST(cxdefs, cxml_comm_node_init){
    cxml_comm_node node;
    cxml_comm_node_init(&node);
    empty_comm_asserts(&node);

    // no seg-fault
    cxml_comm_node_init(NULL);
}

TEST(cxdefs, cxml_pi_node_init){
    cxml_pi_node node;
    cxml_pi_node_init(&node);
    empty_pi_asserts(&node);

    // no seg-fault
    cxml_pi_node_init(NULL);
}

TEST(cxdefs, cxml_dtd_node_init){
    cxml_dtd_node node;
    cxml_dtd_node_init(&node);
    empty_dtd_asserts(&node);

    // no seg-fault
    cxml_dtd_node_init(NULL);
}

TEST(cxdefs, cxml_xhdr_node_init){
    cxml_xhdr_node node;
    cxml_xhdr_node_init(&node);
    empty_xhdr_asserts(&node);

    // no seg-fault
    cxml_xhdr_node_init(NULL);
}

TEST(cxdefs, cxml_get_node_type) {
    cxml_elem_node *node = ALLOC(cxml_elem_node, 1);
    cxml_elem_node_init(node);
    cxml_elem_node_init(node);
    CHECK_EQ(_cxml_get_node_type(node), CXML_ELEM_NODE);
    CHECK_EQ(_cxml_get_node_type(NULL), 0xff);
    FREE(node);
}

TEST(cxdefs, cxml_get_node_pos) {
    cxml_elem_node node;
    cxml_elem_node_init(&node);
    CHECK_EQ(_cxml_get_node_pos(&node), 0);
    node.pos = 5;
    CHECK_EQ(_cxml_get_node_pos(&node), 5);
}

TEST(cxdefs, cxml_get_node_parent) {
    cxml_elem_node node, parent;
    cxml_elem_node_init(&node);
    cxml_elem_node_init(&parent);
    node.parent = &parent;
    CHECK_EQ(_cxml_get_node_parent(&node), &parent);
    CHECK_NULL(_cxml_get_node_parent(&parent));
    CHECK_NULL(_cxml_get_node_parent(NULL));
}

TEST(cxdefs, cxml_node_parent) {
    cxml_elem_node node, parent;
    cxml_elem_node_init(&node);
    cxml_elem_node_init(&parent);
    node.parent = &parent;
    CHECK_EQ(_cxml_node_parent(&node), &parent);
    CHECK_NULL(_cxml_node_parent(&parent));
}

TEST(cxdefs, cxml_unset_parent) {
    cxml_elem_node node, parent;
    cxml_elem_node_init(&node);
    cxml_elem_node_init(&parent);
    node.parent = &parent;
    CHECK_EQ(_cxml_node_parent(&node), &parent);
    _cxml_unset_parent(&node);
    CHECK_NULL(_cxml_node_parent(&node));
}

TEST(cxdefs, cxml_cmp_node) {
    cxml_elem_node *node1 = ALLOC(cxml_elem_node, 1);
    cxml_elem_node *node2 = ALLOC(cxml_elem_node, 1);
    cxml_elem_node_init(node1);
    cxml_elem_node_init(node2);
    CHECK_EQ(_cxml_cmp_node(&node1, &node2), 0);
    node2->pos = 2;

    // node2 comes after node1 -> +2
    CHECK_EQ(_cxml_cmp_node(&node2, &node1), 2);

    // node1 comes before node2 -> -2
    CHECK_EQ(_cxml_cmp_node(&node1, &node2), -2);

    CHECK_EQ(_cxml_cmp_node(&node2, &node2), 0);
    CHECK_EQ(_cxml_cmp_node(&node1, &node1), 0);

    cxml_elem_node_free(node1);
    cxml_elem_node_free(node2);
}
