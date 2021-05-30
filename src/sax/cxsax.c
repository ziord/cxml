/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "sax/cxsax.h"


#define _close_flag \
((1 << 6) | 0xdeadbeef)

#define _close_r(__r)   \
    if ((__r->is_open) == 1) (__r->is_open) = _close_flag;

#define _is_closed_r(__r)    \
    (__r->is_open) == _close_flag

#define _is_open_r(__r)    \
    ((__r->is_open) == 1)

/*
 * private functions exposed publicly for use in cxsax.c
 */
/*** x__*() parsing helper functions ***/
extern void (*x__xml_decl_ptr)(_cxml_parser *parser);

extern void (*x__name_ptr)(_cxml_parser *cxparser);

extern void (*x__attr_ptr)(_cxml_parser *cxparser);

extern void (*x__wrap_elem_ptr)(_cxml_parser *cxparser, cxml_elem_node *node);

extern void (*_cxml_p_show_errors_ptr)(_cxml_parser *cxparser);

extern void (*x__comment_ptr)(_cxml_parser *cxparser);

extern void (*x__cdata_sect_ptr)(_cxml_parser *cxparser);

extern void (*x__dtd_ptr)(_cxml_parser *cxparser);

extern void (*x__pi_ptr)(_cxml_parser *cxparser);

extern void (*x__text_ptr)(_cxml_parser *cxparser);

extern cxml_elem_node* (*_cxml_p_new_elem_ptr)(void);

extern void (*_cxml_p_consume_ptr)(_cxml_parser *cxparser, _cxml_token_t type);

extern void (*_cxml_p_advance_ptr)(_cxml_parser *cxparser);

extern bool (*_cxml_p_is_whitespace_ptr)(_cxml_token token);

extern void (*_cxml_p_resolve_namespace_ptr)(_cxml_parser *parser, cxml_elem_node *elem);

extern void (*_cxml_p_inject_global_namespaces_ptr)(_cxml_parser *parser);



/*********************************
 *           cxml sax            *
 *********************************
 */

static void
_cxml_sax_document_end_event(
        cxml_sax_event_reader *reader,
        cxml_sax_event_t *event, bool suppress);


static cxml_sax_event _cxml_sax_new_event(const cxml_sax_event_t *type){
    return (cxml_sax_event){.is_consumed = false, .type = type ? *type : CXML_SAX_NIL_EVENT};
}

static void _cleanup_reader(cxml_sax_event_reader *reader) {
    cxml_sax_event_t event;
    int stack_size;
    // if reader is closed properly, only 1 item should be on the stack
    if ((stack_size = _cxml_stack_size(&reader->xml_parser->_cx_stack)) > 1){
        while (stack_size-- > 1){
            cxml_elem_node_free(_cxml_stack__pop(&reader->xml_parser->_cx_stack));
        }
    }
    _cxml_sax_document_end_event(reader, &event, true);
}

void cxml_sax_open_event_reader(
        cxml_sax_event_reader* reader,
        const char* file_name,
        bool auto_close)
{
    cxml__assert (reader, "CXML SAX Error: Expected event reader\n")
    cxml__assert (file_name, "CXML SAX Error: Expected file name\n")
    reader->prev_event = reader->curr_event = (cxml_sax_event) {
            .type = CXML_SAX_INIT_STATE_EVENT,
            .is_consumed = false
    };
    reader->xml_parser = ALLOC(_cxml_parser, 1);
    reader->is_well_formed = false;
    reader->is_open = 1;
    reader->auto_close = auto_close;
    _cxml_parser_init(reader->xml_parser, NULL, file_name, true);
}

void cxml_sax_close_event_reader(cxml_sax_event_reader* reader){
    cxml__assert (reader, "CXML SAX Error: Expected event reader object.\n")
    // reader could be closed prematurely
    if ((reader->curr_event.type != CXML_SAX_END_DOCUMENT_EVENT)
        && _is_open_r(reader))
    {
        _cleanup_reader(reader);
    }
    _cxml_lexer_close(&reader->xml_parser->cxlexer);
    _cxml_parser_free(reader->xml_parser);
    FREE(reader->xml_parser);
    _close_r(reader)
}

cxml_sax_event_reader cxml_sax_init(const char *file_name, bool auto_close){
    cxml_sax_event_reader reader;
    cxml_sax_open_event_reader(&reader, file_name, auto_close);
    return reader;
}

