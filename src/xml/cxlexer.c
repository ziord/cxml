/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxdefs.h"
#include "xml/cxlexer.h"
#include "utils/cxutf8hook.h"

#define TOKENTYPE_TO_STR(token_type)    (#token_type)

/********************stream***********************/
extern void _cxml__adjust_stream_buffer(_cxml_lexer *cxlexer);

extern void _cxml__reset_stream_buffer(_cxml_lexer *cxlexer);
/*******************************************/

inline static char get_char(_cxml_lexer *cxlexer);

inline static void cxml__move(_cxml_lexer *cxlexer);

static void skip_whitespace(_cxml_lexer *cxlexer);

static _cxml_token new_cxml_token(_cxml_lexer *cxlexer, _cxml_token_t type);

static _cxml_token_t lex_attr_identifier(_cxml_lexer *cxlexer);

static _cxml_token_t lex_value(_cxml_lexer *cxlexer);

static _cxml_token lex_string(_cxml_lexer *cxlexer);

static _cxml_token new_cxml_err_token(_cxml_lexer *cxlexer, const char *msg);

static _cxml_token lex_comment(_cxml_lexer *cxlexer);

static void set_type(_cxml_token *token);

static void _cxml__read(_cxml_lexer *cxlexer);


inline static bool is_digit(char ch) {
    return (ch >= '0' && ch <= '9');
}

inline static bool is_at_end(_cxml_lexer *cxlexer) {
    return *(cxlexer->current) == '\0';
}

inline static char peek(_cxml_lexer *cxlexer) {
    return *cxlexer->current;
}

inline static char peek_next(_cxml_lexer *cxlexer, int i) {
    if (is_at_end(cxlexer)) return *(cxlexer->current);
    return *(cxlexer->current + i);
}

inline static char peek_prev(_cxml_lexer *cxlexer, int i) {
    return *(cxlexer->current - i);
}

inline static bool has_utf8_bom(const char *str){
    int idx = 0;
    // 0xEF -> 0b11101111
    // 0xBB -> 0b10111011
    // 0xBF -> 0b10111111
    return (((unsigned char)str[idx++] & 0XFF)    == 0xEF
            && ((unsigned char)str[idx++] & 0XFF) == 0xBB
            && ((unsigned char)str[idx] & 0XFF)   == 0xBF);
}

void _cxml_lexer_init(
        _cxml_lexer *cxlexer,
        const char *source,
        const char *filename,
        bool stream)
{
    cxlexer->cfg = cxml_get_config();
    if (stream && filename){
        _cxml_stream_init(&cxlexer->_stream_obj, filename, cxlexer->cfg.chunk_size);
        cxlexer->start = cxlexer->current = cxlexer->_stream_obj._stream_buff;
        // copy some (_cxml_config_gb.chunk_size) bytes into the buffer in preparation for lexing.
        _cxml__read(cxlexer);
    }else{
        cxlexer->start = (void*)source;
        cxlexer->current = (void*)source;
    }
    // escape utf-8 byte order mark if present.
    if (cxlexer->current && has_utf8_bom(cxlexer->current)){
        cxlexer->current += 3;
        cxlexer->start = cxlexer->current;
    }
    cxlexer->line = 1;
    cxlexer->error = 0;
    cxlexer->vflag = 0;
    cxlexer->preserve_sp = cxlexer->cfg.preserve_space;
    cxlexer->preserve_cm = cxlexer->cfg.preserve_comment;
    cxlexer->preserve_cd = cxlexer->cfg.preserve_cdata;
    cxlexer->_should_stream = stream;
    cxlexer->_stream = stream;
    cxlexer->_returned = 0;
}

void _cxml_lexer_close(_cxml_lexer *cxlexer){
    if (cxlexer->_stream){
        _cxml__close_stream(&cxlexer->_stream_obj);
    }
    _cxml_lexer_init(cxlexer, NULL, NULL, 0);
}

void _cxml_token_init(_cxml_token *token) {
    token->literal_type = CXML_NON_LITERAL;
}

inline static int get_length(_cxml_lexer *cxlexer) {
    if (!cxlexer->preserve_sp) {
        int spaces = 0, len = _cxml_int_cast(cxlexer->current - cxlexer->start);
        for (int i = len, j = 1; i > 0; i--, j++) { // trim ending spaces
            if (isspace((unsigned char)*(cxlexer->current - j )))
            {
                spaces++;
            } else {
                break;
            }
        }
        while (isspace((unsigned char)*cxlexer->start)) {  // trim beginning spaces
            cxlexer->start++;
        }
        return _cxml_int_cast(cxlexer->current - cxlexer->start - spaces);
    }
    return _cxml_int_cast(cxlexer->current - cxlexer->start);
}

inline static bool is_volatile_type(_cxml_token_t type){
    // types whose values (`start` field) might still be needed/useful after the
    // tokens have been consumed.
    return (type == CXML_TOKEN_IDENTIFIER)
            || (type == CXML_TOKEN_STRING);
}

