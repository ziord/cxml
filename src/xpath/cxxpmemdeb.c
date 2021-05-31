/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "xpath/cxxpvisitors.h"

/**expression building visitor**/
static void cxml_xp_bvisit_NameTest(cxml_xp_nametest *node, cxml_string *acc);

static void cxml_xp_bvisit_Step(cxml_xp_step *node, cxml_string *acc);

static void cxml_xp_bvisit_Predicate(cxml_xp_predicate *node, cxml_string *acc);

static void cxml_xp_bvisit_Num(cxml_xp_num *node, cxml_string *acc);

static void cxml_xp_bvisit_TypeTest(cxml_xp_typetest *node, cxml_string *acc);

static void cxml_xp_bvisit_String(cxml_xp_string *node, cxml_string *acc);

static void cxml_xp_bvisit_Path(cxml_xp_path *node, cxml_string *acc);

static void cxml_xp_bvisit_FunctionCall(cxml_xp_functioncall *node, cxml_string *acc);

static void cxml_xp_bvisit_UnaryOp(cxml_xp_unaryop *node, cxml_string *acc);

static void cxml_xp_bvisit_BinaryOp(cxml_xp_binaryop *node, cxml_string *acc);

static void cxml_xp_bvisit_NodeTest(cxml_xp_nodetest *node, cxml_string *acc);

/********************************/


/**debug visitors**/

static void cxml_xp_dvisit_NameTest(cxml_xp_nametest *node);

static void cxml_xp_dvisit_Step(cxml_xp_step *node);

static void cxml_xp_dvisit_Predicate(cxml_xp_predicate *node);

static void cxml_xp_dvisit_Num(cxml_xp_num *node);

static void cxml_xp_dvisit_TypeTest(cxml_xp_typetest *node);

static void cxml_xp_dvisit_String(cxml_xp_string *node);

static void cxml_xp_dvisit_Path(cxml_xp_path *node);

static void cxml_xp_dvisit_FunctionCall(cxml_xp_functioncall *node);

static void cxml_xp_dvisit_UnaryOp(cxml_xp_unaryop *node);

static void cxml_xp_dvisit_BinaryOp(cxml_xp_binaryop *node);

static void cxml_xp_dvisit_NodeTest(cxml_xp_nodetest *node);

/********************************/


/******node freeing visitors*********/

static void cxml_xp_fvisit_Step(cxml_xp_step *node);

static void cxml_xp_fvisit_Predicate(cxml_xp_predicate *node);

static void cxml_xp_fvisit_Num(cxml_xp_num *node);

static void cxml_xp_fvisit_NameTest(cxml_xp_nametest *node);

static void cxml_xp_fvisit_TypeTest(cxml_xp_typetest *node);

static void cxml_xp_fvisit_String(cxml_xp_string *node);

static void cxml_xp_fvisit_Path(cxml_xp_path *path);

static void cxml_xp_fvisit_FunctionCall(cxml_xp_functioncall *node);

static void cxml_xp_fvisit_UnaryOp(cxml_xp_unaryop *node);

static void cxml_xp_fvisit_BinaryOp(cxml_xp_binaryop *node);

static void cxml_xp_fvisit_NodeTest(cxml_xp_nodetest *node);

/********************************/

/*** debug visitors ***/
void cxml_xp_dvisit(cxml_xp_astnode * ast_node){  // generic cxml_xp_visit
    switch(ast_node->wrapped_type){
        case CXML_XP_AST_UNARYOP_NODE:
            cxml_xp_dvisit_UnaryOp(ast_node->wrapped_node.unary);
            break;
        case CXML_XP_AST_BINOP_NODE:
            cxml_xp_dvisit_BinaryOp(ast_node->wrapped_node.binary);
            break;
        case CXML_XP_AST_PREDICATE_NODE:
            cxml_xp_dvisit_Predicate(ast_node->wrapped_node.predicate);
            break;
        case CXML_XP_AST_FUNCTION_CALL_NODE:
            cxml_xp_dvisit_FunctionCall(ast_node->wrapped_node.func_call);
            break;
        case CXML_XP_AST_NUM_NODE:
            cxml_xp_dvisit_Num(ast_node->wrapped_node.num);
            break;
        case CXML_XP_AST_STR_LITERAL_NODE:
            cxml_xp_dvisit_String(ast_node->wrapped_node.str_literal);
            break;
        case CXML_XP_AST_STEP_NODE:
            cxml_xp_dvisit_Step(ast_node->wrapped_node.step);
            break;
        case CXML_XP_AST_PATH_NODE:
            cxml_xp_dvisit_Path(ast_node->wrapped_node.path);
            break;
        default: break;
    }
}


