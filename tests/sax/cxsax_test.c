/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include <utils/cxutf8hook.h>
#include "cxfixture.h"

int reader_init_asserts(cxml_sax_event_reader *reader, bool auto_close){
    CHECK_EQ(reader->auto_close, auto_close)
    CHECK_EQ(reader->prev_event.type, CXML_SAX_INIT_STATE_EVENT)
    CHECK_EQ(reader->curr_event.type, CXML_SAX_INIT_STATE_EVENT)
    CHECK_FALSE(reader->curr_event.is_consumed)
    CHECK_EQ(reader->xml_parser->current_tok.literal_type, CXML_NON_LITERAL)
    CHECK_EQ(reader->xml_parser->prev_tok.literal_type, CXML_NON_LITERAL)
    CHECK_EQ(reader->xml_parser->xml_header, NULL)
    CHECK_EQ(reader->xml_parser->xml_doctype, NULL)
    CHECK_EQ(reader->xml_parser->root_element, NULL)
    CHECK_EQ(reader->xml_parser->root_node, NULL)
    CHECK_EQ(reader->xml_parser->err_msg, NULL)
    CHECK_EQ(reader->xml_parser->current_scope, NULL)
    CHECK_FALSE(reader->xml_parser->has_header)
    CHECK_FALSE(reader->xml_parser->has_dtd)
    CHECK_FALSE(reader->xml_parser->is_root_wrapped)
    CHECK_FALSE(reader->xml_parser->is_root_wrapped)
    CHECK_EQ(reader->xml_parser->pos_c, 0)
    return 1;
}
cts test_cxml_sax_init(){
    char *fp = get_file_path("foo.xml");
    cxml_sax_event_reader reader = cxml_sax_init(fp, true);
    FREE(fp);
    CHECK(reader_init_asserts(&reader, true))
    cxml_pass()
}

cxml_sax_event_reader get_event_reader(char *file_name, bool auto_close){
    char *fp = get_file_path(file_name);
    cxml_sax_event_reader reader = cxml_stream_file(fp, auto_close);
    FREE(fp);
    return reader;
}

cts test_cxml_sax_has_event(){
    deb()
    cxml_sax_event_reader reader = get_event_reader("wf_xml_1.xml", true);
    // at the initialization state, cxml_sax_has_event() will always report true
    CHECK_TRUE(cxml_sax_has_event(&reader))
    CHECK_FALSE(cxml_sax_has_event(NULL))
    // trigger an actual event
    cxml_sax_next_event(&reader);
    // there should be more events.
    CHECK_TRUE(cxml_sax_has_event(&reader))
    cxml_sax_close_event_reader(&reader);
    cxml_pass()
}

cts test_cxml_sax_get_event(){
    deb()
    cxml_sax_event_reader reader = get_event_reader("wf_xml_1.xml", true);
    CHECK_EQ(cxml_sax_get_event(&reader), CXML_SAX_BEGIN_DOCUMENT_EVENT)
    CHECK_EQ(cxml_sax_get_event(&reader), CXML_SAX_XML_HEADER_EVENT)
    // we're under the default configuration of preserving space
    // between items in the doc, hence the next event will be a text event
    CHECK_EQ(cxml_sax_get_event(&reader), CXML_SAX_TEXT_EVENT)
    CHECK_EQ(cxml_sax_get_event(&reader), CXML_SAX_BEGIN_ELEMENT_EVENT)
    CHECK_EQ(cxml_sax_get_event(NULL), CXML_SAX_NIL_EVENT)
    cxml_sax_close_event_reader(&reader);
    cxml_pass()
}

cts test_cxml_sax_is_well_formed(){
    deb()
    cxml_sax_event_reader reader = get_event_reader("wf_xml_1.xml", true);
    // we can't tell if the document is well formed until the entire
    // document has been consumed
    CHECK_EQ(cxml_sax_is_well_formed(&reader), -1)
    CHECK_EQ(cxml_sax_is_well_formed(NULL), 0)
    while (cxml_sax_has_event(&reader)){
        cxml_sax_get_event(&reader);
    }
    CHECK_EQ(reader.curr_event.type, CXML_SAX_END_DOCUMENT_EVENT)
    CHECK_NE(reader.is_open, 1)
    CHECK_TRUE(cxml_sax_is_well_formed(&reader))
    // event reader is automatically close when all events has been consumed
    cxml_pass()
}

