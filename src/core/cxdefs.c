/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "core/cxdefs.h"

// transpose_text = 1 : & -> &amp;  transpose forward
// transpose_text = 0 : &amp; -> &  transpose backward/reverse

// predefined entities "untransposed" version
const char *_cxml_pred_entities_ut[] = {"<", ">", "&", "\"", "'"};

// predefined entities transposed version
const char *_cxml_pred_entities_t[] = {"&lt;", "&gt;", "&amp;", "&quot;", "&apos;"};

// constant names
const char *_cxml_xml_name = "xml";
const char *_cxml_xmlns_name = "xmlns";

// length or each predefined entity (transposed form)
int _cxml_pred_entities_t_lens[] = {4, 4, 5, 6, 6};

// Reserved Prefixes and Namespace Names (URIs)
const char *_cxml_reserved_prefixes[] = {"xml", "xmlns"};
const char *_cxml_reserved_namespaces[] = {"http://www.w3.org/XML/1998/namespace", "http://www.w3.org/2000/xmlns/"};

// Reserved Prefixes and Namespace Names (URIs) - Lengths
const int _cxml_reserved_prefixes_len[] = {3, 5};
const int _cxml_reserved_namespaces_len[] = {36, 29};

static void _cxml_table_attr_free(cxml_table *table, bool _FREE);



//* --impls--*

void cxml_name_init(cxml_name *name){
    name->pname = NULL;
    name->lname = NULL;
    name->pname_len = 0;
    name->lname_len = 0;
    cxml_string_init(&name->qname);
}

void cxml_elem_node_init(cxml_elem_node *elem) {
    if (!elem) return;
    elem->_type = CXML_ELEM_NODE;
    cxml_name_init(&elem->name);
    cxml_list_init(&elem->children);
    elem->attributes = NULL;
    elem->namespace = NULL;
    elem->namespaces = NULL;
    elem->is_namespaced = 0;
    elem->has_attribute = false;
    elem->is_self_enclosing = true;
    elem->has_child = false;
    elem->has_text = false;
    elem->has_comment = false;
    elem->has_parent = false;
    elem->parent = NULL;
    elem->pos = 0;
}

void cxml_root_node_init(cxml_root_node* root_node){
    if (!root_node) return;
    cxml_string_init(&root_node->name);
    cxml_list_init(&root_node->children);
    root_node->namespaces = NULL;
    root_node->root_element = NULL;
    root_node->has_child = false;
    root_node->_type = CXML_ROOT_NODE;
    root_node->pos = 0;
    root_node->is_well_formed = 0;
}

void cxml_pi_node_init(cxml_pi_node* pi){
    if (!pi) return;
    pi->_type = CXML_PI_NODE;
    cxml_string_init(&pi->target);
    cxml_string_init(&pi->value);
    pi->parent = NULL;
    pi->pos = 0;
}

void cxml_xhdr_node_init(cxml_xhdr_node* xml_hdr){
    if (!xml_hdr) return;
    xml_hdr->_type = CXML_XHDR_NODE;
    xml_hdr->parent = NULL;
    cxml_table_init(&xml_hdr->attributes);
}

void cxml_text_node_init(cxml_text_node *text) {
    if (!text) return;
    text->_type = CXML_TEXT_NODE;
    text->has_entity = false;
    cxml_string_init(&text->value);
    cxml_number_init(&text->number_value);
    text->parent = NULL;
    text->is_cdata = 0;
    text->pos = 0;
}

void cxml_attr_node_init(cxml_attr_node* cx_attr){
    if (!cx_attr) return;
    cx_attr->_type = CXML_ATTR_NODE;
    cxml_name_init(&cx_attr->name);
    cxml_string_init(&cx_attr->value);
    cxml_number_init(&cx_attr->number_value);
    cx_attr->parent = NULL;
    cx_attr->pos = 0;
    cx_attr->namespace = NULL;
}

void cxml_ns_node_init(cxml_ns_node* cx_ns){
    if (!cx_ns) return;
    cx_ns->_type = CXML_NS_NODE;
    cxml_string_init(&cx_ns->prefix);
    cxml_string_init(&cx_ns->uri);
    cx_ns->parent = NULL;
    cx_ns->is_default = 0;
    cx_ns->is_global = 0;
    cx_ns->pos = 0;
}

