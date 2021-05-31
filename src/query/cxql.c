/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "query/cxql.h"


extern bool _cxml__is_alpha(char ch);

extern bool _cxml__is_identifier(char ch);

void cxq_init_lexer(_cxml_query_lexer *lexer, const char *src);

void cxq_free_lexer(_cxml_query_lexer *);

void cxq_free_q(_cxml_q *qry);


static _cxml_query_lexer *cxq__create_lexer(const char *src) {
    _cxml_query_lexer *q_lexer = ALLOC(_cxml_query_lexer, 1);
    cxq_init_lexer(q_lexer, src);
    return q_lexer;
}

void cxq_init_lexer(_cxml_query_lexer *lexer, const char* src) {
    lexer->current = lexer->start = lexer->src = src;
}

void cxq_init_query(_cxml_query *q_object) {
    cxml_list_init(&q_object->q_o_list);
    cxml_list_init(&q_object->q_r_list);
    cxml_string_init(&q_object->q_name);
    q_object->expr = NULL;
}

void cxq_init_q_text(struct _cxml_q_text *q_text){
    q_text->flags = 1;
    q_text->text = NULL;
}

void cxq_init_q_comm(struct _cxml_q_comm *q_comm){
    q_comm->flags = 1;
    q_comm->comment = NULL;
}

void cxq_init_q_attr(struct _cxml_q_attr *q_attr){
    q_attr->flags = 1;
    q_attr->value = NULL;
    q_attr->key = NULL;
}

void cxq_init_q(_cxml_q *q) {
    q->q_text = NULL;
    q->q_attr = NULL;
    q->q_comm = NULL;
}

static _cxml_query *cxq__new_query() {
    _cxml_query *query = ALLOC(_cxml_query, 1);
    cxq_init_query(query);
    return query;
}

static _cxml_q *cxq__new_q() {
    _cxml_q *q = ALLOC(_cxml_q, 1);
    cxq_init_q(q);
    return q;
}

static void cxq__panic(
        _cxml_query_lexer *lexer,
        _cxml_query* query,
        const char* err)
{
    cxq_free_query(query);
    if (err){
        cxml_error("CXML Query Error: Error occurred at column %ld while"
                   " parsing query expression.\n%s\n",
                   (lexer->current - lexer->src + 1), err);
    }
}

inline static bool _cxq__is_identifier(char ch){
    return (_cxml__is_identifier(ch) || ch == ':');
}

static void cxq__expect_or_err(
        _cxml_query_lexer *lexer,
        _cxml_query* query,
        char *msg,
        int argc, ...)
{
    va_list chars;
    va_start(chars, argc);
    int expected;
    for (int i = 0; i < argc; i++)
    {
        expected = va_arg(chars, int);
        // return, if we match any of the given types
        if ((*lexer->current) == expected){
            va_end(chars);
            return;
        }
    }
    va_end(chars);
    cxq__panic(lexer, query, msg);
}

static void cxq__value(_cxml_query_lexer *lex, char target) {
    while (*lex->current != target && *lex->current)
    {
        lex->current++;
    }
    if (cxml_get_config().show_warnings){
        lex->current - lex->start == 0 ?
        fprintf(stderr, "Warning: Empty value in query expression\n") : 0;
    }
}

static void cxq__name(_cxml_query_lexer *lex, _cxml_query *query) {
    lex->start = ++lex->current;
    if (!_cxml__is_alpha(*lex->current)){
        cxq__panic(lex, query, "Not a valid element name.");
    }
    while ((*lex->current != '>')
          && (_cxq__is_identifier(*lex->current)))
    {
        lex->current++;
    }
    cxml_string_append(&query->q_name, lex->start,
                       _cxml_int_cast(lex->current - lex->start));

    // catch erroneously specified tag_name
    cxq__expect_or_err(lex, query, "Expected '>'", 1, '>');
    lex->current++;

    // ensure query ends with '/'
    cxq__expect_or_err(lex, query, "Expected '/'", 1, '/');
}

static void cxq__identifier(_cxml_query_lexer *lex, _cxml_query *q) {
    if (!_cxml__is_alpha(*lex->current)){
        cxq__panic(lex, q, "Expected identifier.");
        return;
    }
    while ((_cxq__is_identifier(*lex->current))
          && (*lex->current != '/'))
    {
        lex->current++;
    }
}

