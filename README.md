
<p align="center">
    <p align="center">
        <img src="https://github.com/ziord/cxml/blob/master/docs/assets/logo-256.png">
    </p>
    <p align="center">
        <a href="https://en.wikipedia.org/wiki/C_(programming_language)">
            <img alt="built with C" src="https://img.shields.io/badge/built%20with-C-blue.svg?style=plastic">
        </a>
        <a href="https://github.com/ziord/cxml/blob/master/LICENSE.txt">
            <img alt="CXML License" src="https://img.shields.io/github/license/ziord/cxml?style=plastic" >
        </a>
        <a href="https://en.wikipedia.org/wiki/C_(programming_language)">
            <img alt="c 11" src="https://img.shields.io/badge/C-11-blue.svg?style=plastic">
        </a>
        <a href="https://github.com/ziord/cxml/issues" >
            <img alt="issues" src="https://img.shields.io/github/issues/ziord/cxml?style=plastic">
        </a>
        <a href="https://github.com/ziord/cxml/stargazers">
            <img alt="stars" src="https://img.shields.io/github/stars/ziord/cxml?style=plastic">
        </a>
        <a href="https://github.com/ziord/cxml/network/members">
            <img alt="forks" src="https://img.shields.io/github/forks/ziord/cxml?style=plastic">
        </a>
    </p>
</p>


<br/>