void cxml_xp_dvisit_Step(cxml_xp_step* node){
_CXML__TRACE(
        _cxml_dprint("<--In cxml_xp_step node-->\n")
    if (node->path_spec){
        if (node->path_spec == 1){
            _cxml_dprint("\t/\n")
        }else{
            _cxml_dprint("\t//\n")
        }
    }
    if (node->has_attr_axis){
        _cxml_dprint("\t@\n")
    }
)
    if (node->node_test){
        cxml_xp_dvisit_NodeTest(node->node_test);
    }
    else if (node->abbrev_step){
        if (node->abbrev_step == 1){
            _cxml_dprint("\t.\n")
        }else{
            _cxml_dprint("\t..\n")
        }
    }
    cxml_for_each (pred, &node->predicates){
        cxml_xp_dvisit_Predicate(pred);
    }
}

void cxml_xp_dvisit_NodeTest(cxml_xp_nodetest* node){
    _cxml_dprint("<--In cxml_xp_nodetest node-->\n")

    if (node->t_type == CXML_XP_NODE_TEST_NAMETEST){
        cxml_xp_dvisit_NameTest(&node->name_test);
    }else{
        cxml_xp_dvisit_TypeTest(&node->type_test);
    }
}

void cxml_xp_dvisit_Predicate(cxml_xp_predicate* node){
_CXML__TRACE(
    _cxml_dprint("<--In cxml_xp_predicate node-->\n")
    _cxml_dprint("\t[\n"))
    cxml_xp_dvisit(node->expr_node);
_CXML__TRACE(
        _cxml_dprint("\t]\n"))
}

void cxml_xp_dvisit_Num(cxml_xp_num* node){
_CXML__TRACE(
    _cxml_dprint("<--In cxml_xp_num node-->\n")
    _cxml_dprint("\t(%s)\n", cxml_string_as_raw(&node->val)))
}

void cxml_xp_dvisit_NameTest(cxml_xp_nametest* node){
_CXML__TRACE(
    _cxml_dprint("<--In cxml_xp_nametest node-->\n")
    _cxml_dprint("\t(%s)\n", cxml_string_len(&node->name.qname) ?
                             cxml_string_as_raw(&node->name.qname) : "(wildcard *)"))
}

void cxml_xp_dvisit_TypeTest(cxml_xp_typetest* node){
_CXML__TRACE(
    _cxml_dprint("<--In cxml_xp_typetest node-->\n")
    if (node->t_type == CXML_XP_TYPE_TEST_PI){
        if (node->has_target){
            _cxml_dprint("\tprocessinng-instruction( '%s' )\n", cxml_string_as_raw(&node->target))
        }else{
            _cxml_dprint("\tprocessinng-instruction()\n")
        }
    }
    else{
        switch(node->t_type){
            case CXML_XP_TYPE_TEST_NODE:
                _cxml_dprint("\tnode()\n")
                break;
            case CXML_XP_TYPE_TEST_TEXT:
                _cxml_dprint("\ttext()\n")
                break;
            case CXML_XP_TYPE_TEST_COMMENT:
                _cxml_dprint("\tcomment()\n")
                break;
            default: break;
        }
    })
}

void cxml_xp_dvisit_String(cxml_xp_string* node){
_CXML__TRACE(
    _cxml_dprint("<--In cxml_xp_string node-->\n")
    _cxml_dprint("\t(%s)\n", cxml_string_as_raw(&node->str));)
}

void cxml_xp_dvisit_Path(cxml_xp_path* node){
    _cxml_dprint("<--In cxml_xp_path node-->\n")
    cxml_for_each(step, &node->steps)
    {
        cxml_xp_dvisit_Step(step);
    }
}


