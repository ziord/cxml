/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include <cxml/cxml.h>

// For the query language syntax, please see the README.md of this directory.

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

void element_selection_using_an_optional_expression(){
    // so far, the query expressions used only consisted of rigid expressions.
    // optional expressions have been described in better detail in the readme.md
    // this just puts in proper context.

    cxml_root_node *root = cxml_load_file("foo.xml", false);
    // find element 'ant' even if the element has a comment child or not.
    cxml_find(root, "<ant>/[#comment]/");
    // find element 'ant' even if the element has some text (containing 'foo') or not.
    cxml_find(root, "<ant>/[$text|='foo']/");
    cxml_destroy(root);
}


void element_selection_using_mixed_expressions(){
    // we can combine rigid expressions and mixed expressions within a query expression

    cxml_root_node *root = cxml_load_file("foo.xml", false);
    // some examples
    cxml_find(root, "<ant>/[#comment]/$text/@xyz/");
    cxml_find(root, "<ant>/abc='f'/[#comment|=' ']/$text/@xyz/");
    cxml_destroy(root);
}

void multiple_element_selections(){
    // we can find multiple elements of the same name, given a particular criteria
    cxml_root_node *root = cxml_load_file("foo.xml", false);

    // create a list object to store the elements to be found.
    cxml_list elements = new_cxml_list();

    // here we find all 'ball' elements passing in the list in which the elements would be stored.
    cxml_find_all(root, "<ball>/", &elements);

    // we can print all elements found in the list
    char *str;
    cxml_for_each(node, &elements){
        str = cxml_element_to_rstring(node);
        printf("%s\n", str);
        free(str);
    }

    // we must free the list which stores the elements
    // this however, doesn't free the items in the list, only the internals of the list
    cxml_list_free(&elements);

    // freeing the root node frees the elements initially stored in the `elements` list
    cxml_free_root_node(root);
}
