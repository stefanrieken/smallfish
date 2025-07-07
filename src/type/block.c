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
    *result = parse_expr(ch, '}'); // TODO should also return on '}' or other closing chars
    *ch = getchar();

    as_obj(*result)->type = tag_obj(core_types[CT_BLOCK]->type);
    return true;
}

void print_block_cb(Object * ctx, WORD val) {
    printf("{ "); print_list(val, ctx); printf("}");
}

void block_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Block"), tag_obj(ct->type));
    define(ct->type, string_literal("mark"), make_prim(mark_array_cb));
    define(ct->type, string_literal("print"), make_prim(print_block_cb));
// TODO eval to lambda
    ct->parse = parse_block;
};

