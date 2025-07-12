#include <stdio.h>

#include "../smallfish.h"
#include "../parse.h"
#include "../gc.h"

#include "parr.h"
#include "expr.h"
#include "dict.h"
#include "prim.h"
#include "string.h"
#include "int.h"

int CT_PARR;

bool parse_array(int * ch, WORD * result) {
    if (*ch != '[') return false;
    *ch = read_non_whitespace_char(']');
    *result = parse_expr(ch, ']');
    *ch = getchar();

    as_obj(*result)->type = tag_obj(core_types[CT_PARR]->type);
    return true;
}

void gc_mark_obj_array(Object * obj_array, Object * ctx) {
    for (int i=0; i<obj_array->size/sizeof(WORD);i ++) {
        gc_mark(obj_array->value.ws[i], ctx);
    }
}

WORD mark_array_cb(WORD val, Object * expr, Object * ctx) {
    gc_mark_obj_array(as_obj(val), ctx);
    return nil;
}

void print_list(WORD val, Object * ctx) {
    Object * expr = as_obj(val);
    for(int i=0; i<expr->size/sizeof(WORD);i++) {
        print_val(expr->value.ws[i], ctx);printf(" ");
    }
}

WORD print_parr_cb(WORD val, Object * expr, Object * ctx) {
    printf("[ "); print_list(val, ctx); printf("]");
    return nil;
}

void parr_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Array"), tag_obj(ct->type));
    define(ct->type, string_literal("mark"), make_prim(mark_array_cb));
    define(ct->type, string_literal("print"), make_prim(print_parr_cb));

    ct->parse = parse_array;
};

