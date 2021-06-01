/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"
#include <Muon/Muon.h>

void parser_init_asserts(_cxml_parser *parser){
    CHECK_NULL(parser->root_element);
    CHECK_NULL(parser->xml_header);
    CHECK_NULL(parser->xml_doctype);
    CHECK_NULL(parser->root_node);
    CHECK_NULL(parser->err_msg);
    CHECK_NULL(parser->current_scope);
    CHECK_FALSE(parser->has_dtd);
    CHECK_FALSE(parser->has_header);
    CHECK_FALSE(parser->is_root_wrapped);
    CHECK_TRUE(cxml_list_is_empty(&parser->attr_list));
    CHECK_TRUE(cxml_list_is_empty(&parser->errors));
    CHECK_TRUE(cxml_list_is_empty(&parser->errors));
    CHECK_TRUE(cxml_table_is_empty(&parser->attr_checker));
    CHECK_TRUE(_cxml_stack_is_empty(&parser->_cx_stack));
}
TEST(cxparser, cxml_parser_init){
    _cxml_parser parser;
    _cxml_parser_init(&parser, wf_xml_9, NULL, false);
    parser_init_asserts(&parser);
}

TEST(cxparser, create_root_node){
    cxml_root_node *root = create_root_node();
    CHECK_NOT_NULL(root);
    CHECK_TRUE(cxml_string_raw_equals(&root->name, cxml_get_config().doc_name));
    cxml_free_root_node(root);
}


TEST(cxparser, cxml_parse_xml){
    cxml_root_node *root = cxml_parse_xml(wf_xml_9);
    CHECK_NOT_NULL(root);
    CHECK_NOT_NULL(root->root_element);
    CHECK_NOT_NULL(root->namespaces);
    // global namespaces cannot be empty
    CHECK_EQ(cxml_list_size(root->namespaces), 2);
    CHECK_TRUE(root->has_child);
    CHECK_TRUE(root->is_well_formed);
    CHECK_EQ(root->pos, 1);
    cxml_free_root_node(root);
}


// TEST(cxparser, cxml_parse_xml_lazy){
//     char *fp = get_file_path("wf_xml_1.xml");
//     cxml_root_node *root = cxml_parse_xml_lazy(fp);
//     FREE(fp);
//     CHECK_NOT_NULL(root);
//     CHECK_NOT_NULL(root->root_element);
//     CHECK_NOT_NULL(root->namespaces);
//     // global namespaces cannot be empty
//     CHECK_EQ(cxml_list_size(root->namespaces), 2);
//     CHECK_TRUE(root->has_child);
//     CHECK_TRUE(root->is_well_formed);
//     CHECK_EQ(root->pos, 1);
//     cxml_free_root_node(root);
// }


TEST(cxparser, cxml_parser_free){
    _cxml_parser parser;
    _cxml_parser_init(&parser, wf_xml_9, NULL, false);
    CHECK_EQ(parser.cxlexer.start, wf_xml_9);
    _cxml_parser_free(&parser);
    parser_init_asserts(&parser);
    CHECK_EQ(strlen(parser.cxlexer.start), 0);
}