void cxml_comm_node_init(cxml_comm_node *cx_comment) {
    if (!cx_comment) return;
    cx_comment->_type = CXML_COMM_NODE;
    cxml_string_init(&cx_comment->value);
    cx_comment->parent = NULL;
    cx_comment->pos = 0;
}

void cxml_dtd_node_init(cxml_dtd_node* dtd){
    if (!dtd) return;
    dtd->_type = CXML_DTD_NODE;
    cxml_string_init(&dtd->value);
    dtd->parent = NULL;
}

void cxml_name_free(cxml_name *name){
    if (!name) return;
    cxml_string_free(&name->qname);
    name->pname = name->lname = NULL;
    name->pname_len = name->lname_len = 0;
}

void cxml_text_node_free(cxml_text_node *text) {
    if (!text) return;
    _cxml_dprint("FREEING - cxml text (`%.*s`)\n",
                 cxml_string_len(&text->value),
                 cxml_string_as_raw(&text->value));
    cxml_string_free(&text->value);
    // don't free parent, it would be freed automatically when
    // freeing root/document node
    FREE(text);
}

void cxml_comm_node_free(cxml_comm_node* comment){
    if (!comment) return;
    _cxml_dprint("FREEING - cxml comment")
    _cxml_dprint(" (`%s`) object - \n", cxml_string_as_raw(&comment->value))
    cxml_string_free(&comment->value);
    FREE(comment);
}

void cxml_root_node_free(cxml_root_node* doc){
    if (!doc) return;
    _cxml_dprint("FREEING - cxml document `%s`\n",
          cxml_string_len(&doc->name) ?
          cxml_string_as_raw(&doc->name) : "")
    cxml_string_free(&doc->name);
    cxml_for_each(child, &doc->children){
        cxml_node_free(child);
    }
    if (doc->namespaces){
        cxml_for_each(ns, doc->namespaces){
            cxml_ns_node_free(ns);
        }
        cxml_list_free(doc->namespaces);
        FREE(doc->namespaces);
    }
    cxml_list_free(&doc->children);
    FREE(doc);
}

void cxml_pi_node_free(cxml_pi_node* pi){
    if (!pi) return;
    _cxml_dprint("FREEING - cxml processing-instruction ")
    _cxml_dprint("target: `%s`, string value: `%s`\n",
          cxml_string_len(&pi->target) ? cxml_string_as_raw(&pi->target) : "",
          cxml_string_as_raw(&pi->value))
    cxml_string_free(&pi->target);
    cxml_string_free(&pi->value);
    FREE(pi);
}

void cxml_dtd_node_free(cxml_dtd_node *dtd){
    if (!dtd) return;
    _cxml_dprint("FREEING - cxml dtd")
    _cxml_dprint(" (%s) object - \n", cxml_string_as_raw(&dtd->value))
    cxml_string_free(&dtd->value);
    FREE(dtd);
}

void cxml_xhdr_node_free(cxml_xhdr_node *xml){
    if (!xml) return;
    _cxml_dprint("FREEING - cxml xml declaration\n")
    _cxml_table_attr_free(&xml->attributes, false);
    FREE(xml);
}


void cxml_attr_node_free(cxml_attr_node* attr){
    if (!attr) return;
    _cxml_dprint("FREEING - cxml attribute: key: `%s`, value: `%s`\n",
                 cxml_string_as_raw(&attr->name.qname),
                 cxml_string_as_raw(&attr->value))
    cxml_name_free(&attr->name);
    cxml_string_free(&attr->value);
    FREE(attr);
}

void cxml_ns_node_free(cxml_ns_node *ns){
    if (!ns) return;
    _cxml_dprint("FREEING - cxml namespace: prefix: `%s`, uri: `%s`\n",
           cxml_string_len(&ns->prefix) ? cxml_string_as_raw(&ns->prefix) : "",
           cxml_string_as_raw(&ns->uri))
    cxml_string_free(&ns->prefix);
    cxml_string_free(&ns->uri);
    FREE(ns);
}

