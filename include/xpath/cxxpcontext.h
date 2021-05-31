/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXXPCONTEXT_H
#define CXML_CXXPCONTEXT_H

#include "core/cxstack.h"

// context state
struct _cxml_xp_context_state {
    int ctx_pos;
    int ctx_size;
    void *ctx_node;
};

void _cxml_xp_init_context(struct _cxml_xp_context_state* context);

void _cxml_xp_push_context(
        _cxml_stack *ctx_stack,
        struct _cxml_xp_context_state *curr_ctx,
        struct _cxml_xp_context_state *new_ctx,
        void* ctx_node,
        int ctx_pos,
        int ctx_size);

void _cxml_xp_pop_context(
        _cxml_stack *ctx_stack,
        struct _cxml_xp_context_state *context);

#endif //CXML_CXXPCONTEXT_H