static void _cxml_sax_expect_or_err(
        _cxml_parser *parser,
        const char* err_msg,
        int argc, ...)
{
    va_list types;
    va_start(types, argc);
    _cxml_token_t token_type;
    for (int i = 0; i < argc; i++) {
        token_type = va_arg(types, _cxml_token_t);
        // return, if we match any of the given types
        if (parser->current_tok.type == token_type){
            va_end(types);
            return;
        }
    }
    va_end(types);
    // err if no single type is matched
    cxml_error("CXML SAX Error: Error at line %d\n%s\n",
               parser->cxlexer.line, err_msg);
}

/*********************************
 *                               *
 *      Event drivers            *
 *********************************
 */
static void
_cxml_sax_element_start_event(_cxml_parser *parser, cxml_sax_event_t *event)
{
    if (parser->is_root_wrapped){
        cxml_error("CXML SAX Error: Multiple roots element found, which is not allowed.\n");
    }
    cxml_elem_node *node = _cxml_p_new_elem_ptr();
    _cxml_stack__push(&parser->_cx_stack, node);
    x__name_ptr(parser);
    // expect
    _cxml_sax_expect_or_err(parser,
                            "Expected identifier, '/' or '>'.",
                            3, CXML_TOKEN_IDENTIFIER,
                            CXML_TOKEN_F_SLASH, CXML_TOKEN_G_THAN);
    // if element has no attributes, resolve namespace now, and not later
    if (parser->current_tok.type != CXML_TOKEN_IDENTIFIER){
        // resolve namespace if any
        _cxml_p_resolve_namespace_ptr(parser, node);
    }
    if (parser->current_tok.type == CXML_TOKEN_G_THAN){
        _cxml_p_consume_ptr(parser, CXML_TOKEN_G_THAN);
    }
    *event = CXML_SAX_BEGIN_ELEMENT_EVENT;
}

static void
_cxml_sax_element_end_event(_cxml_parser *parser, cxml_sax_event_t *event){
    cxml_elem_node *elem = _cxml_stack__pop(&parser->_cx_stack);
    x__wrap_elem_ptr(parser, elem);
    cxml_elem_node_free(elem);
    *event = CXML_SAX_END_ELEMENT_EVENT;
}

static void
_cxml_sax_attr_event(_cxml_parser *parser, cxml_sax_event_t *event){
    // ensure element was previously found
    if (parser->prev_tok.type != CXML_TOKEN_IDENTIFIER){
        cxml_error("CXML SAX Error: Potential text/identifier outside element node.");
    }
    while (parser->current_tok.type == CXML_TOKEN_IDENTIFIER) {
        x__attr_ptr(parser);
    }
    // current token may be '>' if not self-enclosing
    // e.g. <foo x='bar'>
    // if self-enclosing, then current token would be '/'
    // e.g. <foo x='bar' /> <- in that case, it's up to x__wrap_elem() to handle
    if (parser->current_tok.type == CXML_TOKEN_G_THAN){
        _cxml_p_consume_ptr(parser, CXML_TOKEN_G_THAN);
    }
    cxml_elem_node *elem = _cxml_stack__get(&parser->_cx_stack);

    // resolve namespace if any
    _cxml_p_resolve_namespace_ptr(parser, elem);

    if (elem->namespaces && elem->has_attribute){
        *event = CXML_SAX_NAMESPACE_ATTRIBUTE_EVENT;
    }else if (elem->namespaces && !elem->has_attribute){
        *event = CXML_SAX_NAMESPACE_EVENT;
    }else if (!elem->namespaces && elem->has_attribute){
        *event = CXML_SAX_ATTRIBUTE_EVENT;
    }
}

static void
_cxml_sax_text_event(_cxml_parser *parser, cxml_sax_event_t *event){
    if (!parser->root_element
        && !_cxml_p_is_whitespace_ptr(parser->current_tok))
    {
        // non-whitespace text should not come before root element
        _cxml_p_advance_ptr(parser);
        *event = CXML_SAX_TEMP_ERROR_STATE_EVENT;
    }else if (parser->is_root_wrapped){
        // in this case text should not come after root node is wrapped.
        *event = CXML_SAX_TEMP_ERROR_STATE_EVENT;
    }else{
        x__text_ptr(parser);
        *event = CXML_SAX_TEXT_EVENT;
    }
}

