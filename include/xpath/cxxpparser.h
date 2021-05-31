/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXXPPARSER_H
#define CXML_CXXPPARSER_H

#include "xml/cxparser.h"
#include "cxxplexer.h"
#include "cxxpdata.h"
#include "cxxpvisitors.h"
#include "cxxpcontext.h"


typedef struct {
    // number of tokens consumed
    int consume_cnt;
    // flag to indicate if the result of evaluating an expr is an empty node-set
    bool is_empty_nodeset;
    // flag to determine if the parsed path nodes resides inside a predicate node
    bool from_predicate;
    // previous token being parsed
    _cxml_xp_token prev_tok;
    // current token being parsed
    _cxml_xp_token current_tok;
    // list to store all allocated _cxml_xp_data objects for later de-allocation
    cxml_list data_nodes;
    // xpath ast stack
    _cxml_stack ast_stack;
    // result accumulator stack
    _cxml_stack acc_stack;
    // node-set accumulator | store current nodes (node-set)
    cxml_set nodeset;
    // global xml namespace
    cxml_ns_node *xml_namespace;
    // lexer for the xpath expression
    _cxml_xp_lexer lexer;
    // current root node
    cxml_root_node *root_node;
    cxml_elem_node *root_element;
    // lru cache for already evaluated nodes
    _cxml_lru_cache lru_cache;
    // list to store all allocated cxml_list objects used in caching, for later de-allocation
    cxml_list alloc_set_list;
    // context_state stack
    _cxml_stack ctx_stack;
    // context state of the xpath node objects
    struct _cxml_xp_context_state context;
} _cxml_xp_parser;

_cxml_xp_parser _xpath_parser;


void _cxml_xpath_parser_free();

void cxml_xp_free_ast_nodes(_cxml_xp_parser *xpp);

#endif //CXML_CXXPPARSER_H
