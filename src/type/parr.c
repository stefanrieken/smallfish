#include <stdio.h>

#include "../smallfish.h"
#include "../parse.h"
#include "../gc.h"

#include "method.h"
#include "parr.h"
#include "expr.h"
#include "dict.h"
#include "prim.h"
#include "string.h"
#include "int.h"

int CT_PARR;

// Was adapted from parse_expr
bool parse_arr1(int * ch, WORD * result, char until) {
    int size = 0; // Not presently feasible to pre-alloc larger than used
    int idx = 0; // have none yet

    WORD * expr = NULL; 

    while (*ch == '#') { while (*ch != EOF && *ch != '\n') *ch = getchar(); *ch = getchar(); }

    // Note: 'until' is essentially just '\n' for REPL, or non-applicable otherwise
    // We should simply always recognize all common expression-delimiting characters (brackets, ';', commas)
    WORD o;
    while (*ch != EOF && *ch != ';' && *ch != until && parse_object(ch, &o)) {
        if (idx == size) expr = reallocate(expr, WORD, ++size);
        expr[idx++] = o;
        if (*ch != EOF && *ch != until && is_whitespace_char(*ch)) *ch = read_non_whitespace_char(until);
    }

    if (idx == 0) { *result = nil; return false; } // no result
    // allow single-value (sub)expression to be just the value
//    if (idx == 1 /*&& until == ')'*/) { *result = expr[0]; free(expr); return true; }

    *result = tag_obj(add_object(&objects, expr, core_types[CT_EXPR]->type, sizeof(WORD) * size));
    return true;
}

bool parse_array(int * ch, WORD * result) {
    if (*ch != '[') return false;
    *ch = read_non_whitespace_char(']');
    bool success =
    parse_arr1(ch, result, ']');
    *ch = getchar();

    
    if (success) as_obj(*result)->type = tag_obj(core_types[CT_PARR]->type);
    else *result = tag_obj(add_object(&objects, NULL, core_types[CT_PARR]->type, 0)); // new empty array
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

// Input is an array of expressions, outcome is an array of values
WORD eval_array(WORD val, Object * ctx) {
    Object *array = as_obj(val);
    WORD * result = allocate(WORD, array->size/sizeof(WORD));
    for(int i=0;i<array->size/sizeof(WORD);i++) {
        result[i] = eval(array->value.ws[i], ctx);
    }
    return tag_obj(add_object(&objects, result, core_types[CT_PARR]->type, array->size));
}

WORD eval_array_cb(WORD val, Object * this_expr, Object * ctx) {
    return eval_array(val, ctx);
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

WORD parr_get_cb(WORD val, Object * expr, Object * ctx) {
    WORD idx = eval(expr->value.ws[2], ctx);
    return as_obj(val)->value.ws[as_int(idx)];
}

WORD foreach_parr_cb(WORD val, Object * expr, Object * ctx) {
    Object * lambda = as_obj(eval(expr->value.ws[2], ctx));
    if (as_obj(lambda->type) != core_types[CT_LAMBDA]->type) { printf("Error: expected a callback\n"); return nil; }
    Object * arglist = as_obj(lambda->value.ws[0]);
    if (arglist == objects) { printf("Error: callback needs arg\n"); return nil; }

    for (int i=0;i<as_obj(val)->size/sizeof(WORD);i++) {
        // Not re-using the ctx is obviously not fast, but first just make it work
        Object * lambda_ctx = make_dict(lambda->value.ws[2], core_types[CT_DICT]->type);
        //DictEntry entry =
        define(lambda_ctx, as_obj(arglist->value.ws[0]), as_obj(val)->value.ws[i]);
        eval(lambda->value.ws[1], lambda_ctx);
    }
    return nil;
}

void parr_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Array"), tag_obj(ct->type));
    define(ct->type, string_literal("mark"), make_prim(mark_array_cb));
    define(ct->type, string_literal("eval"), make_prim(eval_array_cb));
    define(ct->type, string_literal("print"), make_prim(print_parr_cb));
    define(ct->type, string_literal("get"), make_prim(parr_get_cb));
    define(ct->type, string_literal("foreach"), make_prim(foreach_parr_cb));

    ct->parse = parse_array;
};