static _cxml_token new_cxml_token(_cxml_lexer *cxlexer, _cxml_token_t type) {
    _cxml_token token;
    token.line = cxlexer->line;
    token.type = type;
    if (type == CXML_TOKEN_TEXT) {
        token.length = get_length(cxlexer);
    } else {
        token.length = _cxml_int_cast(cxlexer->current - cxlexer->start);
    }
    if (cxlexer->_stream && is_volatile_type(type))
    {
        /*
         * since in `stream` mode, the stream buffer might be resized.
         * which might cause the previously created tokens to point to garbage,
         * we allocate the value of each `volatile` token type, in order to prevent
         * this. There's no need for this in regular mode, since we have a
         * pointer to the entire string in memory.
        */
        token.start = ALLOC(char, (token.length + 1));
        memcpy(token.start, cxlexer->start, token.length);
    }else{
        token.start = cxlexer->start;
    }
    if (token.type == CXML_TOKEN_STRING){
        token.length--;  // escape ending quotation '"' | '\''
        set_type(&token);
    }
    else if (token.type == CXML_TOKEN_TEXT){
        set_type(&token);
    }else{
        token.literal_type = CXML_NON_LITERAL;
    }
    return token;
}

static _cxml_token new_cxml_err_token(_cxml_lexer *cxlexer, const char *msg) {
    cxlexer->error = true;
    _cxml_token token = new_cxml_token(cxlexer, CXML_TOKEN_ERROR);
    int msg_len = _cxml_int_cast strlen(msg);
    int len =  msg_len + token.length + 2;
    char *temp = CALLOC(char, len);
    strncpy(temp, msg, msg_len);
    strncat(temp + msg_len, token.start, token.length);
    token.start = temp;
    token.length = len;
    return token;
}

//<!-- comment -- >
static _cxml_token lex_comment(_cxml_lexer *cxlexer) {
    _cxml_token dummy_tok = {.line=cxlexer->line};

     /*
      * start_offset simply stores the byte offset between
      * current and start, which would later be used to
      * set start to `current`'s previous position after the
      * comment has been successfully lexed.
      * We could simply set a char* start to cxlexer.current here,
      * however, we run the risk of invalidating such pointer when
      * cxml__move() is called in stream mode (due to reallocation/resizing)
      */

    // move from ! to --
    cxml__move(cxlexer);
    if ((peek(cxlexer) == '-' && peek_next(cxlexer, 1) == '-')) {
        // - - - -
        cxml__move(cxlexer);
        cxml__move(cxlexer);

        while (peek(cxlexer) != '-' && !is_at_end(cxlexer)) {
            cxml__move(cxlexer);

            if (*cxlexer->current == '-' && *(cxlexer->current + 1) != '-') {
                cxml__move(cxlexer);
            }
        }
        if (is_at_end(cxlexer)) {
            cxlexer->error = true;
            return dummy_tok;
        }
        if (peek(cxlexer) == '-' && peek_next(cxlexer, 1) == '-') {
            // - - - -
            cxml__move(cxlexer);
            cxml__move(cxlexer);

            // some comments may not be fully closed e.g. <!---- missing '>'
            if (peek(cxlexer) != '>'){
                cxlexer->error = true;
            }else{
                cxml__move(cxlexer); // move past '>'
            }
        } else {
            cxlexer->error = true;
        }
    }
    // stop lexing immediately
    if (cxlexer->error) return dummy_tok;
    if (cxlexer->preserve_cm) {
        dummy_tok = new_cxml_token(cxlexer, CXML_TOKEN_COMMENT);
    }
    !cxlexer->preserve_cm ? skip_whitespace(cxlexer) : (void)0;
    return dummy_tok;
}

static void skip_whitespace(_cxml_lexer *cxlexer) {
    // if we're at the start of a value (text), and
    // the preserve space flag is on, we do not skip spaces
    if (cxlexer->vflag == '>' && cxlexer->preserve_sp) return;
    while (isspace((unsigned char)peek(cxlexer))) {
        cxml__move(cxlexer);
    }
}

static void _cxml__read(_cxml_lexer *cxlexer){
     /*
      * we need to read from file when cxlexer.current approaches
      * end of valid text in the stream buffer (already read from file)
      * which is when the lexer's current position (cxlexer.current) from the
      * beginning of the stream buffer is less than about 10 bytes when subtracted from
      * chars_read_into_sbuff (which represents the number of chars already read into the buffer)
      * used       --> (cxlexer.current - cxlexer._stream_obj._stream_buff)
      * total read --> (cxlexer._stream_obj._nbytes_read_into_sbuff)
      * total read - used = amount/bytes left
      */
    if ((cxlexer->_stream_obj._nbytes_read_into_sbuff - (cxlexer->current - cxlexer->_stream_obj._stream_buff)) <= 10)
    {
        _cxml_stream* stream_obj = &cxlexer->_stream_obj;
        size_t byte_count = cxlexer->_stream_obj._chunk_start_size, actual_byte_count = 0;
        /*
         * on resize,
         *  `stream_obj->_chunk_curr_size` is set to at least `stream_obj->_chunk_start_size`.
         * Thus, the extra condition "&& stream_obj->_nbytes_read_into_sbuff", prevents us from
         * resizing/adjusting the stream buffer when there's still very much space, that is,
         * when byte_count is same as stream_obj->_chunk_curr_size and no chars has been read yet,
         * (stream_obj->_nbytes_read_into_sbuff is a count of how chars has been read from the
         * file into the stream buffer)
         */
        if (((stream_obj->_nbytes_read_into_sbuff + byte_count)
            >= stream_obj->_chunk_curr_size)
            && stream_obj->_nbytes_read_into_sbuff)
        {
            _cxml__adjust_stream_buffer(cxlexer);
        }

        if ((actual_byte_count = fread((stream_obj->_stream_buff + stream_obj->_nbytes_read_into_sbuff),
                                       sizeof(char), byte_count, stream_obj->_file)) < byte_count)
        {
            if (feof(stream_obj->_file)){
                stream_obj->_stream_buff[(stream_obj->_nbytes_read_into_sbuff + actual_byte_count)] = '\0';
                cxlexer->_should_stream = 0;
            }else{
                cxml_error("CXML Error: Error occurred while streaming file\n");
            }
        }
        stream_obj->_nbytes_read_into_sbuff += actual_byte_count;
    }
}

