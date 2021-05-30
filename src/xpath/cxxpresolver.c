/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "xpath/cxxpresolver.h"
#include "xpath/cxxpparser.h"


struct _nodeset_cmp_table{
    void (*fn)(
            _cxml_xp_data *l,
            _cxml_xp_data *r,
            void (*_push)(_cxml_xp_data *));
};


_CX_ATR_NORETURN void _cxml_xp_eval_err(const char* cause){
    cxml_error("CXML Runtime Error: Could not evaluate the xpath expression: `%s`\n"
               "The xpath expression is syntactically valid, "
               "but its evaluation has failed at runtime.\n"
               "Possible causes: %s\n", _xpath_parser.lexer.expr, cause);
}


/*
 * Get the index mapping to each operator defined in the comparison table
 */
static int _xp_op_index(cxml_xp_op op){
    switch (op)
    {
        case CXML_XP_OP_EQ:  return 0;
        case CXML_XP_OP_NEQ: return 1;
        case CXML_XP_OP_LT:  return 2;
        case CXML_XP_OP_LEQ: return 3;
        case CXML_XP_OP_GT:  return 4;
        case CXML_XP_OP_GEQ: return 5;
        default:             return -1;
    }
}

static int _xp_data_type_index(_cxml_xp_data_t type){
    switch (type)
    {
        case CXML_XP_DATA_NODESET:  return 0;
        case CXML_XP_DATA_STRING:   return 1;
        case CXML_XP_DATA_NUMERIC:  return 2;
        case CXML_XP_DATA_BOOLEAN:  return 3;
        default:                    return -1;
    }
}


/*
 * The Algorithm as described by the xpath 1.0 specification:
 *
 * *******************  (nodesets)
 * ...Comparisons that involve node-sets are defined in terms of comparisons that
 * do not involve node-sets; this is defined uniformly for =, !=, <=, <, >= and >.
 * Second, comparisons that do not involve node-sets are defined for = and !=.
 * Third, comparisons that do not involve node-sets are defined for <=, <, >= and >.
 *
 * If both objects to be compared are node-sets, then the comparison will be true if
 * and only if there is a node in the first node-set and a node in the second node-set
 * such that the result of performing the comparison on the string-values of the two
 * nodes is true.
 *
 * If one object to be compared is a node-set and the other is a number,
 * then the comparison will be true if and only if there is a node in the node-set
 * such that the result of performing the comparison on the number to be compared
 * and on the result of converting the string-value of that node to a number using
 * the number function is true.
 *
 * If one object to be compared is a node-set and the
 * other is a string, then the comparison will be true if and only if there is a node
 * in the node-set such that the result of performing the comparison on the string-value
 * of the node and the other string is true.
 *
 * If one object to be compared is a node-set
 * and the other is a boolean, then the comparison will be true if and only if the result
 * of performing the comparison on the boolean and on the result of converting the node-set
 * to a boolean using the boolean function is true.
 *
 * *******************  (non-nodesets)
 * When neither object to be compared is a node-set and the operator is = or !=,
 * then the objects are compared by converting them to a common type as follows and then comparing them.
 *
 * If at least one object to be compared is a boolean,
 * then each object to be compared is converted to a boolean as if by applying the boolean function.
 *
 * Otherwise, if at least one object to be compared is a number,
 * then each object to be compared is converted to a number as if by applying the number function.
 *
 * Otherwise, both objects to be compared are converted to strings as if by applying the string function.
 *
 * The = comparison will be true if and only if the objects are equal;
 * the != comparison will be true if and only if the objects are not equal.
 *
 * Numbers are compared for equality according to IEEE 754 [IEEE 754].
 *
 * Two booleans are equal if either both are true or both are false.
 *
 * Two strings are equal if and only if they consist of the same sequence of UCS characters
 *
 * When neither object to be compared is a node-set and the operator is <=, <, >= or >, then the objects
 * are compared by converting both objects to numbers and comparing the numbers according to IEEE 754
 *
 *
 * ================================================================
 * In simplified terms:
 * ================================================================
 * (for non-nodeset)
    * Under = and != operators
        If 2 non-nodeset data is to be compared, compare in this order:
        - if one of the data is boolean, cast the other to boolean
        - If one of the data is number, cast the other to number,
        - else cast both to strings and compare them

    * Under <, >, <=, >= operators
        If 2 non-nodeset data is to be compared,
        - convert both to numbers and compare them

 * (for nodeset)
    * Under = and != operators
        when comparing:
        - nodeset and nodeset
            true if and only if there is a node in the first node-set and a node in
            the second node-set such that the result of performing the comparison on
            the string-values of the two nodes is true, else false.

        - nodeset and number
            true if and only if there is a node in the node-set such that the result of performing
            the comparison on the number to be compared and on the result of converting the
            string-value of that node to a number using the number function is true, else false.

        - nodeset and string
            true if and only if there is a node in the node-set such that the result of performing
            the comparison on the string-value of the node and the other string is true, else false.

        - nodeset and boolean
            true if and only if the result of performing the comparison on the boolean and on the
            result of converting the node-set to a boolean using the boolean function is true, else false.

    * Under <, >, <=, >= operators
            Same as the above, except that the nodes to be found must first be converted to numbers
            before being compared.
 */


