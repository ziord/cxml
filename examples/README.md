## Overview

The examples on utilizing the interfaces provided by cxml can be found in the source files of this directory. The source files contain trivial and simplistic examples that highlights some of the features of cxml, and illustrates a usage approach. These are only simple examples, not an exhaustive guide on the features or usage of the features of the library. 

This page highlights some information on the cxml library, which is what's available for the interim while the full library documentation is being worked on. Bear with me.


## XML and DOM Concepts

This section contains some XML concepts that should be known beforehand when dealing with XML documents, or working the cxml library.

```xml
<bar>
    <A>It's foo-bar!</A>
    <B>This is a foo element</B>
    <C>Such a simple foo-bar document</C>
</bar>
```
<sup><sub><sub>foo.xml</sub></sub></sup>
<br/>

Below is a summary of XML (and DOM) terms using the document above as a reference.

- Root Node: This is also known as the document node, and is a virtual node that "houses" all other nodes in an XML document. We cannot _see_ the root node here, because it's virtual, but yet significant. The [W3C specification](https://www.w3.org/TR/1999/REC-xpath-19991116/#root-node) defines it nicely, and this [Stack Overflow answer](https://stackoverflow.com/a/14726436) paints a clear picture of it.
<br/>

- Root Element: This is the element that houses/contains other elements in an XML document. A well-formed XML document must have only 1 root element. In the document above, this is the topmost `bar` element.
<br/>

