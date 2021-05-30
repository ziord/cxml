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

// we can begin pulling events.
// the SAX API provides several events that can be checked, listed below:
//
//*  CXML_SAX_ATTRIBUTE_EVENT               <- returned when only attributes are encountered
//*  CXML_SAX_NAMESPACE_EVENT               <- returned when only namespaces are encountered
//*  CXML_SAX_NAMESPACE_ATTRIBUTE_EVENT     <- returned when attributes and namespaces are encountered
//*  CXML_SAX_CDATA_EVENT                   <- returned when a CData is encountered
//*  CXML_SAX_COMMENT_EVENT                 <- returned when a comment is encountered
//*  CXML_SAX_DTD_EVENT                     <- returned when a dtd structure is encountered
//*  CXML_SAX_PROCESSING_INSTRUCTION_EVENT  <- returned when a processing-instruction is encountered
//*  CXML_SAX_XML_HEADER_EVENT              <- returned when an xml header (<?xml ...?>) is encountered
//*  CXML_SAX_TEXT_EVENT                    <- returned when a text is encountered
//*  CXML_SAX_BEGIN_DOCUMENT_EVENT          <- returned when the beginning of a document is encountered.
//*  CXML_SAX_END_DOCUMENT_EVENT            <- returned when the end of a document is encountered.
//*  CXML_SAX_BEGIN_ELEMENT_EVENT           <- returned when the beginning of an element is encountered
//*  CXML_SAX_END_ELEMENT_EVENT             <- returned when the end of an element is encountered

void collecting_elements(){
    // we don't want trailing whitespace around text or whitespace separating
    // nodes to be regarded as text
    cxml_cfg_preserve_space(0);

    // create an event reader object:
    // 'true' (auto_close) here allows the reader to automatically close itself once all events are exhausted.
    // cxml_stream_file() returns an event reader object that can be used in pulling xml events.
    cxml_sax_event_reader reader = cxml_stream_file("foo.xml", true);


    // event object for storing the current event
    cxml_sax_event_t event;

    // To: string to store element names
    cxml_string name = new_cxml_string();

    // To: store the root element's name
    cxml_string root_name = new_cxml_string();

    // To: store collected data
    cxml_string data = new_cxml_string();


    // We want to print all items in the document, excluding the xml header.
    // see collecting_xml_header_node() below for working with xml headers.

    // cxml_sax_has_event() returns false if the end of the document is encountered.
    // events that aren't checked/consumed are automatically consumed by the reader.
    while (cxml_sax_has_event(&reader)){

        // get the current/available event - this pulls us the next available event
        event = cxml_sax_get_event(&reader);
        switch (event)
        {
            // start of the document.
            case CXML_SAX_BEGIN_DOCUMENT_EVENT:
                printf("--- Streaming started! ---\n");
                break;

                // end of the document.
            case CXML_SAX_END_DOCUMENT_EVENT:
                printf("--- Streaming ended! ---\n");
                break;

                // start of the element
            case CXML_SAX_BEGIN_ELEMENT_EVENT:
                // consume the current event by retrieving the element's name

                // cxml_string_as_raw() returns NULL, when a cxml_string object is empty
                // this helps us know we're at the start of the root element, since it must be
                // the first element encountered in an xml document.
                if (!cxml_string_as_raw(&root_name)){
                    cxml_sax_get_element_name(&reader, &root_name);
                    printf("[Root Element Start] - `%s`\n", cxml_string_as_raw(&root_name));
                }else{
                    // if root_name is not empty, then it means we're the start of other elements
                    // (descendants/children of the root element)
                    cxml_sax_get_element_name(&reader, &name);
                    printf("[Element Start] - `%s`\n", cxml_string_as_raw(&name));
                }
                break;

                // end of the element
            case CXML_SAX_END_ELEMENT_EVENT:
                if (cxml_string_as_raw(&name)){
                    printf("[Element End] - `%s`\n", cxml_string_as_raw(&name));
                    cxml_string_free(&name);
                }else{
                    printf("[Root Element End] - `%s`\n", cxml_string_as_raw(&root_name));
                    cxml_string_free(&root_name);
                }
                break;

                // an attribute is encountered
            case CXML_SAX_ATTRIBUTE_EVENT:
            {
                cxml_list attributes = new_cxml_list();
                // collect all encountered attributes in the element
                cxml_sax_as_attribute_list(&reader, &attributes);
                // iterate over the attributes and print them all
                char *str;
                cxml_for_each(attr, &attributes){
                    // attr is a cxml_attribute_node
                    // we can do whatever we need to with it, but here I choose to print it
                    str = cxml_attribute_to_rstring(attr);
                    printf("\t%s\n", str);
                    // we own this string
                    free(str);
                }
                // free/cleanup the list
                cxml_list_free(&attributes);
            }
                break;

                // a comment is encountered
            case CXML_SAX_COMMENT_EVENT:
                // get the content of the comment
                cxml_sax_get_comment_data(&reader, &data);
                printf("[Comment] - `%s`\n", cxml_string_as_raw(&data));
                cxml_string_free(&data);
                break;

                // print the text encountered
            case CXML_SAX_TEXT_EVENT:
                // get the text
                cxml_sax_get_text_data(&reader, &data);
                printf("[Text] - `%s`\n", cxml_string_as_raw(&data));
                cxml_string_free(&data);
                break;

            default:
                break;
        }
    }
    // it is important to note that if auto_close isn't set to true above, one would have to call
    // cxml_sax_close_event_reader() on reader after the streaming is done.
    // i.e. cxml_sax_close_event_reader(&reader);  to close and cleanup the reader object.
}

