/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXPRINTER_H
#define CXML_CXPRINTER_H

#include "cxparser.h"
#include "utils/cxutils.h"

char *cxml_prettify(void *node);

char* cxml_stringify(void *node);

void cxml_element_to_string(cxml_element_node* node, cxml_string *str);

char *cxml_element_to_rstring(cxml_element_node *node);

void cxml_attribute_to_string(cxml_attribute_node *attr_node, cxml_string *str);

char *cxml_attribute_to_rstring(cxml_attribute_node *attr_node);

void cxml_namespace_to_string(cxml_namespace_node* ns_node, cxml_string *str);

char* cxml_namespace_to_rstring(cxml_namespace_node* ns_node);

void cxml_pi_to_string(cxml_pi_node *pi, cxml_string *str);

char *cxml_pi_to_rstring(cxml_pi_node *pi);

void cxml_comment_to_string(cxml_comment_node *comm_node, cxml_string *str);

char *cxml_comment_to_rstring(cxml_comment_node *comm_node);

void cxml_dtd_to_string(cxml_dtd_node *dtd_node, cxml_string *str);

char *cxml_dtd_to_rstring(cxml_dtd_node *dtd_node);

void cxml_text_to_string(cxml_text_node *text_node, cxml_string *str);

char *cxml_text_to_rstring(cxml_text_node *text_node);

void cxml_document_to_string(cxml_root_node *root, cxml_string *str);

char *cxml_document_to_rstring(cxml_root_node *root);

void cxml_xhdr_to_string(cxml_xhdr_node *hdr, cxml_string *str);

char *cxml_xhdr_to_rstring(cxml_xhdr_node *xmlhdr);

void cxml_node_to_string(void* node, cxml_string *str);

char *cxml_node_to_rstring(void *node);

void cxml_element_to_file(cxml_elem_node *elem, const char* file_name);

void cxml_comment_to_file(cxml_comm_node *comment, const char* file_name);

void cxml_pi_to_file(cxml_pi_node *pi, const char* file_name);

void cxml_text_to_file(cxml_text_node *text, const char* file_name);

void cxml_dtd_to_file(cxml_dtd_node *dtd, const char* file_name);

void cxml_xhdr_to_file(cxml_xhdr_node *xml_hdr, const char* file_name);

void cxml_document_to_file(cxml_root_node *root, const char* file_name);

void cxml_node_to_file(void *node, const char* file_name);


#endif //CXML_CXPRINTER_H
