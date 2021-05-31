/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "xml/cxprinter.h"


/***************************
 *                         *
 * utility functions       *
 ***************************
 */

extern bool _cxml__is_identifier(char ch);

extern bool _cxml__is_alpha(char ch);

_CX_ATR_NORETURN inline static
void __transpose_err(const char* err_msg, int chars_len, const char* chars){
    cxml_error("CXML Error: Error occurred while "
               "converting nodes to text\n->\t`%.*s`\n%s",
               chars_len, chars, err_msg);
}

inline static bool _is_valid_char_reference(const char *start, int len){
    //  &#[0-9]+ | &#x[0-9a-fA-F]+
    // start is after '#'
    int c = 0;
    if (*start == 'x'){
        start++;
        c++;
        goto hex_dig;
    }
    if (c >= len || !isdigit((unsigned char)*start)) return false;
    while ((c < len) && isdigit((unsigned char)*start)) start++, c++;
    goto cmp;

    hex_dig:
    if (c >= len || !isxdigit((unsigned char)*start)) return false;
    while ((c < len) && isxdigit((unsigned char)*start)) start++, c++;

    cmp:
    if (c >= len || *start != ';') return false;
    return true;
}

inline static bool _is_valid_ent_reference(const char *start, int len){
    //  '&' Name ';'
    // start is after '&'
    int c = 0;
    if (!_cxml__is_alpha(*start) || c >= len) return false;
    while ((c < len) && _cxml__is_identifier(*start)) start++, c++;
    if (c >= len || *start != ';') return false;
    return true;
}

inline static void _remove_newline(cxml_string *str){
    if (str->_raw_chars[str->_len - 2] == '\r'
        && str->_raw_chars[str->_len - 1] == '\n')
    {
        str->_raw_chars[str->_len - 2] = '\0';
        str->_len -= 2;
    }
    else if (str->_raw_chars[str->_len - 1] == '\n'){
        str->_raw_chars[str->_len - 1] = '\0';
        str->_len--;
    }
}


