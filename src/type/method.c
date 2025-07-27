#include <stdio.h>

#include "../smallfish.h"
#include "../parse.h"
#include "../gc.h"

#include "expr.h"
#include "parr.h"
#include "dict.h"
#include "prim.h"
#include "string.h"
#include "int.h"
#include "block.h"
#include "bool.h"

int CT_METH;
int CT_LAMBDA;

WORD make_method(WORD args, WORD body, Object * ctx) {
    WORD * method = allocate(WORD, 2);
    method[0] = eval(args, ctx); // If args is an array potentially containing expressions, then eval(args) is an array containing the resulting values
    method[1] = body; // TODO body arg should be evaluable too. If block, (eval block) -> (bind env block) -> lambda closure?
    return tag_obj(add_object(&objects, method, core_types[CT_METH]->type, sizeof(WORD) * 2));
}

// More precisely, makes a lambda closure
WORD make_lambda(WORD args, WORD body, Object * ctx) {
    WORD * lambda = allocate(WORD, 3);
    lambda[0] = eval(args, ctx); // If args is an array potentially containing expressions, then eval(args) is an array containing the resulting values
    lambda[1] = body; // Body is (likely) already eval'ed at this point
    lambda[2] = tag_obj(ctx);
    return tag_obj(add_object(&objects, lambda, core_types[CT_LAMBDA]->type, sizeof(WORD) * 2));
}

WORD apply_native(WORD val, WORD obj, Object * expr, Object * caller_ctx, bool is_method) {
    Object * meth = as_obj(val);
    Object * args = as_obj(meth->value.ws[0]); // = actually argnames
    WORD body = meth->value.ws[1];

    Object * ctx = make_dict(is_method ? (as_obj(obj)->compound ? obj : nil) : meth->value.ws[2], core_types[CT_DICT]->type);
    // add_object(&objects, allocate(uint8_t, (args->size+sizeof(WORD))*2), core_types[CT_DICT]->type, (args->size+sizeof(WORD))*2);

    if (is_method) {
        // Set obj argument
        define(ctx, as_obj(as_label(args->value.ws[0])), obj);
    }

    if (args != objects) { // = nil
        // Add rest of arguments
        int i=is_method; // TODO ONLY when calling via .apply
        for (; i<args->size/sizeof(WORD) && i < expr->size/sizeof(WORD)-1-!is_method; i++) {
            define(ctx, as_obj(as_label(args->value.ws[i])), eval(expr->value.ws[i+1+!is_method], caller_ctx));
        }
        // Add expected args not in expr
        for(;i<args->size/sizeof(WORD); i++) {
            define(ctx, as_obj(as_label(args->value.ws[i])), nil);
        }
    }

    define(ctx, as_obj(STR_ENV), tag_obj(ctx));

    // If our body is a block literal, that block evals to a bound lambda.
    // Then eval'ing that lambda brings us right back here.
    // Technically the block should eval to a bound lambda with a Sequence as its body
    // But that detail is still TODO, so this is how we solve it for now.
    if (as_obj(as_obj(body)->type) == core_types[CT_BLOCK]->type) return doseq(body, ctx);
    else return eval(body, ctx);
}

// To evaluate an expression of the type `obj message arg1 arg2`,
// we lookup the message; if type is native method, this function is called.
WORD apply_method(WORD msg, WORD obj, Object * expr, Object * caller_ctx) {
    return apply_native(msg, obj, expr, caller_ctx, true);
}

// To call a lambda as if it were a method
WORD apply_lambda(WORD msg, WORD obj, Object * expr, Object * caller_ctx) {
    return apply_native(msg, obj, expr, caller_ctx, false);
}

// To evaluate an expression of the type `lambda apply arg1 arg2`,
// this `apply` primitive is invoked, either from native code or from
// another primitive for something like `if`.
// TODO technically speaking this is a `funcall`, not an `apply`.
// Even though none of our `apply` methods cleanly combine a resolved method
// with its arguments, but rather with the expression from which is was resolved,
// the resulting `expr` argument layout is still different from this one.
// We detect and make up for that difference in apply_native, but wwe may also
// want to further acknowlege it in choice of naming.
WORD apply_lambda_cb(WORD lambda, Object * expr, Object * caller_ctx) {
    return apply_native(lambda, nil, expr, caller_ctx, false);
}

WORD eval_lambda(WORD lambda, Object * caller_ctx) {
    return apply_native(lambda, nil, NULL, caller_ctx, false);
}

WORD eval_lambda_cb(WORD lambda, Object * expr, Object * ctx) {
    return eval_lambda(lambda, ctx);
}

WORD loop_cb(WORD lambda, Object * expr, Object * ctx) {
    WORD result;
    do {
       result =
           doseq(as_obj(lambda)->value.ws[1], ctx);
    } while (as_obj(as_obj(result)->type) == core_types[CT_TRUE]->type);
    return nil;
}

WORD print_meth_cb(WORD val, Object * expr, Object * ctx) {
    printf("method "); print_list(val, ctx);
    return nil;
}

WORD print_lambda_cb(WORD val, Object * expr, Object * ctx) {
    printf("lambda "); print_list(val, ctx);
    return nil;
}

void meth_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Method"), tag_obj(ct->type));
    set_parent(ct->type, core_types[CT_PARR]->type);
    define(ct->type, string_literal("print"), make_prim(print_meth_cb));

//    ct->apply = apply_method;
}

void lambda_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Lambda"), tag_obj(ct->type));
    set_parent(ct->type, core_types[CT_PARR]->type);
    define(ct->type, string_literal("eval"), make_prim(eval_lambda_cb));
    define(ct->type, string_literal("print"), make_prim(print_lambda_cb));
    define(ct->type, string_literal("apply"), make_prim(apply_lambda_cb));
    define(ct->type, string_literal("loop"), make_prim(loop_cb));

//    ct->apply = apply_method;
}

