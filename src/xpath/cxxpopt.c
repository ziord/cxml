/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "xpath/cxxpvisitors.h"

/*
 * These node visitors check if a predicate expression can be optimized.
 * If so, the result would only be computed once in visit_Predicate (cxxpeval.c),
 * as opposed to the number of nodes in a given context, and the number of contexts available.
 * There are 2 good possibilities of optimization in a predicate expression:
 *
 * ALWAYS_TRUE: predicate expressions that'll always be true, irrespective of the context node
 *              or the number of times they are evaluated.
 * ALWAYS_FALSE: predicate expressions that'll always be false, irrespective of the context node
 *              or the number of times they are evaluated.
 * In other words, Truthy or Falsy optimizations.
 *
 * How do we tell a predicate expression can be optimized?
 * Well, as long as each step node in a path node's expression (if available)
 * has a path spec (path specification) of '/' and '//' (1 or 2)
 * or contains only constants (strings, numbers), AND the result of the predicate expression doesn't
 * evaluate to a number, then it can be optimized. If it evaluates to number, it cannot be optimized
 * because numbers would be compared with the context node's position, which doesn't always get
 * evaluated to the same result as/since the context node changes.
 *
 * For example  `//foo//bar` in:
 *      // *[//foo//bar]
 *  can be optimized.
 *  However, this cannot be optimized:
 *      / *[//foo + .//bar] -> `//foo + .//bar` yields number (which translates to a match for the
 *      context node's position) and also depends on the context node.
 *  This is due to the (not so obvious) fact that in a predicate expression,
 *  step nodes (of a path node) with a path spec of 1 or 2 ('/' or '//') begins
 *  evaluation from the root node.
 */

extern bool _is_poisonous_by_computation(int pos);

extern bool _is_poisonous_by_default_arg(int pos);

static void cxml_xp_ovisit_Step(cxml_xp_step *node, _cxml_xp_ret_t *ret_type);

static void cxml_xp_ovisit_Predicate(cxml_xp_predicate *node, _cxml_xp_ret_t *ret_type);

static void cxml_xp_ovisit_Num(cxml_xp_num *node, _cxml_xp_ret_t *ret_type);

static void cxml_xp_ovisit_String(cxml_xp_string *node, _cxml_xp_ret_t *ret_type);

static void cxml_xp_ovisit_Path(cxml_xp_path *node, _cxml_xp_ret_t *ret_type);

static void cxml_xp_ovisit_FunctionCall(cxml_xp_functioncall *node, _cxml_xp_ret_t *ret_type);

static void cxml_xp_ovisit_UnaryOp(cxml_xp_unaryop *node, _cxml_xp_ret_t *ret_type);

static void cxml_xp_ovisit_BinaryOp(cxml_xp_binaryop *node, _cxml_xp_ret_t *ret_type);


inline static bool _is_arithmetic_op(cxml_xp_op op){
    switch (op)
    {
        case CXML_XP_OP_PLUS:
        case CXML_XP_OP_MINUS:
        case CXML_XP_OP_DIV:
        case CXML_XP_OP_MULT:
        case CXML_XP_OP_MOD: return 1;
        default: return 0;
    }
}

inline static bool _is_relative_op(cxml_xp_op op){
    switch (op)
    {
        case CXML_XP_OP_LT:
        case CXML_XP_OP_LEQ:
        case CXML_XP_OP_GT:
        case CXML_XP_OP_GEQ:
        case CXML_XP_OP_NEQ:
        case CXML_XP_OP_EQ: return 1;
        default: return 0;
    }
}

inline static bool _is_logic_op(cxml_xp_op op){
    switch (op)
    {
        case CXML_XP_OP_AND:
        case CXML_XP_OP_OR: return 1;
        default: return 0;
    }
}

// a * 2 + x
_cxml_xp_ret_t _compute_type(cxml_xp_op op){
    // straight-forward due to type coercion
    if (_is_relative_op(op) || _is_logic_op(op)){
        return CXML_XP_RET_BOOLEAN;
    }else if (_is_arithmetic_op(op)){
        return CXML_XP_RET_NUMBER;
    }else if (op == CXML_XP_OP_PIPE){
        return CXML_XP_RET_NODESET;
    }
    // unreachable
    return 0;
}