cts test_cxml_stream_file(){
    char *fp = get_file_path("foo.xml");
    cxml_sax_event_reader reader = cxml_stream_file(fp, false);
    FREE(fp);
    CHECK(reader_init_asserts(&reader, false))
    while (cxml_sax_has_event(&reader)){
        cxml_sax_get_event(&reader);
    }
    CHECK_EQ(reader.curr_event.type, CXML_SAX_END_DOCUMENT_EVENT)
    CHECK_EQ(reader.is_open, 1)
    cxml_sax_close_event_reader(&reader);
    CHECK_NE(reader.is_open, 1)
    cxml_pass()
}


/****object getters*****/


cts test_cxml_sax_as_comment_node(){
    cxml_sax_event_reader reader = get_event_reader("wf_xml_3.xml", true);
    cxml_comment_node *comment;
    cxml_list comments = new_cxml_list();
    while (cxml_sax_has_event(&reader))
    {
        if (cxml_sax_get_event(&reader) == CXML_SAX_COMMENT_EVENT){
            cxml_list_append(&comments, cxml_sax_as_comment_node(&reader));
        }
    }
    CHECK_EQ(cxml_list_size(&comments), 2)
    char *values[] = {"foobar", "cimple"};
    int i = 0;
    cxml_for_each(com, &comments){
        comment = com;
        CHECK_TRUE(cxml_string_raw_equals(&comment->value, values[i]))
        cxml_free_comment_node(comment);
        i++;
    }
    cxml_list_free(&comments);
    CHECK_EQ(cxml_sax_as_comment_node(&reader), NULL)
    CHECK_EQ(cxml_sax_as_comment_node(NULL), NULL)
    cxml_pass()
}

cts test_cxml_sax_as_pi_node(){
    cxml_sax_event_reader reader = get_event_reader("foo.xml", true);
    cxml_pi_node *pi;
    cxml_list pis = new_cxml_list();
    while (cxml_sax_has_event(&reader))
    {
        if (cxml_sax_get_event(&reader) == CXML_SAX_PROCESSING_INSTRUCTION_EVENT){
            cxml_list_append(&pis, cxml_sax_as_pi_node(&reader));
        }
    }
    CHECK_EQ(cxml_list_size(&pis), 3)
    char *values[] = {"my-proc ", "xml stuff", "python thingy"};
    char *targets[] = {"processor", "ext", "language"};
    int i = 0;
    cxml_for_each(n, &pis){
        pi = n;
        CHECK_TRUE(cxml_string_raw_equals(&pi->target, targets[i]))
        CHECK_TRUE(cxml_string_raw_equals(&pi->value, values[i]))
        cxml_free_pi_node(pi);
        i++;
    }
    cxml_list_free(&pis);
    CHECK_EQ(cxml_sax_as_pi_node(&reader), NULL)
    CHECK_EQ(cxml_sax_as_pi_node(NULL), NULL)
    cxml_pass()
}

cts test_cxml_sax_as_text_node(){
    cxml_config cfg = cxml_get_config();
    cxml_cfg_preserve_space(0);
    cxml_sax_event_reader reader = get_event_reader("wf_xml_3.xml", true);
    cxml_text_node *text;
    cxml_list texts = new_cxml_list();
    while (cxml_sax_has_event(&reader))
    {
        if (cxml_sax_get_event(&reader) == CXML_SAX_TEXT_EVENT){
            cxml_list_append(&texts, cxml_sax_as_text_node(&reader));
        }
    }
    CHECK_EQ(cxml_list_size(&texts), 3)
    char *values[] = {"computers", "hardware", "NT"};

    int i = 0;
    cxml_for_each(n, &texts){
        text = n;
        CHECK_TRUE(cxml_string_raw_equals(&text->value, values[i]))
        cxml_free_text_node(text);
        i++;
    }
    cxml_list_free(&texts);
    cxml_set_config(cfg);
    CHECK_EQ(cxml_sax_as_text_node(&reader), NULL)
    CHECK_EQ(cxml_sax_as_text_node(NULL), NULL)
    cxml_pass()
}

