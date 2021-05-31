/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXML_H
#define CXML_CXML_H

#include "xml/cxprinter.h"

#if defined(CXML_USE_QUERY_MOD)
    #include "query/cxqapi.h"
#endif

#if defined(CXML_USE_XPATH_MOD)
    #include "xpath/cxxpeval.h"
#endif

#if defined(CXML_USE_SAX_MOD)
    #include "sax/cxsax.h"
#endif

cxml_root_node* cxml_load_file(const char *fn, bool stream);
cxml_root_node*  cxml_load_string(const char *str);

#endif //CXML_CXML_H