void
_transpose_text_entities(
        cxml_string *str,
        cxml_string *transpose,
        int transpose_fwd,
        bool is_cdata,
        cxml_config *cfg)
{
    /*
     * transpose str containing or not containing predefined
     * entities into their recommended formats.
     *
     * params:
     *   |  str: string object/nodes containing some text
     *   |  transpose: cxml_string to store the transposed text
     *   |  transpose_fwd: flag to check the type of transformation to be performed.
     *
     * return: transposed cxml_string object
     */

    // transpose forward:
    // & -> &amp;
    // < -> &lt; ..etc
    //
    // transpose backward:
    // &amp; -> &
    // &lt;  -> < ..etc
    // cxml_string *str = text;

    // predefined entity transpose

    int _ind=0;
    if (cfg->transpose_text && transpose_fwd) {
        const char* chars = cxml_string_as_raw(str);
        int chars_len = _cxml_int_cast cxml_string_len(str);
        char ch;
        for (int i = 0; i < chars_len; i++) {
            ch = chars[i];
            // is ch among xml predefined entities?
            // this switch sets _ind to -1 if not so, else the index of
            // ch in the predefined entities string/char array.
            // this is then used to find the corresponding transformation
            // in _cxml_pred_entities_t, as well as the right length for such
            // transformation, and then forwarded to append.
            // strict_transpose transposes ALL predefined entities if true,
            // and only '&', '<', '>' if false
            if (cfg->strict_transpose){
                switch(ch){
                    case '<':   _ind = 0x00;  break;
                    case '>':   _ind = 0x01;  break;
                    case '&':   _ind = 0x02;  break;
                    case '"':   _ind = 0x03;  break;
                    case '\'':  _ind = 0x04;  break;
                    default:    _ind = -1;    break;
                }
            }else{
                switch(ch){
                    case '<':   _ind = 0x00;  break;
                    case '>':   _ind = 0x01;  break;
                    case '&':   _ind = 0x02;  break;
                    default:    _ind = -1;    break;
                }
            }

            if (_ind != -1)
            {
                if (is_cdata) goto append_transpose;

                // ensure we don't mistake any of the &amp;, &quot;,..etc for a '&'
                // since we're only matching first letters
                if (_ind == 0x02)
                {
                    // append predefined entity to transpose since it's no match
                    // i.e. transpose such '&' as it doesn't match any predefined
                    // entity ("&lt;", "&gt;", "&amp;", "&quot;", "&apos;")
                    // because it stands alone
                    if (i+1 > chars_len)  goto append_transpose; // if not, it's not a match

                    // possible character reference &#[0-9]+ | &#x[0-9a-fA-F]+
                    if  (chars[i + 1] == '#')
                    {
                        if (cfg->strict_transpose
                            && (((i + 2) >= chars_len)
                                || !_is_valid_char_reference(&chars[i + 2], chars_len - (i + 2))))
                        {
                            __transpose_err("Invalid/deformed character reference found.",
                                            chars_len, chars);
                        }
                        // escape the character found since it's invalid/deformed
                        goto append_transpose;
                    }
                    else
                    {
                        if (((i + 2) >= chars_len)
                            || !_is_valid_ent_reference(&chars[i + 2], chars_len - (i + 2)))
                        {
                            if (cfg->strict_transpose){
                                __transpose_err("Invalid/deformed entity reference found.",
                                                chars_len, chars);
                            }
                            // escape the character found since it's invalid/deformed
                            goto append_transpose;
                        }
                        // entity reference is valid
                        goto append_char;
                    }
                }
                // append transpose since a predefined entity match occurred.
                goto append_transpose;
            }
            else{
                // append char since no predefined entity match occurred.
                goto append_char;
            }
            // append ch char if we match a predefined
            // entity or character reference in text when ch is '&',
            // i.e. when ch overlaps with any of the predefined entities.
            // This can only happen when ch is '&'.
            // This means that ch is not a standalone '&' but actually part of
            // a predefined entity in text --> (&amp;, &gt;, etc.)
            append_char:
            cxml_string_append(transpose, &ch, 1);
            continue;

            // append predefined entity if ch doesn't overlap
            // with any predefined entity or character reference
            append_transpose:
            cxml_string_append(transpose, _cxml_pred_entities_t[_ind],
                               _cxml_pred_entities_t_lens[_ind]);
        }
    }
    else if (cfg->transpose_text && !transpose_fwd)
    {
        cxml_string into = new_cxml_string();
        for (int i = 0; i < _CXML_PRED_ENTITY_LEN; i++) {
            // do not modify `str`
            cxml_string_replace(str, _cxml_pred_entities_t[i], _cxml_pred_entities_ut[i], &into);
        }
        cxml_string_str_append(transpose, &into);
        cxml_string_free(&into);
    }
}

inline static void _space_indent(cxml_string* str, int level, cxml_config *cfg){
    if (level > 0){
        if (cfg->indent_space_size <= 0
           || cfg->indent_space_size > 30)
        {
            cfg->indent_space_size = 2;
        }
        // indent_space_size:  // ' ' + ' ' + ' ' = '   ' -- size of space indent
        // '   ' + '   ' + ...
        cxml_string_n_append(str, ' ', cfg->indent_space_size * level);
    }
}

static void _text(
        cxml_text_node* text,
        cxml_string* acc,
        int transpose_fwd,
        cxml_config *cfg)
{
    // transpose if the parser is configured to transpose text
    // and if entities exist
    if (cfg->transpose_text && (text->has_entity || text->is_cdata)){
        _transpose_text_entities(&text->value, acc, transpose_fwd, text->is_cdata, cfg);
    }else{ // else normal append
        cxml_string_str_append(acc, &text->value);
    }
}


/***************************
 *                         *
 * Node printing functions *
 ***************************
 */

/*
 * helpers
 */
static void _cxml_print_attrs(
        cxml_string *str_acc,
        cxml_table *attributes,
        cxml_list *namespaces,
        cxml_config *cfg);

// helper function for printing closing elem/root node tags
static void _elem_ctag_print_hlpr(
        cxml_string* str_acc,
        cxml_elem_node* elem,
        cxml_root_node* root,
        const int* level,
        cxml_config *cfg)

{
    _space_indent(str_acc, *level, cfg);
    cxml_string_append(str_acc, "</", 2);
    cxml_string_str_append(str_acc, (elem ? &elem->name.qname : &root->name));
    cxml_string_append(str_acc, ">\n", 2);
}