- Well-formedness: An XML document is well-formed if it conforms to certain rules defined by the XML specification. [This resource](https://cutt.ly/Vnp6j7s) provides more details on this, as well as [this Wiki page](https://en.wikipedia.org/wiki/Well-formed_document).
<br/>

- Node Relationships: In the document above, element `bar` is the *parent* of the element `A`. Elements `A`, `B`, and `C` are all siblings, because they're all embedded in the _same parent_ element. Collectively, elements `A`, `B`, and `C` are the children of the root element `bar`.
For the element `A`, the text `It's foo-bar!` is its *child*, `bar` is the *ancestor* of the text contained in `A` (`It's foo-bar!`), while the text contained in `A` is a *descendant* of `bar`. The same applies for the element `B`, and `C`. 
Of course we don't have cousins, uncles, and aunts, etc.
<br/>


## The CXML Query Language (CXQL)

cxql is a very small, lightweight query expression language that (currently) enables only selection of elements, given one or more criteria/conditions. However, the query API provides sets of functions that enables creation, update, and deletion operations on XML documents.
 

#### Constructs

* `[]`       - optional group
* `/`        - condition separator and expression terminator.
* `=`        - equal to
* `|=`       - contains or partial equal to
* `$text`    - match any found text
* `#comment` - match any found comment
* `@`        - select an attribute using only its name (or key if you will)

**Key Notes**
* All query expressions must begin with the name of the element to be selected, for example: `<element_name>/`.

* Each of the sub-expression separated by `/` is known as a **condition/criterion** in the query expression.

* Query conditions outside a `[` and `]` are collectively known as a _rigid expression_, (excluding the element name specifier i.e. `<element_name>/`), while conditions inside a `[` and `]` are collectively known as an _optional expression_.

* All conditions in a rigid expression must be satisfied by the element to be selected, when used, for the element to be selected.

* If a rigid expression fails (that is, one of the conditions in the expression is not satisfied), then an optional expression is checked (if available), and if any of the conditions in the expression is satisfied, the element is selected, otherwise the result of the query expression is `NULL`. 
See [Query Resolution](#query-resolution) below for more information.


#### Query Rules
1. A rigid expression can precede an optional expression and vice-versa.

2. A rigid/optional expression can only come after element/tag names (within `<` and `>`) and at least one `/`.

   Reiterating on 2 above:
   * A rigid expression must come after an element/tag name or after an optional expression.
   * An optional expression must come after an element/tag name or after a rigid expression.
3. A query expression must terminate with `/`.

4. Conditions in expressions must be separated by at least one `/`.

 5. A query expression cannot contain whitespace between conditions or expressions.


Here is an example of a valid query expression which selects an element having a name `element_name`: `<element_name>/`.

#### Using Conditions in Query Expressions

Conditions can be added to query expressions to make an element selection more streamlined.
The current available conditions are discussed below:


**Attribute Conditions**

An attribute condition simply selects an element, given that the element has a particular attribute.
Attribute conditions come in three flavours:
* `@attribute_key`
This syntax denotes an attribute condition using only the attribute's key.
For example: `<element_name>/@id/` selects an element `element_name` if it has any attribute with key `id`
<br/>

* `attribute_key='attribute_value'` 
This syntax denotes an attribute condition using both the attribute's key and its value. The `=` symbolizes an "exact" match.
For example: `<element_name>/id="abc"/`
In plain words, this expression can be interpreted as: 
  > find me an element named `element_name`, that has the attribute `id="abc"`

  `<element_name>/id="abc"/class="basic"/` selects an element named `element_name`, that has the attributes `id="abc"` and `class="basic"` (in any particular order). 
  other examples: `<foobar>/name="ziord"/lib="cxml"/`
<br/>

* `attribute_key|='attribute_value'` 
This syntax denotes an attribute condition using both the attribute's key and its value, but in this case, an element is selected if it has an attribute with key `attribute_key`, which "contains" `attribute_value`. The `|=` symbolizes a "partial" match.
For example: `<element_name>/id|="abc"/` selects an element `element_name` if it has an attribute with key `id` whose value contains `abc`.
Given an element `<foo id="that abc dude"/>`, then the query expression `<foo>/id|='abc'/` selects `foo`.


**Comment Conditions**

A comment condition simply selects an element, given that the element has a particular comment.
Comment conditions also come in three flavours:

* `#comment` 
This syntax denotes a comment condition with no extra constraints, which can be used to select an element, if the element has a comment child.
For example: `<element_name>/#comment/` selects an element `element_name` if it has at least one comment child.
<br/>

* `#comment="comment value"`
This syntax denotes a comment condition with a constraint. This can be used to select an element, if the element has a comment child and the content of the comment matches/or is exactly equal to the specified value on the righthand side of the `=`.
For example: `<element_name>/#comment='foobar'/` selects an element `element_name` if the element has at least one comment child with content/string-value exactly equal to `foobar`.
<br/>

* `#comment|="comment value"`
This syntax also denotes a comment condition with a constraint.
Just like the attribute condition, this selects an element, having a comment child whose content/string-value contains the specified value on the righthand side of the `|=`.
For example: `/<element_name>/#comment|='foobar'/` selects an element `element_name` if it has at least one comment child with content/string-value containing `foobar`.
Given an element `<foo><!--this is xyz here--></foo>`, then the query expression `<foo>/#comment|='xyz'/` selects `foo`.


**Text Conditions**

A text condition simply selects an element, given that the element has a particular text. Text conditions also come in three flavours (should be pretty obvious now):

* `$text` 
This syntax denotes a text condition without any further constraint, which can be used to select an element, if the element has a text child.
For example: `<element_name>/$text/` selects an element `element_name` if it has at least one text child.
<br/>

* `$text="some text"` 
This syntax adds a further constraint to the text condition. 
For example: `<element_name>/$text="some text"/` selects an element `element_name` if it has at least one text child that has the value `some text`. 
<br/>

* `$text|="some text"`
Just like the second flavour, but allows a partial match. That is given the query expression: `<element_name>/$text|="some text"/`, an element `element_name` having a text child, whose value contains `some text` is selected.

The pattern should be very obvious by now.


#### Combining Conditions in a Query Expression

We can combine any of the conditions discussed above in a query expression `<foo>/@id/$text` selects an element that has an attribute `id`, and at least one text child.
Other example: `<note>/$text="Fish"/#comment|="stuff"/@bar/@name='Perry'/`


#### Using Conditions in Rigid and Optional Expressions

In the examples given so far, only rigid conditions have been used within expressions. It is possible to combine optional conditions in a query expression. This acts like some fail-safe, when selecting an element. That is, if the rigid conditions are not satisfied by the element to be selected, the optional conditions available are checked, and if one of the conditions matches, then the element is selected.

For example, given the xml document:
```xml
<foo a="1" b="2" c="3">some text stuff</foo>
```
the query expression:
`<foo>/#comment/` 
would fail to select `foo` above, because `foo` has no comment child.
However, if an optional condition is added like so:
`<foo>/#comment/[@b]/`
then `foo` would be selected because the optional expression (`[...]`) containing the attribute condition `@b` is satisfied by `foo`.

Similarly, the condition `<foo>/[$comment]/` selects `foo` because the comment condition is an optional expression, and is only checked when any condition of the rigid expression(s) in the query expression fails. See [Query Resolution](#query-resolution) below for more information.

Given the query expression below:

`<element_name>/rcondition1/rcondition2/rconditionN/[ocondition1/ocondition2/oconditionN]/`

we can summarize the operations described above as follows:

> find me an element with `element_name` satisfying the conditions `rcondition1`..`rconditionN`. If any of those conditions fails, check if the element satisfies any of `ocondition1`..`oconditionN` conditions, if it does, fetch me the element, if it doesn't, then skip that element.

Optional expressions can precede rigid expressions and vice-versa.
That is:
 * `<element_name>/[id="abc"]/class='basic'/`
 * `<element_name>/class='basic'/[id="abc"]/`
are both valid.
<br/>

<a name='query-resolution'></a>
#### Query Resolution

When an element name is matched, the following occurs:

- If rigid *and* optional expressions are both present in the query expression:
    * rigid expressions are checked first, if the result yields NULL
    * then the result of the query expression is NULL.
    **In details:**
      * If there are multiple rigid expressions, then a given element matches or is selected if and only if it satisfies all conditions in the rigid expressions.
      * If there are multiple optional expressions, then an element matches or is selected if at least one condition in any of the optional expressions is satisfied, given that one of the conditions in the rigid expressions present, fails (that is, it's not satisfied by the element to be selected).

- If only rigid expressions are present:
    * The conditions in the rigid expressions are checked, and if any of the conditions present fails, the element isn't selected.

- If only optional expressions are present:
    * The conditions in the optional expressions are not checked. Only the element's name is used as the selection criterion, and if it matches the element name in the query expression, the element is selected.


#### Trivials

- `/` before a `]` can be omitted
for example: 
`<note>/[name='Tom'/class='basic'/]/` is the same as `<note>/[name='Tom'/class='basic']/`
<br/>

- `/` after a `[` can be omitted
for example:
`<note>/[/name='Tom'/class='basic']/` is the same as `<note>/[name='Tom'/class='basic']/`
<br/>

- Multiple `/` are allowed in separating conditions and also in terminating query expressions.
for example: `<note>///who='the boy'//[@text]///` is valid.


**Other examples of valid expressions**
 * `<note>/[name='Tom'/class='basic']/to='Perry'/from='Miah'/`
 * `<note>/[name="Tom"/class='basic'/]/to="Perry"/from='Miah'/`
 * `<note>//to="Perry"/from='Miah'/[name="Tom"/class='basic']/`
 * `<note>/$text="Perry"/text='Miah'/[#comment="Tom"/class='basic'/]/`
 * `<note>/comment="Perry"/$text|='Miah'/[#comment|="Tom"/@class/]/`
 * `<note>/to="Perry"//[/from='Miah']//text='Miah'/`
 * `<note>/to="Perry"//@from//`
 * `<note>/$text//`
 * `<note>//#comment/`
 * `<note>/to="Perry"/from='Miah'/`
 * `<note>/[name="Tom"/class='basic'/]/`
<br/>

#### CXQL and XPATH

There are things that needs to be pointed out regarding cxql and XPATH. Although these two can be used in tandem, it is important to understand the selection methodologies employed by both.

- cxql doesn't allow selection of namespaces.

- Namespaced attribute or element selection only selects elements or matches attribute conditions by the names seen in the XML document, this means that namespaced attribute/element names are not expanded during selection, unlike XPATH which utilizes the expanded names of namespaced attributes/elements during selection.

  For example given:
  ```xml
  <foo x:a='1' x:b='2' xmlns:x="http://some-uri.com">
    <foo y:a='3' xmlns:y="http://some-uri.com"/>
  </foo>
  ``` 

  The query `<foo>/@x:a/` will select only the root element, unlike XPATH, where `//foo[@x:a]` selects both the root element and it's child element.

- For feature rich selections, it's just better to use the XPATH API.

<br/>


#### Working with Configurations

cxml provides access to configurations that can used in parsing an xml document, from setting the indentation size, when nodes are printed/converted to strings, to controlling the actual parsing of the xml document, which can be used across all available interfaces (query, sax, xpath). Please see [working with config](https://github.com/ziord/cxml/blob/master/examples/working_with_config.c) after reading through this for a concrete example.

Below are some of the configuration functions:

**Fancy Printing**

```c
cxml_cfg_enable_fancy_printing(bool enable)
```
Allows node names to be attached to their string representations. If this is enabled, an element strigified or converted to a string would look like this:

```[Element]='<an_element/>'```

And an attribute would look like this:

```[Attribute]='x:a="1"'```

This only applies when converting a node to a string, and is enabled by default.


**Indentation**

```c
cxml_cfg_set_indent_space_size(short size)
```
Allows the indentation size between different levels of nodes to be set. An example of a printed node with indentation set to 4 spaces:

```xml
<bar>
    <ball>
        It's a foo-bar!
    </ball>
</bar>
```
This only applies when converting a document/element node to a string. Indentation size is set to 2 by default.


**Document Name**

```c
cxml_cfg_set_doc_name(const char *doc_name)
```
Sets the display name for the root/document node when the document is converted to a string. The default is: `XMLDocument` i.e. 
```xml
<XMLDocument>
    <root_element>
      ..
    </root_element>
</XMLDocument>
```
This behaviour can be disabled using `cxml_cfg_show_doc_as_top_level()`. This configuration only applies when converting the root/document node to a string.


**Text Transposition**

```c
cxml_cfg_set_text_transposition(bool transpose_text, bool use_strict_mode)
```

Allows text containing entities to be transposed where applicable. E.g. from `&` -> `&amp;`, `>` -> `&gt;` or `&gt;` -> `>`. This only applies when converting a node to a string. `use_strict_mode` ensures that cxml errors when bad/broken/ill formed entities are used within text.
This configuration is enabled by default, with `use_strict_mode` disabled. It is important to note that enabling `use_strict_mode` with `transpose_text` disabled is irrelevant.


**Whitespace**

```c
cxml_cfg_preserve_space(bool preserve_space)
```
Allows whitespace separating nodes or around texts to be preserved or stripped depending on how its used. This can be used to eliminate redundant spaces that isn't necessarily important text to the user. This applies to the XML parser itself and is enabled by default. This however does not affect whitespaces separating nodes when an element or root node is converted to a string, instead it applies to the values stored within nodes themselves.


**Comments**

```c
cxml_cfg_preserve_comment(bool preserve)
```
This can be used to toggle comments on or off. If disabled, comments within an XML document would be ignored by the parser during parsing. This is enabled by default, that is comments are preserved.


**CData**

```c
cxml_cfg_preserve_cdata(bool preserve)
```
Just like the option for comments, this can be used to preserve or ignore cdata structures in an XML document.
This is enabled by default. 


**DTD**

```c
cxml_cfg_trim_dtd(bool trim_dtd)
```
Allows lengthy DTD structures to be reduced to a simplified form.
For example, when this is enabled, a DTD structure of the form: `<!DOCTYPE note SYSTEM "example.dtd">` would be reduced to `<!DOCTYPE note>`. 
This applies to the XML parser itself and is enabled by default.
NOTE: cxml has no use for the DTD structure because it is non-validating - it doesn't validate an XML document against any DTD available.


**Top Level**

```c
cxml_cfg_show_doc_as_top_level(bool show)
```
Enables the root node's name to be attached to the document's string representation. This is enabled by default.


**Warnings**

```c
cxml_cfg_show_warnings(bool show)
```
Allows warnings reported by the XML parser to be displayed when recoverable errors are encountered during parsing.
This is enabled by default.


**Debug**

```c
cxml_cfg_enable_debugging(bool enable)
```
Allows node destruction processes to be traceable for debugging purposes. This is disabled by default.


**Namespaces**

```c
cxml_cfg_allow_default_namespace(bool allow)
```
Allows default namespaces to be bound to elements within the scope of such namespaces. This is enabled by default.

```c
cxml_cfg_allow_duplicate_namespaces(bool allow)
```
Allows duplicate namespaces declared within a single element in an XML document without erring during parsing. This is disabled by default.


#### How to Use the Examples

`working_with_cxml_query_*.c` might be the best place to start. Then one can move on to `working_with_cxml_xpath.c`, `working_with_cxml_config.c`, and finally `working_with_cxml_sax_*.c`. These examples are filled with comments that explains in details the lines of code.