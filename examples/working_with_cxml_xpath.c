/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include <cxml/cxml.h>
#include <assert.h>

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

void working_with_xpath(){
    // Given the xml document above (foo.xml),
    // we want to select an element without any constraint.

    // parse the file, and obtain the root node.
    // the _root node_ not to be mistaken with the root element (in this case 'bar')
    // is also known as the document node which is a "virtual" node that
    // houses all other nodes (elements, comments, etc  including the root element) found
    // in an xml document.
    cxml_root_node *root = cxml_load_file("foo.xml", false);

    // the xpath API exposes a handy function for running xpath queries against cxml nodes
    // the cxml_xpath() function evaluates an xpath expression against a given a root node, or an element node,
    // and returns a nodeset (a set) containing the evaluated result.
    // The nodeset can be empty, indicating that no node was found, the nodeset returned also has to
    // be freed by the user. The nodeset is a `cxml_set` object, and can be freed using `cxml_set_free()`
    // which frees the internals of the set.
    // However, since the set itself is allocated, we have to call free() to free the set afterward.
    cxml_set *nodeset = cxml_xpath(root, "//ball[position()=1]");

    // we can access the items in a `cxml_set` object in two ways, via iteration, or direct access.
    // via direct access:
    // we do this because we're certain that the xpath expression when evaluated
    // against the foo.xml document, yields only one element node.
    cxml_element_node *element = cxml_set_get(nodeset, 0);
    cxml_element_node *confirm = NULL;

    // accessing the same element via iteration
    cxml_for(node, &nodeset->items){  // `items` contains the items stored in the set.
        confirm = node;
    }
    assert(element==confirm);
    // if you're pretty comfortable with xpath, it should be easy to go from here.
    cxml_destroy(root);
}
