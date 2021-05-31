/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

/**Test suites**/

/* cxlist.c test suite */
extern void suite_cxlist();
extern void suite_cxstr();
extern void suite_cxtable();
extern void suite_cxmset();
extern void suite_cxgrptable();
extern void suite_cxstack();
extern void suite_cxlrucache();
extern void suite_cxliteral();
extern void suite_cxmem();
extern void suite_cxdefs();
extern void suite_cxqapi();
extern void suite_cxsax();
extern void suite_cxutils();
extern void suite_cxlexer();
extern void suite_cxparser();
extern void suite_cxxpath();
extern void suite_cxprinter();


/** Super suites **/

/* super-suite to store all suites from core folder */
void super_suite_internals(){
    // cxlist.c module test suite
    suite_cxlist();
    // cxstr.c module test suite
    suite_cxstr();
    // cxtable.c module test suite
    suite_cxtable();
    // cxmset.c module test suite
    suite_cxmset();
    // cxgrptable.c module test suite
    suite_cxgrptable();
    // cxstack.c module test suite
    suite_cxstack();
    // cxlrucache.c module test suite
    suite_cxlrucache();
    // cxliteral.c module test suite
    suite_cxliteral();
    // cxmem.c module test suite
    suite_cxmem();
    // cxdefs.c module test suite
    suite_cxdefs();
}

void super_suite_utils(){
    suite_cxutils();
}

void super_suite_xml(){
    // cxlexer.c module test suite
    suite_cxlexer();
    // cxparser.c module test suite
    suite_cxparser();
    // cxprinter.c module test suite
    suite_cxprinter();
}


#if defined(CXML_USE_QUERY_MOD)
void super_suite_query(){
    // cxqapi.c module test suite
    suite_cxqapi();
}
#else
void super_suite_query(){

}
#endif


#if defined(CXML_USE_SAX_MOD)
void super_suite_sax(){
    // cxsax.c module test suite
    suite_cxsax();
}
#else
void super_suite_sax(){

}
#endif

#if defined(CXML_USE_XPATH_MOD)
void super_suite_xpath(){
    // cxxpath.c module test suite
    suite_cxxpath();
}
#else
void super_suite_xpath(){

}
#endif


int main(){
    cxml_cfg_enable_fancy_printing(0);
    cxml_cfg_show_warnings(0);
    cxml_cfg_enable_debugging(0);
    set_data_path();
    CXML_TEST_RUNNER(6,
                     super_suite_internals,
                     super_suite_utils,
                     super_suite_xml,
                     super_suite_query,
                     super_suite_sax,
                     super_suite_xpath)
    free_data_path();
    printf("\nhello world\n");
}
