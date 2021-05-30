/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXXPLEXER_H
#define CXML_CXXPLEXER_H

/*
 * Minimal xpath (1.0) selection syntax support including:
 *
 * expressions
 * predicates
 * wildcards
 * comments
 * function calls
 *
 */
#include "xml/cxlexer.h"
#include "core/cxstack.h"


typedef enum{
    CXML_XP_TOKEN_NAME = 0,       // node_name
    CXML_XP_TOKEN_F_SLASH ,    // '/'
    CXML_XP_TOKEN_DF_SLASH,    // '//'
    CXML_XP_TOKEN_AT,         // @
    CXML_XP_TOKEN_DOT,        // .
    CXML_XP_TOKEN_D_DOT,       // ..
    CXML_XP_TOKEN_R_BRACKET,     // (
    CXML_XP_TOKEN_L_BRACKET,      // )
    CXML_XP_TOKEN_NUMBER,         // 1, 2,3
    CXML_XP_TOKEN_L_SQR_BRACKET, // [
    CXML_XP_TOKEN_R_SQR_BRACKET,  // ]
    CXML_XP_TOKEN_PIPE,              // |
    CXML_XP_TOKEN_LITERAL,      // "..."
    CXML_XP_TOKEN_COMMA,        // ','
    CXML_XP_TOKEN_COLON,        // ':'

    // relative binary operators
    CXML_XP_TOKEN_LTHAN,            // <
    CXML_XP_TOKEN_GTHAN,            // >
    CXML_XP_TOKEN_EQ,               // =
    CXML_XP_TOKEN_LTHAN_EQ,         // <=
    CXML_XP_TOKEN_GTHAN_EQ,         // >=
    CXML_XP_TOKEN_NOT_EQ,            // !=
    /* *****  */

    // binary and unary operators
    CXML_XP_TOKEN_PLUS,   // '+'
    CXML_XP_TOKEN_MINUS,  // '-'
    CXML_XP_TOKEN_STAR,       // *
    /* *****  */

    // named operators
    CXML_XP_TOKEN_AND,          // 'and'
    CXML_XP_TOKEN_OR,           // 'or'
    CXML_XP_TOKEN_MOD,  // 'mod'
    CXML_XP_TOKEN_DIV,   // 'div'
    /* *****  */

    // node type
    CXML_XP_TOKEN_TEXT_F,       // text()
    CXML_XP_TOKEN_COMMENT_F,    // comment()
    CXML_XP_TOKEN_PI_F,       // processing-instruction()
    CXML_XP_TOKEN_NODE_F,       // node()
    /* ***** */

    CXML_XP_TOKEN_END
} _cxml_xp_token_t;

typedef struct {
    _cxml_xp_token_t type;
    int length;
    const char *start;
} _cxml_xp_token;

typedef struct {
    int line_no;  /* line number: useful for error context display */
    int col_no;   /* column number: useful for error context display */
    const char* expr;
    const char *current;
    const char *start;
} _cxml_xp_lexer;


void cxml_print_xpath_tokens(const char *expr);

#endif //CXML_CXXPLEXER_H
