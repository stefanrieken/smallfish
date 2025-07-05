#include <stdio.h>

#include "../smallfish.h"
#include "../gc.h"

#include "dict.h"
#include "prim.h"
#include "string.h"

int CT_INT;

WORD  plus_cb(Object * ctx, WORD obj, WORD arg1) {  return tag_int(as_int(obj)+as_int(eval(arg1, ctx))); }
WORD   min_cb(Object * ctx, WORD obj, WORD arg1) {  return tag_int(as_int(obj)-as_int(eval(arg1, ctx))); }
WORD times_cb(Object * ctx, WORD obj, WORD arg1) {  return tag_int(as_int(obj)*as_int(eval(arg1, ctx))); }
WORD   div_cb(Object * ctx, WORD obj, WORD arg1) {  return tag_int(as_int(obj)/as_int(eval(arg1, ctx))); }
WORD remnd_cb(Object * ctx, WORD obj, WORD arg1) {  return tag_int(as_int(obj)%as_int(eval(arg1, ctx))); }

WORD  andb_cb(Object * ctx, WORD obj, WORD arg1) {  return tag_int(as_int(obj)&as_int(eval(arg1, ctx))); }
WORD   orb_cb(Object * ctx, WORD obj, WORD arg1) {  return tag_int(as_int(obj)|as_int(eval(arg1, ctx))); }
WORD   xor_cb(Object * ctx, WORD obj, WORD arg1) {  return tag_int(as_int(obj)^as_int(eval(arg1, ctx))); }
WORD  notb_cb(Object * ctx, WORD obj) {  return tag_int(~as_int(obj)); }

void print_int(WORD val, Object * ctx) {
    printf("%d", as_int(val));
}
void print_int_cb(Object * ctx, WORD val) {
    return print_int(val, ctx);
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

    ct->apply = apply_to_self; // for now at least; but there are interesting ways to apply ints
    ct->parse = parse_int1;
};

