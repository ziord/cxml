/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXXPAST_H
#define CXML_CXXPAST_H

#include "core/cxstr.h"
#include "core/cxdefs.h"
#include "cxxplexer.h"


typedef enum{
    CXML_XP_AST_OBJECT,
    CXML_XP_AST_NIL,
    CXML_XP_AST_UNARYOP_NODE,
    CXML_XP_AST_BINOP_NODE,
    CXML_XP_AST_PREDICATE_NODE,
    CXML_XP_AST_FUNCTION_CALL_NODE,
    CXML_XP_AST_NUM_NODE,
    CXML_XP_AST_STR_LITERAL_NODE,
    CXML_XP_AST_STEP_NODE,
    CXML_XP_AST_NODETEST_NODE,
    CXML_XP_AST_PATH_NODE
}cxml_xp_ast_t;


typedef enum{
    CXML_XP_OP_PLUS,  // '+'
    CXML_XP_OP_MINUS, // '-'
    CXML_XP_OP_MULT,  // '*'
    CXML_XP_OP_DIV,  // 'div'
    CXML_XP_OP_MOD,  // 'mod'
    CXML_XP_OP_EQ,   // '=' (conditional)
    CXML_XP_OP_NEQ,  // !=
    CXML_XP_OP_LT,   // '<'
    CXML_XP_OP_LEQ,  // <=
    CXML_XP_OP_GT,   // '>'
    CXML_XP_OP_GEQ,  // >=
    CXML_XP_OP_AND,  // 'and'
    CXML_XP_OP_OR,   // 'or'
    CXML_XP_OP_PIPE  // '|'
}cxml_xp_op;

//short node_test,      // '//nm  | // * | //nt()'  <- 1 (//nm), 2 (//*), 3 (//nt()), 0 (none)
//short axis_spec,      // '//@nm | //@* | //@nt()' <- 1 (//@nm), 2 (//@*), 3 (//@nt()), 0 (none)
//short self_or_parent, // '//.'  | '//..'          <- 1 (self), 2 (parent), 0 (none)

/*
 * Possible forms:
 * 1) 'pname' : 'lname'     CXML_XP_NODE_TEST_TPNAME_TLNAME
 * 2)  * : 'lname'           CXML_XP_NODE_TEST_TWILDCARD_TLNAME (xpath 2.0)
 * 3) 'pname' : *          CXML_XP_NODE_TEST_TPNAME_TWILDCARD
 * 4)  *                    CXML_XP_NODE_TEST_TWILDCARD
 * 5) 'name'                CXML_XP_NODE_TEST_TNAME
 */
typedef enum{
    CXML_XP_NODE_TEST_NAMETEST,   // 'nm'
    CXML_XP_NODE_TEST_TYPETEST    // 'nt()'
}cxml_xp_node_test_t;

typedef enum{
    CXML_XP_TYPE_TEST_COMMENT,  // comment()
    CXML_XP_TYPE_TEST_TEXT,     // text()
    CXML_XP_TYPE_TEST_NODE,     // node()
    CXML_XP_TYPE_TEST_PI,       // processing-instruction()
}cxml_xp_typetest_t;

typedef enum{
    CXML_XP_NAME_TEST_NAME,              // 'nm'
    CXML_XP_NAME_TEST_WILDCARD,          // '*'
    CXML_XP_NAME_TEST_WILDCARD_LNAME,    // '*:nm'
    CXML_XP_NAME_TEST_PNAME_WILDCARD,    // 'nm:*'
    CXML_XP_NAME_TEST_PNAME_LNAME,       // 'pf:nm'
}cxml_xp_nametest_t;

typedef enum{
    CXML_XP_ABBREV_STEP_TNIL,
    CXML_XP_ABBREV_STEP_TSELF,  // '//.'
    CXML_XP_ABBREV_STEP_TPARENT // '//..'
}cxml_xp_abbrev_step_t;


typedef enum {
    CXML_XP_RET_STRING = 2,
    CXML_XP_RET_NUMBER = 3,
    CXML_XP_RET_NODESET = 4,
    CXML_XP_RET_BOOLEAN = 5
}_cxml_xp_ret_t;

typedef struct{
    cxml_xp_ast_t type;
    cxml_xp_op op;
    struct cxml_xp_astnode* node;
}cxml_xp_unaryop;

typedef struct{
    cxml_xp_ast_t type;
    cxml_xp_op op;
    struct cxml_xp_astnode* l_node;
    struct cxml_xp_astnode* r_node;
}cxml_xp_binaryop;

typedef struct{
    cxml_xp_ast_t type;
    cxml_string val;
}cxml_xp_num;

typedef struct{
    cxml_xp_ast_t type;
    struct cxml_xp_astnode* expr_node;
}cxml_xp_predicate;

typedef struct{
    cxml_xp_ast_t type;
    int pos;
    _cxml_xp_ret_t ret_type;
    cxml_string name;
    cxml_list args;     // stores cxml_xp_astnode*
}cxml_xp_functioncall;

typedef struct{
    cxml_xp_ast_t type;
    cxml_string str;
}cxml_xp_string;

typedef struct{
    cxml_xp_nametest_t t_type;  // test type
    cxml_name name;             // empty if wildcard
}cxml_xp_nametest;

typedef struct{
    bool has_target;            // false if not processing-instruction or if target is missing in pi()
    cxml_xp_typetest_t t_type;  // test type
    cxml_string target;         // target literal, empty if not PI (for processing-instruction('target'))
}cxml_xp_typetest;

typedef struct{
    cxml_xp_ast_t type;
    bool has_attr_axis;
    cxml_xp_node_test_t t_type;         // test type
    union{
        cxml_xp_nametest name_test;
        cxml_xp_typetest type_test;     // also known as kind_test, e.g. node(), text(), etc.
    };
}cxml_xp_nodetest;

typedef struct{
    cxml_xp_ast_t type;
    short abbrev_step;              // '.' -> 1 | '..' -> 2 | None -> 0
    short path_spec;                // '/' -> 1 | '//' -> 2 | None -> 0
    bool has_attr_axis;             // '@'
    cxml_xp_nodetest* node_test;    // NodeTest  -> stores node-test directly
    cxml_list predicates;           // Predicate* -> stores predicate directly
}cxml_xp_step;

typedef struct{
    cxml_xp_ast_t type;
    bool from_predicate;    // determines if this path node lives inside a predicate
    cxml_list steps;        // ->stores step directly
}cxml_xp_path;

typedef struct cxml_xp_astnode{  // cxml xpath ast node
    cxml_xp_ast_t type;
    cxml_xp_ast_t wrapped_type;
    union{
        cxml_xp_unaryop* unary;
        cxml_xp_binaryop* binary;
        cxml_xp_num* num;
        cxml_xp_predicate* predicate;
        cxml_xp_functioncall* func_call;
        cxml_xp_string* str_literal;
        cxml_xp_step* step;
        cxml_xp_path* path;
    }wrapped_node;
}cxml_xp_astnode;

#endif //CXML_CXXPAST_H
