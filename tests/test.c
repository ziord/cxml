/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

/**Test suites**/

/* cxlist.c test suite */
extern void suite_cxlist(void);
extern void suite_cxstr(void);
extern void suite_cxtable(void);
extern void suite_cxmset(void);
extern void suite_cxgrptable(void);
extern void suite_cxstack(void);
extern void suite_cxlrucache(void);
extern void suite_cxliteral(void);
extern void suite_cxmem(void);
extern void suite_cxdefs(void);
extern void suite_cxqapi(void);
extern void suite_cxsax(void);
extern void suite_cxutils(void);
extern void suite_cxlexer(void);
extern void suite_cxparser(void);
extern void suite_cxxpath(void);
extern void suite_cxprinter(void);


/** Super suites **/

/* super-suite to store all suites from core folder */
void super_suite_internals(void){
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

void super_suite_utils(void){
    suite_cxutils();
}

void super_suite_xml(void){
    // cxlexer.c module test suite
    suite_cxlexer();
    // cxparser.c module test suite
    suite_cxparser();
    // cxprinter.c module test suite
    suite_cxprinter();
}


#if defined(CXML_USE_QUERY_MOD)
void super_suite_query(void){
    // cxqapi.c module test suite
    suite_cxqapi();
}
#else
void super_suite_query(void){

}
#endif


#if defined(CXML_USE_SAX_MOD)
void super_suite_sax(void){
    // cxsax.c module test suite
    suite_cxsax();
}
#else
void super_suite_sax(void){

}
#endif

#if defined(CXML_USE_XPATH_MOD)
void super_suite_xpath(void){
    // cxxpath.c module test suite
    suite_cxxpath();
}
#else
void super_suite_xpath(void){

}
#endif


int main(void){
    cxml_cfg_enable_fancy_printing(0);
    cxml_cfg_show_warnings(0);
    cxml_cfg_enable_debugging(0);
    set_data_path();
    int status;
    CXML_TEST_RUNNER(&status, 6,
                     super_suite_internals,
                     super_suite_utils,
                     super_suite_xml,
                     super_suite_query,
                     super_suite_sax,
                     super_suite_xpath)
    free_data_path();
    return status;
}