static void _cxml_xhdr_print_hlpr(
        cxml_string* str_acc,
        cxml_xhdr_node* node,
        cxml_config *cfg)
{
    cxml_string_append(str_acc, "<?", 2); // <?
    cxml_string_append(str_acc, _cxml_xml_name, 3);
    _cxml_print_attrs(str_acc, &node->attributes, NULL, cfg);
    cxml_string_append(str_acc, "?>", 2);
}

static void _cxml_pi_print_hlpr(
        cxml_string* str_acc,
        cxml_pi_node* node)
{
    cxml_string_append(str_acc, "<?", 2); // <?
    cxml_string_str_append(str_acc, &node->target);
    if (cxml_string_len(&node->value))
    {
        cxml_string_append(str_acc, " ", 1); //
        cxml_string_str_append(str_acc, &node->value);
    }
    cxml_string_append(str_acc, "?>", 2); //
}


/*
 * printing functions
 */
static void _cxml_print_attrs(
        cxml_string* str_acc,
        cxml_table* attributes,
        cxml_list *namespaces,
        cxml_config *cfg)
{
    if (!attributes && !namespaces) return;
    cxml_attr_node *attr_node;
    cxml_ns_node *ns_node;

    if (!attributes){
        cxml_for_each(ns, namespaces)
        {
            ns_node = ns;
            // we do not display global namespaces (for now).
            if (ns_node->is_global) continue;
            cxml_string_append(str_acc, " ", 1);
            if (ns_node->is_default) {
                // xmlns="http://foo/bar" | key->`xmlns`
                cxml_string_append(str_acc, _CXML_RESERVED_NS_PREFIX_XMLNS,
                                   _CXML_RESERVED_NS_PREFIX_XMLNS_LEN);
            } else {
                cxml_string_append(str_acc, "xmlns:", 6);
                cxml_string_str_append(str_acc, &ns_node->prefix);
            }
            cxml_string_append(str_acc, "=", 1); // =
            cxml_string_append(str_acc, "\"", 1);
            cxml_string_str_append(str_acc, &ns_node->uri);  // 'uri'
            cxml_string_append(str_acc, "\"", 1);
        }
        return;
    }

    int len = cxml_table_size(attributes) + cxml_list_size(namespaces);
    void **arr = ALLOC(void *, len);
    int i = 0, j;

    cxml_for_each(k, &attributes->keys){
        arr[i++] = cxml_table_get(attributes, k);
    }

    if (!namespaces){
        qsort(arr, len, sizeof(void *), _cxml_cmp_node);
        for (j = 0; j < i; j++) {
            attr_node = arr[j];
            cxml_string_append(str_acc, " ", 1);
            cxml_string_str_append(str_acc, &attr_node->name.qname);  // attr1
            cxml_string_append(str_acc, "=", 1); // =
            cxml_string_append(str_acc, "\"", 1);
            if (cfg->strict_transpose) {
                _transpose_text_entities(&attr_node->value, str_acc, true, false, cfg);
            } else {
                cxml_string_str_append(str_acc, &attr_node->value);  // 'value'
            }
            cxml_string_append(str_acc, "\"", 1);
        }
        FREE(arr);
        return;
    }

    cxml_for_each(ns, namespaces){
        arr[i++] = ns;
    }
    qsort(arr, len, sizeof(void *), _cxml_cmp_node);

    for (j = 0; j < i; j++){
        if (_cxml_node_type(arr[j]) == CXML_ATTR_NODE){
            attr_node = arr[j];
            cxml_string_append(str_acc, " ", 1);
            cxml_string_str_append(str_acc, &attr_node->name.qname);  // attr1
            cxml_string_append(str_acc, "=", 1); // =
            cxml_string_append(str_acc, "\"", 1);
            if (cfg->strict_transpose){
                _transpose_text_entities(&attr_node->value, str_acc, true, false, cfg);
            }else{
                cxml_string_str_append(str_acc, &attr_node->value);  // 'value'
            }
            cxml_string_append(str_acc, "\"", 1);
        }else{
            ns_node = arr[j];
            // we do not display global namespaces.
            if (ns_node->is_global) continue;
            cxml_string_append(str_acc, " ", 1);
            if (ns_node->is_default){
                // xmlns="http://foo/bar" | key->`xmlns`
                cxml_string_append(str_acc, _CXML_RESERVED_NS_PREFIX_XMLNS,
                                   _CXML_RESERVED_NS_PREFIX_XMLNS_LEN);
            }else{
                cxml_string_append(str_acc, "xmlns:", 6);
                cxml_string_str_append(str_acc, &ns_node->prefix);
            }
            cxml_string_append(str_acc, "=", 1); // =
            cxml_string_append(str_acc, "\"", 1);
            cxml_string_str_append(str_acc, &ns_node->uri);  // 'uri'
            cxml_string_append(str_acc, "\"", 1);
        }
    }
    FREE(arr);
}

