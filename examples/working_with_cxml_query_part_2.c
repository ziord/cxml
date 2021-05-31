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

void element_selection_with_attribute_condition(){
    // Here, we want to select an element using an attribute condition.

    // as usual we load the document
    cxml_root_node *root = cxml_load_file("foo.xml", false);

    char *str = NULL;

    // we want to select the 'ball' element that has an attribute
    cxml_element_node *element = cxml_find(root, "<ball>/@a/");
    // stringify
    str = cxml_element_to_rstring(element);
    printf("%s\n", str);
    free(str);

    // now that we've got ball, we can do anything we need with it.
    //
    // some useful operations later..
    // we can also find an element using another flavour of the attribute condition
    // this selects the same ball element
    element = cxml_find(root, "<ball>/a='abcd'/");
    // stringify
    str = cxml_element_to_rstring(element);
    printf("%s\n", str);
    free(str);

    // this selects the same ball element
    element = cxml_find(root, "<ball>/a|='cd'/");
    // stringify
    str = cxml_element_to_rstring(element);
    printf("%s\n", str);
    free(str);

    // notice that in the three ways we accessed the same element above, we do not free the element,
    // this is because we're dealing with the same element, which is embedded in `root` and
    // will be freed automatically when the root node is destroyed/freed.

    // selection can begin from an element
    // this selects the same root element 'bar'
    element = cxml_find(root, "<bar>/");
    // stringify
    str = cxml_element_to_rstring(element);
    printf("%s\n", str);
    free(str);


    // if there are multiple elements with the same name, satisfying the same condition,
    // cxml_find() selects only the first element in document order.
    // use cxml_find_all() to select multiple elements.
    // selects the same ball element
    element = cxml_find(element, "<ball>/@a/");
    // stringify
    str = cxml_element_to_rstring(element);
    printf("%s\n", str);
    free(str);
    cxml_destroy(root);
}


void element_selection_with_text_condition(){
    // Here, we want to select an element using a text condition.

    // as usual we load the document
    cxml_root_node *root = cxml_load_file("foo.xml", false);

    // we want to select the 'book' element that has some text
    // <book>Bye for now</book>
    cxml_element_node *element = cxml_find(root, "<book>/$text/");
    // both selects the same element
    // cxml_find(root, "<book>/$text='Bye for now'")
    // cxml_find(root, "<book>/$text|='for'")

    // stringify
    cxml_string str = new_cxml_string();

    // using cxml_element_to_string(), pass in a cxml_string object to store the string repr
    // this is different from cxml_element_to_rstring(), which returns a char *
    cxml_element_to_string(element, &str);
    // print the string
    printf("%s\n", cxml_string_as_raw(&str));
    // free the string
    cxml_string_free(&str);
    cxml_destroy(root);
}


void element_selection_with_comment_condition(){
    // Here, we want to select an element using a text condition.

    // as usual we load the document
    cxml_root_node *root = cxml_load_file("foo.xml", false);

    // we want to select the 'book' element that has a comment
    // <book><!--We love beautiful books!--></book>
    cxml_element_node *element = cxml_find(root, "<book>/#comment/");
    // both selects the same element
    // cxml_find(root, "<book>/#comment='We love beautiful books!'")
    // cxml_find(root, "<book>/#comment|='love'")

    cxml_string str = new_cxml_string();
    // using cxml_element_to_string()
    cxml_element_to_string(element, &str);
    // print the string
    printf("%s\n", cxml_string_as_raw(&str));
    // free the string
    cxml_string_free(&str);

    cxml_destroy(root);
}