inline static void cxml__move(_cxml_lexer *cxlexer) {
    cxlexer->_should_stream ? _cxml__read(cxlexer) : (void)0;
    *(cxlexer->current) == '\n' ? cxlexer->line++ : 0;
    cxlexer->current++;
}

_cxml_token _cxml_move_until(_cxml_lexer *lexer, char target,
                             _cxml_token_t type, const char *msg)
{
    while (peek(lexer) != target){
        if (is_at_end(lexer)) break;
        cxml__move(lexer);
    }
    return is_at_end(lexer) ?
           new_cxml_err_token(lexer, msg) :
           new_cxml_token(lexer, type);
}

static char get_char(_cxml_lexer *cxlexer) {
    if (!is_at_end(cxlexer)){
        cxml__move(cxlexer);
        return *(cxlexer->current - 1);
    }
    return *cxlexer->current;
}

inline static bool is_comment(_cxml_lexer *cxlexer) {
    // *cxlexer->current == '!'
    return (memcmp(cxlexer->current, "!--", 3) == 0);
}

inline static bool is_dtd(_cxml_lexer *cxlexer) {
    if (peek_next(cxlexer, 1) == 'D') {
        return (memcmp(cxlexer->current, "!DOCTYPE", 8) == 0);
    }
    return false;
}

inline static bool is_cdata(_cxml_lexer *cxlexer) {
    if (peek_next(cxlexer, 1) == '[') {
        return (memcmp(cxlexer->current, "![CDATA[", 8) == 0);
    }
    return false;
}

inline static bool is_id_symbol(char ch) {
    switch (ch) {
        case '_':
        case '-':
        case '.':
            return true;
        default:
            return false;
    }
}

bool _cxml__is_alpha(char ch) {
    return ((ch >= 'a' && ch <= 'z')
            || ((ch >= 'A' && ch <= 'Z'))
            || (ch == '_')
            || (unsigned char) ch >= 0xC0);
}

bool _cxml__is_identifier(char ch){
    return _cxml__is_alpha(ch)
           || (ch >= '0' && ch <= '9')
           || ch == '-'
           || ch == '.';
}

inline static bool is_open_or_close(_cxml_lexer *cxlexer, char ch) {
    return (ch == '>' || ch == '<') || is_at_end(cxlexer);
}

static void set_type(_cxml_token* token){
    int ret;
    if ((ret = _cxml_is_integer(token->start, token->length))){
        token->literal_type = ret == 1 ? CXML_INTEGER_LITERAL : CXML_XINTEGER_LITERAL;
    }else if (_cxml_is_double(token->start, token->length)){
        token->literal_type = CXML_DOUBLE_LITERAL;
    }else{
        token->literal_type = CXML_STRING_LITERAL;
    }
}

//<tag attr="value">real value</tag>
static _cxml_token_t lex_attr_identifier(_cxml_lexer *cxlexer) {
    while (_cxml__is_identifier(peek(cxlexer)))
    {
        cxml__move(cxlexer);
    }
    return CXML_TOKEN_IDENTIFIER;
}

//** dtd lexing
// the lexer lexes an xml dtd structure, in a very forgiving style,
// i.e. it doesn't care too much about exactness, and only ensures
// that the dtd structure provided "matches" (even if partially)
// the dtd structure defined by the xml ebnf grammar.
// [PS: This lexing could still fail.]
// **//
static void _identifier(_cxml_lexer *lexer){
    while (_cxml__is_alpha(*lexer->current)
           || is_digit(*lexer->current)
           || is_id_symbol(*lexer->current))
    {
        cxml__move(lexer);
    }
}

inline static void _space(_cxml_lexer *lexer){
    while (isspace((unsigned char)*lexer->current)) {
        cxml__move(lexer);
    }
}

_CX_ATR_NORETURN inline static void _err(_cxml_lexer *lexer, const char *msg){
    cxml_error("Error occurred at line %d\n%s\nSource: `%.*s`",
            lexer->line, msg, 20, lexer->current);
}

inline static void _move(_cxml_lexer *lexer, int n){
    for (int i = 0; i < n; i++) cxml__move(lexer);
}

inline static bool _is_id(char ch){
    return _cxml__is_alpha(ch) || ch == ':';
}

inline static void _pereference(_cxml_lexer *lexer){
    // PEReference  ::=  '%' Name ';'
    // skip '%'
    lexer->current++;
    if (_is_id(*lexer->current)) {
        _identifier(lexer);
        // skip ';'
        if (*lexer->current == ';'){
            lexer->current++;
        }else{
            _err(lexer, "Invalid PEReference");
        }
    }else{
        _err(lexer, "Expected identifier");
    }
}

