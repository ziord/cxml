/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "xpath/cxxplexer.h"

/**
 lexer
**/

inline static void _cxml_xp__move(_cxml_xp_lexer *xplexer);

static void _cxml_xp_lex_error(char *msg);

extern bool _cxml__is_alpha(char ch);

extern bool _cxml__is_identifier(char ch);


void _cxml_xp_token_init(_cxml_xp_token* token){
    token->length = 0;
    token->type = -1;
    token->start = NULL;
}

static _cxml_xp_token _cx_new_xpath_token(){
    _cxml_xp_token token;
    _cxml_xp_token_init(&token);
    return token;
}

void _cxml_xp_lexer_init(_cxml_xp_lexer *xplexer, const char *expr) {
    xplexer->expr = xplexer->start = xplexer->current = expr;
    xplexer->col_no = 0;
    xplexer->line_no = 1;
}

static _cxml_xp_token
_cx_create_xpath_token(_cxml_xp_lexer *xplexer, _cxml_xp_token_t type){
    _cxml_xp_token token = _cx_new_xpath_token();
    token.type = type;
    token.length = (int) (xplexer->current - xplexer->start);
    token.start = xplexer->start;
    return token;
}

inline static bool _cx_at_end(_cxml_xp_lexer *xplexer){
    return *(xplexer->current) == '\0';
}

static void _cx_skip_whitespace(_cxml_xp_lexer *xplexer){
    if (_cx_at_end(xplexer)) return;
    while (isspace((unsigned char)*xplexer->current)){
        _cxml_xp__move(xplexer);
    }
}

// check if function name actually matches expected name
static _cxml_xp_token_t
_cx_expect(_cxml_xp_lexer *xplexer,
        char* rem,
        int b_ind,
        int r_ind,
        _cxml_xp_token_t type,
        bool is_nodetest_kwd)
{
    if ((xplexer->current - xplexer->start) == (b_ind + r_ind))
    {
        if (memcmp((xplexer->start + b_ind), rem, r_ind) == 0){
            // 'text' coincides with 'text()' - same goes for other type-test tokens
            if (is_nodetest_kwd){
                _cx_skip_whitespace(xplexer);
                if (*xplexer->current == '(') return type;
                return CXML_XP_TOKEN_NAME;
            }
            return type;
        }
    }
    return CXML_XP_TOKEN_NAME;
}

static _cxml_xp_token_t _cx_id_type(_cxml_xp_lexer *xplexer){
    switch(*xplexer->start)
    {
        case 'a': return _cx_expect(xplexer, "nd", 1, 2, CXML_XP_TOKEN_AND, 0); // and
        case 'o': return _cx_expect(xplexer, "r", 1, 1, CXML_XP_TOKEN_OR, 0);  // or
        case 'd': return _cx_expect(xplexer, "iv", 1, 2, CXML_XP_TOKEN_DIV, 0); //div
        case 'm': return _cx_expect(xplexer, "od", 1, 2, CXML_XP_TOKEN_MOD, 0); //mod
        case 't': return _cx_expect(xplexer, "ext", 1, 3, CXML_XP_TOKEN_TEXT_F, 1); // text
        case 'n': return _cx_expect(xplexer, "ode", 1, 3, CXML_XP_TOKEN_NODE_F, 1); // node
        case 'p': return _cx_expect(xplexer, "rocessing-instruction", 1, 21, CXML_XP_TOKEN_PI_F, 1); // p-i
        case 'c': return _cx_expect(xplexer, "omment", 1, 6, CXML_XP_TOKEN_COMMENT_F, 1); // comment
        default:  return CXML_XP_TOKEN_NAME;
    }
}

static _cxml_xp_token _cx_identifier(_cxml_xp_lexer *xplexer) {
    while (_cxml__is_identifier(*xplexer->current))
    {
        _cxml_xp__move(xplexer);
    }
    return _cx_create_xpath_token(xplexer, _cx_id_type(xplexer));
}

