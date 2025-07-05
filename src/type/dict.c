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

WORD ls(Object * ctx, WORD val) {
    Object * dict = as_obj(val);
printf("Dcit size %d\n", dict->size);
    if (dict->value.dict[0].value != nil) {
        printf("parent: "); print_val(dict->value.dict[0].value, ctx); printf("\n");
        ls(ctx, dict->value.dict[0].value);
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

// Avoid using full 'ls' here
void print_dict(WORD val, Object * ctx) {
    printf("(dictionary)");
}

void set_parent(Object * obj, Object * parent) {
    obj->value.dict[0].value = tag_obj(parent);
}

void dict_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Dictionary"), tag_obj(ct->type));
    define(ct->type, string_literal("mark"), make_prim(mark_array_cb)); // comes down to the same thing
    define(ct->type, string_literal("define"), make_prim(define_cb));
    define(ct->type, string_literal("ls"), make_prim(ls));

    ct->apply = NULL;
};

