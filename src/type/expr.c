#include <stdio.h>

#include "../smallfish.h"
#include "../parse.h"
#include "../gc.h"

#include "expr.h"
#include "parr.h"
#include "method.h"
#include "dict.h"
#include "prim.h"
#include "string.h"
#include "int.h"

int CT_EXPR;
int CT_SEQ;

// toplevel (to call by REPL, block, etc.)
// NOTE: if expr size == 1, just return value
bool parse_expr(int * ch, WORD * result, char until) {
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
    if (idx == 1 && until == ')') { *result = expr[0]; free(expr); }
    else {
        *result = tag_obj(add_object(&objects, expr, core_types[CT_EXPR]->type, sizeof(WORD) * size));
    }
    return true;
}

// Provide this function in the CoreType struct, as
// subexpressions are what may appear at any object position.
bool parse_subexpr(int * ch, WORD * result) {
    if (*ch != '(') return false;
    *ch = read_non_whitespace_char(')');
    //bool success =
    parse_expr(ch, result, ')'); // TODO handle empty subexpr
    *ch = getchar();
    return true;
}

bool parse_seq(int * ch, WORD * result, char until) {
    WORD * sequence = NULL; int idx = 0;
    WORD expr;
    while (parse_expr(ch, &expr, until)) {
        sequence=reallocate(sequence, WORD, (idx+1)*sizeof(WORD));
         sequence[idx++] = expr;
        if (*ch == ';') *ch = read_non_whitespace_char(until); else {break;}
    }
    if (idx == 0) { *result = nil; return false;}
    *result = tag_obj(add_object(&objects, sequence, core_types[CT_SEQ]->type, idx*sizeof(WORD)));
    return true;
}

WORD doseq(WORD val, Object * ctx) {
    //printf("Doseq'ing\n");
    Object * seq = as_obj(val);
    WORD result;
    for (int i=0; i<seq->size/sizeof(WORD); i++) {
        result = eval(seq->value.ws[i], ctx);
    }
    return result;
}

WORD print_seq_cb(WORD val, Object * expr, Object * ctx) {
    Object * seq = as_obj(val);
    char * sep = "";
    for (int i=0;i<seq->size/sizeof(WORD);i++) {
        printf("%s", sep); print_list(seq->value.ws[i], ctx);
        sep = "; ";
    }
    return nil;
}

WORD print_expr_cb(WORD val, Object * expr, Object * ctx) {
    printf("( "); print_list(val, ctx); printf(")");
    return nil;
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
    if (entry == NULL) { printf ("Method not found\n"); return nil; }

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

void expr_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Expression"), tag_obj(ct->type));
    set_parent(ct->type, core_types[CT_PARR]->type);

    define(ct->type, string_literal("print"), make_prim(print_expr_cb));

    ct->parse = parse_subexpr;
};

void seq_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Sequence"), tag_obj(ct->type));
    set_parent(ct->type, core_types[CT_PARR]->type);

    define(ct->type, string_literal("print"), make_prim(print_seq_cb));

// Can't just parse a sequence anywhere we expect a single object
//    ct->parse = parse_seq;
};