static void _cxml_print_elem_otag(
        cxml_string* str_acc,
        cxml_elem_node* node,
        const int* level,
        cxml_config *cfg)
{
    _space_indent(str_acc, *level, cfg);
    cxml_string_append(str_acc, "<", 1); // <
    cxml_string_str_append(str_acc, &node->name.qname);
    if (node->has_attribute || node->namespaces){
        _cxml_print_attrs(str_acc, node->attributes, node->namespaces, cfg);
    }
    if (!(node->is_self_enclosing)){
        cxml_string_append(str_acc, ">\n", 2);
    }
}

static void _cxml_print_elem_ctag(
        cxml_string* str_acc,
        cxml_elem_node* node,
        const int* level,
        cxml_config *cfg)
{
    if (node->is_self_enclosing){
        cxml_string_append(str_acc,  "/>\n", 3);
    }else{
        _elem_ctag_print_hlpr(str_acc, node, NULL, level, cfg);
    }
}

static void _cxml_print_xhdr(
        cxml_string* str_acc,
        cxml_xhdr_node* node,
        const int* level,
        cxml_config *cfg)
{
    _space_indent(str_acc, (*level + 1), cfg);
    _cxml_xhdr_print_hlpr(str_acc, node, cfg);
    cxml_string_append(str_acc, "\n", 1);
}

static void _cxml_print_doc_otag(
        cxml_string* str_acc,
        cxml_root_node* node,
        int* level,
        cxml_config *cfg)
{
    // should <XMLDocument></XMLDocument> be shown as the top level element
    // node containing all other nodes (root element, dtd, header, etc.)?
    if (cfg->show_doc_as_top_level){
        _space_indent(str_acc, *level, cfg);
        cxml_string_append(str_acc, "<", 1);  // <
        cxml_string_str_append(str_acc, &node->name);
        cxml_string_append(str_acc, ">\n", 2);  // >
    }else{
        (*level)--;
    }
}

static void _cxml_print_doc_ctag(
        cxml_string* str_acc,
        cxml_root_node* node,
        const int* level,
        cxml_config *cfg)
{
    if (cfg->show_doc_as_top_level){
        _elem_ctag_print_hlpr(str_acc, NULL, node, level, cfg);
    }
}

static void _cxml_print_pi(
        cxml_string* str_acc,
        cxml_pi_node* node,
        const int* level,
        cxml_config *cfg)
{
    _space_indent(str_acc, (*level + 1), cfg);
    _cxml_pi_print_hlpr(str_acc, node);
    cxml_string_append(str_acc, "\n", 1);
}

static void _cxml_print_comm(
        cxml_string* str_acc,
        cxml_comm_node* node,
        const int* level,
        cxml_config *cfg)
{
    if (level){ _space_indent(str_acc, (*level + 1), cfg); }
    cxml_string_append(str_acc, "<!--", 4);  // opening
    cxml_string_str_append(str_acc, &node->value);
    cxml_string_append(str_acc, "-->", 3);  // closing
    cxml_string_append(str_acc, "\n", 1);
}

static void _cxml_print_dtd(
        cxml_string* str_acc,
        cxml_dtd_node* node,
        const int* level,
        cxml_config *cfg)
{
    if (level){ _space_indent(str_acc, (*level + 1), cfg); }
    cxml_string_str_append(str_acc, &node->value);
    cxml_string_append(str_acc, "\n", 1);
}