static _cxml_q* cxq__attr(_cxml_query_lexer *lex, _cxml_query *query){
    _cxml_q *q_expr = cxq__new_q();
    q_expr->q_attr = ALLOC(struct _cxml_q_attr, 1);
    cxq_init_q_attr(q_expr->q_attr);
    if (*lex->current == '@'){
        lex->current++;
        q_expr->q_attr->flags |= _CXQ_MATCH_KEY_ONLY;
    }
    lex->start = lex->current;
    cxq__identifier(lex, query);
    // store identifier value
    q_expr->q_attr->key = new_alloc_cxml_string();
    cxml_string_append(q_expr->q_attr->key, lex->start,
                       _cxml_int_cast(lex->current - lex->start));
    // @attr
    if (q_expr->q_attr->flags & _CXQ_MATCH_KEY_ONLY){
        return q_expr;
    }
    // attr='value' | attr != 'value'
    if (*lex->current == '|'){
        lex->current++;
        cxq__expect_or_err(lex, query, "Expected '='", 1, '=');
        q_expr->q_attr->flags |= _CXQ_MATCH_PARTIAL;
    }else{
        cxq__expect_or_err(lex, query, "Expected '='", 1, '=');
        q_expr->q_attr->flags |= _CXQ_MATCH_EXACT;
    }
    // move past '='
    lex->current++;
    cxq__expect_or_err(lex, query, "Expected ' or \"", 2, '\'', '\"');

    // move past '\"' || '\''
    char opening = *lex->current++;

    // value starts here
    lex->start = lex->current;
    cxq__value(lex, opening);

    // match opening quotation
    cxq__expect_or_err(lex, query, "String literal not properly closed.", 1, opening);
    q_expr->q_attr->value = new_alloc_cxml_string();
    cxml_string_append(q_expr->q_attr->value, lex->start,
                       _cxml_int_cast(lex->current - lex->start));

    // move past '\"' || '\''
    lex->current++;
    return q_expr;
}

static _cxml_q* cxq__text(_cxml_query_lexer *lex, _cxml_query *query){
    // escape '$'
    lex->current++;
    _cxml_q *q_expr = cxq__new_q();
    q_expr->q_text = ALLOC(struct _cxml_q_text, 1);
    cxq_init_q_text(q_expr->q_text);
    lex->start = lex->current;
    if (memcmp(lex->current, "text", 4) != 0){
        cxq__panic(lex, query, "Expected keyword 'text' after '$'");
    }else{
        // *lex->current == 't', so escape 'ext'
        lex->current += 4;
    }
    // $text <- match any
    if (*lex->current != '|' && *lex->current != '='){
        q_expr->q_text->flags |= _CXQ_MATCH_ANY;
        return q_expr;
    }

    // $text|='some text' <- match partial
    // $text='some text'  <- match specific
    if (*lex->current == '|'){
        // move past '|'
        lex->current++;
        cxq__expect_or_err(lex, query, "Expected '='", 1, '=');
        q_expr->q_text->flags |= _CXQ_MATCH_PARTIAL;
    }else{
        cxq__expect_or_err(lex, query, "Expected '='", 1, '=');
        q_expr->q_text->flags |= _CXQ_MATCH_EXACT;
    }

    // move past '='
    lex->current++;
    cxq__expect_or_err(lex, query, "Expected ' or \"", 2, '\'', '\"');
    // move past '\"' || '\''
    char opening = *lex->current++;
    // value starts here
    lex->start = lex->current;
    cxq__value(lex, opening);
    // match opening quotation
    cxq__expect_or_err(lex, query, "String literal not properly closed.", 1, opening);

    // store text value
    q_expr->q_text->text = new_alloc_cxml_string();
    cxml_string_append(q_expr->q_text->text, lex->start,
                       _cxml_int_cast(lex->current - lex->start));

    // move past '\"' || '\''
    lex->current++;
    return q_expr;
}

static _cxml_q* cxq__comm(_cxml_query_lexer *lex, _cxml_query *query){
    // escape '#'
    lex->current++;
    _cxml_q *q_expr = cxq__new_q();
    q_expr->q_comm = ALLOC(struct _cxml_q_comm, 1);
    cxq_init_q_comm(q_expr->q_comm);
    lex->start = lex->current;
    if (memcmp(lex->current, "comment", 7) != 0){
        cxq__panic(lex, query, "Expected keyword 'comment' after '#'");
    }else{
        // *lex->current == 'c', so escape 'omment'
        lex->current += 7;
    }
    // #comment
    if (*lex->current != '|' && *lex->current != '='){
        q_expr->q_comm->flags |= _CXQ_MATCH_ANY;
        return q_expr;
    }

    // #comment|='some comment' <- match partial
    // #comment='some comment'  <- match specific
    if (*lex->current == '|'){
        // move past '|'
        lex->current++;
        cxq__expect_or_err(lex, query, "Expected '='", 1, '=');
        q_expr->q_comm->flags |= _CXQ_MATCH_PARTIAL;
    }else{
        // #comment='some comment'
        cxq__expect_or_err(lex, query, "Expected '='", 1, '=');
        q_expr->q_comm->flags |= _CXQ_MATCH_EXACT;
    }

    // move past '='
    lex->current++;
    cxq__expect_or_err(lex, query, "Expected ' or \"", 2, '\'', '\"');
    // move past '\"' || '\''
    char opening = *lex->current++;
    // value starts here
    lex->start = lex->current;
    cxq__value(lex, opening);
    // match opening quotation
    cxq__expect_or_err(lex, query, "String literal not properly closed.", 1, opening);

    // store comment value
    q_expr->q_comm->comment = new_alloc_cxml_string();
    cxml_string_append(q_expr->q_comm->comment, lex->start,
                       _cxml_int_cast(lex->current - lex->start));

    // move past '\"' || '\''
    lex->current++;
    return q_expr;
}