inline static void _consume_char(_cxml_lexer *lexer, char expected, const char *msg){
    // S? '>'
    if (isspace((unsigned char)*lexer->current)) _space(lexer);
    if (*lexer->current == expected) cxml__move(lexer);
    else _err(lexer, msg);
}

inline static void _literal(_cxml_lexer *lexer){
    char expect = *lexer->current;
    // skip ' | "
    lexer->current++;
    while (*lexer->current != expect && !is_at_end(lexer)){
        cxml__move(lexer);
    }
    // skip ' | "
    if (*lexer->current != expect) _err(lexer, "Unterminated literal");
    lexer->current++;
    _space(lexer);
}

static void _externalid(_cxml_lexer *lexer, int type){
    // ExternalID ::=  'SYSTEM' S SystemLiteral
    //              |  'PUBLIC' S PubidLiteral S SystemLiteral

    // skip 'SYSTEM' or 'PUBLIC'
    _move(lexer, 6);
    _space(lexer);
    bool end = false;
    if (*lexer->current == '\'' || *lexer->current == '"'){
        lex: ;
        _literal(lexer);
        if (end) return;
        if (type == 2){  // 'PUBLIC'
            _space(lexer);
            end = true;
            goto lex;
        }
    }else{
        _err(lexer, "Expected literal");
    }
}

inline static void _choice(_cxml_lexer *lexer);
inline static void _seq(_cxml_lexer *lexer);

inline static void _cp(_cxml_lexer *lexer){
    /*
     * cp           ::=  (Name | choice | seq) ('?' | '*' | '+')?
     * choice       ::=  '(' S? cp ( S? '|' S? cp )+ S? ')'
     * seq          ::=  '(' S? cp ( S? ',' S? cp )* S? ')'
     */
    _identifier(lexer);
    if (*lexer->current == '('){
        _choice(lexer);
    }
    switch (*lexer->current)
    {
        case '?':
        case '*':
        case '+':
            cxml__move(lexer);
    }
}

inline static void _choice(_cxml_lexer *lexer){
    /*
     * cp           ::=  (Name | choice | seq) ('?' | '*' | '+')?
     * choice       ::=  '(' S? cp ( S? '|' S? cp )+ S? ')'
     * seq          ::=  '(' S? cp ( S? ',' S? cp )* S? ')'
     */

    if (*lexer->current == '('){
        cxml__move(lexer);
        _space(lexer);
    }
    _cp(lexer);
    _space(lexer);
    if (*lexer->current == ','){  // dispatch to _seq()
        _seq(lexer);
        _space(lexer);
        if (*lexer->current != '|') return;
    }
    while (*lexer->current == '|'){
        cxml__move(lexer);
        _space(lexer);
        _cp(lexer);
    }
    // S? is handled in _consume_char()
    _consume_char(lexer, ')', "Expected ')'");
}

inline static void _seq(_cxml_lexer *lexer){
    /*
     * cp           ::=  (Name | choice | seq) ('?' | '*' | '+')?
     * choice       ::=  '(' S? cp ( S? '|' S? cp )+ S? ')'
     * seq          ::=  '(' S? cp ( S? ',' S? cp )* S? ')'
     */
    if (*lexer->current == '('){
        cxml__move(lexer);
        _space(lexer);
        _cp(lexer);
    }

    while (*lexer->current == ','){
        cxml__move(lexer);
        _space(lexer);
        _cp(lexer);
    }
    // S? is handled in _consume_char()
    _consume_char(lexer, ')', "Expected ')'");
}

inline static void _children(_cxml_lexer *lexer){
    /*
     * children     ::=  (choice | seq) ('?' | '*' | '+')?
     * cp           ::=  (Name | choice | seq) ('?' | '*' | '+')?
     * choice       ::=  '(' S? cp ( S? '|' S? cp )+ S? ')'
     * seq          ::=  '(' S? cp ( S? ',' S? cp )* S? ')'
    */
    // we already consumed '(' S? in _contentspec() - so we call these functions directly
    _choice(lexer);
    switch (*lexer->current)
    {
        case '?':
        case '*':
        case '+':
            cxml__move(lexer);
    }
}

inline static void _mixed(_cxml_lexer *lexer){
    // Mixed  ::=  '(' S? '#PCDATA' (S? '|' S? Name)* S? ')*'
    //          |  '(' S? '#PCDATA' S? ')'

    cxml__move(lexer);
    _identifier(lexer);
    while (*lexer->current != ')' && !is_at_end(lexer))
    {
        _space(lexer);
        if (*lexer->current == '|') cxml__move(lexer);
        _space(lexer);
        _identifier(lexer);
    }
    // skip ')'
    _consume_char(lexer, ')', "Expected ')'");
    if (*lexer->current == '*') cxml__move(lexer);
}