void cxml_xp_dvisit_FunctionCall(cxml_xp_functioncall* node){
_CXML__TRACE(
    _cxml_dprint("<--In cxml_xp_functioncall node-->\n")
    _cxml_dprint("\t%s(\n", cxml_string_as_raw(&node->name)))

    cxml_for_each(arg, &node->args){
        cxml_xp_dvisit(arg);
    }
    _cxml_dprint("\t\t)\n")
}
void cxml_xp_dvisit_UnaryOp(cxml_xp_unaryop* node){
    _cxml_dprint("<--In cxml_xp_unaryop node-->\n")
_CXML__TRACE(
    if (node->op == CXML_XP_OP_PLUS) {
        _cxml_dprint("\t + \n")
    } else if (node->op == CXML_XP_OP_MINUS) {
        _cxml_dprint("\t - \n")
    }
)
    cxml_xp_dvisit(node->node);
}
void cxml_xp_dvisit_BinaryOp(cxml_xp_binaryop* node){
    _cxml_dprint("<--In cxml_xp_binaryop node-->\n")
    cxml_xp_dvisit(node->l_node);
_CXML__TRACE(
        _cxml_dprint("op:");
    switch(node->op){
        case CXML_XP_OP_PLUS:
            _cxml_dprint("\t + \n")
            break;
        case CXML_XP_OP_MINUS:
            _cxml_dprint("\t - \n")
            break;
        case CXML_XP_OP_MULT:
            _cxml_dprint("\t * \n")
            break;
        case CXML_XP_OP_EQ:
            _cxml_dprint("\t = \n")
            break;
        case CXML_XP_OP_DIV:
            _cxml_dprint("\t div \n")
            break;
        case CXML_XP_OP_MOD:
            _cxml_dprint("\t mod \n")
            break;
        case CXML_XP_OP_LEQ:
            _cxml_dprint("\t <= \n")
            break;
        case CXML_XP_OP_GEQ:
            _cxml_dprint("\t >= \n")
            break;
        case CXML_XP_OP_NEQ:
            _cxml_dprint("\t != \n")
            break;
        case CXML_XP_OP_GT:
            _cxml_dprint("\t > \n")
            break;
        case CXML_XP_OP_LT:
            _cxml_dprint("\t < \n")
            break;
        case CXML_XP_OP_AND:
            _cxml_dprint("\t and \n")
            break;
        case CXML_XP_OP_OR:
            _cxml_dprint("\t or \n")
            break;
        case CXML_XP_OP_PIPE:
            _cxml_dprint("\t | \n")
            break;
    }
)
    cxml_xp_dvisit(node->r_node);
}


/*** node freeing visitors ***/

void cxml_xp_fvisit(cxml_xp_astnode * ast_node){  // generic cxml_xp_visit
    _cxml_dprint("<--FREEING (cxml_xp_astnode) node-->\n")

    switch(ast_node->wrapped_type){
        case CXML_XP_AST_UNARYOP_NODE:
            cxml_xp_fvisit_UnaryOp(ast_node->wrapped_node.unary);
            break;
        case CXML_XP_AST_BINOP_NODE:
            cxml_xp_fvisit_BinaryOp(ast_node->wrapped_node.binary);
            break;
        case CXML_XP_AST_PREDICATE_NODE:
            cxml_xp_fvisit_Predicate(ast_node->wrapped_node.predicate);
            break;
        case CXML_XP_AST_FUNCTION_CALL_NODE:
            cxml_xp_fvisit_FunctionCall(ast_node->wrapped_node.func_call);
            break;
        case CXML_XP_AST_NUM_NODE:
            cxml_xp_fvisit_Num(ast_node->wrapped_node.num);
            break;
        case CXML_XP_AST_STR_LITERAL_NODE:
            cxml_xp_fvisit_String(ast_node->wrapped_node.str_literal);
            break;
        case CXML_XP_AST_STEP_NODE:
            cxml_xp_fvisit_Step(ast_node->wrapped_node.step);
            break;
        case CXML_XP_AST_PATH_NODE:
            cxml_xp_fvisit_Path(ast_node->wrapped_node.path);
            break;
        default: break;
    }
    FREE(ast_node);
}

// F
void cxml_xp_fvisit_Step(cxml_xp_step* node){
_CXML__TRACE(
        _cxml_dprint("<--FREEING (cxml_xp_step) node-->\n")
    if (node->path_spec){
        if (node->path_spec == 1){
            _cxml_dprint("\t/\n")
        }else{
            _cxml_dprint("\t//\n")
        }
    }
    if (node->has_attr_axis){
        _cxml_dprint("\t@\n")
    }
)
    if (node->node_test){  //
        cxml_xp_fvisit_NodeTest(node->node_test);
    }
    else if (node->abbrev_step){
        if (node->abbrev_step == 1){
            _cxml_dprint("\t.\n")
        }else{
            _cxml_dprint("\t..\n")
        }
    }
    cxml_for_each (pred, &node->predicates){
        cxml_xp_fvisit_Predicate(pred);
    }
    cxml_list_free(&node->predicates);
    FREE(node);
}

