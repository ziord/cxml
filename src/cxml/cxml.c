/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

/**
 * C XML Minimalistic Library (CXML)
 * @author: Jeremiah Ikosin (ziord)
 */

#include "cxml/cxml.h"

/*
 * xml parsing helper functions
 */
cxml_root_node* cxml_load_file(const char *fn, bool stream) {
    if (!fn) return NULL;
    cxml_root_node *root = NULL;
    if (stream){
        root = cxml_parse_xml_lazy(fn);
    }else{
        char* src_buff = NULL;
        _cxml_read_file(fn, &src_buff);
        root = cxml_parse_xml(src_buff);
        FREE(src_buff);
    }
    return root;
}

cxml_root_node*  cxml_load_string(const char *str) {
    if (!str) return NULL;
    return cxml_parse_xml(str);
}