static void
_cxml_sax_cdata_event(_cxml_parser *parser, cxml_sax_event_t *event){
    if (!parser->root_element
        && !_cxml_p_is_whitespace_ptr(parser->current_tok))
    {
        // cdata should not come before root element
        _cxml_p_advance_ptr(parser);
        *event = CXML_SAX_TEMP_ERROR_STATE_EVENT;
    }else if (parser->is_root_wrapped){
        // cdata should not come after root node is wrapped.
        *event = CXML_SAX_TEMP_ERROR_STATE_EVENT;
    }else{
        x__cdata_sect_ptr(parser);
        *event = CXML_SAX_CDATA_EVENT;
    }
}

static void
_cxml_sax_pi_event(_cxml_parser *parser, cxml_sax_event_t *event){
    x__pi_ptr(parser);
    *event = CXML_SAX_PROCESSING_INSTRUCTION_EVENT;
}


static void
_cxml_sax_xml_hdr_or_pi_event(_cxml_parser *parser, cxml_sax_event_t *event){
  // if we already have an xml header, then it's a processing-instruction
  if (parser->has_header){
      _cxml_sax_pi_event(parser, event);
  }else{
      // if not, try obtaining header,
      x__xml_decl_ptr(parser);
      // if header now exists, then we're guaranteed that it's
      // an xml header event.
      if (parser->has_header){
          *event = CXML_SAX_XML_HEADER_EVENT;
      }else{
          // if not, then x__xml_decl() must have invoke the function for a
          // processing instruction node (see x__xml_decl() definition for insight)
          *event = CXML_SAX_PROCESSING_INSTRUCTION_EVENT;
      }
  }
}

static void
_cxml_sax_comment_event(_cxml_parser *parser, cxml_sax_event_t *event){

    x__comment_ptr(parser);
    *event = CXML_SAX_COMMENT_EVENT;
}

static void
_cxml_sax_dtd_event(_cxml_parser *parser, cxml_sax_event_t *event){
    x__dtd_ptr(parser);
    *event = CXML_SAX_DTD_EVENT;
}

static void
_cxml_sax_document_start_event(_cxml_parser *parser, cxml_sax_event_t *event){
    _cxml_p_advance_ptr(parser);
    // create root node and register it to parser
    parser->root_node = create_root_node();
    _cxml_stack__push(&parser->_cx_stack, parser->root_node);
    _cxml_p_inject_global_namespaces_ptr(parser);
    *event = CXML_SAX_BEGIN_DOCUMENT_EVENT;
}

static void
_cxml_sax_document_end_event(
        cxml_sax_event_reader *reader,
        cxml_sax_event_t *event, bool suppress)
{
    if (!reader->xml_parser->is_root_wrapped && !suppress){
        cxml_error("CXML SAX Error: Expected element `%s` closing tag\n",
                   cxml_string_as_raw(&reader->xml_parser->root_node->name));
    }
    _cxml_p_show_errors_ptr(reader->xml_parser);
    reader->is_well_formed = cxml_list_is_empty(&reader->xml_parser->errors);
    cxml_root_node_free(reader->xml_parser->root_node);
    *event = CXML_SAX_END_DOCUMENT_EVENT;
    reader->prev_event = reader->curr_event;
    // CXML_SAX_END_DOCUMENT_EVENT is auto consumed on eof
    reader->curr_event = (cxml_sax_event){.type = CXML_SAX_END_DOCUMENT_EVENT, .is_consumed = true};
    // pop the root node off the stack
    _cxml_stack__pop(&reader->xml_parser->_cx_stack);
}


/*********************************
 *                               *
 *          Helpers              *
 *********************************
 */
static void _clone_namespaces(cxml_list *namespaces, cxml_list *clones){
    cxml_ns_node *ns;
    cxml_for_each(namespace, namespaces)
    {
        ns = ALLOC(cxml_ns_node, 1);
        cxml_ns_node_init(ns);
        _unwrap__cxnode(ns, namespace)->is_default ? (void)0 :
        cxml_string_dcopy(&ns->prefix, &_unwrap__cxnode(ns, namespace)->prefix);
        cxml_string_dcopy(&ns->uri, &_unwrap__cxnode(ns, namespace)->uri);
        ns->is_default = _unwrap__cxnode(ns, namespace)->is_default;
        ns->is_global = _unwrap__cxnode(ns, namespace)->is_global;
        ns->pos = _unwrap__cxnode(ns, namespace)->pos;
        // no need to associate the parent here
        cxml_list_append(clones, ns);
    }
}


