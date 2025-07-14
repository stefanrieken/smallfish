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

// Note: this result is NOT tagged!
WORD parse_int_with_radix(int * ch, int radix) {
    int result = 0;
    // Allow letters for numbers 10 and higher, dependent on radix
    int normalized = *ch;
    if (*ch >= 'a' && *ch <= 'z') normalized = '0'+10+(*ch-'a');
    else if (*ch >= 'A' && *ch <= 'Z') normalized = '0'+10+(*ch-'A');
    while(normalized >= '0' && normalized <= '0'+radix) {
        result = (result * radix) + (normalized-'0');
        *ch = getchar();
        normalized = *ch;
        if (*ch >= 'a' && *ch <= 'z') normalized = '0'+10+(*ch-'a');
        else if (*ch >= 'A' && *ch <= 'Z') normalized = '0'+10+(*ch-'A');
        else if (*ch < '0' || *ch > '9') break;
    }
    return result;
}

bool parse_int(int * ch, WORD * result) {
    if (*ch < '0' || *ch > '9') return false; // not an int
    int radix = 10;
    if (*ch == '0') {
        radix = 8; // Allow for octal by way of leading zero
        *ch = getchar();
        if (*ch == 'x') { radix=16; *ch = getchar(); }
        if (*ch == 'b') { radix= 2; *ch = getchar(); }
    }
    *result = tag_int(parse_int_with_radix(ch, radix));
    return true;
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

    ct->parse = parse_int;
};

