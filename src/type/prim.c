#include <stdio.h>

#include "../smallfish.h"
#include "../gc.h"

#include "dict.h"
#include "string.h"
#include "prim.h"

int CT_PRIM;

void print_prim(WORD val, Object * ctx) {
    printf("(primitive)");
}

WORD apply_prim(WORD meth, WORD obj, Object * expr, Object * ctx) {
    Object * prim = as_obj(meth);
    PrimitiveCb cb = (PrimitiveCb) prim->value.ptr;
    return cb(obj, expr, ctx);
/*
    WORD arg1 = (expr != NULL && expr->size > (sizeof(WORD) * 2)) ? expr->value.ws[2] : nil;
    WORD arg2 = (expr != NULL && expr->size > (sizeof(WORD) * 3)) ? expr->value.ws[3] : nil;
    WORD arg3 = (expr != NULL && expr->size > (sizeof(WORD) * 4)) ? expr->value.ws[4] : nil;
    return cb(ctx, obj, arg1, arg2, arg3);
    */
}

WORD make_prim(PrimitiveCb cb) {
    return tag_obj(add_object(&objects, cb, core_types[CT_PRIM]->type, 0));
}

WORD apply_cb(WORD obj, Object * expr, Object * ctx) {
    // Assume 1-arg expr to keep this simple
    return apply_prim(obj, eval(expr->value.ws[2], ctx), NULL, ctx);
}

void prim_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Primitive"), tag_obj(ct->type));
    define(ct->type, string_literal("apply"), make_prim(apply_cb));

    ct->apply = apply_prim;
};

