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

void print_int(WORD val) {
    printf("%d", as_int(val));
}

CoreType * int_core_type(Object * ctx) {
    Object * type = make_class(ctx, "Int", nil, nil);
    define(type, string_literal("+"), make_prim( plus_cb), true);
    define(type, string_literal("-"), make_prim(  min_cb), true);
    define(type, string_literal("*"), make_prim(times_cb), true);
    define(type, string_literal("/"), make_prim(  div_cb), true);
    define(type, string_literal("%"), make_prim(remnd_cb), true);

    CoreType * result = allocate(CoreType, 1);
    result->type = type;
    result->eval = eval_to_self;
    result->apply = NULL; // for now at least; but there are interesting ways to apply ints
    result->print = print_int;
    result->mark = NULL; // not a pointer
    return result;
};

