#include <stdio.h>

#include "../smallfish.h"
#include "../gc.h"

#include "dict.h"
#include "string.h"
#include "int.h"

int CT_PARR;
int CT_EXPR;

static inline
bool is_whitespace_char(int ch) {
    return ch == ' ' || ch == '\r' || ch == '\n'|| ch == '\t';
}

bool is_bracket(int ch) {
    return ch == '(' || ch == ')' || ch == '{'|| ch == '}' || ch == '['|| ch == ']';
}

bool is_binary_expr(int ch) {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%'
        || ch == '<' || ch == '>' || ch == '&' || ch == '|' || ch == '^'
        || ch == '!' || ch == '~';
}

int read_non_whitespace_char(char until) {
    int ch = getchar();
    if (ch == EOF || ch == until) return ch;
    while(is_whitespace_char(ch)) ch = getchar();
    return ch;
}

bool parse_int(int * ch, int * value) {
    bool success = false;
    *value = 0;
    while (*ch >= '0' && *ch <= '9') {
        success = true;
        *value = *value * 10 + (*ch - '0');
        *ch = getchar();
    }
    return success;
}

// Let's start simple
Object * parse_expr(char until) {
//    WORD msg = nil;
    int value = 0;

    int size = 1; //3; // expect ternary TODO use alloc_aligned
    int idx = 0; // have none yet
    WORD * expr = allocate(WORD, size);

    int ch = read_non_whitespace_char(until);
    while (ch != EOF && ch != until) {
        if (idx == size) expr = reallocate(expr, WORD, ++size);
        if(parse_int(&ch, &value)) {
            // int
//            printf("Parsed value %d\n", value);
            expr[idx++] = tag_int(value);
            if (ch != EOF && ch != until && is_whitespace_char(ch)) ch = read_non_whitespace_char(until);
        } else if (ch == '(') {
            expr[idx++] = tag_obj(parse_expr(')'));
            ch = read_non_whitespace_char(until);
        } else if (ch == '[') {
            expr[idx++] = tag_obj(parse_expr(']'));
            as_obj(expr[idx-1])->type=CT_PARR;
            ch = read_non_whitespace_char(until);
        } else if (is_binary_expr(ch)) {
            // binary expression. TODO merge with label below, only continuation criteria differ
            char buffer[256]; int i=0;
            while (is_binary_expr(ch)) {
                buffer[i++] = ch;
                ch=getchar();
            }
            buffer[i++] = '\0';
            expr[idx++] = tag_obj(string_literal(buffer));
            if (ch != EOF && ch != until && is_whitespace_char(ch)) ch = read_non_whitespace_char(until);            
        } else {
            // label
            char buffer[256]; int i=0;
            while (ch != EOF && ch != until && !is_whitespace_char(ch) && !is_bracket(ch)) {
                buffer[i++] = ch;
                ch=getchar();
            }
            buffer[i++] = '\0';
            //printf("*%s*\n", buffer);
            expr[idx++] = tag_obj(string_literal(buffer));
            if (ch != EOF && ch != until && is_whitespace_char(ch)) ch = read_non_whitespace_char(until);
        }
    }

    if (idx == 0) return NULL;
    return add_object(&objects, expr, CT_EXPR, size*sizeof(WORD));
}

void gc_mark_obj_array(Object * obj_array) {
    for (int i=0; i<obj_array->size;i += sizeof(WORD)) {
        gc_mark(obj_array->value.ws[i]);
    }
}

extern void print_val(WORD val);

void print_list(WORD val) {
    Object * expr = as_obj(val);
    for(int i=0; i<expr->size/sizeof(WORD);i++) {
        print_val(expr->value.ws[i]);printf(" ");
    }
}

void print_parr(WORD val) {
    printf("[ "); print_list(val); printf("]");
}

void print_expr(WORD val) {
    printf("( "); print_list(val); printf(")");
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

CoreType * parr_core_type(Object * ctx) {
    Object * type = make_class(ctx, "Array", nil, nil);

    CoreType * result = allocate(CoreType, 1);
    result->type = type;
    result->eval = eval_to_self;
    result->print = print_parr;
    result->mark = gc_mark_obj_array;
    return result;
};

CoreType * expr_core_type(Object * ctx) {
    // TODO ad parray as parent
    Object * type = make_class(ctx, "Expression", nil, nil);

    CoreType * result = allocate(CoreType, 1);
    result->type = type;
    result->eval = eval_expr;
    result->apply = NULL; // for now; but TODO expr == native code?
    result->print = print_expr;
    result->mark = gc_mark_obj_array;
    return result;
};
