/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxdefs.h"
#include "xml/cxparser.h"

#define parse__error(_p, ...)           \
fprintf(stderr, __VA_ARGS__);           \
_cxml_parser_free(_p);                  \
exit(EXIT_FAILURE);

extern _cxml_token cxml_get_token(_cxml_lexer *cxlexer);

extern char *cxml_token_type_as_str(_cxml_token_t type);

static void x__dispatch_token(_cxml_parser *parser);

static void _cxml_p__inc_err(_cxml_parser *cxparser, char* error);

static void x__comment(_cxml_parser *parser);

static void x__dtd(_cxml_parser *parser);

static void x__pi(_cxml_parser *parser);

static void cxml_free_attr_checker(cxml_table *attr_checker);

static bool _cxml_p__is_whitespace(_cxml_token token);


void _cxml_parser_init(
        _cxml_parser *parser,
        const char *src,
        const char* file_name,
        bool stream)
{
    cxml__assert(parser, "Expected cxml parser.")
    cxml__assert((src || file_name),
            "cxml parser couldn't find any file "
            "name or xml source string.")
    _cxml_lexer_init(&parser->cxlexer, src, file_name, stream);
    _cxml_token_init(&parser->current_tok);
    _cxml_token_init(&parser->prev_tok);
    parser->err_msg = NULL;
    parser->has_header = false;
    parser->has_dtd = false;
    parser->is_root_wrapped = false;
    parser->xml_header = NULL;
    parser->root_element = NULL;
    parser->root_node = NULL;
    cxml_list_init(&parser->errors);
    cxml_list_init(&parser->attr_list);
    cxml_table_init(&parser->attr_checker);
    parser->current_scope = NULL;
    parser->xml_doctype = NULL;
    _cxml_stack_init(&parser->_cx_stack);  // init stack
    parser->pos_c = 0;
    parser->cfg = cxml_get_config();
}

void _cxml_parser_free(_cxml_parser *cxparser) {
    /*
     * free parser by freeing all its attributes in an
     * (necessary) ordered fashion
     */
    cxml__assert(cxparser, "Expected cxml parser.")
    if (cxparser->err_msg){
        cxparser->err_msg[0] == '-' &&
        cxparser->err_msg[1] == '>' ?
        FREE(cxparser->err_msg)     : (void)0;
    }
    // free stack
    _cxml_stack_free(&(cxparser->_cx_stack));
    cxml_list_free(&cxparser->errors);
    // we do not free the root node as this would be used by other processes
    _cxml_scope_table_free(cxparser->current_scope);
    cxml_list_free(&cxparser->attr_list);
    cxml_free_attr_checker(&cxparser->attr_checker);
    // make freed state definite
    _cxml_parser_init(cxparser, "", NULL, false);
}

void cxml_free_attr_checker(cxml_table *attr_checker){
    cxml_for_each(key, &attr_checker->keys){
        FREE(key);
    }
    cxml_table_free(attr_checker);
}

_CX_ATR_NORETURN static void
_cxml_p__handle_error(_cxml_parser *cxparser, _cxml_token *token) {
    /*
     * handle error by emitting a helpful error message, and exiting
     */
    if (!cxparser->err_msg) {
        cxparser->err_msg = cxml_token_type_as_str(token->type);
    }
    if (token){
        parse__error(cxparser, "Error occurred at line %d\nReason: %.*s\n%s\n",
                   token->line, token->length,
                   token->start, cxparser->err_msg)
    }else{
        parse__error(cxparser, "Error occurred at line %d\n-> %s\n",
                     cxparser->cxlexer.line, cxparser->err_msg)
    }
}

static void _cxml_p__advance(_cxml_parser *cxparser){
    /*
     * advance to the next token in the token sequence,
     * i.e. get the next token.
     * If there's an error while trying to obtain the token,
     * try to handle the error gracefully.
     */
    cxparser->prev_tok = cxparser->current_tok;
    _cxml_token token = cxml_get_token(&cxparser->cxlexer);
    if (!cxparser->cxlexer.error) {  //
        cxparser->current_tok = token;
    } else {
        _cxml_p__handle_error(cxparser, &token);
    }
}

inline static void _cxml_p__set_parent(void* parent, void* child){
    if (_cxml_node_type(parent) == CXML_ELEM_NODE){
        cxml_list_append(&_unwrap_cxnode(cxml_elem_node, parent)->children, child);
        _unwrap_cxnode(cxml_elem_node, parent)->has_child = true;
    }else{
        cxml_list_append(&_unwrap_cxnode(cxml_root_node, parent)->children, child);
        _unwrap_cxnode(cxml_root_node, parent)->has_child = true;
    }
}

static void _cxml_p__consume(_cxml_parser *cxparser, _cxml_token_t type) {
    /*
     * consume current token only if its type equals
     * the type passed in, else err
     */
    if (cxparser->current_tok.type == type) {
        _cxml_p__advance(cxparser);
    } else {
        parse__error(cxparser, "Error occurred at line: %d\nCXML Parse Error: "
                    "expected token type %s, got %s ('%.*s')\n",
                     cxparser->cxlexer.line,
                     cxml_token_type_as_str(type),
                     cxml_token_type_as_str(cxparser->current_tok.type),
                     cxparser->current_tok.length,
                     cxparser->current_tok.start)
    }
}

#define __new_cxml_func(node_type)   \
inline static cxml_##node_type##_node* _new_cxml_##node_type(){           \
    cxml_##node_type##_node* node = ALLOCR(cxml_##node_type##_node, 1, \
                                      "Error during parsing.. Not enough memory");    \
    cxml_##node_type##_node_init(node);    \
    return node;            \
}

/***
 * utility functions for making nodes with an initialized state
 * ***/

__new_cxml_func(elem)

__new_cxml_func(text)

__new_cxml_func(comm)

__new_cxml_func(dtd)

__new_cxml_func(xhdr)

__new_cxml_func(attr)

__new_cxml_func(pi)

__new_cxml_func(ns)

/**************************/

cxml_root_node* create_root_node(){
    cxml_root_node *doc = ALLOCR(cxml_root_node, 1, "Error during parsing.. Not enough memory");
    cxml_root_node_init(doc);
    cxml_config cfg = cxml_get_config();
    cxml_string_append(&doc->name, cfg.doc_name, (int) strlen(cfg.doc_name));
    return doc;
}

static void _create_root_node(_cxml_parser *cxparser){
    cxparser->root_node = create_root_node();
}

