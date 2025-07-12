#include <stdio.h>

#include "../smallfish.h"
#include "../gc.h"

#include "dict.h"
#include "prim.h"
#include "string.h"
#include "bool.h"

int CT_INT;

WORD  plus_cb(WORD obj, Object * expr, Object * ctx) {  return tag_int(as_int(obj)+as_int(eval(expr->value.ws[2], ctx))); }
WORD   min_cb(WORD obj, Object * expr, Object * ctx) {  return tag_int(as_int(obj)-as_int(eval(expr->value.ws[2], ctx))); }
WORD times_cb(WORD obj, Object * expr, Object * ctx) {  return tag_int(as_int(obj)*as_int(eval(expr->value.ws[2], ctx))); }
WORD   div_cb(WORD obj, Object * expr, Object * ctx) {  return tag_int(as_int(obj)/as_int(eval(expr->value.ws[2], ctx))); }
WORD remnd_cb(WORD obj, Object * expr, Object * ctx) {  return tag_int(as_int(obj)%as_int(eval(expr->value.ws[2], ctx))); }

WORD  andb_cb(WORD obj, Object * expr, Object * ctx) {  return tag_int(as_int(obj)&as_int(eval(expr->value.ws[2], ctx))); }
WORD   orb_cb(WORD obj, Object * expr, Object * ctx) {  return tag_int(as_int(obj)|as_int(eval(expr->value.ws[2], ctx))); }
WORD   xor_cb(WORD obj, Object * expr, Object * ctx) {  return tag_int(as_int(obj)^as_int(eval(expr->value.ws[2], ctx))); }
WORD  notb_cb(WORD obj, Object * expr, Object * ctx) {  return tag_int(~as_int(obj)); }

WORD  lt_cb(WORD obj, Object * expr, Object * ctx) {  return tag_obj(as_int(obj)<as_int(eval(expr->value.ws[2], ctx)) ? core_types[CT_TRUE]->type : core_types[CT_FALSE]->type); };
WORD  gt_cb(WORD obj, Object * expr, Object * ctx) {  return tag_obj(as_int(obj)>as_int(eval(expr->value.ws[2], ctx)) ? core_types[CT_TRUE]->type : core_types[CT_FALSE]->type); };
WORD lte_cb(WORD obj, Object * expr, Object * ctx) {  return tag_obj(as_int(obj)<=as_int(eval(expr->value.ws[2], ctx)) ? core_types[CT_TRUE]->type : core_types[CT_FALSE]->type); };
WORD gte_cb(WORD obj, Object * expr, Object * ctx) {  return tag_obj(as_int(obj)>=as_int(eval(expr->value.ws[2], ctx)) ? core_types[CT_TRUE]->type : core_types[CT_FALSE]->type); };
WORD  eq_cb(WORD obj, Object * expr, Object * ctx) {  return tag_obj(as_int(obj)==as_int(eval(expr->value.ws[2], ctx)) ? core_types[CT_TRUE]->type : core_types[CT_FALSE]->type); };

void print_int(WORD val, Object * ctx) {
    printf("%d", as_int(val));
}
WORD print_int_cb(WORD obj, Object * expr, Object * ctx) {
    print_int(obj, ctx);
    return nil;
}

bool parse_int1(int * ch, WORD * result) {
    bool success = false;
    WORD value = 0;
    while (*ch >= '0' && *ch <= '9') {
        success = true;
        value = value * 10 + (*ch - '0');
        *ch = getchar();
    }
    *result = tag_int(value);
    return success;
}

void int_core_type(CoreType * ct, Object * ctx) {
    Object * type = ct->type;
    define(ctx, string_literal("Int"), tag_obj(type));
    define(type, string_literal("print"), make_prim(print_int_cb));
    define(type, string_literal("+"), make_prim( plus_cb));
    define(type, string_literal("-"), make_prim(  min_cb));
    define(type, string_literal("*"), make_prim(times_cb));
    define(type, string_literal("/"), make_prim(  div_cb));
    define(type, string_literal("%"), make_prim(remnd_cb));
    define(type, string_literal("&"), make_prim( andb_cb));
    define(type, string_literal("|"), make_prim(  orb_cb));
    define(type, string_literal("^"), make_prim(  xor_cb));
    define(type, string_literal("~"), make_prim( notb_cb));
    define(type, string_literal("<"), make_prim( lt_cb));
    define(type, string_literal(">"), make_prim( gt_cb));
    define(type, string_literal("<="), make_prim( lte_cb));
    define(type, string_literal(">="), make_prim( gte_cb));
    define(type, string_literal("=="), make_prim( eq_cb));

//    ct->apply = apply_to_self; // for now at least; but there are interesting ways to apply ints
    ct->parse = parse_int1;
};

