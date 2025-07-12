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
int CT_SEQUENCE;

bool parse_block(int * ch, WORD * result) {
    if (*ch != '{') return false;
    *ch = read_non_whitespace_char('}');
    *result = parse_expr(ch, '}'); // TODO should also return on '}' or other closing chars
    *ch = getchar();

    as_obj(*result)->type = tag_obj(core_types[CT_BLOCK]->type);

    return true;
}

WORD print_block_cb(WORD val, Object * expr, Object * ctx) {
    printf("{ "); print_list(val, ctx); printf("}");
    return nil;
}

// Eval'ing a block turns it into a closure; i.e. binds it.
// The closure can then be passed on to other functions like
// `define` and `lambda`, which may combine it with an arg list
// (and in case of a method definition, undo its binding); or
// it may be eval'ed directly in `if`s and `loop`s.
WORD eval_block(WORD block, Object * ctx) {
//printf("Eval'ing block\n");
    return make_lambda(nil, block, ctx);
}
WORD eval_block_cb(WORD block, Object * this_expr, Object * ctx) {
    return eval_block(block, ctx);
}

WORD doseq(WORD block, Object * ctx) {
    return eval_expr(block, ctx);
}

WORD eval_sequence_cb(WORD block, Object * this_expr, Object * ctx) {
    return doseq(block, ctx);
}

void block_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Block"), tag_obj(ct->type));
    define(ct->type, string_literal("eval"), make_prim(eval_block_cb));
    define(ct->type, string_literal("mark"), make_prim(mark_array_cb));
    define(ct->type, string_literal("print"), make_prim(print_block_cb));

    ct->parse = parse_block;
};