static int
_cxml_p__check_equals(const char *str_val, const char *check, int len1, int len2) {
    /*
     * check for equality between two char* object,
     * this check considers lower cases equivalents of the first char* argument,
     * assuming `check` is in lower case form.
     */
    if (len1 != len2) return 0;
    for (int i = 0; i < len1; i++) {
        if (tolower((unsigned char)str_val[i]) != check[i]) {
            return 0;
        }
    }
    return 1;
}

static void
_cxml_p__check_pred_entity(cxml_string *str, int *has, cxml_config *cfg) {
    /*
     * inspect str for predefined entities
     */
    // "<>&\"'"
    if (cfg->transpose_text) { // transpose forward
        // check if any untransposed predefined entity (&, <, >) exists in str
        *has = strpbrk(cxml_string_as_raw(str), _CXML_PRED_ENTITY) ? 1 : 0;
        return;
    }
    *has = -1;
}

void cxml_get_unexp_pos_err_msg(_cxml_parser *cxparser){
    /*
     * get error message when parser errs from non-well-formed xml/
     * or from tokens found at an unexpected position
     */
    char* token = cxml_token_type_as_str(cxparser->current_tok.type);
    size_t len = 100;
    char* buff = ALLOC(char, len);
    // `->` is a flag that determines if cxparser.err_msg is allocated
    // or not, to help when freeing cxparser
    snprintf(buff, len, "-> %s found at unexpected position. | "
                        "Possibly outside xml root element\n", token+5);
    cxparser->err_msg = buff;
}

inline static cxml_comm_node* _get_comment(_cxml_token *token){
    cxml_comm_node *comment = _new_cxml_comm();
    // '<!--' ^^^^^^^ '-->'
    cxml_string_append(&comment->value, token->start + 4, token->length - 7);
    return comment;
}

int _cxml_is_ns_URI_equal(const char *found, const char *expected, int f_len, int e_len){
    const char *start = found;
    const char *end = found + f_len - 1;
    start++, end--;  // escape ''/' || '"'
    int sub = 0;
    while (isspace((unsigned char)*start) && start != end) start++, sub++;
    if (start == end) return 0;  // empty or len 1
    while (isspace((unsigned char)*end)) end--, sub++;
    if ((f_len - sub - 2) != e_len) return 0;
    return memcmp(start, expected, e_len) == 0;
}

inline static void _backtrack_ns_scope(_cxml_parser *parser) {
    struct _cxml_scope_table *scope = parser->current_scope;
    parser->current_scope = scope->enclosing_scope;
    _cxml_scope_table_free(scope);
}

inline static void _set_lname(cxml_name *name, int lname_len){
    // for when name has no prefix - local name is same as qualified name
    // dirty hack -> since we do not want to copy lname
    // we just have the `lname` pointer point to the beginning of `qname`
    name->lname = cxml_string_as_raw(&name->qname);
    name->lname_len = lname_len;
}

inline static void _set_name(cxml_name *name, int pname_len, int lname_len){
    // for when name has a prefix
    name->pname = cxml_string_as_raw(&name->qname);
    name->lname = name->pname + pname_len + 1;
    name->pname_len = pname_len;
    name->lname_len = lname_len;
}

extern void cxml_string_from_alloc(cxml_string *str, char **raw, int len);

inline static void
_cxml_append_or_init_tok(cxml_string *str, _cxml_token *token, _cxml_parser *cxparser){
    if (!cxparser->cxlexer._stream){
        cxml_string_append(str, token->start, token->length);
    }else{
        cxml_string_from_alloc(str, &token->start, token->length);
    }
}

inline static void
_cxml_append_or_init_chars(cxml_string *str, char *start, int length, _cxml_parser *cxparser){
    if (!cxparser->cxlexer._stream){
        cxml_string_append(str, start, length);
    }else{
        cxml_string_from_alloc(str, &start, length);
    }
}

#define _cxml_free_unused_tok(token, parser) \
    if (((parser)->cxlexer._stream)) FREE((token)->start);

static void x__name(_cxml_parser *cxparser) {
    /*
     * parse tag name
     */
    // PI node, DOC node, XML_HDR node, ELEM node,
    cxml_elem_node *node = _cxml_stack__get(&cxparser->_cx_stack);
    int pname_len = 0;
    // Constraint (1).(d). : Element names MUST NOT have the prefix xmlns.
    if (_cxml_p__check_equals(cxparser->current_tok.start,
            _CXML_RESERVED_NS_PREFIX_XMLNS,
            cxparser->current_tok.length,
            _CXML_RESERVED_NS_PREFIX_XMLNS_LEN))
    {
        // err. `xmlns` MUST not be used as an element prefix, see constraint (1)
        cxparser->err_msg = "CXML Parse Error: Element name cannot have the "
                            "prefix `xmlns`.";
        _cxml_p__handle_error(cxparser, &cxparser->current_tok);
    }

    _cxml_append_or_init_tok(&node->name.qname, &cxparser->current_tok, cxparser);
    _cxml_p__consume(cxparser, CXML_TOKEN_IDENTIFIER);

    if (cxparser->current_tok.type == CXML_TOKEN_COLON) {
        pname_len = cxml_string_len(&node->name.qname);  // get the length of the namespace prefix
        cxml_string_append(&node->name.qname, ":", 1);
        _cxml_p__advance(cxparser);     // move past CXML_TOKEN_COLON
        _cxml_p__consume(cxparser, CXML_TOKEN_IDENTIFIER);
        _cxml_append_or_init_tok(&node->name.qname, &cxparser->prev_tok, cxparser);
        _set_name(&node->name, pname_len, cxparser->prev_tok.length);
    }else{
        _set_lname(&node->name, cxparser->prev_tok.length);
    }

    // obtain root element
    if (!cxparser->root_element && (node->_type == CXML_ELEM_NODE))
    {
        cxparser->root_element = node;
    }
    // create a new namespace scope:
    struct _cxml_scope_table *new_scope = _cxml_scope_table_new();
    new_scope->enclosing_scope = cxparser->current_scope;
    cxparser->current_scope = new_scope;
}

