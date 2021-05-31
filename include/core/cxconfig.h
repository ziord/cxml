/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXCONFIG_H
#define CXML_CXCONFIG_H

#include "cxcomm.h"

// configs
typedef struct _cxml_config{
    // xml document name
    const char* doc_name;
    // stream chunk size
    unsigned int chunk_size;
    // transpose text ? & -> &amp;  &amp; -> &
    bool transpose_text;
    // should <XMLDocument></XMLDocument> be shown as toplevel when document is parsed to string ?
    bool show_doc_as_top_level;
    // should type names of nodes be printed alongside the node contents ?
    bool print_fancy;
    // preserve space
    bool preserve_space;
    // size of space to be used in indenting when rendering xml doc
    short indent_space_size;
    // preserve comment
    bool preserve_comment;
    // preserve cdata
    bool preserve_cdata;
    // show parser warnings ?
    bool show_warnings;
    // enable debug information
    bool enable_debugging;
    // preserve entire dtd string ? true preserves, false keeps only dtd name
    bool preserve_dtd_structure;
    // transpose ALL predefined entities if true, and only '&', '<', '>' if false
    bool strict_transpose;
    // validate namespaced attributes for uniqueness
    bool ensure_ns_attribute_unique;
    // allows elements have a default namespace
    bool allow_default_namespace;
    // other configs goes here
}cxml_config;

cxml_config cxml_get_config();

cxml_config cxml_get_config_defaults();

void cxml_set_config(cxml_config config);

void cxml_reset_config();

void cxml_cfg_enable_fancy_printing(bool enable);

void cxml_cfg_set_indent_space_size(short size);

void cxml_cfg_set_doc_name(const char *doc_name);

void cxml_cfg_set_chunk_size(int size);

void cxml_cfg_set_text_transposition(bool transpose_text, bool use_strict_mode);

void cxml_cfg_preserve_space(bool preserve_space);

void cxml_cfg_preserve_comment(bool preserve);

void cxml_cfg_preserve_cdata(bool preserve);

void cxml_cfg_trim_dtd(bool trim_dtd);

void cxml_cfg_show_doc_as_top_level(bool show);

void cxml_cfg_show_warnings(bool show);

void cxml_cfg_enable_debugging(bool enable);

void cxml_cfg_allow_default_namespace(bool allow);

void cxml_cfg_allow_duplicate_namespaces(bool enable);


#endif //CXML_CXCONFIG_H