void cxml_node_free(void *node){
    if (!node) return;
    switch(_cxml_node_type(node)){
        case CXML_TEXT_NODE:
            cxml_text_node_free(node);
            break;
        case CXML_XHDR_NODE:
            cxml_xhdr_node_free(node);
            break;
        case CXML_ELEM_NODE:
            cxml_elem_node_free(node);
            break;
        case CXML_PI_NODE:
            cxml_pi_node_free(node);
            break;
        case CXML_DTD_NODE:
            cxml_dtd_node_free(node);
            break;
        case CXML_COMM_NODE:
            cxml_comm_node_free(node);
            break;
        case CXML_ROOT_NODE:
            cxml_root_node_free(node);
            break;
        case CXML_ATTR_NODE:
            cxml_attr_node_free(node);
            break;
        case CXML_NS_NODE:
            cxml_ns_node_free(node);
            break;
        default:
            break;
    }
}

static void _cxml_table_attr_free(cxml_table *table, bool _FREE) {
    if (!table) return;
    _cxml_dprint("FREEING - cxml attribute table (size: %d) - \n",
                 cxml_table_size(table))
    // free table, by freeing embedded items (attributes), and the table itself
    for (int i = 0; i < table->capacity; i++) {
        _cxml_ht_entry *entry = &table->entries[i];
        if (entry->key != NULL) {
            cxml_attr_node_free(entry->value);
        }
    }
    cxml_table_free(table);
    _FREE ? FREE(table) : (void)0;
}

void cxml_elem_node_free(cxml_elem_node *node) {
    if (!node) return;
    _cxml_dprint("FREEING - cxml element (%.*s)\n",
                 cxml_string_len(&node->name.qname),
                 cxml_string_as_raw(&node->name.qname))
    // free name
    cxml_name_free(&node->name);

    // free attributes
    _cxml_table_attr_free(node->attributes, true);

    // free namespaces
    if (node->namespaces){
        cxml_for_each(ns, node->namespaces){
            cxml_ns_node_free(ns);
        }
        cxml_list_free(node->namespaces);
        FREE(node->namespaces);
    }

    // we don't free namespace to which node belongs because it'll be freed
    // in the element in which the namespace was defined/declared.

    // free children
    cxml_for_each(obj, &node->children) {
        cxml_node_free(obj);
    }
    cxml_list_free(&node->children);
    FREE(node);
}

/*
 * Delete/destroy any valid cxml node
 */
void cxml_destroy(void *node){
    cxml_node_free(node);
}

void (*cxml_free_element_node)(cxml_elem_node *node) = cxml_elem_node_free;

void (*cxml_free_text_node)(cxml_text_node *node) = cxml_text_node_free;

void (*cxml_free_root_node)(cxml_root_node *node) = cxml_root_node_free;

void (*cxml_free_comment_node)(cxml_comment_node *node) = cxml_comm_node_free;

void (*cxml_free_attribute_node)(cxml_attribute_node *node) = cxml_attr_node_free;

void (*cxml_free_namespace_node)(cxml_namespace_node *node) = cxml_ns_node_free;

void (*cxml_free_pi_node)(cxml_pi_node *node) = cxml_pi_node_free;

void (*cxml_free_xhdr_node)(cxml_xhdr_node *node) = cxml_xhdr_node_free;

void (*cxml_free_dtd_node)(cxml_dtd_node *node) = cxml_dtd_node_free;

void (*cxml_free_node)(void *node) = cxml_node_free;


_cxml_node_t _cxml_get_node_type(void* node){
    if (!node) return 0xff;
    _cxml_node_t type = *(_cxml_node_t *) (node);
    switch(type)
    {
        case CXML_PI_NODE:
        case CXML_NS_NODE:
        case CXML_DTD_NODE:
        case CXML_TEXT_NODE:
        case CXML_ELEM_NODE:
        case CXML_XHDR_NODE:
        case CXML_COMM_NODE:
        case CXML_ATTR_NODE:
        case CXML_ROOT_NODE:
                              return type;
        default:              return 0xff;
    }
}

unsigned  int _cxml_get_node_pos(void *node) {
    switch (_cxml_node_type(node))
    {
        case CXML_TEXT_NODE:
            return _unwrap_cxtext_node(node)->pos;
        case CXML_ELEM_NODE:
            return _unwrap_cxelem_node(node)->pos;
        case CXML_COMM_NODE:
            return _unwrap_cxcomm_node(node)->pos;
        case CXML_ATTR_NODE:
            return _unwrap_cxattr_node(node)->pos;
        case CXML_ROOT_NODE:
            return _unwrap_cxroot_node(node)->pos;
        case CXML_PI_NODE:
            return _unwrap_cxpi_node(node)->pos;
        case CXML_NS_NODE:
            return _unwrap_cxns_node(node)->pos;
        default: return 0;
    }
}