static void x__ns(_cxml_parser *cxparser){
    /*
         * Constraints:
         * (1) Reserved Prefixes and Namespace Names
         *     (a) The prefix xml is by definition bound to the namespace name
         *         http://www.w3.org/XML/1998/namespace. It MAY, but need not, be declared,
         *         and MUST NOT be bound to any other namespace name. Other prefixes MUST NOT
         *         be bound to this namespace name, and it MUST NOT be declared as the default namespace.
         *     (b) The prefix xmlns is used only to declare namespace bindings and is by definition bound
         *         to the namespace name http://www.w3.org/2000/xmlns/. It MUST NOT be declared.
         *     (c) Other prefixes MUST NOT be bound to this namespace name, and it MUST NOT be declared
         *         as the default namespace.
         *     (d) Element names MUST NOT have the prefix xmlns.
         *
         * (2) Prefix Declared
         *     The namespace prefix, unless it is xml or xmlns, MUST have been
         *     declared in a namespace declaration attribute in either the start-tag
         *     of the element where the prefix is used or in an ancestor element
         *     (i.e., an element in whose content the prefixed markup occurs).
         *
         * (3) No Prefix Undeclaring
         *     In a namespace declaration for a prefix (i.e., where the NSAttName is a PrefixedAttName),
         *     the attribute value MUST NOT be empty.
         *
         * (4) Attributes Unique
         *     No element must have two attributes with the same expanded name.
         */

    // consume 'xmlns'
    _cxml_p__consume(cxparser, CXML_TOKEN_IDENTIFIER);
    _cxml_free_unused_tok(&cxparser->prev_tok, cxparser);

    // cxml_elem_node *node = _cxml_stack__get(&cxparser->_cx_stack);
    cxml_ns_node *namespace = _new_cxml_ns();
    namespace->parent = _cxml_stack__get(&cxparser->_cx_stack);
    bool is_xml_prefix = 0, is_prefix = 0;
    // check if it's a default namespace or a prefixed one:
    if (cxparser->current_tok.type == CXML_TOKEN_COLON){  // prefixed namespace
        _cxml_p__advance(cxparser);  // move past CXML_TOKEN_COLON
        // Constraint (1).(b).
        if (_cxml_p__check_equals(cxparser->current_tok.start,
                                  _CXML_RESERVED_NS_PREFIX_XMLNS,
                                  cxparser->current_tok.length,
                                  _CXML_RESERVED_NS_PREFIX_XMLNS_LEN))
        {
            // err. `xmlns` MUST not be declared as a prefix, see constraint (1)
            cxparser->err_msg = "CXML Parse Error: `xmlns` cannot be used as a"
                                " namespace prefix.";
            _cxml_p__handle_error(cxparser, &cxparser->current_tok);
        }
        else if (_cxml_p__check_equals(cxparser->current_tok.start,
                                       _CXML_RESERVED_NS_PREFIX_XML,
                                       cxparser->current_tok.length,
                                       _CXML_RESERVED_NS_PREFIX_XML_LEN))
        {
            is_xml_prefix = 1;
        }
        is_prefix = 1;
        _cxml_append_or_init_tok(&namespace->prefix, &cxparser->current_tok,
                                 cxparser);
        _cxml_p__consume(cxparser, CXML_TOKEN_IDENTIFIER);
    }
    _cxml_p__consume(cxparser, CXML_TOKEN_EQUAL);
    // Constraint (1).(a).
    if (is_xml_prefix){
        // if prefix is `xml` ensure the URI conforms
        if (!_cxml_is_ns_URI_equal(cxparser->current_tok.start,
                                   _CXML_RESERVED_NS_URI_XML,
                                   cxparser->current_tok.length,
                                   _CXML_RESERVED_NS_URI_XML_LEN))
        {
            // err. `xml` prefix namespace name MUST be the same as the
            // reserved namespace name. See constraint (1)
            cxparser->err_msg = "CXML Parse Error: Namespace URI for `xml` prefix does "
                                "not match the reserved standard URI.";
            _cxml_p__handle_error(cxparser, &cxparser->current_tok);
        }
        namespace->is_global = true;
        goto ns_uri;
    }
    // no other prefix or default namespace must use the reserved namespace
    // names(URIs) also ensure that prefix declarations doesn't have empty URIs
    else if (is_prefix)
    {
        bool is_empty = 1;
        char opening = cxparser->current_tok.start[0];
        // escape '"' || '\'' at both sides of the string
        for (int i = 1; i <= cxparser->current_tok.length - 1; i++){
            if (!isspace((unsigned char)cxparser->current_tok.start[i])
                && cxparser->current_tok.start[i] != opening)
            {
                is_empty = 0;
                break;
            }
        }
        // Constraint (3)
        if (is_empty){
            // err. no namespace with a prefix can have an empty URI
            cxparser->err_msg = "CXML Parse Error: Found an empty namespace URI for "
                                "a namespace with a declared prefix.";
            _cxml_p__handle_error(cxparser, &cxparser->current_tok);
        }
    }
    // Constraint (1).(c).
    if (_cxml_is_ns_URI_equal(cxparser->current_tok.start,
                              _CXML_RESERVED_NS_URI_XML,
                              cxparser->current_tok.length,
                              _CXML_RESERVED_NS_URI_XML_LEN)
        || _cxml_is_ns_URI_equal(cxparser->current_tok.start,
                                 _CXML_RESERVED_NS_URI_XMLNS,
                                 cxparser->current_tok.length,
                                 _CXML_RESERVED_NS_URI_XMLNS_LEN))
    {
        // err. no namespace (default or prefixed) MUST have the same URI as the
        // reserved URIs or namespace names. See constraint (1)
        cxparser->err_msg = "CXML Parse Error: Namespace URI is in "
                            "collision with the reserved standard URI(s).";
        _cxml_p__handle_error(cxparser, &cxparser->current_tok);
    }
    ns_uri:
    _cxml_append_or_init_chars(&namespace->uri,
                               cxparser->current_tok.start,
                               cxparser->current_tok.length,
                               cxparser);
    _cxml_p__consume(cxparser, CXML_TOKEN_STRING);
    namespace->is_default = !is_prefix;
    // insert the namespace into the current scope
    if (_cxml_scope_table_insert(cxparser->current_scope,
                             is_prefix ?
                             cxml_string_as_raw(&namespace->prefix) :
                             _cxml_xmlns_name,
                             namespace) == 0x02)
    {
        cxparser->err_msg = "CXML Parse Error: Duplicate namespaces "
                            "found in element declaration.";
        _cxml_p__handle_error(cxparser, NULL);
    }
    namespace->pos = ++cxparser->pos_c;
    // append to `attr_list` for later resolution
    cxml_list_append(&cxparser->attr_list, namespace);
}


