/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "xpath/cxxpcontext.h"

/*
 * context informatiom implementation for xpath nodesets
 */
void _cxml_xp_init_context(struct _cxml_xp_context_state* context){
    context->ctx_node = NULL;
    context->ctx_pos = -1;
    context->ctx_size = -1;
}

inline static void _cxml_xp_set_context(
        struct _cxml_xp_context_state *ctx,
        void* ctx_node,
        int ctx_pos,
        int ctx_size)
{
    ctx->ctx_node = ctx_node;
    ctx->ctx_pos = ctx_pos;
    ctx->ctx_size = ctx_size;
}

void _cxml_xp_push_context(
        _cxml_stack *ctx_stack,
        struct _cxml_xp_context_state *curr_ctx,
        struct _cxml_xp_context_state *new_ctx,
        void* ctx_node,
        int ctx_pos,
        int ctx_size)
{
    // initialize a new context, copy the context into the current context
    // and push the new context on the stack
    _cxml_xp_set_context(new_ctx, ctx_node, ctx_pos, ctx_size);
    *curr_ctx = *new_ctx;
    _cxml_stack__push(ctx_stack, new_ctx);
}

void _cxml_xp_pop_context(
        _cxml_stack *ctx_stack,
        struct _cxml_xp_context_state *context)
{
    // pop the current context off the stack
    _cxml_stack__pop(ctx_stack);
    // get the current context on the stack
    struct _cxml_xp_context_state *ctx = _cxml_stack__get(ctx_stack);
    // if not null, set `context` to it
    if (ctx){
        _cxml_xp_set_context(context, ctx->ctx_node, ctx->ctx_pos, ctx->ctx_size);
    }else{
        _cxml_xp_init_context(context);
    }
}
