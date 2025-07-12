#include <stdio.h>

#include "../smallfish.h"
#include "../gc.h"

#include "dict.h"
#include "prim.h"
#include "string.h"

int CT_BOOL;
int CT_TRUE;
int CT_FALSE;

WORD if_true_cb(WORD obj, Object * expr, Object * ctx) {
    if (expr->size / sizeof(WORD) < 3) return nil;
    return eval(expr->value.ws[2], ctx);
}

WORD if_false_cb(WORD obj, Object * expr, Object * ctx) {
    if (expr->size / sizeof(WORD) < 4) return nil;
    return eval(expr->value.ws[3], ctx);
}

void bool_core_type(CoreType * ct, Object * ctx) {
    Object * type = ct->type;
    set_parent(ct->type, core_types[CT_DICT]->type); // Retain Dictionary abilities for True & False objects
    define(ctx, string_literal("Bool"), tag_obj(type));
}

void true_core_type(CoreType * ct, Object * ctx) {
    Object * type = ct->type;
    set_parent(ct->type, core_types[CT_BOOL]->type);
    define(ctx, string_literal("True"), tag_obj(type));
    type->type = tag_obj(type); // Make its own type
    define(type, string_literal("?"), make_prim(if_true_cb));
}

void false_core_type(CoreType * ct, Object * ctx) {
    Object * type = ct->type;
    set_parent(ct->type, core_types[CT_BOOL]->type);
    define(ctx, string_literal("False"), tag_obj(type));
    type->type = tag_obj(type); // Make its own type
    define(type, string_literal("?"), make_prim(if_false_cb));
}