static void x__attr(_cxml_parser *cxparser) {
    /*
     * parse all name="value" pairs
     */
    // name="value"

    if (_cxml_p__check_equals(cxparser->current_tok.start,
                              _cxml_xmlns_name,
                              cxparser->current_tok.length, 5))
    {
        x__ns(cxparser);
    }else{
        int pname_len = 0;

        cxml_attr_node* attr = _new_cxml_attr();
        _cxml_p__consume(cxparser, CXML_TOKEN_IDENTIFIER);
        attr->pos = ++cxparser->pos_c;
        _cxml_append_or_init_tok(&attr->name.qname, &cxparser->prev_tok, cxparser);

        if (cxparser->current_tok.type == CXML_TOKEN_COLON){  // namespaced attributes
            pname_len = cxparser->prev_tok.length;
            cxml_string_append(&attr->name.qname, ":", 1);
            _cxml_p__advance(cxparser);     // move past CXML_TOKEN_COLON
            _cxml_p__consume(cxparser, CXML_TOKEN_IDENTIFIER);
            _cxml_append_or_init_tok(&attr->name.qname,
                                     &cxparser->prev_tok,
                                     cxparser);
            _set_name(&attr->name, pname_len, cxparser->prev_tok.length);
        }else{
            _set_lname(&attr->name, cxparser->prev_tok.length);
        }
        _cxml_p__consume(cxparser, CXML_TOKEN_EQUAL);
        _cxml_append_or_init_chars(&attr->value,
                                   cxparser->current_tok.start,
                                   cxparser->current_tok.length,
                                   cxparser);
        cxml_set_literal(&attr->number_value, cxparser->current_tok.literal_type, &attr->value);
        attr->parent = _cxml_stack__get(&cxparser->_cx_stack);
        _cxml_p__consume(cxparser, CXML_TOKEN_STRING);
        attr->parent->has_attribute = true;
        // add to `attributes` for later namespace resolution
        cxml_list_append(&cxparser->attr_list, attr);
    }
}

static void x__xml_decl(_cxml_parser *cxparser) {
    /*
     * parse xml (header) declaration
     * : <?xml version="1.0" encoding="UTF-8"?>
     *
     */
    if (cxparser->current_tok.type == CXML_TOKEN_L_THAN) {
        _cxml_p__advance(cxparser);     // move past CXML_TOKEN_L_THAN
    }
    // may/not be xml prolog
    if (cxparser->current_tok.type == CXML_TOKEN_Q_MARK) {
        _cxml_p__advance(cxparser);     // move past CXML_TOKEN_Q_MARK
        if (!_cxml_p__check_equals(cxparser->current_tok.start,
                                   _cxml_xml_name, cxparser->current_tok.length, 3))
        {
            x__pi(cxparser);
            return;
        }
        _cxml_p__consume(cxparser, CXML_TOKEN_IDENTIFIER);
        _cxml_free_unused_tok(&cxparser->prev_tok, cxparser)
        cxml_xhdr_node* xml_hdr = _new_cxml_xhdr();
        cxml_attr_node* attr;
        while (cxparser->current_tok.type == CXML_TOKEN_IDENTIFIER) {
            attr = _new_cxml_attr();
            attr->pos = ++cxparser->pos_c;
            _cxml_append_or_init_tok(&attr->name.qname,
                                     &cxparser->current_tok,
                                     cxparser);
            _cxml_p__consume(cxparser, CXML_TOKEN_IDENTIFIER);
            if (cxparser->current_tok.type == CXML_TOKEN_COLON){
                cxparser->err_msg = "CXML Parse Error: Cannot use "
                                    "namespace prefix in xml prolog.";
                _cxml_p__handle_error(cxparser, &cxparser->prev_tok);
            }
            else if (cxml_string_lraw_equals(&attr->name.qname, _cxml_xmlns_name, 5)){
                cxparser->err_msg = "CXML Parse Error: Cannot declare "
                                    "namespace in xml prolog.";
                _cxml_p__handle_error(cxparser, &cxparser->prev_tok);
            }
            _cxml_p__consume(cxparser, CXML_TOKEN_EQUAL);
            _cxml_append_or_init_chars(&attr->value,
                                       cxparser->current_tok.start,
                                       cxparser->current_tok.length,
                                       cxparser);
            _cxml_p__consume(cxparser, CXML_TOKEN_STRING);
            if (cxml_table_put(&xml_hdr->attributes,
                               cxml_string_as_raw(&attr->name.qname),
                               attr) == 0x02)
            {
                cxparser->err_msg = "CXML Parse Error: Duplicate "
                                    "attributes found in xml prolog.";
                _cxml_p__handle_error(cxparser, NULL);
            }
        }
        xml_hdr->parent = cxparser->root_node;
        cxml_list_append(&cxparser->root_node->children, xml_hdr);
        cxparser->xml_header = xml_hdr;
        cxparser->has_header = 1;
        _cxml_p__consume(cxparser, CXML_TOKEN_Q_MARK);
        _cxml_p__consume(cxparser, CXML_TOKEN_G_THAN);
    }
}

extern _cxml_token _cxml_move_until(
        _cxml_lexer *lexer, char target,
        _cxml_token_t type, const char *msg);

static void x__pi(_cxml_parser *cxparser) {
    // take start from name or `<`
    while (cxparser->current_tok.type == CXML_TOKEN_L_THAN
           || cxparser->current_tok.type == CXML_TOKEN_Q_MARK)
    {
        // move past current token
        _cxml_p__advance(cxparser);
    }
    if (cxparser->current_tok.type == CXML_TOKEN_IDENTIFIER)
    {
        cxml_pi_node* node = _new_cxml_pi();
        _cxml_append_or_init_tok(&node->target, &cxparser->current_tok, cxparser);
        // move past CXML_TOKEN_IDENTIFIER
        _cxml_p__advance(cxparser);
        if (cxparser->current_tok.type == CXML_TOKEN_Q_MARK){
            goto end;
        }
        // make sure document is namespace well formed - we cannot have namespace prefixes in a PI target
        if (cxparser->current_tok.type == CXML_TOKEN_COLON){
            cxparser->err_msg = "CXML Parse Error: Cannot have namespace"
                                " prefix in processing-instruction target.";
            _cxml_p__handle_error(cxparser, &cxparser->prev_tok);
        }
        // store the last valid line parsed
        int line = cxparser->current_tok.line;
        // try to obtain the string value of the processing-instruction
        _cxml_token token = _cxml_move_until(&cxparser->cxlexer, '?',
                                             CXML_TOKEN_TEXT, "");
        // if we run into an error, report the error, using the line information stored initially
        if (token.type == CXML_TOKEN_ERROR){
            parse__error(cxparser, "Error occurred at line %d\n-> `%.*s`\n%s\n",
                         line, token.length <= 30 ? token.length : 30, // print first 30 chars
                         token.start, "Invalid processing-instruction declaration. Missing '?'.")
        }
        cxml_string_append(&node->value, token.start, token.length);
        _cxml_p__advance(cxparser);
        end:
        _cxml_p__consume(cxparser, CXML_TOKEN_Q_MARK);
        _cxml_p__consume(cxparser, CXML_TOKEN_G_THAN);
        node->parent = _cxml_stack__get(&cxparser->_cx_stack);
        _cxml_p__set_parent(node->parent, node);
        node->pos = ++cxparser->pos_c;
    }
}

