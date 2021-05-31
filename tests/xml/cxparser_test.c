/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

int parser_init_asserts(_cxml_parser *parser){
    CHECK_EQ(parser->root_element, NULL)
    CHECK_EQ(parser->xml_header, NULL)
    CHECK_EQ(parser->xml_doctype, NULL)
    CHECK_EQ(parser->root_node, NULL)
    CHECK_EQ(parser->err_msg, NULL)
    CHECK_EQ(parser->current_scope, NULL)
    CHECK_FALSE(parser->has_dtd)
    CHECK_FALSE(parser->has_header)
    CHECK_FALSE(parser->is_root_wrapped)
    CHECK_TRUE(cxml_list_is_empty(&parser->attr_list))
    CHECK_TRUE(cxml_list_is_empty(&parser->errors))
    CHECK_TRUE(cxml_list_is_empty(&parser->errors))
    CHECK_TRUE(cxml_table_is_empty(&parser->attr_checker))
    CHECK_TRUE(_cxml_stack_is_empty(&parser->_cx_stack))
    return 1;
}
cts test__cxml_parser_init(){
    _cxml_parser parser;
    _cxml_parser_init(&parser, wf_xml_9, NULL, false);
    CHECK(parser_init_asserts(&parser))
    cxml_pass()
}

cts test_create_root_node(){
    cxml_root_node *root = create_root_node();
    CHECK_NE(root, NULL)
    CHECK_TRUE(cxml_string_raw_equals(&root->name, cxml_get_config().doc_name))
    cxml_free_root_node(root);
    cxml_pass()
}


cts test_cxml_parse_xml(){
    cxml_root_node *root = cxml_parse_xml(wf_xml_9);
    CHECK_NE(root, NULL)
    CHECK_NE(root->root_element, NULL)
    CHECK_NE(root->namespaces, NULL)
    // global namespaces cannot be empty
    CHECK_EQ(cxml_list_size(root->namespaces), 2)
    CHECK_TRUE(root->has_child)
    CHECK_TRUE(root->is_well_formed)
    CHECK_EQ(root->pos, 1)
    cxml_free_root_node(root);
    cxml_pass()
}


cts test_cxml_parse_xml_lazy(){
    char *fp = get_file_path("wf_xml_1.xml");
    cxml_root_node *root = cxml_parse_xml_lazy(fp);
    FREE(fp);
    CHECK_NE(root, NULL)
    CHECK_NE(root->root_element, NULL)
    CHECK_NE(root->namespaces, NULL)
    // global namespaces cannot be empty
    CHECK_EQ(cxml_list_size(root->namespaces), 2)
    CHECK_TRUE(root->has_child)
    CHECK_TRUE(root->is_well_formed)
    CHECK_EQ(root->pos, 1)
    cxml_free_root_node(root);
    cxml_pass()
}


cts test__cxml_parser_free(){
    _cxml_parser parser;
    _cxml_parser_init(&parser, wf_xml_9, NULL, false);
    CHECK_EQ(parser.cxlexer.start, wf_xml_9)
    _cxml_parser_free(&parser);
    CHECK(parser_init_asserts(&parser))
    CHECK_EQ(strlen(parser.cxlexer.start), 0)
    cxml_pass()
}

void suite_cxparser(){
    cxml_suite(cxparser)
    {
        cxml_add_m_test(5,
                        test__cxml_parser_init,
                        test_create_root_node,
                        test_cxml_parse_xml,
                        test_cxml_parse_xml_lazy,
                        test__cxml_parser_free
        )
        cxml_run_suite()
    }
}