cts test_cxml_sax_as_cdsect_node(){
    cxml_config cfg = cxml_get_config();
    cxml_cfg_preserve_space(0);
    cxml_sax_event_reader reader = get_event_reader("cdata.xml", true);
    cxml_text_node *cdata;
    cxml_list cdatas = new_cxml_list();
    while (cxml_sax_has_event(&reader))
    {
        if (cxml_sax_get_event(&reader) == CXML_SAX_CDATA_EVENT){
            cxml_list_append(&cdatas, cxml_sax_as_cdsect_node(&reader));
        }
    }
    CHECK_EQ(cxml_list_size(&cdatas), 3)
    char *values[] = {"Testing cdata stuff",
                      "\"just testing this & that />",
                      "for(int i=0; i<10; i++){printf(\"foobar!\");}"};

    int i = 0;
    cxml_for_each(n, &cdatas){
        cdata = n;
        CHECK_TRUE(cxml_string_raw_equals(&cdata->value, values[i]))
        CHECK_TRUE(cdata->is_cdata)
        cxml_free_text_node(cdata);
        i++;
    }
    cxml_list_free(&cdatas);
    cxml_set_config(cfg);
    CHECK_EQ(cxml_sax_as_cdsect_node(&reader)) // reader is close, NULLd
    CHECK_EQ(cxml_sax_as_cdsect_node(NULL), NULL)
    cxml_pass()
}

cts test_cxml_sax_as_namespace_list(){
    cxml_sax_event_reader reader = get_event_reader("wf_xml_4.xml", false);
    cxml_list namespaces = new_cxml_list();

    while (cxml_sax_has_event(&reader))
    {
        // an attribute event is only delivered after all the attributes have been parsed
        if (cxml_sax_next_event(&reader) == CXML_SAX_NAMESPACE_EVENT){
            // obtain all the attributes found in the element
            cxml_sax_as_namespace_list(&reader, &namespaces);
            break;
        }
    }
    // we need to close the reader
    // 1. because we didn't specify `auto_close` as true
    // 2. because we didn't wait till all the events were consumed.
    cxml_sax_close_event_reader(&reader);
    CHECK_EQ(cxml_list_size(&namespaces), 1)

    cxml_ns_node *ns = cxml_list_get(&namespaces, 0);
    CHECK_TRUE(cxml_string_raw_equals(&ns->prefix, "f"))
    CHECK_TRUE(cxml_string_raw_equals(&ns->uri, "https://www.w3schools.com/furniture"))
    cxml_free_namespace_node(ns);
    cxml_list_free(&namespaces);
    cxml_sax_as_namespace_list(NULL, &namespaces);
    cxml_sax_as_namespace_list(&reader, NULL);
    cxml_sax_as_namespace_list(&reader, &namespaces);
    CHECK_EQ(cxml_list_size(&namespaces), 0)
    cxml_pass()
}

cts test_cxml_sax_as_attribute_list(){
    cxml_sax_event_reader reader = get_event_reader("cdata.xml", false);
    cxml_attr_node *attr;
    cxml_list attrs = new_cxml_list();
    char *keys[] = {"mod", "van", "hex"};
    char *values[] = {"63", "'40'", "0xdeadbeef"};
    double num[] = {63, 0, 0xdeadbeef};

    int i = 0;
    while (cxml_sax_has_event(&reader))
    {
        // an attribute event is only delivered after all the attributes have been parsed
        if (cxml_sax_get_event(&reader) == CXML_SAX_ATTRIBUTE_EVENT){
            // obtain all the attributes found in the element
            cxml_sax_as_attribute_list(&reader, &attrs);
            break;
        }
    }
    // we need to close the reader
    // 1. because we didn't specify `auto_close` as true
    // 2. because we didn't wait till all the events were consumed.
    cxml_sax_close_event_reader(&reader);
    CHECK_EQ(cxml_list_size(&attrs), 3)

    i = 0;
    cxml_for_each(n, &attrs){
        attr = n;
        CHECK_TRUE(cxml_string_raw_equals(&attr->name.qname, keys[i]))
        CHECK_TRUE(cxml_string_raw_equals(&attr->value, values[i]))
        CHECK_EQ(attr->number_value.dec_val, num[i])
        cxml_free_attribute_node(attr);
        i++;
    }
    cxml_list_free(&attrs);
    CHECK_EQ(cxml_sax_as_attribute_node(NULL, keys[0]), NULL)
    CHECK_EQ(cxml_sax_as_attribute_node(&reader, NULL), NULL)
    CHECK_EQ(cxml_sax_as_attribute_node(&reader, keys[i]), NULL)
    cxml_pass()
}

