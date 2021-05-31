/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

/*
* todo:
*      for each test, test other configurations i.e.
*      comment on/off, cdata on/off, namespace defaults on/off,
*      space preservation on/off, indent_space_size, print_fancy, etc.
*      This should be a collaboratory test with cxconfig.c module
*/

cts test_cxml_prettify(){
    cxml_cfg_preserve_space(0);
    cxml_cfg_show_doc_as_top_level(0);
    cxml_root_node *root = get_root("ugly.xml", false);
    cxml_assert__not_null(root)
    char *got = cxml_prettify(root);
    char *expected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                     "<breakfast_menu>\n"
                     "  Hi there, roo!\n"
                     "  <food1>\n"
                     "    <name class=\"pooh\">\n"
                     "      winnie the pooh bear...!\n"
                     "    </name>\n"
                     "    <price>\n"
                     "      $5.95\n"
                     "    </price>\n"
                     "    <description>\n"
                     "      <br/>\n"
                     "      <br/>\n"
                     "      100 acre wood ::)\n"
                     "    </description>\n"
                     "  </food1>\n"
                     "</breakfast_menu>";
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(expected, got, strlen(expected), strlen(got)))
    cxml_assert__null(cxml_prettify(NULL))
    FREE(got);
    cxml_destroy(root);
    cxml_pass()
}


void suite_cxprinter() {
    cxml_suite(cxprinter)
    {
        cxml_add_test_setup(fixture_no_fancy_printing_and_warnings)
        cxml_add_test_teardown(fixture_no_fancy_printing_and_warnings)
        cxml_add_m_test(1,
                        test_cxml_prettify
        )
        cxml_run_suite()
    }
}
