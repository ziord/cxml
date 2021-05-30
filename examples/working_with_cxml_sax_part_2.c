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
    <?bean  some alternatives?>
</bar>
 *
 */

// we're streaming the xml file containing the document highlighted in the comment above

void collecting_comments(){
    // create a reader object:
    cxml_sax_event_reader reader = cxml_stream_file("foo.xml", true); // auto_close set to true

    // event object for storing the current event
    cxml_sax_event_t event;

    // there are 2 ways of obtaining comments:
    //1* obtaining the values directly
    //2* obtaining the comment node (cxml_comment_node) directly

    while (cxml_sax_has_event(&reader))
    {
        // get the current/available event
        if (cxml_sax_get_event(&reader) ==  CXML_SAX_COMMENT_EVENT)
        {
            // this consumes the CXML_SAX_COMMENT_EVENT, returning a comment node
            /*
             * we can obtain the comment node directly using cxml_sax_as_comment_node()
             * or simply the comment value, using cxml_sax_get_comment_data() as seen in
             * collecting_elements() in using_sax_pt1.c.
             * Here, we demonstrate the other approach;
             */
            cxml_comment_node *comment = cxml_sax_as_comment_node(&reader);
            // we can simply print out the comment's value since we own the comment node,
            // instead of calling cxml_comment_to_rstring()

            // stringify the comment node
            printf("Comment: `%s`\n", cxml_string_as_raw(&comment->value));  // comment->value is a cxml_string object.


            // `comment` is now our responsibility, we must free it after we're done with it.
            // cxml_destroy(comment) or cxml_comment_node_free(comment) can also be used in freeing the node.
            // freeing the comment node also automatically frees its internal state, including its `value`.
            cxml_free_comment_node(comment);
        }
    }
}


void collecting_processing_instructions(){
    // create a reader object:
    cxml_sax_event_reader reader = cxml_stream_file("foo.xml", true); // auto_close set to true

    // event object for storing the current event
    cxml_sax_event_t event;

    // there are 2 ways of obtaining processing instructions:
    //1* obtaining the value directly if the target is known beforehand
    //2* obtaining the node (cxml_pi_node) directly

    // <?boot some other irrelevant data later?>
    //  |---| |-------------------------------|
    //    |                 |
    // target           string-value
    //
    // cxml_pi_node *pi;
    // some useful fields
    // pi->target           -  target of this processing instruction (represented as a cxml_string object)
    // pi->value            -  string value of this processing instruction (represented as a cxml_string object)
    // pi->parent           -  parent of this processing instruction (could be an element or the root node)
    while (cxml_sax_has_event(&reader))
    {
        // get the current/available event
        if (cxml_sax_get_event(&reader) ==  CXML_SAX_PROCESSING_INSTRUCTION_EVENT)
        {
            // this consumes the CXML_SAX_COMMENT_EVENT, returning a comment node
            /*
             * we can obtain the processing instruction node directly using cxml_sax_as_pi_node()
             * or simply the string-value, using cxml_sax_get_pi_data() if the target is known beforehand
             *
             * Here, we demonstrate both approaches;
             */

            /*
             *
             * First approach:
             cxml_string str = new_cxml_string();
             // this obtains the string-value of the processing instruction node, and stores it in `str`
             // note that the processing instruction's target is also passed as well.
             cxml_sax_get_pi_data(&reader, "bean", &str);
             printf("PI: `%s`\n", cxml_string_as_raw(&str));
             cxml_string_free(&str);
             */

            // Second approach:
            // this obtains the processing-instruction node directly.
            cxml_pi_node *pi = cxml_sax_as_pi_node(&reader);

            // stringify the pi node
            char *str = cxml_pi_to_rstring(pi);
            printf("PI: `%s`\n", str);
            free(str);

            // `pi` is now our responsibility, we must free it after we're done with it.
            // cxml_destroy(pi) or cxml_pi_node_free(pi) can also be used in freeing the node.
            // freeing the pi node also automatically frees its internal state, including its `value`.
            cxml_free_pi_node(pi);
        }
    }
}
