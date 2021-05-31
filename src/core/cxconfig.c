/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxconfig.h"

static cxml_config _cxml_config_gb = {
        .doc_name = "XMLDocument",
        .preserve_space = 1,
        .preserve_comment = 1,
        .preserve_cdata = 1,
        .chunk_size = 0x1400000,
        .transpose_text = 1,
        .indent_space_size = 2,
        .show_doc_as_top_level = 1,
        .print_fancy = 1,
        .show_warnings = 1,
        .enable_debugging = 0,
        .preserve_dtd_structure = 0,
        .strict_transpose = 0,
        .ensure_ns_attribute_unique = 1,
        .allow_default_namespace = 1
};


cxml_config cxml_get_config(){
    return _cxml_config_gb;
}

cxml_config cxml_get_config_defaults(){
    return (cxml_config) {
            .doc_name = "XMLDocument",
            .preserve_space = 1,
            .preserve_comment = 1,
            .preserve_cdata = 1,
            .chunk_size = 0x1400000,
            .transpose_text = 1,
            .indent_space_size = 2,
            .show_doc_as_top_level = 1,
            .print_fancy = 1,
            .show_warnings = 1,
            .enable_debugging = 0,
            .preserve_dtd_structure = 0,
            .strict_transpose = 0,
            .ensure_ns_attribute_unique = 1,
            .allow_default_namespace = 1
    };
}

void cxml_set_config(cxml_config config){
    _cxml_config_gb = config;
}

void cxml_reset_config(){
    cxml_set_config(cxml_get_config_defaults());
}

void cxml_cfg_enable_fancy_printing(bool enable){
    _cxml_config_gb.print_fancy = enable;
}

void cxml_cfg_set_indent_space_size(short size){
    _cxml_config_gb.indent_space_size = size;
}

void cxml_cfg_set_doc_name(const char *doc_name){
    if (!doc_name) return;
    _cxml_config_gb.doc_name = doc_name;
}

void cxml_cfg_set_chunk_size(int size){
    _cxml_config_gb.chunk_size = size;
}

void cxml_cfg_set_text_transposition(bool transpose_text, bool use_strict_mode){
    _cxml_config_gb.transpose_text = transpose_text;
    _cxml_config_gb.strict_transpose = use_strict_mode;
}

void cxml_cfg_preserve_space(bool preserve_space){
    _cxml_config_gb.preserve_space = preserve_space;
}

void cxml_cfg_preserve_comment(bool preserve){
    _cxml_config_gb.preserve_comment = preserve;
}

void cxml_cfg_preserve_cdata(bool preserve){
    _cxml_config_gb.preserve_cdata = preserve;
}

void cxml_cfg_trim_dtd(bool trim_dtd){
    _cxml_config_gb.preserve_dtd_structure = !trim_dtd;
}

void cxml_cfg_show_doc_as_top_level(bool show){
    _cxml_config_gb.show_doc_as_top_level = show;
}

void cxml_cfg_show_warnings(bool show){
    _cxml_config_gb.show_warnings = show;
}

void cxml_cfg_enable_debugging(bool enable){
    _cxml_config_gb.enable_debugging = enable;
}

void cxml_cfg_allow_default_namespace(bool allow){
    _cxml_config_gb.allow_default_namespace = allow;
}

void cxml_cfg_allow_duplicate_namespaces(bool allow){
    _cxml_config_gb.ensure_ns_attribute_unique = !allow;
}
