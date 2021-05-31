/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

// todo: more comprehensive tests

cts test_cxml_xpath(){
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    CHECK(root)
    cxml_set *nodeset = cxml_xpath(root, "//*");
    CHECK_NE(nodeset, NULL)
    CHECK_EQ(cxml_set_size(nodeset), 2)

    int i = 0;
    char *expected[] = {"fruit", "name"};
    cxml_for_each(node, &nodeset->items)
    {
        CHECK_EQ(_cxml_node_type(node), CXML_ELEM_NODE)
        CHECK_TRUE(cxml_string_raw_equals(
                &_unwrap__cxnode(elem, node)->name.qname, expected[i]))
        i++;
    }

    CHECK_EQ(cxml_xpath(root, NULL), NULL)
    CHECK_EQ(cxml_xpath(NULL, "//*"), NULL)
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
