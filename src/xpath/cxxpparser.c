/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxdefs.h"
#include "xpath/cxxpparser.h"
#include "xpath/cxxplib.h"

extern void _cxml_xp_lexer_init(_cxml_xp_lexer *xplexer, const char *expr);

extern void _cxml_xp_token_init(_cxml_xp_token* token);

extern _cxml_xp_token _cxml_xp_get_token(_cxml_xp_lexer *xplexer);

static void _cxml_xp_p__consume(_cxml_xp_token_t type);

static void _cxml_xp_p__advance();

static void _cxml_xp_p__copy_token_v(cxml_string *str, _cxml_xp_token *token);

static void *_cxml_xp_p__pop();

static void _cxml_xp_p__push(void *node);

static bool _cxml_xp_p__stack_empty();

static void location_path();

static void absolute_location_path();

static void relative_location_path();

static void step();

static void predicate();

static void node_test();

static void function_call();

static void expression(int rbp);

static void unary();

static void binary();

static void num();

static void group();

static void str_literal();

static void _cxml_xp__err(
        _cxml_xp_token *token,
        const char *msg,
        const int *_line,
        const int *_col);


void _cxml_xpath_parser_init() {
    _xpath_parser.consume_cnt = 0;

    _xpath_parser.is_empty_nodeset = 0;

    _cxml_xp_token_init(&_xpath_parser.current_tok);

    _cxml_xp_token_init(&_xpath_parser.prev_tok);

    _cxml_stack_init(&_xpath_parser.ast_stack);

    _cxml_stack_init(&_xpath_parser.ctx_stack);

    _cxml_stack_init(&_xpath_parser.acc_stack);

    cxml_set_init(&_xpath_parser.nodeset);

    cxml_list_init(&_xpath_parser.data_nodes);

    _cxml_xp_init_context(&_xpath_parser.context);

    _cxml_cache_init(&_xpath_parser.lru_cache);

    cxml_list_init(&_xpath_parser.alloc_set_list);

    _xpath_parser.xml_namespace = NULL;
}

extern struct _cxml_xp_func_LU_val _cxml_xp_lookup_fn_name(cxml_string *name, int arity);

void cxml_xp_free_ast_nodes(_cxml_xp_parser *xpp){
    cxml_xp_fvisit(_cxml_stack__pop(&xpp->ast_stack));
}

void _cxml_xpath_parser_free(){
_CXML__TRACE(
    cxml_string acc = new_cxml_string();
    cxml_xp_bvisit(_cxml_stack__get(&_xpath_parser.ast_stack), &acc);
    printf("Built/Debugged expression: %s\n", cxml_string_as_raw(&acc));
    cxml_string_free(&acc);
)
    // free the ast nodes
    cxml_xp_free_ast_nodes(&_xpath_parser);

    // free all data nodes, and list storing the nodes
    cxml_for_each(data, &_xpath_parser.data_nodes){
        _cxml_xp_data_free(data);
    }
    cxml_list_free(&_xpath_parser.data_nodes);

    // free accumulating nodeset
    cxml_set_free(&_xpath_parser.nodeset);

    // free ast stack
    _cxml_stack_free(&_xpath_parser.ast_stack);

    // free context state stack
    _cxml_stack_free(&_xpath_parser.ctx_stack);

    // free result accumulator stack
    _cxml_stack_free(&_xpath_parser.acc_stack);

    // free lru-cache
    _cxml_cache_free(&_xpath_parser.lru_cache);

    // free the xml namespace
    cxml_ns_node_free(_xpath_parser.xml_namespace);

    // free all allocated lists used in caching nodesets,
    // as well as the outer list storing the allocated lists
    cxml_for_each(list, &_xpath_parser.alloc_set_list){
        cxml_list_free(list);
        FREE(list);
    }
    cxml_list_free(&_xpath_parser.alloc_set_list);

    // init parser
    _cxml_xpath_parser_init();
}

/**
 * advance to the next token in the token sequence,
 * i.e. get the next token.
 */
static void _cxml_xp_p__advance() {
    _xpath_parser.prev_tok = _xpath_parser.current_tok;
    _cxml_xp_token token  = _cxml_xp_get_token(&_xpath_parser.lexer);
    if ((int)token.type == 0xff){
        _cxml_xp__err(&token, "Invalid token.", NULL, NULL);
    }
    _xpath_parser.current_tok = token;
}

