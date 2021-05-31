/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

cts test__cxml_lexer_init(){
    _cxml_lexer lexer;
    cxml_config cfg = cxml_get_config();
    _cxml_lexer_init(&lexer, wf_xml_6, NULL, false);
    CHECK_EQ(lexer.start, wf_xml_6);
    CHECK_EQ(lexer.current, wf_xml_6);
    CHECK_EQ(lexer.line, 1);
    CHECK_FALSE(lexer.error);
    CHECK_FALSE(lexer._stream);
    CHECK_FALSE(lexer._should_stream);
    CHECK_FALSE(lexer._returned);
    CHECK_EQ(lexer.vflag, 0);
    CHECK_EQ(lexer.preserve_sp, cfg.preserve_space);
    CHECK_EQ(lexer.preserve_cm, cfg.preserve_comment);
    CHECK_EQ(lexer.preserve_cd, cfg.preserve_cdata);
    cxml_pass()
}

cts test__cxml_token_init(){
    _cxml_token tok;
    _cxml_token_init(&tok);
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    cxml_pass()
}

cts test__cxml_lexer_close(){
    _cxml_lexer lexer;
    _cxml_lexer_init(&lexer, wf_xml_6, NULL, false);
    _cxml_lexer_close(&lexer);
    CHECK_EQ(lexer.start, NULL);
    CHECK_EQ(lexer.current, NULL);
    cxml_pass()
}

extern _cxml_token cxml_get_token(_cxml_lexer *cxlexer);

cts test_cxml_get_token(){
    cxml_cfg_preserve_space(0);
    _cxml_lexer lexer;
    // <fruit><name>apple</name><name>banana</name></fruit>
    _cxml_lexer_init(&lexer, wf_xml_10, NULL, false);
    _cxml_token tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_L_THAN);
    CHECK_EQ(tok.start, wf_xml_10);
    CHECK_EQ(lexer.start, wf_xml_10);
    CHECK_EQ(lexer.current, (wf_xml_10+1));
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    CHECK_EQ(tok.length, 1);
    CHECK_EQ(tok.line, 1);

    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_IDENTIFIER);
    CHECK_EQ(tok.start, (wf_xml_10+1));
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    CHECK_EQ(tok.length, 5);
    CHECK_EQ(tok.line, 1);

    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_G_THAN);
    CHECK_EQ(tok.start, (wf_xml_10+6));
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    CHECK_EQ(lexer.vflag, '>');
    CHECK_EQ(tok.length, 1);
    CHECK_EQ(tok.line, 1);
    _cxml_lexer_close(&lexer);

    char *src = "'\n0x12345\t'";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_STRING);
    CHECK_EQ(tok.start, (src + 1));
    CHECK_EQ(tok.literal_type, CXML_XINTEGER_LITERAL);
    CHECK_EQ(tok.length, 9);
    CHECK_EQ(tok.line, 2);
    _cxml_lexer_close(&lexer);

    src = "' 12.1e-1 '";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_STRING);
    CHECK_EQ(tok.start, (src + 1));
    CHECK_EQ(tok.literal_type, CXML_DOUBLE_LITERAL);
    CHECK_EQ(tok.length, 9);
    CHECK_EQ(tok.line, 1);
    _cxml_lexer_close(&lexer);

    src = "\n12345";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_TEXT);
    CHECK_EQ(tok.start, (src + 1));
    CHECK_EQ(tok.literal_type, CXML_INTEGER_LITERAL);
    CHECK_EQ(tok.length, 5);
    CHECK_EQ(tok.line, 2);
    _cxml_lexer_close(&lexer);

    src = " = \r\n<!--foo--> \n: <![CDATA[abc]]>";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_EQUAL);
    CHECK_EQ(tok.start, (src + 1));
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    CHECK_EQ(tok.length, 1);
    CHECK_EQ(tok.line, 1);

    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_COMMENT);
    CHECK_EQ(tok.start, (src + 5));
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    CHECK_EQ(tok.length, 10);
    CHECK_EQ(tok.line, 2);

    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_COLON);
    CHECK_EQ(tok.start, (src + 17));
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    CHECK_EQ(tok.length, 1);
    CHECK_EQ(tok.line, 3);

    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_CDATA);
    CHECK_EQ(tok.start, (src + 19));
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    CHECK_EQ(tok.length, 15);
    CHECK_EQ(tok.line, 3);
    _cxml_lexer_close(&lexer);

    src = "<!DOCTYPE note SYSTEM \"example.dtd\">";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_DOCTYPE);
    CHECK_EQ(lexer.start, tok.start);
    CHECK_EQ(lexer.current, (tok.start + 26)) // at eof;
    CHECK_NE(tok.start, src);
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    // under trim_dtd configuration, only the dtd name is preserved
    CHECK_EQ(tok.length, 4);
    CHECK_EQ(tok.line, 1);
    _cxml_lexer_close(&lexer);

    src = "<!DOCTYPE note SYSTEM \"example.dtd\">";
    cxml_cfg_trim_dtd(0);
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_DOCTYPE);
    CHECK_EQ(tok.start, (src + 10));
    // (stars from 'note)
    // 25 because the '>' at the end of the dtd structure isn't included
    CHECK_EQ(tok.length, 25);
    _cxml_lexer_close(&lexer);

    src = "'foo";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_ERROR);
    CHECK_TRUE(lexer.error);
    CHECK_NE(tok.start, (src + 1));
    CHECK_EQ(strncmp(tok.start, "Unterminated string -> ", 23), 0);
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    CHECK_GT(tok.length, 23);
    CHECK_EQ(tok.line, 1);
    // error tokens are allocated
    FREE(tok.start);
    _cxml_lexer_close(&lexer);

    src = "<!-bad comment->";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_ERROR);
    CHECK_TRUE(lexer.error);
    CHECK_NE(tok.start, src);
    CHECK_EQ(strncmp(tok.start, "Invalid token -> ", 17), 0);
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    CHECK_GT(tok.length, 17);
    CHECK_EQ(tok.line, 1);
    // error tokens are allocated
    FREE(tok.start);
    _cxml_lexer_close(&lexer);

    src = "<!--bad comment2->";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_ERROR);
    CHECK_TRUE(lexer.error);
    CHECK_NE(tok.start, src);
    CHECK_EQ(strncmp(tok.start, "Comment not properly closed! -> ", 32), 0);
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    CHECK_GT(tok.length, 32);
    CHECK_EQ(tok.line, 1);
    // error tokens are allocated
    FREE(tok.start);
    _cxml_lexer_close(&lexer);

    src = "<!--bad --comment3->";
    _cxml_lexer_init(&lexer, src, NULL, false);
    tok = cxml_get_token(&lexer);
    CHECK_EQ(tok.type, CXML_TOKEN_ERROR);
    CHECK_TRUE(lexer.error);
    CHECK_NE(tok.start, src);
    CHECK_EQ(strncmp(tok.start, "Comment not properly closed! -> ", 32), 0);
    CHECK_EQ(tok.literal_type, CXML_NON_LITERAL);
    CHECK_GT(tok.length, 32);
    CHECK_EQ(tok.line, 1);
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
