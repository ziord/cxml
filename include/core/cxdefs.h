/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXDEFS_H
#define CXML_CXDEFS_H

#include "cxconfig.h"
#include "cxstr.h"
#include "cxlist.h"
#include "cxtable.h"
#include "cxliteral.h"
#include "cxmset.h"


/** defs **/

#define _cxml_int_cast  (int)

#define cxml__assert(test, ...) \
if (!(test)){ cxml_error(__VA_ARGS__);}

#define _CXML__TRACE(code) \
if (cxml_get_config().enable_debugging){ do {code} while(0);}

#define _cxml_dprint(...)  \
if (cxml_get_config().enable_debugging){ printf(__VA_ARGS__);}

/** reserved namespaces **/
// prefixes
#define _CXML_RESERVED_NS_PREFIX_XML         "xml"
#define _CXML_RESERVED_NS_PREFIX_XMLNS       "xmlns"
#define _CXML_RESERVED_NS_PREFIX_XML_LEN     (3)
#define _CXML_RESERVED_NS_PREFIX_XMLNS_LEN   (5)

// uris
#define _CXML_RESERVED_NS_URI_XML            "http://www.w3.org/XML/1998/namespace"
#define _CXML_RESERVED_NS_URI_XMLNS          "http://www.w3.org/2000/xmlns/"
#define _CXML_RESERVED_NS_URI_XML_LEN        (36)
#define _CXML_RESERVED_NS_URI_XMLNS_LEN      (29)

// predefined entities
#define _CXML_PRED_ENTITY    "&<>'\""
#define _CXML_PRED_ENTITY_LEN     (5)

// global config
#define CXML_GLOBAL_CONFIG _cxml_config_gb

// stack allocation limit
#define _CXML_MAX_STACK_ALLOCATABLE_SIZE     (0x4e20)

#define _unwrap_cxnode(type, nd)   ((type*)nd)

#define _unwrap__cxnode(type, nd)   ((cxml_##type##_node*)nd)

#define _cxml_node_type(__node) (*(_cxml_node_t *)__node)

#define _cxml__get_node_children(node)                      \
(_cxml_node_type(node) == CXML_ELEM_NODE) ?                 \
    &_unwrap_cxnode(cxml_elem_node, node)->children  :      \
    &_unwrap_cxnode(cxml_root_node, node)->children



// cx node types
typedef enum{
    /**core node types**/
    // text node type
    CXML_TEXT_NODE,

    // element node type
    CXML_ELEM_NODE,

    // comment node type
    CXML_COMM_NODE,

    // attribute node type
    CXML_ATTR_NODE,

    // xml root node
    CXML_ROOT_NODE,

    // xml processing instruction node type
    CXML_PI_NODE,

    // xml namespace node
    CXML_NS_NODE,

    // xml header type
    CXML_XHDR_NODE,

    // xml dtd node type
    CXML_DTD_NODE
}_cxml_node_t;


typedef struct _cx_name {
    int pname_len;
    int lname_len;
    char *pname;            // prefix name
    char *lname;            // local name
    cxml_string  qname;     // qualified name
}cxml_name;

/** core nodes **/
// namespace node
typedef struct _cx_ns_node{
    _cxml_node_t _type;
    bool is_default;        // default namespace e.g. xmlns='foo://'
    bool is_global;         // global namespaces like 'xml'
    unsigned int pos;
    cxml_string prefix;     // prefix can be empty if it's a default namespace
    cxml_string uri;        // namespace name
    void* parent;
}cxml_ns_node;

// element node
typedef struct _cx_elem_node{
    _cxml_node_t _type;         // node type
    bool has_child;
    bool has_parent;
    bool has_text;
    bool has_attribute;
    bool has_comment;
    bool is_self_enclosing;
    bool is_namespaced;         // determines whether a node is under a namespace or not
    unsigned int pos;
    cxml_list children;         // list of child nodes
    cxml_name name;             // node name
    cxml_table *attributes;     // node attributes
    cxml_ns_node *namespace;
    cxml_list *namespaces;      // namespace(s) declared in element node
    void *parent;               // node's parent (could be a cxml_elem_node or cxml_root_node)
}cxml_elem_node;

// root/document node
typedef struct _cx_doc_node{
    _cxml_node_t _type;             // node type
    unsigned int pos;
    bool has_child;
    bool is_well_formed;            // is the xml document well formed?
    cxml_list children;             // child nodes
    cxml_string name;               // node name
    cxml_elem_node *root_element;
    cxml_list *namespaces;          // store global namespaces
}cxml_root_node;

// text node
typedef struct _cx_text_node{
    _cxml_node_t _type;
    unsigned int pos;
    bool has_entity;
    bool is_cdata;
    cxml_string value;
    cxml_number number_value;
    void* parent;
}cxml_text_node;

// attribute node
typedef struct _cx_attr_node{
    _cxml_node_t _type;
    unsigned int pos;
    cxml_name name;
    cxml_string value;
    cxml_number number_value;
    cxml_ns_node *namespace;
    cxml_elem_node *parent;
}cxml_attr_node;