static void _cxml_xp_p__consume(_cxml_xp_token_t type) {
    /*
     * consume current token only if its type equals
     * the type passed in, else err
     */
    if (_xpath_parser.current_tok.type == type){
        _cxml_xp_p__advance();
        _xpath_parser.consume_cnt++;
    }
    else{
        _cxml_xp__err(&_xpath_parser.current_tok, "Unexpected token.", NULL, NULL);
    }
}

// copy an xpath token's value into a cxml_string,
static void
_cxml_xp_p__copy_token_v(cxml_string *str, _cxml_xp_token *token){
    cxml_string_append(str, token->start, token->length);
}

static void _cxml_xp__err(
        _cxml_xp_token *token,
        const char *msg,
        const int *_line,
        const int *_col)
{
    cxml_string context_msg = new_cxml_string();
    cxml_string_raw_append(&context_msg, "Found at least %d valid xpath expression tokens.\n");
    cxml_string_raw_append(&context_msg, "Token `%.*s` that caused this error "
                                         "was found at line: %d, column: %d.\n");
    if (msg){
        cxml_string_raw_append(&context_msg, "Error message: ");
        cxml_string_raw_append(&context_msg, msg);
        cxml_string_append(&context_msg, "\n", 1);
    }
    cxml_string_raw_append(&context_msg, "Hence, `%s` is not a valid xpath expression.\n");
    char* raw = cxml_string_as_raw(&context_msg);
    // beginning col is offset of token's length from col_no current position
    // since col_no will count to the end of a token before returning it as a
    // complete token to the parser.
    int col = _col ? (*_col) : (_xpath_parser.lexer.col_no - (token->length - 1));  // -1 to drop at the token's first char
    fprintf(stderr, raw,
            _xpath_parser.consume_cnt,
            token->length, token->start,
            (_line ? *_line : _xpath_parser.lexer.line_no), col,
            _xpath_parser.lexer.expr);
    cxml_string_free(&context_msg);
    _cxml_xpath_parser_free();
    exit(EXIT_FAILURE);
}

struct _cxml_xp_binding_power_LU{  // binding-power lookup-table
    short bp;
    void (*prefix)(void);
    void (*infix)(void);
};

enum pow{
    None=0,
    Twenty=20,
    Thirty=30,
    Forty=40,
    Fifty=50,
    Sixty=60,
    Seventy=70,
    Eighty=80,
    Ninety=90,
    Max=110
};

struct _cxml_xp_binding_power_LU bpow_LUTable[] = {
        {Max, function_call, NULL},                     // CXML_XP_TOKEN_NAME
        {Eighty, absolute_location_path, NULL},         // CXML_XP_TOKEN_F_SLASH
        {Eighty,  absolute_location_path, NULL},        // CXML_XP_TOKEN_DF_SLASH
        {None, relative_location_path, NULL},           // CXML_XP_TOKEN_AT
        {None, relative_location_path, NULL},           // CXML_XP_TOKEN_DOT
        {None, relative_location_path, NULL},           // CXML_XP_TOKEN_D_DOT
        {None,  NULL, NULL},                            // CXML_XP_TOKEN_R_BRACKET
        {None,  group, NULL},                           // CXML_XP_TOKEN_L_BRACKET
        {None,  num, NULL},                             // CXML_XP_TOKEN_NUMBER
        {Ninety,  NULL, NULL},                          // CXML_XP_TOKEN_L_SQR_BRACKET
        {None, NULL, NULL},                             // CXML_XP_TOKEN_R_SQR_BRACKET
        {Seventy, NULL, binary},                        // CXML_XP_TOKEN_PIPE
        {None,  str_literal, NULL},                     // CXML_XP_TOKEN_LITERAL
        {None, NULL, NULL},                             // CXML_XP_TOKEN_COMMA
        {None, NULL, NULL},                             // CXML_XP_TOKEN_COLON
        {Forty,  NULL, binary},                         // CXML_XP_TOKEN_LTHAN
        {Forty,  NULL, binary},                         // CXML_XP_TOKEN_GTHAN
        {Forty, NULL, binary},                          // CXML_XP_TOKEN_EQ
        {Forty, NULL, binary},                          // CXML_XP_TOKEN_LTHAN_EQ
        {Forty, NULL, binary},                          // CXML_XP_TOKEN_GTHAN_EQ
        {Forty, NULL, binary},                          // CXML_XP_TOKEN_NOT_EQ
        {Fifty, unary, binary},                         // CXML_XP_TOKEN_PLUS
        {Fifty, unary, binary},                         // CXML_XP_TOKEN_MINUS
        {Sixty, relative_location_path, binary},        // CXML_XP_TOKEN_STAR
        {Thirty, function_call, binary},                // CXML_XP_TOKEN_AND
        {Twenty, function_call, binary},                // CXML_XP_TOKEN_OR
        {Sixty, function_call, binary},                 // CXML_XP_TOKEN_MOD
        {Sixty, function_call, binary},                 // CXML_XP_TOKEN_DIV
        {None, relative_location_path, NULL},           // CXML_XP_TOKEN_TEXT_F
        {None, relative_location_path, NULL},           // CXML_XP_TOKEN_COMMENT_F
        {None,  relative_location_path, NULL},          // CXML_XP_TOKEN_PI_F
        {None, relative_location_path, NULL},           // CXML_XP_TOKEN_NODE_F
        {None, NULL, NULL}                              // CXML_XP_TOKEN_END
};


