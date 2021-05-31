/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include <cxml/cxml.h>
#include <assert.h>

//
/*
 * For the query language syntax, please see the README.md of this directory.
 *
 * These examples only explore one or two selection features of the query API.
 * The API also provides features for creation, update, and deletion of nodes
 * of an XML document.
 */

/*
 * foo.xml
 *
 <bar>
    <ball>It's a foo-bar!</ball>
    <ball a="abcd">It's foo-bar!</ball>
    <duck a="xyz" b="123">This is a duck element</duck>
    <duck>This is not a duck element</duck>
    <bird b="bod" c="123" d="0xabc">Such a simple looking bird</bird>
    <ant>So many ants</ant>
    <ant>Not so many ants</ant>
    <book>Bye for now</book>
    <book><!--We love beautiful books!--></book>
</bar>
 *
 */

void element_selection_with_no_conditions(){
    // Given the xml document above (foo.xml),
    // we want to select an element without any constraint.

    // Parse the file containing xml, and obtain the root node:
    //
    // the _root node_ not to be mistaken with the root element (in this case 'bar')
    // is also known as the document node which is a "virtual" node that
    // houses all other nodes (elements, comments, etc  including the root element) found
    // in an xml document.
    // here we can call cxml_load_file() because the input is from a file.
    // However cxml_load_string(const char *str) can be used when parsing a string containing xml.
    // The second parameter indicates that we do not want the file to be parsed lazily, i.e.
    // in stream mode. In stream mode, the file is loaded into memory chunk by chunk until
    // it's completely parsed, however in non-stream mode, it's loaded into memory at once.
    // If a file is too large to fit into memory at once, it's better to use the SAX interface
    // than enabling stream mode here.
    cxml_root_node *root = cxml_load_file("foo.xml", false);

    // this selects the first 'duck' element in document order in the root node.
    // Here, the search starts from the root node. It also possible to search
    // from an element node, that is, cxml_find(element_node,  "<stuff>/");
    cxml_element_node *element = cxml_find(root, "<duck>/");

    // we can confirm by printing the element selected.
    char *str = cxml_element_to_rstring(element);
    printf("%s\n", str);

    // str is allocated, and now our responsibility
    free(str);

    // We can delete this element directly, or simply destroy the root node,
    // which automatically deletes the element.
    // NOTE:
    // Do not call any of the cxml_free_*_node()/cxml_*_node_free() functions on a node directly,
    // and then delete the node's parent, or ancestor (the container node),
    // except the node is stand-alone, else this _will_ cause double free, when
    // the parent or ancestor (container node) is later freed.
    // Instead use any of the cxml_delete_*() functions.
    cxml_delete_element(element);

    // we can free the root node in a number of ways:
    // cxml_destroy(root) | cxml_free_root_node(root) | cxml_root_node_free(root)
    // any of these functions can be used on the root node at anytime because the root node
    // is the general "container" of all other nodes, and isn't contained or embedded in any other node.
    cxml_destroy(root);
}