// F
void cxml_xp_fvisit_NodeTest(cxml_xp_nodetest* node){
    _cxml_dprint("<--FREEING (cxml_xp_nodetest) node-->\n")

    if (node->t_type == CXML_XP_NODE_TEST_NAMETEST){
        cxml_xp_fvisit_NameTest(&node->name_test);
    }else{
        cxml_xp_fvisit_TypeTest(&node->type_test);
    }
    FREE(node);
}

// F
void cxml_xp_fvisit_Predicate(cxml_xp_predicate* node){
    _cxml_dprint("<--FREEING (cxml_xp_predicate) node-->\n")
    _cxml_dprint("\t[\n")
    cxml_xp_fvisit(node->expr_node);
    _cxml_dprint("\t]\n")
    FREE(node);
}

// F
void cxml_xp_fvisit_Num(cxml_xp_num* node){
    _cxml_dprint("<--FREEING (cxml_xp_num) node-->\n")
    _cxml_dprint("\t(%s)\n", cxml_string_as_raw(&node->val))
    cxml_string_free(&node->val);
    FREE(node);
}

// F
void cxml_xp_fvisit_NameTest(cxml_xp_nametest* node){
    _cxml_dprint("<--FREEING (cxml_xp_nametest) node-->\n")
    _cxml_dprint("\t(%s)\n", cxml_string_len(&node->name.qname) ?
                             cxml_string_as_raw(&node->name.qname) : "(wildcard *)")
    cxml_name_free(&node->name);
    // name test isn't allocated. DON'T FREE
}

// F
void cxml_xp_fvisit_TypeTest(cxml_xp_typetest* node){
_CXML__TRACE(
        _cxml_dprint("<--FREEING (cxml_xp_typetest) node-->\n")
    if (node->t_type == CXML_XP_TYPE_TEST_PI){
        if (node->has_target){
            _cxml_dprint("\tprocessinng-instruction( '%s' )\n", cxml_string_as_raw(&node->target));
        }else{
            _cxml_dprint("\tprocessinng-instruction()\n")
        }
    }else{
        switch(node->t_type){
            case CXML_XP_TYPE_TEST_NODE:
                _cxml_dprint("\tnode()\n")
                break;
            case CXML_XP_TYPE_TEST_TEXT:
                _cxml_dprint("\ttext()\n")
                break;
            case CXML_XP_TYPE_TEST_COMMENT:
                _cxml_dprint("\tcomment()\n")
                break;
            default: break;
        }
    }
)
    if (node->has_target){
        cxml_string_free(&node->target);
    }
    // type test isn't allocated. DON'T FREE
}

// F
void cxml_xp_fvisit_String(cxml_xp_string* node){
    _cxml_dprint("<--FREEING (cxml_xp_string) node-->\n")
    _cxml_dprint("\t(%s)\n", cxml_string_as_raw(&node->str))
    cxml_string_free(&node->str);
    FREE(node);
}

// F
void cxml_xp_fvisit_Path(cxml_xp_path* path){
    _cxml_dprint("<--FREEING (cxml_xp_path) node-->\n")

    cxml_for_each(step, &path->steps)
    {
        cxml_xp_fvisit_Step(step);
    }
    cxml_list_free(&path->steps);
    FREE(path);
}

// F
void cxml_xp_fvisit_FunctionCall(cxml_xp_functioncall* node){
    _cxml_dprint("<--FREEING (cxml_xp_functioncall) node-->\n")
    _cxml_dprint("\t%s(\n", cxml_string_as_raw(&node->name))

    cxml_for_each(arg, &node->args){
        cxml_xp_fvisit(arg);
    }
    cxml_string_free(&node->name);
    cxml_list_free(&node->args);
    FREE(node);
    _cxml_dprint("\t\t)\n")
}

// F
void cxml_xp_fvisit_UnaryOp(cxml_xp_unaryop* node){
    _CXML__TRACE(
            _cxml_dprint("<--FREEING (cxml_xp_unaryop) node-->\n")
            if (node->op == CXML_XP_OP_PLUS) {
                _cxml_dprint("\t + \n")
            } else if (node->op == CXML_XP_OP_MINUS) {
                _cxml_dprint("\t - \n")
            }
     )
    cxml_xp_fvisit(node->node);
    FREE(node);
}