void _cxml_xp_p__push(void *node){
    // add an element node to the top of the stack
    _cxml_stack__push(&_xpath_parser.ast_stack, node);
}

void * _cxml_xp_p__pop(){
    // pop an element node off the stack
    return _cxml_stack__pop(&_xpath_parser.ast_stack);
}

bool _cxml_xp_p__stack_empty(){
    return _cxml_stack_is_empty(&_xpath_parser.ast_stack);
}

cxml_xp_op _cxml_xp_get_op(_cxml_xp_token_t type){
    switch(type){
        case CXML_XP_TOKEN_MINUS:       return CXML_XP_OP_MINUS;
        case CXML_XP_TOKEN_PLUS:        return CXML_XP_OP_PLUS;
        case CXML_XP_TOKEN_STAR:        return CXML_XP_OP_MULT;
        case CXML_XP_TOKEN_EQ:          return CXML_XP_OP_EQ;
        case CXML_XP_TOKEN_DIV:         return CXML_XP_OP_DIV;
        case CXML_XP_TOKEN_MOD:         return CXML_XP_OP_MOD;
        case CXML_XP_TOKEN_LTHAN_EQ:    return CXML_XP_OP_LEQ;
        case CXML_XP_TOKEN_GTHAN_EQ:    return CXML_XP_OP_GEQ;
        case CXML_XP_TOKEN_NOT_EQ:      return CXML_XP_OP_NEQ;
        case CXML_XP_TOKEN_GTHAN:       return CXML_XP_OP_GT;
        case CXML_XP_TOKEN_LTHAN:       return CXML_XP_OP_LT;
        case CXML_XP_TOKEN_AND:         return CXML_XP_OP_AND;
        case CXML_XP_TOKEN_OR:          return CXML_XP_OP_OR;
        case CXML_XP_TOKEN_PIPE:        return CXML_XP_OP_PIPE;
        default: return 0xff;
    }
}

static cxml_xp_astnode* new_astnode(){
    cxml_xp_astnode* node = ALLOC(cxml_xp_astnode, 1);
    node->type = CXML_XP_AST_OBJECT;
    node->wrapped_type = CXML_XP_AST_NIL;
    return node;
}

static cxml_xp_unaryop* new_unary(){
    cxml_xp_unaryop* unary = ALLOC(cxml_xp_unaryop, 1);
    unary->type = CXML_XP_AST_UNARYOP_NODE;
    unary->op = -1;
    unary->node = NULL;
    return unary;
}

static cxml_xp_num* new_num(){
    cxml_xp_num* num = ALLOC(cxml_xp_num, 1);
    num->type = CXML_XP_AST_NUM_NODE;
    cxml_string_init(&num->val);
    return num;
}

static cxml_xp_functioncall* new_function_call(){
    cxml_xp_functioncall* func_call = ALLOC(cxml_xp_functioncall, 1);
    func_call->type = CXML_XP_AST_FUNCTION_CALL_NODE;
    func_call->pos = -1;
    cxml_string_init(&func_call->name);
    cxml_list_init(&func_call->args);
    return func_call;
}

static cxml_xp_path* new_path(){
    cxml_xp_path* path_node = ALLOC(cxml_xp_path, 1);
    path_node->type = CXML_XP_AST_PATH_NODE;
    path_node->from_predicate = 0;
    cxml_list_init(&path_node->steps);
    return path_node;
}

static cxml_xp_predicate* new_predicate(){
    cxml_xp_predicate* pred = ALLOC(cxml_xp_predicate, 1);
    pred->type = CXML_XP_AST_PREDICATE_NODE;
    pred->expr_node = NULL;
    return pred;
}

static cxml_xp_string* new_str_literal(){
    cxml_xp_string* literal = ALLOC(cxml_xp_string, 1);
    literal->type = CXML_XP_AST_STR_LITERAL_NODE;
    cxml_string_init(&literal->str);
    return literal;
}