/*
 * nodeset comparison template functions
 */
static void
_cmp_nodeset_and_nodeset_equality_template(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *),
        bool (*cmp)(cxml_string *, cxml_string *))
        // direction of args doesn't matter in equality comparision,
        // plus, both objects being compared are of the same type (nodeset)
{
    /*
     * nodeset equality comparison works with strings
     */
    cxml_string l_str = new_cxml_string(), r_str = new_cxml_string();
    cxml_list *curr = (cxml_list_size(&left->nodeset.items) <
                       cxml_list_size(&right->nodeset.items)) ?
                      &left->nodeset.items :
                      &right->nodeset.items;
    cxml_list *other = (curr == &left->nodeset.items) ?
                       &right->nodeset.items :
                       &left->nodeset.items;
    bool ret = false;
    cxml_for_each(node, curr)
    {
        cxml_for_each(_node, other)
        {
            _cxml_xp__node_string_val(node, &l_str);
            _cxml_xp__node_string_val(_node, &r_str);
            if (cmp(&l_str, &r_str)) {
                cxml_string_free(&l_str);
                cxml_string_free(&r_str);
                ret = true;
                break;
            }
            cxml_string_free(&l_str);
            cxml_string_free(&r_str);
        }
    }
    _cxml_xp_data_clear(left);
    left->type = CXML_XP_DATA_BOOLEAN;
    left->boolean = ret;
    _push(left);
}


static void
_cmp_nodeset_and_nodeset_relative_template(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *),
        bool (*cmp)(cxml_number *, cxml_number *))
{
    /*
     * relative comparison works only with numbers, hence each
     * node to be compared must be converted to numbers.
     */
    bool ret = false;
    cxml_number l_num = new_cxml_number(), r_num = new_cxml_number();
    cxml_for_each(node, &left->nodeset.items)
    {
        cxml_for_each(_node, &right->nodeset.items)
        {
            _cxml_xp__node_num_val(node, &l_num);
            _cxml_xp__node_num_val(_node, &r_num);
            if (cmp(&l_num, &r_num)) {
                ret = true;
                break;
            }
        }
    }
    _cxml_xp_data_clear(left);
    left->type = CXML_XP_DATA_BOOLEAN;
    left->boolean = ret;
    _push(left);
}

static void
_cmp_nodeset_and_string_equality_template(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *),
        bool (*cmp)(cxml_string *, cxml_string *))
        // direction of args doesn't matter in equality comparision
{
    bool ret = false;
    cxml_string l_str = new_cxml_string(), *r_str = &right->str;
    cxml_for_each(node, &left->nodeset.items)
    {
        _cxml_xp__node_string_val(node, &l_str);
        if (cmp(&l_str, r_str))
        {
            cxml_string_free(&l_str);
            ret = true;
            break;
        }
        cxml_string_free(&l_str);
    }
    _cxml_xp_data_clear(left);
    left->type = CXML_XP_DATA_BOOLEAN;
    left->boolean = ret;
    _push(left);
}

/*  #flip_args
 *  // flip_args argument:
 *  // flip args is used to determine the direction of the comparison, since
    // the direction matters when relative comparison (<, >, <=, >=) is being performed,
    // unlike equality comparison (==). This helps to prevent writing overly duplicated
    // code, when only the operands needs to be flipped for comparison to take place,
    // for example:
    // nodeset and string -> `nodeset < string`, `string < nodeset`
 */

