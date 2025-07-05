#include <stdio.h>

#include "../smallfish.h"
#include "../parse.h"
#include "../gc.h"

#include "expr.h"
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
    // allow single-value (sub)expression to be just the value
    if (idx == 1 && until == ')') { WORD result = expr[0]; free(expr); return result; }

    return tag_obj(add_object(&objects, expr, core_types[CT_EXPR]->type, sizeof(WORD) * size));
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

    as_obj(*result)->type = tag_obj(core_types[CT_PARR]->type);
    return true;
}

void gc_mark_obj_array(Object * obj_array, Object * ctx) {
    for (int i=0; i<obj_array->size/sizeof(WORD);i ++) {
        gc_mark(obj_array->value.ws[i], ctx);
    }
}

WORD mark_array_cb(Object * ctx, WORD val) {
    gc_mark_obj_array(as_obj(val), ctx);
    return nil;
}

void print_list(WORD val, Object * ctx) {
    Object * expr = as_obj(val);
    for(int i=0; i<expr->size/sizeof(WORD);i++) {
        print_val(expr->value.ws[i], ctx);printf(" ");
    }
}

void print_parr_cb(Object * ctx, WORD val) {
    printf("[ "); print_list(val, ctx); printf("]");
}

void print_expr_cb(Object * ctx, WORD val) {
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

    Object * type = is_int(obj) ? core_types[CT_INT]->type : as_obj(as_obj(obj)->type);//core_types[as_obj(obj)->type];
    DictEntry * entry = lookup(type, name);
    if (entry == NULL) { printf ("Method %s not found\n", as_obj(name)->value.str); return nil; }

    // apply
    // TODO: this is the same code snippet as in `message1`; defer both to a centralized `apply` instead.

    // Assume for now: method is always either CT_METH or CT_PRIM
    // We could support other core type methods, but then the simplest way to support them is
    // to directly add their evaluators here.
    WORD meth = entry->value;
    if (as_obj(meth)->type == tag_obj(core_types[CT_PRIM]->type)) return apply_prim(meth, obj, expr, ctx);
    if (as_obj(meth)->type == tag_obj(core_types[CT_METH]->type)) return apply_method(meth, obj, expr, ctx);
    // If type is neither prim nor meth, consider looking up an 'apply' function on the method's type.
    // Note: this can become  a bit of a recursive rabbit hole, so let's leave it out for now.
//    Object * mtype = is_int(meth) ? core_types[CT_INT] : as_obj(as_obj(meth)->type);
//    return message1(obj, STR_APPLY, ctx);
    printf("Can't eval method type\n");
    return nil;
}

WORD eval_expr_cb(Object * ctx, WORD expr) {
    return eval_expr(expr, ctx);
}

void parr_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Array"), tag_obj(ct->type));
    define(ct->type, string_literal("mark"), make_prim(mark_array_cb));
    define(ct->type, string_literal("print"), make_prim(print_parr_cb));

    ct->parse = parse_array;
};

void expr_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Expression"), tag_obj(ct->type));
    set_parent(ct->type, core_types[CT_PARR]->type);

    define(ct->type, string_literal("print"), make_prim(print_expr_cb));
    define(ct->type, string_literal("eval"), make_prim(eval_expr_cb));

    ct->apply = NULL; // for now; but TODO expr == native code?
    ct->parse = parse_subexpr;
};

WORD make_method(WORD args, WORD body) {
    WORD * method = allocate(WORD, 2);
    method[0] = args;
    method[1] = body;
    return tag_obj(add_object(&objects, method, core_types[CT_METH]->type, sizeof(WORD) * 2));
}

WORD apply_method(WORD msg, WORD obj, Object * expr, Object * caller_ctx) {
    Object * meth = as_obj(msg);
    Object * args = as_obj(meth->value.ws[0]); // = actually argnames
    WORD body = meth->value.ws[1];
//printf("Num args: %d\n", args->size / sizeof(WORD));
    Object * ctx = add_object(&objects, allocate(uint8_t, (args->size+sizeof(WORD))*2), core_types[CT_DICT]->type, (args->size+sizeof(WORD))*2);
    // Set obj argument as search parent, ONLY IF it is a dict (TODO change lookup so that it can be a named parent)
    ctx->value.dict[0].name = nil;
    ctx->value.dict[0].value = obj == tag_obj(core_types[CT_DICT]->type) ? obj : nil;
    // Set obj argument (TODO merge with above)
//printf("Setting name %s\n", as_obj(args->value.ws[0])->value.str);
    ctx->value.dict[1].name = args->value.ws[0];
    ctx->value.dict[1].value = obj;
    // Add rest of arguments
    for (int i=1; i<args->size/sizeof(WORD); i++) {
//printf("Setting name %s\n", as_obj(args->value.ws[i])->value.str);
        ctx->value.dict[i+1].name = args->value.ws[i];
        ctx->value.dict[i+1].value = eval(expr->value.ws[i+1], caller_ctx);
    }
//printf("Eval'ing method body\n");
    return eval(body, ctx);
}

void print_meth_cb(Object * ctx, WORD val) {
    printf("method "); print_list(val, ctx);
}

void meth_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Method"), tag_obj(ct->type));
    set_parent(ct->type, core_types[CT_PARR]->type);
    define(ct->type, string_literal("print"), make_prim(print_meth_cb));

    ct->apply = apply_method;
}