inline static void _contentspec(_cxml_lexer *lexer){
    /*
     * contentspec  ::=  'EMPTY' | 'ANY' | Mixed | children
     * children     ::=  (choice | seq) ('?' | '*' | '+')?
     * cp           ::=  (Name | choice | seq) ('?' | '*' | '+')?
     * choice       ::=  '(' S? cp ( S? '|' S? cp )+ S? ')'
     * seq          ::=  '(' S? cp ( S? ',' S? cp )* S? ')'
     * Mixed        ::=  '(' S? '#PCDATA' (S? '|' S? Name)* S? ')*'
     *               |  '(' S? '#PCDATA' S? ')'
     */
    // skip '('
    cxml__move(lexer);
    _space(lexer);
    // Mixed
    if (*lexer->current == '#'){
        _mixed(lexer);
        return;
    }
    // children
    _children(lexer);
}

static void _elementdecl(_cxml_lexer *lexer){
    // elementdecl -> '<!ELEMENT' S Name S contentspec S? '>'
    if (strncmp(lexer->current, "ELEMENT", 7)==0){
        lexer->current += 7;
        _space(lexer);
        _identifier(lexer);
        _space(lexer);
        // contentspec -> 'EMPTY' | 'ANY' | Mixed | children
        if (*lexer->current == 'E'){
            _move(lexer, 5);
        }else if (*lexer->current == 'A'){
            _move(lexer, 3);
        }else if (*lexer->current == '('){
            _contentspec(lexer);
        }else{
            _err(lexer, "Expected DTD content specification");
        }
        // S? '>'
        _consume_char(lexer, '>', "Expected '>'");
    }else{
        _err(lexer, "Unknown DTD markup declaration");
    }
}

inline static void _enumeration(_cxml_lexer *lexer){
    // Enumeration  ::= '(' S? Name (S? '|' S? Name)* S? ')'
    // skip '('
    cxml__move(lexer);
    _space(lexer);          // S?
    _identifier(lexer);     // Name
    while (*lexer->current != ')' && !is_at_end(lexer))
    {
        _space(lexer);
        if (*lexer->current == '|') cxml__move(lexer);
        _space(lexer);
        _identifier(lexer);
    }
    // skip ')'
    _consume_char(lexer, ')', "Expected ')'");
}

static void _atttype(_cxml_lexer *lexer){
    /*
     * AttType  ::= 'CDATA' | 'ID' | 'IDREF' | 'IDREFS' | 'ENTITY'
     *          |  'ENTITIES' | 'NMTOKEN' | 'NMTOKENS'
     *          |   ('NOTATION' S) ? '(' S? Name (S? '|' S? Name)* S? ')'
     *          |  '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')'
     */
    if (*lexer->current == 'C'){
        _move(lexer, 5);
    }
    else if (*lexer->current == 'N' && peek_next(lexer, 1) == 'O')
    {
        // ('NOTATION' S) ? '(' S? Name (S? '|' S? Name)* S? ')'
        if (strncmp(lexer->current, "NOTATION", 8) == 0){
            _move(lexer, 8);
            _space(lexer);
            if (*lexer->current == '('){
                _enumeration(lexer);
            }else{
                _err(lexer, "Expected '('");
            }
        }else{
            _err(lexer, "Expected 'NOTATION'");
        }
    }else if (*lexer->current == '('){
        // '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')'
        _enumeration(lexer);
    }
    else if (*lexer->current == 'I')
    {
        // ID
        _move(lexer, 2);
        if (*lexer->current == 'R'){
            // REF
            _move(lexer, 3);
            if (*lexer->current == 'S') lexer->current++;
        }
    }
    else if (*lexer->current == 'E'){
        // ENTIT
        _move(lexer, 5);
        if (*lexer->current == 'Y') lexer->current++; // Y
        else _move(lexer, 3);  // IES
    }
    else if (*lexer->current == 'N'){
        // 'NMTOKEN' | 'NMTOKENS'
        _move(lexer, 7);
        if (*lexer->current == 'S') lexer->current++;
    }
}

static void _defaultdecl(_cxml_lexer *lexer){
    // DefaultDecl ->  '#REQUIRED' | '#IMPLIED' | (('#FIXED' S)? AttValue)
    _space(lexer);
    if (*lexer->current == '#')
    {
        cxml__move(lexer);
        switch (*lexer->current)
        {
            case 'R':   // #REQUIRED
                _move(lexer, 8);
                break;
            case 'I':   // #IMPLIED
                _move(lexer, 7);
                break;
            case 'F':   // #FIXED
                _move(lexer, 5);
                _space(lexer);
                // AttValue -> '...' | "..."
                _literal(lexer);
                break;
            default:
                _err(lexer, "Unknown DTD default declaration");
        }
        _space(lexer);
    }else{
        _err(lexer, "Expected DTD default declaration");
    }
}

static void _attlistdecl(_cxml_lexer *lexer){
    // AttlistDecl -> '<!ATTLIST' S Name AttDef* S? '>'
    if (strncmp(lexer->current, "ATTLIST", 7)==0){
        _move(lexer, 7);
        _space(lexer);
        _identifier(lexer);
        // AttDef ->  S Name S AttType S DefaultDecl
        _space(lexer);
        _identifier(lexer);
        _space(lexer);
        /*
         * AttType  ::=  'CDATA' | 'ID' | 'IDREF' | 'IDREFS' | 'ENTITY'
         *              |  'ENTITIES' | 'NMTOKEN' | 'NMTOKENS'
         *              | ('NOTATION' S) ? '(' S? Name (S? '|' S? Name)* S? ')'
         *              | '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')'
         */
        _atttype(lexer);
        _defaultdecl(lexer);
        _consume_char(lexer, '>', "Expected '>'");
    }else{
        _err(lexer, "Unknown DTD markup declaration");
    }
}

