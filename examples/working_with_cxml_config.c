/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include <cxml/cxml.h>

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


void working_with_configs(){

    // Some configs
    // we don't want spaces around texts
    // this affects the actual nodes (and their values) contained in the root node
    // that'll be generated by the parser, not the stringified version/representation of the nodes.
    cxml_cfg_preserve_space(0);

    // keep comments within the document
    cxml_cfg_preserve_comment(1);

    // discard cdata texts
    cxml_cfg_preserve_cdata(0);

    // do not trim the DTD structure. Keep its original structure intact.
    cxml_cfg_trim_dtd(0);

    // forgive our namespace declarations if they're not unique.
    cxml_cfg_allow_duplicate_namespaces(1);

    // we don't want to see debug traces
    cxml_cfg_enable_debugging(0);

    // The configuration functions used above needs to be called before
    // parsing the xml file or string.
    cxml_root_node *root = cxml_load_file("foo.xml", false);


    // the configuration functions below should be called before
    // converting a node to a string.

    // allow node levels within the document to be very visible
    cxml_cfg_set_indent_space_size(4);

    // allows entities in the text to be transposed (where applicable)
    // in a non-strict manner.
    cxml_cfg_set_text_transposition(1, 0);

    //
    // <XMLDocument>            <- we don't want to see this when we convert the root node to a string
    //      <my_element>
    //          some text later..
    //      </my_element>
    // </XMLDocument>           <- we don't want to see this when we convert the root node to a string
    cxml_cfg_show_doc_as_top_level(0);

    //
    // [Element]='              <- We don't want things like this attached to our nodes
    //   <an_element/>'
    cxml_cfg_enable_fancy_printing(0);

    // Change the document/root node's name.
    // Setting this is however meaningless since we already disabled root node's name from being attached
    // to it's string representation (see cxml_cfg_show_doc_as_top_level(0) above) but I leave this here for the
    // sake of example.
    cxml_cfg_set_doc_name("Documento!");
    char *str = cxml_document_to_rstring(root);
    printf("%s\n", str);
    free(str);
    cxml_destroy(root);
}

void resetting_config(){
    // cxml uses a global config variable.
    // to reset the config to its default state use this:
    cxml_reset_config();

    // to get the defaults:
    cxml_config cfg = cxml_get_config_defaults();

    // set config using a cxml_config struct:
    cxml_set_config(cfg);
}

