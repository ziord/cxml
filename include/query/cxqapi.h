/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXQAPI_H
#define CXML_CXQAPI_H

#include "xml/cxparser.h"
#include "cxql.h"

// General functions
bool cxml_is_well_formed(cxml_root_node *node);

cxml_node_t cxml_get_node_type(void *node);

cxml_dtd_node *cxml_get_dtd_node(cxml_root_node *node);

cxml_xhdr_node *cxml_get_xml_hdr_node(cxml_root_node *node);

cxml_elem_node *cxml_get_root_element(cxml_root_node *node);

double cxml_get_number(void *node);

/*
 * create, select, update, delete  (CSUD)
 * The query api allows selection of nodes anywhere in the document,
 * and also allows for updating nodes, as well as deleting them.
 */
/*********************************
 *                               *
 * selection methods/functions   *
 *********************************
 */

cxml_element_node* cxml_find(void *root, const char *query);

void cxml_find_all(void *root, const char *query, cxml_list *acc);

void cxml_find_children(void *root, const char *query, cxml_list *acc);

void cxml_children(void *node, cxml_list *acc);

cxml_element_node *cxml_next_element(cxml_element_node *node);

cxml_elem_node *cxml_previous_element(cxml_elem_node *node);

cxml_comment_node *cxml_next_comment(cxml_comment_node *node);

cxml_comm_node *cxml_previous_comment(cxml_comm_node *node);

cxml_text_node *cxml_next_text(cxml_text_node *node);

cxml_text_node *cxml_previous_text(cxml_text_node *node);

void *cxml_first_child(void *root);

void *cxml_find_first_child(void *root, const char *query);

void *cxml_parent(void *node);

void *cxml_find_parent(void *root, const char *query);

void cxml_ancestors(void *node, cxml_list *acc);

void cxml_find_ancestors(void *root, const char *query, cxml_list *acc);

void cxml_descendants(void *node, cxml_list *acc);

void cxml_find_descendants(void *root, const char *query, cxml_list *acc);

void *cxml_next_sibling(void *node);

extern void* (*cxml_next)(void *node);

void *cxml_previous_sibling(void *node);

void* (*cxml_previous)(void *node);

void *cxml_find_next_sibling(void *root, const char *query);

void *cxml_find_previous_sibling(void *root, const char *query);

void cxml_siblings(void *node, cxml_list *acc);

void cxml_find_siblings(void *root, const char *query, cxml_list *acc);

cxml_attribute_node *cxml_get_attribute(cxml_elem_node *node, const char *name_key);

cxml_attribute_node *cxml_find_attribute(void *root, const char *query, const char *name_key);

void cxml_attributes(cxml_elem_node *node, cxml_list *acc);

void cxml_find_attributes(void *root, const char *query, cxml_list *acc);

extern void (*cxml_find_attribute_list)(void *root, const char *query, cxml_list *acc);

extern void (*cxml_attribute_list)(cxml_elem_node *node, cxml_list *acc);

cxml_string *cxml_text_as_cxml_string(void *root, const char *concat);

char *cxml_text(void *root, const char *concat);

char *cxml_get_name(void *node);

cxml_namespace_node *cxml_get_bound_namespace(void *node);

void cxml_get_comments(void *root, cxml_list *acc, bool recursive);


/*********************************
 *                               *
 * creation methods/functions    *
 *********************************
 */
 /**General**/
void *cxml_create_node(cxml_node_t type);

int cxml_set_name(void *node, const char *pname, const char *lname);        // for elem, and attr

int cxml_add_child(void *parent, void *child);                              // for elem, and root

int cxml_set_namespace(void *node, cxml_namespace_node *ns);                // for elem, and attr

int cxml_add_namespace(cxml_element_node *node, cxml_namespace_node *ns);   // for elem, and attr

int cxml_add_attribute(cxml_element_node *elem, cxml_attribute_node *attr);  // for elem

int cxml_set_attribute_value(cxml_attribute_node *node, const char *value);

int cxml_set_attribute_name(cxml_attribute_node *node, const char *pname, const char *lname);

int cxml_set_attribute_data(
        cxml_attribute_node *node,
        const char *pname,
        const char *lname,
        const char *value);

int cxml_set_text_value(cxml_text_node *text, const char *value, bool is_cdata);

int cxml_set_comment_value(cxml_comment_node *comment, const char *value);

int cxml_set_pi_target(cxml_pi_node *pi, const char *target);

int cxml_set_pi_value(cxml_pi_node *pi, const char *value);

int cxml_set_pi_data(cxml_pi_node *pi, const char *target, const char *value);

int cxml_set_namespace_prefix(cxml_namespace_node *ns, const char *prefix);

int cxml_set_namespace_uri(cxml_namespace_node *ns, const char *uri);

int cxml_set_namespace_data(cxml_namespace_node *ns, const char *prefix, const char *uri);

int cxml_set_root_node_name(cxml_root_node *root, char *name);

int cxml_set_root_element(cxml_root_node *root, cxml_element_node *node);


/*********************************
 *                               *
 * update methods/functions      *
 *********************************
 */

int cxml_insert_before(void *node, void *ins);

int cxml_insert_after(void *node, void *ins);


/*********************************
 *                               *
 * delete methods/functions      *
 *********************************
 */
int cxml_delete_element(cxml_element_node *elem);

int cxml_drop_element(cxml_element_node *elem);

cxml_element_node* cxml_drop_element_by_query(void *root, const char *query);

int cxml_delete_elements(void *root);

int cxml_delete_elements_by_query(void *root, const char *query);

int cxml_drop_elements(void *root, cxml_list *acc);

int cxml_drop_elements_by_query(void *root, const char *query, cxml_list *acc);

int cxml_delete_comment(cxml_comment_node *comm);

int cxml_drop_comment(cxml_comment_node *comm);

int cxml_delete_text(cxml_text_node *text);

int cxml_drop_text(cxml_text_node *text);

int cxml_delete_attribute(cxml_attribute_node *attr);

int cxml_drop_attribute(cxml_attribute_node *attr);

int cxml_unbind_element(cxml_element_node *element);

int cxml_unbind_attribute(cxml_attribute_node *attr);

int cxml_delete_pi(cxml_pi_node *pi);

int cxml_drop_pi(cxml_pi_node *pi);

int cxml_delete_dtd(cxml_dtd_node *dtd);

int cxml_drop_dtd(cxml_dtd_node *dtd);

int cxml_delete_xml_hdr(cxml_xhdr_node *xhdr);

int cxml_drop_xml_hdr(cxml_xhdr_node *xhdr);

int cxml_delete_descendants(cxml_element_node *node);

int cxml_drop_descendants(cxml_element_node *node, cxml_list *acc);

int cxml_delete_parent(void *node);     // all, excludes root

int cxml_delete_comments(void *root, bool recursive);  // root, elem

int cxml_drop_comments(void *root, bool recursive, cxml_list *acc);  // root, elem

int cxml_delete_texts(void *root, bool recursive);

int cxml_drop_texts(void *root, bool recursive, cxml_list *acc);

int cxml_delete_pis(void *root, bool recursive);

int cxml_drop_pis(void *root, bool recursive, cxml_list *acc);

int cxml_delete_prolog(void *root);

int cxml_drop_prolog(void *root, cxml_list *acc);

int cxml_delete_document(cxml_root_node *node);

int cxml_delete(void *node);  // generic

void cxml_clear(void *node);

#endif //CXML_CXQAPI_H
