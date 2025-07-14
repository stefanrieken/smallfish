#include <stdio.h>

#include "../smallfish.h"
#include "../parse.h"
#include "../gc.h"

#include "block.h"
#include "expr.h"
#include "parr.h"
#include "method.h"
#include "dict.h"
#include "prim.h"
#include "string.h"
#include "int.h"

int CT_BLOCK;

bool parse_block(int * ch, WORD * result) {
    if (*ch != '{') return false;
    *ch = read_non_whitespace_char('}');
    bool success = parse_seq(ch, result, '}'); // TODO parse sequence; and what to do if empty?
    *ch = getchar(); // TODO check valid

    if (!success) *result = nil; // allow empty block
    //Object * value = success ? as_obj(*result) : NULL; // allow empty block
    // Wrap expression / sequence / value inside block; to be unwrapped on eval == bind
    *result = tag_obj(add_object(&objects, as_obj(*result), core_types[CT_BLOCK]->type, 0));

    return true;
}

WORD mark_block_cb(WORD val, Object * expr, Object * ctx) {
    gc_mark(tag_obj(as_obj(val)->value.obj), ctx);
    return nil;
}

WORD print_block_cb(WORD val, Object * expr, Object * ctx) {
    printf("{ "); if (as_obj(val)->value.obj != NULL) print_list(tag_obj(as_obj(val)->value.obj), ctx); printf("}");
    return nil;
}

// Eval'ing a block turns it into a closure; i.e. binds it.
// The closure can then be passed on to other functions like
// `define` and `lambda`, which may combine it with an arg list
// (and in case of a method definition, undo its binding); or
// it may be eval'ed directly in `if`s and `loop`s.
WORD eval_block(WORD block, Object * ctx) {
    return make_lambda(nil, tag_obj(as_obj(block)->value.obj), ctx);
}
WORD eval_block_cb(WORD block, Object * this_expr, Object * ctx) {
    return eval_block(block, ctx);
}

WORD eval_sequence_cb(WORD block, Object * this_expr, Object * ctx) {
    return doseq(block, ctx);
}

void block_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Block"), tag_obj(ct->type));
    define(ct->type, string_literal("eval"), make_prim(eval_block_cb));
    define(ct->type, string_literal("mark"), make_prim(mark_block_cb));
    define(ct->type, string_literal("print"), make_prim(print_block_cb));

    ct->parse = parse_block;
};