static cxml_xp_nodetest* new_node_test(){
    cxml_xp_nodetest* nt = ALLOC(cxml_xp_nodetest, 1);
    nt->type = CXML_XP_AST_NODETEST_NODE;
    return nt;
}

static cxml_xp_step* new_step(){
    cxml_xp_step* step = ALLOC(cxml_xp_step, 1);
    step->type = CXML_XP_AST_STEP_NODE;
    step->abbrev_step = 0;
    step->has_attr_axis = 0;
    step->node_test = NULL;
    step->path_spec = 0;
    cxml_list_init(&step->predicates);
    return step;
}

static cxml_xp_binaryop* new_binary(){
    cxml_xp_binaryop* binary = ALLOC(cxml_xp_binaryop, 1);
    binary->type = CXML_XP_AST_BINOP_NODE;
    binary->op = -1;
    binary->l_node = NULL;
    binary->r_node = NULL;
    return binary;
}

/*** symbols & productions ***/

void num(){
    cxml_xp_num* num = new_num();
    _cxml_xp_p__copy_token_v(&num->val, &_xpath_parser.current_tok);
    _cxml_xp_p__consume(CXML_XP_TOKEN_NUMBER);
    cxml_xp_astnode* node = new_astnode();
    node->wrapped_node.num = num;
    node->wrapped_type = CXML_XP_AST_NUM_NODE;
    _cxml_xp_p__push(node);
}

void str_literal(){
    cxml_xp_string* literal = new_str_literal();
    cxml_string_append(&literal->str, _xpath_parser.current_tok.start + 1,
                       _xpath_parser.current_tok.length - 2);
    _cxml_xp_p__consume(CXML_XP_TOKEN_LITERAL);
    cxml_xp_astnode* node = new_astnode();
    node->wrapped_node.str_literal = literal;
    node->wrapped_type = CXML_XP_AST_STR_LITERAL_NODE;
    _cxml_xp_p__push(node);
}

void unary(){
    cxml_xp_unaryop* unary = new_unary();
    cxml__assert((_xpath_parser.current_tok.type == CXML_XP_TOKEN_MINUS ||
                 _xpath_parser.current_tok.type == CXML_XP_TOKEN_PLUS),
                 "Expected '+' or '-'")
    _cxml_xp_token tok = _xpath_parser.current_tok;
    unary->op = _cxml_xp_get_op(tok.type);
    if (unary->op == 0xff){
        _cxml_xp__err(&tok, "Unexpected operator.", NULL, NULL);
    }
    _cxml_xp_p__consume(tok.type);
    expression(bpow_LUTable[tok.type].bp << 1);
    unary->node = _cxml_xp_p__pop();
    cxml_xp_astnode* node = new_astnode();
    node->wrapped_node.unary = unary;
    node->wrapped_type = CXML_XP_AST_UNARYOP_NODE;
    _cxml_xp_p__push(node);
}

void binary(){
    cxml_xp_astnode* left = _cxml_xp_p__pop();
    _cxml_xp_token op_token = _xpath_parser.current_tok;
    cxml_xp_op op = _cxml_xp_get_op(op_token.type);
    if (op == 0xff){   // fail fast
        int col = _xpath_parser.lexer.col_no - (op_token.length - 1);
        _cxml_xp__err(&op_token, "Unknown operator.", &_xpath_parser.lexer.line_no, &col);
    }
    _cxml_xp_p__consume(op_token.type);
    expression(bpow_LUTable[op_token.type].bp);
    cxml_xp_astnode* right = _cxml_xp_p__pop();
    cxml_xp_binaryop* binary = new_binary();
    binary->op = op;
    binary->l_node = left;
    binary->r_node = right;
    cxml_xp_astnode* node = new_astnode();
    node->wrapped_node.binary = binary;
    node->wrapped_type = CXML_XP_AST_BINOP_NODE;
    _cxml_xp_p__push(node);
}

void group(){
    _cxml_xp_p__consume(CXML_XP_TOKEN_L_BRACKET);
    expression(0);
    _cxml_xp_p__consume(CXML_XP_TOKEN_R_BRACKET);
}


static void prefix(){
    _cxml_xp_token token = _xpath_parser.current_tok;
    void (*func)() = bpow_LUTable[token.type].prefix;
    if (func){
        func();
    }else{
        int col = _xpath_parser.lexer.col_no - (token.length - 1);
        _cxml_xp__err(&token, "Token found at unexpected position.", &_xpath_parser.lexer.line_no, &col);
    }
}

