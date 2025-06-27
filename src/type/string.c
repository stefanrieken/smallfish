#include <stdio.h>
#include <string.h>

#include "../smallfish.h"
#include "../gc.h"
#include "dict.h"

int CT_STRING_RO;
int CT_STRING;

extern Object * add_object1(ObjectTable ** table, void * value, int type, int size, int search_from);

char * copy(char * str) {
    char * copy = allocate(char,strlen(str)+1);
    strcpy(copy, str); // if only strcpy would return its result, I wouldn't need to write this out
    return copy;
}

Object * make_string(char * value) {
    return add_object(&objects, copy(value), CT_STRING, strlen(value)+1);
}

Object * string_literal(char * value) {
    int first_free = 0;
    value = copy(value);
    for (int i=1; i<objects->value.count;i++) {
        // TODO delegate to types or even classes
        if(objects[i].type == CT_STRING_RO && strcmp((const char *) value, (const char *) objects[i].value.str) == 0) {
//            printf("Found: %s\n", objects[i].value.str);
            return &(objects[i]);
        } else if (first_free == 0 && objects[i].refcount == GC_FREE) first_free = i;
    }
    // Not found; add
    //printf("First free: %d\n", first_free);
    return add_object1(&objects, value, CT_STRING_RO, strlen(value)+1, first_free == 0 ? 1 : first_free);
}

void print_string(WORD val) {
    printf("%s", as_obj(val)->value.str);
}

WORD resolve_label(WORD val, Object * ctx) {
    DictEntry * entry = lookup(ctx, val);
    if (entry == NULL) return val;
    return entry->value;
}

CoreType * label_core_type(Object * ctx) {
    Object * type = make_class(ctx, "Label", nil, nil);

    CoreType * result = allocate(CoreType, 1);
    result->type = type;
    result->eval = resolve_label;
    result->print = print_string;
    result->mark = gc_mark_none; // retain system strings
    return result;
};

CoreType * string_core_type(Object * ctx) {
    Object * type = make_class(ctx, "String", nil, nil);

    CoreType * result = allocate(CoreType, 1);
    result->type = type;
    result->eval = eval_to_self;
    result->apply = NULL; // for now at least; maybe apply string == map key?
    result->print = print_string;
    result->mark = gc_mark_none; // TODO write correct gc function (should be simple)
    return result;
};

