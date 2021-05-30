/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include <cxml/cxml.h>

/*
 * foo.xml
 *
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE bar SYSTEM "example.dtd">
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

// we're streaming the xml file containing the document highlighted in the comment above

void collecting_dtd_node(){
    // create a reader object:
    // here, we would like to cleanup the event reader ourselves.
    // this is imperative because we're only in search of a single event,
    // and once it's found we exit the loop, as seen below.
    // Even if auto_close is set to 'true', the reader would only be cleaned up
    // when the CXML_SAX_END_DOCUMENT_EVENT is encountered, which won't hold in this case.
    cxml_sax_event_reader reader = cxml_stream_file("foo.xml", false); // auto_close set to false

    // event object for storing the current event
    cxml_sax_event_t event;

    while (cxml_sax_has_event(&reader))
    {
        // get the current/available event
        event = cxml_sax_get_event(&reader);

        // xml header (<?xml version="1.0" encoding="UTF-8"?>)
        //
        // cxml_dtd_node *dtd
        // some useful fields:
        // dtd->value     -   a `cxml_string` object containing the entire dtd string
        //                    (for example: <!DOCTYPE note SYSTEM "example.dtd">)
        // dtd->parent    -   parent of this node which is always the root node

        // currently, when an xml header is found, if needed, it must be consumed
        // directly as a node.
        if (event ==  CXML_SAX_DTD_EVENT)
        {
            // this consumes the CXML_SAX_DTD_EVENT, returning a dtd node
            cxml_dtd_node *dtd = cxml_sax_as_dtd_node(&reader);
            // stringify the header node
            char *str = cxml_dtd_to_rstring(dtd);
            printf("%s\n", str);
            free(str);

            // the `parent` field will always be NULL when a dtd node is consumed,
            // because it's originally a pointer to the document's root node, which
            // isn't consumed until the document is streamed completely (when auto_close is set to true),
            // or cxml_sax_close_event_reader() is called on the reader.

            // or simply print it directly
            // as noted in using_query_pt1.c, cxml_string_as_raw() returns the underlying
            // char * pointer in cxml_string, and mustn't be freed directly, except you
            // really know what you're doing. In all cases, to free a cxml_string object,
            // call `cxml_string_free()` on the object.
            printf("%s\n", cxml_string_as_raw(&dtd->value));

            // `dtd` is now our responsibility, we must free it after we're done with it.
            // cxml_destroy(dtd) or cxml_dtd_node_free(dtd) can also be used in freeing the node
            // freeing the dtd node automatically frees its internal state, including its `value`.
            cxml_free_dtd_node(dtd);
            break;
        }

    }
    // close the event reader because we didn't wait until CXML_SAX_END_DOCUMENT_EVENT
    // was encountered, and auto_close was set to false.
    cxml_sax_close_event_reader(&reader);
}

void collecting_xml_header_node(){
    // create a reader object:
    // here, we would like to cleanup the event reader ourselves.
    // this is imperative because we're only in search of a single event,
    // and once it's found we exit the loop, as seen below.
    // Even if auto_close is set to 'true', the reader would only be cleaned up
    // when the CXML_SAX_END_DOCUMENT_EVENT is encountered, which won't hold in this case.
    cxml_sax_event_reader reader = cxml_stream_file("foo.xml", false);

    // event object for storing the current event
    cxml_sax_event_t event;

    while (cxml_sax_has_event(&reader))
    {
        // get the current/available event
        event = cxml_sax_get_event(&reader);

        // xml header (<?xml version="1.0" encoding="UTF-8"?>)
        //
        // cxml_xhdr_node *header
        // some useful fields:
        // header->attributes     -   attributes found in the header represented as a cxml_table object
        //                            (for example: version="1.0" encoding="UTF-8")
        // header->parent         -   parent of this node which is always the root node

        // currently, when an xml header is found, if needed, it must be consumed
        // directly as a node.
        if (event ==  CXML_SAX_XML_HEADER_EVENT)
        {
            // this consumes the CXML_SAX_XML_HEADER_EVENT, returning an xml header node
            cxml_xhdr_node *header = cxml_sax_as_xml_hdr_node(&reader);
            // stringify the header node
            char *str = cxml_xhdr_to_rstring(header);
            printf("%s\n", str);
            free(str);

            // attributes in header
            // header->attributes is a `cxml_table` object.
            cxml_attribute_node *attr = cxml_table_get(&header->attributes, "version"); // version="1.0"
            str = cxml_attribute_to_rstring(attr);
            printf("%s\n", str);
            free(str);

            attr = cxml_table_get(&header->attributes, "encoding"); // encoding="UTF-8"
            str = cxml_attribute_to_rstring(attr);
            printf("%s\n", str);
            free(str);

            // the `parent` field will always be NULL when a xml header node is consumed,
            // because it's originally a pointer to the document's root node, which
            // isn't consumed until the document is streamed completely (when auto_close is set to true),
            // or cxml_sax_close_event_reader() is called on the reader.

            // `header` is now our responsibility, we must free it after we're done with it.
            // cxml_destroy(header) or cxml_xhdr_node_free(header) can also be used in freeing the node
            // freeing the header node automatically frees all attributes in the node.
            cxml_free_xhdr_node(header);
            break;
        }
    }
    // close the event reader because we didn't wait until CXML_SAX_END_DOCUMENT_EVENT
    // was encountered, and auto_close was set to false.
    cxml_sax_close_event_reader(&reader);
}