cts test_cxml_sax_as_attribute_node(){
    cxml_sax_event_reader reader = get_event_reader("cdata.xml", false);
    cxml_attr_node *attr;
    cxml_list attrs = new_cxml_list();
    char *keys[] = {"mod", "van", "hex"};
    char *values[] = {"63", "'40'", "0xdeadbeef"};
    double num[] = {63, 0, 0xdeadbeef};

    int i = 0;
    while (cxml_sax_has_event(&reader))
    {
        // an attribute event is only delivered after all the attributes have been parsed
        if (cxml_sax_next_event(&reader) == CXML_SAX_ATTRIBUTE_EVENT){
            // obtain all the attributes found in the element
            for (; i<3; i++){
                cxml_list_append(&attrs, cxml_sax_as_attribute_node(&reader, keys[i]));
            }
            break;
        }
    }
    // we need to close the reader
    // 1. because we didn't specify `auto_close` as true
    // 2. because we didn't wait till all the events were consumed.
    cxml_sax_close_event_reader(&reader);
    CHECK_EQ(cxml_list_size(&attrs), 3)

    i = 0;
    cxml_for_each(n, &attrs){
        attr = n;
        CHECK_TRUE(cxml_string_raw_equals(&attr->name.qname, keys[i]))
        CHECK_TRUE(cxml_string_raw_equals(&attr->value, values[i]))
        CHECK_EQ(attr->number_value.dec_val, num[i])
        cxml_free_attribute_node(attr);
        i++;
    }
    cxml_list_free(&attrs);
    CHECK_EQ(cxml_sax_as_attribute_node(NULL, keys[0]), NULL)
    CHECK_EQ(cxml_sax_as_attribute_node(&reader, NULL), NULL)
    CHECK_EQ(cxml_sax_as_attribute_node(&reader, keys[i]), NULL)
    cxml_pass()
}

cts test_cxml_sax_as_dtd_node(){
    cxml_sax_event_reader reader = get_event_reader("wf_xml_5.xml", false);
    cxml_dtd_node *dtd = NULL;
    while (cxml_sax_has_event(&reader))
    {
        if (cxml_sax_get_event(&reader) == CXML_SAX_DTD_EVENT){
            dtd = cxml_sax_as_dtd_node(&reader);
            break;
        }
    }
    cxml_sax_close_event_reader(&reader);
    CHECK_NE(dtd, NULL)
    CHECK_TRUE(cxml_string_raw_equals(&dtd->value, "<!DOCTYPE person>"))
    CHECK_EQ(cxml_sax_as_dtd_node(NULL), NULL)
    cxml_free_dtd_node(dtd);
    cxml_pass()
}

cts test_cxml_sax_as_xml_hdr_node(){
    cxml_sax_event_reader reader = get_event_reader("wf_xml_1.xml", false);
    cxml_xhdr_node *xh = NULL;

    while (cxml_sax_has_event(&reader))
    {
        if (cxml_sax_get_event(&reader) == CXML_SAX_XML_HEADER_EVENT){
            xh = cxml_sax_as_xml_hdr_node(&reader);
            break;
        }
    }
    cxml_sax_close_event_reader(&reader);
    CHECK_NE(xh, NULL)
    char *values[] = {"1.0", "UTF-8"};
    char *keys[] = {"version", "encoding"};
    cxml_attribute_node *v;
    for (int i=0; i<2; i++){
        v = cxml_table_get(&xh->attributes, keys[i]);
        CHECK_NE(v, NULL)
        CHECK_TRUE(cxml_string_raw_equals(&v->name.qname, keys[i]))
        CHECK_TRUE(cxml_string_lraw_equals(
                &v->value,
                values[i],
                strlen(values[i])))
    }
    cxml_free_xhdr_node(xh);
    CHECK_EQ(cxml_sax_as_xml_hdr_node(NULL), NULL)
    cxml_pass()
}


/****data getters*****/

