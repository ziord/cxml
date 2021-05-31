/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXQL_H
#define CXML_CXQL_H

#include "core/cxdefs.h"
#include "xml/cxlexer.h"

/* The cxml query language */


#define     _CXQ_MATCH_ANY           (1 << 1)
#define     _CXQ_MATCH_PARTIAL       (1 << 2)
#define     _CXQ_MATCH_EXACT         (1 << 3)
#define     _CXQ_MATCH_KEY_ONLY      (1 << 4)


struct _cxml_q_text{
    int flags;
    cxml_string *text;
};

struct _cxml_q_comm{
    int flags;
    cxml_string *comment;
};

struct _cxml_q_attr{
    int flags;
    cxml_string *key;
    cxml_string *value;
};

typedef struct{
    struct _cxml_q_text *q_text;
    struct _cxml_q_comm *q_comm;
    struct _cxml_q_attr *q_attr;
}_cxml_q;

/*
 * query expression e.g. "<tag_name>/[name='ziord']/id='xy'/"
 * would be parsed into a _cxml_query object
 */
typedef struct{
    cxml_string q_name;      // pseudonymous to tag name
    cxml_list q_o_list;      // _cxml_q (optional),
    cxml_list q_r_list;      // _cxml_q (rigid)
    const char* expr;
}_cxml_query;

typedef struct{
    const char *src;
    const char* current;
    const char* start;
}_cxml_query_lexer;


_cxml_query *cxq_parse_query(const char *query);

void cxq_free_query(_cxml_query *query);


#endif //CXML_CXQL_H