static void infix(){
    _cxml_xp_token token = _xpath_parser.current_tok;
    void (*func)() = bpow_LUTable[token.type].infix;
    if (func){
        func();
    }else{
        int col = _xpath_parser.lexer.col_no - (token.length - 1);
        _cxml_xp__err(&token, "Token found at unexpected position.", &_xpath_parser.lexer.line_no, &col);
    }
}

void expression(int rbp){
    prefix();
    while (rbp < bpow_LUTable[_xpath_parser.current_tok.type].bp){
        infix();
    }
}

inline static bool _is_kwd_token(_cxml_xp_token *token){
    return (token->type == CXML_XP_TOKEN_AND
            || token->type == CXML_XP_TOKEN_MOD
            || token->type == CXML_XP_TOKEN_OR
            || token->type == CXML_XP_TOKEN_DIV);
}


/*
 * FunctionCall ::= FunctionName '(' ( Argument ( ',' Argument )* )? ')'
   Argument ::= Expr
   FunctionName ::= QName
 */
void function_call(){
    // save name for lookup
    _cxml_xp_token token = _xpath_parser.current_tok;
    int line = _xpath_parser.lexer.line_no;
    int col = _xpath_parser.lexer.col_no - (token.length - 1);
    // when keywords `and`, `or`, `mod`, or `div` are used as names, we get to this point
    if (_is_kwd_token(&token))
    {
        // simply modify the token types to be of type `name`
        token.type = _xpath_parser.current_tok.type = CXML_XP_TOKEN_NAME;
    }
    _cxml_xp_p__consume(CXML_XP_TOKEN_NAME);
    if (_xpath_parser.current_tok.type == CXML_XP_TOKEN_L_BRACKET)
    {
        cxml_xp_functioncall* func = new_function_call();
        _cxml_xp_p__copy_token_v(&func->name, &_xpath_parser.prev_tok);
        _cxml_xp_p__consume(CXML_XP_TOKEN_L_BRACKET);
        if (_xpath_parser.current_tok.type != CXML_XP_TOKEN_R_BRACKET)
        {
            expression(0);
            cxml_list_append(&func->args, _cxml_xp_p__pop());
            while (_xpath_parser.current_tok.type == CXML_XP_TOKEN_COMMA)
            {
                _cxml_xp_p__consume(CXML_XP_TOKEN_COMMA);
                expression(0);
                cxml_list_append(&func->args, _cxml_xp_p__pop());
            }
        }
        _cxml_xp_p__consume(CXML_XP_TOKEN_R_BRACKET);

        // validate function
        int arity = cxml_list_size(&func->args);
        struct _cxml_xp_func_LU_val ret = _cxml_xp_lookup_fn_name(&func->name, arity);
        if (ret.pos == -1)  // function wasn't found
        {
            // err
            _cxml_xp__err(&token, "Not a valid cxml xpath function.", &line, &col);
            // exits
        }
        else if (ret.pos == -2)  // function was found but with different arity
        {
            // err
            char buff[150];
            snprintf(buff, 150,
                     "Function invoked with wrong number of arguments.\n"
                     "Expected %d number of %s, got %d.",
                     abs(ret.arity), ((abs(ret.arity) > 1) ? "arguments" : "argument"),
                     arity);
            // exits
            _cxml_xp__err(&token, buff, &line, &col);
        }
        func->pos = ret.pos;
        func->ret_type = ret.ret_type;
        cxml_xp_astnode* node = new_astnode();
        node->wrapped_type = CXML_XP_AST_FUNCTION_CALL_NODE;
        node->wrapped_node.func_call = func;
        _cxml_xp_p__push(node);
    }
    else{
        relative_location_path();
    }
}

/*
 *
 predicate ::= Step | '/' Step | '//' Step | PrimaryExpr | PrimaryExpr Predicate
 primaryexpr ::= expression
 expression ::=
                (absolute_path_location)
                (relative_path_location)
                (function-call)
                (string)
                (number)

 * PredicateExpr ::= Expr

 * Predicate ::= '[' PredicateExpr ']' -> '[' Expr ']'

'@'? NodeTest Predicate* | ('.' | '..')
 */
void predicate(){
    _cxml_xp_p__consume(CXML_XP_TOKEN_L_SQR_BRACKET);
    // set flag to determine if the parsed path nodes resides inside a predicate node
    _xpath_parser.from_predicate = true;
    expression(0);
    // turn off flag
    _xpath_parser.from_predicate = false;
    _cxml_xp_p__consume(CXML_XP_TOKEN_R_SQR_BRACKET);
    cxml_xp_predicate* pred = new_predicate();
    pred->expr_node = _cxml_xp_p__pop();
    // no need to wrap in ASTNode since predicate specifically goes into cxml_xp_step node's
    // predicate field
    _cxml_xp_p__push(pred);
}

