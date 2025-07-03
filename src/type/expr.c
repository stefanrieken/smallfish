#include <stdio.h>

#include "../smallfish.h"
#include "../parse.h"
#include "../gc.h"

#include "dict.h"
#include "prim.h"
#include "string.h"
#include "int.h"

int CT_PARR;
int CT_EXPR;
int CT_METH;

// the CoreType struct based version

// toplevel (to call by REPL, block, etc.)
// NOTE: if expr size == 1, just return value
WORD parse_expr(int * ch, char until) {
    int size = 0; // Not presently feasible to pre-alloc larger than used
    int idx = 0; // have none yet
    WORD * expr = NULL; 

    while (*ch != EOF && *ch != until) {
        if (idx == size) expr = reallocate(expr, WORD, ++size);
        expr[idx++] = parse_object(ch);
        if (*ch != EOF && *ch != until && is_whitespace_char(*ch)) *ch = read_non_whitespace_char(until);
    }
    if (idx == 0) { return nil; } // no result
    if (idx == 1) { WORD result = expr[0]; free(expr); return result; }

    return tag_obj(add_object(&objects, expr, CT_EXPR, core_types[CT_EXPR]->type, sizeof(WORD) * size));
}

// Provide this function in the CoreType struct, as
// subexpressions are what may appear at any object position.
bool parse_subexpr(int * ch, WORD * result) {
    if (*ch != '(') return false;
    *ch = read_non_whitespace_char(')');
    *result = parse_expr(ch, ')');
    *ch = getchar();
    return true;
}

bool parse_array(int * ch, WORD * result) {
    if (*ch != '[') return false;
    *ch = read_non_whitespace_char(']');
    *result = parse_expr(ch, ']');
    *ch = getchar();

    as_obj(*result)->type = CT_PARR;
    return true;
}

void gc_mark_obj_array(Object * obj_array) {
    for (int i=0; i<obj_array->size;i += sizeof(WORD)) {
        gc_mark(obj_array->value.ws[i]);
    }
}

void print_list(WORD val, Object * ctx) {
    Object * expr = as_obj(val);
    for(int i=0; i<expr->size/sizeof(WORD);i++) {
        print_val(expr->value.ws[i], ctx);printf(" ");
    }
}

void print_parr(WORD val, Object * ctx) {
    printf("[ "); print_list(val, ctx); printf("]");
}

void print_expr(WORD val, Object * ctx) {
    printf("( "); print_list(val, ctx); printf(")");
}

#define OBJ_POS 0
#define MSG_POS 1
WORD eval_expr(WORD val, Object * ctx) {
    Object * expr = as_obj(val);

    if (expr->size == 0) return val; // nothing useful here, maybe return nil?   

    WORD obj = eval(expr->value.ws[OBJ_POS], ctx);
    if (expr->size == 1*sizeof(WORD)) return obj;
    WORD name = expr->value.ws[MSG_POS];

    CoreType * type = is_int(obj) ? core_types[CT_INT] : core_types[as_obj(obj)->type];
    // TODO look up specific user-defined class
    DictEntry * entry = lookup(type->type, name);
    if (entry == NULL) { printf ("%s not found\n", as_obj(name)->value.str); return nil; }

    // apply
    WORD meth = entry->value;
    //printf("Method %d %d type: %d\n", meth, is_int(meth), is_int(meth) ? CT_INT : as_obj(meth)->type);
    CoreType * mtype = is_int(meth) ? core_types[CT_INT] : core_types[as_obj(meth)->type];
    //printf("Apply method: %p\n", mtype->apply);
    return mtype->apply(meth, obj, expr, ctx);

}

WORD eval_expr_cb(Object * ctx, WORD expr) {
    return eval_expr(expr, ctx);
}

void parr_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Array"), tag_obj(ct->type));

    ct->eval = eval_to_self;
    ct->parse = parse_array;
    ct->print = print_parr;
    ct->mark = gc_mark_obj_array;
};

void expr_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Expression"), tag_obj(ct->type));
    set_parent(ct->type, core_types[CT_PARR]->type);

    define(ct->type, string_literal("eval"), make_prim(eval_expr_cb));

    ct->eval = eval_expr;
    ct->apply = NULL; // for now; but TODO expr == native code?
    ct->parse = parse_subexpr;
    ct->print = print_expr;
    ct->mark = gc_mark_obj_array;
};

WORD make_method(WORD args, WORD body) {
    WORD * method = allocate(WORD, 2);
    method[0] = args;
    method[1] = body;
    return tag_obj(add_object(&objects, method, CT_METH, core_types[CT_METH]->type, sizeof(WORD) * 2));
}

WORD apply_method(WORD msg, WORD obj, Object * expr, Object * caller_ctx) {
    Object * meth = as_obj(msg);
    Object * args = as_obj(meth->value.ws[0]);
    WORD body = meth->value.ws[1];
printf("Args size: %d\n", args->size);
    Object * ctx = add_object(&objects, allocate(uint8_t, args->size*2), CT_DICT, core_types[CT_DICT]->type, args->size*2);
    // TODO finally properly work out dict parent, type,
    // otherwise we also cannot access instance variables
    ctx->value.dict[0].name = nil;
    ctx->value.dict[0].value = nil;
    ctx->value.dict[1].name = args->value.ws[0];
    ctx->value.dict[1].value = expr->value.ws[OBJ_POS];

    for (int i=1; i<args->size/sizeof(WORD); i++) {
        ctx->value.dict[i+1].name = args->value.ws[i];
        ctx->value.dict[i+1].value = eval(expr->value.ws[i+1], caller_ctx);
    }

    return eval(body, ctx);
}

void print_method(WORD val, Object * ctx) {
    printf("(a method)");
}

void meth_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Method"), tag_obj(ct->type));
    set_parent(ct->type, core_types[CT_PARR]->type);

    ct->eval = eval_to_self;
    ct->apply = apply_method;
    ct->print = print_method;
    ct->mark = gc_mark_obj_array;
}