static _cxml_xp_token _cx_number(_cxml_xp_lexer *xplexer){
    int dc = 0;
    if (*xplexer->current == '.') _cxml_xp__move(xplexer), dc++;
    while ( isdigit((unsigned char)(*xplexer->current))
           || isxdigit((unsigned char)*xplexer->current)
           || *xplexer->current == 'x'
           || *xplexer->current == 'X'
           || *xplexer->current == '-'
           || *xplexer->current == '+' )
    {
        _cxml_xp__move(xplexer);
        if (*xplexer->current == '.'){
            _cxml_xp__move(xplexer), dc++;
        }
    }
    return dc <= 1 ?
           _cx_create_xpath_token(xplexer, CXML_XP_TOKEN_NUMBER) :
           _cx_create_xpath_token(xplexer, CXML_XP_TOKEN_NAME);
}

static _cxml_xp_token _cx_literal(_cxml_xp_lexer *xplexer){
    char stop = *xplexer->start == '\'' ? '\'' : '"';
    while (!_cx_at_end(xplexer) && *xplexer->current != stop){
        _cxml_xp__move(xplexer);
    }
    _cx_at_end(xplexer) ?
    _cxml_xp_lex_error("Unterminated literal in xpath expression") :
    (void)0;
    _cxml_xp__move(xplexer);
    return _cx_create_xpath_token(xplexer, CXML_XP_TOKEN_LITERAL);
}

inline
static void _cxml_xp__move(_cxml_xp_lexer *xplexer){
    if (_cx_at_end(xplexer)) return;
    xplexer->current++;
    if (*(xplexer->current) == '\n'){
        xplexer->line_no++;
        xplexer->col_no = 0;
    }else{
        xplexer->col_no++;
    }
}

inline static void _cxml_xp__move_st(_cxml_xp_lexer *xplexer, int step){
    for (int i=0; i<step; i++){
        _cxml_xp__move(xplexer);
    }
}

inline static char _cxml_xp_peek(_cxml_xp_lexer *xplexer, int pos){
    if (_cx_at_end(xplexer)) return *xplexer->current;
    return *(xplexer->current + pos);
}

static void _cxml_xp_lex_error(char* msg){
    cxml_error("%s\n", msg);
}

static void _cx_skip_xpath_comment(_cxml_xp_lexer *xplexer){
    if (_cx_at_end(xplexer)) return;
    if (_cxml_xp_peek(xplexer, 0) == '(' && _cxml_xp_peek(xplexer, 1) == ':')
    {
        start:
        _cxml_xp__move_st(xplexer, 2);
        int in_nested_comment = 0;
        while ( (_cxml_xp_peek(xplexer, 0) != ':'
                 && _cxml_xp_peek(xplexer, 1) != ')')
               || in_nested_comment)
        {
            _cxml_xp__move(xplexer);
            if (_cxml_xp_peek(xplexer, -1) == '(' && _cxml_xp_peek(xplexer, 0) == ':')
            {
                in_nested_comment++;
               _cxml_xp__move(xplexer);
            }
            else if (_cxml_xp_peek(xplexer, -1) == ':'
                     && _cxml_xp_peek(xplexer, 0) == ')'
                     && in_nested_comment)
            {
                in_nested_comment--;
                _cxml_xp__move(xplexer);
            }
        }
        // fail fast ?
        if (_cx_at_end(xplexer))
        {
            _cxml_xp_lex_error("Comment in xpath expression not terminated");
        }
        // :, ) encountered
        if (_cxml_xp_peek(xplexer, 0) == ':' && _cxml_xp_peek(xplexer, 1) == ')')
        {
            _cxml_xp__move_st(xplexer, 2);
        }
        else
        {
            _cxml_xp_lex_error("Comment in xpath expression not properly terminated");
        }
    }
    _cx_skip_whitespace(xplexer);
    // if more comments are found, skip as well.
    if (_cxml_xp_peek(xplexer, 0) == '(' && _cxml_xp_peek(xplexer, 1) == ':') goto start;
}

