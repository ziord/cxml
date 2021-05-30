/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

cts test__cxml_lexer_init(){
    _cxml_lexer lexer;
    cxml_config cfg = cxml_get_config();
    _cxml_lexer_init(&lexer, wf_xml_6, NULL, false);
    cxml_assert__eq(lexer.start, wf_xml_6)
    cxml_assert__eq(lexer.current, wf_xml_6)
    cxml_assert__one(lexer.line)
    cxml_assert__false(lexer.error)
    cxml_assert__false(lexer._stream)
    cxml_assert__false(lexer._should_stream)
    cxml_assert__false(lexer._returned)
    cxml_assert__zero(lexer.vflag)
    cxml_assert__eq(lexer.preserve_sp, cfg.preserve_space)
    cxml_assert__eq(lexer.preserve_cm, cfg.preserve_comment)
    cxml_assert__eq(lexer.preserve_cd, cfg.preserve_cdata)
    cxml_pass()
}

cts test__cxml_token_init(){
    _cxml_token tok;
    _cxml_token_init(&tok);
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    cxml_pass()
}

cts test__cxml_lexer_close(){
    _cxml_lexer lexer;
    _cxml_lexer_init(&lexer, wf_xml_6, NULL, false);
    _cxml_lexer_close(&lexer);
    cxml_assert__null(lexer.start)
    cxml_assert__null(lexer.current)
    cxml_pass()
}

extern _cxml_token cxml_get_token(_cxml_lexer *cxlexer);

cts test_cxml_get_token(){
    cxml_cfg_preserve_space(0);
    _cxml_lexer lexer;
    // <fruit><name>apple</name><name>banana</name></fruit>
    _cxml_lexer_init(&lexer, wf_xml_10, NULL, false);
    _cxml_token tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_L_THAN)
    cxml_assert__eq(tok.start, wf_xml_10)
    cxml_assert__eq(lexer.start, wf_xml_10)
    cxml_assert__eq(lexer.current, (wf_xml_10+1))
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    cxml_assert__one(tok.length)
    cxml_assert__one(tok.line)

    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_IDENTIFIER)
    cxml_assert__eq(tok.start, (wf_xml_10+1))
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    cxml_assert__eq(tok.length, 5)
    cxml_assert__one(tok.line)

    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_G_THAN)
    cxml_assert__eq(tok.start, (wf_xml_10+6))
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    cxml_assert__eq(lexer.vflag, '>')
    cxml_assert__one(tok.length)
    cxml_assert__one(tok.line)
    _cxml_lexer_close(&lexer);

    char *src = "'\n0x12345\t'";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_STRING)
    cxml_assert__eq(tok.start, (src + 1))
    cxml_assert__eq(tok.literal_type, CXML_XINTEGER_LITERAL)
    cxml_assert__eq(tok.length, 9)
    cxml_assert__two(tok.line)
    _cxml_lexer_close(&lexer);

    src = "' 12.1e-1 '";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_STRING)
    cxml_assert__eq(tok.start, (src + 1))
    cxml_assert__eq(tok.literal_type, CXML_DOUBLE_LITERAL)
    cxml_assert__eq(tok.length, 9)
    cxml_assert__one(tok.line)
    _cxml_lexer_close(&lexer);

    src = "\n12345";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_TEXT)
    cxml_assert__eq(tok.start, (src + 1))
    cxml_assert__eq(tok.literal_type, CXML_INTEGER_LITERAL)
    cxml_assert__eq(tok.length, 5)
    cxml_assert__two(tok.line)
    _cxml_lexer_close(&lexer);

    src = " = \r\n<!--foo--> \n: <![CDATA[abc]]>";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_EQUAL)
    cxml_assert__eq(tok.start, (src + 1))
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    cxml_assert__eq(tok.length, 1)
    cxml_assert__one(tok.line)

    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_COMMENT)
    cxml_assert__eq(tok.start, (src + 5))
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    cxml_assert__eq(tok.length, 10)
    cxml_assert__eq(tok.line, 2)

    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_COLON)
    cxml_assert__eq(tok.start, (src + 17))
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    cxml_assert__eq(tok.length, 1)
    cxml_assert__eq(tok.line, 3)

    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_CDATA)
    cxml_assert__eq(tok.start, (src + 19))
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    cxml_assert__eq(tok.length, 15)
    cxml_assert__eq(tok.line, 3)
    _cxml_lexer_close(&lexer);

    src = "<!DOCTYPE note SYSTEM \"example.dtd\">";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_DOCTYPE)
    cxml_assert__eq(lexer.start, tok.start)
    cxml_assert__eq(lexer.current, (tok.start + 26)) // at eof
    cxml_assert__neq(tok.start, src)
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    // under trim_dtd configuration, only the dtd name is preserved
    cxml_assert__eq(tok.length, 4)
    cxml_assert__one(tok.line)
    _cxml_lexer_close(&lexer);

    src = "<!DOCTYPE note SYSTEM \"example.dtd\">";
    cxml_cfg_trim_dtd(0);
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_DOCTYPE)
    cxml_assert__eq(tok.start, (src + 10))
    // (stars from 'note)
    // 25 because the '>' at the end of the dtd structure isn't included
    cxml_assert__eq(tok.length, 25)
    _cxml_lexer_close(&lexer);

    src = "'foo";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_ERROR)
    cxml_assert__true(lexer.error)
    cxml_assert__neq(tok.start, (src + 1))
    cxml_assert__zero(strncmp(tok.start, "Unterminated string -> ", 23))
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    cxml_assert__gt(tok.length, 23)
    cxml_assert__one(tok.line)
    // error tokens are allocated
    FREE(tok.start);
    _cxml_lexer_close(&lexer);

    src = "<!-bad comment->";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_ERROR)
    cxml_assert__true(lexer.error)
    cxml_assert__neq(tok.start, src)
    cxml_assert__zero(strncmp(tok.start, "Invalid token -> ", 17))
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    cxml_assert__gt(tok.length, 17)
    cxml_assert__one(tok.line)
    // error tokens are allocated
    FREE(tok.start);
    _cxml_lexer_close(&lexer);

    src = "<!--bad comment2->";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_ERROR)
    cxml_assert__true(lexer.error)
    cxml_assert__neq(tok.start, src)
    cxml_assert__zero(strncmp(tok.start, "Comment not properly closed! -> ", 32))
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    cxml_assert__gt(tok.length, 32)
    cxml_assert__one(tok.line)
    // error tokens are allocated
    FREE(tok.start);
    _cxml_lexer_close(&lexer);

    src = "<!--bad --comment3->";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    cxml_assert__eq(tok.type, CXML_TOKEN_ERROR)
    cxml_assert__true(lexer.error)
    cxml_assert__neq(tok.start, src)
    cxml_assert__zero(strncmp(tok.start, "Comment not properly closed! -> ", 32))
    cxml_assert__eq(tok.literal_type, CXML_NON_LITERAL)
    cxml_assert__gt(tok.length, 32)
    cxml_assert__one(tok.line)
    // error tokens are allocated
    FREE(tok.start);
    _cxml_lexer_close(&lexer);

    cxml_pass()
}


void suite_cxlexer(){
    cxml_suite(cxlexer)
    {
        cxml_add_m_test(4,
                        test__cxml_lexer_init,
                        test__cxml_token_init,
                        test__cxml_lexer_close,
                        test_cxml_get_token
        )
        cxml_run_suite()
    }
}
