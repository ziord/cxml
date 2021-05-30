/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

cts test__cxml_read_file(){
    char *fp = get_file_path("wf_xml_1.xml");
    char *dest;
    cxml_assert__zero(_cxml_read_file(fp, NULL))
    cxml_assert__zero(_cxml_read_file(NULL, &dest))
    cxml_assert__one(_cxml_read_file(fp, &dest))
    cxml_assert__not_null(dest)
    FREE(dest);
    FREE(fp);
    cxml_pass()
}

void suite_cxutils(){
    cxml_suite(cxutils)
    {
        cxml_add_test(test__cxml_read_file)
        cxml_run_suite()
    }
}
