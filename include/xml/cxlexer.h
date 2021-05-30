/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_LEXER_H
#define CXML_LEXER_H

#include "core/cxliteral.h"
#include "core/cxconfig.h"
#include "cxstream.h"


typedef enum{
    //single character tokens
    CXML_TOKEN_G_THAN, // >
    CXML_TOKEN_L_THAN, // <
    CXML_TOKEN_Q_MARK, // ?
    CXML_TOKEN_EQUAL,  // =
    CXML_TOKEN_TEXT,   // ...
    CXML_TOKEN_F_SLASH, // /
    CXML_TOKEN_COLON,   // :

    //compound tokens
    CXML_TOKEN_STRING,
    CXML_TOKEN_VALUE,
    //could be ELEMENT NAME, ATTRIBUTE IDENTIFIER NAME
    CXML_TOKEN_IDENTIFIER,
    CXML_TOKEN_COMMENT,
    // cxml is a non-validating xml parser,
    // hence no actual use of DOCTYPE except for rendering purposes
    CXML_TOKEN_DOCTYPE,

    // eof, init, errors,
    CXML_TOKEN_EOF,
    CXML_TOKEN_CDATA,
    CXML_TOKEN_ERROR
}_cxml_token_t;


typedef struct {
    _cxml_token_t type;
    cxml_literal_t literal_type;
    char *start;
    int length;
    int line;
} _cxml_token;

typedef struct {
    char *start;
    char *current;
    int line;
    bool error;
    bool preserve_sp;
    bool preserve_cm;
    bool preserve_cd;
    _cxml_stream _stream_obj;
    char vflag;  // value flag <tag>...value...</tag>
    bool _stream;
    bool _should_stream;
    int _returned;
    cxml_config cfg;
} _cxml_lexer;


void _cxml_lexer_init(
        _cxml_lexer *cxlexer,
        const char *source,
        const char *filename,
        bool stream);

void _cxml_token_init(_cxml_token *token);

void cxml_print_tokens(const char *src);

void _cxml_lexer_close(_cxml_lexer *cxlexer);
#endif //cXML_LEXER_H