void working_with_elements(){

    cxml_root_node *root = cxml_load_file("foo.xml", false);

    // <bird b="bod" c="123" d="0xabc">Such a simple looking bird</bird>
    cxml_element_node *element = cxml_find(root, "<bird>/");

    // Some useful fields:
    //* element->name            - the name of this element (a cxml_name object)
    //* element->attributes      - is a pointer to the attributes of the element (represented as a `cxml_table` object)
    //* element->namespaces      - the namespaces defined in this element.
    //* element->has_child       - does this element have a child ? (bool)
    //* element->has_parent      - does this element have a parent ? (bool) (A parent can be an element or a root node).
    //* element->has_attribute   - does this element have an attribute ? (bool)
    //* element->has_comment     - does this element have a comment (child)? (bool)
    //* element->has_text        - does this element have a text (child)? (bool)
    //* element->is_self_enclosing - is this element of the form `<element />`? (if an element is self enclosing,
    //                              it has no children)
    //* element->is_namespaced   - is the element namespaced? a namespaced element is an element that is bound to a
    //                            namespace for example <x:abc>foo</x:abc>
    //* element->children        - a cxml_list object containing the children of this element
    //* element->parent          - the parent of this element

    // element->name is a `cxml_name` object.
    // we can access the `qname` (qualified name) of the element, which is a `cxml_string`
    cxml_string *name = &element->name.qname;

    // now we can display this name using cxml_string_as_raw()
    // `cxml_string_as_raw()` simply returns the underlying char * in cxml_string.
    // this SHOULD NOT be freed directly, except well, if you really know what you're doing.
    // Instead, to free a cxml_string, use the function `cxml_string_free()`.
    // However, we do not need to free the element's name, as this is handled automatically
    // when the element is deleted (cxml_delete_element()) or when the root node is destroyed.
    printf("Name: %s\n", cxml_string_as_raw(name));
    // other fields
    printf("Has attribute? %s\n", element->has_attribute ? "true" : "false");
    printf("Has child? %s\n", element->has_child ? "true" : "false");
    printf("Has comment? %s\n", element->has_comment ? "true" : "false");
    printf("Has text? %s\n", element->has_text ? "true" : "false");
    printf("Has parent? %s\n", element->has_parent ? "true" : "false");
    printf("Is namespaced? %s\n", element->is_namespaced ? "true" : "false");
    if (element->has_parent){
        // we use `cxml_node_to_rstring()` which is a generic cxml_node to string function
        // This is useful if we do not know the parent's type (since the parent of an element can be an element
        // of the root node - if the element is the root element).
        // However, in our case, we do know that the parent of 'ball' element is the element 'bar'
        // but we proceed however, for the sake of example.
        char *str = cxml_node_to_rstring(element->parent);
        printf("Parent: %s\n", str);
        free(str);
    }

    // free the root node which automatically frees the selected element
    cxml_destroy(root);
}

void working_with_attributes(){
    cxml_root_node *root = cxml_load_file("foo.xml", false);
    cxml_element_node *element = cxml_find(root, "<bird>/");

    // the element selected is: <bird b="bod" c="123" d="0xabc">Such a simple looking bird</bird>
    // let's obtain the attribute node 'c'
    // as mentioned earlier, the `attribute` field of a cxml_element_node is a `cxml_table` object.
    // we can access items stored in this table using `cxml_table_get()`
    cxml_attribute_node *attribute = cxml_table_get(element->attributes, "c");

    //* some useful fields:
    //* attribute->name          - Just like an element, an attribute's name/key is a `cxml_name` object.
    //* attribute->value         - this attribute's value (a cxml_string object)
    //* attribute->number_value  - this numerical equivalent of the attribute's value (a `cxml_number` object.)
    //* attribute->namespace     - pointer to this attribute's namespace if it's bound to one. a:foo="bar",
    //                             is an example of an attribute bound to a namespace.
    //* attribute->parent        - this attribute's parent, which is always an element node
    printf("Key: %s\n", cxml_string_as_raw(&attribute->name.qname));
    printf("Value: %s\n", cxml_string_as_raw(&attribute->value));
    // obtain the string representation of the attribute
    char *repr = cxml_attribute_to_rstring(attribute);
    printf("%s\n", repr);
    free(repr);
    // we can access the deserialized numerical equivalent directly or using cxml_get_number(attribute)
    double num = attribute->number_value.dec_val; // 123
    printf("Number value: %f\n", num);

    // attribute  d="0xabc"
    attribute = cxml_table_get(element->attributes, "d");
    printf("Key: %s\n", cxml_string_as_raw(&attribute->name.qname));
    printf("Value: %s\n", cxml_string_as_raw(&attribute->value));
    // even hex values are deserialized automatically:
    printf("Number value: %f\n", cxml_get_number(attribute)); // or attribute->number_value.dec_val
    // attribute->parent -> element 'bird'
    // obtain the string representation of the attribute's parent which is always an element.
    repr = cxml_element_to_rstring(attribute->parent);
    printf("Parent: %s\n", repr);
    free(repr);

    // free the root node which automatically frees the selected attributes
    cxml_destroy(root);
}


