#include <stdbool.h>
#include <stdio.h>

#include "../smallfish.h"
#include "../gc.h"

#include "string.h"
#include "prim.h"
#include "expr.h"

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

    entry->name=name;
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
WORD define_cb(Object * ctx, WORD dict, WORD name, WORD val, WORD body) {
    // TODO improve callback form to better detect number of args
    if (body != nil) {
        return define1(as_obj(dict), name, make_method(val, body))->value;
    } else
    // Don't eval name, just demand it to be label
    return define1(as_obj(dict), name, eval(val, ctx))->value;
}


Object * make_class(Object * ctx, char * name, Object * type, WORD parent) {
    Object * obj = add_object(&objects, make_dict(parent, 1), CT_DICT, type, sizeof(DictEntry) * 1);
    define(ctx, string_literal(name),tag_obj(obj));
    return obj;
}

DictEntry * lookup(Object * dict, WORD name) {
    // TODO also search parents
    
    // lookup last-defined-first
//    for (int i=1; i<(dict->size/sizeof(DictEntry)); i++) {
    for (int i=(dict->size/sizeof(DictEntry))-1; i>=0; i--) {
        DictEntry * entry = &(dict->value.dict[i]);
        if (entry->name == name) {
            return entry;
        }// else printf("%s != %s\n", as_obj(entry->name)->value.str, as_obj(name)->value.str);
    }
    return NULL;
}

WORD ls(Object * ctx, WORD val) {
    Object * dict = as_obj(val);
    if (dict->value.dict[0].value != nil) {
        printf("Parent:\n");
        ls(ctx, dict->value.dict[0].value);
    }

    for (int i=1; i<(dict->size/sizeof(DictEntry)); i++) {
        DictEntry * entry = &(dict->value.dict[i]);
        printf("%-10s : ", as_obj(entry->name)->value.str);
        print_val(entry->value);
        printf("\n");
    }
    return nil;
}

// Avoid using full 'ls' here
void print_dict(WORD val) {
    printf("(dictionary)");
}

// TODO at least so far we could just use gc_mark_obj_array
void gc_mark_dict(Object * dict) {
    for (int i=0; i<dict->size;i += sizeof(Dictionary)) {
        DictEntry * entry = (DictEntry *) (dict->value.str+i);
        gc_mark(entry->name);
        gc_mark(entry->value);
    }
}

void dict_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Dict"), tag_obj(ct->type));

    define(ct->type, string_literal("define"), make_prim(define_cb));
    define(ct->type, string_literal("ls"), make_prim( ls));

    ct->apply = NULL;
    ct->eval = eval_to_self;
    ct->print = print_dict;
    ct->mark = gc_mark_dict;
};