inline static void _set_lname(cxml_name *name, int lname_len){
    // for setting only local name in name object
    name->lname = cxml_string_as_raw(&name->qname);
    name->lname_len = lname_len;
}

inline static void _set_pname(cxml_name *name, int pname_len){
    // for setting only prefix name in name object
    name->pname = cxml_string_as_raw(&name->qname);
    name->pname_len = pname_len;
}

inline static void _set_name(cxml_name *name, int pname_len, int lname_len){
    // for when name has a prefix
    name->pname = cxml_string_as_raw(&name->qname);
    name->lname = name->pname + pname_len + 1;
    name->pname_len = pname_len;
    name->lname_len = lname_len;
}

static void
_store_name_test(cxml_xp_nodetest* nt_node, _cxml_xp_token* token){
    nt_node->t_type = CXML_XP_NODE_TEST_NAMETEST;
    cxml_name_init(&nt_node->name_test.name);
    /*
     * possible forms:
     * 1) 'prefix' : 'name'     CXML_XP_NODE_TEST_TPNAME_TLNAME
     * 2)  * : 'name'           CXML_XP_NODE_TEST_TWILDCARD_TLNAME
     * 3) 'prefix' : *          CXML_XP_NODE_TEST_TPNAME_TWILDCARD
     * 4)  *                    CXML_XP_NODE_TEST_TWILDCARD
     * 5) 'name'                CXML_XP_NODE_TEST_TNAME
     */
    cxml_xp_nametest *nametest = &nt_node->name_test;
    if (token->type == CXML_XP_TOKEN_STAR){  // either star or name
        if (_xpath_parser.current_tok.type == CXML_XP_TOKEN_COLON){
            // wildcard name (2)
            _cxml_xp_p__consume(CXML_XP_TOKEN_COLON);
            _cxml_xp_p__consume(CXML_XP_TOKEN_NAME);
            nametest->t_type = CXML_XP_NAME_TEST_WILDCARD_LNAME;
            cxml_string_append(&nametest->name.qname, _xpath_parser.prev_tok.start,
                               _xpath_parser.prev_tok.length);
            _set_lname(&nametest->name, _xpath_parser.prev_tok.length);
        }else{
            // wildcard (4)
            nametest->t_type = CXML_XP_NAME_TEST_WILDCARD;
        }
    }else{ // name
        if (_xpath_parser.current_tok.type == CXML_XP_TOKEN_COLON){
            _cxml_xp_p__consume(CXML_XP_TOKEN_COLON);
            if (_xpath_parser.current_tok.type == CXML_XP_TOKEN_STAR){
                // prefix wildcard (3)
                _cxml_xp_p__consume(CXML_XP_TOKEN_STAR);
                // we store the prefix in qname, only to ensure that pname has
                // something to point to. (since this is a prefix wildcard state -> pf:*).
                cxml_string_append(&nametest->name.qname, token->start,
                                   token->length);
                _set_pname(&nametest->name, token->length);
                nametest->t_type = CXML_XP_NAME_TEST_PNAME_WILDCARD;
            }else if (_xpath_parser.current_tok.type == CXML_XP_TOKEN_NAME
                      || _is_kwd_token(&_xpath_parser.current_tok))
            {
                // prefix name (1)
                _cxml_xp_p__consume(_xpath_parser.current_tok.type);
                nametest->t_type = CXML_XP_NAME_TEST_PNAME_LNAME;
                // store the qualified name (QName) in `name` field
                cxml_string_append(&nametest->name.qname, token->start,
                                   token->length);
                cxml_string_append(&nametest->name.qname, ":", 1);
                cxml_string_append(&nametest->name.qname, _xpath_parser.prev_tok.start,
                                   _xpath_parser.prev_tok.length);
                // set the local name and prefix name
                _set_name(&nametest->name, token->length, _xpath_parser.prev_tok.length);
            }
        }else{
            // name (5)
            nametest->t_type = CXML_XP_NAME_TEST_NAME;
            cxml_string_append(&nametest->name.qname, token->start,
                               token->length);
            _set_lname(&nametest->name, token->length);
        }
    }
}