static void x__misc(_cxml_parser *cxparser){
    while (cxparser->current_tok.type == CXML_TOKEN_COMMENT
           || cxparser->current_tok.type == CXML_TOKEN_L_THAN
           || cxparser->current_tok.type == CXML_TOKEN_TEXT
           || cxparser->current_tok.type == CXML_TOKEN_CDATA
           )
    {
        if (cxparser->current_tok.type == CXML_TOKEN_COMMENT){
            x__comment(cxparser);
        }
        else if (cxparser->current_tok.type == CXML_TOKEN_TEXT){
            if (!_cxml_p__is_whitespace(cxparser->current_tok)){
                _cxml_p__inc_err(cxparser, "Found non-whitespace text at an unexpected position.");
            }

            _cxml_p__advance(cxparser);
        }
        else if (cxparser->current_tok.type == CXML_TOKEN_CDATA){
            _cxml_p__inc_err(cxparser, "Found CDATA at an unexpected position.");
            _cxml_p__advance(cxparser);
        }
        else{
            _cxml_p__consume(cxparser, CXML_TOKEN_L_THAN);
            if (cxparser->current_tok.type == CXML_TOKEN_Q_MARK){
                x__pi(cxparser);
            }
        }
    }
}

static void x__dtd(_cxml_parser *cxparser) {
    /*
     * parse xml dtd
     */
    if (cxparser->current_tok.type == CXML_TOKEN_DOCTYPE) {
        // create cxml_elem_node for dtd
        cxparser->xml_doctype = _new_cxml_dtd();

        cxml_string_append(&cxparser->xml_doctype->value, "<!DOCTYPE ", 10);
        cxml_string_append(&cxparser->xml_doctype->value,
                           cxparser->current_tok.start,
                           cxparser->current_tok.length);
        cxml_string_append(&cxparser->xml_doctype->value, ">", 1);

        cxparser->xml_doctype->_type = CXML_DTD_NODE;

        // set associations/relationships
        cxparser->xml_doctype->parent = cxparser->root_node; // will always be dtd's parent
        cxml_list_append(&cxparser->root_node->children, cxparser->xml_doctype);
        _cxml_p__consume(cxparser, CXML_TOKEN_DOCTYPE);
        cxparser->has_dtd = true;
    }
}


static void x__cdata_sect(_cxml_parser *cxparser){
    // cdata is only valid within root element node
    cxml_text_node* text = _new_cxml_text();
    // "<![CDATA[" ^^^^^ "]]>"
    cxml_string_append(
            &text->value,
            cxparser->current_tok.start + 9,  // escape "<![CDATA["
            cxparser->current_tok.length - 12); // and "]]>"

    text->parent = _cxml_stack__get(&cxparser->_cx_stack);
    text->is_cdata = 1;
    _cxml_p__set_parent(text->parent, text);
    text->pos = ++cxparser->pos_c;
    _cxml_p__consume(cxparser, CXML_TOKEN_CDATA);
}


static void x__comment(_cxml_parser *cxparser) {
    /*
     * parse xml comment
     */
    cxml_comm_node* comment = _get_comment(&cxparser->current_tok);
    // set associations/relationships
    comment->parent = _cxml_stack__get(&cxparser->_cx_stack);
    // set flags
    if (_cxml_get_node_type(comment->parent) == CXML_ELEM_NODE) {
        cxml_list_append(&_unwrap_cxnode(cxml_elem_node, comment->parent)->children, comment);
        _unwrap_cxnode(cxml_elem_node, comment->parent)->has_child = true;
        _unwrap_cxnode(cxml_elem_node, comment->parent)->has_comment = true;
    }else{
        cxml_list_append(&_unwrap_cxnode(cxml_root_node, comment->parent)->children, comment);
        _unwrap_cxnode(cxml_root_node, comment->parent)->has_child = true;
    }
    comment->pos = ++cxparser->pos_c;
    _cxml_p__consume(cxparser, CXML_TOKEN_COMMENT);
}

static void x__text(_cxml_parser *cxparser) {
    /*
     * parse xml text
     */
    // for text found outside root node
    if (cxparser->is_root_wrapped){
        // err if text found isn't whitespace
        if (!_cxml_p__is_whitespace(cxparser->current_tok)){
            _cxml_p__inc_err(cxparser, "Found non-whitespace text outside root element.");
        }
        _cxml_p__advance(cxparser);
        return;
    }
    if (cxparser->current_tok.type == CXML_TOKEN_TEXT) {
        int has;
        cxml_text_node *TEXT = _new_cxml_text();
        TEXT->parent = _cxml_stack__get(&cxparser->_cx_stack);
        cxml_string_append(&TEXT->value,
                           cxparser->current_tok.start,
                           cxparser->current_tok.length);
        // check for xml predefined entities if the parser is configured to transpose text
        if (cxparser->cfg.transpose_text) {
            _cxml_p__check_pred_entity(&TEXT->value, &has, &cxparser->cfg);
            if (has && has != -1) {
                TEXT->has_entity = true; // useful for transposition
            }
        }
        // literals
        // set text literal type and numeric val if applicable
        cxml_set_literal(&TEXT->number_value, cxparser->current_tok.literal_type, &TEXT->value);
        if (_cxml_node_type(TEXT->parent) == CXML_ELEM_NODE){
            cxml_list_append(&_unwrap_cxnode(cxml_elem_node, TEXT->parent)->children, TEXT);
            _unwrap_cxnode(cxml_elem_node, TEXT->parent)->has_text = 1;
        }else{
            cxml_list_append(&_unwrap_cxnode(cxml_root_node, TEXT->parent)->children, TEXT);
        }
        TEXT->pos = ++cxparser->pos_c;
        _cxml_p__consume(cxparser, CXML_TOKEN_TEXT);
    }
}

