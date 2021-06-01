/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

//#define CXML_T_NO_SUITE_TEST_SHUFFLE
#include "cxfixture.h"
#include <Muon/Muon.h>

/*
 * Test for the major functions exported by cxqapi.c
 *
 * [PS: One may find that some code segments are duplicated; this is intentional.]
 */


// General functions
TEST(cxqapi, cxml_is_well_formed){
    cxml_root_node *root = get_root("wf_xml_1.xml", false);
    CHECK_NOT_NULL(root);
    CHECK_TRUE(cxml_is_well_formed(root));
    cxml_destroy(root);
    root = cxml_load_string(df_xml_1);
    CHECK_FALSE(cxml_is_well_formed(root));
    CHECK_FALSE(cxml_is_well_formed(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_get_node_type){
    cxml_root_node root;
    cxml_root_node_init(&root);
    CHECK_EQ(cxml_get_node_type(&root), CXML_ROOT_NODE);
    cxml_elem_node elem;
    cxml_elem_node_init(&elem);
    CHECK_EQ(cxml_get_node_type(&elem), CXML_ELEM_NODE);
    CHECK_EQ(cxml_get_node_type(NULL), 0xff);
}

TEST(cxqapi, cxml_get_dtd_node){
    cxml_root_node *root = get_root("wf_xml_5.xml", true);
    CHECK_NOT_NULL(root);
    cxml_dtd_node *node = cxml_get_dtd_node(root);
    CHECK_NOT_NULL(node);
    CHECK_EQ(cxml_get_node_type(node), CXML_DTD_NODE);
    cxml_destroy(root);

    root = get_root("wf_xml_1.xml", true);
    CHECK_NULL(cxml_get_dtd_node(root));
    CHECK_NULL(cxml_get_dtd_node(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_get_xml_hdr_node){
    cxml_root_node *root = get_root("wf_xml_1.xml", false);

    CHECK_NOT_NULL(root);
    cxml_xhdr_node *node = cxml_get_xml_hdr_node(root);
    CHECK_NOT_NULL(node);
    CHECK_EQ(cxml_get_node_type(node), CXML_XHDR_NODE);
    cxml_destroy(root);

    root = get_root("wf_xml_5.xml", true);

    CHECK_NULL(cxml_get_xml_hdr_node(root));
    CHECK_NULL(cxml_get_xml_hdr_node(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_get_root_element){
    cxml_root_node *root = get_root("wf_xml_2.xml", false);
    CHECK_NOT_NULL(root);
    cxml_elem_node *elem = cxml_get_root_element(root);
    CHECK_NE(elem), NULL;;
    CHECK_NULL(cxml_get_root_element(NULL));
    cxml_destroy(root);
}

/*
 * create, select, update, delete  (CSUD)
 * one can argue that "create" is a specific form of "update".
 * The query api allows selection of nodes anywhere in the document, and also allows for
 * updating nodes, as well as deleting them.
 */
/*********************************
 *                               *
 * selection methods/functions   *
 *********************************
 */

int name_asserts(
        cxml_name *name,
        const char *pname,
        const char *lname,
        const char *qname)
{
    if (pname){
        CHECK_NULL(strncmp(name->pname, pname, name->pname_len));
    }
    if (lname){
        CHECK_NULL(strncmp(name->lname, lname, name->lname_len));
    }
    if (qname){
        CHECK_TRUE(cxml_string_raw_equals(&name->qname, qname));
    }
    return 1;
}

TEST(cxqapi, cxml_find){
    cxml_root_node *root = get_root("wf_xml_1.xml", true);
    CHECK_NOT_NULL(root);
    cxml_elem_node *elem = cxml_find(root, "<div>/");
    CHECK_NOT_NULL(elem);
    CHECK_EQ(name_asserts(&elem->name, NULL, "div", "div"), 1);
    cxml_elem_node *elem2 = cxml_find(root, "<div>/@mod/[@section]/");
    CHECK_EQ(elem, elem2);
    CHECK_EQ(name_asserts(&elem2->name, NULL, "div", "div"), 1);
    cxml_free_root_node(root);
}

TEST(cxqapi, cxml_find_all){
    cxml_root_node *root = get_root("wf_xml_2.xml", true);
    CHECK_NOT_NULL(root);
    cxml_list list = new_cxml_list();
    cxml_find_all(root, "<term>/$text/", &list);
    CHECK_EQ(cxml_list_size(&list), 4);
    cxml_for_each(elem, &list)
    {
        CHECK_EQ(name_asserts(&(_unwrap__cxnode(elem, elem))->name, NULL, "term", "term"), 1);
        CHECK_TRUE(_unwrap__cxnode(elem, elem)->has_text);
    }
    cxml_list_free(&list);

    cxml_find_all(root, "<term>/", NULL);
    CHECK_NULL(cxml_list_size(&list));
    cxml_find_all(root, NULL, &list);
    CHECK_NULL(cxml_list_size(&list));
    cxml_find_all(NULL, "<term>/$text/", &list);
    CHECK_NULL(cxml_list_size(&list));
    cxml_find_all(root, "<term>/#comment='foobar'/", &list);
    CHECK_NULL(cxml_list_size(&list));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_find_children){
    cxml_root_node *root = get_root("wf_xml_2.xml", true);
    CHECK_NOT_NULL(root);

    cxml_list list = new_cxml_list();
    cxml_find_children(root, "<synonym>/", &list);
    CHECK_GE(cxml_list_size(&list), 2);
    cxml_list_free(&list);

    cxml_elem_node *elem = root->root_element;
    cxml_find_children(elem, "<relationship>/", &list);
    CHECK_EQ(cxml_list_size(&list), 1);
    cxml_list_free(&list);

    cxml_find_children(elem, NULL, &list);
    CHECK_NULL(cxml_list_size(&list));

    cxml_find_children(NULL, "<relationship>/", &list);
    CHECK_NULL(cxml_list_size(&list));

    cxml_find_children(elem, "<relationship>/", NULL);
    CHECK_NULL(cxml_list_size(&list));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_children){
    cxml_root_node *root = get_root("wf_xml_4.xml", false);
    CHECK_NOT_NULL(root);

    cxml_list list = new_cxml_list();
    cxml_children(root, &list);
    CHECK_EQ(cxml_list_size(&list), 1);
    cxml_list_free(&list);

    cxml_elem_node *elem = root->root_element;
    cxml_children(elem, &list);
    CHECK_GE(cxml_list_size(&list), 3);
    cxml_list_free(&list);

    cxml_children(elem, NULL);
    CHECK_NULL(cxml_list_size(&list));

    cxml_children(NULL, &list);
    CHECK_NULL(cxml_list_size(&list));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_next_element){
    cxml_root_node *root = get_root("wf_xml_4.xml", false);
    CHECK_NOT_NULL(root);
    cxml_element_node *elem = cxml_find(root, "<f:width>/");
    CHECK_NOT_NULL(elem);
    CHECK_EQ(name_asserts(&elem->name, "f", "width", "f:width"), 1);
    CHECK_NOT_NULL(elem->namespace);
    CHECK_TRUE(cxml_string_raw_equals(&elem->namespace->prefix, "f"));

    cxml_element_node *next = cxml_next_element(elem);
    CHECK_NOT_NULL(next);
    CHECK_EQ(name_asserts(&next->name, "f", "length", "f:length"), 1);
    CHECK_NOT_NULL(next->namespace);
    CHECK_TRUE(cxml_string_raw_equals(&next->namespace->prefix, "f"));

    CHECK_NULL(cxml_next_element(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_previous_element){
    cxml_root_node *root = get_root("wf_xml_4.xml", true);
    CHECK_NOT_NULL(root);
    cxml_element_node *elem = cxml_find(root, "<f:width>/");
    CHECK_NOT_NULL(elem);
    CHECK_EQ(name_asserts(&elem->name, "f", "width", "f:width"), 1);
    CHECK_NOT_NULL(elem->namespace);
    CHECK_TRUE(cxml_string_raw_equals(&elem->namespace->prefix, "f"));

    cxml_element_node *previous = cxml_previous_element(elem);
    CHECK_NOT_NULL(previous);
    CHECK_EQ(name_asserts(&previous->name, "f", "name", "f:name"), 1);
    CHECK_NOT_NULL(previous->namespace);
    CHECK_TRUE(cxml_string_raw_equals(&previous->namespace->prefix, "f"));

    CHECK_NULL(cxml_previous_element(NULL));
    cxml_destroy(root);
}

// cimple
TEST(cxqapi, cxml_next_comment){
    cxml_root_node *root = get_root("wf_xml_3.xml", true);
    CHECK_NOT_NULL(root);
    cxml_element_node *elem = cxml_find(root, "<entry>/");
    CHECK_NOT_NULL(elem);
    CHECK_EQ(name_asserts(&elem->name, NULL, "entry", "entry"), 1);
    cxml_comment_node *comm = NULL;
    cxml_for_each(n, &elem->children)
    {
        if (_cxml_node_type(n) == CXML_COMM_NODE){
            comm = n;
            break;
        }
    }
    CHECK_TRUE(cxml_string_raw_equals(&comm->value, "foobar"));

    cxml_comment_node *next = cxml_next_comment(comm);
    CHECK_NOT_NULL(next);
    CHECK_TRUE(cxml_string_raw_equals(&next->value, "cimple"));

    CHECK_NULL(cxml_next_comment(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_previous_comment){
    cxml_root_node *root = get_root("wf_xml_3.xml", false);
    CHECK_NOT_NULL(root);
    cxml_element_node *elem = cxml_find(root, "<entry>/");
    CHECK_NOT_NULL(elem);
    CHECK_EQ(name_asserts(&elem->name, NULL, "entry", "entry"), 1);
    cxml_comment_node *comm = NULL;
    // obtain the last comment
    cxml_for_each(n, &elem->children)
    {
        if (_cxml_node_type(n) == CXML_COMM_NODE){
            if (comm){
                comm = n;
                break;
            }
            comm = n;
        }
    }
    CHECK_TRUE(cxml_string_raw_equals(&comm->value, "cimple"));

    cxml_comment_node *previous = cxml_previous_comment(comm);
    CHECK_NOT_NULL(previous);
    CHECK_TRUE(cxml_string_raw_equals(&previous->value, "foobar"));

    CHECK_NULL(cxml_previous_comment(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_next_text){
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    CHECK_NOT_NULL(root);
    cxml_element_node *elem = cxml_find(root, "<noodles>/");
    CHECK_NOT_NULL(elem);
    CHECK_EQ(name_asserts(&elem->name, NULL, "noodles", "noodles"), 1);
    cxml_text_node *text = NULL;
    cxml_for_each(n, &elem->children)
    {
        if (_cxml_node_type(n) == CXML_TEXT_NODE){
            text = n;
            break;
        }
    }
    CHECK_TRUE(cxml_string_raw_equals(&text->value, "indomie"));

    cxml_text_node *next = cxml_next_text(text);
    CHECK_NOT_NULL(next);
    CHECK_TRUE(cxml_string_raw_equals(&next->value, "super-pack"));

    CHECK_NULL(cxml_next_text(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_previous_text){
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    CHECK_NOT_NULL(root);
    cxml_element_node *elem = cxml_find(root, "<noodles>/");
    CHECK_NOT_NULL(elem);
    CHECK_EQ(name_asserts(&elem->name, NULL, "noodles", "noodles"), 1);
    cxml_text_node *text = NULL;
    cxml_for_each(n, &elem->children)
    {
        if (_cxml_node_type(n) == CXML_TEXT_NODE){
            if (text){
                text = n;
                break;
            }
            text = n;
        }
    }
    CHECK_TRUE(cxml_string_raw_equals(&text->value, "super-pack"));

    cxml_text_node *previous = cxml_previous_text(text);
    CHECK_NOT_NULL(previous);
    CHECK_TRUE(cxml_string_raw_equals(&previous->value, "indomie"));

    CHECK_NULL(cxml_previous_text(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_first_child){
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    CHECK_NOT_NULL(root);

    cxml_element_node *elem = cxml_find(root, "<noodles>/");
    CHECK_NOT_NULL(elem);

    cxml_text_node *first = cxml_first_child(elem);
    CHECK_NOT_NULL(first);
    CHECK_TRUE(cxml_string_raw_equals(&first->value, "indomie"));
    CHECK_NULL(cxml_first_child(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_find_first_child){
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    CHECK_NOT_NULL(root);

    cxml_text_node *first = cxml_find_first_child(root, "<noodles>/");
    CHECK_NOT_NULL(first);
    CHECK_TRUE(cxml_string_raw_equals(&first->value, "indomie"));
    CHECK_NULL(cxml_find_first_child(NULL, "<entry>/"));
    CHECK_NULL(cxml_find_first_child(root, "<foobar>/"));
    CHECK_NULL(cxml_find_first_child(root, NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_parent){
    cxml_root_node *root = get_root("wf_xml_2.xml", true);
    CHECK_NOT_NULL(root);
    // finds first element
    cxml_element_node *elem = cxml_find(root, "<relationship>/");
    cxml_element_node *par = cxml_find(root, "<synonym>/");
    CHECK_NOT_NULL(elem);
    CHECK_NOT_NULL(par);

    cxml_element_node *parent = cxml_parent(elem);
    CHECK_NOT_NULL(par);
    CHECK_EQ(par, parent);
    CHECK_EQ(name_asserts(&parent->name, NULL, "synonym", "synonym"), 1);

    CHECK_NULL(cxml_parent(root));
    CHECK_NULL(cxml_parent(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_find_parent){
    cxml_root_node *root = get_root("wf_xml_4.xml", false);
    CHECK_NOT_NULL(root);
    // finds first element
    cxml_element_node *elem = cxml_find(root, "<f:width>/");
    cxml_element_node *par = cxml_find(root, "<f:table>/");
    CHECK_NOT_NULL(elem);
    CHECK_NOT_NULL(par);

    cxml_element_node *parent = cxml_find_parent(root, "<f:width>/");
    CHECK_NOT_NULL(parent);
    CHECK_EQ(par, parent);
    CHECK_EQ(name_asserts(&parent->name, "f", "table", "f:table"), 1);

    CHECK_NULL(cxml_find_parent(root, "<xyz>/"));
    CHECK_NULL(cxml_find_parent(root, NULL));
    CHECK_NULL(cxml_find_parent(NULL, "<f:width>/"));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_ancestors){
    cxml_cfg_set_doc_name("ancestor");
    cxml_root_node *root = get_root("wf_xml_2.xml", false);
    CHECK_NOT_NULL(root);
    cxml_element_node *elem = cxml_find(root, "<relationship>/$text|='xyz'/");
    CHECK_NOT_NULL(elem);

    cxml_list ancestors = new_cxml_list();
    char *ancestor_names[] = {"synonym", "entry", "thesaurus", "ancestor"};
    cxml_ancestors(elem, &ancestors);
    CHECK_EQ(cxml_list_size(&ancestors), 4);
    int i = 0;
    cxml_for_each(node, &ancestors)
    {
        if (cxml_get_node_type(node) != CXML_ROOT_NODE){
            CHECK_EQ(name_asserts, 1(;
                    &_unwrap__cxnode(elem, node)->name, NULL, ancestor_names[i], ancestor_names[i]))
        }else{
            CHECK_EQ(cxml_string_raw_equals, 1(;
                    &_unwrap__cxnode(root, node)->name, ancestor_names[i]))
        }
        i++;
    }
    cxml_list_free(&ancestors);
    cxml_ancestors(NULL, &ancestors);
    CHECK_NULL(cxml_list_size(&ancestors));
    cxml_ancestors(elem, NULL);
    cxml_destroy(root);
}

TEST(cxqapi, cxml_find_ancestors){
    cxml_root_node *root = get_root("wf_xml_2.xml", true);
    CHECK_NOT_NULL(root);

    cxml_list ancestors = new_cxml_list();
    char *ancestor_names[] = {"synonym", "entry", "thesaurus", "XMLDocument"};
    cxml_find_ancestors(root, "<relationship>/$text|='xyz'/", &ancestors);
    CHECK_EQ(cxml_list_size(&ancestors), 4);
    int i = 0;
    cxml_for_each(node, &ancestors)
    {
        if (cxml_get_node_type(node) != CXML_ROOT_NODE){
            CHECK_EQ(name_asserts, 1(;
                    &_unwrap__cxnode(elem, node)->name, NULL, ancestor_names[i], ancestor_names[i]))
        }else{
            CHECK_EQ(cxml_string_raw_equals, 1(;
                    &_unwrap__cxnode(root, node)->name, ancestor_names[i]))
        }
        i++;
    }
    cxml_list_free(&ancestors);

    cxml_ancestors(NULL, &ancestors);
    CHECK_NULL(cxml_list_size(&ancestors));

    cxml_find_ancestors(root, NULL, &ancestors);
    CHECK_NULL(cxml_list_size(&ancestors));

    cxml_find_ancestors(NULL, "<relationship>/$text|='abc'/", &ancestors);
    CHECK_NULL(cxml_list_size(&ancestors));

    cxml_find_ancestors(root, "<relationship>/$text|='abc'/", NULL);
    CHECK_NULL(cxml_list_size(&ancestors));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_descendants){
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    CHECK_NOT_NULL(root);
    cxml_element_node *elem = cxml_find(root, "<noodles>/");
    CHECK_NOT_NULL(elem);

    cxml_list descendants = new_cxml_list();
    char *ancestor_names_v[] = {
            "indomie", "seasoning", "maggi", "br", "mr-chef",
            "super-pack", "others"
    };
    cxml_descendants(elem, &descendants);
    CHECK_EQ(cxml_list_size(&descendants), 7);
    int i = 0;
    cxml_for_each(node, &descendants)
    {
        if (cxml_get_node_type(node) != CXML_TEXT_NODE){
            CHECK_EQ(name_asserts, 1(;
                    &_unwrap__cxnode(elem, node)->name, NULL,
                    ancestor_names_v[i], ancestor_names_v[i]))
        }else{
            CHECK_EQ(cxml_string_raw_equals, 1(;
                    &_unwrap__cxnode(text, node)->value, ancestor_names_v[i]))
        }
        i++;
    }
    cxml_list_free(&descendants);

    cxml_descendants(NULL, &descendants);
    CHECK_NULL(cxml_list_size(&descendants));

    cxml_descendants(elem, NULL);
    CHECK_NULL(cxml_list_size(&descendants));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_find_descendants){
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    CHECK_NOT_NULL(root);

    cxml_list descendants = new_cxml_list();
    char *ancestor_names_v[] = {
            "indomie", "seasoning", "maggi", "br", "mr-chef",
            "super-pack", "others"
    };
    cxml_find_descendants(root, "<noodles>/", &descendants);
    CHECK_EQ(cxml_list_size(&descendants), 7);
    int i = 0;
    cxml_for_each(node, &descendants)
    {
        if (cxml_get_node_type(node) != CXML_TEXT_NODE){
            CHECK_EQ(name_asserts, 1(;
                    &_unwrap__cxnode(elem, node)->name,
                    NULL, ancestor_names_v[i], ancestor_names_v[i]))
        }else{
            CHECK_EQ(cxml_string_raw_equals, 1(;
                    &_unwrap__cxnode(text, node)->value, ancestor_names_v[i]))
        }
        i++;
    }
    cxml_list_free(&descendants);
    cxml_find_descendants(NULL, "<noodles>/", &descendants);
    CHECK_NULL(cxml_list_size(&descendants));

    cxml_find_descendants(root, NULL, &descendants);
    CHECK_NULL(cxml_list_size(&descendants));

    cxml_find_descendants(root, "<noodles>/", NULL);
    CHECK_NULL(cxml_list_size(&descendants));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_next_sibling){
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    CHECK_NOT_NULL(root);
    cxml_element_node *elem = cxml_find(root, "<name>/");
    CHECK_NOT_NULL(elem);
    CHECK_EQ(name_asserts(&elem->name, NULL, "name", "name"), 1);

    cxml_element_node *next = cxml_next_sibling(elem);
    CHECK_NOT_NULL(next);
    CHECK_EQ(name_asserts(&next->name, NULL, "color", "color"), 1);
    CHECK_NULL(next->namespace);

    CHECK_NULL(cxml_next_sibling(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_previous_sibling){
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    CHECK_NOT_NULL(root);
    cxml_element_node *elem = cxml_find(root, "<br>/");
    CHECK_NOT_NULL(elem);
    CHECK_EQ(name_asserts(&elem->name, NULL, "br", "br"), 1);

    cxml_text_node *previous = cxml_previous_sibling(elem);
    CHECK_NOT_NULL(previous);
    CHECK_TRUE(cxml_string_raw_equals(&previous->value, "red"));
    CHECK_NULL(cxml_previous_sibling(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_find_next_sibling){
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    CHECK_NOT_NULL(root);

    cxml_element_node *next = cxml_find_next_sibling(root, "<color>/");
    CHECK_NOT_NULL(next);
    CHECK_EQ(name_asserts(&next->name, NULL, "shape", "shape"), 1);
    CHECK_NULL(next->namespace);

    CHECK_NULL(cxml_find_next_sibling(root, NULL));
    CHECK_NULL(cxml_find_next_sibling(NULL, "<br>/"));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_find_previous_sibling){
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    CHECK_NOT_NULL(root);

    cxml_element_node *previous = cxml_find_previous_sibling(root, "<shape>/");
    CHECK_NOT_NULL(previous);
    CHECK_EQ(name_asserts(&previous->name, NULL, "color", "color"), 1);
    CHECK_NULL(previous->namespace);

    CHECK_NULL(cxml_find_next_sibling(root, NULL));
    CHECK_NULL(cxml_find_next_sibling(NULL, "<br>/"));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_siblings){
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    CHECK_NOT_NULL(root);
    cxml_element_node *elem = cxml_find(root, "<seasoning>/");
    CHECK_NOT_NULL(elem);

    cxml_list siblings = new_cxml_list();
    char *sibling_names_v[] = {
            "indomie", "super-pack", "others"
    };
    cxml_siblings(elem, &siblings);
    CHECK_EQ(cxml_list_size(&siblings), 3);
    int i = 0;
    cxml_for_each(node, &siblings)
    {
        if (cxml_get_node_type(node) != CXML_TEXT_NODE){
            CHECK_EQ(name_asserts, 1(;
                    &_unwrap__cxnode(elem, node)->name, NULL,
                    sibling_names_v[i], sibling_names_v[i]))
        }else{
            CHECK_EQ(cxml_string_raw_equals, 1(;
                    &_unwrap__cxnode(text, node)->value, sibling_names_v[i]))
        }
        i++;
    }
    cxml_list_free(&siblings);

    cxml_siblings(NULL, &siblings);
    CHECK_NULL(cxml_list_size(&siblings));

    cxml_siblings(elem, NULL);
    CHECK_NULL(cxml_list_size(&siblings));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_find_siblings){
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    CHECK_NOT_NULL(root);

    cxml_list siblings = new_cxml_list();
    char *sibling_names_v[] = {
            "indomie", "super-pack", "others"
    };
    cxml_find_siblings(root, "<seasoning>/", &siblings);
    CHECK_EQ(cxml_list_size(&siblings), 3);
    int i = 0;
    cxml_for_each(node, &siblings)
    {
        if (cxml_get_node_type(node) != CXML_TEXT_NODE){
            CHECK_EQ(name_asserts, 1(;
                    &_unwrap__cxnode(elem, node)->name, NULL, sibling_names_v[i], sibling_names_v[i]))
        }else{
            CHECK_EQ(cxml_string_raw_equals, 1(;
                    &_unwrap__cxnode(text, node)->value, sibling_names_v[i]))
        }
        i++;
    }
    cxml_list_free(&siblings);

    cxml_siblings(NULL, &siblings);
    CHECK_NULL(cxml_list_size(&siblings));

    cxml_find_siblings(root, NULL, &siblings);
    CHECK_NULL(cxml_list_size(&siblings));

    cxml_find_siblings(NULL, "<seasoning>/", &siblings);
    CHECK_NULL(cxml_list_size(&siblings));

    cxml_find_siblings(root, "<seasoning>/", NULL);
    CHECK_NULL(cxml_list_size(&siblings));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_get_attribute){
    cxml_root_node *root = get_root("wf_xml_1.xml", false);
    CHECK_NOT_NULL(root);

    cxml_element_node *elem = cxml_get_root_element(root);
    cxml_attribute_node *attr = cxml_get_attribute(elem, "fish");
    CHECK_NOT_NULL(attr);
    CHECK_EQ(name_asserts(&attr->name, NULL, "fish", "fish"), 1);
    CHECK_TRUE(cxml_string_raw_equals(&attr->value, "mackerel"));

    CHECK_NULL(cxml_get_attribute(NULL, "fish"));
    CHECK_NULL(cxml_get_attribute(elem, NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_find_attribute){
    cxml_root_node *root = get_root("wf_xml_1.xml", true);
    CHECK_NOT_NULL(root);

    cxml_attribute_node *attr = cxml_find_attribute(root, "<mod>/", "section");
    CHECK_NOT_NULL(attr);
    CHECK_EQ(name_asserts(&attr->name, NULL, "section", "section"), 1);
    CHECK_TRUE(cxml_string_raw_equals(&attr->value, "converter"));

    CHECK_EQ(cxml_find_attribute(root, "<xyz>/", "section")) // no element xy, NULLz;
    CHECK_EQ(cxml_find_attribute(root, NULL, "section"))   // NUL, NULLL;
    CHECK_EQ(cxml_find_attribute(root, "<xyz>/", NULL))    // no attribute nam, NULLe;
    CHECK_EQ(cxml_find_attribute(NULL, "<xyz>/", "section"))   // no root nod, NULLe;
    CHECK_EQ(cxml_find_attribute(root, "<and>/", ""))  // empty attribut, NULLe;
    cxml_destroy(root);
}

TEST(cxqapi, cxml_attributes){
    cxml_root_node *root = get_root("wf_xml_8.xml", true);
    CHECK_NOT_NULL(root);

    cxml_element_node *elem = cxml_find(root, "<or>/");
    CHECK_NOT_NULL(elem);

    cxml_list attributes = new_cxml_list();
    cxml_attributes(elem, &attributes);
    CHECK_EQ(cxml_list_size(&attributes), 3);

    char *attr_names[]  = {"mod", "van", "hex"};
    char *attr_values[] = {"63", "'40'", "0xdeadbeef"};
    long attr_num_values[] = {63, 0, 0xdeadbeef};

    int i = 0;
    cxml_for_each(attr, &attributes)
    {
        CHECK_EQ(name_asserts(&((cxml_attribute_node*)attr)->name, NULL, attr_names[i], attr_names[i]), 1);
        CHECK_TRUE(cxml_string_raw_equals(&((cxml_attribute_node *) attr)->value, attr_values[i]));
        CHECK_EQ(((cxml_attribute_node*)attr)->number_value.dec_val, attr_num_values[i]);
        i++;
    }

    cxml_list_free(&attributes);

    cxml_attributes(NULL, &attributes);
    CHECK_NULL(cxml_list_size(&attributes));

    cxml_attributes(elem, NULL);
    CHECK_NULL(cxml_list_size(&attributes));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_find_attributes){
    cxml_root_node *root = get_root("wf_xml_8.xml", false);
    CHECK_NOT_NULL(root);

    cxml_list attributes = new_cxml_list();
    cxml_find_attributes(root, "<or>/", &attributes);
    CHECK_EQ(cxml_list_size(&attributes), 3);

    char *attr_names[]  = {"mod", "van", "hex"};
    char *attr_values[] = {"63", "'40'", "0xdeadbeef"};
    long attr_num_values[] = {63, 0, 0xdeadbeef};

    int i = 0;
    cxml_for_each(attr, &attributes)
    {
        CHECK_EQ(name_asserts(&((cxml_attribute_node*)attr)->name, NULL, attr_names[i], attr_names[i]), 1);
        CHECK_TRUE(cxml_string_raw_equals(&((cxml_attribute_node *) attr)->value, attr_values[i]));
        CHECK_EQ(((cxml_attribute_node*)attr)->number_value.dec_val, attr_num_values[i]);
        i++;
    }

    cxml_list_free(&attributes);

    cxml_find_attributes(root, NULL, &attributes);
    CHECK_NULL(cxml_list_size(&attributes));

    cxml_find_attributes(root, "<and>/", NULL);
    CHECK_NULL(cxml_list_size(&attributes));

    cxml_find_attributes(NULL, "<div>/", &attributes);
    CHECK_NULL(cxml_list_size(&attributes));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_text_as_cxml_string){
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    CHECK_NOT_NULL(root);

    //text
    cxml_string text = new_cxml_string_s("indomiemaggimr-chefsuper-pack");
    cxml_string *str = cxml_text_as_cxml_string(root, NULL);
    CHECK_TRUE(cxml_string_equals(&text, str));
    CHECK_NULL(cxml_text(NULL, NULL));
    cxml_string_free(&text);
    cxml_string_free(str);
    FREE(str);

    text = new_cxml_string_s("indomie~maggi~mr-chef~super-pack");
    str = cxml_text_as_cxml_string(root, "~");
    CHECK_TRUE(cxml_string_equals(&text, str));
    cxml_string_free(&text);
    cxml_string_free(str);
    FREE(str);

    text = new_cxml_string_s("indomie maggi mr-chef super-pack");
    str = cxml_text_as_cxml_string(root, " ");
    CHECK_TRUE(cxml_string_equals(&text, str));
    cxml_string_free(&text);
    cxml_string_free(str);
    FREE(str);

    text = new_cxml_string_s("indomiemaggimr-chefsuper-pack");
    str = cxml_text_as_cxml_string(root, "");
    CHECK_TRUE(cxml_string_equals(&text, str));
    cxml_string_free(&text);
    cxml_string_free(str);
    FREE(str);

    CHECK_NULL(cxml_text_as_cxml_string(NULL, "foobar"));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_text){
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    CHECK_NOT_NULL(root);

    //text
    cxml_string text = new_cxml_string_s("appleredblueroundish");
    char *str = cxml_text(root, NULL);
    CHECK_TRUE(cxml_string_lraw_equals(&text, str, strlen(str)));
    CHECK_NULL(cxml_text(NULL, NULL));
    cxml_string_free(&text);
    FREE(str);

    text = new_cxml_string_s("apple|red|blue|roundish");
    str = cxml_text(root, "|");
    CHECK_TRUE(cxml_string_lraw_equals(&text, str, strlen(str)));
    cxml_string_free(&text);
    FREE(str);

    text = new_cxml_string_s("apple^^^red^^^blue^^^roundish");
    str = cxml_text(root, "^^^");
    CHECK_TRUE(cxml_string_lraw_equals(&text, str, strlen(str)));
    cxml_string_free(&text);
    FREE(str);

    text = new_cxml_string_s("appleredblueroundish");
    str = cxml_text(root, "");
    CHECK_TRUE(cxml_string_lraw_equals(&text, str, strlen(str)));
    cxml_string_free(&text);
    FREE(str);

    CHECK_NULL(cxml_text(NULL, "foobar"));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_get_name){
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    CHECK_NOT_NULL(root);
    // returned name must be freed by us
    char *name = cxml_get_name(root);
    // default document name is XMLDocument
    CHECK_TRUE(cxml_string_lraw_equals(&root->name, name, strlen(name)));
    FREE(name);

    cxml_element_node *elem = cxml_find(root, "<shape>/");
    name = cxml_get_name(elem);
    CHECK_TRUE(cxml_string_lraw_equals(&elem->name.qname, name, strlen(name)));
    FREE(name);

    CHECK_EQ(cxml_get_name(NULL)), NULL;;
    cxml_destroy(root);

}

TEST(cxqapi, cxml_get_bound_namespace){
    /*
     * "<f:table xmlns:f=\"https://www.w3schools.com/furniture\">\n"
"        <f:name>African Coffee Table<![CDATA[Testing cdata stuff]]></f:name>\n"
"        <f:width>80</f:width>\n"
"        <f:length>120</f:length>\n"
" </f:table>";
     */
    cxml_root_node *root = get_root("wf_xml_4.xml", true);
    CHECK_NOT_NULL(root);

    cxml_element_node *elem = cxml_find(root, "<f:width>/");
    CHECK_NOT_NULL(elem);

    cxml_element_node *root_elem = root->root_element; // 'f:table'
    CHECK_NOT_NULL(root_elem);
    CHECK_NOT_NULL(root_elem->namespaces);

    // obtain the namespace the element elem is bound to
    cxml_namespace_node *ns = cxml_get_bound_namespace(elem);
    CHECK_NOT_NULL(ns);

    // find the namespace node from where it was declared in its parent element (root_elem)
    cxml_namespace_node *target = cxml_list_get(root_elem->namespaces, 0);

    // check that the namespace found is same as the namespace bound to the element 'f:width'
    CHECK_EQ(ns, target);
    CHECK_TRUE(cxml_string_equals(&ns->prefix, &target->prefix));
    CHECK_TRUE(cxml_string_equals(&ns->uri, &target->uri));

    CHECK_NULL(cxml_get_bound_namespace(NULL));

    cxml_destroy(root);

}

TEST(cxqapi, cxml_get_comments){
    cxml_root_node *root = get_root("wf_xml_3.xml", false);
    CHECK_NOT_NULL(root);

    cxml_list comments = new_cxml_list();
    cxml_get_comments(root, &comments, 1);
    CHECK_EQ(cxml_list_size(&comments), 2);

    char *values[] = {"foobar", "cimple"};
    int i = 0;
    cxml_for_each(comment, &comments)
    {
        CHECK_TRUE(cxml_string_lraw_equals(;
                &((cxml_comment_node *) comment)->value,
                values[i], strlen(values[i])))
        i++;
    }
    cxml_list_free(&comments);

    cxml_get_comments(root, &comments, 0);
    CHECK_NULL(cxml_list_size(&comments));

    cxml_element_node *elem = cxml_find(root, "<entry>/");
    cxml_get_comments(elem, &comments, 1);
    CHECK_EQ(cxml_list_size(&comments), 2);

    cxml_list_free(&comments);

    cxml_get_comments(root, NULL, 0);
    CHECK_NULL(cxml_list_size(&comments));

    cxml_get_comments(NULL, &comments, 0);
    CHECK_NULL(cxml_list_size(&comments));

    cxml_destroy(root);
}


/*********************************
 *                               *
 * creation methods/functions    *
 *********************************
 */
/**General**/
TEST(cxqapi, cxml_create_node){
    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    CHECK_NOT_NULL(elem);
    CHECK_EQ(elem->_type, CXML_ELEM_NODE);

    CHECK_NULL(cxml_create_node(100));
    cxml_destroy(elem);
}

TEST(cxqapi, cxml_set_name){
    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    CHECK_NOT_NULL(elem);

    // cannot use xmlns as prefix name of element
    CHECK_FALSE(cxml_set_name(elem, "xmlns", "bar"));
    CHECK_TRUE(cxml_set_name(elem, "foo", "bar"));
    CHECK_EQ(name_asserts(&elem->name, "foo", "bar", "foo:bar"), 1);
    cxml_name_free(&elem->name);

    // fails: we can't set prefix name, if the element/attribute has no local name,
    // without setting a local name (local name cannot be empty)
    CHECK_FALSE(cxml_set_name(elem, "foo", NULL));
    CHECK_FALSE(cxml_set_name(elem, "foo", ""));
    CHECK_FALSE(cxml_set_name(elem, NULL, ""));
    CHECK_FALSE(cxml_set_name(elem, "", "new"));
    CHECK_FALSE(cxml_set_name(NULL, "foo", "new"));

    CHECK_TRUE(cxml_set_name(elem, "xy", "abc"));
    CHECK_EQ(name_asserts(&elem->name, "xy", "abc", "xy:abc"), 1);

    CHECK_TRUE(cxml_set_name(elem, NULL, "abc"));
    CHECK_EQ(name_asserts(&elem->name, "xy", "abc", "xy:abc"), 1);
    cxml_destroy(elem);


    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE);
    CHECK_NOT_NULL(attr);

    CHECK_TRUE(cxml_set_name(attr, "foo", "bar"));;
    CHECK_EQ(name_asserts(&attr->name, "foo", "bar", "foo:bar"), 1);

    // this shouldn't fail, we can set the prefix name since the local name isn't empty.
    // (it was set above.)
    CHECK_TRUE(cxml_set_name(attr, "foo", NULL));

    // fails: we can't set prefix name, if the element/attribute has no local name,
    // without setting a local name (local name cannot be empty)
    CHECK_FALSE(cxml_set_name(attr, "foo", ""));
    CHECK_FALSE(cxml_set_name(attr, NULL, ""));
    CHECK_FALSE(cxml_set_name(attr, "", "new"));
    CHECK_FALSE(cxml_set_name(NULL, "foo", "new"));
    CHECK_EQ(name_asserts(&attr->name, "foo", "bar", "foo:bar"), 1);

    // this should fail (not a valid identifier)
    CHECK_FALSE(cxml_set_name(attr, NULL, "203"));

    CHECK_EQ(name_asserts(&attr->name, "foo", "bar", "foo:bar"))  // unchange, 1d;

    CHECK_FALSE(cxml_set_name(attr, ".123", "bad"));
    CHECK_EQ(name_asserts(&attr->name, "foo", "bar", "foo:bar"))  // unchange, 1d;

    CHECK_TRUE(cxml_set_name(attr, "xy", "abc"));
    CHECK_EQ(name_asserts(&attr->name, "xy", "abc", "xy:abc"), 1);
    CHECK_TRUE(cxml_set_name(attr, NULL, "bad"));
    CHECK_EQ(name_asserts(&attr->name, "xy", "bad", "xy:bad"), 1);

    // we can set the prefix name alone since the local name isn't empty.
    CHECK_TRUE(cxml_set_name(attr, "dodo", NULL));
    CHECK_EQ(name_asserts(&attr->name, "dodo", "bad", "dodo:bad"), 1);

    cxml_destroy(attr);

}

TEST(cxqapi, cxml_add_child){
    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    CHECK_NOT_NULL(elem);
    CHECK_TRUE(cxml_set_name(elem, NULL, "abc"));
    CHECK_EQ(name_asserts(&elem->name, NULL, "abc", "abc"), 1);
    CHECK_NULL(cxml_list_size(&elem->children));
    CHECK_FALSE(elem->has_text);
    CHECK_FALSE(elem->has_child);

    char *got = cxml_element_to_rstring(elem);
    CHECK_NOT_NULL(got);
    char *expected = "<abc/>";
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    FREE(got);
    got = NULL;

    cxml_text_node *text = cxml_create_node(CXML_TEXT_NODE);
    CHECK_NOT_NULL(text);
    CHECK_NULL(text->parent);

    cxml_set_text_value(text, "this is some text", false);
    CHECK_TRUE(cxml_add_child(elem, text));
    CHECK_NOT_NULL(text->parent);
    CHECK_NULL(text->number_value.dec_val);
    CHECK_EQ(text->parent, elem);
    CHECK_TRUE(elem->has_child);
    CHECK_TRUE(elem->has_text);
    CHECK_EQ(cxml_list_size(&elem->children), 1);

    got = cxml_element_to_rstring(elem);
    CHECK_NOT_NULL(got);
    expected = "<abc>\n"
               "  this is some text\n"
               "</abc>";
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    FREE(got);

    CHECK_FALSE(cxml_add_child(elem, NULL));
    CHECK_EQ(cxml_list_size(&elem->children), 1);

    CHECK_FALSE(cxml_add_child(NULL, text));
    CHECK_EQ(cxml_list_size(&elem->children), 1);
    // this also frees `text` above
    cxml_destroy(elem);;
}

TEST(cxqapi, cxml_set_namespace){
    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    CHECK_TRUE(cxml_set_name(elem, "pp", "element"));
    CHECK_TRUE(name_asserts(&elem->name, "pp", "element", "pp:element"));
    // the element becomes namespaced when it is bound to an actual namespace object
    CHECK_FALSE(elem->is_namespaced);
    CHECK_NULL(elem->namespace);
    cxml_namespace_node *ns = cxml_create_node(CXML_NS_NODE);
    CHECK_NOT_NULL(ns);
    char *pref = "prefix";
    char *uri = "http://not-a-uri.com/";
    // cannot set xmlns as namespace prefix
    CHECK_FALSE(cxml_set_namespace_prefix(ns, "xmlns"));
    // we cannot call cxml_set_namespace_prefix() to directly set the
    // namespace prefix to "xml" as the query API does not trust that we'll
    // call cxml_set_namespace_uri() to set the corresponding URI for the xml prefix.
    // the only way this can be done, is by using cxml_set_namespace_data()
    CHECK_FALSE(cxml_set_namespace_prefix(ns, "xml"));
    // the standard uri must be used for the xml uri; this isn't it
    CHECK_FALSE(cxml_set_namespace_uri(ns, "http://www.w3.org/2000/xmlns/"));

    CHECK_TRUE(cxml_set_namespace_prefix(ns, pref));
    CHECK_TRUE(cxml_set_namespace_uri(ns, uri));
    CHECK_TRUE(cxml_string_raw_equals(&ns->prefix, pref));
    CHECK_TRUE(cxml_string_raw_equals(&ns->uri, uri));
    // mismatching prefix
    CHECK_FALSE(cxml_set_namespace(elem, ns));
    CHECK_TRUE(cxml_set_name(elem, "prefix", NULL));

    CHECK_TRUE(cxml_set_namespace(elem, ns));
    CHECK_NOT_NULL(elem->namespace);
    // now its bound
    CHECK_TRUE(elem->is_namespaced);
    CHECK_EQ(elem->namespace, ns);

    CHECK_FALSE(cxml_set_namespace(elem, NULL));
    CHECK_FALSE(cxml_set_namespace(NULL, ns));

    cxml_destroy(elem);

    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE);
    CHECK_NOT_NULL(attr);
    // fails, no prefix
    CHECK_FALSE(cxml_set_namespace(attr, ns));

    CHECK_TRUE(cxml_set_name(attr, NULL, "attribute"));
    CHECK_TRUE(name_asserts(&attr->name, NULL, "attribute", "attribute"));
    // fails, no prefix
    CHECK_FALSE(cxml_set_namespace(attr, ns));

    // we can set the prefix name alone since the local name isn't empty.
    CHECK_TRUE(cxml_set_name(attr, "prefix", NULL));
    CHECK_TRUE(name_asserts(&attr->name, "prefix", "attribute", "prefix:attribute"));
    // succeeds
    CHECK_TRUE(cxml_set_namespace(attr, ns));
    CHECK_NOT_NULL(attr->namespace);
    CHECK_EQ(attr->namespace, ns);

    cxml_destroy(attr);
    // we have to destroy `ns` because the element doesn't own it. It's only bound to it.
    // so when `elem` was destroyed/deleted, it doesn't affect `ns`
    cxml_destroy(ns);
}

TEST(cxqapi, cxml_add_namespace){
    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    CHECK_NOT_NULL(elem);
    CHECK_TRUE(cxml_set_name(elem, NULL, "element"));
    CHECK_NULL(elem->namespaces);
    char *str = cxml_element_to_rstring(elem);
    CHECK_NOT_NULL(str);
    CHECK_TRUE(cxml_string_llraw_equals(str, "<element/>", strlen(str), 10));
    FREE(str);

    cxml_namespace_node *ns = cxml_create_node(CXML_NS_NODE);
    CHECK_NOT_NULL(ns);
    char *pref = "xml";
    char *uri = "http://www.w3.org/XML/1998/namespace";
    // fails: this uri belongs to xmlns
    CHECK_FALSE(cxml_set_namespace_data(ns, "xyz", "http://www.w3.org/2000/xmlns/"));
    CHECK_FALSE(cxml_set_namespace_data(ns, "xyz", uri));
    CHECK_TRUE(cxml_set_namespace_data(ns, pref, uri));
    // elem now owns `ns`
    CHECK_TRUE(cxml_add_namespace(elem, ns));
    CHECK_NOT_NULL(elem->namespaces);
    CHECK_EQ(cxml_list_size(elem->namespaces), 1);
    str = cxml_element_to_rstring(elem);
    CHECK_NOT_NULL(str);
    // global namespaces are not shown
    CHECK_TRUE(cxml_string_llraw_equals(str, "<element/>", strlen(str), 10));
    // cannot set namespace of `elem` to `ns` -> 'xml' since `elem` doesn't have this prefix,
    CHECK_FALSE(cxml_set_namespace(elem, ns));
    // update `elem`'s name to include the prefix of the namespace node it wants to be set to
    CHECK_TRUE(cxml_set_name(elem, "xml", "element"));
    CHECK_TRUE(cxml_set_namespace(elem, ns));
    FREE(str);
    str = cxml_element_to_rstring(elem);
    CHECK_TRUE(cxml_string_llraw_equals(str, "<xml:element/>", strlen(str), 14));
    FREE(str);

    CHECK_FALSE(cxml_add_namespace(NULL, ns));
    CHECK_FALSE(cxml_add_namespace(elem, NULL));
    // `elem` owns `ns` so it frees it automatically.
    cxml_destroy(elem);
}

TEST(cxqapi, cxml_add_attribute){
    // "<fruit a=\"boy\"><name>apple</name></fruit>"
    cxml_root_node *root = cxml_load_string(wf_xml_09);
    CHECK_NOT_NULL(root);
    cxml_element_node *elem = root->root_element;
    CHECK_EQ(cxml_table_size(elem->attributes), 1);

    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE),
                        *attr2 = cxml_create_node(CXML_ATTR_NODE);
    CHECK_NOT_NULL(attr);
    CHECK_NOT_NULL(attr2);
    cxml_set_attribute_data(attr, NULL, "soup", "egusi");
    cxml_set_attribute_data(attr2, "xml", "fish", "coté");

    CHECK_TRUE(cxml_add_attribute(elem, attr));
    CHECK_TRUE(cxml_add_attribute(elem, attr2));
    CHECK_EQ(cxml_table_size(elem->attributes), 3);
    char *got = cxml_element_to_rstring(elem);
    CHECK_NOT_NULL(got);
    char *expected = "<fruit a=\"boy\" soup=\"egusi\" xml:fish=\"coté\">\n"
                     "  <name>\n"
                     "    apple\n"
                     "  </name>\n"
                     "</fruit>";
    CHECK_TRUE(cxml_string_llraw_equals(expected, got, strlen(expected), strlen(got)));
    FREE(got);

    CHECK_FALSE(cxml_add_attribute(elem, NULL));
    CHECK_FALSE(cxml_add_attribute(NULL, attr));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_set_attribute_value){
    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE);
    CHECK_NOT_NULL(attr);
    CHECK_TRUE(cxml_set_attribute_value(attr, "sweet"));
    CHECK_TRUE(cxml_string_lraw_equals(&attr->value, "sweet", 5));
    cxml_string_free(&attr->value);

    CHECK_TRUE(cxml_set_attribute_value(attr, "0xfeedbac"));
    CHECK_EQ(attr->number_value.dec_val, 0xfeedbac);

    CHECK_FALSE(cxml_set_attribute_value(attr, NULL));
    CHECK_FALSE(cxml_set_attribute_value(NULL, "sweet"));

    cxml_destroy(attr);
}

TEST(cxqapi, cxml_set_attribute_name){
    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE);
    CHECK_NOT_NULL(attr);
    CHECK_TRUE(cxml_set_attribute_name(attr, NULL, "sweet"));
    CHECK_TRUE(cxml_string_lraw_equals(&attr->name.qname, "sweet", 5));
    char *str = cxml_attribute_to_rstring(attr);
    CHECK_TRUE(cxml_string_llraw_equals(str, "sweet=\"\"", strlen(str), 8));
    cxml_name_free(&attr->name);
    FREE(str);

    CHECK_TRUE(cxml_set_attribute_name(attr, "xyz", "sweet"));
    str = cxml_attribute_to_rstring(attr);
    CHECK_TRUE(cxml_string_llraw_equals(str, "xyz:sweet=\"\"", strlen(str), 12));
    FREE(str);

    CHECK_FALSE(cxml_set_attribute_name(attr, "", ""));
    CHECK_FALSE(cxml_set_attribute_name(attr, NULL, ""));
    CHECK_FALSE(cxml_set_attribute_name(NULL, "fx", ""));
    CHECK_FALSE(cxml_set_attribute_name(NULL, "fx", "amazing"));

    cxml_destroy(attr);
}

TEST(cxqapi, cxml_set_attribute_data){
    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE);
    CHECK_NOT_NULL(attr);
    CHECK_TRUE(cxml_set_attribute_data(attr, "xml", "food", "rice"));
    char *str = cxml_attribute_to_rstring(attr);
    CHECK_TRUE(cxml_string_llraw_equals(str, "xml:food=\"rice\"", strlen(str), 15));
    FREE(str);

    CHECK_FALSE(cxml_set_attribute_data(attr, "xmlns", NULL, "rice"));
    CHECK_FALSE(cxml_set_attribute_data(NULL, "xml", "food", "rice"));
    CHECK_FALSE(cxml_set_attribute_data(attr, "xml", "food", NULL));
    cxml_destroy(attr);

}

TEST(cxqapi, cxml_set_text_value){
    cxml_text_node *text = cxml_create_node(CXML_TEXT_NODE);
    CHECK_NOT_NULL(text);
    CHECK_TRUE(cxml_set_text_value(text, "this is some text", false));
    CHECK_TRUE(cxml_set_text_value(text, "this is some <![CDATA[stuffy]]>", true));
    CHECK_TRUE(cxml_string_raw_equals(&text->value, "this is some <![CDATA[stuffy]]>"));
    CHECK_TRUE(cxml_set_text_value(text, " 12356\r ", false));
    CHECK_EQ(text->number_value.dec_val, 12356);
    CHECK_FALSE(cxml_set_text_value(NULL, "this is some text", false));
    CHECK_FALSE(cxml_set_text_value(text, NULL, false));
    cxml_destroy(text);
}

TEST(cxqapi, cxml_set_comment_value){
    cxml_comment_node *comment = cxml_create_node(CXML_COMM_NODE);
    CHECK_NOT_NULL(comment);
    CHECK_TRUE(cxml_set_comment_value(comment, "this is some text"));
    CHECK_FALSE(cxml_set_comment_value(NULL, "this is some text"));
    CHECK_FALSE(cxml_set_comment_value(comment, NULL));
    cxml_destroy(comment);
}

TEST(cxqapi, cxml_set_pi_target){
    cxml_pi_node *pi = cxml_create_node(CXML_PI_NODE);
    CHECK_NOT_NULL(pi);
    CHECK_TRUE(cxml_set_pi_target(pi, "abc"));
    CHECK_TRUE(cxml_string_lraw_equals(&pi->target, "abc", 3));

    CHECK_FALSE(cxml_set_pi_target(pi, ""));
    CHECK_FALSE(cxml_set_pi_target(pi, NULL));
    CHECK_FALSE(cxml_set_pi_target(NULL, "abc"));
    cxml_destroy(pi);
}

TEST(cxqapi, cxml_set_pi_value){
    cxml_pi_node *pi = cxml_create_node(CXML_PI_NODE);
    CHECK_NOT_NULL(pi);
    CHECK_TRUE(cxml_set_pi_value(pi, "abc"));
    CHECK_TRUE(cxml_string_lraw_equals(&pi->value, "abc", 3));

    CHECK_FALSE(cxml_set_pi_value(pi, NULL));
    CHECK_FALSE(cxml_set_pi_value(NULL, "abc"));
    cxml_destroy(pi);
}

TEST(cxqapi, cxml_set_pi_data){
    cxml_pi_node *pi = cxml_create_node(CXML_PI_NODE);
    CHECK_NOT_NULL(pi);
    CHECK_TRUE(cxml_set_pi_data(pi, "abc", "some value"));
    CHECK_TRUE(cxml_string_lraw_equals(&pi->target, "abc", 3));
    CHECK_TRUE(cxml_string_lraw_equals(&pi->value, "some value", 10));

    CHECK_FALSE(cxml_set_pi_data(pi, NULL, "some value"));
    CHECK_FALSE(cxml_set_pi_data(pi, "abc", NULL));
    CHECK_FALSE(cxml_set_pi_data(NULL, "abc", "some value"));

    cxml_destroy(pi);
}

TEST(cxqapi, cxml_set_namespace_prefix){
    cxml_namespace_node *ns = cxml_create_node(CXML_NS_NODE);
    CHECK_NOT_NULL(ns);
    CHECK_TRUE(cxml_set_namespace_prefix(ns, "abc"));
    CHECK_TRUE(cxml_string_lraw_equals(&ns->prefix, "abc", 3));

    CHECK_FALSE(cxml_set_namespace_prefix(ns, ""));
    CHECK_FALSE(cxml_set_namespace_prefix(ns, NULL));
    CHECK_FALSE(cxml_set_namespace_prefix(NULL, "abc"));
    cxml_destroy(ns);
}

TEST(cxqapi, cxml_set_namespace_uri){
    cxml_namespace_node *ns = cxml_create_node(CXML_NS_NODE);
    CHECK_NOT_NULL(ns);
    CHECK_TRUE(cxml_set_namespace_uri(ns, "abc"));
    CHECK_TRUE(cxml_string_lraw_equals(&ns->uri, "abc", 3));

    CHECK_FALSE(cxml_set_namespace_uri(ns, NULL));
    // reserved for xml
    CHECK_FALSE(cxml_set_namespace_uri(ns, "http://www.w3.org/XML/1998/namespace"));
    // reserved for xmlns
    CHECK_FALSE(cxml_set_namespace_uri(ns, "http://www.w3.org/2000/xmlns/"));
    CHECK_FALSE(cxml_set_namespace_uri(NULL, "abc"));
    cxml_destroy(ns);
}

TEST(cxqapi, cxml_set_namespace_data){
    cxml_namespace_node *ns = cxml_create_node(CXML_NS_NODE);
    CHECK_NOT_NULL(ns);
    CHECK_TRUE(cxml_set_namespace_data(ns, "abc", "some value"));
    CHECK_TRUE(cxml_string_lraw_equals(&ns->prefix, "abc", 3));
    CHECK_TRUE(cxml_string_lraw_equals(&ns->uri, "some value", 10));

    CHECK_FALSE(cxml_set_namespace_data(ns, NULL, "some value"));
    CHECK_FALSE(cxml_set_namespace_data(ns, "abc", NULL));
    // reserved for xml
    CHECK_FALSE(cxml_set_namespace_data(ns, "abc", "http://www.w3.org/XML/1998/namespace"));
    // reserved for xmlns
    CHECK_FALSE(cxml_set_namespace_data(ns, "abc", "http://www.w3.org/2000/xmlns/"));
    CHECK_FALSE(cxml_set_namespace_data(NULL, "abc", "some value"));

    cxml_destroy(ns);
}

TEST(cxqapi, cxml_set_root_node_name){
    cxml_root_node *root = cxml_create_node(CXML_ROOT_NODE);
    CHECK_NOT_NULL(root);
    CHECK_TRUE(cxml_set_root_node_name(root, "doc"));
    CHECK_TRUE(cxml_string_lraw_equals(&root->name, "doc", 3));

    CHECK_FALSE(cxml_set_root_node_name(root, NULL));
    CHECK_FALSE(cxml_set_root_node_name(NULL, "doc"));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_set_root_element){
    cxml_root_node *root = cxml_create_node(CXML_ROOT_NODE);
    CHECK_FALSE(root->has_child);
    CHECK_NULL(root->root_element);

    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    CHECK_TRUE(cxml_set_root_element(root, elem));
    CHECK_EQ(root->root_element, elem);
    CHECK_TRUE(root->has_child);

    cxml_destroy(root);
    root = cxml_load_string(wf_xml_7);
    elem = cxml_create_node(CXML_ELEM_NODE);
    // already has a root element
    CHECK_FALSE(cxml_set_root_element(root, elem));
    CHECK_NE(root->root_element, elem);

    cxml_destroy(root);
    // we must free `elem` because it's not bound to `root`.
    cxml_destroy(elem);
}


/*********************************
 *                               *
 * update methods/functions      *
 *********************************
 */

TEST(cxqapi, cxml_insert_before){
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    CHECK_NOT_NULL(root);
    // create an element to be inserted
    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    CHECK_NOT_NULL(elem);
    CHECK_FALSE(elem->has_parent);
    CHECK_TRUE(cxml_set_name(elem, "xml", "bar"));

    // find an element we want to insert `elem` before
    cxml_element_node *name = cxml_find(root, "<name>/");
    // insert `elem` before `name`
    CHECK_TRUE(cxml_insert_before(name, elem));
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    char *expected = "<fruit>\n"
                     "  <xml:bar/>\n"
                     "  <name>\n"
                     "    apple\n"
                     "  </name>\n"
                     "</fruit>";
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(elem->parent, root->root_element);
    CHECK_TRUE(elem->has_parent);

    CHECK_FALSE(cxml_insert_before(name, NULL));
    CHECK_FALSE(cxml_insert_before(NULL, elem));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_insert_after){
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    CHECK_NOT_NULL(root);
    // create a comment to be inserted
    cxml_comment_node *comment = cxml_create_node(CXML_COMM_NODE);
    CHECK_NOT_NULL(comment);
    CHECK_NULL(comment->parent);
    CHECK_TRUE(cxml_set_comment_value(comment, "this is a simple comment"));

    // find a place we want to insert `comment` after
    cxml_element_node *name = cxml_find(root, "<name>/");
    cxml_text_node *text = cxml_first_child(name);
    CHECK_FALSE(name->has_comment);
    // insert `comment` after `text`
    CHECK_TRUE(cxml_insert_after(text, comment));
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    char *expected = "<fruit>\n"
                     "  <name>\n"
                     "    apple\n"
                     "    <!--this is a simple comment-->\n"
                     "  </name>\n"
                     "</fruit>";
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(comment->parent, name);
    CHECK_NOT_NULL(comment->parent);
    CHECK_TRUE(name->has_comment);
    CHECK_EQ(cxml_list_size(&name->children), 2);

    CHECK_FALSE(cxml_insert_after(name, NULL));
    CHECK_FALSE(cxml_insert_before(NULL, comment));

    cxml_destroy(root);
}


/*********************************
 *                               *
 * delete methods/functions      *
 *********************************
 */

int element_repr_asserts(cxml_root_node *root){
    char *expected = "<fruit>\n"
                     "  <name>\n"
                     "    apple\n"
                     "  </name>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 1);
    CHECK_TRUE(root->root_element->has_child);
    CHECK_FALSE(root->root_element->is_self_enclosing);
    FREE(got);
    return 1;
}

TEST(cxqapi, cxml_delete_element){
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    CHECK_NOT_NULL(root);
    CHECK_TRUE(element_repr_asserts(root));

    cxml_element_node *name = cxml_find(root, "<name>/");
    CHECK_TRUE(cxml_delete_element(name));
    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_NULL(cxml_list_size(&root->root_element->children));
    CHECK_FALSE(root->root_element->has_child);
    CHECK_TRUE(root->root_element->is_self_enclosing);
    cxml_destroy(root);
    FREE(got);
    got = NULL;

    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    root = cxml_load_string(wf_xml_7);
    cxml_element_node *color = cxml_find(root, "<color>/");
    cxml_element_node *shape = cxml_find(root, "<shape>/");
    CHECK_NOT_NULL(color);
    CHECK_NOT_NULL(shape);
    CHECK_EQ(cxml_list_size(&root->root_element->children), 3);

    CHECK_TRUE(cxml_delete_element(color));
    CHECK_TRUE(cxml_delete_element(shape));
    expected = "<fruit>\n"
               "  <name>\n"
               "    apple\n"
               "  </name>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 1);
    FREE(got);

    CHECK_FALSE(cxml_delete_element(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_drop_element){
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    CHECK_NOT_NULL(root);
    CHECK_TRUE(element_repr_asserts(root));

    cxml_element_node *name = cxml_find(root, "<name>/");
    CHECK_TRUE(cxml_drop_element(name));
    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_NULL(cxml_list_size(&root->root_element->children));
    CHECK_FALSE(root->root_element->has_child);
    CHECK_TRUE(root->root_element->is_self_enclosing);
    cxml_destroy(root);
    // `name` is no longer a part of `root` since we dropped it,
    // so its our responsibility to free it.
    cxml_destroy(name);
    FREE(got);
    got = NULL;

    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    root = cxml_load_string(wf_xml_7);
    cxml_element_node *color = cxml_find(root, "<color>/");
    cxml_element_node *shape = cxml_find(root, "<shape>/");
    CHECK_NOT_NULL(color);
    CHECK_NOT_NULL(shape);
    CHECK_EQ(cxml_list_size(&root->root_element->children), 3);

    CHECK_TRUE(cxml_drop_element(color));
    CHECK_TRUE(cxml_drop_element(shape));
    expected = "<fruit>\n"
               "  <name>\n"
               "    apple\n"
               "  </name>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 1);
    FREE(got);

    CHECK_FALSE(cxml_drop_element(NULL));
    cxml_destroy(root);
    // `color` & `shape` is no longer a part of `root` since we dropped it,
    // so its our responsibility to free it.
    cxml_destroy(color);
    cxml_destroy(shape);
}

TEST(cxqapi, cxml_drop_element_by_query){
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    CHECK_NOT_NULL(root);
    CHECK_TRUE(element_repr_asserts(root));

    cxml_element_node *name = cxml_drop_element_by_query(root, "<name>/");
    CHECK_NOT_NULL(name);

    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_NULL(cxml_list_size(&root->root_element->children));
    CHECK_FALSE(root->root_element->has_child);
    CHECK_TRUE(root->root_element->is_self_enclosing);
    cxml_destroy(root);
    // `name` is no longer a part of `root` since we dropped it,
    // so its our responsibility to free it.
    cxml_destroy(name);
    FREE(got);
    got = NULL;

    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    root = cxml_load_string(wf_xml_7);
    CHECK_EQ(cxml_list_size(&root->root_element->children), 3);

    cxml_element_node *color = cxml_drop_element_by_query(root, "<color>/");
    cxml_element_node *shape = cxml_drop_element_by_query(root, "<shape>/");
    CHECK_NOT_NULL(color);
    CHECK_NOT_NULL(shape);
    expected = "<fruit>\n"
               "  <name>\n"
               "    apple\n"
               "  </name>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 1);
    FREE(got);

    CHECK_FALSE(cxml_drop_element_by_query(root, "<xyz>/"));
    CHECK_FALSE(cxml_drop_element_by_query(NULL, "<xyz>/"));
    CHECK_FALSE(cxml_drop_element_by_query(root, NULL));
    cxml_destroy(root);
    // `color` & `shape` is no longer a part of `root` since we dropped it,
    // so its our responsibility to free it.
    cxml_destroy(color);
    cxml_destroy(shape);
}

TEST(cxqapi, cxml_delete_elements){
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    CHECK_NOT_NULL(root);
    CHECK_TRUE(element_repr_asserts(root));

    // delete all elements in the root element
    CHECK(cxml_delete_elements(root->root_element));

    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_NULL(cxml_list_size(&root->root_element->children));
    CHECK_FALSE(root->root_element->has_child);
    CHECK_TRUE(root->root_element->is_self_enclosing);
    cxml_destroy(root);
    FREE(got);
    got = NULL;

    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    root = cxml_load_string(wf_xml_7);
    CHECK_EQ(cxml_list_size(&root->root_element->children), 3);

    cxml_element_node *color = cxml_find(root, "<color>/");
    cxml_element_node *shape = cxml_find(root, "<shape>/");
    CHECK_NOT_NULL(color);
    CHECK_NOT_NULL(shape);
    // `br` gets deleted
    CHECK(cxml_delete_elements(color));
    // there are no elements in `shape`
    CHECK_FALSE(cxml_delete_elements(shape));
    expected = "<fruit>\n"
               "  <name>\n"
               "    apple\n"
               "  </name>\n"
               "  <color>\n"
               "    red\n"
               "    blue\n"
               "  </color>\n"
               "  <shape>\n"
               "    roundish\n"
               "  </shape>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 3);
    FREE(got);

    CHECK_FALSE(cxml_delete_elements(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_delete_elements_by_query){
    cxml_root_node *root = cxml_load_string(wf_xml_10);
    CHECK_NOT_NULL(root);
    // "<fruit><name>apple</name><name>banana</name></fruit>"
    char *got = cxml_element_to_rstring(root->root_element);
    char *expected = "<fruit>\n"
                     "  <name>\n"
                     "    apple\n"
                     "  </name>\n"
                     "  <name>\n"
                     "    banana\n"
                     "  </name>\n"
                     "</fruit>";
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 2);
    CHECK_TRUE(root->root_element->has_child);
    CHECK_FALSE(root->root_element->is_self_enclosing);
    FREE(got);
    got = NULL;

    CHECK_TRUE(cxml_delete_elements_by_query(root, "<name>/"));

    expected = "<fruit/>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_NULL(cxml_list_size(&root->root_element->children));
    CHECK_FALSE(root->root_element->has_child);
    CHECK_TRUE(root->root_element->is_self_enclosing);
    cxml_destroy(root);
    FREE(got);
    got = NULL;

    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    root = cxml_load_string(wf_xml_7);
    CHECK_EQ(cxml_list_size(&root->root_element->children), 3);

    CHECK_TRUE(cxml_delete_elements_by_query(root, "<color>/"));
    CHECK_TRUE(cxml_delete_elements_by_query(root, "<shape>/"));

    expected = "<fruit>\n"
               "  <name>\n"
               "    apple\n"
               "  </name>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 1);
    FREE(got);

    CHECK_FALSE(cxml_delete_elements_by_query(root, "<shape>/"));
    CHECK_FALSE(cxml_delete_elements_by_query(root, NULL));
    CHECK_FALSE(cxml_delete_elements_by_query(NULL, "<shape>/"));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_drop_elements){
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    CHECK_NOT_NULL(root);
    CHECK_TRUE(element_repr_asserts(root));

    cxml_list list = new_cxml_list();
    // drop all elements in the root element
    CHECK_TRUE(cxml_drop_elements(root->root_element, &list));
    CHECK_EQ(cxml_list_size(&list), 1);

    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_NULL(cxml_list_size(&root->root_element->children));
    CHECK_FALSE(root->root_element->has_child);
    CHECK_TRUE(root->root_element->is_self_enclosing);
    cxml_destroy(root);
    FREE(got);
    cxml_list_free(&list);

    CHECK_FALSE(cxml_drop_elements(NULL, &list));
    CHECK_FALSE(cxml_drop_elements(root, NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_drop_elements_by_query){
    // "<fruit><name>apple</name><name>banana</name></fruit>"
    cxml_root_node *root = cxml_load_string(wf_xml_10);
    CHECK_NOT_NULL(root);

    cxml_list list = new_cxml_list();
    // drop all `name` elements in the root element
    CHECK_TRUE(cxml_drop_elements_by_query(root->root_element, "<name>/", &list));
    CHECK_EQ(cxml_list_size(&list), 2);

    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_NULL(cxml_list_size(&root->root_element->children));
    CHECK_FALSE(root->root_element->has_child);
    CHECK_TRUE(root->root_element->is_self_enclosing);
    cxml_destroy(root);
    FREE(got);
    cxml_list_free(&list);

    CHECK_FALSE(cxml_drop_elements_by_query(root->root_element, NULL, &list));
    CHECK_FALSE(cxml_drop_elements_by_query(root->root_element, "<name>/", NULL));
    CHECK_FALSE(cxml_drop_elements_by_query(NULL, "<name>/", &list));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_delete_comment){
    // <fruit><!--some comment--><name>apple</name><!--another comment--></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_11);
    CHECK_NOT_NULL(root);

    cxml_comment_node *comm = cxml_first_child(root->root_element);
    CHECK_NOT_NULL(comm);
    CHECK_TRUE(cxml_delete_comment(comm));
    char *expected = "<fruit>\n"
                     "  <name>\n"
                     "    apple\n"
                     "  </name>\n"
                     "  <!--another comment-->\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 2);
    FREE(got);

    CHECK_FALSE(cxml_delete_comment(NULL));
    cxml_destroy(root);

}

TEST(cxqapi, cxml_drop_comment){
    // <fruit><!--some comment--><name>apple</name><!--another comment--></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_11);
    CHECK_NOT_NULL(root);

    cxml_comment_node *comm = cxml_first_child(root->root_element);
    CHECK_NOT_NULL(comm);
    CHECK_TRUE(cxml_drop_comment(comm));
    char *expected = "<fruit>\n"
                     "  <name>\n"
                     "    apple\n"
                     "  </name>\n"
                     "  <!--another comment-->\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 2);
    FREE(got);

    CHECK_FALSE(cxml_drop_comment(NULL));
    cxml_destroy(root);
    // `comm` is no longer a part of `root` since we dropped it,
    // so its our responsibility to free it.
    cxml_destroy(comm);
}

TEST(cxqapi, cxml_delete_text){
    // <fruit><name>apple</name></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    CHECK_NOT_NULL(root);

    cxml_element_node *name = cxml_find(root, "<name>/");
    CHECK_NOT_NULL(name);
    CHECK_EQ(cxml_list_size(&name->children), 1);
    CHECK_FALSE(name->is_self_enclosing);
    CHECK_TRUE(name->has_text);

    CHECK_TRUE(cxml_delete_text(cxml_first_child(name)));

    CHECK_NULL(cxml_list_size(&name->children));
    CHECK_TRUE(name->is_self_enclosing);
    CHECK_FALSE(name->has_text);
    CHECK_FALSE(name->has_child);
    char *expected = "<fruit>\n"
                     "  <name/>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 1);
    FREE(got);

    CHECK_FALSE(cxml_drop_text(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_drop_text){
    // <fruit><name>apple</name></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    CHECK_NOT_NULL(root);

    cxml_element_node *name = cxml_find(root, "<name>/");
    CHECK_NOT_NULL(name);
    CHECK_EQ(cxml_list_size(&name->children), 1);
    CHECK_FALSE(name->is_self_enclosing);
    CHECK_TRUE(name->has_text);

    cxml_text_node *text = cxml_first_child(name);
    CHECK_TRUE(cxml_drop_text(text));

    CHECK_NULL(cxml_list_size(&name->children));
    CHECK_TRUE(name->is_self_enclosing);
    CHECK_FALSE(name->has_text);
    CHECK_FALSE(name->has_child);
    char *expected = "<fruit>\n"
                     "  <name/>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 1);
    CHECK_TRUE(root->root_element->has_child);
    FREE(got);

    CHECK_FALSE(cxml_drop_text(NULL));
    cxml_destroy(root);
    // `text` is no longer a part of `root` since we dropped it,
    // so its our responsibility to free it.
    cxml_destroy(text);
}

TEST(cxqapi, cxml_delete_attribute){
    // <x:fruit one="1" two="2" xmlns:x="uri"></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_12);
    CHECK_NOT_NULL(root);
    CHECK_EQ(cxml_table_size(root->root_element->attributes), 2);


    cxml_attribute_node *attr = cxml_get_attribute(root->root_element, "one");
    CHECK_TRUE(cxml_delete_attribute(attr));
    char *expected = "<x:fruit two=\"2\" xmlns:x=\"uri\"/>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_table_size(root->root_element->attributes), 1);
    FREE(got);

    CHECK_FALSE(cxml_delete_attribute(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_drop_attribute){
    // <x:fruit one="1" two="2" xmlns:x="uri"></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_12);
    CHECK_NOT_NULL(root);
    CHECK_EQ(cxml_table_size(root->root_element->attributes), 2);

    cxml_attribute_node *attr = cxml_get_attribute(root->root_element, "two");
    CHECK_TRUE(cxml_drop_attribute(attr));
    char *expected = "<x:fruit one=\"1\" xmlns:x=\"uri\"/>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_table_size(root->root_element->attributes), 1);
    FREE(got);

    CHECK_FALSE(cxml_drop_attribute(NULL));
    cxml_destroy(root);
}


TEST(cxqapi, cxml_unbound_element){
    // <x:fruit one="1" two="2" xmlns:x="uri"></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_12);
    CHECK_NOT_NULL(root);
    CHECK_EQ(cxml_list_size(root->root_element->namespaces), 1);
    CHECK_TRUE(root->root_element->is_namespaced);

    CHECK_TRUE(cxml_unbind_element(root->root_element));
    CHECK_FALSE(root->root_element->is_namespaced);
    CHECK_NULL(root->root_element->namespace);
    char *expected = "<fruit one=\"1\" two=\"2\" xmlns:x=\"uri\"/>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(root->root_element->namespaces)) // still exists, but unboun, 1d;
    CHECK_FALSE(root->root_element->is_namespaced);
    FREE(got);

    CHECK_FALSE(cxml_unbind_element(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_unbound_attribute){
    // "<a xmlns:x=\"uri\"><x:fruit one=\"1\" two=\"2\" x:three=\"3\" four=\"4\"></x:fruit></a>"
    cxml_root_node *root = cxml_load_string(wf_xml_13);
    CHECK_NOT_NULL(root);
    CHECK_EQ(cxml_list_size(root->root_element->namespaces), 1);

    cxml_attribute_node *attr = cxml_get_attribute(cxml_first_child(root->root_element), "x:three");
    CHECK_NOT_NULL(attr);
    CHECK_NOT_NULL(attr->namespace);
    CHECK_TRUE(cxml_unbind_attribute(attr));
    CHECK_NULL(attr->namespace);

    char *expected = "<a xmlns:x=\"uri\">\n  <x:fruit one=\"1\" two=\"2\" three=\"3\" four=\"4\"/>\n</a>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(root->root_element->namespaces), 1);
    CHECK_NULL(attr->namespace);
    FREE(got);

    CHECK_FALSE(cxml_unbind_attribute(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_delete_pi){
    // <fruit><?pi data?><class>basic<?pi data?></class><?pi data2?></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_14);
    CHECK_NOT_NULL(root);

    CHECK_EQ(cxml_list_size(&root->root_element->children), 3);
    cxml_pi_node *pi = cxml_first_child(root->root_element);
    CHECK_NOT_NULL(pi);

    CHECK_TRUE(cxml_delete_pi(pi));

    char *expected = "<fruit>\n"
                     "  <class>\n"
                     "    basic\n"
                     "    <?pi data?>\n"
                     "  </class>\n"
                     "  <?pi data2?>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 2);
    FREE(got);

    CHECK_FALSE(cxml_delete_pi(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_drop_pi){
    // <fruit><?pi data?><class>basic<?pi data?></class><?pi data2?></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_14);
    CHECK_NOT_NULL(root);

    CHECK_EQ(cxml_list_size(&root->root_element->children), 3);
    cxml_pi_node *pi = cxml_first_child(root->root_element);
    CHECK_NOT_NULL(pi);

    CHECK_TRUE(cxml_drop_pi(pi));

    char *expected = "<fruit>\n"
                     "  <class>\n"
                     "    basic\n"
                     "    <?pi data?>\n"
                     "  </class>\n"
                     "  <?pi data2?>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 2);
    FREE(got);

    CHECK_FALSE(cxml_drop_pi(NULL));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_delete_dtd){
    // <!DOCTYPE people_list SYSTEM "example.dtd"><start>testing</start>
    cxml_root_node *root = cxml_load_string(wf_xml_dtd);
    CHECK_NOT_NULL(root);
    CHECK_EQ(cxml_list_size(&root->children), 2);

    cxml_dtd_node *dtd = cxml_get_dtd_node(root);
    CHECK_NOT_NULL(dtd);

    CHECK_TRUE(cxml_delete_dtd(dtd));

    CHECK_EQ(cxml_list_size(&root->children), 1);

    char *expected = "<XMLDocument>\n"
                     "  <start>\n"
                     "    testing\n"
                     "  </start>\n"
                     "</XMLDocument>";
    char *got = cxml_document_to_rstring(root);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 1);
    FREE(got);

    CHECK_FALSE(cxml_delete_dtd(NULL));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_drop_dtd){
    // <!DOCTYPE people_list SYSTEM "example.dtd"><start>testing</start>
    cxml_root_node *root = cxml_load_string(wf_xml_dtd);
    CHECK_NOT_NULL(root);
    CHECK_EQ(cxml_list_size(&root->children), 2);

    cxml_dtd_node *dtd = cxml_get_dtd_node(root);
    CHECK_NOT_NULL(dtd);

    CHECK_TRUE(cxml_drop_dtd(dtd));

    CHECK_EQ(cxml_list_size(&root->children), 1);

    char *expected = "<XMLDocument>\n"
                     "  <start>\n"
                     "    testing\n"
                     "  </start>\n"
                     "</XMLDocument>";
    char *got = cxml_document_to_rstring(root);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 1);
    FREE(got);

    CHECK_FALSE(cxml_drop_dtd(NULL));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_delete_xml_hdr){
    // "<?xml version=\"1.0\"?><start>testing</start>"
    cxml_root_node *root = cxml_load_string(wf_xml_xhdr);
    CHECK_NOT_NULL(root);
    CHECK_EQ(cxml_list_size(&root->children), 2);

    cxml_xhdr_node *xhdr = cxml_get_xml_hdr_node(root);
    CHECK_NOT_NULL(xhdr);

    CHECK_TRUE(cxml_delete_xml_hdr(xhdr));

    CHECK_EQ(cxml_list_size(&root->children), 1);

    char *expected = "<XMLDocument>\n"
                     "  <start>\n"
                     "    testing\n"
                     "  </start>\n"
                     "</XMLDocument>";
    char *got = cxml_document_to_rstring(root);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 1);
    FREE(got);

    CHECK_FALSE(cxml_delete_xml_hdr(NULL));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_drop_xml_hdr){
    // "<?xml version=\"1.0\"?><start>testing</start>"
    cxml_root_node *root = cxml_load_string(wf_xml_xhdr);
    CHECK_NOT_NULL(root);
    CHECK_EQ(cxml_list_size(&root->children), 2);

    cxml_xhdr_node *xhdr = cxml_get_xml_hdr_node(root);
    CHECK_NOT_NULL(xhdr);

    CHECK_TRUE(cxml_drop_xml_hdr(xhdr));

    CHECK_EQ(cxml_list_size(&root->children), 1);

    char *expected = "<XMLDocument>\n"
                     "  <start>\n"
                     "    testing\n"
                     "  </start>\n"
                     "</XMLDocument>";
    char *got = cxml_document_to_rstring(root);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 1);
    FREE(got);

    CHECK_FALSE(cxml_drop_xml_hdr(NULL));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_delete_descendants){
    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    cxml_root_node * root = cxml_load_string(wf_xml_7);
    CHECK_NOT_NULL(root);

    CHECK(cxml_delete_descendants(root->root_element));
    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_NULL(cxml_list_size(&root->root_element->children));
    CHECK_FALSE(root->root_element->has_child);
    CHECK_TRUE(root->root_element->is_self_enclosing);
    FREE(got);

    CHECK_FALSE(cxml_delete_descendants(false));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_drop_descendants){
    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    cxml_root_node * root = cxml_load_string(wf_xml_7);
    CHECK_NOT_NULL(root);
    CHECK_EQ(cxml_list_size(&root->root_element->children), 3);

    cxml_list desc = new_cxml_list();
    CHECK_TRUE(cxml_drop_descendants(root->root_element, &desc));
    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_NULL(cxml_list_size(&root->root_element->children));
    CHECK_FALSE(root->root_element->has_child);
    CHECK_TRUE(root->root_element->is_self_enclosing);
    CHECK_EQ(cxml_list_size(&desc), 3);
    cxml_list_free(&desc);
    FREE(got);

    CHECK_FALSE(cxml_drop_descendants(root->root_element, NULL));
    CHECK_FALSE(cxml_drop_descendants(NULL, &desc));

    cxml_destroy(root);
}

TEST(cxqapi, cxml_delete_parent){
    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    CHECK_NOT_NULL(root);
    CHECK_EQ(cxml_list_size(&root->children), 1);

    cxml_element_node *elem = cxml_find(root, "<shape>/");
    CHECK_NOT_NULL(elem);

    // the parent of the element `shape` is the root element.
    // Deleting this means the root node no longer has any child, since that
    // was its only child.
    CHECK_TRUE(cxml_delete_parent(elem));

    char *expected = "<XMLDocument/>";
    char *got = cxml_document_to_rstring(root);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_NULL(cxml_list_size(&root->children));
    CHECK_FALSE(root->has_child);
    FREE(got);
    cxml_destroy(root);

    CHECK_FALSE(cxml_delete_parent(NULL));

    root = cxml_load_string(wf_xml_7);
    CHECK_EQ(cxml_list_size(&root->root_element->children), 3);
    elem = cxml_find(root, "<br>/");
    CHECK(cxml_delete_parent(elem));
    expected = "<fruit>\n"
               "  <name>\n"
               "    apple\n"
               "  </name>\n"
               "  <shape>\n"
               "    roundish\n"
               "  </shape>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&root->root_element->children), 2);
    FREE(got);

    cxml_destroy(root);
}

TEST(cxqapi, cxml_delete_comments){
    // "<start><!--first comment--><begin><abc><!--maybe-->
    //  </abc><xyz>foo<!--sometimes-->bar</xyz></begin><!--first comment-->
    //  </start>"
    cxml_root_node *root = cxml_load_string(wf_xml_15);
    CHECK_NOT_NULL(root);

    int comment_count = 0;
    cxml_for_each(node, &root->root_element->children){
        if (cxml_get_node_type(node) == CXML_COMM_NODE) comment_count++;
    }
    // root element has two comments
    CHECK_EQ(comment_count, 2);
    CHECK_TRUE(root->root_element->has_comment);
    // non-recursive delete
    CHECK(cxml_delete_comments(root->root_element, false));
    CHECK_FALSE(root->root_element->has_comment);
    char *expected =  "<start>\n"
                      "  <begin>\n"
                      "    <abc>\n"
                      "      <!--maybe-->\n"
                      "    </abc>\n"
                      "    <xyz>\n"
                      "      foo\n"
                      "      <!--sometimes-->\n"
                      "      bar\n"
                      "    </xyz>\n"
                      "  </begin>\n"
                      "</start>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    FREE(got);
    got = NULL;
    cxml_destroy(root);

    root = cxml_load_string(wf_xml_15);
    // recursive delete
    CHECK(cxml_delete_comments(root, true));
    expected =  "<start>\n"
                "  <begin>\n"
                "    <abc/>\n"
                "    <xyz>\n"
                "      foo\n"
                "      bar\n"
                "    </xyz>\n"
                "  </begin>\n"
                "</start>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_FALSE(root->root_element->has_comment);
    FREE(got);

    CHECK_FALSE(cxml_delete_comments(root, true));
    CHECK_FALSE(cxml_delete_comments(NULL, true));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_drop_comments){
    // "<start><!--first comment--><begin><abc><!--maybe-->
    //  </abc><xyz>foo<!--sometimes-->bar</xyz></begin><!--first comment-->
    //  </start>"
    cxml_root_node *root = cxml_load_string(wf_xml_15);
    CHECK_NOT_NULL(root);

    int comment_count = 0;
    cxml_for_each(node, &root->root_element->children){
        if (cxml_get_node_type(node) == CXML_COMM_NODE) comment_count++;
    }
    // root element has two comments
    CHECK_EQ(comment_count, 2);
    CHECK_TRUE(root->root_element->has_comment);

    cxml_list comments = new_cxml_list();
    // non-recursive delete
    CHECK(cxml_drop_comments(root->root_element, false, &comments));
    CHECK_FALSE(root->root_element->has_comment);
    CHECK_EQ(cxml_list_size(&comments), 2);
    char *expected =  "<start>\n"
                      "  <begin>\n"
                      "    <abc>\n"
                      "      <!--maybe-->\n"
                      "    </abc>\n"
                      "    <xyz>\n"
                      "      foo\n"
                      "      <!--sometimes-->\n"
                      "      bar\n"
                      "    </xyz>\n"
                      "  </begin>\n"
                      "</start>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    FREE(got);
    got = NULL;
    cxml_list_free(&comments);
    cxml_destroy(root);

    root = cxml_load_string(wf_xml_15);
    // recursive delete
    CHECK(cxml_drop_comments(root, true, &comments));
    expected =  "<start>\n"
                "  <begin>\n"
                "    <abc/>\n"
                "    <xyz>\n"
                "      foo\n"
                "      bar\n"
                "    </xyz>\n"
                "  </begin>\n"
                "</start>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_FALSE(root->root_element->has_comment);
    CHECK_EQ(cxml_list_size(&comments), 4);
    cxml_list_free(&comments);
    FREE(got);

    CHECK_FALSE(cxml_drop_comments(root, true, &comments));
    CHECK_FALSE(cxml_drop_comments(root->root_element, true, NULL));
    CHECK_FALSE(cxml_drop_comments(NULL, true, &comments));
    CHECK_NULL(cxml_list_size(&comments));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_drop_texts){
    /*
     * "<thesaurus>"
     * "begin"
     * "<entry>"
     * "benign"
     * "<term>successful</term>"
     * "<synonym>"
     * "misspelling"
     * "<term>successful</term>"
     * "<relationship>misspelling of</relationship>"
     * "stuff"
     * "</synonym>"
     * "</entry>"
     * "end"
     * "</thesaurus>"
     */
    cxml_root_node *root = cxml_load_string(wf_xml_16);
    CHECK_NOT_NULL(root);

    int text_count = 0;
    cxml_for_each(node, &root->root_element->children){
        if (cxml_get_node_type(node) == CXML_TEXT_NODE) text_count++;
    }
    // root element has two comments
    CHECK_EQ(text_count, 2);
    CHECK_TRUE(root->root_element->has_text);
    cxml_list texts = new_cxml_list();

    // non-recursive delete
    CHECK(cxml_drop_texts(root->root_element, false, &texts));
    CHECK_FALSE(root->root_element->has_text);
    CHECK_EQ(cxml_list_size(&texts), 2);
    char *expected =  "<thesaurus>\n"
                      "  <entry>\n"
                      "    benign\n"
                      "    <term>\n"
                      "      successful\n"
                      "    </term>\n"
                      "    <synonym>\n"
                      "      misspelling\n"
                      "      <term>\n"
                      "        successful\n"
                      "      </term>\n"
                      "      <relationship>\n"
                      "        misspelling of\n"
                      "      </relationship>\n"
                      "      stuff\n"
                      "    </synonym>\n"
                      "  </entry>\n"
                      "</thesaurus>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    FREE(got);
    cxml_list_free(&texts);
    cxml_destroy(root);

    root = cxml_load_string(wf_xml_16);
    // recursive delete
    CHECK(cxml_drop_texts(root, true, &texts));
    expected =  "<thesaurus>\n"
                "  <entry>\n"
                "    <term/>\n"
                "    <synonym>\n"
                "      <term/>\n"
                "      <relationship/>\n"
                "    </synonym>\n"
                "  </entry>\n"
                "</thesaurus>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    CHECK_EQ(cxml_list_size(&texts), 8);
    cxml_list_free(&texts);
    FREE(got);

    CHECK_FALSE(cxml_drop_texts(root, true, &texts));
    CHECK_FALSE(cxml_drop_texts(root->root_element, true, NULL));
    CHECK_FALSE(cxml_drop_texts(NULL, true, &texts));
    CHECK_NULL(cxml_list_size(&texts));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_delete_texts){
    /*
     * "<thesaurus>"
     * "begin"
     * "<entry>"
     * "benign"
     * "<term>successful</term>"
     * "<synonym>"
     * "misspelling"
     * "<term>successful</term>"
     * "<relationship>misspelling of</relationship>"
     * "stuff"
     * "</synonym>"
     * "</entry>"
     * "end"
     * "</thesaurus>"
     */
    cxml_root_node *root = cxml_load_string(wf_xml_16);
    CHECK_NOT_NULL(root);

    int text_count = 0;
    cxml_for_each(node, &root->root_element->children){
        if (cxml_get_node_type(node) == CXML_TEXT_NODE) text_count++;
    }
    // root element has two comments
    CHECK_EQ(text_count, 2);
    CHECK_TRUE(root->root_element->has_text);

    // non-recursive delete
    CHECK(cxml_delete_texts(root->root_element, false));
    CHECK_FALSE(root->root_element->has_text);
    char *expected =  "<thesaurus>\n"
                      "  <entry>\n"
                      "    benign\n"
                      "    <term>\n"
                      "      successful\n"
                      "    </term>\n"
                      "    <synonym>\n"
                      "      misspelling\n"
                      "      <term>\n"
                      "        successful\n"
                      "      </term>\n"
                      "      <relationship>\n"
                      "        misspelling of\n"
                      "      </relationship>\n"
                      "      stuff\n"
                      "    </synonym>\n"
                      "  </entry>\n"
                      "</thesaurus>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    FREE(got);
    cxml_destroy(root);

    root = cxml_load_string(wf_xml_16);
    // recursive delete
    CHECK(cxml_delete_texts(root, true));
    expected =  "<thesaurus>\n"
                "  <entry>\n"
                "    <term/>\n"
                "    <synonym>\n"
                "      <term/>\n"
                "      <relationship/>\n"
                "    </synonym>\n"
                "  </entry>\n"
                "</thesaurus>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    FREE(got);

    CHECK_FALSE(cxml_delete_texts(root, true));
    CHECK_FALSE(cxml_delete_texts(root->root_element, true));
    CHECK_FALSE(cxml_delete_texts(NULL, true));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_delete_pis){
    // <fruit><?pi data?><class>basic<?pi data?></class><?pi data2?></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_14);
    CHECK_NOT_NULL(root);

    int pi_count = 0;
    cxml_for_each(node, &root->root_element->children){
        if (cxml_get_node_type(node) == CXML_PI_NODE) pi_count++;
    }
    // root element has two comments
    CHECK_EQ(pi_count, 2);
    // non-recursive delete
    CHECK(cxml_delete_pis(root->root_element, false));
    char *expected = "<fruit>\n"
                     "  <class>\n"
                     "    basic\n"
                     "    <?pi data?>\n"
                     "  </class>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    FREE(got);
    cxml_destroy(root);
    got = NULL;

    root = cxml_load_string(wf_xml_14);
    // recursive delete
    CHECK(cxml_delete_pis(root, true));
    expected = "<fruit>\n"
               "  <class>\n"
               "    basic\n"
               "  </class>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_NOT_NULL(got);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    FREE(got);

    CHECK_FALSE(cxml_delete_pis(root, true));
    CHECK_FALSE(cxml_delete_pis(NULL, true));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_drop_pis){
    // <fruit><?pi data?><class>basic<?pi data?></class><?pi data2?></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_14);
    CHECK_NOT_NULL(root);

    cxml_list pis = new_cxml_list();
    int pi_count = 0;
    cxml_for_each(node, &root->root_element->children){
        if (cxml_get_node_type(node) == CXML_PI_NODE) pi_count++;
    }
    // root element has two comments
    CHECK_EQ(pi_count, 2);
    // non-recursive delete
    CHECK(cxml_drop_pis(root->root_element, false, &pis));
    CHECK_EQ(cxml_list_size(&pis), 2);
    char *expected = "<fruit>\n"
                     "  <class>\n"
                     "    basic\n"
                     "    <?pi data?>\n"
                     "  </class>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    FREE(got);
    cxml_list_free(&pis);
    cxml_destroy(root);

    root = cxml_load_string(wf_xml_14);
    // recursive delete
    CHECK(cxml_drop_pis(root, true, &pis));
    CHECK_EQ(cxml_list_size(&pis), 3);
    expected = "<fruit>\n"
               "  <class>\n"
               "    basic\n"
               "  </class>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    FREE(got);
    cxml_list_free(&pis);

    CHECK_FALSE(cxml_drop_pis(root->root_element, false, &pis));
    CHECK_FALSE(cxml_drop_pis(root->root_element, false, NULL));
    CHECK_FALSE(cxml_drop_pis(NULL, false, &pis));
    CHECK_NULL(cxml_list_size(&pis));
    cxml_destroy(root);
}

TEST(cxqapi, cxml_delete_prolog){
    // <?xml version="1.0"?><!DOCTYPE people_list SYSTEM "example.dtd"><start>testing</start>
    cxml_root_node *root = cxml_load_string(wf_xml_plg);
    CHECK_NOT_NULL(root);
    CHECK_EQ(cxml_list_size(&root->children), 3);

    CHECK(cxml_delete_prolog(root));
    char *expected = "<start>\n"
                     "  testing\n"
                     "</start>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    // root element is the only child left
    CHECK_EQ(cxml_list_size(&root->children), 1);
    FREE(got);

    CHECK_FALSE(cxml_delete_prolog(NULL));
    cxml_destroy(root);

}

TEST(cxqapi, cxml_drop_prolog){
    // <?xml version="1.0"?><!DOCTYPE people_list SYSTEM "example.dtd"><start>testing</start>
    cxml_root_node *root = cxml_load_string(wf_xml_plg);
    CHECK_NOT_NULL(root);
    CHECK_EQ(cxml_list_size(&root->children), 3);

    cxml_list prolog = new_cxml_list();
    CHECK_TRUE(cxml_drop_prolog(root, &prolog));
    CHECK_EQ(cxml_list_size(&prolog), 2);
    char *expected = "<start>\n"
                     "  testing\n"
                     "</start>";
    char *got = cxml_element_to_rstring(root->root_element);
    CHECK_TRUE(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)));
    // root element is the only child left
    CHECK_EQ(cxml_list_size(&root->children), 1);
    cxml_list_free(&prolog);
    FREE(got);

    CHECK_FALSE(cxml_drop_prolog(root, &prolog));
    CHECK_FALSE(cxml_drop_prolog(root, NULL));
    CHECK_FALSE(cxml_drop_prolog(NULL, &prolog));
    cxml_destroy(root);

}

TEST(cxqapi, cxml_delete_document){
    cxml_root_node *root = cxml_load_string(wf_xml_plg);
    CHECK_NOT_NULL(root);
    CHECK_TRUE(cxml_delete_document(root));
    CHECK_FALSE(cxml_delete_document(NULL));
}

TEST(cxqapi, cxml_delete){
    cxml_root_node *root = cxml_load_string(wf_xml_plg);
    CHECK_NOT_NULL(root);
    CHECK_TRUE(cxml_delete(root));
    CHECK_FALSE(cxml_delete(NULL));
}

TEST(cxqapi, cxml_clear){
    cxml_root_node *root = cxml_load_string(wf_xml_plg);
    CHECK_NOT_NULL(root);
    // should not seg-fault.
    cxml_clear(root);
}

TEST(cxqapi, cxml_get_number){
    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE);
    CHECK_NOT_NULL(attr);
    CHECK_TRUE(cxml_set_attribute_data(attr, "xml", "food", "12345"));
    double n = cxml_get_number(attr);
    CHECK_EQ(n, 12345);

    cxml_text_node *text = cxml_create_node(CXML_TEXT_NODE);
    cxml_set_text_value(text, "0x100", false);
    n = cxml_get_number(text);
    CHECK_EQ(n, 0x100);

    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    CHECK_NULL(cxml_get_number(elem));
    CHECK_NULL(cxml_get_number(NULL));

    cxml_free_attribute_node(attr);
    cxml_free_text_node(text);
    cxml_free_element_node(elem);
}