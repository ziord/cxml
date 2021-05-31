/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

// todo: more comprehensive tests

cts test_cxml_xpath(){
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    cxml_assert(root)
    cxml_set *nodeset = cxml_xpath(root, "//*");
    cxml_assert__not_null(nodeset)
    cxml_assert__two(cxml_set_size(nodeset))

    int i = 0;
    char *expected[] = {"fruit", "name"};
    cxml_for_each(node, &nodeset->items)
    {
        cxml_assert__eq(_cxml_node_type(node), CXML_ELEM_NODE)
        cxml_assert__true(cxml_string_raw_equals(
                &_unwrap__cxnode(elem, node)->name.qname, expected[i]))
        i++;
    }

    cxml_assert__null(cxml_xpath(root, NULL))
    cxml_assert__null(cxml_xpath(NULL, "//*"))
    // automatically cleans up all nodes, including in the nodeset
    cxml_destroy(root);
    cxml_set_free(nodeset);
    FREE(nodeset);
    cxml_pass()
}

void suite_cxxpath(){
    cxml_suite(cxxpath)
    {
        cxml_add_test(test_cxml_xpath)
        cxml_run_suite()
    }
}