static void _cxml_print_text(
        cxml_string* str_acc,
        cxml_text_node* text_node,
        const int* level,
        cxml_config *cfg)
{

    // temporarily hold text_node->value object
    cxml_string tmp = text_node->value;

    // strip text, and assign the new cxml_string to text_node temporarily
    cxml_string stripped = new_cxml_string();
    cxml_string_strip_space(&text_node->value, &stripped);
    text_node->value = stripped;

    // check if cxml_string returned is emtpy, if so, skip appending anything.
    bool is_empty = !text_node->value._len;
    if (is_empty) goto free_label;

    // add space indent for justification/alignment with nodes before it
    _space_indent(str_acc, (*level+1), cfg);

    if (text_node->is_cdata){
        cxml_string_append(str_acc, "<![CDATA[", 9);
        _text(text_node, str_acc, 1, cfg);
        cxml_string_append(str_acc, "]]>", 3);
    }else{
        // transpose forward when printing element to string, and forward
        // when printing element to file (to ensure that xml doc is well-defined)
        _text(text_node, str_acc, 1, cfg);
    }

    free_label:
    // free the stripped cxml_string
    cxml_string_free(&text_node->value);

    // assign text_node its original value
    text_node->value = tmp;
    !is_empty ?
    cxml_string_append(str_acc, "\n", 1) : (void)0;
}


static void _cxml_print_opening(
        cxml_string* str_acc,
        void* node,
        int* level,
        cxml_config *cfg)
{
    switch(_cxml_node_type(node))
    {
        case CXML_ELEM_NODE:
            _cxml_print_elem_otag(str_acc, node, level, cfg);
            break;
        case CXML_PI_NODE:
            _cxml_print_pi(str_acc, node, level, cfg);
            break;
        case CXML_ROOT_NODE:
            _cxml_print_doc_otag(str_acc, node, level, cfg);
            break;
        case CXML_XHDR_NODE:
            _cxml_print_xhdr(str_acc, node, level, cfg);
            break;
        default: break;
    }
}


static void _cxml_print_closing(
        cxml_string* str_acc,
        void* node,
        int* level,
        cxml_config *cfg)
{
    switch(_cxml_node_type(node))
    {
        case CXML_ELEM_NODE:
            _cxml_print_elem_ctag(str_acc, node, level, cfg);
            break;
        case CXML_ROOT_NODE:
            _cxml_print_doc_ctag(str_acc, node, level, cfg);
            break;
        default: break;
    }
}



/****************************
 *                          *
 * Node to string functions *
 ****************************
 */


/*
 * <stuff>
 *      <child1>
 *          text
 *      </child1>
 *      <child2>
 *          text
 *      </child2>
 * </stuff>
 */

static void _cxml_node_tostring(
        cxml_string* str_acc,
        void* node,
        int* level,
        cxml_config *cfg)
{
    _cxml_print_opening(str_acc, node, level, cfg);

    if (_cxml_node_type(node) == CXML_ROOT_NODE || _cxml_node_type(node) == CXML_ELEM_NODE)
    {
        cxml_for_each(child, _cxml__get_node_children(node))
        {
            switch(_cxml_node_type(child))
            {
                case CXML_ELEM_NODE:
                {
                    ++(*level);
                    _cxml_node_tostring(str_acc, child, level, cfg);
                    --(*level);
                    break;
                }
                case CXML_DTD_NODE:
                    _cxml_print_dtd(str_acc, child, level, cfg);
                    break;
                case CXML_COMM_NODE:
                    _cxml_print_comm(str_acc, child, level, cfg);
                    break;
                case CXML_TEXT_NODE:
                    _cxml_print_text(str_acc, child, level, cfg);
                    break;
                case CXML_PI_NODE:
                    _cxml_print_pi(str_acc, child, level, cfg);
                    break;
                case CXML_XHDR_NODE:
                    _cxml_print_xhdr(str_acc, child, level, cfg);
                    break;
                default:
                    break;
            }
        }
        _cxml_print_closing(str_acc, node, level, cfg);
    }
}


void cxml_element_to_string(cxml_element_node* node, cxml_string *str){
    // <stuff>...</stuff>
    if (!node || !str) return;
    cxml_config cfg = cxml_get_config();
    (cfg.print_fancy) ? cxml_string_append(str, "[Element]='\n", 12) : (void)0;
    int level = 0;
    _cxml_node_tostring(str, node, &level, &cfg);
    _remove_newline(str);
    cfg.print_fancy ? cxml_string_append(str, "'", 1) : (void)0;
}

char *cxml_element_to_rstring(cxml_element_node *node){
    cxml_string str = new_cxml_string();
    cxml_element_to_string(node, &str);
    return cxml_string_as_raw(&str);
}