## Table of Contents
- [ Overview ](#about)
- [Quick Start](#quick-start)
- [Quick Questions](#quick-questions)
- [Tests and Examples](#tests-and-examples)
- [Documentation](#docs)
- [Installation](#installation)
- [Dependencies](#dependencies)
- [Contributing](#contributing)
- [Reporting Bugs/Requesting Features](#feature-bug-reporting)
- [Project Non-goals](#non-goals)
- [License](#license)


\
<a name='about'></a>
### Overview

cxml (_C XML Minimalistic Library_) is a powerful and flexible XML library for C with a focus on simplicity and ease of use, coupled with features that enables quick processing of XML documents. 

cxml provides a DOM, and streaming interface for interacting with XML documents. This includes XPATH (1.0) support for simple/complex operations on the DOM, a built-in, simple and intuitive query language and an API for selection/creation/deletion/update operations (which may be used as an alternative to the XPATH API or in tandem with it), and a SAX-like interface for streaming large XML documents with no callback requirement. cxml works with any XML file encoded in an ASCII compatible encoding (UTF-8 for example).

One should be able to quickly utilize the library in processing or extracting data from an XML document almost effortlessy.


Note: cxml is a non-validating XML parser library. This means that DTD structures aren't used for validating the XML document. However, cxml enforces correct use of namespaces, and general XML well-formedness.
\
<a name='quick-start'></a>
### Quick Start

Say we have an XML file named "foo.xml", containing some tags/elements:

```xml
<bar>
    <bar>It's foo-bar!</bar>
    <bar/>
    <foo>This is a foo element</foo>
    <bar>Such a simple foo-bar document</bar>
    <foo/>
    <bar>So many bars here</bar>
    <bar>Bye for now</bar> 
</bar>
```
<sup><sub><sub>foo.xml</sub></sub></sup>
<br/>

---------------------------------------------------------------
<br/>
<sup><sub>Using XPATH</sub></sup>

\
We can perform a simple XPATH operation that selects all `bar` elements that have some text child/node and also are the first (element) child of their parents (as an example). 

```c
#include <cxml/cxml.h>

int main(){
    // load/parse xml file (`false` ensures the file isn't loaded 'lazily')
    cxml_root_node *root = cxml_load_file("foo.xml", false);

    // using the xpath interface, select all bar elements.
    cxml_set *node_set = cxml_xpath(root, "//bar[text() and position()=1]");
    char *item;

    // display all selected "bar" elements
    cxml_for_each(node, &node_set->items){
        // get the string representation of the element found
        item = cxml_element_to_rstring(node);
        // we own this string, we must free.
        printf("%s\n", item);
        free(item);
    }
    // free root node
    cxml_destroy(root);
    // cleanup the set
    cxml_set_free(node_set);
    // it's allocated, so it has to be freed.
    free(node_set);

    return 0;
}

```

A large subset of XPATH 1.0 is supported. Check out [this page](https://github.com/ziord/cxml/blob/master/docs/ARCHITECTURE.md#xpath-non-supported) for non-supported XPATH features.
<br/>

---------------------------------------------------------------
<br/>
<sup><sub>Using CXQL</sub></sup>

\
Suppose we only need the first "bar" element, we can still utilize the XPATH interface, taking the first element in the node set returned. 
However, cxml ships with a built-in query language, that makes this quite easy.

Using the query language:
```c
#include <cxml/cxml.h>

int main(){
    // load/parse xml file
    cxml_root_node *root = cxml_load_file("foo.xml", false);

    // find 'bar' element
    cxml_element_node *elem = cxml_find(root, "<bar>/");

    // get the string representation of the element found
    char *str = cxml_element_to_rstring(elem);
    printf("%s\n", str);

    // we own this string, so we must free.
    free(str);

    // We destroy the entire root, which frees `elem` automatically
    cxml_destroy(root);

    return 0;
}

```
\
An example to find the first `bar` element containing text "simple":

```c
#include <cxml/cxml.h>

int main(){
    // load/parse xml file
    cxml_root_node *root = cxml_load_file("foo.xml", false);

    cxml_element_node *elem = cxml_find(root, "<bar>/$text|='simple'/");

    char *str = cxml_element_to_rstring(elem);
    printf("%s\n", str);

    free(str);

    // We destroy the entire root, which frees `elem` automatically
    cxml_destroy(root);

    return 0;
}
```
In actuality, this selects the first `bar` element, having a text (child) node, whose string-value contains "simple".
The query language ins't limited to finding only "first" elements. Check out the [documentation](#docs) for more details on this.

\
Here's a quick example that pretty prints an XML document:
```c
#include <cxml/cxml.h>

int main(){
    // load/parse xml file
    cxml_root_node *root = cxml_load_file("foo.xml", false);

    // get the "prettified" string
    char *pretty = cxml_prettify(root);
    printf("%s\n", pretty);

    // we own this string.
    free(pretty);

    // destroy root
    cxml_destroy(root);

    return 0;
}
```
<br />

---------------------------------------------------------------
<br/>
<sup><sub>Using SAX</sub></sup>

\
The SAX API may be the least convenient, but can be rewarding for very large files.

Here's an example to print every text and the name of every element found in the XML document, using the API:

```c
#include <cxml/cxml.h>

int main(){
    // create an event reader object ('true' allows the reader to close itself once all events are exhausted)
    cxml_sax_event_reader reader = cxml_stream_file("foo.xml", true);

    // event object for storing the current event
    cxml_sax_event_t event;

    // cxml string objects to store name and text
    cxml_string name = new_cxml_string();
    cxml_string text = new_cxml_string();

    while (cxml_sax_has_event(&reader)){ // while there are events available to be processed.
        // get us the current event
        event = cxml_sax_get_event(&reader);

        // check if the event type is the beginning of an element
        if (event == CXML_SAX_BEGIN_ELEMENT_EVENT)
        {
            // consume the current event by collecting the element's name
            cxml_sax_get_element_name(&reader, &name);
            printf("Element: `%s`\n", cxml_string_as_raw(&name));
            cxml_string_free(&name);
        }
        // or a text event
        else if (event == CXML_SAX_TEXT_EVENT)
        {
            // consume the current event by collecting the text data
            cxml_sax_get_text_data(&reader, &text);
            printf("Text: `%s`\n", cxml_string_as_raw(&text));
            cxml_string_free(&text);
        }
    }

    return 0;
}
```


\
<a name='quick-questions'></a>
### Quick Questions
If you have little questions that you feel isn't worth opening an issue for, use [cxml's discussions](https://github.com/ziord/cxml/discussions). 


<a name='tests-and-examples'></a>
### Tests and Examples
The [tests](https://github.com/ziord/cxml/blob/master/tests) folder contains the tests.  See the [examples](https://github.com/ziord/cxml/blob/master/examples) folder for more examples, and use cases. 


<a name='docs'></a>
### Documentation
This is still a work in progress. See the [examples](https://github.com/ziord/cxml/blob/master/examples) folder for now.


<a name='installation'></a>
### Installation

Check out the [installation guide](https://github.com/ziord/cxml/blob/master/docs/INSTALLATION.md) for information on how to install, build or use the library in your project.


<a name='dependencies'></a>
### Dependencies
cxml only depends on the C standard library. All that is needed to build the library from sources is a C11 compliant compiler.


<a name='contributing'></a>
### Contributing
Your contributions are absolutely welcome! See the [contribution guidelines](https://github.com/ziord/cxml/blob/master/docs/CONTRIBUTING.md) to learn more. You can also check out [the project architecture](https://github.com/ziord/cxml/blob/master/docs/ARCHITECTURE.md) for a high-level description of the entire project. Thanks!


<a name='feature-bug-reporting'></a>
### Reporting Bugs/Requesting Features
cxml is in its early stages, but under active development. Any bugs found can be reported by [opening an issue](https://github.com/ziord/cxml/issues) (check out the [issue template](https://github.com/ziord/cxml/blob/master/.github/ISSUE_TEMPLATE)). [Please be nice](https://github.com/ziord/cxml/blob/master/docs/CODE_OF_CONDUCT.md). Providing details for reproducibility of the bug(s) would help greatly in implementing a fix, or better still, if you have a fix, consider [contributing](https://github.com/ziord/cxml/blob/master/docs/CONTRIBUTING.md).
You can also open an issue if you have a feature request that could improve the library.


<a name='non-goals'></a>
### Project Non-goals
cxml started out as a little personal experiment, but along the line, has acquired much more features than I had initially envisioned. However, some things are/will not be in view for this project. Here are some of the non-goals:

- Contain every possible feature (DTD validation, namespace well-formedness validation, etc.)
- Be the most powerful/sophisticated XML library.
- Be the "best" XML library.

However, to take a full advantage of this library, you should have a good understanding of XML, including its dos, and dont's. 


<a name='license'></a>
### License
cxml is distributed under the [MIT License.](https://github.com/ziord/cxml/blob/master/LICENSE.txt)