// [optional='stuff'/optional2='other_stuff']/ |
static void cxq__q_optional(_cxml_query_lexer *lex, _cxml_query *query)
{
    lex->start = ++lex->current;  // move past '['
    // escape all '/'
    while (*lex->current == '/') ++lex->current;
    _cxml_q* expr = NULL;
    while (*lex->current != ']' && *lex->current) {
        if (isalpha((unsigned char)(*lex->current)) || (*lex->current == '@')){
            expr = cxq__attr(lex, query);
        }else if (*lex->current == '#'){
            expr = cxq__comm(lex, query);
        }else if (*lex->current == '$'){
            expr = cxq__text(lex, query);
        }
        cxq__expect_or_err(lex, query, "Expected '/' or ']'", 2, '/', ']');
        // save the _cxml_q node created
        cxml_list_append(&query->q_o_list, expr);
        // escape all '/'
        while (*lex->current == '/') ++lex->current;
    }
    cxq__expect_or_err(lex, query, "Expected ']'", 1, ']');
    ++lex->current;
    cxq__expect_or_err(lex, query, "Expected '/'", 1, '/');
}

// rigid='stuff'/rigid='other_stuff'/
static void cxq__q_rigid(_cxml_query *query, _cxml_query_lexer *lex) {
    // prev token must be '/' (<name>'/')
    if (*(lex->current - 1) != '/'){
        cxq__panic(lex, query, "Expected '/'.");
    }
    _cxml_q* expr = NULL;
    if (isalpha((unsigned char)(*lex->current)) || (*lex->current == '@')){
        expr = cxq__attr(lex, query);
    }else if (*lex->current == '#'){
        expr = cxq__comm(lex, query);
    }else if (*lex->current == '$'){
        expr = cxq__text(lex, query);
    }
    cxq__expect_or_err(lex, query, "Expected '/'", 1, '/');
    // save the _cxml_q node created
    cxml_list_append(&query->q_r_list, expr);
}

// <tag_name>/[optional='stuff']/attr='value'/
_cxml_query *_cxq__parse__query(_cxml_query_lexer *lexer) {
    _cxml_query *query = cxq__new_query();
    cxq__expect_or_err(lexer, query, "Expected '<'", 1, '<');
    while (true) {
        lexer->start = lexer->current;
        char ch = *lexer->current;
        if (isalpha((unsigned char)ch) || ch == '#' || ch == '$' || ch == '@') {
            cxq__q_rigid(query, lexer);
        } else if (ch == '/') {
            lexer->current++;
            continue;
        } else if (ch == '<') {
            if (!cxml_string_len(&query->q_name)){
                cxq__name(lexer, query);
            } else{
                cxq__panic(lexer, query,
                           "Multiple tags not allowed in query expression.");
            }
        } else if (ch == '[') {
            cxq__q_optional(lexer, query);
        } else if (isspace((unsigned char)ch)){
            cxq__panic(lexer, query,
                       "Whitespace is not allowed in query expression.");
        }
        else {
            break;
        }
    }

    if (*lexer->current != '\0'){
        cxq__panic(lexer, query, lexer->current);
    }
    if (*(lexer->current-1) != '/'){
        cxq__panic(lexer, query, "Query expression must terminate with a '/'.");
    }
    return query;
}

_cxml_query *cxq_parse_query(const char *query_expr) {
    _cxml_query_lexer *lexer = cxq__create_lexer(query_expr);
    _cxml_query *query = _cxq__parse__query(lexer);
    cxq_free_lexer(lexer);
    if (query){
        query->expr = query_expr;
        // catch erroneous query expressions without <element_name>
        if (!cxml_string_len(&query->q_name)){
            cxq__panic(lexer, query,
                    "Nameless <> expression, query "
                    "expression must have a name.");
        }
    }
    return query;
}

void cxq_free_lexer(_cxml_query_lexer *lexer) {
    FREE(lexer);
}

void cxq_free_query(_cxml_query *query) {
    cxml_string_free(&query->q_name);
    cxml_list *qo_list = &query->q_o_list;
    cxml_list *qr_list = &query->q_r_list;
    cxml_for_each(o_node, qo_list){
        cxq_free_q(o_node);
    }
    cxml_for_each(r_node, qr_list){
        cxq_free_q(r_node);
    }
    cxml_list_free(qo_list);
    cxml_list_free(qr_list);
    FREE(query);
}

void cxq_free_q(_cxml_q *qry) {
    // free comment
    if (qry->q_comm){
        cxml_string_free(qry->q_comm->comment);
        FREE(qry->q_comm->comment);
        FREE(qry->q_comm);
    }
    // free text
    if (qry->q_text){
        cxml_string_free(qry->q_text->text);
        FREE(qry->q_text->text);
        FREE(qry->q_text);
    }
    // free attr
    if (qry->q_attr){
        cxml_string_free(qry->q_attr->key);
        FREE(qry->q_attr->key);
        cxml_string_free(qry->q_attr->value);
        FREE(qry->q_attr->value);
        FREE(qry->q_attr);
    }
    // free _cxml_q
    FREE(qry);
}