void* _cxml_get_node_parent(void *node){
    if (!node) return NULL;
    switch (_cxml_get_node_type(node))
    {
        case CXML_ELEM_NODE:
            return _unwrap_cxelem_node(node)->parent;
        case CXML_TEXT_NODE:
            return _unwrap_cxtext_node(node)->parent;
        case CXML_COMM_NODE:
            return _unwrap_cxcomm_node(node)->parent;
        case CXML_ATTR_NODE:
            return _unwrap_cxattr_node(node)->parent;
        case CXML_PI_NODE:
            return _unwrap_cxpi_node(node)->parent;
        case CXML_XHDR_NODE:
            return _unwrap_cxxhdr_node(node)->parent;
        case CXML_DTD_NODE:
            return _unwrap_cxdtd_node(node)->parent;
        case CXML_NS_NODE:
            return _unwrap_cxns_node(node)->parent;
        default:
            return NULL;
    }
}

void* _cxml_node_parent(void *node){
    switch (_cxml_node_type(node))
    {
        case CXML_ELEM_NODE:
            return _unwrap_cxelem_node(node)->parent;
        case CXML_TEXT_NODE:
            return _unwrap_cxtext_node(node)->parent;
        case CXML_COMM_NODE:
            return _unwrap_cxcomm_node(node)->parent;
        case CXML_ATTR_NODE:
            return _unwrap_cxattr_node(node)->parent;
        case CXML_PI_NODE:
            return _unwrap_cxpi_node(node)->parent;
        case CXML_XHDR_NODE:
            return _unwrap_cxxhdr_node(node)->parent;
        case CXML_DTD_NODE:
            return _unwrap_cxdtd_node(node)->parent;
        case CXML_NS_NODE:
            return _unwrap_cxns_node(node)->parent;
        default:
            return NULL;
    }
}

int _cxml_cmp_node (const void *n1, const void *n2){
    return _cxml_int_cast(_cxml_get_node_pos(*(void **)n1)
                         - _cxml_get_node_pos(*(void **)n2));
}

/*
 * Unset `parent` node as the parent of `child` node
 */
void _cxml_unset_parent(void *node){

    switch (_cxml_node_type(node))
    {
        case CXML_TEXT_NODE:
            _unwrap__cxnode(text, node)->parent = NULL;
            break;
        case CXML_ELEM_NODE:
            _unwrap__cxnode(elem, node)->parent = NULL;
            _unwrap__cxnode(elem, node)->has_parent = false;
            break;
        case CXML_COMM_NODE:
            _unwrap__cxnode(comm, node)->parent = NULL;
            break;
        case CXML_ATTR_NODE:
            _unwrap__cxnode(attr, node)->parent = NULL;
            break;
        case CXML_PI_NODE:
            _unwrap__cxnode(pi, node)->parent = NULL;
            break;
        case CXML_NS_NODE:
            _unwrap__cxnode(ns, node)->parent = NULL;
            break;
        case CXML_XHDR_NODE:
            _unwrap__cxnode(xhdr, node)->parent = NULL;
            break;
        case CXML_DTD_NODE:
            _unwrap__cxnode(dtd, node)->parent = NULL;
            break;
        default: return;
    }
}


/***
 * utility function definitions for unwrapping nodes
 * ***/

#define _cx_unwrap_def(nd)                                          \
cxml_##nd##_node*  _unwrap_cx##nd##_node(cxml_##nd##_node* nd){     \
            return ((cxml_##nd##_node*)nd);                         \
}

_cx_unwrap_def(elem)

_cx_unwrap_def(attr)

_cx_unwrap_def(text)

_cx_unwrap_def(comm)

_cx_unwrap_def(root)

_cx_unwrap_def(pi)

_cx_unwrap_def(xhdr)

_cx_unwrap_def(dtd)

_cx_unwrap_def(ns)

#undef  _cx_unwrap_def

/**************************/