inline static bool _cx_check(_cxml_xp_lexer *xplexer, char ch){
    if (_cx_at_end(xplexer)) return false;
    return (*xplexer->current == ch) ? _cxml_xp__move(xplexer), 1 : 0;
}

_cxml_xp_token _cxml_xp_get_token(_cxml_xp_lexer *xplexer) {
#define return_token(ttype)   \
return _cx_create_xpath_token(xplexer, ttype);

    while(true){
        _cx_skip_whitespace(xplexer);
        _cx_skip_xpath_comment(xplexer);

        xplexer->start = xplexer->current;

        if (_cx_at_end(xplexer)){
            return_token(CXML_XP_TOKEN_END)
        }
        char ch = *xplexer->current;
        _cxml_xp__move(xplexer);

        if (_cxml__is_alpha(ch))
        {
            return _cx_identifier(xplexer);
        }
        else if (isdigit((unsigned char)ch)
                || (ch == '.' && isdigit((unsigned char)_cxml_xp_peek(xplexer, 1))))
        {
            return _cx_number(xplexer);
        }
        else if (ch == '!' && *xplexer->current == '=')
        {
            _cxml_xp__move(xplexer);
            return_token(CXML_XP_TOKEN_NOT_EQ)
        }
        switch(ch)
        {
            case '<':
                return_token(_cx_check(xplexer, '=') ? CXML_XP_TOKEN_LTHAN_EQ : CXML_XP_TOKEN_LTHAN)
            case '>':
                return_token(_cx_check(xplexer, '=') ? CXML_XP_TOKEN_GTHAN_EQ : CXML_XP_TOKEN_GTHAN)
            case '.':
                return_token(_cx_check(xplexer, '.') ? CXML_XP_TOKEN_D_DOT : CXML_XP_TOKEN_DOT)
            case '/':
                return_token(_cx_check(xplexer, '/') ? CXML_XP_TOKEN_DF_SLASH : CXML_XP_TOKEN_F_SLASH)
            case '(':
                return_token(CXML_XP_TOKEN_L_BRACKET)
            case ')':
                return_token(CXML_XP_TOKEN_R_BRACKET)
            case '@':
                return_token(CXML_XP_TOKEN_AT)
            case '=':
                return_token(CXML_XP_TOKEN_EQ)
            case '*':
                return_token(CXML_XP_TOKEN_STAR)
            case '[':
                return_token(CXML_XP_TOKEN_L_SQR_BRACKET)
            case ']':
                return_token(CXML_XP_TOKEN_R_SQR_BRACKET)
            case '|':
                return_token(CXML_XP_TOKEN_PIPE)
            case '\'':
            case '"':
                return _cx_literal(xplexer);
            case '+':
                return_token(CXML_XP_TOKEN_PLUS)
            case '-':
                return_token(CXML_XP_TOKEN_MINUS)
            case ',':
                return_token(CXML_XP_TOKEN_COMMA)
            case ':':
                return_token(CXML_XP_TOKEN_COLON)
            default:
                return_token(0xff)
        }
    }
#undef return_token
}