// F
void cxml_xp_fvisit_BinaryOp(cxml_xp_binaryop* node){
    _cxml_dprint("<--FREEING (cxml_xp_binaryop) node-->\n")
    cxml_xp_fvisit(node->l_node);
_CXML__TRACE(
        _cxml_dprint("op:");
    switch(node->op){
        case CXML_XP_OP_PLUS:
            _cxml_dprint("\t + \n");
            break;
        case CXML_XP_OP_MINUS:
            _cxml_dprint("\t - \n");
            break;
        case CXML_XP_OP_MULT:
            _cxml_dprint("\t * \n");
            break;
        case CXML_XP_OP_EQ:
            _cxml_dprint("\t = \n");
            break;
        case CXML_XP_OP_DIV:
            _cxml_dprint("\t div \n");
            break;
        case CXML_XP_OP_MOD:
            _cxml_dprint("\t mod \n");
            break;
        case CXML_XP_OP_LEQ:
            _cxml_dprint("\t <= \n");
            break;
        case CXML_XP_OP_GEQ:
            _cxml_dprint("\t >= \n");
            break;
        case CXML_XP_OP_NEQ:
            _cxml_dprint("\t != \n");
            break;
        case CXML_XP_OP_GT:
            _cxml_dprint("\t > \n");
            break;
        case CXML_XP_OP_LT:
            _cxml_dprint("\t < \n");
            break;
        case CXML_XP_OP_AND:
            _cxml_dprint("\t and \n");
            break;
        case CXML_XP_OP_OR:
            _cxml_dprint("\t or \n");
            break;
        case CXML_XP_OP_PIPE:
            _cxml_dprint("\t | \n");
            break;
    }
)
    cxml_xp_fvisit(node->r_node);
    FREE(node);
}


/**expression building visitors**/
void cxml_xp_bvisit(cxml_xp_astnode * ast_node, cxml_string *acc){
    switch(ast_node->wrapped_type){
        case CXML_XP_AST_UNARYOP_NODE:
            cxml_xp_bvisit_UnaryOp(ast_node->wrapped_node.unary, acc);
            break;
        case CXML_XP_AST_BINOP_NODE:
            cxml_xp_bvisit_BinaryOp(ast_node->wrapped_node.binary, acc);
            break;
        case CXML_XP_AST_PREDICATE_NODE:
            cxml_xp_bvisit_Predicate(ast_node->wrapped_node.predicate, acc);
            break;
        case CXML_XP_AST_FUNCTION_CALL_NODE:
            cxml_xp_bvisit_FunctionCall(ast_node->wrapped_node.func_call, acc);
            break;
        case CXML_XP_AST_NUM_NODE:
            cxml_xp_bvisit_Num(ast_node->wrapped_node.num, acc);
            break;
        case CXML_XP_AST_STR_LITERAL_NODE:
            cxml_xp_bvisit_String(ast_node->wrapped_node.str_literal, acc);
            break;
        case CXML_XP_AST_STEP_NODE:
            cxml_xp_bvisit_Step(ast_node->wrapped_node.step, acc);
            break;
        case CXML_XP_AST_PATH_NODE:
            cxml_xp_bvisit_Path(ast_node->wrapped_node.path, acc);
            break;
        default: break;
    }
}

void cxml_xp_bvisit_NameTest(cxml_xp_nametest* node, cxml_string *acc){
    switch (node->t_type){
        case CXML_XP_NAME_TEST_PNAME_LNAME:
        case CXML_XP_NAME_TEST_NAME:
            cxml_string_str_append(acc, &node->name.qname);
            break;
        case CXML_XP_NAME_TEST_WILDCARD:
            cxml_string_append(acc, "*", 1);
            break;
        case CXML_XP_NAME_TEST_WILDCARD_LNAME:
            cxml_string_append(acc, "*:", 2);
            cxml_string_append(acc, node->name.lname, node->name.lname_len);
            break;
        case CXML_XP_NAME_TEST_PNAME_WILDCARD:
            cxml_string_append(acc, node->name.pname, node->name.pname_len);
            cxml_string_append(acc, ":*", 2);
            break;
        default:
            break;
    }
}

