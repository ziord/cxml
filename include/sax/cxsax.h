/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXSAX_H
#define CXML_CXSAX_H

#include "xml/cxparser.h"


/*
 * event types
 */
typedef enum _cxml_sax_event_t{
    /* initialization event */
    CXML_SAX_INIT_STATE_EVENT,

    /* error state event */
    // temporary state means full event is yet to be decided:
    // this happens when we run into a recoverable error
    CXML_SAX_TEMP_ERROR_STATE_EVENT,

    /* main/consumable event types */
    CXML_SAX_NIL_EVENT,
    // only attributes were encountered
    CXML_SAX_ATTRIBUTE_EVENT,
    // only namespaces were encountered
    CXML_SAX_NAMESPACE_EVENT,
    // attributes and namespaces were encountered
    CXML_SAX_NAMESPACE_ATTRIBUTE_EVENT,
    CXML_SAX_CDATA_EVENT,
    CXML_SAX_COMMENT_EVENT,
    CXML_SAX_DTD_EVENT,
    CXML_SAX_PROCESSING_INSTRUCTION_EVENT,
    CXML_SAX_XML_HEADER_EVENT,
    CXML_SAX_TEXT_EVENT,
    CXML_SAX_BEGIN_DOCUMENT_EVENT,
    CXML_SAX_END_DOCUMENT_EVENT,
    CXML_SAX_BEGIN_ELEMENT_EVENT,
    CXML_SAX_END_ELEMENT_EVENT
} cxml_sax_event_t;

typedef struct{
    // keep track of the "consumed" state of the event
    bool is_consumed;
    // the event type
    cxml_sax_event_t type;
}cxml_sax_event;

typedef struct {
    bool auto_close;
    bool is_well_formed;
    uint32_t is_open;
    cxml_sax_event prev_event;
    cxml_sax_event curr_event;
    _cxml_parser *xml_parser;
} cxml_sax_event_reader;

void cxml_sax_open_event_reader(
        cxml_sax_event_reader* reader,
        const char* file_name, bool auto_close);

void cxml_sax_close_event_reader(cxml_sax_event_reader* reader);

cxml_sax_event_reader cxml_sax_init(const char *file_name, bool auto_close);

bool cxml_sax_has_event(cxml_sax_event_reader *reader);

extern cxml_sax_event_t (*cxml_sax_next_event)(cxml_sax_event_reader *reader);

cxml_sax_event_t cxml_sax_get_event(cxml_sax_event_reader *reader);

int cxml_sax_is_well_formed(cxml_sax_event_reader *reader);

cxml_sax_event_reader cxml_stream_file(const char *fn, bool auto_close);


/****object getters*****/

cxml_comment_node * cxml_sax_as_comment_node(cxml_sax_event_reader *reader);

cxml_pi_node * cxml_sax_as_pi_node(cxml_sax_event_reader *reader);

cxml_text_node * cxml_sax_as_text_node(cxml_sax_event_reader *reader);

cxml_text_node * cxml_sax_as_cdsect_node(cxml_sax_event_reader *reader);

void cxml_sax_as_namespace_list(cxml_sax_event_reader *reader, cxml_list *namespaces);

void cxml_sax_as_attribute_list(cxml_sax_event_reader *reader, cxml_list *attrs);

cxml_attribute_node* cxml_sax_as_attribute_node(cxml_sax_event_reader *reader, const char* key);

cxml_dtd_node * cxml_sax_as_dtd_node(cxml_sax_event_reader *reader);

cxml_xhdr_node* cxml_sax_as_xml_hdr_node(cxml_sax_event_reader *reader);


/****data getters*****/

void cxml_sax_get_element_name(cxml_sax_event_reader *reader, cxml_string *name);

void cxml_sax_get_comment_data(cxml_sax_event_reader *reader, cxml_string *comm_str);

void cxml_sax_get_text_data(cxml_sax_event_reader *reader, cxml_string *text_str);

void cxml_sax_get_cdsect_data(cxml_sax_event_reader *reader, cxml_string *cdata);

void cxml_sax_get_pi_data(cxml_sax_event_reader *reader,
                          const char *target, cxml_string *value);

void cxml_sax_get_attribute_data(cxml_sax_event_reader *reader,
                                 const char *key, cxml_string *value);

#endif //CXML_CXSAX_H