cts test_cxml_sax_get_element_name(){
    cxml_sax_event_reader reader = get_event_reader("wf_xml_1.xml", false);
    cxml_string name = new_cxml_string();

    while (cxml_sax_has_event(&reader))
    {
        if (cxml_sax_get_event(&reader) == CXML_SAX_BEGIN_ELEMENT_EVENT){
            cxml_sax_get_element_name(&reader, &name);
            break;
        }
    }
    cxml_sax_close_event_reader(&reader);
    CHECK_TRUE(cxml_string_raw_equals(&name, "abc"))
    cxml_string_free(&name);

    cxml_sax_get_element_name(&reader, &name);
    CHECK_EQ(cxml_string_len(&name), 0)

    cxml_sax_get_element_name(&reader, NULL);
    cxml_sax_get_element_name(NULL, &name);
    CHECK_EQ(cxml_string_len(&name), 0)

    cxml_pass()
}

cts test_cxml_sax_get_comment_data(){
    cxml_sax_event_reader reader = get_event_reader("foo.xml", false);
    cxml_string data = new_cxml_string();

    while (cxml_sax_has_event(&reader))
    {
        if (cxml_sax_get_event(&reader) == CXML_SAX_COMMENT_EVENT){
            cxml_sax_get_comment_data(&reader, &data);
            break;
        }
    }
    cxml_sax_close_event_reader(&reader);
    CHECK_TRUE(cxml_string_raw_equals(&data,
            "Recent updates to the Python Package Index for switches.py"))
    cxml_string_free(&data);

    cxml_sax_get_comment_data(&reader, &data);
    CHECK_EQ(cxml_string_len(&data), 0)

    cxml_sax_get_comment_data(&reader, NULL);
    cxml_sax_get_comment_data(NULL, &data);
    CHECK_EQ(cxml_string_len(&data), 0)

    cxml_pass()
}

cts test_cxml_sax_get_text_data(){
    cxml_config cfg = cxml_get_config();
    cxml_cfg_preserve_space(0);
    cxml_sax_event_reader reader = get_event_reader("foo.xml", false);
    cxml_string data = new_cxml_string();

    while (cxml_sax_has_event(&reader))
    {
        if (cxml_sax_get_event(&reader) == CXML_SAX_TEXT_EVENT){
            cxml_sax_get_text_data(&reader, &data);
            break;
        }
    }
    cxml_sax_close_event_reader(&reader);
    CHECK_TRUE(cxml_string_raw_equals(&data,
                                             "PyPI recent updates for switches.py"))
    cxml_string_free(&data);

    cxml_sax_get_text_data(&reader, &data);
    CHECK_EQ(cxml_string_len(&data), 0)

    cxml_sax_get_text_data(&reader, NULL);
    cxml_sax_get_text_data(NULL, &data);
    CHECK_EQ(cxml_string_len(&data), 0)
    cxml_set_config(cfg);
    cxml_pass()
}

cts test_cxml_sax_get_cdsect_data(){
    cxml_config cfg = cxml_get_config();
    cxml_cfg_preserve_space(0);
    cxml_sax_event_reader reader = get_event_reader("cdata.xml", false);
    cxml_string data = new_cxml_string();

    while (cxml_sax_has_event(&reader))
    {
        if (cxml_sax_get_event(&reader) == CXML_SAX_CDATA_EVENT){
            cxml_sax_get_cdsect_data(&reader, &data);
            break;
        }
    }
    cxml_sax_close_event_reader(&reader);
    CHECK_TRUE(cxml_string_raw_equals(&data, "Testing cdata stuff"))
    cxml_string_free(&data);

    cxml_sax_get_cdsect_data(&reader, &data);
    CHECK_EQ(cxml_string_len(&data), 0)

    cxml_sax_get_cdsect_data(&reader, NULL);
    cxml_sax_get_cdsect_data(NULL, &data);
    CHECK_EQ(cxml_string_len(&data), 0)
    cxml_set_config(cfg);
    cxml_pass()
}

