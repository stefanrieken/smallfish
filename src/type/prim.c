#include <stdio.h>

#include "../smallfish.h"
#include "../gc.h"

#include "dict.h"
#include "string.h"
#include "prim.h"

int CT_PRIM;

void print_prim(WORD val) {
    printf("(primitive)");
}

WORD apply_prim(WORD msg, WORD obj, Object * expr, Object * ctx) {
    Object * prim = as_obj(msg);
    PrimitiveCb cb = (PrimitiveCb) prim->value.ptr;
    WORD arg1 = (expr != NULL && expr->size > (sizeof(WORD) * 2)) ? expr->value.ws[2] : nil;
    WORD arg2 = (expr != NULL && expr->size > (sizeof(WORD) * 3)) ? expr->value.ws[3] : nil;
    WORD arg3 = (expr != NULL && expr->size > (sizeof(WORD) * 4)) ? expr->value.ws[4] : nil;
    return cb(ctx, obj, arg1, arg2, arg3);
}

WORD make_prim(void * cb) {
    return tag_obj(add_object(&objects, cb, CT_PRIM, core_types[CT_PRIM]->type, 0));
}

WORD apply_cb(Object * ctx, WORD prim, WORD obj, WORD arg1, WORD arg2, WORD arg3) {
    Object * primitive = as_obj(prim);
    PrimitiveCb cb = (PrimitiveCb) primitive->value.ptr;
    return cb(ctx, obj, arg1, arg2, arg3);
}

void prim_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Primitive"), tag_obj(ct->type));
    define(ct->type, string_literal("apply"), make_prim(apply_cb));

    ct->eval = eval_to_self;
    ct->apply = apply_prim;
    ct->print = print_prim;
    ct->mark = gc_mark_none; // keep primitives
};

