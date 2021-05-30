/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

int parser_init_asserts(_cxml_parser *parser){
    cxml_assert__null(parser->root_element)
    cxml_assert__null(parser->xml_header)
    cxml_assert__null(parser->xml_doctype)
    cxml_assert__null(parser->root_node)
    cxml_assert__null(parser->err_msg)
    cxml_assert__null(parser->current_scope)
    cxml_assert__false(parser->has_dtd)
    cxml_assert__false(parser->has_header)
    cxml_assert__false(parser->is_root_wrapped)
    cxml_assert__true(cxml_list_is_empty(&parser->attr_list))
    cxml_assert__true(cxml_list_is_empty(&parser->errors))
    cxml_assert__true(cxml_list_is_empty(&parser->errors))
    cxml_assert__true(cxml_table_is_empty(&parser->attr_checker))
    cxml_assert__true(_cxml_stack_is_empty(&parser->_cx_stack))
    return 1;
}
cts test__cxml_parser_init(){
    _cxml_parser parser;
    _cxml_parser_init(&parser, wf_xml_9, NULL, false);
    cxml_assert(parser_init_asserts(&parser))
    cxml_pass()
}

cts test_create_root_node(){
    cxml_root_node *root = create_root_node();
    cxml_assert__not_null(root)
    cxml_assert__true(cxml_string_raw_equals(&root->name, cxml_get_config().doc_name))
    cxml_free_root_node(root);
    cxml_pass()
}


cts test_cxml_parse_xml(){
    cxml_root_node *root = cxml_parse_xml(wf_xml_9);
    cxml_assert__not_null(root)
    cxml_assert__not_null(root->root_element)
    cxml_assert__not_null(root->namespaces)
    // global namespaces cannot be empty
    cxml_assert__two(cxml_list_size(root->namespaces))
    cxml_assert__true(root->has_child)
    cxml_assert__true(root->is_well_formed)
    cxml_assert__one(root->pos)
    cxml_free_root_node(root);
    cxml_pass()
}


cts test_cxml_parse_xml_lazy(){
    char *fp = get_file_path("wf_xml_1.xml");
    cxml_root_node *root = cxml_parse_xml_lazy(fp);
    FREE(fp);
    cxml_assert__not_null(root)
    cxml_assert__not_null(root->root_element)
    cxml_assert__not_null(root->namespaces)
    // global namespaces cannot be empty
    cxml_assert__two(cxml_list_size(root->namespaces))
    cxml_assert__true(root->has_child)
    cxml_assert__true(root->is_well_formed)
    cxml_assert__one(root->pos)
    cxml_free_root_node(root);
    cxml_pass()
}


cts test__cxml_parser_free(){
    _cxml_parser parser;
    _cxml_parser_init(&parser, wf_xml_9, NULL, false);
    cxml_assert__eq(parser.cxlexer.start, wf_xml_9)
    _cxml_parser_free(&parser);
    cxml_assert(parser_init_asserts(&parser))
    cxml_assert__zero(strlen(parser.cxlexer.start))
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