static void
_cmp_nodeset_and_string_relative_template(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *),
        bool (*cmp)(cxml_number *, cxml_number *),
        bool flip_args)
{
    bool ret = false;
    cxml_number l_num = new_cxml_number(), r_num = new_cxml_number();
    _cxml_xp_data_to_numeric(right, &r_num);
    cxml_for_each(node, &left->nodeset.items)
    {
        _cxml_xp__node_num_val(node, &l_num);
        if ((!flip_args ? cmp(&l_num, &r_num) :  cmp(&r_num, &l_num))){
            ret = true;
            break;
        }
    }
    _cxml_xp_data_clear(left);
    left->type = CXML_XP_DATA_BOOLEAN;
    left->boolean = ret;
    _push(left);
}

static void
_cmp_string_and_nodeset_relative_template(
        _cxml_xp_data *right,
        _cxml_xp_data *left,
        void (*_push)(_cxml_xp_data *),
        bool (*cmp)(cxml_number *, cxml_number *))
{
    // flip argument, since nodeset `cmp` string isn't the same as
    // string `cmp` nodeset.
    // see #flip_args above
    _cmp_nodeset_and_string_relative_template(right, left, _push, cmp, 1);
}

static void
_cmp_nodeset_and_number_template(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *),
        bool (*cmp)(cxml_number *, cxml_number *),
        bool flip_args)   // see #flip_args above
{
    bool ret = false;
    cxml_number l_num = new_cxml_number(), *r_num = &right->number;
    cxml_for_each(node, &left->nodeset.items)
    {
        _cxml_xp__node_num_val(node, &l_num);
        if ((!flip_args ? cmp(&l_num, r_num) :  cmp(r_num, &l_num)))
        {
            ret = true;
            break;
        }
    }
    _cxml_xp_data_clear(left);
    left->type = CXML_XP_DATA_BOOLEAN;
    left->boolean = ret;
    _push(left);
}

static void
_cmp_number_and_nodeset_template(
        _cxml_xp_data *right,
        _cxml_xp_data *left,
        void (*_push)(_cxml_xp_data *),
        bool (*cmp)(cxml_number *, cxml_number *))
{
    // flip argument, since nodeset `cmp` number isn't the same as
    // number `cmp` nodeset.
    // see #flip_args above
    _cmp_nodeset_and_number_template(right, left, _push, cmp, 1);
}

static void
_cmp_nodeset_and_boolean_equality_template(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *),
        bool (*cmp)(bool, bool))
        // direction of args doesn't matter in equality comparision
{
    bool l_bool, r_bool = right->boolean;
    _cxml_xp_data_to_boolean(left, &l_bool);
    _cxml_xp_data_clear(left);
    left->type = CXML_XP_DATA_BOOLEAN;
    left->boolean = cmp(l_bool, r_bool);
    _push(left);
}

static void
_cmp_nodeset_and_boolean_relative_template(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *),
        bool (*cmp)(cxml_number *, cxml_number *),
        bool flip_args)   // see #flip_args above
{
    cxml_number l_num = new_cxml_number(), r_num = new_cxml_number();
    _cxml_xp_data_to_numeric(left, &l_num);
    _cxml_xp_data_to_numeric(right, &r_num);
    _cxml_xp_data_clear(left);
    left->type = CXML_XP_DATA_BOOLEAN;
    left->boolean = (!flip_args ? cmp(&l_num, &r_num) :  cmp(&r_num, &l_num));
    _push(left);
}

static void
_cmp_boolean_and_nodeset_relative_template(
        _cxml_xp_data *right,
        _cxml_xp_data *left,
        void (*_push)(_cxml_xp_data *),
        bool (*cmp)(cxml_number *, cxml_number *))
{
    // flip argument, since nodeset `cmp` boolean isn't the same as
    // boolean `cmp` nodeset.
    // see #flip_args above
    _cmp_nodeset_and_boolean_relative_template(right, left, _push, cmp, 1);
}

/********************************/

/*
 * helper functions
 *
 *
 * */
