/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXXPEVAL_H
#define CXML_CXXPEVAL_H

#include "core/cxgrptable.h"
#include "xml/cxprinter.h"
#include "cxxpresolver.h"
#include "cxxplib.h"

/**Debug**/
void cxml_xp_debug_expr();

/** public api **/
cxml_set* cxml_xpath(void *root, const char *expr);
#endif