/********event consumers************/
/*
 * we define 2 interfaces, namely:
 * - get_x_data() :: returns only the node values
 * - as_x_node():: returns the node object itself.
 */

/*********************************
 *                               *
 *          Object getters       *
 *********************************
 */
static void*
cxml_sax_as_x_node(cxml_sax_event_reader *reader, cxml_sax_event_t event_type){
    if ((reader->curr_event.type == event_type)
        && !reader->curr_event.is_consumed)
    {
        // delete object, i.e. disassociate it from its family,
        // then return it's value
        void *parent = _cxml_stack__get(&reader->xml_parser->_cx_stack);
        cxml_list *children = _cxml__get_node_children(parent);
        // child/object is last added item in children list
        void *object = cxml_list_safe_delete(children, true);
        // terminate parent relationship
        _cxml_unset_parent(object);
        // flag consumed event
        reader->curr_event.is_consumed = true;
        return object;
    }
    return NULL;
}

cxml_comm_node * cxml_sax_as_comment_node(
        cxml_sax_event_reader *reader)
{
    if (!reader || _is_closed_r(reader)) return NULL;
    return cxml_sax_as_x_node(reader, CXML_SAX_COMMENT_EVENT);
}

cxml_pi_node * cxml_sax_as_pi_node(
        cxml_sax_event_reader *reader)
{
    if (!reader || _is_closed_r(reader)) return NULL;
    return cxml_sax_as_x_node(reader, CXML_SAX_PROCESSING_INSTRUCTION_EVENT);
}

cxml_text_node * cxml_sax_as_text_node(
        cxml_sax_event_reader *reader)
{
    if (!reader || _is_closed_r(reader)) return NULL;
    return cxml_sax_as_x_node(reader, CXML_SAX_TEXT_EVENT);
}

cxml_text_node * cxml_sax_as_cdsect_node(cxml_sax_event_reader *reader){
    if (!reader || _is_closed_r(reader)) return NULL;
    return cxml_sax_as_x_node(reader, CXML_SAX_CDATA_EVENT);
}

cxml_attr_node* cxml_sax_as_attribute_node(
        cxml_sax_event_reader *reader,
        const char* key)
{
    if (!reader || !key || _is_closed_r(reader)) return NULL;
    if ((reader->curr_event.type == CXML_SAX_ATTRIBUTE_EVENT
        || reader->curr_event.type == CXML_SAX_NAMESPACE_ATTRIBUTE_EVENT)
        && !reader->curr_event.is_consumed)
    {
        cxml_elem_node *elem = _cxml_stack__get(&reader->xml_parser->_cx_stack);
        cxml_attr_node *attr = NULL;
        if (elem->has_attribute){
            attr = cxml_table_get(elem->attributes, key);
            if (attr){
                // remove the attr node from the table, it's up to the user
                // to manage the object from here.
                cxml_table_remove(elem->attributes, key);
                attr->parent = NULL;
            }
        }
        // we don't set reader's `is_consumed` flag here because the user may
        // need to call cxml_sax_as_attribute_node() more than once, with
        // different keys each time. This would work as long as the current
        // event is still an attribute/namespace-attribute event.
        return attr;
    }
    return NULL;
}

void cxml_sax_as_attribute_list(cxml_sax_event_reader *reader, cxml_list *attrs){
    if (!reader || !attrs || _is_closed_r(reader)) return;
    if ((reader->curr_event.type == CXML_SAX_ATTRIBUTE_EVENT
        || reader->curr_event.type == CXML_SAX_NAMESPACE_ATTRIBUTE_EVENT)
        && !reader->curr_event.is_consumed)
    {
        cxml_elem_node *elem = _cxml_stack__get(&reader->xml_parser->_cx_stack);
        if (elem->has_attribute){
            cxml_attr_node *attr;
            cxml_for_each(key, &elem->attributes->keys)
            {
                attr = cxml_table_get(elem->attributes, key);
                attr->parent = NULL;
                cxml_list_append(attrs, attr);
            }
            // free the table. The cxml_attr_node objects stored in `attributes` list
            // will be managed by the user. That is, it is up to the user to free the
            // objects when done using them.
            cxml_table_free(elem->attributes);
            elem->has_attribute = false;
        }
        // flag consumed event
        reader->curr_event.is_consumed = true;
    }
}