static void x__wrap_elem(_cxml_parser *cxparser, cxml_elem_node *node) {
    // </stuff>
    // />
    _cxml_p__consume(cxparser, CXML_TOKEN_F_SLASH);

    // check if it is the root element that's being wrapped.
    // if so, set is_root_wrapped to true to propagate error handling of
    // any other element node outside the root element

    if (cxparser->current_tok.type == CXML_TOKEN_IDENTIFIER) {
        if (node == cxparser->root_element)
        {
            cxparser->is_root_wrapped = true;
        }
        // catch bad xml tags with mismatching closing part
        // e.g. <stuff>123</nuff>
        if (node->is_namespaced){
            if (node->namespace->is_default) goto cmp;
            _cxml_token pname = cxparser->current_tok;
            _cxml_p__consume(cxparser, CXML_TOKEN_IDENTIFIER);
            _cxml_p__consume(cxparser, CXML_TOKEN_COLON);
            int len = pname.length + cxparser->current_tok.length + 1;
            char buff[len + 1];
            memcpy(buff, pname.start, pname.length);
            *(buff + pname.length) = ':';
            memcpy(buff + pname.length + 1, cxparser->current_tok.start,
                   cxparser->current_tok.length);
            buff[len] = '\0';
            // free `pname`
            _cxml_free_unused_tok(&pname, cxparser)
            if (cxml_string_lraw_equals(&node->name.qname, buff, len)) goto _if;
            goto _else;
        }
        cmp:
        if (cxml_string_lraw_equals(&node->name.qname,
                                    cxparser->current_tok.start,
                                    cxparser->current_tok.length))
        {
            _if:;
            // free `lname` or `qname` depending on how the body of the
            // if statement is entered (either from jump in if statement above,
            // or from normal condition entry).
            _cxml_free_unused_tok(&cxparser->current_tok, cxparser)
            node->has_child = cxml_list_is_empty(&node->children) ? false : true;
            node->is_self_enclosing = !node->has_child;
            _cxml_p__consume(cxparser, CXML_TOKEN_IDENTIFIER);
        }else{
            _else:;
            size_t len = (100 + cxml_string_len(&node->name.qname));
            char *buff = ALLOC(char, len);
            // `->` is a flag that determines if cxparser.err_msg is allocated
            // or not, to help when freeing cxparser
            snprintf(buff, len, "-> %s %s `%s` ?", "Closing tag mismatch.",
                     "Perhaps you meant", cxml_string_as_raw(&node->name.qname));
            cxparser->err_msg = buff;
            _cxml_p__handle_error(cxparser, &cxparser->current_tok);
        }
    }else{
        node->has_child = false;
        node->is_self_enclosing = true;
    }
    _cxml_p__consume(cxparser, CXML_TOKEN_G_THAN);
    _backtrack_ns_scope(cxparser);
}

_CX_ATR_NORETURN static void _cxml_ns_error(
        _cxml_parser *parser,
        cxml_name *name,
        const char *_for)
{
    parse__error(parser, "Error occurred at line %d\n%s `%.*s` %s `%s`\n",
               parser->cxlexer.line,
               "CXML Parse Error: Could not find namespace "
               "corresponding to the prefix",
               name->pname_len, name->pname,
               _for, cxml_string_as_raw(&name->qname))
}

static cxml_ns_node *_get_ns(
        _cxml_parser *parser,
        cxml_name *name,
        const char* _name)
{
    if (name){
        char buff[name->pname_len + 1];
        memcpy(buff, name->pname, name->pname_len);
        buff[name->pname_len] = '\0';
        return _cxml_scope_table_lookup(parser->current_scope, buff);
    }
    return _cxml_scope_table_lookup(parser->current_scope, _name);
}

static void _resolve_namespace(_cxml_parser *parser, cxml_elem_node *elem)
{
    // one way to optimize this is to not deal with namespace resolution
    // first try to resolve element
    cxml_ns_node *ns;
    if (elem->name.pname)
    {
        ns = _get_ns(parser, &elem->name, NULL);
        if (ns == NULL){  // Constraint (2) Prefix Declared
            _cxml_ns_error(parser, &elem->name, "for element");
        }
        elem->namespace = ns;
        elem->is_namespaced = 1;
    }else if (parser->cfg.allow_default_namespace){ // we need to check for default namespaces
        // we do not need to check if ns is NULL or not, since ns is a default
        ns = _get_ns(parser, NULL, _cxml_xmlns_name);
        // we need to prevent global namespace 'xmlns' or 'xml' from being attached to `elem`
        // since those aren't default namespaces (xmlns:xml=".../" && xmlns:xmlns=".../")
        // the global `xml` namespace requires that the element or attribute
        // explicitly uses the `xml` prefix, and the global `xmlns` namespace cannot be
        // used as a namespace prefix with an element or attribute
        if (ns && ns->is_default){ // only bind a default namespace to `elem`
            elem->namespace = ns;
            elem->is_namespaced = 1;
        }
    }
    // next we resolve attributes
    cxml_attr_node *attr;
    cxml_string expanded_name;
    cxml_for_each(node, &parser->attr_list)
    {
        attr = node;
        if (attr->_type == CXML_NS_NODE){
            if (!elem->namespaces){
                elem->namespaces = new_alloc_cxml_list();
            }
            cxml_list_append(elem->namespaces, node);
            continue;
        }
        if (!elem->attributes){
            elem->attributes = new_alloc_cxml_table();
        }
        if (attr->name.pname){
            ns = _get_ns(parser, &attr->name, NULL);
            if (ns == NULL){  // Constraint (2) Prefix Declared
                _cxml_ns_error(parser, &attr->name, "for attribute");
            }
            attr->namespace = ns;
            if (parser->cfg.ensure_ns_attribute_unique){
                expanded_name = new_cxml_string();
                // expand `x:name` to `foo-uri:name`
                cxml_string_str_append(&expanded_name, &ns->uri);
                cxml_string_append(&expanded_name, ":", 1);
                cxml_string_append(&expanded_name, attr->name.lname, attr->name.lname_len);

                // Constraint (4) Attributes Unique
                if (cxml_table_put(&parser->attr_checker,
                                   cxml_string_as_raw(&expanded_name),
                                   cxml_string_as_raw(&expanded_name)) == 0x02)
                {
                    goto err;
                }
            }
            cxml_table_put(elem->attributes,
                           cxml_string_as_raw(&attr->name.qname),
                           attr);
        }
        else{ // Constraint (4) Attributes Unique
            if (cxml_table_put(elem->attributes,
                           cxml_string_as_raw(&attr->name.qname),
                           attr) == 0x02)
            {
                goto err;
            }
        }
        continue;
        err:
            parse__error(parser, "Error occurred at line: %d\nCXML Parse Error: "
                        "Duplicate attributes found in element declaration: `%s`\n",
                         parser->cxlexer.line,
                         cxml_string_as_raw(&attr->name.qname))
    }
    // free attributes stored earlier in x__attr()
    cxml_list_free(&parser->attr_list);
    // free any allocated string and the attr_checker,
    // used in namespace uniqueness validation
    cxml_free_attr_checker(&parser->attr_checker);
}