static void _entitydecl(_cxml_lexer *lexer){
    // GEDecl -> '<!ENTITY' S Name S EntityDef S? '>'
    // PEDecl ->  '<!ENTITY' S '%' S Name S PEDef S? '>'
    if (strncmp(lexer->current, "ENTITY", 6) == 0){
        _move(lexer, 6);
        _space(lexer);
        if (*lexer->current == '%'){
            lexer->current++;
            _space(lexer);
        }
        _identifier(lexer);
        _space(lexer);
        // EntityDef ->  EntityValue | (ExternalID NDataDecl?)
        // 'SYSTEM' S SystemLiteral |  'PUBLIC' S PubidLiteral S SystemLiteral (S 'NDATA' S Name)?
        if (strncmp(lexer->current, "SYSTEM", 6) == 0)
        {
            _externalid(lexer, 1);
        }else if (strncmp(lexer->current, "PUBLIC", 6) == 0){
            _externalid(lexer, 2);
            _space(lexer);
            // (S 'NDATA' S Name)?
            if (*lexer->current == 'N'){
                _move(lexer, 4);
                _space(lexer);
                _identifier(lexer);
            }
        }else if (*lexer->current == '\'' || *lexer->current == '"'){
            _literal(lexer);
        }else{
            _err(lexer, "Expected EntityDef");
        }
        _consume_char(lexer, '>', "Expected '>'");
    }else{
        _err(lexer, "Unknown DTD markup declaration");
    }
}

static void _notationdecl(_cxml_lexer *lexer){
    /*
     * NotationDecl  ::=  '<!NOTATION' S Name S (ExternalID | PublicID) S? '>'
     * ExternalID    ::=  'SYSTEM' S SystemLiteral |  'PUBLIC' S PubidLiteral S SystemLiteral
     * PublicID      ::=  'PUBLIC' S PubidLiteral
     */
    if (strncmp(lexer->current, "NOTATION", 8)==0){
        _move(lexer, 8);
        _space(lexer);
        _identifier(lexer);
        _space(lexer);
        if (strncmp(lexer->current, "SYSTEM", 6)==0)
        {
            _externalid(lexer, 1);
        }else if (strncmp(lexer->current, "PUBLIC", 6)==0){
            _externalid(lexer, 1);  // 'PUBLIC' S PubidLiteral
            _space(lexer);
            // S SystemLiteral
            char expect = *lexer->current;
            if (expect == '\'' || expect == '"'){
                _literal(lexer);
            }
        }else{
            _err(lexer, "Expected ExternalID or PublicID");
        }
        // S? '>'
        _consume_char(lexer, '>', "Expected '>'");
    }else{
        _err(lexer, "Unknown DTD markup declaration");
    }
}

static void _pi(_cxml_lexer *lexer){
    // skip '<?'
    lexer->current += 2;
    while (*lexer->current != '?' && !is_at_end(lexer)){
        cxml__move(lexer);
    }
    if (*lexer->current != '?' || *(lexer->current + 1) != '>'){
        _err(lexer, "Invalid processing instruction");
    }
    // escape '?>'
    lexer->current += 2;
}

static void _comment(_cxml_lexer *lexer){
    // skip <!--
    lexer->current += 4;
    while (*lexer->current != '-' && !is_at_end(lexer)) {
        cxml__move(lexer);

        if (*lexer->current == '-' && *(lexer->current + 1) != '-') {
            cxml__move(lexer);
        }
    }
    if ((*lexer->current) == '-'
        && *(lexer->current + 1) == '-'
        && *(lexer->current + 2) == '>')
    {
        // escape '-->'
        lexer->current += 3;
    }else{
        _err(lexer, "Improperly terminated comment");
    }
}

static void _markupdecl(_cxml_lexer *lexer){
    // markupdecl -> elementdecl | AttlistDecl | EntityDecl
    //             | NotationDecl |  PI | Comment
    char next_ch = peek_next(lexer, 1);
    if (next_ch == '?'){
        // processing-instruction
        _pi(lexer);
    }else if ((next_ch == '!')    // <! - -
            && (peek_next(lexer, 2) == '-')
            && peek_next(lexer, 3) == '-')
    {
        // comment
        _comment(lexer);
    }else{
        // escape '<!'
        _move(lexer, 2);
        char next = peek_next(lexer, 1);
        switch (next)
        {
            case 'L':   // ELEMENT (elementdecl)
                _elementdecl(lexer);
                break;
            case 'T':   // ATTLIST (AttlistDecl)
                _attlistdecl(lexer);
                break;
            case 'N':   // ENTITY (EntityDecl)
                _entitydecl(lexer);
                break;
            case 'O':   // NOTATION (NotationDecl)
                _notationdecl(lexer);
                break;
            default:
                _err(lexer, "Unknown markup declaration");
        }
    }
}

static void _intsubset(_cxml_lexer *lexer){
    // intSubset   ::= (markupdecl | PEReference | S)*

    // skip '[' -> ('[' (markupdecl | PEReference | S)* ']' S?)? from lex_dtd()
    lexer->current++;
    _space(lexer);
    while ((*lexer->current == '%' || *lexer->current == '<') && !is_at_end(lexer)){
        if (*lexer->current == '%'){
            _pereference(lexer);
        }else{
            _markupdecl(lexer);
        }
        _space(lexer);
    }
}