static inline bool cxml_string_is_not_equal(cxml_string* str_1, cxml_string* str_2){
    return !cxml_string_equals(str_1, str_2);
}

static inline bool cxml_number_is_less_equal(cxml_number* num1, cxml_number* num2){
    return !cxml_number_is_greater(num1, num2);
}

static inline bool cxml_number_is_greater_equal(cxml_number* num1, cxml_number* num2){
    return !cxml_number_is_less(num1, num2);
}

static inline bool cxml_boolean_is_equal(bool b1, bool b2) { return b1 == b2; }

static inline bool cxml_boolean_is_not_equal(bool b1, bool b2) { return b1 != b2; }

/********************************/

/*
 * nodeset comparison functions
 */

/*
 *
 * _cmp_nodeset_and_nodeset
 */
static void
_cmp_nodeset_and_nodeset_EQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_nodeset_equality_template(
            left, right, _push, cxml_string_equals);
}

static void
_cmp_nodeset_and_nodeset_NEQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_nodeset_equality_template(
            left, right, _push, cxml_string_is_not_equal);
}



static void
_cmp_nodeset_and_nodeset_LT(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_nodeset_relative_template(
            left, right, _push, cxml_number_is_less);
}

static void
_cmp_nodeset_and_nodeset_LEQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_nodeset_relative_template(
            left, right, _push, cxml_number_is_less_equal);
}

static void
_cmp_nodeset_and_nodeset_GT(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_nodeset_relative_template(
            left, right, _push, cxml_number_is_greater);
}

static void
_cmp_nodeset_and_nodeset_GEQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_nodeset_relative_template(
            left, right, _push, cxml_number_is_greater_equal);
}

/*
 * _cmp_nodeset_and_string
 */

static void
_cmp_nodeset_and_string_EQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_string_equality_template(
            left, right, _push, cxml_string_equals);
}

static void
_cmp_nodeset_and_string_NEQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_string_equality_template(
            left, right, _push, cxml_string_is_not_equal);
}

static void
_cmp_nodeset_and_string_LT(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_string_relative_template(
            left, right, _push, cxml_number_is_less, 0);
}

static void
_cmp_nodeset_and_string_LEQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_string_relative_template(
            left, right, _push, cxml_number_is_less_equal, 0);
}

static void
_cmp_nodeset_and_string_GT(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_string_relative_template(
            left, right, _push, cxml_number_is_greater, 0);
}

static void
_cmp_nodeset_and_string_GEQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_string_relative_template(
            left, right, _push, cxml_number_is_greater_equal, 0);
}

/*
 * _cmp_nodeset_and_number
 */

static void
_cmp_nodeset_and_number_EQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_number_template(left, right, _push, cxml_number_is_equal, 0);
}

static void
_cmp_nodeset_and_number_NEQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_number_template(left, right, _push, cxml_number_is_not_equal, 0);
}

static void
_cmp_nodeset_and_number_LT(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_number_template(left, right, _push, cxml_number_is_less, 0);
}

static void
_cmp_nodeset_and_number_LEQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_number_template(left, right, _push, cxml_number_is_less_equal, 0);
}

static void
_cmp_nodeset_and_number_GT(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_number_template(left, right, _push, cxml_number_is_greater, 0);
}

static void
_cmp_nodeset_and_number_GEQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_number_template(left, right, _push, cxml_number_is_greater_equal, 0);
}

/*
 * _cmp_nodeset_and_boolean
 */

static void
_cmp_nodeset_and_boolean_EQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_boolean_equality_template(
            left, right, _push, cxml_boolean_is_equal);
}

static void
_cmp_nodeset_and_boolean_NEQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_boolean_equality_template(
            left, right, _push, cxml_boolean_is_not_equal);
}

static void
_cmp_nodeset_and_boolean_LT(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_boolean_relative_template(
            left, right, _push, cxml_number_is_less, 0);
}

static void
_cmp_nodeset_and_boolean_LEQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_boolean_relative_template(
            left, right, _push, cxml_number_is_less_equal, 0);
}

static void
_cmp_nodeset_and_boolean_GT(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_boolean_relative_template(
            left, right, _push, cxml_number_is_greater, 0);
}