void cxml_attribute_to_string(cxml_attribute_node *attr_node, cxml_string *str){
    // name="value"
    if (!attr_node || !str) return;
    cxml_config cfg = cxml_get_config();
    cfg.print_fancy ? cxml_string_append(str, "[Attribute]='", 13) : (void)0;
    cxml_string_str_append(str, &attr_node->name.qname);
    cxml_string_append(str, "=", 1);
    cxml_string_append(str, "\"", 1);
    if (cfg.strict_transpose){
        _transpose_text_entities(&attr_node->value, str, true, false, &cfg);
    }else{
        cxml_string_str_append(str, &attr_node->value);
    }
    cxml_string_append(str, "\"", 1);
    cfg.print_fancy ? cxml_string_append(str, "'", 1) : (void)0;
}

char* cxml_attribute_to_rstring(cxml_attribute_node* attr_node){
    // name="value"
    cxml_string tmp = new_cxml_string();
    cxml_attribute_to_string(attr_node, &tmp);
    return cxml_string_as_raw(&tmp);
}

void cxml_namespace_to_string(cxml_namespace_node* ns_node, cxml_string *str){
    // xmlns:pref="value"
    if (!ns_node || !str) return;
    cxml_config cfg = cxml_get_config();
    cfg.print_fancy ? cxml_string_append(str, "[Namespace]='", 13) : (void)0;
    if (!ns_node->is_default){
        cxml_string_str_append(str,  &ns_node->prefix);
    }else{
        cxml_string_append(str,  _cxml_xmlns_name, _CXML_RESERVED_NS_URI_XMLNS_LEN);
    }
    cxml_string_append(str, "=", 1);
    cxml_string_append(str, "\"", 1);
    cxml_string_str_append(str, &ns_node->uri);
    cxml_string_append(str, "\"", 1);
    cfg.print_fancy ? cxml_string_append(str, "'", 1) : (void)0;
}

char* cxml_namespace_to_rstring(cxml_namespace_node* ns_node){
    // xmlns:pref="value"
    cxml_string tmp = new_cxml_string();
    cxml_namespace_to_string(ns_node, &tmp);
    return cxml_string_as_raw(&tmp);
}

void cxml_comment_to_string(cxml_comment_node* comm_node, cxml_string *str){
    // <!-- a comment -->
    if (!comm_node || !str) return;
    cxml_config cfg = cxml_get_config();
    cfg.print_fancy ? cxml_string_append(str, "[Comment]='", 11) : (void)0;
    cxml_string_append(str, "<!--", 4);  // opening
    cxml_string_str_append(str, &comm_node->value);
    cxml_string_append(str, "-->", 3);  // closing
    cfg.print_fancy ? cxml_string_append(str, "'", 1) : (void)0;
}

char* cxml_comment_to_rstring(cxml_comment_node* comm_node){
    // <!-- a comment -->
    cxml_string tmp = new_cxml_string();
    cxml_comment_to_string(comm_node, &tmp);
    return cxml_string_as_raw(&tmp);
}

void cxml_dtd_to_string(cxml_dtd_node* dtd_node, cxml_string *str){
    // dtd
    if (!dtd_node || !str) return;
    cxml_config cfg = cxml_get_config();
    cfg.print_fancy ? cxml_string_append(str, "[Document-Type-Definition]='", 28) : (void)0;
    cxml_string_str_append(str, &dtd_node->value);
    cfg.print_fancy ? cxml_string_append(str, "'", 1) : (void)0;
}

char* cxml_dtd_to_rstring(cxml_dtd_node* dtd_node){
    // dtd
    cxml_string tmp = new_cxml_string();
    cxml_dtd_to_string(dtd_node, &tmp);
    return cxml_string_as_raw(&tmp);
}

void cxml_text_to_string(cxml_text_node* text_node, cxml_string *str){
    // ...some text...
    if (!text_node || !str) return;
    cxml_config cfg = cxml_get_config();
    cfg.print_fancy ? cxml_string_append(str, "[Text]='", 8) : (void)0;
    // transpose backward when printing element to string
    _text(text_node, str, 0, &cfg);
    cfg.print_fancy ? cxml_string_append(str, "'", 1) : (void)0;
}

char* cxml_text_to_rstring(cxml_text_node* text_node){
    // ...some text...
    cxml_string tmp = new_cxml_string();
    cxml_text_to_string(text_node, &tmp);
    return cxml_string_as_raw(&tmp);
}

