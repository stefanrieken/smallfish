#include <stdbool.h>
#include <stdio.h>

#include "../smallfish.h"
#include "../gc.h"

#include "string.h"
#include "prim.h"

int CT_DICT;

Dictionary * make_dict(WORD type, WORD parent, int num_entries) {
    Dictionary * dict = allocate(DictEntry, num_entries); // includes header ...
    DictHeader * h = (DictHeader *) dict;
    h->type = type;
    h->parent = parent;
//    h->is_functional_parent = 0;
    // printf("Made dict %p\n", dict);
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

WORD define_cb(Object * ctx, WORD dict, WORD name, WORD val) {
    // Don't eval name, just demand it to be label
    return define1(as_obj(dict), name, eval(val, ctx))->value;
}


Object * make_class(Object * ctx, char * name, WORD type, WORD parent) {
    Object * obj = add_object(&objects, make_dict(type, parent, 1), CT_DICT, sizeof(DictEntry) * 1);
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

void ls(Object * ctx, WORD val) {
    Object * dict = as_obj(val);
    // TODO ls parent first
    for (int i=1; i<(dict->size/sizeof(DictEntry)); i++) {
        DictEntry * entry = &(dict->value.dict[i]);
        printf("%-10s : ", as_obj(entry->name)->value.str);
        print_val(entry->value);
        printf("\n");
    }
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

CoreType * dict_core_type(Object * ctx) {
    Object * type = make_class(ctx, "Dict", nil, nil);
    define(type, string_literal("define"), make_prim(define_cb));
    define(type, string_literal("ls"), make_prim( ls));

    CoreType * result = allocate(CoreType, 1);
    result->type = type;
    result->apply = NULL;
    result->eval = eval_to_self;
    result->print = print_dict;
    result->mark = gc_mark_dict;
    return result;
};