static void
_cmp_nodeset_and_boolean_GEQ(
        _cxml_xp_data *left,
        _cxml_xp_data *right,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_boolean_relative_template(
            left, right, _push, cxml_number_is_greater_equal, 0);
}

/*
 * _cmp_string_and_nodeset
 */

static void
_cmp_string_and_nodeset_EQ(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_string_EQ(r_node, l_node, _push);
}

static void
_cmp_string_and_nodeset_NEQ(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_string_NEQ(r_node, l_node, _push);
}

static void
_cmp_string_and_nodeset_LT(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_string_and_nodeset_relative_template(
            r_node, l_node, _push, cxml_number_is_less);
}

static void
_cmp_string_and_nodeset_LEQ(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_string_and_nodeset_relative_template(
            r_node, l_node, _push, cxml_number_is_less_equal);
}

static void
_cmp_string_and_nodeset_GT(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_string_and_nodeset_relative_template(
            r_node, l_node, _push, cxml_number_is_greater);
}

static void
_cmp_string_and_nodeset_GEQ(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_string_and_nodeset_relative_template(
            r_node, l_node, _push, cxml_number_is_greater_equal);
}


/*
 * _cmp_number_and_nodeset
 */

static void
_cmp_number_and_nodeset_EQ(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_number_EQ(r_node, l_node, _push);
}

static void
_cmp_number_and_nodeset_NEQ(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_number_NEQ(r_node, l_node, _push);
}

static void
_cmp_number_and_nodeset_LT(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_number_and_nodeset_template(r_node, l_node, _push, cxml_number_is_less);
}

static void
_cmp_number_and_nodeset_LEQ(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_number_and_nodeset_template(
            r_node, l_node, _push, cxml_number_is_less_equal);
}

static void
_cmp_number_and_nodeset_GT(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_number_and_nodeset_template(
            r_node, l_node, _push, cxml_number_is_greater);
}


static void
_cmp_number_and_nodeset_GEQ(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_number_and_nodeset_template(
            r_node, l_node, _push, cxml_number_is_greater_equal);
}


/*
 * _cmp_number_and_nodeset
 */

static void
_cmp_boolean_and_nodeset_EQ(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_boolean_EQ(r_node, l_node, _push);
}

static void
_cmp_boolean_and_nodeset_NEQ(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_nodeset_and_boolean_NEQ(r_node, l_node, _push);
}

static void
_cmp_boolean_and_nodeset_LT(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_boolean_and_nodeset_relative_template(
            r_node, l_node, _push, cxml_number_is_less);
}

static void
_cmp_boolean_and_nodeset_LEQ(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_boolean_and_nodeset_relative_template(
            r_node, l_node, _push, cxml_number_is_less_equal);
}

static void
_cmp_boolean_and_nodeset_GT(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_boolean_and_nodeset_relative_template(
            r_node, l_node, _push, cxml_number_is_greater);
}


static void
_cmp_boolean_and_nodeset_GEQ(
        _cxml_xp_data *l_node,
        _cxml_xp_data *r_node,
        void (*_push)(_cxml_xp_data *))
{
    _cmp_boolean_and_nodeset_relative_template(
            r_node, l_node, _push, cxml_number_is_greater_equal);
}