void cxml_pi_to_string(cxml_pi_node* pi, cxml_string *str){
    if (!pi || !str) return;
    cxml_config cfg = cxml_get_config();
    cfg.print_fancy ?
    cxml_string_append(str, "[Processing-Instruction]='", 26) : (void)0;
    _cxml_pi_print_hlpr(str, pi);
    cfg.print_fancy ? cxml_string_append(str, "'", 1) : (void)0;
}

char* cxml_pi_to_rstring(cxml_pi_node* pi){
    cxml_string tmp = new_cxml_string();
    cxml_pi_to_string(pi, &tmp);
    return cxml_string_as_raw(&tmp);
}

void cxml_xhdr_to_string(cxml_xhdr_node* hdr, cxml_string *str){
    if (!hdr || !str) return;
    cxml_config cfg = cxml_get_config();
    cfg.print_fancy ?
    cxml_string_append(str, "[XMLDeclaration]='\n", 19) : (void)0;
    _cxml_xhdr_print_hlpr(str, hdr, &cfg);
    cfg.print_fancy ? cxml_string_append(str, "'", 1) : (void)0;
}

char* cxml_xhdr_to_rstring(cxml_xhdr_node* hdr){
    cxml_string tmp = new_cxml_string();
    cxml_xhdr_to_string(hdr, &tmp);
    return cxml_string_as_raw(&tmp);
}

void cxml_document_to_string(cxml_root_node *root, cxml_string *str){
    if (!root || !str) return;
    cxml_config cfg = cxml_get_config();
    cfg.print_fancy ? cxml_string_append(str, "[Document]='\n", 13) : (void)0;
    int level = 0;
    if (root->has_child){
        _cxml_node_tostring(str, root, &level, &cfg);
        _remove_newline(str);
    }else{
        cxml_string_append(str, "<", 1);
        cxml_string_str_append(str, &root->name);
        cxml_string_append(str, "/>", 2);
    }
    cfg.print_fancy ? cxml_string_append(str, "'", 1) : (void)0;
}

char* cxml_document_to_rstring(cxml_root_node *root){
    cxml_string str = new_cxml_string();
    cxml_document_to_string(root, &str);
    return cxml_string_as_raw(&str);
}

void cxml_node_to_string(void* node, cxml_string *str){
    // dispatcher for _cxml_obj node and other node types - to cxml_string.
    switch(_cxml_get_node_type(node))
    {
        case CXML_PI_NODE:        cxml_pi_to_string(node, str);         break;
        case CXML_NS_NODE:        cxml_namespace_to_string(node, str);  break;
        case CXML_DTD_NODE:       cxml_dtd_to_string(node, str);        break;
        case CXML_ROOT_NODE:      cxml_document_to_string(node, str);   break;
        case CXML_ELEM_NODE:      cxml_element_to_string(node, str);    break;
        case CXML_COMM_NODE:      cxml_comment_to_string(node, str);    break;
        case CXML_TEXT_NODE:      cxml_text_to_string(node, str);       break;
        case CXML_ATTR_NODE:      cxml_attribute_to_string(node, str);  break;
        case CXML_XHDR_NODE:      cxml_xhdr_to_string(node, str);       break;
        default:                                                        return;
    }
}

char* cxml_node_to_rstring(void* node){
    // dispatcher for _cxml_obj node and other node types - to char*.
    switch(_cxml_get_node_type(node))
    {
        case CXML_PI_NODE:        return cxml_pi_to_rstring(node);
        case CXML_NS_NODE:        return cxml_namespace_to_rstring(node);
        case CXML_DTD_NODE:       return cxml_dtd_to_rstring(node);
        case CXML_ROOT_NODE:      return cxml_document_to_rstring(node);
        case CXML_ELEM_NODE:      return cxml_element_to_rstring(node);
        case CXML_COMM_NODE:      return cxml_comment_to_rstring(node);
        case CXML_TEXT_NODE:      return cxml_text_to_rstring(node);
        case CXML_ATTR_NODE:      return cxml_attribute_to_rstring(node);
        case CXML_XHDR_NODE:      return cxml_xhdr_to_rstring(node);
        default:                  break;
    }
    return NULL;
}


char* cxml_prettify(void * node){
    return cxml_node_to_rstring(node);
}

char* cxml_stringify(void *node){
    return cxml_node_to_rstring(node);
}