void cxml_xp_ovisit(cxml_xp_astnode * ast_node, _cxml_xp_ret_t *ret_type){
    switch(ast_node->wrapped_type){
        case CXML_XP_AST_UNARYOP_NODE:
            cxml_xp_ovisit_UnaryOp(ast_node->wrapped_node.unary, ret_type);
            break;
        case CXML_XP_AST_BINOP_NODE:
            cxml_xp_ovisit_BinaryOp(ast_node->wrapped_node.binary, ret_type);
            break;
        case CXML_XP_AST_PREDICATE_NODE:
            cxml_xp_ovisit_Predicate(ast_node->wrapped_node.predicate, ret_type);
            break;
        case CXML_XP_AST_FUNCTION_CALL_NODE:
            cxml_xp_ovisit_FunctionCall(ast_node->wrapped_node.func_call, ret_type);
            break;
        case CXML_XP_AST_NUM_NODE:
            cxml_xp_ovisit_Num(ast_node->wrapped_node.num, ret_type);
            break;
        case CXML_XP_AST_STR_LITERAL_NODE:
            cxml_xp_ovisit_String(ast_node->wrapped_node.str_literal, ret_type);
            break;
        case CXML_XP_AST_STEP_NODE:
            cxml_xp_ovisit_Step(ast_node->wrapped_node.step, ret_type);
            break;
        case CXML_XP_AST_PATH_NODE:
            cxml_xp_ovisit_Path(ast_node->wrapped_node.path, ret_type);
            break;
        default: break;
    }
}

void cxml_xp_ovisit_Step(cxml_xp_step* node, _cxml_xp_ret_t *ret_type){
    // we only care about name-test and type-test if and only if
    // they have a path spec of '/' or '//' (1 or 2 respectively),
    // else stop visiting immediately.
    if (node->path_spec == 0){
        *ret_type = _CXML_XP_PS_POISON;
        return;
    }
    cxml_for_each (pred, &node->predicates){
        cxml_xp_ovisit_Predicate(pred, ret_type);
        if (*ret_type == _CXML_XP_PS_POISON) return;
    }
}

void cxml_xp_ovisit_Predicate(cxml_xp_predicate* node, _cxml_xp_ret_t *ret_type){
    cxml_xp_ovisit(node->expr_node, ret_type);
}

void cxml_xp_ovisit_Num(cxml_xp_num* node, _cxml_xp_ret_t *ret_type){
    (void)node;
    *ret_type = CXML_XP_RET_NUMBER;
}

void cxml_xp_ovisit_String(cxml_xp_string* node, _cxml_xp_ret_t *ret_type){
    (void)node;
    *ret_type = CXML_XP_RET_STRING;
}

void cxml_xp_ovisit_Path(cxml_xp_path* node, _cxml_xp_ret_t *ret_type){
    // return immediately we discover the expression is no longer optimizable
    if (*ret_type == _CXML_XP_PS_POISON) return;
    cxml_for_each(step, &node->steps)
    {
        cxml_xp_ovisit_Step(step, ret_type);
        if (*ret_type == _CXML_XP_PS_POISON) return;
    }
}

void cxml_xp_ovisit_FunctionCall(cxml_xp_functioncall* node, _cxml_xp_ret_t *ret_type){
    // return immediately we discover the expression is no longer optimizable
    if (*ret_type == _CXML_XP_PS_POISON){
        return;
    }
    // function uses the context node in its computation
    else if (_is_poisonous_by_computation(node->pos)){
        *ret_type = _CXML_XP_PS_POISON;
        return;
    }
    // is argument list empty and does it use
    // the context node as its default argument ?
    else if ((cxml_list_is_empty(&node->args)
            && _is_poisonous_by_default_arg(node->pos)))
    {
        *ret_type = _CXML_XP_PS_POISON;
        return;
    }
    cxml_for_each(arg, &node->args){
        cxml_xp_ovisit(arg, ret_type);
        if (*ret_type == _CXML_XP_PS_POISON) return;
    }
    *ret_type = node->ret_type;
}

void cxml_xp_ovisit_UnaryOp(cxml_xp_unaryop* node, _cxml_xp_ret_t *ret_type){
    // return immediately we discover the expression is no longer optimizable
    if (*ret_type == _CXML_XP_PS_POISON) return;
    cxml_xp_ovisit(node->node, ret_type);
}

void cxml_xp_ovisit_BinaryOp(cxml_xp_binaryop* node, _cxml_xp_ret_t *ret_type){
    _cxml_xp_ret_t l_type, r_type;
    // visit left and right operand to ensure we do not have step nodes with `path_spec` = 0
    cxml_xp_ovisit(node->l_node, &l_type);
    if (l_type == _CXML_XP_PS_POISON){
        *ret_type = _CXML_XP_PS_POISON;
        return;
    }
    cxml_xp_ovisit(node->r_node, &r_type);
    if (r_type == _CXML_XP_PS_POISON){
        *ret_type = _CXML_XP_PS_POISON;
        return;
    }
    *ret_type = _compute_type(node->op);
}