void collecting_texts(){
    // create a reader object:
    cxml_sax_event_reader reader = cxml_stream_file("foo.xml", true); // auto_close set to true

    // event object for storing the current event
    cxml_sax_event_t event;

    // there are 2 ways of obtaining texts:
    //1* obtaining the value directly
    //2* obtaining the text node (cxml_text_node) directly

    while (cxml_sax_has_event(&reader))
    {
        // get the current/available event
        event = cxml_sax_get_event(&reader);

        if (event ==  CXML_SAX_TEXT_EVENT)
        {
            // this consumes the CXML_SAX_TEXT_EVENT, returning a text node
            /*
             * we can obtain the text node directly using cxml_sax_as_text_node()
             * or simply the text value, using cxml_sax_get_text_data() as seen in
             * collecting_elements() above.
             * Here, we demonstrate the other approach;
             */
            cxml_text_node *text = cxml_sax_as_text_node(&reader);
            // we can simply print out the text's value since we own the text node,
            // instead of calling cxml_text_to_rstring()

            // stringify the text node
            printf("Text: `%s`\n", cxml_string_as_raw(&text->value));  // text->value is a cxml_string object.


            // `text` is now our responsibility, we must free it after we're done with it.
            // cxml_destroy(text) or cxml_text_node_free(text) can also be used in freeing the node.
            // freeing the text node also, automatically frees its internal state, including its `value`.
            cxml_free_text_node(text);
        }
    }
}

void collecting_attributes(){
    // create an event reader object
    // 'true' (auto_close) here allows the reader to automatically close itself once all events are exhausted.
    // cxml_stream_file() returns an event reader object that can be used in pulling xml events.
    cxml_sax_event_reader reader = cxml_stream_file("foo.xml", true);

    // event object for storing the current event
    cxml_sax_event_t event;

    // To: string to store element names
    cxml_string name = new_cxml_string();
    // To: store collected attribute values
    cxml_string value = new_cxml_string();
    // To: point to found attributes
    cxml_attribute_node *attribute = NULL;

    // Here, we're interested in obtaining attributes in the xml document.

    // there are 3 ways of obtaining attributes:
    //1* obtaining the values directly, when the key is known
    //2* obtaining the attribute node directly (cxml_attribute_node), when the key is known
    //3* obtaining all attribute nodes directly, all at once
    while (cxml_sax_has_event(&reader)){

        // get the current/available event - this pulls us the next available event
        event = cxml_sax_get_event(&reader);
        switch (event)
        {
            case CXML_SAX_BEGIN_ELEMENT_EVENT:
                // consume the current event by retrieving the element's name
                cxml_sax_get_element_name(&reader, &name);

                // when an attribute is encountered, we want to print it
                // specifically, these are the only elements in the documents having attributes:
                // <duck a="xyz" b="123">This is a duck element</duck>
                // <bird b="bod" c="123" d="0xabc">Such a simple looking bird</bird>
                if (cxml_sax_get_event(&reader) == CXML_SAX_ATTRIBUTE_EVENT){

                    // check for 'duck' element
                    if (cxml_string_raw_equals(&name, "duck")){
                        // get the value of the attribute 'a' in 'duck' using (1) mentioned above
                        // here, we obtain the values directly, since we know the key beforehand
                        cxml_sax_get_attribute_data(&reader, "a", &value);
                        printf("Key: `a`, Value: `%s`\n", cxml_string_as_raw(&value));
                        cxml_string_free(&value); // free this value, in prep for the next

                        // get the value of the attribute 'b' in 'duck' using (2) mentioned above
                        // here, we obtain the attribute node directly, since we know the key beforehand
                        attribute = cxml_sax_as_attribute_node(&reader, "b");
                        // access the attribute's name/key, and print. The attribute's name/key is a `cxml_name` object
                        // and its `qname` is a cxml_string.
                        // Here, we print the attribute's qname (qualified name)
                        printf("Key: `%s`, ", cxml_string_as_raw(&attribute->name.qname));
                        // access the attribute's name/key, and print.
                        // Here, we print the attribute's value (a cxml_string)
                        printf("Value: `%s`\n", cxml_string_as_raw(&attribute->value));
                        // we now own this attribute and need to free it.
                        cxml_free_attribute_node(attribute);
                    }
                        // check for 'bird' element
                    else if (cxml_string_raw_equals(&name, "bird")){
                        // here we take the third approach to obtaining attributes:
                        // obtaining all attribute nodes directly, all at once

                        // list to store all found attributes in "bird" element
                        cxml_list list = new_cxml_list();
                        // collect all attributes found in the element 'bird' into `list`
                        cxml_sax_as_attribute_list(&reader, &list);
                        char *str = NULL;
                        // iterate over the list and print all attributes present
                        cxml_for_each(attr, &list){
                            // stringify the attribute
                            str = cxml_attribute_to_rstring(attr);
                            printf("%s\n", str);
                            // we own this string, so we must free it when done.
                            free(str);
                            // this attribute is now our responsibility, hence, we must free it
                            cxml_free_attribute_node(attr);
                        }
                        // cleanup the list
                        cxml_list_free(&list);
                    }
                }
                break;

            default:
                break;
        }
    }
    // it is important to note that if auto_close isn't set to true above, one would have to call
    // cxml_sax_close_event_reader() on reader after the streaming is done.
    // i.e. cxml_sax_close_event_reader(&reader);  to close and cleanup the reader object.
}