/***************************
 *                         *
 * Node to file functions  *
 ***************************
 */


void cxml_element_to_file(
        cxml_elem_node *elem,
        const char* file_name)
{
    if (!elem || !file_name) return;
    cxml_string v = new_cxml_string();
    cxml_config cfg = cxml_get_config();
    int level = 0;
    _cxml_node_tostring(&v, elem, &level, &cfg);
    _remove_newline(&v);
    _cxml_write_file(file_name, cxml_string_as_raw(&v), cxml_string_len(&v));
    cxml_string_free(&v);
}

void cxml_comment_to_file(
        cxml_comm_node *comment,
        const char* file_name)
{
    if (!comment || !file_name) return;
    cxml_string v = new_cxml_string();
    cxml_string_append(&v, "<!--", 4);  // opening
    cxml_string_str_append(&v, &comment->value);
    cxml_string_append(&v, "-->", 3);  // closing
    _cxml_write_file(file_name, cxml_string_as_raw(&v), cxml_string_len(&v));
    cxml_string_free(&v);
}

void cxml_pi_to_file(
        cxml_pi_node *pi,
        const char* file_name)
{
    if (!pi || !file_name) return;
    cxml_string v = new_cxml_string();
    _cxml_pi_print_hlpr(&v, pi);
    _cxml_write_file(file_name, cxml_string_as_raw(&v), cxml_string_len(&v));
    cxml_string_free(&v);
}

void cxml_text_to_file(
        cxml_text_node *text,
        const char* file_name)
{
    if (!text || !file_name) return;
    cxml_string v = new_cxml_string();
    cxml_config cfg = cxml_get_config();
    if (text->is_cdata){
        cxml_string_append(&v, "<![CDATA[", 9);
        // transpose forward when printing element to file
        // (to ensure that xml doc is well-defined)
        _text(text, &v, 1, &cfg);
        cxml_string_append(&v, "]]>", 3);
    }else{
        // transpose forward when printing element to file
        // (to ensure that xml doc is well-defined)
        _text(text, &v, 1, &cfg);
    }
    _cxml_write_file(file_name, cxml_string_as_raw(&v), cxml_string_len(&v));
    cxml_string_free(&v);
}

void cxml_dtd_to_file(
        cxml_dtd_node *dtd,
        const char* file_name)
{
    if (!dtd || !file_name) return;
    cxml_string v = new_cxml_string();
    cxml_string_str_append(&v, &dtd->value);
    _cxml_write_file(file_name, cxml_string_as_raw(&v), cxml_string_len(&v));
    cxml_string_free(&v);
}

void cxml_xhdr_to_file(
        cxml_xhdr_node *xml_hdr,
        const char* file_name)
{
    if (!xml_hdr || !file_name) return;
    cxml_string v = new_cxml_string();
    cxml_config cfg = cxml_get_config();
    _cxml_xhdr_print_hlpr(&v, xml_hdr, &cfg);
    _cxml_write_file(file_name, cxml_string_as_raw(&v), cxml_string_len(&v));
    cxml_string_free(&v);
}

void cxml_document_to_file(
        cxml_root_node *root,
        const char* file_name)
{
    if (!root || !file_name) return;
    cxml_string v = new_cxml_string();
    cxml_config cfg = cxml_get_config();
    int level = 0;
    _cxml_node_tostring(&v, root, &level, &cfg);
    _remove_newline(&v);
    _cxml_write_file(file_name, cxml_string_as_raw(&v), cxml_string_len(&v));
    cxml_string_free(&v);
}

void cxml_node_to_file(void *node, const char* file_name){
    // print self-contained nodes to file.
    switch(_cxml_get_node_type(node))
    {
        case CXML_PI_NODE:        cxml_pi_to_file(node, file_name);         break;
        case CXML_DTD_NODE:       cxml_dtd_to_file(node, file_name);        break;
        case CXML_ROOT_NODE:      cxml_document_to_file(node, file_name);   break;
        case CXML_ELEM_NODE:      cxml_element_to_file(node, file_name);    break;
        case CXML_COMM_NODE:      cxml_comment_to_file(node, file_name);    break;
        case CXML_TEXT_NODE:      cxml_text_to_file(node, file_name);       break;
        case CXML_XHDR_NODE:      cxml_xhdr_to_file(node, file_name);       break;
        default:                                                            break;
    }
}