void cxml_sax_as_namespace_list(cxml_sax_event_reader *reader, cxml_list *namespaces) {
    if (!reader || !namespaces || _is_closed_r(reader)) return;
    if ((reader->curr_event.type == CXML_SAX_NAMESPACE_EVENT
        || reader->curr_event.type == CXML_SAX_NAMESPACE_ATTRIBUTE_EVENT)
        && !reader->curr_event.is_consumed)
    {
        cxml_elem_node *elem = _cxml_stack__get(&reader->xml_parser->_cx_stack);
        if (elem->namespaces){
            // once namespace list is returned to the user and the namespaces are freed,
            // we no longer have reference to the namespace which becomes an issue when
            // trying to resolve the namespace for a later element or attribute later on.
            _clone_namespaces(elem->namespaces, namespaces);
        }
        // flag consumed event
        reader->curr_event.is_consumed = true;
    }
}

cxml_dtd_node * cxml_sax_as_dtd_node(cxml_sax_event_reader *reader){
    if (!reader || _is_closed_r(reader)) return NULL;
    return cxml_sax_as_x_node(reader, CXML_SAX_DTD_EVENT);
}

cxml_xhdr_node* cxml_sax_as_xml_hdr_node(cxml_sax_event_reader *reader){
    if (!reader || _is_closed_r(reader)) return NULL;
    void * hdr = cxml_sax_as_x_node(reader, CXML_SAX_XML_HEADER_EVENT);
    if (hdr){
        cxml__assert(
                hdr == reader->xml_parser->xml_header,
                "CXML SAX Internal Error: XML header available but not found."
                )
    }
    reader->xml_parser->xml_header = NULL;
    return hdr;
}


/*********************************
 *                               *
 *          Data getters         *
 *********************************
 */

void cxml_sax_get_element_name(cxml_sax_event_reader *reader, cxml_string *name){
    /*
     * returns a copy of the name of the element.
     */
    if (!reader || !name) return;
    if ((reader->curr_event.type == CXML_SAX_BEGIN_ELEMENT_EVENT)
        && !reader->curr_event.is_consumed)
    {
        cxml_elem_node *elem = _cxml_stack__get(&reader->xml_parser->_cx_stack);
        // copy name, because we need name when performing element wrapping
        cxml_string_dcopy(name, &elem->name.qname);
        // flag consumed event
        reader->curr_event.is_consumed = true;
    }
}

void cxml_sax_get_comment_data(cxml_sax_event_reader *reader, cxml_string *comm_str){
    if (!reader || !comm_str) return;
    cxml_comm_node *comment = cxml_sax_as_comment_node(reader);
    if (comment){
        cxml_string_str_append(comm_str, &comment->value);
        cxml_comm_node_free(comment);
    }
}

void cxml_sax_get_text_data(cxml_sax_event_reader *reader, cxml_string *text_str){
    if (!reader || !text_str) return;
    cxml_text_node *text = cxml_sax_as_text_node(reader);
    if (text){
        cxml_string_str_append(text_str, &text->value);
        cxml_text_node_free(text);
    }
}

void cxml_sax_get_pi_data(cxml_sax_event_reader *reader,
                          const char *target, cxml_string *value)
{
    if (!reader || !value) return;
    cxml_pi_node *proc = cxml_sax_as_pi_node(reader);
    if (proc){
        if (target){
            if (cxml_string_cmp_raw_equals(&proc->target, target)){
                goto get_val;
            }
            return;
        }
        get_val:
        cxml_string_str_append(value, &proc->value);
        cxml_pi_node_free(proc);
    }
}

void cxml_sax_get_attribute_data(cxml_sax_event_reader *reader,
                                 const char *key, cxml_string *value)
{
    if (!reader || !value) return;
    cxml_attr_node *attr = cxml_sax_as_attribute_node(reader, key);
    if (attr){
        cxml_string_str_append(value, &attr->value);
        cxml_attr_node_free(attr);
    }
}

void cxml_sax_get_cdsect_data(cxml_sax_event_reader *reader, cxml_string *cdata){
    if (!reader || !cdata) return;
    cxml_text_node *text = cxml_sax_as_cdsect_node(reader);
    if (text){
        cxml_string_str_append(cdata, &text->value);
        cxml_text_node_free(text);
    }
}

