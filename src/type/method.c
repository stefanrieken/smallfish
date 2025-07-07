#include <stdio.h>

#include "../smallfish.h"
#include "../parse.h"
#include "../gc.h"

#include "expr.h"
#include "parr.h"
#include "dict.h"
#include "prim.h"
#include "string.h"
#include "int.h"

int CT_METH;

WORD make_method(WORD args, WORD body, Object * ctx) {
    WORD * method = allocate(WORD, 2);
    method[0] = eval(args, ctx); // If args is an array potentially containing expressions, then eval(args) is an array containing the resulting values
    method[1] = body; // TODO body arg should be evaluable too. If block, (eval block) -> (bind env block) -> lambda closure?
    return tag_obj(add_object(&objects, method, core_types[CT_METH]->type, sizeof(WORD) * 2));
}

WORD apply_method(WORD msg, WORD obj, Object * expr, Object * caller_ctx) {
    Object * meth = as_obj(msg);
    Object * args = as_obj(meth->value.ws[0]); // = actually argnames
    WORD body = meth->value.ws[1];
//printf("Num args: %d\n", args->size / sizeof(WORD));
    Object * ctx = add_object(&objects, allocate(uint8_t, (args->size+sizeof(WORD))*2), core_types[CT_DICT]->type, (args->size+sizeof(WORD))*2);
    // Set obj argument as search parent, ONLY IF it is a dict (TODO change lookup so that it can be a named parent)
    ctx->value.dict[0].name = nil;
    ctx->value.dict[0].value = obj == tag_obj(core_types[CT_DICT]->type) ? obj : nil;
    // Set obj argument
//printf("Setting name %s\n", as_obj(args->value.ws[0])->value.str);
    ctx->value.dict[1].name = as_label(args->value.ws[0]);
    ctx->value.dict[1].value = obj;
    // Add rest of arguments
    for (int i=1; i<args->size/sizeof(WORD); i++) {
//printf("Setting name %s\n", as_obj(args->value.ws[i])->value.str);
        ctx->value.dict[i+1].name = as_label(args->value.ws[i]);
        ctx->value.dict[i+1].value = eval(expr->value.ws[i+1], caller_ctx);
    }
//printf("Eval'ing method body\n");
    return eval(body, ctx);
}

void print_meth_cb(Object * ctx, WORD val) {
    printf("method "); print_list(val, ctx);
}

void meth_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Method"), tag_obj(ct->type));
    set_parent(ct->type, core_types[CT_PARR]->type);
    define(ct->type, string_literal("print"), make_prim(print_meth_cb));

    ct->apply = apply_method;
}