static void _store_type_test(cxml_xp_nodetest* nt_node){
    // store the node-test type
    nt_node->t_type = CXML_XP_NODE_TEST_TYPETEST;
    // processing-instruction
    if (_xpath_parser.current_tok.type == CXML_XP_TOKEN_PI_F)
    {
        nt_node->type_test.t_type = CXML_XP_TYPE_TEST_PI;
        _cxml_xp_p__advance();  // move past CXML_XP_TOKEN_PI_F
        _cxml_xp_p__consume(CXML_XP_TOKEN_L_BRACKET);
        if (_xpath_parser.current_tok.type == CXML_XP_TOKEN_LITERAL)
        {
            cxml_string_init(&nt_node->type_test.target);
            cxml_string_append(&nt_node->type_test.target,
                               _xpath_parser.current_tok.start + 1,
                               _xpath_parser.current_tok.length - 2);
            _cxml_xp_p__advance();  // move past CXML_XP_TOKEN_LITERAL
            nt_node->type_test.has_target = 1;
        }else{
            nt_node->type_test.has_target = 0;
        }
        _cxml_xp_p__consume(CXML_XP_TOKEN_R_BRACKET);
    }
    else // others (node, text, comment)
    {
        nt_node->type_test.has_target = 0;
        switch(_xpath_parser.current_tok.type)
        {
            case CXML_XP_TOKEN_TEXT_F:
                nt_node->type_test.t_type = CXML_XP_TYPE_TEST_TEXT;
                break;
            case CXML_XP_TOKEN_COMMENT_F:
                nt_node->type_test.t_type = CXML_XP_TYPE_TEST_COMMENT;
                break;
            case CXML_XP_TOKEN_NODE_F:
                nt_node->type_test.t_type = CXML_XP_TYPE_TEST_NODE;
                break;
            default: break;
        }
        _cxml_xp_p__consume(_xpath_parser.current_tok.type);
        _cxml_xp_p__consume(CXML_XP_TOKEN_L_BRACKET);
        _cxml_xp_p__consume(CXML_XP_TOKEN_R_BRACKET);
    }
}

void node_test(){
    cxml_xp_step* node = _cxml_xp_p__pop();
    cxml_xp_nodetest* nt_node = new_node_test();
    _cxml_xp_token tok;
    // dropping from function call lands here..
    if (_xpath_parser.prev_tok.type == CXML_XP_TOKEN_NAME)
    {
        tok = _xpath_parser.prev_tok;
        _store_name_test(nt_node, &tok);
    }
    // name-test
    else if (_xpath_parser.current_tok.type == CXML_XP_TOKEN_NAME
            || _xpath_parser.current_tok.type == CXML_XP_TOKEN_STAR
            || _is_kwd_token(&_xpath_parser.current_tok)) // keywords `and`, `or`, etc. could also be used as valid names
    {
        _cxml_xp_p__consume(_xpath_parser.current_tok.type);
        tok = _xpath_parser.prev_tok;
        _store_name_test(nt_node, &tok);
    }
    // type-test
    else if (_xpath_parser.current_tok.type == CXML_XP_TOKEN_NODE_F
            || _xpath_parser.current_tok.type == CXML_XP_TOKEN_COMMENT_F
            || _xpath_parser.current_tok.type == CXML_XP_TOKEN_TEXT_F
            || _xpath_parser.current_tok.type == CXML_XP_TOKEN_PI_F)
    {
        _store_type_test(nt_node);
    }
    else
    {
        _cxml_xp__err(
                _xpath_parser.current_tok.length &&  // current has length?
                *_xpath_parser.current_tok.start ?   // current is not eof?
                &_xpath_parser.current_tok :        // then use current
                &_xpath_parser.prev_tok,            // else use previous
                "Expected node test.", NULL, NULL);
    }
    // '@' attr axis is simply a modifier to the node-test
    nt_node->has_attr_axis = node->has_attr_axis;
    node->node_test = nt_node;
    // no need to wrap in ASTNode since node_test specifically goes into cxml_xp_step node's
    // node_test field
    _cxml_xp_p__push(node);
}

inline static short get_path_spec(_cxml_xp_token_t type){
    if (type == CXML_XP_TOKEN_F_SLASH){
        return 1;
    }else if (type == CXML_XP_TOKEN_DF_SLASH){
        return 2;
    }
    return 0;
}

/*
 * Step ::= '@'? NameTest Predicate* | ('.' | '..')
 */
