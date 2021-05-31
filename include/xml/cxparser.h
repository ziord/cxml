/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXPARSER_H
#define CXML_CXPARSER_H

#include "xml/cxlexer.h"
#include "core/cxstack.h"
#include "xml/cxscope.h"
#include "core/cxdefs.h"
#include "core/cxlrucache.h"


typedef struct _cxml_parser{
    _cxml_token current_tok;
    _cxml_token prev_tok;
    cxml_xhdr_node *xml_header;
    // no special treatment for dtd, since non-validating
    cxml_dtd_node *xml_doctype;
    cxml_elem_node *root_element;
    cxml_root_node *root_node;
    bool has_header;
    bool has_dtd;
    // flag for when the root element is wrapped
    bool is_root_wrapped;
    // node position counter
    unsigned int pos_c;
    cxml_list errors;
    cxml_table attr_checker;
    // temporarily store attributes after they're being parsed for post-processing.
    cxml_list attr_list;
    _cxml_stack _cx_stack;
    _cxml_lexer cxlexer;
    char *err_msg;
    // config
    cxml_config cfg;
    // namespace scope lookup - for namespace scoping and resolution
    struct _cxml_scope_table *current_scope;
}_cxml_parser;


void _cxml_parser_init(
        _cxml_parser *parser,
        const char *src,
        const char* file_name,
        bool stream);

cxml_root_node* create_root_node();

cxml_root_node* cxml_parse_xml(const char *src);

cxml_root_node* cxml_parse_xml_lazy(const char *file_name);

void _cxml_parser_free(_cxml_parser *cxparser);


#endif //CXML_CXPARSER_H