static void append_type_test(cxml_xp_typetest* type_test, cxml_string *acc){
    switch(type_test->t_type){
        case CXML_XP_TYPE_TEST_TEXT:
            cxml_string_append(acc, "text()", 6);
            break;
        case CXML_XP_TYPE_TEST_COMMENT:
            cxml_string_append(acc, "comment()", 9);
            break;
        case CXML_XP_TYPE_TEST_PI:
            // minimize processing-instruction(..) to PI(..)
            if (type_test->has_target){
                cxml_string_append(acc, "PI(", 3);
                cxml_string_str_append(acc, &type_test->target);
                cxml_string_append(acc, ")", 1);
            }else{
                cxml_string_append(acc, "PI()", 4);
            }
            break;
        case CXML_XP_TYPE_TEST_NODE:
            cxml_string_append(acc, "node()", 6);
            break;
        default:
            break;
    }
}

static char* get_cxml_op_repr(cxml_xp_op op){
    switch(op){
        case CXML_XP_OP_PLUS:   return " + ";
        case CXML_XP_OP_MINUS:  return " - ";
        case CXML_XP_OP_MULT:   return " * ";
        case CXML_XP_OP_DIV:    return " div ";
        case CXML_XP_OP_MOD:    return " mod ";
        case CXML_XP_OP_EQ:     return " = ";
        case CXML_XP_OP_NEQ:    return " != ";
        case CXML_XP_OP_LT:     return " < ";
        case CXML_XP_OP_LEQ:    return " <= ";
        case CXML_XP_OP_GT:     return " > ";
        case CXML_XP_OP_GEQ:    return " >= ";
        case CXML_XP_OP_AND:    return " and ";
        case CXML_XP_OP_OR:     return " or ";
        case CXML_XP_OP_PIPE:   return " | ";
        default:
            cxml_error("Unknown operator type");
    }
}

void cxml_xp_bvisit_Step(cxml_xp_step* node, cxml_string *acc){
    switch (node->path_spec){
        case 1: cxml_string_append(acc, "/", 1);  break;
        case 2: cxml_string_append(acc, "//", 2); break;
        default: break;
    }
    if (node->has_attr_axis){
        cxml_string_append(acc, "@", 1);
    }
    if (node->node_test){
        cxml_xp_bvisit_NodeTest(node->node_test, acc);
    }
    else if (node->abbrev_step){
        if (node->abbrev_step == 1){
            cxml_string_append(acc, ".", 1);
        }else{
            cxml_string_append(acc, "..", 2);
        }
    }
    cxml_for_each (pred, &node->predicates){
        cxml_xp_bvisit_Predicate(pred, acc);
    }
}

void cxml_xp_bvisit_Predicate(cxml_xp_predicate* node, cxml_string *acc){
    cxml_string_append(acc, "[", 1);
    cxml_xp_bvisit(node->expr_node, acc);
    cxml_string_append(acc, "]", 1);
}

void cxml_xp_bvisit_Num(cxml_xp_num* node, cxml_string *acc){
    cxml_string_str_append(acc, &node->val);
}

void cxml_xp_bvisit_TypeTest(cxml_xp_typetest* node, cxml_string *acc){
    append_type_test(node, acc);
}

void cxml_xp_bvisit_String(cxml_xp_string* node, cxml_string *acc){
    cxml_string_str_append(acc, &node->str);
}

void cxml_xp_bvisit_Path(cxml_xp_path* node, cxml_string *acc){
    cxml_for_each(step, &node->steps)
    {
        cxml_xp_bvisit_Step(step, acc);
    }
}

void cxml_xp_bvisit_FunctionCall(cxml_xp_functioncall* node, cxml_string *acc){
    cxml_string_str_append(acc, &node->name);
    cxml_string_append(acc, "(", 1);
    cxml_for_each(arg, &node->args){
        cxml_xp_bvisit(arg, acc);
    }
    cxml_string_append(acc, ")", 1);
}

void cxml_xp_bvisit_UnaryOp(cxml_xp_unaryop* node, cxml_string *acc){
    cxml_string_append(acc, node->op == CXML_XP_OP_MINUS ? " -" : " +", 2);
    cxml_xp_bvisit(node->node, acc);
}

void cxml_xp_bvisit_BinaryOp(cxml_xp_binaryop* node, cxml_string *acc){
    cxml_xp_bvisit(node->l_node, acc);
    cxml_string_raw_append(acc, get_cxml_op_repr(node->op));
    cxml_xp_bvisit(node->r_node, acc);
}

void cxml_xp_bvisit_NodeTest(cxml_xp_nodetest* node, cxml_string *acc){
    if (node->t_type == CXML_XP_NODE_TEST_NAMETEST){
        cxml_xp_bvisit_NameTest(&node->name_test, acc);
    }else{
        cxml_xp_bvisit_TypeTest(&node->type_test, acc);
    }
}