int cxml_sax_is_well_formed(cxml_sax_event_reader *reader){
    if (!reader) return 0;
    if (reader->curr_event.type == CXML_SAX_END_DOCUMENT_EVENT
        && reader->curr_event.is_consumed
        && _is_closed_r(reader))
    {
        return reader->is_well_formed;
    }
    return -1;
}


/*********************************
 *                               *
 *          Event pullers        *
 *********************************
 */

bool cxml_sax_has_event(cxml_sax_event_reader *reader){
    if (!reader || _is_closed_r(reader)) return false;
    return ((reader->curr_event.type != CXML_SAX_END_DOCUMENT_EVENT
            || (reader->curr_event.type == CXML_SAX_END_DOCUMENT_EVENT
            && !reader->curr_event.is_consumed))
         && (reader->curr_event.type != CXML_SAX_NIL_EVENT));
}

cxml_sax_event_t (*cxml_sax_next_event)(cxml_sax_event_reader *reader) = cxml_sax_get_event;


cxml_sax_event_t cxml_sax_get_event(cxml_sax_event_reader *reader){
    if (!reader) return CXML_SAX_NIL_EVENT;
    if (reader->curr_event.type == CXML_SAX_END_DOCUMENT_EVENT){
        return CXML_SAX_END_DOCUMENT_EVENT;
    }

    cxml_sax_event_t event = CXML_SAX_NIL_EVENT;

    start:
    if (reader->xml_parser->root_node == NULL){
        _cxml_sax_document_start_event(reader->xml_parser, &event);
        reader->curr_event.type = event;
        // CXML_SAX_BEGIN_DOCUMENT_EVENT is auto consumed on start
        reader->curr_event.is_consumed = true;
        return event;
    }

    _cxml_parser *parser = reader->xml_parser;

    switch(parser->current_tok.type)
    {
        case CXML_TOKEN_L_THAN:  // elem_start, xml hdr, proc_inst
        {
            _cxml_p_consume_ptr(parser, CXML_TOKEN_L_THAN);
            switch (parser->current_tok.type)
            {
                case CXML_TOKEN_IDENTIFIER:  // elem_start
                    _cxml_sax_element_start_event(parser, &event);
                    break;
                case CXML_TOKEN_Q_MARK:   // xml hdr || proc_inst
                    _cxml_sax_xml_hdr_or_pi_event(parser, &event);
                    break;
                case CXML_TOKEN_F_SLASH:  // elem_end -> wrap_tag()
                    _cxml_sax_element_end_event(parser, &event);
                    break;
                case CXML_TOKEN_ERROR:
                    cxml_error("%.*s\n", parser->current_tok.length,
                               parser->current_tok.start);
                default:
                    _cxml_sax_expect_or_err(
                            parser, "Expected identifier, '?' or '/'",
                            1, CXML_TOKEN_IDENTIFIER);
            }
            break;
        }
        case CXML_TOKEN_Q_MARK:
            _cxml_sax_xml_hdr_or_pi_event(parser, &event);
            break;
        case CXML_TOKEN_IDENTIFIER:
            _cxml_sax_attr_event(parser, &event);
            break;
        case CXML_TOKEN_TEXT:
            _cxml_sax_text_event(parser, &event);
            break;
        case CXML_TOKEN_F_SLASH:
            _cxml_sax_element_end_event(parser, &event);
            break;
        case CXML_TOKEN_COMMENT:  // direct call to comment
            _cxml_sax_comment_event(parser, &event);
            break;
        case CXML_TOKEN_DOCTYPE:
            _cxml_sax_dtd_event(parser, &event);
            break;
        case CXML_TOKEN_CDATA:  // cdata
            _cxml_sax_cdata_event(parser, &event);
            break;
        case CXML_TOKEN_EOF:
            _cxml_sax_document_end_event(reader, &event, false);
            if (reader->auto_close){
                cxml_sax_close_event_reader(reader);
            }
            return event;
        default:
            cxml_error("CXML SAX Error: Error at line %d.\n"
                       "Unexpected token found",
                       parser->cxlexer.line);
    }
    // if a recoverable error occurs, try to get next event.
    if (event == CXML_SAX_TEMP_ERROR_STATE_EVENT){
        _cxml_p_advance_ptr(parser);
        goto start;
    }
    reader->prev_event = reader->curr_event;
    reader->curr_event = _cxml_sax_new_event(&event);
    return event;
}


cxml_sax_event_reader cxml_stream_file(const char *fn, bool auto_close) {
    return cxml_sax_init(fn, auto_close);
}