/*
 * We only care about the binary operations involving at least 1 nodeset node,
 * Since the other combinations are already handled in _cxml_xp_resolve_relative_operation_on_non_nodeset()
 * Thus, using a ragged array:
*                     right
*                       ^
[                       |
                        nodeset         string            number            boolean
    left-> nodeset  [['#', '#', '#'], ['#', '#', '#'], ['#', '#', '#'], ['#', '#', '#'] ],
           string   [['#', '#', '#'],       0,                0,                0       ],
           number   [['#', '#', '#'],       0,                0,                0       ],
           boolean  [['#', '#', '#'],       0,                0,                0       ],
                       |
                       ^
                       op
]
array = A x Y x Z
where
        left  -> A
        right -> Y
        op    -> Z
*/
struct _nodeset_cmp_table nodeset_cmp_table[4][4][6] = \
{
        // r 1
        {
                // =, !=, <, <=, >, >=
                {
                        {.fn=_cmp_nodeset_and_nodeset_EQ}, {_cmp_nodeset_and_nodeset_NEQ},  // nodeset x nodeset
                        {_cmp_nodeset_and_nodeset_LT}, {_cmp_nodeset_and_nodeset_LEQ},
                        {_cmp_nodeset_and_nodeset_GT}, {_cmp_nodeset_and_nodeset_GEQ}
                },

                {
                        {_cmp_nodeset_and_string_EQ}, {_cmp_nodeset_and_string_NEQ},        // nodeset x string
                        {_cmp_nodeset_and_string_LT}, {_cmp_nodeset_and_string_LEQ},
                        {_cmp_nodeset_and_string_GT}, {_cmp_nodeset_and_string_GEQ}
                },

                {
                        {_cmp_nodeset_and_number_EQ}, {_cmp_nodeset_and_number_NEQ},        // nodeset x number
                        {_cmp_nodeset_and_number_LT}, {_cmp_nodeset_and_number_LEQ},
                        {_cmp_nodeset_and_number_GT}, {_cmp_nodeset_and_number_GEQ}
                },

                {
                        {_cmp_nodeset_and_boolean_EQ}, {_cmp_nodeset_and_boolean_NEQ},      // nodeset x boolean
                        {_cmp_nodeset_and_boolean_LT}, {_cmp_nodeset_and_boolean_LEQ},
                        {_cmp_nodeset_and_boolean_GT}, {_cmp_nodeset_and_boolean_GEQ}
                }
        },
        // r 2
        {
                {
                        {_cmp_string_and_nodeset_EQ}, {_cmp_string_and_nodeset_NEQ},       // string x nodeset
                        {_cmp_string_and_nodeset_LT}, {_cmp_string_and_nodeset_LEQ},
                        {_cmp_string_and_nodeset_GT}, {_cmp_string_and_nodeset_GEQ}
                },

        },
        // r 3
        {
                {
                        {_cmp_number_and_nodeset_EQ}, {_cmp_number_and_nodeset_NEQ},       // number x nodeset
                        {_cmp_number_and_nodeset_LT}, {_cmp_number_and_nodeset_LEQ},
                        {_cmp_number_and_nodeset_GT}, {_cmp_number_and_nodeset_GEQ}
                },
        },
        // r 4
        {
                {
                        {_cmp_boolean_and_nodeset_EQ}, {_cmp_boolean_and_nodeset_NEQ},       // boolean x nodeset
                        {_cmp_boolean_and_nodeset_LT}, {_cmp_boolean_and_nodeset_LEQ},
                        {_cmp_boolean_and_nodeset_GT}, {_cmp_boolean_and_nodeset_GEQ}
                },
        },
};

static void _cxml_nodeset_union(
        cxml_set *left,
        cxml_set *right,
        void (*_nodeset_sorter)(cxml_list *),
        cxml_list *sorted)
{
    int size_before = cxml_set_size(left);
    cxml_set_extend(left, right);
    // no changes, right is a subset of left
    if (size_before == cxml_set_size(left)){
        cxml_list_qextend(sorted, &left->items);
    }
    else{
        _nodeset_sorter(&left->items);
        cxml_list_qextend(sorted, &left->items);
    }
}


void
_cxml_xp_resolve_arithmetic_operation(
        _cxml_xp_data* left,
        _cxml_xp_data* right,
        void (*_push)(_cxml_xp_data *),
        cxml_xp_op op)
{
    cxml_number left_num = new_cxml_number(),
                right_num = new_cxml_number();
    // coerce operands to numeric type
    _cxml_xp_data_to_numeric(left, &left_num);
    _cxml_xp_data_to_numeric(right, &right_num);

    // use the converted-to value
    if ((left_num.type == CXML_NUMERIC_NAN_T)
        || (right_num.type == CXML_NUMERIC_NAN_T))
    {
        _cxml_xp_data* node = right;
        _cxml_xp_data_clear(node);
        node->type = CXML_XP_DATA_NUMERIC;
        node->number = new_cxml_number();
        _push(node);
        return;
    }
    double l_val, r_val, res;
    l_val = left_num.dec_val;
    r_val = right_num.dec_val;
    switch(op)
    {
        case CXML_XP_OP_PLUS:
        {
            res = l_val + r_val;
            break;
        }
        case CXML_XP_OP_MINUS:
        {
            res = l_val - r_val;
            break;
        }
        case CXML_XP_OP_MULT:
        {
            res = l_val * r_val;
            break;
        }
        case CXML_XP_OP_DIV:
        {
            // Err if any of the value is 0
            if (l_val == 0 || r_val == 0){
                _cxml_xp_eval_err("Division operation by Zero.");
            }
            res = l_val / r_val;
            break;
        }
        case CXML_XP_OP_MOD:
        {
            // Err if any of the value is 0
            if (l_val == 0 || r_val == 0){
                _cxml_xp_eval_err("Modulus operation on Zero.");
            }
            res = (double)((long)l_val % (long)r_val);
            break;
        }
        default: return;
    }
    _cxml_xp_data* node = right;
    _cxml_xp_data_clear(node);
    node->type = CXML_XP_DATA_NUMERIC;
    node->number.dec_val = res;
    node->number.type = CXML_NUMERIC_DOUBLE_T;
    _push(node);
}

