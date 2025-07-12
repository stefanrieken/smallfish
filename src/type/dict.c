#include <stdbool.h>
#include <stdio.h>

#include "../smallfish.h"
#include "../gc.h"

#include "string.h"
#include "prim.h"
#include "parr.h"
#include "method.h"

int CT_DICT;

Dictionary * make_dict(WORD parent, int num_entries) {
    DictEntry * dict = allocate(DictEntry, num_entries); // including parent
    dict->name = nil; // or "(parent)"
    dict->value = parent;
    return dict;
}

DictEntry * define1(Object * dict, WORD name, WORD value) {
    dict->value.dict = reallocate(dict->value.dict, uint8_t, dict->size+sizeof(DictEntry));
    DictEntry * entry = &(dict->value.dict[dict->size / sizeof(DictEntry)]);
    dict->size += sizeof(DictEntry);

    entry->name=as_label(name);
    entry->value=value;

    return entry;
}

DictEntry * define(Object * dict, Object * name, WORD value) {
    return define1(dict,tag_obj(name),value);
}

// If 'define' / 'add' has more than two arguments, assume it to be a method
// definition. (Equivalent syntax Scheme's shortcut lambda definition.)
// In theory methods can be created independently from classes, and separate
// from their assignment, but lacking an equivelent of `bind env`, it seems
// best to hijack this short form for methods. (It's terser than `add-method`.)
WORD define_cb(WORD dict, Object * expr, Object * ctx) {
    WORD name = expr->value.ws[2];
    if (expr->size / sizeof(WORD) > 4) {
        WORD args = expr->value.ws[3]; // TODO we can afford to eval this now
        WORD body = eval(expr->value.ws[4], ctx);
        printf("Body is %p\n", as_obj(as_obj(body)->type));
        if (as_obj(body)->type == tag_obj(core_types[CT_LAMBDA]->type)) body =  as_obj(body)->value.ws[1];  // unwrap lambda
        return define1(as_obj(dict), name, make_method(args, body, ctx))->value; // TODO do eval body
    } else {
        // Don't eval name, just demand it to be label TODO well maybe use string
        return define1(as_obj(dict), name, eval(expr->value.ws[3], ctx))->value;
    }
}

DictEntry * lookup(Object * dict, WORD name) {
    if(dict == objects) return NULL; // dict is nill

    // lookup last-defined-first
    // Don't check parent pointer (has nil name)
    for (int i=(dict->size/sizeof(DictEntry))-1; i>0; i--) {
        DictEntry * entry = &(dict->value.dict[i]);
        if (entry->name == name) {
            return entry;
        }// else printf("%s != %s\n", as_obj(entry->name)->value.str, as_obj(name)->value.str);
    }
    // Continue search at parent
    if(dict->size >0) return lookup(as_obj(dict->value.dict[0].value), name);
    return NULL;
}

WORD ls(WORD val, Object * expr, Object * ctx) {
    Object * dict = as_obj(val);
    if (dict->value.dict[0].value != nil) {
        printf("parent: "); print_val(dict->value.dict[0].value, ctx); printf("\n");
        ls(dict->value.dict[0].value, expr, ctx);
        printf("self:\n");
    }

    for (int i=1; i<(dict->size/sizeof(DictEntry)); i++) {
        DictEntry * entry = &(dict->value.dict[i]);
        printf("  %-10s : ", as_obj(entry->name)->value.str);
        print_val(entry->value, ctx);
        printf("\n");
    }
    return nil;
}

WORD parent_cb(WORD val, Object * expr, Object * ctx) {
    return as_obj(val)->value.dict[0].value;
}

// env bind [x] {..}
// This 'manually' defines a lambda with arguments and / or an explicit environment,
// overwriting the zero arguments and local env already bound by the block's eval. 
// Wherever this syntax appears too verbose or formal, we may (like in 'define')
// just pass arglist and body as two arguments: `values map ["x"] { x + 1 }`.

WORD bind_cb(WORD val, Object * expr, Object * ctx) {
    Object * env = as_obj(val); // The explicit context
    WORD args = eval(expr->value.ws[2], ctx);
    WORD lambda = eval(expr->value.ws[3], ctx); // <-- block self-evals to lambda here
    as_obj(lambda)->value.ws[0] = args;
    as_obj(lambda)->value.ws[3] = tag_obj(env);

    return lambda;
}

void set_parent(Object * obj, Object * parent) {
    obj->value.dict[0].value = tag_obj(parent);
}

void dict_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Dictionary"), tag_obj(ct->type));
    define(ct->type, string_literal("mark"), make_prim(mark_array_cb)); // comes down to the same thing
    define(ct->type, string_literal("define"), make_prim(define_cb));
    define(ct->type, string_literal("parent"), make_prim(parent_cb));
    define(ct->type, string_literal("ls"), make_prim(ls));
    define(ct->type, string_literal("bind"), make_prim(bind_cb));
};