static char * _cxml_xp_get_token_type(_cxml_xp_token_t type) {
#define return_type(ttype)    return (#ttype);

    switch (type)
    {
        case CXML_XP_TOKEN_NUMBER:
            return_type(CXML-XPATH-TOKEN-NUMBER)
        case CXML_XP_TOKEN_L_SQR_BRACKET:
            return_type(CXML-XPATH-TOKEN-LEFT-SQUARE-BRACKET)
        case CXML_XP_TOKEN_STAR:
            return_type(CXML-XPATH-TOKEN-STAR)
        case CXML_XP_TOKEN_DOT:
            return_type(CXML-XPATH-TOKEN-DOT)
        case CXML_XP_TOKEN_L_BRACKET:
            return_type(CXML-XPATH-TOKEN-LEFT-BRACKET)
        case CXML_XP_TOKEN_NAME:
            return_type(CXML-XPATH-TOKEN-NAME)
        case CXML_XP_TOKEN_AT:
            return_type(CXML-XPATH-TOKEN-AT)
        case CXML_XP_TOKEN_DF_SLASH:
            return_type(CXML-XPATH-TOKEN-DOUBLE-FORWARD-SLASH)
        case CXML_XP_TOKEN_F_SLASH:
            return_type(CXML-XPATH-TOKEN-FORWARD-SLASH)
        case CXML_XP_TOKEN_R_BRACKET:
            return_type(CXML-XPATH-TOKEN-RIGHT-BRACKET)
        case CXML_XP_TOKEN_R_SQR_BRACKET:
            return_type(CXML-XPATH-TOKEN-RIGHT-SQUARE-BRACKET)
        case CXML_XP_TOKEN_D_DOT:
            return_type(CXML-XPATH-TOKEN-DOUBLE-DOT)
        case CXML_XP_TOKEN_PIPE:
            return_type(CXML-XPATH-TOKEN-PIPE)
        case CXML_XP_TOKEN_PLUS:
            return_type(CXML-XPATH-TOKEN-PLUS)
        case CXML_XP_TOKEN_MINUS:
            return_type(CXML-XPATH-TOKEN-MINUS)
        case CXML_XP_TOKEN_MOD:
            return_type(CXML-XPATH-TOKEN-MOD)
        case CXML_XP_TOKEN_DIV:
            return_type(CXML-XPATH-TOKEN-DIV)
        case CXML_XP_TOKEN_LTHAN:
            return_type(CXML-XPATH-TOKEN-LESS-THAN)
        case CXML_XP_TOKEN_GTHAN:
            return_type(CXML-XPATH-TOKEN-GREATER-THAN)
        case CXML_XP_TOKEN_LTHAN_EQ:
            return_type(CXML-XPATH-TOKEN-LESS-THAN-EQUAL)
        case CXML_XP_TOKEN_GTHAN_EQ:
            return_type(CXML-XPATH-TOKEN-GREATER-THAN-EQUAL)
        case CXML_XP_TOKEN_NOT_EQ:
            return_type(CXML-XPATH-TOKEN-NOT-EQUAL)
        case CXML_XP_TOKEN_AND:
            return_type(CXML-XPATH-TOKEN-AND)
        case CXML_XP_TOKEN_OR:
            return_type(CXML-XPATH-TOKEN-OR)
        case CXML_XP_TOKEN_END:
            return_type(CXML-XPATH-TOKEN-END)
        case CXML_XP_TOKEN_EQ:
            return_type(CXML-XPATH-TOKEN-EQUAL)
        case CXML_XP_TOKEN_LITERAL:
            return_type(CXML-XPATH-TOKEN-LITERAL)
        case CXML_XP_TOKEN_COMMA:
            return_type(CXML-XPATH-TOKEN-COMMA)
        case CXML_XP_TOKEN_TEXT_F:
            return_type(CXML-XPATH-TOKEN-TEXT-TYPE)
        case CXML_XP_TOKEN_COMMENT_F:
            return_type(CXML-XPATH-TOKEN-COMMENT-TYPE)
        case CXML_XP_TOKEN_PI_F:
            return_type(CXML-XPATH-TOKEN-PI-TYPE)
        case CXML_XP_TOKEN_NODE_F:
            return_type(CXML-XPATH-TOKEN-NODE-TYPE)
        case CXML_XP_TOKEN_COLON:
            return_type(CXML-XPATH-TOKEN-COLON)
        default:
            return_type(CXML-XPATH-TOKEN-ERROR)
    }
#undef return_type
}

void cxml_print_xpath_tokens(const char* expr){
    if (!expr) return;
    _cxml_xp_lexer _xp_lexer;
    _cxml_xp_lexer_init(&_xp_lexer, expr);
    while (true){
        _cxml_xp_token token = _cxml_xp_get_token(&_xp_lexer);
        printf(" | %*s\t\t%02d\t\t'%.*s'\n", 0x32,
               _cxml_xp_get_token_type(token.type),
               token.type,
               token.length, token.start);
        if (token.type == CXML_XP_TOKEN_END) break;
    }
}
