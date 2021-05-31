/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

//#define CXML_T_NO_SUITE_TEST_SHUFFLE
#include "cxfixture.h"

/*
 * Test for the major functions exported by cxqapi.c
 *
 * [PS: One may find that some code segments are duplicated; this is intentional.]
 */


// General functions
cts test_cxml_is_well_formed(){
    deb()
    cxml_root_node *root = get_root("wf_xml_1.xml", false);
    cxml_assert__not_null(root)
    cxml_assert__true(cxml_is_well_formed(root))
    cxml_destroy(root);
    root = cxml_load_string(df_xml_1);
    cxml_assert__false(cxml_is_well_formed(root))
    cxml_assert__false(cxml_is_well_formed(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_get_node_type(){
    deb()
    cxml_root_node root;
    cxml_root_node_init(&root);
    cxml_assert__eq(cxml_get_node_type(&root), CXML_ROOT_NODE)
    cxml_elem_node elem;
    cxml_elem_node_init(&elem);
    cxml_assert__eq(cxml_get_node_type(&elem), CXML_ELEM_NODE)
    cxml_assert__eq(cxml_get_node_type(NULL), 0xff)
    cxml_pass()
}

cts test_cxml_get_dtd_node(){
    deb()
    cxml_root_node *root = get_root("wf_xml_5.xml", true);
    cxml_assert__not_null(root)
    cxml_dtd_node *node = cxml_get_dtd_node(root);
    cxml_assert__not_null(node)
    cxml_assert__eq(cxml_get_node_type(node), CXML_DTD_NODE)
    cxml_destroy(root);

    root = get_root("wf_xml_1.xml", true);
    cxml_assert__null(cxml_get_dtd_node(root))
    cxml_assert__null(cxml_get_dtd_node(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_get_xml_hdr_node(){
    deb()
    cxml_root_node *root = get_root("wf_xml_1.xml", false);

    cxml_assert__not_null(root)
    cxml_xhdr_node *node = cxml_get_xml_hdr_node(root);
    cxml_assert__not_null(node)
    cxml_assert__eq(cxml_get_node_type(node), CXML_XHDR_NODE)
    cxml_destroy(root);

    root = get_root("wf_xml_5.xml", true);

    cxml_assert__null(cxml_get_xml_hdr_node(root))
    cxml_assert__null(cxml_get_xml_hdr_node(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_get_root_element(){
    deb()
    cxml_root_node *root = get_root("wf_xml_2.xml", false);
    cxml_assert__not_null(root)
    cxml_elem_node *elem = cxml_get_root_element(root);
    cxml_assert__not_null(elem);
    cxml_assert__null(cxml_get_root_element(NULL))
    cxml_destroy(root);
    cxml_pass()
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
        cxml_assert__zero(strncmp(name->pname, pname, name->pname_len))
    }
    if (lname){
        cxml_assert__zero(strncmp(name->lname, lname, name->lname_len))
    }
    if (qname){
        cxml_assert__true(cxml_string_raw_equals(&name->qname, qname))
    }
    return 1;
}

cts test_cxml_find(){
    deb()
    cxml_root_node *root = get_root("wf_xml_1.xml", true);
    cxml_assert__not_null(root)
    cxml_elem_node *elem = cxml_find(root, "<div>/");
    cxml_assert__not_null(elem)
    cxml_assert__one(name_asserts(&elem->name, NULL, "div", "div"))
    cxml_elem_node *elem2 = cxml_find(root, "<div>/@mod/[@section]/");
    cxml_assert__eq(elem, elem2)
    cxml_assert__one(name_asserts(&elem2->name, NULL, "div", "div"))
    cxml_free_root_node(root);
    cxml_pass()
}

cts test_cxml_find_all(){
    deb()
    cxml_root_node *root = get_root("wf_xml_2.xml", true);
    cxml_assert__not_null(root)
    cxml_list list = new_cxml_list();
    cxml_find_all(root, "<term>/$text/", &list);
    cxml_assert__eq(cxml_list_size(&list), 4)
    cxml_for_each(elem, &list)
    {
        cxml_assert__one(name_asserts(&(_unwrap__cxnode(elem, elem))->name, NULL, "term", "term"))
        cxml_assert__true(_unwrap__cxnode(elem, elem)->has_text)
    }
    cxml_list_free(&list);

    cxml_find_all(root, "<term>/", NULL);
    cxml_assert__zero(cxml_list_size(&list))
    cxml_find_all(root, NULL, &list);
    cxml_assert__zero(cxml_list_size(&list))
    cxml_find_all(NULL, "<term>/$text/", &list);
    cxml_assert__zero(cxml_list_size(&list))
    cxml_find_all(root, "<term>/#comment='foobar'/", &list);
    cxml_assert__zero(cxml_list_size(&list))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_find_children(){
    deb()
    cxml_root_node *root = get_root("wf_xml_2.xml", true);
    cxml_assert__not_null(root)

    cxml_list list = new_cxml_list();
    cxml_find_children(root, "<synonym>/", &list);
    cxml_assert__geq(cxml_list_size(&list), 2)
    cxml_list_free(&list);

    cxml_elem_node *elem = root->root_element;
    cxml_find_children(elem, "<relationship>/", &list);
    cxml_assert__one(cxml_list_size(&list))
    cxml_list_free(&list);

    cxml_find_children(elem, NULL, &list);
    cxml_assert__zero(cxml_list_size(&list))

    cxml_find_children(NULL, "<relationship>/", &list);
    cxml_assert__zero(cxml_list_size(&list))

    cxml_find_children(elem, "<relationship>/", NULL);
    cxml_assert__zero(cxml_list_size(&list))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_children(){
    deb()
    cxml_root_node *root = get_root("wf_xml_4.xml", false);
    cxml_assert__not_null(root)

    cxml_list list = new_cxml_list();
    cxml_children(root, &list);
    cxml_assert__one(cxml_list_size(&list))
    cxml_list_free(&list);

    cxml_elem_node *elem = root->root_element;
    cxml_children(elem, &list);
    cxml_assert__geq(cxml_list_size(&list), 3)
    cxml_list_free(&list);

    cxml_children(elem, NULL);
    cxml_assert__zero(cxml_list_size(&list))

    cxml_children(NULL, &list);
    cxml_assert__zero(cxml_list_size(&list))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_next_element(){
    deb()
    cxml_root_node *root = get_root("wf_xml_4.xml", false);
    cxml_assert__not_null(root)
    cxml_element_node *elem = cxml_find(root, "<f:width>/");
    cxml_assert__not_null(elem)
    cxml_assert__one(name_asserts(&elem->name, "f", "width", "f:width"))
    cxml_assert__not_null(elem->namespace)
    cxml_assert__true(cxml_string_raw_equals(&elem->namespace->prefix, "f"))

    cxml_element_node *next = cxml_next_element(elem);
    cxml_assert__not_null(next)
    cxml_assert__one(name_asserts(&next->name, "f", "length", "f:length"))
    cxml_assert__not_null(next->namespace)
    cxml_assert__true(cxml_string_raw_equals(&next->namespace->prefix, "f"))

    cxml_assert__null(cxml_next_element(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_previous_element(){
    deb()
    cxml_root_node *root = get_root("wf_xml_4.xml", true);
    cxml_assert__not_null(root)
    cxml_element_node *elem = cxml_find(root, "<f:width>/");
    cxml_assert__not_null(elem)
    cxml_assert__one(name_asserts(&elem->name, "f", "width", "f:width"))
    cxml_assert__not_null(elem->namespace)
    cxml_assert__true(cxml_string_raw_equals(&elem->namespace->prefix, "f"))

    cxml_element_node *previous = cxml_previous_element(elem);
    cxml_assert__not_null(previous)
    cxml_assert__one(name_asserts(&previous->name, "f", "name", "f:name"))
    cxml_assert__not_null(previous->namespace)
    cxml_assert__true(cxml_string_raw_equals(&previous->namespace->prefix, "f"))

    cxml_assert__null(cxml_previous_element(NULL))
    cxml_destroy(root);
    cxml_pass()
}

// cimple
cts test_cxml_next_comment(){
    deb()
    cxml_root_node *root = get_root("wf_xml_3.xml", true);
    cxml_assert__not_null(root)
    cxml_element_node *elem = cxml_find(root, "<entry>/");
    cxml_assert__not_null(elem)
    cxml_assert__one(name_asserts(&elem->name, NULL, "entry", "entry"))
    cxml_comment_node *comm = NULL;
    cxml_for_each(n, &elem->children)
    {
        if (_cxml_node_type(n) == CXML_COMM_NODE){
            comm = n;
            break;
        }
    }
    cxml_assert__true(cxml_string_raw_equals(&comm->value, "foobar"))

    cxml_comment_node *next = cxml_next_comment(comm);
    cxml_assert__not_null(next)
    cxml_assert__true(cxml_string_raw_equals(&next->value, "cimple"))

    cxml_assert__null(cxml_next_comment(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_previous_comment(){
    deb()
    cxml_root_node *root = get_root("wf_xml_3.xml", false);
    cxml_assert__not_null(root)
    cxml_element_node *elem = cxml_find(root, "<entry>/");
    cxml_assert__not_null(elem)
    cxml_assert__one(name_asserts(&elem->name, NULL, "entry", "entry"))
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
    cxml_assert__true(cxml_string_raw_equals(&comm->value, "cimple"))

    cxml_comment_node *previous = cxml_previous_comment(comm);
    cxml_assert__not_null(previous)
    cxml_assert__true(cxml_string_raw_equals(&previous->value, "foobar"))

    cxml_assert__null(cxml_previous_comment(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_next_text(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    cxml_assert__not_null(root)
    cxml_element_node *elem = cxml_find(root, "<noodles>/");
    cxml_assert__not_null(elem)
    cxml_assert__one(name_asserts(&elem->name, NULL, "noodles", "noodles"))
    cxml_text_node *text = NULL;
    cxml_for_each(n, &elem->children)
    {
        if (_cxml_node_type(n) == CXML_TEXT_NODE){
            text = n;
            break;
        }
    }
    cxml_assert__true(cxml_string_raw_equals(&text->value, "indomie"))

    cxml_text_node *next = cxml_next_text(text);
    cxml_assert__not_null(next)
    cxml_assert__true(cxml_string_raw_equals(&next->value, "super-pack"))

    cxml_assert__null(cxml_next_text(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_previous_text(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    cxml_assert__not_null(root)
    cxml_element_node *elem = cxml_find(root, "<noodles>/");
    cxml_assert__not_null(elem)
    cxml_assert__one(name_asserts(&elem->name, NULL, "noodles", "noodles"))
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
    cxml_assert__true(cxml_string_raw_equals(&text->value, "super-pack"))

    cxml_text_node *previous = cxml_previous_text(text);
    cxml_assert__not_null(previous)
    cxml_assert__true(cxml_string_raw_equals(&previous->value, "indomie"))

    cxml_assert__null(cxml_previous_text(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_first_child(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    cxml_assert__not_null(root)

    cxml_element_node *elem = cxml_find(root, "<noodles>/");
    cxml_assert__not_null(elem)

    cxml_text_node *first = cxml_first_child(elem);
    cxml_assert__not_null(first)
    cxml_assert__true(cxml_string_raw_equals(&first->value, "indomie"))
    cxml_assert__null(cxml_first_child(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_find_first_child(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    cxml_assert__not_null(root)

    cxml_text_node *first = cxml_find_first_child(root, "<noodles>/");
    cxml_assert__not_null(first)
    cxml_assert__true(cxml_string_raw_equals(&first->value, "indomie"))
    cxml_assert__null(cxml_find_first_child(NULL, "<entry>/"))
    cxml_assert__null(cxml_find_first_child(root, "<foobar>/"))
    cxml_assert__null(cxml_find_first_child(root, NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_parent(){
    deb()
    cxml_root_node *root = get_root("wf_xml_2.xml", true);
    cxml_assert__not_null(root)
    // finds first element
    cxml_element_node *elem = cxml_find(root, "<relationship>/");
    cxml_element_node *par = cxml_find(root, "<synonym>/");
    cxml_assert__not_null(elem)
    cxml_assert__not_null(par)

    cxml_element_node *parent = cxml_parent(elem);
    cxml_assert__not_null(par)
    cxml_assert__eq(par, parent)
    cxml_assert__one(name_asserts(&parent->name, NULL, "synonym", "synonym"))

    cxml_assert__null(cxml_parent(root))
    cxml_assert__null(cxml_parent(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_find_parent(){
    deb()
    cxml_root_node *root = get_root("wf_xml_4.xml", false);
    cxml_assert__not_null(root)
    // finds first element
    cxml_element_node *elem = cxml_find(root, "<f:width>/");
    cxml_element_node *par = cxml_find(root, "<f:table>/");
    cxml_assert__not_null(elem)
    cxml_assert__not_null(par)

    cxml_element_node *parent = cxml_find_parent(root, "<f:width>/");
    cxml_assert__not_null(parent)
    cxml_assert__eq(par, parent)
    cxml_assert__one(name_asserts(&parent->name, "f", "table", "f:table"))

    cxml_assert__null(cxml_find_parent(root, "<xyz>/"))
    cxml_assert__null(cxml_find_parent(root, NULL))
    cxml_assert__null(cxml_find_parent(NULL, "<f:width>/"))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_ancestors(){
    deb()
    cxml_cfg_set_doc_name("ancestor");
    cxml_root_node *root = get_root("wf_xml_2.xml", false);
    cxml_assert__not_null(root)
    cxml_element_node *elem = cxml_find(root, "<relationship>/$text|='xyz'/");
    cxml_assert__not_null(elem)

    cxml_list ancestors = new_cxml_list();
    char *ancestor_names[] = {"synonym", "entry", "thesaurus", "ancestor"};
    cxml_ancestors(elem, &ancestors);
    cxml_assert__eq(cxml_list_size(&ancestors), 4)
    int i = 0;
    cxml_for_each(node, &ancestors)
    {
        if (cxml_get_node_type(node) != CXML_ROOT_NODE){
            cxml_assert__one(name_asserts(
                    &_unwrap__cxnode(elem, node)->name, NULL, ancestor_names[i], ancestor_names[i]))
        }else{
            cxml_assert__one(cxml_string_raw_equals(
                    &_unwrap__cxnode(root, node)->name, ancestor_names[i]))
        }
        i++;
    }
    cxml_list_free(&ancestors);
    cxml_ancestors(NULL, &ancestors);
    cxml_assert__zero(cxml_list_size(&ancestors))
    cxml_ancestors(elem, NULL);
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_find_ancestors(){
    deb()
    cxml_root_node *root = get_root("wf_xml_2.xml", true);
    cxml_assert__not_null(root)

    cxml_list ancestors = new_cxml_list();
    char *ancestor_names[] = {"synonym", "entry", "thesaurus", "XMLDocument"};
    cxml_find_ancestors(root, "<relationship>/$text|='xyz'/", &ancestors);
    cxml_assert__eq(cxml_list_size(&ancestors), 4)
    int i = 0;
    cxml_for_each(node, &ancestors)
    {
        if (cxml_get_node_type(node) != CXML_ROOT_NODE){
            cxml_assert__one(name_asserts(
                    &_unwrap__cxnode(elem, node)->name, NULL, ancestor_names[i], ancestor_names[i]))
        }else{
            cxml_assert__one(cxml_string_raw_equals(
                    &_unwrap__cxnode(root, node)->name, ancestor_names[i]))
        }
        i++;
    }
    cxml_list_free(&ancestors);

    cxml_ancestors(NULL, &ancestors);
    cxml_assert__zero(cxml_list_size(&ancestors))

    cxml_find_ancestors(root, NULL, &ancestors);
    cxml_assert__zero(cxml_list_size(&ancestors))

    cxml_find_ancestors(NULL, "<relationship>/$text|='abc'/", &ancestors);
    cxml_assert__zero(cxml_list_size(&ancestors))

    cxml_find_ancestors(root, "<relationship>/$text|='abc'/", NULL);
    cxml_assert__zero(cxml_list_size(&ancestors))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_descendants(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    cxml_assert__not_null(root)
    cxml_element_node *elem = cxml_find(root, "<noodles>/");
    cxml_assert__not_null(elem)

    cxml_list descendants = new_cxml_list();
    char *ancestor_names_v[] = {
            "indomie", "seasoning", "maggi", "br", "mr-chef",
            "super-pack", "others"
    };
    cxml_descendants(elem, &descendants);
    cxml_assert__eq(cxml_list_size(&descendants), 7)
    int i = 0;
    cxml_for_each(node, &descendants)
    {
        if (cxml_get_node_type(node) != CXML_TEXT_NODE){
            cxml_assert__one(name_asserts(
                    &_unwrap__cxnode(elem, node)->name, NULL,
                    ancestor_names_v[i], ancestor_names_v[i]))
        }else{
            cxml_assert__one(cxml_string_raw_equals(
                    &_unwrap__cxnode(text, node)->value, ancestor_names_v[i]))
        }
        i++;
    }
    cxml_list_free(&descendants);

    cxml_descendants(NULL, &descendants);
    cxml_assert__zero(cxml_list_size(&descendants))

    cxml_descendants(elem, NULL);
    cxml_assert__zero(cxml_list_size(&descendants))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_find_descendants(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    cxml_assert__not_null(root)

    cxml_list descendants = new_cxml_list();
    char *ancestor_names_v[] = {
            "indomie", "seasoning", "maggi", "br", "mr-chef",
            "super-pack", "others"
    };
    cxml_find_descendants(root, "<noodles>/", &descendants);
    cxml_assert__eq(cxml_list_size(&descendants), 7)
    int i = 0;
    cxml_for_each(node, &descendants)
    {
        if (cxml_get_node_type(node) != CXML_TEXT_NODE){
            cxml_assert__one(name_asserts(
                    &_unwrap__cxnode(elem, node)->name,
                    NULL, ancestor_names_v[i], ancestor_names_v[i]))
        }else{
            cxml_assert__one(cxml_string_raw_equals(
                    &_unwrap__cxnode(text, node)->value, ancestor_names_v[i]))
        }
        i++;
    }
    cxml_list_free(&descendants);
    cxml_find_descendants(NULL, "<noodles>/", &descendants);
    cxml_assert__zero(cxml_list_size(&descendants))

    cxml_find_descendants(root, NULL, &descendants);
    cxml_assert__zero(cxml_list_size(&descendants))

    cxml_find_descendants(root, "<noodles>/", NULL);
    cxml_assert__zero(cxml_list_size(&descendants))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_next_sibling(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    cxml_assert__not_null(root)
    cxml_element_node *elem = cxml_find(root, "<name>/");
    cxml_assert__not_null(elem)
    cxml_assert__one(name_asserts(&elem->name, NULL, "name", "name"))

    cxml_element_node *next = cxml_next_sibling(elem);
    cxml_assert__not_null(next)
    cxml_assert__one(name_asserts(&next->name, NULL, "color", "color"))
    cxml_assert__null(next->namespace)

    cxml_assert__null(cxml_next_sibling(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_previous_sibling(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    cxml_assert__not_null(root)
    cxml_element_node *elem = cxml_find(root, "<br>/");
    cxml_assert__not_null(elem)
    cxml_assert__one(name_asserts(&elem->name, NULL, "br", "br"))

    cxml_text_node *previous = cxml_previous_sibling(elem);
    cxml_assert__not_null(previous)
    cxml_assert__true(cxml_string_raw_equals(&previous->value, "red"))
    cxml_assert__null(cxml_previous_sibling(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_find_next_sibling(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    cxml_assert__not_null(root)

    cxml_element_node *next = cxml_find_next_sibling(root, "<color>/");
    cxml_assert__not_null(next)
    cxml_assert__one(name_asserts(&next->name, NULL, "shape", "shape"))
    cxml_assert__null(next->namespace)

    cxml_assert__null(cxml_find_next_sibling(root, NULL))
    cxml_assert__null(cxml_find_next_sibling(NULL, "<br>/"))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_find_previous_sibling(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    cxml_assert__not_null(root)

    cxml_element_node *previous = cxml_find_previous_sibling(root, "<shape>/");
    cxml_assert__not_null(previous)
    cxml_assert__one(name_asserts(&previous->name, NULL, "color", "color"))
    cxml_assert__null(previous->namespace)

    cxml_assert__null(cxml_find_next_sibling(root, NULL))
    cxml_assert__null(cxml_find_next_sibling(NULL, "<br>/"))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_siblings(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    cxml_assert__not_null(root)
    cxml_element_node *elem = cxml_find(root, "<seasoning>/");
    cxml_assert__not_null(elem)

    cxml_list siblings = new_cxml_list();
    char *sibling_names_v[] = {
            "indomie", "super-pack", "others"
    };
    cxml_siblings(elem, &siblings);
    cxml_assert__eq(cxml_list_size(&siblings), 3)
    int i = 0;
    cxml_for_each(node, &siblings)
    {
        if (cxml_get_node_type(node) != CXML_TEXT_NODE){
            cxml_assert__one(name_asserts(
                    &_unwrap__cxnode(elem, node)->name, NULL,
                    sibling_names_v[i], sibling_names_v[i]))
        }else{
            cxml_assert__one(cxml_string_raw_equals(
                    &_unwrap__cxnode(text, node)->value, sibling_names_v[i]))
        }
        i++;
    }
    cxml_list_free(&siblings);

    cxml_siblings(NULL, &siblings);
    cxml_assert__zero(cxml_list_size(&siblings))

    cxml_siblings(elem, NULL);
    cxml_assert__zero(cxml_list_size(&siblings))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_find_siblings(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    cxml_assert__not_null(root)

    cxml_list siblings = new_cxml_list();
    char *sibling_names_v[] = {
            "indomie", "super-pack", "others"
    };
    cxml_find_siblings(root, "<seasoning>/", &siblings);
    cxml_assert__eq(cxml_list_size(&siblings), 3)
    int i = 0;
    cxml_for_each(node, &siblings)
    {
        if (cxml_get_node_type(node) != CXML_TEXT_NODE){
            cxml_assert__one(name_asserts(
                    &_unwrap__cxnode(elem, node)->name, NULL, sibling_names_v[i], sibling_names_v[i]))
        }else{
            cxml_assert__one(cxml_string_raw_equals(
                    &_unwrap__cxnode(text, node)->value, sibling_names_v[i]))
        }
        i++;
    }
    cxml_list_free(&siblings);

    cxml_siblings(NULL, &siblings);
    cxml_assert__zero(cxml_list_size(&siblings))

    cxml_find_siblings(root, NULL, &siblings);
    cxml_assert__zero(cxml_list_size(&siblings))

    cxml_find_siblings(NULL, "<seasoning>/", &siblings);
    cxml_assert__zero(cxml_list_size(&siblings))

    cxml_find_siblings(root, "<seasoning>/", NULL);
    cxml_assert__zero(cxml_list_size(&siblings))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_get_attribute(){
    deb()
    cxml_root_node *root = get_root("wf_xml_1.xml", false);
    cxml_assert__not_null(root)

    cxml_element_node *elem = cxml_get_root_element(root);
    cxml_attribute_node *attr = cxml_get_attribute(elem, "fish");
    cxml_assert__not_null(attr)
    cxml_assert__one(name_asserts(&attr->name, NULL, "fish", "fish"))
    cxml_assert__true(cxml_string_raw_equals(&attr->value, "mackerel"))

    cxml_assert__null(cxml_get_attribute(NULL, "fish"))
    cxml_assert__null(cxml_get_attribute(elem, NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_find_attribute(){
    deb()
    cxml_root_node *root = get_root("wf_xml_1.xml", true);
    cxml_assert__not_null(root)

    cxml_attribute_node *attr = cxml_find_attribute(root, "<mod>/", "section");
    cxml_assert__not_null(attr)
    cxml_assert__one(name_asserts(&attr->name, NULL, "section", "section"))
    cxml_assert__true(cxml_string_raw_equals(&attr->value, "converter"))

    cxml_assert__null(cxml_find_attribute(root, "<xyz>/", "section")) // no element xyz
    cxml_assert__null(cxml_find_attribute(root, NULL, "section"))   // NULL
    cxml_assert__null(cxml_find_attribute(root, "<xyz>/", NULL))    // no attribute name
    cxml_assert__null(cxml_find_attribute(NULL, "<xyz>/", "section"))   // no root node
    cxml_assert__null(cxml_find_attribute(root, "<and>/", ""))  // empty attribute
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_attributes(){
    deb()
    cxml_root_node *root = get_root("wf_xml_8.xml", true);
    cxml_assert__not_null(root)

    cxml_element_node *elem = cxml_find(root, "<or>/");
    cxml_assert__not_null(elem)

    cxml_list attributes = new_cxml_list();
    cxml_attributes(elem, &attributes);
    cxml_assert__eq(cxml_list_size(&attributes), 3)

    char *attr_names[]  = {"mod", "van", "hex"};
    char *attr_values[] = {"63", "'40'", "0xdeadbeef"};
    long attr_num_values[] = {63, 0, 0xdeadbeef};

    int i = 0;
    cxml_for_each(attr, &attributes)
    {
        cxml_assert__one(name_asserts(&((cxml_attribute_node*)attr)->name, NULL, attr_names[i], attr_names[i]))
        cxml_assert__true(cxml_string_raw_equals(&((cxml_attribute_node *) attr)->value, attr_values[i]))
        cxml_assert__eq(((cxml_attribute_node*)attr)->number_value.dec_val, attr_num_values[i])
        i++;
    }

    cxml_list_free(&attributes);

    cxml_attributes(NULL, &attributes);
    cxml_assert__zero(cxml_list_size(&attributes))

    cxml_attributes(elem, NULL);
    cxml_assert__zero(cxml_list_size(&attributes))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_find_attributes(){
    deb()
    cxml_root_node *root = get_root("wf_xml_8.xml", false);
    cxml_assert__not_null(root)

    cxml_list attributes = new_cxml_list();
    cxml_find_attributes(root, "<or>/", &attributes);
    cxml_assert__eq(cxml_list_size(&attributes), 3)

    char *attr_names[]  = {"mod", "van", "hex"};
    char *attr_values[] = {"63", "'40'", "0xdeadbeef"};
    long attr_num_values[] = {63, 0, 0xdeadbeef};

    int i = 0;
    cxml_for_each(attr, &attributes)
    {
        cxml_assert__one(name_asserts(&((cxml_attribute_node*)attr)->name, NULL, attr_names[i], attr_names[i]))
        cxml_assert__true(cxml_string_raw_equals(&((cxml_attribute_node *) attr)->value, attr_values[i]))
        cxml_assert__eq(((cxml_attribute_node*)attr)->number_value.dec_val, attr_num_values[i])
        i++;
    }

    cxml_list_free(&attributes);

    cxml_find_attributes(root, NULL, &attributes);
    cxml_assert__zero(cxml_list_size(&attributes))

    cxml_find_attributes(root, "<and>/", NULL);
    cxml_assert__zero(cxml_list_size(&attributes))

    cxml_find_attributes(NULL, "<div>/", &attributes);
    cxml_assert__zero(cxml_list_size(&attributes))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_text_as_cxml_string(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_6);
    cxml_assert__not_null(root)

    //text
    cxml_string text = new_cxml_string_s("indomiemaggimr-chefsuper-pack");
    cxml_string *str = cxml_text_as_cxml_string(root, NULL);
    cxml_assert__true(cxml_string_equals(&text, str))
    cxml_assert__null(cxml_text(NULL, NULL))
    cxml_string_free(&text);
    cxml_string_free(str);
    FREE(str);

    text = new_cxml_string_s("indomie~maggi~mr-chef~super-pack");
    str = cxml_text_as_cxml_string(root, "~");
    cxml_assert__true(cxml_string_equals(&text, str))
    cxml_string_free(&text);
    cxml_string_free(str);
    FREE(str);

    text = new_cxml_string_s("indomie maggi mr-chef super-pack");
    str = cxml_text_as_cxml_string(root, " ");
    cxml_assert__true(cxml_string_equals(&text, str))
    cxml_string_free(&text);
    cxml_string_free(str);
    FREE(str);

    text = new_cxml_string_s("indomiemaggimr-chefsuper-pack");
    str = cxml_text_as_cxml_string(root, "");
    cxml_assert__true(cxml_string_equals(&text, str))
    cxml_string_free(&text);
    cxml_string_free(str);
    FREE(str);

    cxml_assert__null(cxml_text_as_cxml_string(NULL, "foobar"))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_text(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    cxml_assert__not_null(root)

    //text
    cxml_string text = new_cxml_string_s("appleredblueroundish");
    char *str = cxml_text(root, NULL);
    cxml_assert__true(cxml_string_lraw_equals(&text, str, strlen(str)))
    cxml_assert__null(cxml_text(NULL, NULL))
    cxml_string_free(&text);
    FREE(str);

    text = new_cxml_string_s("apple|red|blue|roundish");
    str = cxml_text(root, "|");
    cxml_assert__true(cxml_string_lraw_equals(&text, str, strlen(str)))
    cxml_string_free(&text);
    FREE(str);

    text = new_cxml_string_s("apple^^^red^^^blue^^^roundish");
    str = cxml_text(root, "^^^");
    cxml_assert__true(cxml_string_lraw_equals(&text, str, strlen(str)))
    cxml_string_free(&text);
    FREE(str);

    text = new_cxml_string_s("appleredblueroundish");
    str = cxml_text(root, "");
    cxml_assert__true(cxml_string_lraw_equals(&text, str, strlen(str)))
    cxml_string_free(&text);
    FREE(str);

    cxml_assert__null(cxml_text(NULL, "foobar"))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_get_name(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    cxml_assert__not_null(root)
    // returned name must be freed by us
    char *name = cxml_get_name(root);
    // default document name is XMLDocument
    cxml_assert__true(cxml_string_lraw_equals(&root->name, name, strlen(name)))
    FREE(name);

    cxml_element_node *elem = cxml_find(root, "<shape>/");
    name = cxml_get_name(elem);
    cxml_assert__true(cxml_string_lraw_equals(&elem->name.qname, name, strlen(name)))
    FREE(name);

    cxml_assert__null(cxml_get_name(NULL));
    cxml_destroy(root);

    cxml_pass()
}

cts test_cxml_get_bound_namespace(){
    deb()
    /*
     * "<f:table xmlns:f=\"https://www.w3schools.com/furniture\">\n"
"        <f:name>African Coffee Table<![CDATA[Testing cdata stuff]]></f:name>\n"
"        <f:width>80</f:width>\n"
"        <f:length>120</f:length>\n"
" </f:table>";
     */
    cxml_root_node *root = get_root("wf_xml_4.xml", true);
    cxml_assert__not_null(root)

    cxml_element_node *elem = cxml_find(root, "<f:width>/");
    cxml_assert__not_null(elem)

    cxml_element_node *root_elem = root->root_element; // 'f:table'
    cxml_assert__not_null(root_elem)
    cxml_assert__not_null(root_elem->namespaces)

    // obtain the namespace the element elem is bound to
    cxml_namespace_node *ns = cxml_get_bound_namespace(elem);
    cxml_assert__not_null(ns)

    // find the namespace node from where it was declared in its parent element (root_elem)
    cxml_namespace_node *target = cxml_list_get(root_elem->namespaces, 0);

    // check that the namespace found is same as the namespace bound to the element 'f:width'
    cxml_assert__eq(ns, target)
    cxml_assert__true(cxml_string_equals(&ns->prefix, &target->prefix))
    cxml_assert__true(cxml_string_equals(&ns->uri, &target->uri))

    cxml_assert__null(cxml_get_bound_namespace(NULL))

    cxml_destroy(root);

    cxml_pass()
}

cts test_cxml_get_comments(){
    deb()
    cxml_root_node *root = get_root("wf_xml_3.xml", false);
    cxml_assert__not_null(root)

    cxml_list comments = new_cxml_list();
    cxml_get_comments(root, &comments, 1);
    cxml_assert__two(cxml_list_size(&comments))

    char *values[] = {"foobar", "cimple"};
    int i = 0;
    cxml_for_each(comment, &comments)
    {
        cxml_assert__true(cxml_string_lraw_equals(
                &((cxml_comment_node *) comment)->value,
                values[i], strlen(values[i])))
        i++;
    }
    cxml_list_free(&comments);

    cxml_get_comments(root, &comments, 0);
    cxml_assert__zero(cxml_list_size(&comments))

    cxml_element_node *elem = cxml_find(root, "<entry>/");
    cxml_get_comments(elem, &comments, 1);
    cxml_assert__two(cxml_list_size(&comments))

    cxml_list_free(&comments);

    cxml_get_comments(root, NULL, 0);
    cxml_assert__zero(cxml_list_size(&comments))

    cxml_get_comments(NULL, &comments, 0);
    cxml_assert__zero(cxml_list_size(&comments))

    cxml_destroy(root);
    cxml_pass()
}


/*********************************
 *                               *
 * creation methods/functions    *
 *********************************
 */
/**General**/
cts test_cxml_create_node(){
    deb()
    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    cxml_assert__not_null(elem)
    cxml_assert__eq(elem->_type, CXML_ELEM_NODE)

    cxml_assert__null(cxml_create_node(100))
    cxml_destroy(elem);
    cxml_pass()
}

cts test_cxml_set_name(){
    deb()
    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    cxml_assert__not_null(elem)

    // cannot use xmlns as prefix name of element
    cxml_assert__false(cxml_set_name(elem, "xmlns", "bar"))
    cxml_assert__true(cxml_set_name(elem, "foo", "bar"))
    cxml_assert__one(name_asserts(&elem->name, "foo", "bar", "foo:bar"))
    cxml_name_free(&elem->name);

    // fails: we can't set prefix name, if the element/attribute has no local name,
    // without setting a local name (local name cannot be empty)
    cxml_assert__false(cxml_set_name(elem, "foo", NULL))
    cxml_assert__false(cxml_set_name(elem, "foo", ""))
    cxml_assert__false(cxml_set_name(elem, NULL, ""))
    cxml_assert__false(cxml_set_name(elem, "", "new"))
    cxml_assert__false(cxml_set_name(NULL, "foo", "new"))

    cxml_assert__true(cxml_set_name(elem, "xy", "abc"))
    cxml_assert__one(name_asserts(&elem->name, "xy", "abc", "xy:abc"))

    cxml_assert__true(cxml_set_name(elem, NULL, "abc"))
    cxml_assert__one(name_asserts(&elem->name, "xy", "abc", "xy:abc"))
    cxml_destroy(elem);


    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE);
    cxml_assert__not_null(attr)

    cxml_assert__true(cxml_set_name(attr, "foo", "bar"));
    cxml_assert__one(name_asserts(&attr->name, "foo", "bar", "foo:bar"))

    // this shouldn't fail, we can set the prefix name since the local name isn't empty.
    // (it was set above.)
    cxml_assert__true(cxml_set_name(attr, "foo", NULL))

    // fails: we can't set prefix name, if the element/attribute has no local name,
    // without setting a local name (local name cannot be empty)
    cxml_assert__false(cxml_set_name(attr, "foo", ""))
    cxml_assert__false(cxml_set_name(attr, NULL, ""))
    cxml_assert__false(cxml_set_name(attr, "", "new"))
    cxml_assert__false(cxml_set_name(NULL, "foo", "new"))
    cxml_assert__one(name_asserts(&attr->name, "foo", "bar", "foo:bar"))

    // this should fail (not a valid identifier)
    cxml_assert__false(cxml_set_name(attr, NULL, "203"))

    cxml_assert__one(name_asserts(&attr->name, "foo", "bar", "foo:bar"))  // unchanged

    cxml_assert__false(cxml_set_name(attr, ".123", "bad"))
    cxml_assert__one(name_asserts(&attr->name, "foo", "bar", "foo:bar"))  // unchanged

    cxml_assert__true(cxml_set_name(attr, "xy", "abc"))
    cxml_assert__one(name_asserts(&attr->name, "xy", "abc", "xy:abc"))
    cxml_assert__true(cxml_set_name(attr, NULL, "bad"))
    cxml_assert__one(name_asserts(&attr->name, "xy", "bad", "xy:bad"))

    // we can set the prefix name alone since the local name isn't empty.
    cxml_assert__true(cxml_set_name(attr, "dodo", NULL))
    cxml_assert__one(name_asserts(&attr->name, "dodo", "bad", "dodo:bad"))

    cxml_destroy(attr);

    cxml_pass()
}

cts test_cxml_add_child(){
    deb()
    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    cxml_assert__not_null(elem)
    cxml_assert__true(cxml_set_name(elem, NULL, "abc"))
    cxml_assert__one(name_asserts(&elem->name, NULL, "abc", "abc"))
    cxml_assert__zero(cxml_list_size(&elem->children))
    cxml_assert__false(elem->has_text)
    cxml_assert__false(elem->has_child)

    char *got = cxml_element_to_rstring(elem);
    cxml_assert__not_null(got)
    char *expected = "<abc/>";
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    FREE(got);
    got = NULL;

    cxml_text_node *text = cxml_create_node(CXML_TEXT_NODE);
    cxml_assert__not_null(text)
    cxml_assert__null(text->parent)

    cxml_set_text_value(text, "this is some text", false);
    cxml_assert__true(cxml_add_child(elem, text))
    cxml_assert__not_null(text->parent)
    cxml_assert__zero(text->number_value.dec_val)
    cxml_assert__eq(text->parent, elem)
    cxml_assert__true(elem->has_child)
    cxml_assert__true(elem->has_text)
    cxml_assert__one(cxml_list_size(&elem->children))

    got = cxml_element_to_rstring(elem);
    cxml_assert__not_null(got)
    expected = "<abc>\n"
               "  this is some text\n"
               "</abc>";
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    FREE(got);

    cxml_assert__false(cxml_add_child(elem, NULL))
    cxml_assert__one(cxml_list_size(&elem->children))

    cxml_assert__false(cxml_add_child(NULL, text))
    cxml_assert__one(cxml_list_size(&elem->children))
    // this also frees `text` above
    cxml_destroy(elem);;
    cxml_pass()
}

cts test_cxml_set_namespace(){
    deb()
    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    cxml_assert__true(cxml_set_name(elem, "pp", "element"))
    cxml_assert__true(name_asserts(&elem->name, "pp", "element", "pp:element"))
    // the element becomes namespaced when it is bound to an actual namespace object
    cxml_assert__false(elem->is_namespaced)
    cxml_assert__null(elem->namespace)
    cxml_namespace_node *ns = cxml_create_node(CXML_NS_NODE);
    cxml_assert__not_null(ns)
    char *pref = "prefix";
    char *uri = "http://not-a-uri.com/";
    // cannot set xmlns as namespace prefix
    cxml_assert__false(cxml_set_namespace_prefix(ns, "xmlns"))
    // we cannot call cxml_set_namespace_prefix() to directly set the
    // namespace prefix to "xml" as the query API does not trust that we'll
    // call cxml_set_namespace_uri() to set the corresponding URI for the xml prefix.
    // the only way this can be done, is by using cxml_set_namespace_data()
    cxml_assert__false(cxml_set_namespace_prefix(ns, "xml"))
    // the standard uri must be used for the xml uri; this isn't it
    cxml_assert__false(cxml_set_namespace_uri(ns, "http://www.w3.org/2000/xmlns/"))

    cxml_assert__true(cxml_set_namespace_prefix(ns, pref))
    cxml_assert__true(cxml_set_namespace_uri(ns, uri))
    cxml_assert__true(cxml_string_raw_equals(&ns->prefix, pref))
    cxml_assert__true(cxml_string_raw_equals(&ns->uri, uri))
    // mismatching prefix
    cxml_assert__false(cxml_set_namespace(elem, ns))
    cxml_assert__true(cxml_set_name(elem, "prefix", NULL))

    cxml_assert__true(cxml_set_namespace(elem, ns))
    cxml_assert__not_null(elem->namespace)
    // now its bound
    cxml_assert__true(elem->is_namespaced)
    cxml_assert__eq(elem->namespace, ns)

    cxml_assert__false(cxml_set_namespace(elem, NULL))
    cxml_assert__false(cxml_set_namespace(NULL, ns))

    cxml_destroy(elem);

    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE);
    cxml_assert__not_null(attr)
    // fails, no prefix
    cxml_assert__false(cxml_set_namespace(attr, ns))

    cxml_assert__true(cxml_set_name(attr, NULL, "attribute"))
    cxml_assert__true(name_asserts(&attr->name, NULL, "attribute", "attribute"))
    // fails, no prefix
    cxml_assert__false(cxml_set_namespace(attr, ns))

    // we can set the prefix name alone since the local name isn't empty.
    cxml_assert__true(cxml_set_name(attr, "prefix", NULL))
    cxml_assert__true(name_asserts(&attr->name, "prefix", "attribute", "prefix:attribute"))
    // succeeds
    cxml_assert__true(cxml_set_namespace(attr, ns))
    cxml_assert__not_null(attr->namespace)
    cxml_assert__eq(attr->namespace, ns)

    cxml_destroy(attr);
    // we have to destroy `ns` because the element doesn't own it. It's only bound to it.
    // so when `elem` was destroyed/deleted, it doesn't affect `ns`
    cxml_destroy(ns);
    cxml_pass()
}

cts test_cxml_add_namespace(){
    deb()
    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    cxml_assert__not_null(elem)
    cxml_assert__true(cxml_set_name(elem, NULL, "element"))
    cxml_assert__null(elem->namespaces)
    char *str = cxml_element_to_rstring(elem);
    cxml_assert__not_null(str)
    cxml_assert__true(cxml_string_llraw_equals(str, "<element/>", strlen(str), 10))
    FREE(str);

    cxml_namespace_node *ns = cxml_create_node(CXML_NS_NODE);
    cxml_assert__not_null(ns)
    char *pref = "xml";
    char *uri = "http://www.w3.org/XML/1998/namespace";
    // fails: this uri belongs to xmlns
    cxml_assert__false(cxml_set_namespace_data(ns, "xyz", "http://www.w3.org/2000/xmlns/"))
    cxml_assert__false(cxml_set_namespace_data(ns, "xyz", uri))
    cxml_assert__true(cxml_set_namespace_data(ns, pref, uri))
    // elem now owns `ns`
    cxml_assert__true(cxml_add_namespace(elem, ns))
    cxml_assert__not_null(elem->namespaces)
    cxml_assert__one(cxml_list_size(elem->namespaces))
    str = cxml_element_to_rstring(elem);
    cxml_assert__not_null(str)
    // global namespaces are not shown
    cxml_assert__true(cxml_string_llraw_equals(str, "<element/>", strlen(str), 10))
    // cannot set namespace of `elem` to `ns` -> 'xml' since `elem` doesn't have this prefix,
    cxml_assert__false(cxml_set_namespace(elem, ns))
    // update `elem`'s name to include the prefix of the namespace node it wants to be set to
    cxml_assert__true(cxml_set_name(elem, "xml", "element"))
    cxml_assert__true(cxml_set_namespace(elem, ns))
    FREE(str);
    str = cxml_element_to_rstring(elem);
    cxml_assert__true(cxml_string_llraw_equals(str, "<xml:element/>", strlen(str), 14))
    FREE(str);

    cxml_assert__false(cxml_add_namespace(NULL, ns))
    cxml_assert__false(cxml_add_namespace(elem, NULL))
    // `elem` owns `ns` so it frees it automatically.
    cxml_destroy(elem);
    cxml_pass()
}

cts test_cxml_add_attribute(){
    deb()
    // "<fruit a=\"boy\"><name>apple</name></fruit>"
    cxml_root_node *root = cxml_load_string(wf_xml_09);
    cxml_assert__not_null(root)
    cxml_element_node *elem = root->root_element;
    cxml_assert__one(cxml_table_size(elem->attributes))

    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE),
                        *attr2 = cxml_create_node(CXML_ATTR_NODE);
    cxml_assert__not_null(attr)
    cxml_assert__not_null(attr2)
    cxml_set_attribute_data(attr, NULL, "soup", "egusi");
    cxml_set_attribute_data(attr2, "xml", "fish", "coté");

    cxml_assert__true(cxml_add_attribute(elem, attr))
    cxml_assert__true(cxml_add_attribute(elem, attr2))
    cxml_assert__eq(cxml_table_size(elem->attributes), 3)
    char *got = cxml_element_to_rstring(elem);
    cxml_assert__not_null(got)
    char *expected = "<fruit a=\"boy\" soup=\"egusi\" xml:fish=\"coté\">\n"
                     "  <name>\n"
                     "    apple\n"
                     "  </name>\n"
                     "</fruit>";
    cxml_assert__true(cxml_string_llraw_equals(expected, got, strlen(expected), strlen(got)))
    FREE(got);

    cxml_assert__false(cxml_add_attribute(elem, NULL))
    cxml_assert__false(cxml_add_attribute(NULL, attr))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_set_attribute_value(){
    deb()
    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE);
    cxml_assert__not_null(attr)
    cxml_assert__true(cxml_set_attribute_value(attr, "sweet"))
    cxml_assert__true(cxml_string_lraw_equals(&attr->value, "sweet", 5))
    cxml_string_free(&attr->value);

    cxml_assert__true(cxml_set_attribute_value(attr, "0xfeedbac"))
    cxml_assert__eq(attr->number_value.dec_val, 0xfeedbac)

    cxml_assert__false(cxml_set_attribute_value(attr, NULL))
    cxml_assert__false(cxml_set_attribute_value(NULL, "sweet"))

    cxml_destroy(attr);
    cxml_pass()
}

cts test_cxml_set_attribute_name(){
    deb()
    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE);
    cxml_assert__not_null(attr)
    cxml_assert__true(cxml_set_attribute_name(attr, NULL, "sweet"))
    cxml_assert__true(cxml_string_lraw_equals(&attr->name.qname, "sweet", 5))
    char *str = cxml_attribute_to_rstring(attr);
    cxml_assert__true(cxml_string_llraw_equals(str, "sweet=\"\"", strlen(str), 8))
    cxml_name_free(&attr->name);
    FREE(str);

    cxml_assert__true(cxml_set_attribute_name(attr, "xyz", "sweet"))
    str = cxml_attribute_to_rstring(attr);
    cxml_assert__true(cxml_string_llraw_equals(str, "xyz:sweet=\"\"", strlen(str), 12))
    FREE(str);

    cxml_assert__false(cxml_set_attribute_name(attr, "", ""))
    cxml_assert__false(cxml_set_attribute_name(attr, NULL, ""))
    cxml_assert__false(cxml_set_attribute_name(NULL, "fx", ""))
    cxml_assert__false(cxml_set_attribute_name(NULL, "fx", "amazing"))

    cxml_destroy(attr);
    cxml_pass()
}

cts test_cxml_set_attribute_data(){
    deb()
    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE);
    cxml_assert__not_null(attr)
    cxml_assert__true(cxml_set_attribute_data(attr, "xml", "food", "rice"))
    char *str = cxml_attribute_to_rstring(attr);
    cxml_assert__true(cxml_string_llraw_equals(str, "xml:food=\"rice\"", strlen(str), 15))
    FREE(str);

    cxml_assert__false(cxml_set_attribute_data(attr, "xmlns", NULL, "rice"))
    cxml_assert__false(cxml_set_attribute_data(NULL, "xml", "food", "rice"))
    cxml_assert__false(cxml_set_attribute_data(attr, "xml", "food", NULL))
    cxml_destroy(attr);

    cxml_pass()
}

cts test_cxml_set_text_value(){
    deb()
    cxml_text_node *text = cxml_create_node(CXML_TEXT_NODE);
    cxml_assert__not_null(text)
    cxml_assert__true(cxml_set_text_value(text, "this is some text", false))
    cxml_assert__true(cxml_set_text_value(text, "this is some <![CDATA[stuffy]]>", true))
    cxml_assert__true(cxml_string_raw_equals(&text->value, "this is some <![CDATA[stuffy]]>"))
    cxml_assert__true(cxml_set_text_value(text, " 12356\r ", false))
    cxml_assert__eq(text->number_value.dec_val, 12356)
    cxml_assert__false(cxml_set_text_value(NULL, "this is some text", false))
    cxml_assert__false(cxml_set_text_value(text, NULL, false))
    cxml_destroy(text);
    cxml_pass()
}

cts test_cxml_set_comment_value(){
    deb()
    cxml_comment_node *comment = cxml_create_node(CXML_COMM_NODE);
    cxml_assert__not_null(comment)
    cxml_assert__true(cxml_set_comment_value(comment, "this is some text"))
    cxml_assert__false(cxml_set_comment_value(NULL, "this is some text"))
    cxml_assert__false(cxml_set_comment_value(comment, NULL))
    cxml_destroy(comment);
    cxml_pass()
}

cts test_cxml_set_pi_target(){
    deb()
    cxml_pi_node *pi = cxml_create_node(CXML_PI_NODE);
    cxml_assert__not_null(pi)
    cxml_assert__true(cxml_set_pi_target(pi, "abc"))
    cxml_assert__true(cxml_string_lraw_equals(&pi->target, "abc", 3))

    cxml_assert__false(cxml_set_pi_target(pi, ""))
    cxml_assert__false(cxml_set_pi_target(pi, NULL))
    cxml_assert__false(cxml_set_pi_target(NULL, "abc"))
    cxml_destroy(pi);
    cxml_pass()
}

cts test_cxml_set_pi_value(){
    deb()
    cxml_pi_node *pi = cxml_create_node(CXML_PI_NODE);
    cxml_assert__not_null(pi)
    cxml_assert__true(cxml_set_pi_value(pi, "abc"))
    cxml_assert__true(cxml_string_lraw_equals(&pi->value, "abc", 3))

    cxml_assert__false(cxml_set_pi_value(pi, NULL))
    cxml_assert__false(cxml_set_pi_value(NULL, "abc"))
    cxml_destroy(pi);
    cxml_pass()
}

cts test_cxml_set_pi_data(){
    deb()
    cxml_pi_node *pi = cxml_create_node(CXML_PI_NODE);
    cxml_assert__not_null(pi)
    cxml_assert__true(cxml_set_pi_data(pi, "abc", "some value"))
    cxml_assert__true(cxml_string_lraw_equals(&pi->target, "abc", 3))
    cxml_assert__true(cxml_string_lraw_equals(&pi->value, "some value", 10))

    cxml_assert__false(cxml_set_pi_data(pi, NULL, "some value"))
    cxml_assert__false(cxml_set_pi_data(pi, "abc", NULL))
    cxml_assert__false(cxml_set_pi_data(NULL, "abc", "some value"))

    cxml_destroy(pi);
    cxml_pass()
}

cts test_cxml_set_namespace_prefix(){
    deb()
    cxml_namespace_node *ns = cxml_create_node(CXML_NS_NODE);
    cxml_assert__not_null(ns)
    cxml_assert__true(cxml_set_namespace_prefix(ns, "abc"))
    cxml_assert__true(cxml_string_lraw_equals(&ns->prefix, "abc", 3))

    cxml_assert__false(cxml_set_namespace_prefix(ns, ""))
    cxml_assert__false(cxml_set_namespace_prefix(ns, NULL))
    cxml_assert__false(cxml_set_namespace_prefix(NULL, "abc"))
    cxml_destroy(ns);
    cxml_pass()
}

cts test_cxml_set_namespace_uri(){
    deb()
    cxml_namespace_node *ns = cxml_create_node(CXML_NS_NODE);
    cxml_assert__not_null(ns)
    cxml_assert__true(cxml_set_namespace_uri(ns, "abc"))
    cxml_assert__true(cxml_string_lraw_equals(&ns->uri, "abc", 3))

    cxml_assert__false(cxml_set_namespace_uri(ns, NULL))
    // reserved for xml
    cxml_assert__false(cxml_set_namespace_uri(ns, "http://www.w3.org/XML/1998/namespace"))
    // reserved for xmlns
    cxml_assert__false(cxml_set_namespace_uri(ns, "http://www.w3.org/2000/xmlns/"))
    cxml_assert__false(cxml_set_namespace_uri(NULL, "abc"))
    cxml_destroy(ns);
    cxml_pass()
}

cts test_cxml_set_namespace_data(){
    deb()
    cxml_namespace_node *ns = cxml_create_node(CXML_NS_NODE);
    cxml_assert__not_null(ns)
    cxml_assert__true(cxml_set_namespace_data(ns, "abc", "some value"))
    cxml_assert__true(cxml_string_lraw_equals(&ns->prefix, "abc", 3))
    cxml_assert__true(cxml_string_lraw_equals(&ns->uri, "some value", 10))

    cxml_assert__false(cxml_set_namespace_data(ns, NULL, "some value"))
    cxml_assert__false(cxml_set_namespace_data(ns, "abc", NULL))
    // reserved for xml
    cxml_assert__false(cxml_set_namespace_data(ns, "abc", "http://www.w3.org/XML/1998/namespace"))
    // reserved for xmlns
    cxml_assert__false(cxml_set_namespace_data(ns, "abc", "http://www.w3.org/2000/xmlns/"))
    cxml_assert__false(cxml_set_namespace_data(NULL, "abc", "some value"))

    cxml_destroy(ns);
    cxml_pass()
}

cts test_cxml_set_root_node_name(){
    deb()
    cxml_root_node *root = cxml_create_node(CXML_ROOT_NODE);
    cxml_assert__not_null(root)
    cxml_assert__true(cxml_set_root_node_name(root, "doc"))
    cxml_assert__true(cxml_string_lraw_equals(&root->name, "doc", 3))

    cxml_assert__false(cxml_set_root_node_name(root, NULL))
    cxml_assert__false(cxml_set_root_node_name(NULL, "doc"))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_set_root_element(){
    deb()
    cxml_root_node *root = cxml_create_node(CXML_ROOT_NODE);
    cxml_assert__false(root->has_child)
    cxml_assert__null(root->root_element)

    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    cxml_assert__true(cxml_set_root_element(root, elem))
    cxml_assert__eq(root->root_element, elem)
    cxml_assert__true(root->has_child)

    cxml_destroy(root);
    root = cxml_load_string(wf_xml_7);
    elem = cxml_create_node(CXML_ELEM_NODE);
    // already has a root element
    cxml_assert__false(cxml_set_root_element(root, elem))
    cxml_assert__neq(root->root_element, elem)

    cxml_destroy(root);
    // we must free `elem` because it's not bound to `root`.
    cxml_destroy(elem);
    cxml_pass()
}


/*********************************
 *                               *
 * update methods/functions      *
 *********************************
 */

cts test_cxml_insert_before(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    cxml_assert__not_null(root)
    // create an element to be inserted
    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    cxml_assert__not_null(elem)
    cxml_assert__false(elem->has_parent)
    cxml_assert__true(cxml_set_name(elem, "xml", "bar"))

    // find an element we want to insert `elem` before
    cxml_element_node *name = cxml_find(root, "<name>/");
    // insert `elem` before `name`
    cxml_assert__true(cxml_insert_before(name, elem))
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    char *expected = "<fruit>\n"
                     "  <xml:bar/>\n"
                     "  <name>\n"
                     "    apple\n"
                     "  </name>\n"
                     "</fruit>";
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__eq(elem->parent, root->root_element)
    cxml_assert__true(elem->has_parent)

    cxml_assert__false(cxml_insert_before(name, NULL))
    cxml_assert__false(cxml_insert_before(NULL, elem))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_insert_after(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    cxml_assert__not_null(root)
    // create a comment to be inserted
    cxml_comment_node *comment = cxml_create_node(CXML_COMM_NODE);
    cxml_assert__not_null(comment)
    cxml_assert__null(comment->parent)
    cxml_assert__true(cxml_set_comment_value(comment, "this is a simple comment"))

    // find a place we want to insert `comment` after
    cxml_element_node *name = cxml_find(root, "<name>/");
    cxml_text_node *text = cxml_first_child(name);
    cxml_assert__false(name->has_comment)
    // insert `comment` after `text`
    cxml_assert__true(cxml_insert_after(text, comment))
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    char *expected = "<fruit>\n"
                     "  <name>\n"
                     "    apple\n"
                     "    <!--this is a simple comment-->\n"
                     "  </name>\n"
                     "</fruit>";
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__eq(comment->parent, name)
    cxml_assert__not_null(comment->parent)
    cxml_assert__true(name->has_comment)
    cxml_assert__two(cxml_list_size(&name->children))

    cxml_assert__false(cxml_insert_after(name, NULL))
    cxml_assert__false(cxml_insert_before(NULL, comment))

    cxml_destroy(root);
    cxml_pass()
}


/*********************************
 *                               *
 * delete methods/functions      *
 *********************************
 */

int element_repr_asserts(cxml_root_node *root){
    deb()
    char *expected = "<fruit>\n"
                     "  <name>\n"
                     "    apple\n"
                     "  </name>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(&root->root_element->children))
    cxml_assert__true(root->root_element->has_child)
    cxml_assert__false(root->root_element->is_self_enclosing)
    FREE(got);
    return 1;
}

cts test_cxml_delete_element(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    cxml_assert__not_null(root)
    cxml_assert__true(element_repr_asserts(root))

    cxml_element_node *name = cxml_find(root, "<name>/");
    cxml_assert__true(cxml_delete_element(name))
    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__zero(cxml_list_size(&root->root_element->children))
    cxml_assert__false(root->root_element->has_child)
    cxml_assert__true(root->root_element->is_self_enclosing)
    cxml_destroy(root);
    FREE(got);
    got = NULL;

    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    root = cxml_load_string(wf_xml_7);
    cxml_element_node *color = cxml_find(root, "<color>/");
    cxml_element_node *shape = cxml_find(root, "<shape>/");
    cxml_assert__not_null(color)
    cxml_assert__not_null(shape)
    cxml_assert__eq(cxml_list_size(&root->root_element->children), 3)

    cxml_assert__true(cxml_delete_element(color))
    cxml_assert__true(cxml_delete_element(shape))
    expected = "<fruit>\n"
               "  <name>\n"
               "    apple\n"
               "  </name>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_delete_element(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_drop_element(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    cxml_assert__not_null(root)
    cxml_assert__true(element_repr_asserts(root))

    cxml_element_node *name = cxml_find(root, "<name>/");
    cxml_assert__true(cxml_drop_element(name))
    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__zero(cxml_list_size(&root->root_element->children))
    cxml_assert__false(root->root_element->has_child)
    cxml_assert__true(root->root_element->is_self_enclosing)
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
    cxml_assert__not_null(color)
    cxml_assert__not_null(shape)
    cxml_assert__eq(cxml_list_size(&root->root_element->children), 3)

    cxml_assert__true(cxml_drop_element(color))
    cxml_assert__true(cxml_drop_element(shape))
    expected = "<fruit>\n"
               "  <name>\n"
               "    apple\n"
               "  </name>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_drop_element(NULL))
    cxml_destroy(root);
    // `color` & `shape` is no longer a part of `root` since we dropped it,
    // so its our responsibility to free it.
    cxml_destroy(color);
    cxml_destroy(shape);
    cxml_pass()
}

cts test_cxml_drop_element_by_query(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    cxml_assert__not_null(root)
    cxml_assert__true(element_repr_asserts(root))

    cxml_element_node *name = cxml_drop_element_by_query(root, "<name>/");
    cxml_assert__not_null(name)

    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__zero(cxml_list_size(&root->root_element->children))
    cxml_assert__false(root->root_element->has_child)
    cxml_assert__true(root->root_element->is_self_enclosing)
    cxml_destroy(root);
    // `name` is no longer a part of `root` since we dropped it,
    // so its our responsibility to free it.
    cxml_destroy(name);
    FREE(got);
    got = NULL;

    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    root = cxml_load_string(wf_xml_7);
    cxml_assert__eq(cxml_list_size(&root->root_element->children), 3)

    cxml_element_node *color = cxml_drop_element_by_query(root, "<color>/");
    cxml_element_node *shape = cxml_drop_element_by_query(root, "<shape>/");
    cxml_assert__not_null(color)
    cxml_assert__not_null(shape)
    expected = "<fruit>\n"
               "  <name>\n"
               "    apple\n"
               "  </name>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_drop_element_by_query(root, "<xyz>/"))
    cxml_assert__false(cxml_drop_element_by_query(NULL, "<xyz>/"))
    cxml_assert__false(cxml_drop_element_by_query(root, NULL))
    cxml_destroy(root);
    // `color` & `shape` is no longer a part of `root` since we dropped it,
    // so its our responsibility to free it.
    cxml_destroy(color);
    cxml_destroy(shape);
    cxml_pass()
}

cts test_cxml_delete_elements(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    cxml_assert__not_null(root)
    cxml_assert__true(element_repr_asserts(root))

    // delete all elements in the root element
    cxml_assert(cxml_delete_elements(root->root_element))

    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__zero(cxml_list_size(&root->root_element->children))
    cxml_assert__false(root->root_element->has_child)
    cxml_assert__true(root->root_element->is_self_enclosing)
    cxml_destroy(root);
    FREE(got);
    got = NULL;

    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    root = cxml_load_string(wf_xml_7);
    cxml_assert__eq(cxml_list_size(&root->root_element->children), 3)

    cxml_element_node *color = cxml_find(root, "<color>/");
    cxml_element_node *shape = cxml_find(root, "<shape>/");
    cxml_assert__not_null(color)
    cxml_assert__not_null(shape)
    // `br` gets deleted
    cxml_assert(cxml_delete_elements(color))
    // there are no elements in `shape`
    cxml_assert__false(cxml_delete_elements(shape))
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
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__eq(cxml_list_size(&root->root_element->children), 3)
    FREE(got);

    cxml_assert__false(cxml_delete_elements(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_delete_elements_by_query(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_10);
    cxml_assert__not_null(root)
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
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__two(cxml_list_size(&root->root_element->children))
    cxml_assert__true(root->root_element->has_child)
    cxml_assert__false(root->root_element->is_self_enclosing)
    FREE(got);
    got = NULL;

    cxml_assert__true(cxml_delete_elements_by_query(root, "<name>/"))

    expected = "<fruit/>";
    got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__zero(cxml_list_size(&root->root_element->children))
    cxml_assert__false(root->root_element->has_child)
    cxml_assert__true(root->root_element->is_self_enclosing)
    cxml_destroy(root);
    FREE(got);
    got = NULL;

    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    root = cxml_load_string(wf_xml_7);
    cxml_assert__eq(cxml_list_size(&root->root_element->children), 3)

    cxml_assert__true(cxml_delete_elements_by_query(root, "<color>/"))
    cxml_assert__true(cxml_delete_elements_by_query(root, "<shape>/"))

    expected = "<fruit>\n"
               "  <name>\n"
               "    apple\n"
               "  </name>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_delete_elements_by_query(root, "<shape>/"))
    cxml_assert__false(cxml_delete_elements_by_query(root, NULL))
    cxml_assert__false(cxml_delete_elements_by_query(NULL, "<shape>/"))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_drop_elements(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    cxml_assert__not_null(root)
    cxml_assert__true(element_repr_asserts(root))

    cxml_list list = new_cxml_list();
    // drop all elements in the root element
    cxml_assert__true(cxml_drop_elements(root->root_element, &list))
    cxml_assert__one(cxml_list_size(&list))

    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__zero(cxml_list_size(&root->root_element->children))
    cxml_assert__false(root->root_element->has_child)
    cxml_assert__true(root->root_element->is_self_enclosing)
    cxml_destroy(root);
    FREE(got);
    cxml_list_free(&list);

    cxml_assert__false(cxml_drop_elements(NULL, &list))
    cxml_assert__false(cxml_drop_elements(root, NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_drop_elements_by_query(){
    deb()
    // "<fruit><name>apple</name><name>banana</name></fruit>"
    cxml_root_node *root = cxml_load_string(wf_xml_10);
    cxml_assert__not_null(root)

    cxml_list list = new_cxml_list();
    // drop all `name` elements in the root element
    cxml_assert__true(cxml_drop_elements_by_query(root->root_element, "<name>/", &list))
    cxml_assert__two(cxml_list_size(&list))

    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__zero(cxml_list_size(&root->root_element->children))
    cxml_assert__false(root->root_element->has_child)
    cxml_assert__true(root->root_element->is_self_enclosing)
    cxml_destroy(root);
    FREE(got);
    cxml_list_free(&list);

    cxml_assert__false(cxml_drop_elements_by_query(root->root_element, NULL, &list))
    cxml_assert__false(cxml_drop_elements_by_query(root->root_element, "<name>/", NULL))
    cxml_assert__false(cxml_drop_elements_by_query(NULL, "<name>/", &list))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_delete_comment(){
    deb()
    // <fruit><!--some comment--><name>apple</name><!--another comment--></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_11);
    cxml_assert__not_null(root)

    cxml_comment_node *comm = cxml_first_child(root->root_element);
    cxml_assert__not_null(comm)
    cxml_assert__true(cxml_delete_comment(comm))
    char *expected = "<fruit>\n"
                     "  <name>\n"
                     "    apple\n"
                     "  </name>\n"
                     "  <!--another comment-->\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__two(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_delete_comment(NULL))
    cxml_destroy(root);

    cxml_pass()
}

cts test_cxml_drop_comment(){
    deb()
    // <fruit><!--some comment--><name>apple</name><!--another comment--></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_11);
    cxml_assert__not_null(root)

    cxml_comment_node *comm = cxml_first_child(root->root_element);
    cxml_assert__not_null(comm)
    cxml_assert__true(cxml_drop_comment(comm))
    char *expected = "<fruit>\n"
                     "  <name>\n"
                     "    apple\n"
                     "  </name>\n"
                     "  <!--another comment-->\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__two(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_drop_comment(NULL))
    cxml_destroy(root);
    // `comm` is no longer a part of `root` since we dropped it,
    // so its our responsibility to free it.
    cxml_destroy(comm);
    cxml_pass()
}

cts test_cxml_delete_text(){
    deb()
    // <fruit><name>apple</name></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    cxml_assert__not_null(root)

    cxml_element_node *name = cxml_find(root, "<name>/");
    cxml_assert__not_null(name)
    cxml_assert__one(cxml_list_size(&name->children))
    cxml_assert__false(name->is_self_enclosing)
    cxml_assert__true(name->has_text)

    cxml_assert__true(cxml_delete_text(cxml_first_child(name)))

    cxml_assert__zero(cxml_list_size(&name->children))
    cxml_assert__true(name->is_self_enclosing)
    cxml_assert__false(name->has_text)
    cxml_assert__false(name->has_child)
    char *expected = "<fruit>\n"
                     "  <name/>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_drop_text(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_drop_text(){
    deb()
    // <fruit><name>apple</name></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_9);
    cxml_assert__not_null(root)

    cxml_element_node *name = cxml_find(root, "<name>/");
    cxml_assert__not_null(name)
    cxml_assert__one(cxml_list_size(&name->children))
    cxml_assert__false(name->is_self_enclosing)
    cxml_assert__true(name->has_text)

    cxml_text_node *text = cxml_first_child(name);
    cxml_assert__true(cxml_drop_text(text))

    cxml_assert__zero(cxml_list_size(&name->children))
    cxml_assert__true(name->is_self_enclosing)
    cxml_assert__false(name->has_text)
    cxml_assert__false(name->has_child)
    char *expected = "<fruit>\n"
                     "  <name/>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(&root->root_element->children))
    cxml_assert__true(root->root_element->has_child)
    FREE(got);

    cxml_assert__false(cxml_drop_text(NULL))
    cxml_destroy(root);
    // `text` is no longer a part of `root` since we dropped it,
    // so its our responsibility to free it.
    cxml_destroy(text);
    cxml_pass()
}

cts test_cxml_delete_attribute(){
    deb()
    // <x:fruit one="1" two="2" xmlns:x="uri"></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_12);
    cxml_assert__not_null(root)
    cxml_assert__two(cxml_table_size(root->root_element->attributes))


    cxml_attribute_node *attr = cxml_get_attribute(root->root_element, "one");
    cxml_assert__true(cxml_delete_attribute(attr))
    char *expected = "<x:fruit two=\"2\" xmlns:x=\"uri\"/>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_table_size(root->root_element->attributes))
    FREE(got);

    cxml_assert__false(cxml_delete_attribute(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_drop_attribute(){
    deb()
    // <x:fruit one="1" two="2" xmlns:x="uri"></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_12);
    cxml_assert__not_null(root)
    cxml_assert__two(cxml_table_size(root->root_element->attributes))

    cxml_attribute_node *attr = cxml_get_attribute(root->root_element, "two");
    cxml_assert__true(cxml_drop_attribute(attr))
    char *expected = "<x:fruit one=\"1\" xmlns:x=\"uri\"/>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_table_size(root->root_element->attributes))
    FREE(got);

    cxml_assert__false(cxml_drop_attribute(NULL))
    cxml_destroy(root);
    cxml_pass()
}


cts test_cxml_unbound_element(){
    deb()
    // <x:fruit one="1" two="2" xmlns:x="uri"></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_12);
    cxml_assert__not_null(root)
    cxml_assert__one(cxml_list_size(root->root_element->namespaces))
    cxml_assert__true(root->root_element->is_namespaced)

    cxml_assert__true(cxml_unbind_element(root->root_element))
    cxml_assert__false(root->root_element->is_namespaced)
    cxml_assert__null(root->root_element->namespace)
    char *expected = "<fruit one=\"1\" two=\"2\" xmlns:x=\"uri\"/>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(root->root_element->namespaces)) // still exists, but unbound
    cxml_assert__false(root->root_element->is_namespaced)
    FREE(got);

    cxml_assert__false(cxml_unbind_element(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_unbound_attribute(){
    deb()
    // "<a xmlns:x=\"uri\"><x:fruit one=\"1\" two=\"2\" x:three=\"3\" four=\"4\"></x:fruit></a>"
    cxml_root_node *root = cxml_load_string(wf_xml_13);
    cxml_assert__not_null(root)
    cxml_assert__one(cxml_list_size(root->root_element->namespaces))

    cxml_attribute_node *attr = cxml_get_attribute(cxml_first_child(root->root_element), "x:three");
    cxml_assert__not_null(attr)
    cxml_assert__not_null(attr->namespace)
    cxml_assert__true(cxml_unbind_attribute(attr))
    cxml_assert__null(attr->namespace)

    char *expected = "<a xmlns:x=\"uri\">\n  <x:fruit one=\"1\" two=\"2\" three=\"3\" four=\"4\"/>\n</a>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(root->root_element->namespaces))
    cxml_assert__null(attr->namespace)
    FREE(got);

    cxml_assert__false(cxml_unbind_attribute(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_delete_pi(){
    deb()
    // <fruit><?pi data?><class>basic<?pi data?></class><?pi data2?></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_14);
    cxml_assert__not_null(root)

    cxml_assert__eq(cxml_list_size(&root->root_element->children), 3)
    cxml_pi_node *pi = cxml_first_child(root->root_element);
    cxml_assert__not_null(pi)

    cxml_assert__true(cxml_delete_pi(pi))

    char *expected = "<fruit>\n"
                     "  <class>\n"
                     "    basic\n"
                     "    <?pi data?>\n"
                     "  </class>\n"
                     "  <?pi data2?>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__two(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_delete_pi(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_drop_pi(){
    deb()
    // <fruit><?pi data?><class>basic<?pi data?></class><?pi data2?></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_14);
    cxml_assert__not_null(root)

    cxml_assert__eq(cxml_list_size(&root->root_element->children), 3)
    cxml_pi_node *pi = cxml_first_child(root->root_element);
    cxml_assert__not_null(pi)

    cxml_assert__true(cxml_drop_pi(pi))

    char *expected = "<fruit>\n"
                     "  <class>\n"
                     "    basic\n"
                     "    <?pi data?>\n"
                     "  </class>\n"
                     "  <?pi data2?>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__two(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_drop_pi(NULL))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_delete_dtd(){
    deb()
    // <!DOCTYPE people_list SYSTEM "example.dtd"><start>testing</start>
    cxml_root_node *root = cxml_load_string(wf_xml_dtd);
    cxml_assert__not_null(root)
    cxml_assert__two(cxml_list_size(&root->children))

    cxml_dtd_node *dtd = cxml_get_dtd_node(root);
    cxml_assert__not_null(dtd)

    cxml_assert__true(cxml_delete_dtd(dtd))

    cxml_assert__one(cxml_list_size(&root->children))

    char *expected = "<XMLDocument>\n"
                     "  <start>\n"
                     "    testing\n"
                     "  </start>\n"
                     "</XMLDocument>";
    char *got = cxml_document_to_rstring(root);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_delete_dtd(NULL))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_drop_dtd(){
    deb()
    // <!DOCTYPE people_list SYSTEM "example.dtd"><start>testing</start>
    cxml_root_node *root = cxml_load_string(wf_xml_dtd);
    cxml_assert__not_null(root)
    cxml_assert__two(cxml_list_size(&root->children))

    cxml_dtd_node *dtd = cxml_get_dtd_node(root);
    cxml_assert__not_null(dtd)

    cxml_assert__true(cxml_drop_dtd(dtd))

    cxml_assert__one(cxml_list_size(&root->children))

    char *expected = "<XMLDocument>\n"
                     "  <start>\n"
                     "    testing\n"
                     "  </start>\n"
                     "</XMLDocument>";
    char *got = cxml_document_to_rstring(root);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_drop_dtd(NULL))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_delete_xml_hdr(){
    deb()
    // "<?xml version=\"1.0\"?><start>testing</start>"
    cxml_root_node *root = cxml_load_string(wf_xml_xhdr);
    cxml_assert__not_null(root)
    cxml_assert__two(cxml_list_size(&root->children))

    cxml_xhdr_node *xhdr = cxml_get_xml_hdr_node(root);
    cxml_assert__not_null(xhdr)

    cxml_assert__true(cxml_delete_xml_hdr(xhdr))

    cxml_assert__one(cxml_list_size(&root->children))

    char *expected = "<XMLDocument>\n"
                     "  <start>\n"
                     "    testing\n"
                     "  </start>\n"
                     "</XMLDocument>";
    char *got = cxml_document_to_rstring(root);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_delete_xml_hdr(NULL))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_drop_xml_hdr(){
    deb()
    // "<?xml version=\"1.0\"?><start>testing</start>"
    cxml_root_node *root = cxml_load_string(wf_xml_xhdr);
    cxml_assert__not_null(root)
    cxml_assert__two(cxml_list_size(&root->children))

    cxml_xhdr_node *xhdr = cxml_get_xml_hdr_node(root);
    cxml_assert__not_null(xhdr)

    cxml_assert__true(cxml_drop_xml_hdr(xhdr))

    cxml_assert__one(cxml_list_size(&root->children))

    char *expected = "<XMLDocument>\n"
                     "  <start>\n"
                     "    testing\n"
                     "  </start>\n"
                     "</XMLDocument>";
    char *got = cxml_document_to_rstring(root);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__one(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_assert__false(cxml_drop_xml_hdr(NULL))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_delete_descendants(){
    deb()
    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    cxml_root_node * root = cxml_load_string(wf_xml_7);
    cxml_assert__not_null(root)

    cxml_assert(cxml_delete_descendants(root->root_element))
    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__zero(cxml_list_size(&root->root_element->children))
    cxml_assert__false(root->root_element->has_child)
    cxml_assert__true(root->root_element->is_self_enclosing)
    FREE(got);

    cxml_assert__false(cxml_delete_descendants(false))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_drop_descendants(){
    deb()
    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    cxml_root_node * root = cxml_load_string(wf_xml_7);
    cxml_assert__not_null(root)
    cxml_assert__eq(cxml_list_size(&root->root_element->children), 3)

    cxml_list desc = new_cxml_list();
    cxml_assert__true(cxml_drop_descendants(root->root_element, &desc))
    char *expected = "<fruit/>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__zero(cxml_list_size(&root->root_element->children))
    cxml_assert__false(root->root_element->has_child)
    cxml_assert__true(root->root_element->is_self_enclosing)
    cxml_assert__eq(cxml_list_size(&desc), 3)
    cxml_list_free(&desc);
    FREE(got);

    cxml_assert__false(cxml_drop_descendants(root->root_element, NULL))
    cxml_assert__false(cxml_drop_descendants(NULL, &desc))

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_delete_parent(){
    deb()
    // "<fruit><name>apple</name><color>red<br/>blue</color><shape>roundish</shape></fruit>"
    cxml_root_node *root = cxml_load_string(wf_xml_7);
    cxml_assert__not_null(root)
    cxml_assert__one(cxml_list_size(&root->children))

    cxml_element_node *elem = cxml_find(root, "<shape>/");
    cxml_assert__not_null(elem)

    // the parent of the element `shape` is the root element.
    // Deleting this means the root node no longer has any child, since that
    // was its only child.
    cxml_assert__true(cxml_delete_parent(elem))

    char *expected = "<XMLDocument/>";
    char *got = cxml_document_to_rstring(root);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__zero(cxml_list_size(&root->children))
    cxml_assert__false(root->has_child)
    FREE(got);
    cxml_destroy(root);

    cxml_assert__false(cxml_delete_parent(NULL))

    root = cxml_load_string(wf_xml_7);
    cxml_assert__eq(cxml_list_size(&root->root_element->children), 3)
    elem = cxml_find(root, "<br>/");
    cxml_assert(cxml_delete_parent(elem))
    expected = "<fruit>\n"
               "  <name>\n"
               "    apple\n"
               "  </name>\n"
               "  <shape>\n"
               "    roundish\n"
               "  </shape>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__two(cxml_list_size(&root->root_element->children))
    FREE(got);

    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_delete_comments(){
    deb()
    // "<start><!--first comment--><begin><abc><!--maybe-->
    //  </abc><xyz>foo<!--sometimes-->bar</xyz></begin><!--first comment-->
    //  </start>"
    cxml_root_node *root = cxml_load_string(wf_xml_15);
    cxml_assert__not_null(root)

    int comment_count = 0;
    cxml_for_each(node, &root->root_element->children){
        if (cxml_get_node_type(node) == CXML_COMM_NODE) comment_count++;
    }
    // root element has two comments
    cxml_assert__two(comment_count)
    cxml_assert__true(root->root_element->has_comment)
    // non-recursive delete
    cxml_assert(cxml_delete_comments(root->root_element, false))
    cxml_assert__false(root->root_element->has_comment)
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
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    FREE(got);
    got = NULL;
    cxml_destroy(root);

    root = cxml_load_string(wf_xml_15);
    // recursive delete
    cxml_assert(cxml_delete_comments(root, true))
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
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__false(root->root_element->has_comment)
    FREE(got);

    cxml_assert__false(cxml_delete_comments(root, true))
    cxml_assert__false(cxml_delete_comments(NULL, true))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_drop_comments(){
    deb()
    // "<start><!--first comment--><begin><abc><!--maybe-->
    //  </abc><xyz>foo<!--sometimes-->bar</xyz></begin><!--first comment-->
    //  </start>"
    cxml_root_node *root = cxml_load_string(wf_xml_15);
    cxml_assert__not_null(root)

    int comment_count = 0;
    cxml_for_each(node, &root->root_element->children){
        if (cxml_get_node_type(node) == CXML_COMM_NODE) comment_count++;
    }
    // root element has two comments
    cxml_assert__two(comment_count)
    cxml_assert__true(root->root_element->has_comment)

    cxml_list comments = new_cxml_list();
    // non-recursive delete
    cxml_assert(cxml_drop_comments(root->root_element, false, &comments))
    cxml_assert__false(root->root_element->has_comment)
    cxml_assert__two(cxml_list_size(&comments))
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
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    FREE(got);
    got = NULL;
    cxml_list_free(&comments);
    cxml_destroy(root);

    root = cxml_load_string(wf_xml_15);
    // recursive delete
    cxml_assert(cxml_drop_comments(root, true, &comments))
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
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__false(root->root_element->has_comment)
    cxml_assert__eq(cxml_list_size(&comments), 4)
    cxml_list_free(&comments);
    FREE(got);

    cxml_assert__false(cxml_drop_comments(root, true, &comments))
    cxml_assert__false(cxml_drop_comments(root->root_element, true, NULL))
    cxml_assert__false(cxml_drop_comments(NULL, true, &comments))
    cxml_assert__zero(cxml_list_size(&comments))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_drop_texts(){
    deb()
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
    cxml_assert__not_null(root)

    int text_count = 0;
    cxml_for_each(node, &root->root_element->children){
        if (cxml_get_node_type(node) == CXML_TEXT_NODE) text_count++;
    }
    // root element has two comments
    cxml_assert__two(text_count)
    cxml_assert__true(root->root_element->has_text)
    cxml_list texts = new_cxml_list();

    // non-recursive delete
    cxml_assert(cxml_drop_texts(root->root_element, false, &texts))
    cxml_assert__false(root->root_element->has_text)
    cxml_assert__two(cxml_list_size(&texts))
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
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    FREE(got);
    cxml_list_free(&texts);
    cxml_destroy(root);

    root = cxml_load_string(wf_xml_16);
    // recursive delete
    cxml_assert(cxml_drop_texts(root, true, &texts))
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
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    cxml_assert__eq(cxml_list_size(&texts), 8)
    cxml_list_free(&texts);
    FREE(got);

    cxml_assert__false(cxml_drop_texts(root, true, &texts))
    cxml_assert__false(cxml_drop_texts(root->root_element, true, NULL))
    cxml_assert__false(cxml_drop_texts(NULL, true, &texts))
    cxml_assert__zero(cxml_list_size(&texts))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_delete_texts(){
    deb()
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
    cxml_assert__not_null(root)

    int text_count = 0;
    cxml_for_each(node, &root->root_element->children){
        if (cxml_get_node_type(node) == CXML_TEXT_NODE) text_count++;
    }
    // root element has two comments
    cxml_assert__two(text_count)
    cxml_assert__true(root->root_element->has_text)

    // non-recursive delete
    cxml_assert(cxml_delete_texts(root->root_element, false))
    cxml_assert__false(root->root_element->has_text)
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
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    FREE(got);
    cxml_destroy(root);

    root = cxml_load_string(wf_xml_16);
    // recursive delete
    cxml_assert(cxml_delete_texts(root, true))
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
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    FREE(got);

    cxml_assert__false(cxml_delete_texts(root, true))
    cxml_assert__false(cxml_delete_texts(root->root_element, true))
    cxml_assert__false(cxml_delete_texts(NULL, true))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_delete_pis(){
    deb()
    // <fruit><?pi data?><class>basic<?pi data?></class><?pi data2?></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_14);
    cxml_assert__not_null(root)

    int pi_count = 0;
    cxml_for_each(node, &root->root_element->children){
        if (cxml_get_node_type(node) == CXML_PI_NODE) pi_count++;
    }
    // root element has two comments
    cxml_assert__two(pi_count)
    // non-recursive delete
    cxml_assert(cxml_delete_pis(root->root_element, false))
    char *expected = "<fruit>\n"
                     "  <class>\n"
                     "    basic\n"
                     "    <?pi data?>\n"
                     "  </class>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    FREE(got);
    cxml_destroy(root);
    got = NULL;

    root = cxml_load_string(wf_xml_14);
    // recursive delete
    cxml_assert(cxml_delete_pis(root, true))
    expected = "<fruit>\n"
               "  <class>\n"
               "    basic\n"
               "  </class>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    cxml_assert__not_null(got)
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    FREE(got);

    cxml_assert__false(cxml_delete_pis(root, true))
    cxml_assert__false(cxml_delete_pis(NULL, true))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_drop_pis(){
    deb()
    // <fruit><?pi data?><class>basic<?pi data?></class><?pi data2?></fruit>
    cxml_root_node *root = cxml_load_string(wf_xml_14);
    cxml_assert__not_null(root)

    cxml_list pis = new_cxml_list();
    int pi_count = 0;
    cxml_for_each(node, &root->root_element->children){
        if (cxml_get_node_type(node) == CXML_PI_NODE) pi_count++;
    }
    // root element has two comments
    cxml_assert__two(pi_count)
    // non-recursive delete
    cxml_assert(cxml_drop_pis(root->root_element, false, &pis))
    cxml_assert__two(cxml_list_size(&pis))
    char *expected = "<fruit>\n"
                     "  <class>\n"
                     "    basic\n"
                     "    <?pi data?>\n"
                     "  </class>\n"
                     "</fruit>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    FREE(got);
    cxml_list_free(&pis);
    cxml_destroy(root);

    root = cxml_load_string(wf_xml_14);
    // recursive delete
    cxml_assert(cxml_drop_pis(root, true, &pis))
    cxml_assert__eq(cxml_list_size(&pis), 3)
    expected = "<fruit>\n"
               "  <class>\n"
               "    basic\n"
               "  </class>\n"
               "</fruit>";
    got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    FREE(got);
    cxml_list_free(&pis);

    cxml_assert__false(cxml_drop_pis(root->root_element, false, &pis))
    cxml_assert__false(cxml_drop_pis(root->root_element, false, NULL))
    cxml_assert__false(cxml_drop_pis(NULL, false, &pis))
    cxml_assert__zero(cxml_list_size(&pis))
    cxml_destroy(root);
    cxml_pass()
}

cts test_cxml_delete_prolog(){
    deb()
    // <?xml version="1.0"?><!DOCTYPE people_list SYSTEM "example.dtd"><start>testing</start>
    cxml_root_node *root = cxml_load_string(wf_xml_plg);
    cxml_assert__not_null(root)
    cxml_assert__eq(cxml_list_size(&root->children), 3)

    cxml_assert(cxml_delete_prolog(root))
    char *expected = "<start>\n"
                     "  testing\n"
                     "</start>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    // root element is the only child left
    cxml_assert__one(cxml_list_size(&root->children))
    FREE(got);

    cxml_assert__false(cxml_delete_prolog(NULL))
    cxml_destroy(root);

    cxml_pass()
}

cts test_cxml_drop_prolog(){
    deb()
    // <?xml version="1.0"?><!DOCTYPE people_list SYSTEM "example.dtd"><start>testing</start>
    cxml_root_node *root = cxml_load_string(wf_xml_plg);
    cxml_assert__not_null(root)
    cxml_assert__eq(cxml_list_size(&root->children), 3)

    cxml_list prolog = new_cxml_list();
    cxml_assert__true(cxml_drop_prolog(root, &prolog))
    cxml_assert__two(cxml_list_size(&prolog))
    char *expected = "<start>\n"
                     "  testing\n"
                     "</start>";
    char *got = cxml_element_to_rstring(root->root_element);
    cxml_assert__true(cxml_string_llraw_equals(got, expected, strlen(got), strlen(expected)))
    // root element is the only child left
    cxml_assert__one(cxml_list_size(&root->children))
    cxml_list_free(&prolog);
    FREE(got);

    cxml_assert__false(cxml_drop_prolog(root, &prolog))
    cxml_assert__false(cxml_drop_prolog(root, NULL))
    cxml_assert__false(cxml_drop_prolog(NULL, &prolog))
    cxml_destroy(root);

    cxml_pass()
}

cts test_cxml_delete_document(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_plg);
    cxml_assert__not_null(root)
    cxml_assert__true(cxml_delete_document(root))
    cxml_assert__false(cxml_delete_document(NULL))
    cxml_pass()
}

cts test_cxml_delete(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_plg);
    cxml_assert__not_null(root)
    cxml_assert__true(cxml_delete(root))
    cxml_assert__false(cxml_delete(NULL))
    cxml_pass()
}

cts test_cxml_clear(){
    deb()
    cxml_root_node *root = cxml_load_string(wf_xml_plg);
    cxml_assert__not_null(root)
    // should not seg-fault.
    cxml_clear(root);
    cxml_pass()
}

cts test_cxml_get_number(){
    cxml_attribute_node *attr = cxml_create_node(CXML_ATTR_NODE);
    cxml_assert__not_null(attr)
    cxml_assert__true(cxml_set_attribute_data(attr, "xml", "food", "12345"))
    double n = cxml_get_number(attr);
    cxml_assert__eq(n, 12345)

    cxml_text_node *text = cxml_create_node(CXML_TEXT_NODE);
    cxml_set_text_value(text, "0x100", false);
    n = cxml_get_number(text);
    cxml_assert__eq(n, 0x100)

    cxml_element_node *elem = cxml_create_node(CXML_ELEM_NODE);
    cxml_assert__zero(cxml_get_number(elem))
    cxml_assert__zero(cxml_get_number(NULL))

    cxml_free_attribute_node(attr);
    cxml_free_text_node(text);
    cxml_free_element_node(elem);
    cxml_pass()
}


void suite_cxqapi(){
    cxml_suite(cxqapi)
    {
        cxml_add_test_setup(fixture_no_fancy_printing_and_warnings)
        cxml_add_test_teardown(fixture_no_fancy_printing_and_warnings)
        cxml_add_m_test(95,
                        test_cxml_is_well_formed,
                        test_cxml_get_node_type,
                        test_cxml_get_dtd_node,
                        test_cxml_get_xml_hdr_node,
                        test_cxml_get_root_element,
                        test_cxml_find,
                        test_cxml_find_all,
                        test_cxml_find_children,
                        test_cxml_children,
                        test_cxml_next_element,
                        test_cxml_previous_element,
                        test_cxml_next_comment,
                        test_cxml_previous_comment,
                        test_cxml_next_text,
                        test_cxml_previous_text,
                        test_cxml_first_child,
                        test_cxml_find_first_child,
                        test_cxml_parent,
                        test_cxml_find_parent,
                        test_cxml_ancestors,
                        test_cxml_find_ancestors,
                        test_cxml_descendants,
                        test_cxml_find_descendants,
                        test_cxml_next_sibling,
                        test_cxml_previous_sibling,
                        test_cxml_find_next_sibling,
                        test_cxml_find_previous_sibling,
                        test_cxml_siblings,
                        test_cxml_find_siblings,
                        test_cxml_get_attribute,
                        test_cxml_find_attribute,
                        test_cxml_attributes,
                        test_cxml_find_attributes,
                        test_cxml_text_as_cxml_string,
                        test_cxml_text,
                        test_cxml_get_name,
                        test_cxml_get_bound_namespace,
                        test_cxml_get_comments,
                        test_cxml_create_node,
                        test_cxml_set_name,
                        test_cxml_add_child,
                        test_cxml_set_namespace,
                        test_cxml_add_namespace,
                        test_cxml_add_attribute,
                        test_cxml_set_attribute_value,
                        test_cxml_set_attribute_name,
                        test_cxml_set_attribute_data,
                        test_cxml_set_text_value,
                        test_cxml_set_comment_value,
                        test_cxml_set_pi_target,
                        test_cxml_set_pi_value,
                        test_cxml_set_pi_data,
                        test_cxml_set_namespace_prefix,
                        test_cxml_set_namespace_uri,
                        test_cxml_set_namespace_data,
                        test_cxml_set_root_node_name,
                        test_cxml_set_root_element,
                        test_cxml_insert_before,
                        test_cxml_insert_after,
                        test_cxml_delete_element,
                        test_cxml_drop_element,
                        test_cxml_drop_element_by_query,
                        test_cxml_delete_elements,
                        test_cxml_delete_elements_by_query,
                        test_cxml_drop_elements,
                        test_cxml_drop_elements_by_query,
                        test_cxml_delete_comment,
                        test_cxml_drop_comment,
                        test_cxml_delete_text,
                        test_cxml_drop_text,
                        test_cxml_delete_attribute,
                        test_cxml_drop_attribute,
                        test_cxml_unbound_element,
                        test_cxml_unbound_attribute,
                        test_cxml_delete_pi,
                        test_cxml_drop_pi,
                        test_cxml_delete_dtd,
                        test_cxml_drop_dtd,
                        test_cxml_delete_xml_hdr,
                        test_cxml_drop_xml_hdr,
                        test_cxml_delete_descendants,
                        test_cxml_drop_descendants,
                        test_cxml_delete_parent,
                        test_cxml_delete_comments,
                        test_cxml_drop_comments,
                        test_cxml_drop_texts,
                        test_cxml_delete_texts,
                        test_cxml_delete_pis,   //
                        test_cxml_drop_pis,     //
                        test_cxml_delete_prolog,  //
                        test_cxml_drop_prolog,
                        test_cxml_delete_document,
                        test_cxml_delete,
                        test_cxml_clear,
                        test_cxml_get_number
        )
        cxml_run_suite()
    }
}