void
_cxml_xp_resolve_and_or_operation(
        _cxml_xp_data* left,
        _cxml_xp_data* right,
        void (*_push)(_cxml_xp_data *),
        cxml_xp_op op)
{
    bool l_val, r_val, res;
    // coerce operands to boolean type
    _cxml_xp_data_to_boolean(left, &l_val);
    if (l_val && op == CXML_XP_OP_OR){
        // under CXML_XP_OP_OR, the right operand is not evaluated
        // if the left operand evaluates to true
        res = 1;
        goto push;
    }
    else if (!l_val && op == CXML_XP_OP_AND){
        // under CXML_XP_OP_OR, the right operand is not evaluated
        // if the left operand evaluates to false
        res = 0;
        goto push;
    }
    _cxml_xp_data_to_boolean(right, &r_val);
    switch (op)
    {
        case CXML_XP_OP_AND:
        {
            res = l_val && r_val;
            break;
        }
        case CXML_XP_OP_OR:
        {
            res = l_val || r_val;
            break;
        }
        default: return;
    }
    // re-use the right node
    push:
    _cxml_xp_data_clear(right);
    right->type = CXML_XP_DATA_BOOLEAN;
    right->boolean = res;
    _push(right);
}

static void
_cxml_xp_resolve_relative_operation_on_non_nodeset(
        _cxml_xp_data* left,
        _cxml_xp_data* right,
        void (*_push)(_cxml_xp_data *),
        cxml_xp_op op)
{
    //          ! & !=
    bool ret;
    if (op == CXML_XP_OP_EQ || op == CXML_XP_OP_NEQ)
    {
        // if one of the data is boolean, cast the other to boolean
        // boolean type has highest precedence in terms of type coercion and casting.
        if (left->type == CXML_XP_DATA_BOOLEAN || right->type == CXML_XP_DATA_BOOLEAN){
            bool l_bol, r_bol;
            if (left->type == CXML_XP_DATA_BOOLEAN){
                l_bol = left->boolean;
                _cxml_xp_data_to_boolean(right, &r_bol);
            }else{
                _cxml_xp_data_to_boolean(left, &l_bol);
                r_bol = right->boolean;
            }
            // compare
            ret = op == CXML_XP_OP_EQ ? l_bol == r_bol : l_bol != r_bol;
        }
        // If one of the data is number, cast the other to number
        else if (left->type == CXML_XP_DATA_NUMERIC || right->type == CXML_XP_DATA_NUMERIC){
            cxml_number l_num = new_cxml_number(), r_num = new_cxml_number();
            if (left->type == CXML_XP_DATA_NUMERIC){
                l_num = left->number;
                _cxml_xp_data_to_numeric(right, &r_num);
            }else{
                _cxml_xp_data_to_numeric(right, &l_num);
                r_num = right->number;
            }
            ret = op == CXML_XP_OP_EQ ?
                  cxml_number_is_equal(&l_num, &r_num) :
                  cxml_number_is_not_equal(&l_num, &r_num);
        }
        // cast both to strings and compare them
        else {
            cxml_string l_str = new_cxml_string(), r_str = new_cxml_string();
            _cxml_xp_data_to_string(left, &l_str);
            _cxml_xp_data_to_string(right, &r_str);
            ret = (op == CXML_XP_OP_EQ) == cxml_string_equals(&l_str, &r_str);
            cxml_string_free(&l_str);
            cxml_string_free(&r_str);
        }
    }
    else{  // convert both to numbers and compare them --
        cxml_number l_num = new_cxml_number(), r_num = new_cxml_number();
        _cxml_xp_data_to_numeric(left, &l_num);
        _cxml_xp_data_to_numeric(right, &r_num);
        switch(op)
        {
            case CXML_XP_OP_LT:
            {
                ret = cxml_number_is_less(&l_num, &r_num);
                break;
            }
            case CXML_XP_OP_GT:
            {
                ret = cxml_number_is_greater(&l_num, &r_num);
                break;
            }
            case CXML_XP_OP_LEQ:
            {
                ret = !(cxml_number_is_greater(&l_num, &r_num));
                break;
            }
            case CXML_XP_OP_GEQ:
            {
                ret = !(cxml_number_is_less(&l_num, &r_num));
                break;
            }
            default: return;
        }
    }
    _cxml_xp_data_clear(left);
    left->type = CXML_XP_DATA_BOOLEAN;
    left->boolean = ret;
    _push(left);
}