static _cxml_token lex_dtd(_cxml_lexer *lexer){
    // doctypedecl -> '<!DOCTYPE' S Name (S ExternalID)? S? ('[' intSubset* ']' S?)? '>'
    // escape '!DOCTYPE'
    int line = lexer->line;
    _move(lexer, 8);
    _space(lexer);
    if (_is_id(*lexer->current)){ // NAME
        // reset start to the beginning of the dtd name.
        // this is done because the beginning '<!DOCTYPE' would be added by the parser.
        lexer->start = lexer->current;
        _cxml_token token;
        token.type = CXML_TOKEN_DOCTYPE;
        _identifier(lexer);
        int short_len = _cxml_int_cast(lexer->current - lexer->start);
        // (S ExternalID)? S?
        // ExternalID -> 'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral
        _space(lexer);
        if (strncmp(lexer->current, "SYSTEM", 6) == 0)
        {
            _externalid(lexer, 1);
        }else if (strncmp(lexer->current, "PUBLIC", 6) == 0){
            _externalid(lexer, 2);
        }
        // ('[' intSubset* ']' S?)?
        if (*lexer->current == '['){
            _intsubset(lexer);
            if (*lexer->current == ']'){
                lexer->current++;
            }else {
                _err(lexer, "Expected ']'");
            }
        }
        // S? '>'
        _consume_char(lexer, '>', "Expected '>' at end of DTD");
        token.start = lexer->start;
        token.line = line;
        token.literal_type = CXML_NON_LITERAL;
        if (!lexer->cfg.preserve_dtd_structure){
            token.length = short_len;
        }else{
            // do not include '>', since the parser would add it itself -> - 1.
            token.length = _cxml_int_cast(lexer->current - lexer->start) - 1;
        }
        return token;
    }else{
        // first 20 chars
        _err(lexer, "Invalid DTD NAME");
    }
}


static _cxml_token_t lex_value(_cxml_lexer *cxlexer) {
    while (!is_at_end(cxlexer) && peek(cxlexer) != '<') {
        cxml__move(cxlexer);
    }
    return CXML_TOKEN_TEXT;
}


static _cxml_token lex_string(_cxml_lexer *cxlexer) {
    char ch = peek_prev(cxlexer, 1) == '\'' ? '\'' : '"';
    while (!is_at_end(cxlexer) && *cxlexer->current != ch) {
        cxml__move(cxlexer);
    }
    if (is_at_end(cxlexer))
        return new_cxml_err_token(cxlexer, "Unterminated string -> ");
    cxml__move(cxlexer);
    cxlexer->start++;  // escape beginning quotation '"' | '\'' '"' | '\''
    return new_cxml_token(cxlexer, CXML_TOKEN_STRING);
}


static _cxml_token_t lex_cdata(_cxml_lexer *cxlexer){
    cxlexer->current += 8;
    while (!is_at_end(cxlexer)){
        // handle ']' in cdata when ']' doesn't terminate the cdata
        if ((*cxlexer->current == ']' && *(cxlexer->current + 1) != ']'))
        {
            cxml__move(cxlexer);
        }else if ((*cxlexer->current == ']' && *(cxlexer->current + 1) == ']'))
        {
            if (*(cxlexer->current + 2) != '>'){
                cxml__move(cxlexer);
            }else{
                break;
            }
        }else{
            cxml__move(cxlexer);
        }
    }
    if (is_at_end(cxlexer)){
        cxlexer->error = true;
        return CXML_TOKEN_CDATA;
    }
    cxml__move(cxlexer);
    if (*cxlexer->current == ']'){
        cxml__move(cxlexer);
        if (*cxlexer->current == '>'){
            cxml__move(cxlexer);
            return CXML_TOKEN_CDATA;
        }
    }
    cxlexer->error = true;
    return CXML_TOKEN_CDATA;
}