// comment node
typedef struct _cx_comm_node{
    _cxml_node_t _type;
    unsigned int pos;
    cxml_string value;
    void* parent;
}cxml_comm_node;

// processing-instruction node
typedef struct _cx_pi_node{
    _cxml_node_t _type;     // node type
    unsigned int pos;
    cxml_string target;     // node target
    cxml_string value;      // string value
    void* parent;
}cxml_pi_node;


/** dedicated nodes **/
// xml header
typedef struct _cx_xml_hdr_node{
    _cxml_node_t _type;
    cxml_table attributes;
    cxml_root_node* parent;
}cxml_xhdr_node;

// dtd
typedef struct _cx_dtd_node{
    _cxml_node_t _type;
    cxml_string value;
    cxml_root_node* parent;
}cxml_dtd_node;

struct _cx_obj_node{
    _cxml_node_t _type;
};

/************cxml nodes public************/

typedef cxml_elem_node       cxml_element_node;
typedef cxml_comm_node       cxml_comment_node;
typedef cxml_attr_node       cxml_attribute_node;
typedef cxml_ns_node         cxml_namespace_node;
typedef _cxml_node_t         cxml_node_t;
/**************************************/

// Reserved Prefixes
extern const char *_cxml_reserved_prefixes[];

// Reserved Namespace Names (URIs)
extern const char *_cxml_reserved_namespaces[];

extern const int _cxml_reserved_prefixes_len[];

extern const int _cxml_reserved_namespaces_len[];

// "xml"
extern const char *_cxml_xml_name;

// "xmlns"
extern const char *_cxml_xmlns_name;

// predefined entities untransposed version
extern const char* _cxml_pred_entities_ut[];

// predefined entities transposed version
extern const char* _cxml_pred_entities_t[];

// length or each predefined entity transposed version
extern int _cxml_pred_entities_t_lens[];


/***
 * utility function headers for unwrapping nodes
 * ***/

#define _cx_unwrap_proto(nd)   \
cxml_##nd##_node*  _unwrap_cx##nd##_node(cxml_##nd##_node* nd);

_cx_unwrap_proto(elem)

_cx_unwrap_proto(attr)

_cx_unwrap_proto(text)

_cx_unwrap_proto(comm)

_cx_unwrap_proto(root)

_cx_unwrap_proto(pi)

_cx_unwrap_proto(xhdr)

_cx_unwrap_proto(dtd)

_cx_unwrap_proto(ns)

#undef _cx_unwrap_proto

/**************************/

void cxml_elem_node_init(cxml_elem_node* elem);

void cxml_text_node_init(cxml_text_node* text);

void cxml_attr_node_init(cxml_attr_node* cx_attr);

void cxml_ns_node_init(cxml_ns_node* cx_ns);

void cxml_root_node_init(cxml_root_node* root_node);

void cxml_comm_node_init(cxml_comm_node* cx_comment);

void cxml_pi_node_init(cxml_pi_node* pi);

void cxml_dtd_node_init(cxml_dtd_node* dtd);

void cxml_xhdr_node_init(cxml_xhdr_node* xml_hdr);

void cxml_name_init(cxml_name *name);

void cxml_name_free(cxml_name *name);

void cxml_elem_node_free(cxml_elem_node *node);

void cxml_text_node_free(cxml_text_node *text);

void cxml_comm_node_free(cxml_comm_node* comment);

void cxml_attr_node_free(cxml_attr_node* attr);

void cxml_ns_node_free(cxml_ns_node *ns);

void cxml_root_node_free(cxml_root_node* doc);

void cxml_pi_node_free(cxml_pi_node* pi);

void cxml_node_free(void *node);

void cxml_dtd_node_free(cxml_dtd_node *dtd);

void cxml_xhdr_node_free(cxml_xhdr_node *xml);

void cxml_destroy(void *node);

extern void (*cxml_free_element_node)(cxml_elem_node *node);

extern void (*cxml_free_text_node)(cxml_text_node *node);

extern void (*cxml_free_root_node)(cxml_root_node *node);

extern void (*cxml_free_comment_node)(cxml_comment_node *node);

extern void (*cxml_free_attribute_node)(cxml_attribute_node *node);

extern void (*cxml_free_namespace_node)(cxml_namespace_node *node);

extern void (*cxml_free_pi_node)(cxml_pi_node *node);

extern void (*cxml_free_xhdr_node)(cxml_xhdr_node *node);

extern void (*cxml_free_dtd_node)(cxml_dtd_node *node);

extern void (*cxml_free_node)(void *node);

_cxml_node_t _cxml_get_node_type(void* node);

unsigned int _cxml_get_node_pos(void *node);

void* _cxml_get_node_parent(void *node);

void* _cxml_node_parent(void *node);

void _cxml_unset_parent(void *node);

int _cxml_cmp_node(const void *n1, const void *n2);

#endif //CXML_CXDEFS_H
