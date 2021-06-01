/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"
#include <Muon/Muon.h>

// todo: more comprehensive tests

TEST(cxxpath, cxml_xpath){
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    CHECK(root);
    cxml_set *nodeset = cxml_xpath(root, "//*");
    CHECK_NOT_NULL(nodeset);
    CHECK_EQ(cxml_set_size(nodeset), 2);

    int i = 0;
    char *expected[] = {"fruit", "name"};
    cxml_for_each(node, &nodeset->items)
    {
        CHECK_EQ(_cxml_node_type(node), CXML_ELEM_NODE);
        CHECK_TRUE(cxml_string_raw_equals(
                &_unwrap__cxnode(elem, node)->name.qname, expected[i]));
        i++;
    }

    CHECK_NULL(cxml_xpath(root, NULL));
    CHECK_NULL(cxml_xpath(NULL, "//*"));
    // automatically cleans up all nodes, including in the nodeset
    cxml_destroy(root);
    cxml_set_free(nodeset);
    FREE(nodeset);
}