static void x__elem(_cxml_parser *cxparser) {
     /*
      * <stuff fun="ny">...</stuff>
      * <stuff fun="ny" />
      * <stuff />

      * <stuff fun="ny">
      *      text here
      * </stuff>

      * <stuff fun="ny">
      *      text here
      *      <another>
      *          another text here
      *      </another>
      * </stuff>
      */

    cxml_elem_node *node = _new_cxml_elem();
    node->pos = ++cxparser->pos_c;
    _cxml_stack__push(&cxparser->_cx_stack, node);
    x__name(cxparser);

    while (cxparser->current_tok.type == CXML_TOKEN_IDENTIFIER) {
        x__attr(cxparser);
    }

    _resolve_namespace(cxparser, node);

    if (cxparser->current_tok.type == CXML_TOKEN_G_THAN) {
        _cxml_p__advance(cxparser);

        while ( (cxparser->current_tok.type == CXML_TOKEN_L_THAN)
                || (cxparser->current_tok.type == CXML_TOKEN_TEXT)
                || (cxparser->current_tok.type == CXML_TOKEN_COMMENT)
                || (cxparser->current_tok.type == CXML_TOKEN_CDATA))
        {
            x__dispatch_token(cxparser);
        }
        x__wrap_elem(cxparser, node);  // clean up closing tag
    }
    else{  // self_enclosing
        x__wrap_elem(cxparser, node);
        node->has_text = false;
    }
    // pop the current element off the stack
    _cxml_stack__pop(&cxparser->_cx_stack);

    // try to obtain a previously pushed element on the stack
    void *prev_node = _cxml_stack__get(&cxparser->_cx_stack);

    // add to children
    // condition holds if there are more than one root element,
    // that is, after root element has been parsed, there exists
    // yet another element, which isn't a processing instruction node
    if (cxparser->is_root_wrapped && node != cxparser->root_element){
        // err
        _cxml_p__inc_err(cxparser, "Found an element outside the root element.");
        // free the node
        cxml_elem_node_free(node);
        return;
    }

    _cxml_p__set_parent(prev_node, node);
    //make parent
    node->parent = prev_node;

    // set has_parent flag
    node->has_parent = true;

    // has_text would be set if text() is encountered in the while loop above
    // no need to push current `element` node to stack, since prev_node already
    // wraps the current `element` node
}

static void save_avlb_comments(_cxml_parser *cxparser){
    // we may encounter comments/cdata outside/before the root element and
    // outside the dtd, and xml header (if any of those are available),
    // we escape such cdata/texts, save comments for further processing,
    // and set the error flag in cxparser.
    while (cxparser->current_tok.type == CXML_TOKEN_COMMENT
           || cxparser->current_tok.type == CXML_TOKEN_CDATA
           || cxparser->current_tok.type == CXML_TOKEN_TEXT)
    {
        if (cxparser->current_tok.type == CXML_TOKEN_COMMENT){
            x__comment(cxparser);
        }else{
            // consume such tokens to allow the parser continue its parsing
            _cxml_p__advance(cxparser);
            _cxml_p__inc_err(cxparser, "Found one of TEXT/CDATA/Comment(s) outside root element.");
        }
    }
}

static void process_comments(_cxml_parser *cxparser){
    void *par = _cxml_stack__get(&cxparser->_cx_stack);

    cxml_list *children = _cxml__get_node_children(par);
    if (cxparser->has_header){
        int i = 0;
        cxml_for_each(comm, children){
            if (_cxml_node_type(comm) != CXML_COMM_NODE) break;
            _cxml_p__inc_err(cxparser, "Found Comment(s) before xml declaration.");
            cxml_comm_node_free(comm);
            i++;
        }
        // remove freed comment nodes
        for (int j=0; j<i; j++){
            cxml_list_delete(children, 0);
        }
    }
}

static bool _cxml_p__is_whitespace(_cxml_token token){
    int count;
    for (count=0;
        count<token.length && isspace((unsigned char)*(token.start + count));
        count++){}
    return (count == token.length);
}

static void show_errors(_cxml_parser *cxparser){
    int err_count = cxml_list_size(&cxparser->errors);
    char *err = err_count > 1 ? "errors": "error";
    cxml_for_each(error, &cxparser->errors){
        fprintf(stderr, "%s\n", (char*)error);
    }
    if (err_count) fprintf(stderr, "Found at least %d %s during parsing.\n", err_count, err);
}

static void _cxml_p__inc_err(_cxml_parser *cxparser, char* error){
    /*
     * increment error count - add new error
     */
    cxml_list_append(&cxparser->errors, error);
}

static void x__dispatch_token(_cxml_parser *cxparser) {
    /*
     * dispatch tokens to their respective functions
     * for parsing, if no token matches any case,
     * then an error must have occurred, handle such error
     */
    if (cxparser->current_tok.type == CXML_TOKEN_L_THAN) {
        _cxml_p__advance(cxparser);
    }

    switch (cxparser->current_tok.type)
    {
        case CXML_TOKEN_COMMENT:
            x__comment(cxparser);
            break;
        case CXML_TOKEN_Q_MARK:
            x__pi(cxparser);
            break;
        case CXML_TOKEN_IDENTIFIER:
            x__elem(cxparser);
            break;
        case CXML_TOKEN_TEXT:
            x__text(cxparser);
            break;
        case CXML_TOKEN_CDATA:
            x__cdata_sect(cxparser);
            break;
        case CXML_TOKEN_F_SLASH:  // x__wrap_elem()
            break;
        case CXML_TOKEN_ERROR:
            parse__error(cxparser, "%.*s\n", cxparser->current_tok.length, cxparser->current_tok.start)
        case CXML_TOKEN_G_THAN:
        default:
        {
            cxparser->err_msg = "Parse Error.\n"
                               "Possible causes: Nameless tag. "
                               "All tags must have a name";
            _cxml_p__handle_error(cxparser, &cxparser->current_tok);
        }
    }
}