void step(){
    cxml_xp_step* step_node = new_step();
    step_node->path_spec = get_path_spec(_xpath_parser.prev_tok.type);
    // ('.' | '..')
    if (_xpath_parser.current_tok.type == CXML_XP_TOKEN_DOT
    || _xpath_parser.current_tok.type == CXML_XP_TOKEN_D_DOT)
    {
        // '.' -> 1 | '..' -> 2 | None -> 0
        step_node->abbrev_step = (_xpath_parser.current_tok.type == CXML_XP_TOKEN_DOT) ? 1 : 2;
        _cxml_xp_p__consume(_xpath_parser.current_tok.type);
    }
    else{
        if (_xpath_parser.current_tok.type == CXML_XP_TOKEN_AT)
        {
            _cxml_xp_p__consume(CXML_XP_TOKEN_AT);
            step_node->has_attr_axis = 1;
        }
        // push for parsing other nodes into it
        _cxml_xp_p__push(step_node);
        // NameTest
        node_test();
        // pop
        _cxml_xp_p__pop();
        // Predicate
        while (_xpath_parser.current_tok.type == CXML_XP_TOKEN_L_SQR_BRACKET)
        {
            predicate();
            cxml_list_append(&step_node->predicates, _cxml_xp_p__pop());
        }
    }
    // no need to wrap in ASTNode since cxml_xp_step node goes directly into steps
    // field of cxml_xp_path node.
    _cxml_xp_p__push(step_node);
}

/*
 * RelativeLocationPath ::= Step
                        | RelativeLocationPath '/' Step
                        | RelativeLocationPath '//' Step
 */
void relative_location_path(){
    cxml_xp_path* path_node = new_path();
    path_node->from_predicate = _xpath_parser.from_predicate;
    step();
    if (!_cxml_xp_p__stack_empty()){
        cxml_list_append(&path_node->steps, _cxml_xp_p__pop());
    }
    while (_xpath_parser.current_tok.type == CXML_XP_TOKEN_F_SLASH
          || _xpath_parser.current_tok.type == CXML_XP_TOKEN_DF_SLASH)
    {
        _cxml_xp_p__consume(_xpath_parser.current_tok.type);
        step();
        cxml_list_append(&path_node->steps, _cxml_xp_p__pop());
    }
    cxml_xp_astnode* node = new_astnode();
    node->wrapped_type = CXML_XP_AST_PATH_NODE;
    node->wrapped_node.path = path_node;
    _cxml_xp_p__push(node);
}

// helper function for transforming abbreviated step nodes in any of
// the form:
// '/' | '[/]' | '(/)' into '/.' | '[/.]' | '(/.)'
static void _abbrev_step(){
    cxml_xp_step* step_node = new_step();
    step_node->path_spec = get_path_spec(_xpath_parser.prev_tok.type);
    step_node->abbrev_step = 1;
    cxml_xp_path *path_node = new_path();
    path_node->from_predicate = _xpath_parser.from_predicate;
    cxml_list_append(&path_node->steps, step_node);
    cxml_xp_astnode* node = new_astnode();
    node->wrapped_type = CXML_XP_AST_PATH_NODE;
    node->wrapped_node.path = path_node;
    _cxml_xp_p__push(node);
}

/*
 * AbsoluteLocationPath ::= '/' RelativeLocationPath?
                        | '//' RelativeLocationPath
 */
void absolute_location_path(){
    if (_xpath_parser.current_tok.type == CXML_XP_TOKEN_F_SLASH
    || _xpath_parser.current_tok.type == CXML_XP_TOKEN_DF_SLASH)
    {
        _cxml_xp_p__consume(_xpath_parser.current_tok.type);
        // sub-expr -> '(/)' || '[/]' || '/'
        if ((_xpath_parser.prev_tok.type == CXML_XP_TOKEN_F_SLASH)
            && ((_xpath_parser.current_tok.type == CXML_XP_TOKEN_END)
                || (_xpath_parser.current_tok.type == CXML_XP_TOKEN_PIPE)
                || (_xpath_parser.current_tok.type == CXML_XP_TOKEN_R_BRACKET)
                || (_xpath_parser.current_tok.type == CXML_XP_TOKEN_R_SQR_BRACKET)
                || (_xpath_parser.current_tok.type == CXML_XP_TOKEN_COMMA)))
        {
            _abbrev_step();
            return;
        }
    }
    relative_location_path();
}

/*
 * LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 */
void location_path() {
    absolute_location_path();
    while (_xpath_parser.current_tok.type == CXML_XP_TOKEN_PIPE){
        binary();
    }
}

/*
 * QueryString  ::=     "'" LocationPath  "'"
 */

void query_string(const char *query_string) {
    _cxml_xp_lexer_init(&_xpath_parser.lexer, query_string);
    _cxml_xpath_parser_init();
    _cxml_xp_p__advance();
    /***/
    location_path();
    _cxml_xp_p__consume(CXML_XP_TOKEN_END);
}