_cxml_token cxml_get_token(_cxml_lexer *cxlexer) {
#define RETURN(val)     \
    if (cxlexer->_should_stream) cxlexer->_returned++;    \
    return val;

    // reset the buffer after a _cxml_token has been created and
    // returned (to the parser) successfully.
    if (cxlexer->_should_stream && cxlexer->_returned){
        // we only try to reset (resize and adjust the stream buffer)
        // when the amount of bytes consumed >= 75% of the buffer's original size
        if ((cxlexer->current - cxlexer->_stream_obj._stream_buff)
             >= (long) (cxlexer->_stream_obj._chunk_start_size * 0.75))
        {
            _cxml__reset_stream_buffer(cxlexer);
        }
        cxlexer->_returned = 0;
    }

    start:
    // In "preserve space" option,
    // this could potentially be a value (text), may be valid (text) or
    // invalid (spaces separating two different tags/elements)
    if (isspace((unsigned char)*cxlexer->current)) skip_whitespace(cxlexer);

    cxlexer->start = cxlexer->current;

    char ch = get_char(cxlexer);

    if (!is_open_or_close(cxlexer, ch) && (cxlexer->vflag == '>')){
        RETURN (new_cxml_token(cxlexer, lex_value(cxlexer)))
    }
    if (_cxml__is_alpha(ch)) {
        RETURN (new_cxml_token(cxlexer, lex_attr_identifier(cxlexer)))
    }
    switch (ch)
    {
        case '<':
        {
            if ((unsigned char)(*cxlexer->current) == '!')
            {
                if (is_comment(cxlexer)){
                    _cxml_token comment_tok = lex_comment(cxlexer);
                    if (cxlexer->preserve_cm) {
                        RETURN (cxlexer->error ?
                                new_cxml_err_token(cxlexer, "Comment not properly closed! -> ") :
                                comment_tok)
                    }
                    goto start;
                }
                else if (is_cdata(cxlexer)){
                    _cxml_token_t cdata = lex_cdata(cxlexer);
                    if (cxlexer->preserve_cd){
                        RETURN(cxlexer->error ?
                               new_cxml_err_token(cxlexer, "CDATA-SECT not properly closed! -> ") :
                               new_cxml_token(cxlexer, cdata))
                    }
                    goto start;
                }
                else if (is_dtd(cxlexer)) {
                    RETURN (lex_dtd(cxlexer))
                }
                else{
                    RETURN(new_cxml_err_token(cxlexer, "Invalid token -> "))
                }
            }
            else {
                cxlexer->vflag = 0;
                RETURN (new_cxml_token(cxlexer, CXML_TOKEN_L_THAN))
            }
        }
        case '>':
            // catch false positives. '>' could be the first char in a text
            // so if value flag (`vflag`) was just previously set, then the
            // current '>' is actually part of a text value
            if (cxlexer->vflag == '>'){
                // false positive; it's just a text
                RETURN (new_cxml_token(cxlexer, lex_value(cxlexer)))
            }
            // <f>...value...</f>.. set value flag
            cxlexer->vflag = '>';
            RETURN (new_cxml_token(cxlexer, CXML_TOKEN_G_THAN))
        case '\'':
        case '"':
            RETURN (lex_string(cxlexer))
        case '?':
            RETURN (new_cxml_token(cxlexer, CXML_TOKEN_Q_MARK))
        case '=':
            RETURN (new_cxml_token(cxlexer, CXML_TOKEN_EQUAL))
        case '/':
            RETURN (new_cxml_token(cxlexer, CXML_TOKEN_F_SLASH))
        case ':':
            RETURN (new_cxml_token(cxlexer, CXML_TOKEN_COLON))
        default:
            if (is_at_end(cxlexer)) return new_cxml_token(cxlexer, CXML_TOKEN_EOF);
            // eof' any unknown character, and rely on the parser for
            // figuring out if parsing can continue of not.
            RETURN (isdigit((unsigned char)ch)                  ?  // text at wrong position?
                    new_cxml_token(cxlexer, lex_value(cxlexer)) :
                    new_cxml_err_token(cxlexer, "Unrecognized character found -> "))
    }
#undef RETURN
}

char* cxml_token_type_as_str(_cxml_token_t type){
    char* type_str = NULL;
    switch (type) {
        case CXML_TOKEN_G_THAN:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-GREATER-THAN);
            break;
        case CXML_TOKEN_L_THAN:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-LESS-THAN);
            break;
        case CXML_TOKEN_Q_MARK:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-QUESTION-MARK);
            break;
        case CXML_TOKEN_EQUAL:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-EQUAL);
            break;
        case CXML_TOKEN_TEXT:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-TEXT);
            break;
        case CXML_TOKEN_F_SLASH:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-FORWARD-SLASH);
            break;
        case CXML_TOKEN_STRING:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-STRING);
            break;
        case CXML_TOKEN_VALUE:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-VALUE);
            break;
        case CXML_TOKEN_IDENTIFIER:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-IDENTIFIER);
            break;
        case CXML_TOKEN_COMMENT:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-COMMENT);
            break;
        case CXML_TOKEN_DOCTYPE:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-DOCTYPE);
            break;
        case CXML_TOKEN_EOF:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-EOF);
            break;
        case CXML_TOKEN_ERROR:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-ERROR);
            break;
        case CXML_TOKEN_CDATA:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-CDATA);
            break;
        case CXML_TOKEN_COLON:
            type_str = TOKENTYPE_TO_STR(CXML-TOKEN-COLON);
            break;
        default:
            type_str = TOKENTYPE_TO_STR(CXML-UNKNOWN-TOKEN);
            break;
    }
    return type_str;
}

void cxml_print_tokens(const char *src) {
    if (!src) return;
    _cxml_lexer lexer;
    _cxml_lexer_init(&lexer, src, NULL, 0);
    _cxml_token token;
    int line = 0;

    printf("-----------------------------------------------------------\n");

    //Line_No   TokenType   Token_Value

    while (true) {
        token = cxml_get_token(&lexer);
        if (line != token.line) {
            line = token.line;
            printf("%04d\t\t", token.line);
        } else {
            printf("%4s\t\t", "|");
        }
        printf("%02d\t\t", token.type);
        printf("%*s\t\t", 0x1e, cxml_token_type_as_str(token.type));
        printf("'%.*s'\n", token.length, token.start);
        if (token.type == CXML_TOKEN_ERROR || token.type == CXML_TOKEN_EOF) break;
    }
}