static void _inject_global_namespaces(_cxml_parser *parser){
    parser->current_scope =  _cxml_scope_table_new();
    parser->root_node->namespaces = new_alloc_cxml_list();
    cxml_ns_node *ns;
    for (int i = 0; i < 2; i++) {
        ns = _new_cxml_ns();
        ns->is_global = 1;
        cxml_string_append(&ns->prefix,
                           _cxml_reserved_prefixes[i],
                           _cxml_reserved_prefixes_len[i]);
        cxml_string_append(&ns->uri,
                           _cxml_reserved_namespaces[i],
                           _cxml_reserved_namespaces_len[i]);
        _cxml_scope_table_insert(parser->current_scope, _cxml_reserved_prefixes[i], ns);
        cxml_list_append(parser->root_node->namespaces, ns);
    }
}

// prolog ::= XMLDecl? Misc*(doctypedecl Misc*)?
static void x__prolog(_cxml_parser *cxparser){
    // we may encounter comments/cdata outside/before the root element and
    // outside the dtd, and xml header (if any of those are available),
    // we escape such cdata/texts, save comments for further processing,
    // and set the error flag in cxparser.
    save_avlb_comments(cxparser);
    x__xml_decl(cxparser);
    // process comments found earlier than xml header
    process_comments(cxparser);
    x__misc(cxparser);
    x__dtd(cxparser);
    x__misc(cxparser);
    /***********/
}

static void x__document(_cxml_parser *cxparser) {
    /*
     * root document - root node,
     * parse xml recursively into a single node
     * document ::= prolog element Misc*
     */

    // advance to set the current token
    _cxml_p__advance(cxparser);

    // create root node and register it to parser
    _create_root_node(cxparser);
    _cxml_stack__push(&cxparser->_cx_stack, cxparser->root_node);
    cxparser->root_node->pos = ++cxparser->pos_c;
    _inject_global_namespaces(cxparser);
    /***********/
    x__prolog(cxparser);
    /***********/
    // element MISC*
    while (cxparser->current_tok.type == CXML_TOKEN_L_THAN ||
           cxparser->prev_tok.type == CXML_TOKEN_L_THAN ||
           cxparser->current_tok.type == CXML_TOKEN_TEXT ||
           cxparser->current_tok.type == CXML_TOKEN_COMMENT)
    {
        x__dispatch_token(cxparser);  // <, <!--,
    }
    /***********/
    if (cxparser->current_tok.type != CXML_TOKEN_EOF){
        // update cxparser.err_msg
        cxml_get_unexp_pos_err_msg(cxparser);
        // err fast.
        _cxml_p__handle_error(cxparser, &cxparser->current_tok);
    }

    cxparser->root_node->is_well_formed = cxml_list_is_empty(&cxparser->errors);
    cxparser->root_node->root_element = cxparser->root_element;

    if (cxparser->cfg.show_warnings) {
        show_errors(cxparser);
    }
    // pop the root node off the stack
    _cxml_stack__pop(&cxparser->_cx_stack);
}


cxml_root_node* cxml_parse_xml_lazy(const char *file_name) {
    /*
     *  parse xml into a root node by streaming
     */
    cxml__assert(file_name, "Expected file name.")
    _cxml_parser cxparser;
    _cxml_parser_init(&cxparser, NULL, file_name, true);
    x__document(&cxparser);
    _cxml_lexer_close(&cxparser.cxlexer);
    cxml_root_node *root = cxparser.root_node;
    _cxml_parser_free(&cxparser);
    return root;
}

cxml_root_node* cxml_parse_xml(const char *src) {
    /*
     * parse xml into a root node
     */
    cxml__assert(src, "Expected source string.")
    _cxml_parser cxparser;
    _cxml_parser_init(&cxparser, src, NULL, false);
    x__document(&cxparser);
    _cxml_lexer_close(&cxparser.cxlexer);
    cxml_root_node *root = cxparser.root_node;
    _cxml_parser_free(&cxparser);
    return root;
}


/*
 * private functions exposed publicly for use in cxsax.c
 */
/*** x__*() parsing helper functions ***/
void (*x__xml_decl_ptr)(_cxml_parser *parser) = x__xml_decl;

void (*x__name_ptr)(_cxml_parser *cxparser) = x__name;

void (*x__attr_ptr)(_cxml_parser *cxparser) = x__attr;

void (*x__wrap_elem_ptr)(_cxml_parser *cxparser, cxml_elem_node *node) = x__wrap_elem;

void (*x__comment_ptr)(_cxml_parser *cxparser) = x__comment;

void (*x__cdata_sect_ptr)(_cxml_parser *cxparser) = x__cdata_sect;

void (*x__dtd_ptr)(_cxml_parser *cxparser) = x__dtd;

void (*x__pi_ptr)(_cxml_parser *cxparser) = x__pi;

void (*x__text_ptr)(_cxml_parser *cxparser) = x__text;

/**** other functions from cxparser.h used in cxsax.c ****/
void (*_cxml_p_show_errors_ptr)(_cxml_parser *cxparser) = show_errors;

void (*_cxml_p_consume_ptr)(_cxml_parser *cxparser, _cxml_token_t type) = _cxml_p__consume;

cxml_elem_node *(*_cxml_p_new_elem_ptr)(void) = _new_cxml_elem;

void (*_cxml_p_advance_ptr)(_cxml_parser *cxparser) = _cxml_p__advance;

bool (*_cxml_p_is_whitespace_ptr)(_cxml_token token) = _cxml_p__is_whitespace;

void (*_cxml_p_resolve_namespace_ptr)(_cxml_parser *parser, cxml_elem_node *elem) = _resolve_namespace;

void (*_cxml_p_inject_global_namespaces_ptr)(_cxml_parser *parser) = _inject_global_namespaces;