static void
_cxml_xp_resolve_relative_operation_on_nodeset(
        _cxml_xp_data* left,
        _cxml_xp_data* right,
        void (*_push)(_cxml_xp_data *),
        cxml_xp_op op)
{
    if (left->type != CXML_XP_DATA_NIL && right->type != CXML_XP_DATA_NIL){
        nodeset_cmp_table[
                _xp_data_type_index(left->type)
                ][_xp_data_type_index(right->type)
                ][_xp_op_index(op)].fn(left, right, _push);
    }else{
        _cxml_xp_eval_err("Ill-value nodeset data found.");
    }
}

void
_cxml_xp_resolve_relative_operation(
        _cxml_xp_data* left,
        _cxml_xp_data* right,
        void (*_push)(_cxml_xp_data *),
        cxml_xp_op op)
{
    if ((left->type != CXML_XP_DATA_NODESET)
        && (right->type != CXML_XP_DATA_NODESET))
    {
        _cxml_xp_resolve_relative_operation_on_non_nodeset(left, right, _push, op);
    }else{
        _cxml_xp_resolve_relative_operation_on_nodeset(left, right, _push, op);
    }
}

void
_cxml_xp_resolve_pipe_operation(
        _cxml_xp_data* left,
        _cxml_xp_data* right,
        void (*_nodeset_sorter)(cxml_list *),
        void (*_push)(_cxml_xp_data *))
{
    // '|' -> pipe or union operator
    // both operands must be of type nodeset because
    // there are no types of objects that can be converted to node-sets.
    if ((left->type != CXML_XP_DATA_NODESET)
        && (right->type != CXML_XP_DATA_NODESET))
    {
        // re-use the left node
        _cxml_xp_data_clear(left);
        left->type = CXML_XP_DATA_NODESET;
        left->nodeset = new_cxml_set();
        _push(left);
    }
    else{
        if (left->type != CXML_XP_DATA_NODESET){
            _cxml_xp_data_clear(left);
            left->type = CXML_XP_DATA_NODESET;
        }else if (right->type != CXML_XP_DATA_NODESET){
            _cxml_xp_data_clear(right);
            right->type = CXML_XP_DATA_NODESET;
        }
        bool left_empty = cxml_set_is_empty(&left->nodeset);
        bool right_empty = cxml_set_is_empty(&right->nodeset);
        // push right back in if not empty but left is empty
        if (left_empty && !right_empty){
            _cxml_xp_data_clear(left);
            _push(right);
        }
        // push left back in if not empty but right is empty or if both are empty
        else if ((right_empty && !left_empty) || (left_empty && right_empty)){
            _cxml_xp_data_clear(right);
            _push(left);
        }
        else{
            // merge right into left
            cxml_list sorted = new_cxml_list();
            _cxml_nodeset_union(&left->nodeset, &right->nodeset, _nodeset_sorter, &sorted);
            _cxml_xp_data_clear(left);
            _cxml_xp_data_clear(right);
            left->type = CXML_XP_DATA_NODESET;
            cxml_set_extend_list(&left->nodeset, &sorted);
            cxml_list_free(&sorted);
            _push(left);
        }
    }
}