void working_with_texts(){
    cxml_root_node *root = cxml_load_file("foo.xml", false);
    // still using the bird element: <bird b="bod" c="123" d="0xabc">Such a simple looking bird</bird>
    cxml_element_node *element = cxml_find(root, "<bird>/");

    // some useful fields:
    // text->has_entity        - contains things like '& > <' ?
    // text->is_cdata;         - is this a cdata ? (cdata are represented as text nodes in cxml)
    // text->value             - the actual text contained in this node.
    // text->number_value      - the deserialized numerical value of the text.
    // text->parent            - this text node's parent
    cxml_text_node *text = NULL;
    // we can access the element's text child.
    // As mentioned earlier, all children of an element is stored in the `children` field.
    // the `children` is represented internally as a cxml_list.
    cxml_list *children = &element->children;
    // we can iterate over this list using cxml_for_each()
    cxml_for_each(node, children){
        // this condition is pretty irrelevant since the element 'bird' has only one child
        // which is a text. However I've proceeded to add it anyway, for the sake of example.
        if (cxml_get_node_type(node) == CXML_TEXT_NODE){
            text = node;
            break;
        }
    }
    assert(text);
    // another alternative of obtaining bird's text child, since bird has exactly 1 child,
    // which is a text node, is the following:
    text = cxml_list_get(children, 0); // obtain the first child in the list
    // obtain the text's value which is a `cxml_string` object
    cxml_string *value = &text->value;
    printf("Text: %s\n", cxml_string_as_raw(value));
    printf("Is cdata? %s\n", text->is_cdata ? "true" : "false");
    printf("Has entity? %s\n", text->has_entity ? "true" : "false");
    // Just like with attributes, we can also access this text's deserialized numerical value.
    // However, for this text it's a NaN.
    // text->number_value.type -> CXML_NUMERIC_NAN_T
    printf("Number: %f\n", text->number_value.dec_val);
    // or
    printf("Number: %f\n", cxml_get_number(text));

    // free the root node which automatically frees the selected text
    cxml_destroy(root);
}


void working_with_comments(){

    cxml_root_node *root = cxml_load_file("foo.xml", false);
    // using the book element: <book><!--We love beautiful books!--></book>
    cxml_element_node *element = cxml_find(root, "<book>/#comment/");

    // this element has exactly one child, which is a comment node.
    // Usually when an element has more than one child, we can iterate over the element's
    // children to obtain the specific child node we need, especially when we do not know the
    // exact position of the nodes.
    // However, in this case, we know the exact position of the comment child node, so we do not
    // need to iterate over the `children` field of this element in order to obtain the node.
    cxml_comment_node *comment = cxml_list_get(&element->children, 0); // obtain the first (and only) child
    assert(comment);

    // some useful fields:
    // comment->value   - This comment's value (just like a cxml_text_node)
    // comment->parent  - This comment's parent
    char *value = cxml_string_as_raw(&comment->value);
    printf("Value: %s\n", value);
    // convert comment into a string
    char *str = cxml_comment_to_rstring(comment);
    printf("%s\n", str);
    free(str);

    // comment's parent
    // a comment's parent can be an element or a root node, depending on the position of the comment.
    // in our case, the parent of this comment is the element node 'book', however, in cases where it's
    // uncertain, the generic node-to-string function can be used.
    str = cxml_node_to_rstring(comment->parent);
    printf("%s\n", str);
    free(str);

    // free the root node which automatically frees all nodes
    cxml_destroy(root);
}


void working_with_processing_instructions(cxml_pi_node *pi){
    // given a processing-instruction node `pi` <?finder some value in here?>
    // here are some useful fields:
    // pi->target    -  this node's target ('finder' in this case), which is a `cxml_string` object
    // pi->value     -  this node's string-value ('some value in here')
    // pi->parent    -  this node's parent

    printf("Target: %s\n", cxml_string_as_raw(&pi->target));
    printf("String-value: %s\n", cxml_string_as_raw(&pi->value));

    // get the node's string representation.
    char *str = cxml_pi_to_rstring(pi);
    printf("%s\n", str);
    free(str);
    // free the processing instruction node
    cxml_pi_node_free(pi);
}