cts test_cxml_sax_get_pi_data(){
    cxml_config cfg = cxml_get_config();
    cxml_cfg_preserve_space(0);
    cxml_sax_event_reader reader = get_event_reader("foo.xml", false);
    cxml_string data = new_cxml_string();

    char *targets[] = {"processor", "ext", "language"};
    char *values[] = {"my-proc ", "xml stuff", "python thingy"};
    int i = 0;
    while (cxml_sax_has_event(&reader))
    {
        if (cxml_sax_get_event(&reader) == CXML_SAX_PROCESSING_INSTRUCTION_EVENT){
            if (i < 2){
                cxml_sax_get_pi_data(&reader, targets[i], &data);
                CHECK_TRUE(cxml_string_raw_equals(&data, values[i]))
                cxml_string_free(&data);
            } else{
                // if no target is passed, and there exists a pi node, it'll be consumed,
                // and its data would be stored in `data`
                cxml_sax_get_pi_data(&reader, NULL, &data);
                CHECK_TRUE(cxml_string_raw_equals(&data, values[i]))
                cxml_string_free(&data);
                break;
            }
            i++;
        }
    }
    cxml_sax_close_event_reader(&reader);

    cxml_sax_get_pi_data(&reader, NULL, &data);
    CHECK_EQ(cxml_string_len(&data), 0)

    cxml_sax_get_pi_data(&reader, "ext", &data);
    cxml_sax_get_pi_data(&reader, "ext", NULL);
    cxml_sax_get_pi_data(NULL, "ext", &data);
    CHECK_EQ(cxml_string_len(&data), 0)
    cxml_set_config(cfg);
    cxml_pass()
}

cts test_cxml_sax_get_attribute_data(){
    cxml_config cfg = cxml_get_config();
    cxml_cfg_preserve_space(0);
    cxml_sax_event_reader reader = get_event_reader("cdata.xml", false);
    cxml_string data = new_cxml_string();

    char *keys[] = {"mod", "van", "hex"};
    char *values[] = {"63", "'40'", "0xdeadbeef"};

    while (cxml_sax_has_event(&reader))
    {
        if (cxml_sax_get_event(&reader) == CXML_SAX_ATTRIBUTE_EVENT){
            for (int i = 0; i < 3; i++)
            {
                cxml_sax_get_attribute_data(&reader, keys[i], &data);
                CHECK_TRUE(cxml_string_raw_equals(&data, values[i]))
                cxml_string_free(&data);
            }
            break;
        }
    }
    cxml_sax_close_event_reader(&reader);

    cxml_sax_get_attribute_data(&reader, NULL, &data);
    CHECK_EQ(cxml_string_len(&data), 0)

    cxml_sax_get_attribute_data(&reader, "van", &data);
    cxml_sax_get_attribute_data(&reader, "hex", NULL);
    cxml_sax_get_attribute_data(NULL, "mod", &data);
    CHECK_EQ(cxml_string_len(&data), 0)
    cxml_set_config(cfg);
    cxml_pass()
}

cts test_cxml_sax_close_event_reader(){
    cxml_sax_event_reader reader = get_event_reader("wf_xml_1.xml", false);
    cxml_sax_close_event_reader(&reader);
    CHECK_NE(reader.is_open, 1)
    CHECK_EQ(reader.is_open, ((1 << 6) | 3735928559))
    cxml_pass()
}

cts test_cxml_sax_open_event_reader(){
    char *fp = get_file_path("wf_xml_1.xml");
    cxml_sax_event_reader reader;
    cxml_sax_open_event_reader(&reader, fp, false);
    FREE(fp);
    CHECK_EQ(reader.is_open, 1)
    cxml_sax_close_event_reader(&reader);
    cxml_pass()
}


void suite_cxsax() {
    cxml_suite(cxsax)
    {
        cxml_add_m_test(22,
                        test_cxml_sax_init,
                        test_cxml_sax_has_event,
                        test_cxml_sax_get_event,
                        test_cxml_sax_is_well_formed,
                        test_cxml_stream_file,
                        test_cxml_sax_as_comment_node,
                        test_cxml_sax_as_pi_node,
                        test_cxml_sax_as_text_node,
                        test_cxml_sax_as_cdsect_node,
                        test_cxml_sax_as_namespace_list,
                        test_cxml_sax_as_attribute_list,
                        test_cxml_sax_as_attribute_node,
                        test_cxml_sax_as_dtd_node,
                        test_cxml_sax_as_xml_hdr_node,
                        test_cxml_sax_get_element_name,
                        test_cxml_sax_get_comment_data,
                        test_cxml_sax_get_text_data,
                        test_cxml_sax_get_cdsect_data,
                        test_cxml_sax_get_pi_data,
                        test_cxml_sax_get_attribute_data,
                        test_cxml_sax_close_event_reader,
                        test_cxml_sax_open_event_reader
        )
        cxml_run_suite()
    }
}
