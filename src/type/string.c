#include <stdio.h>
#include <string.h>

#include "../smallfish.h"
#include "../parse.h"
#include "../gc.h"

#include "dict.h"
#include "prim.h"

int CT_STRING_RO;
int CT_STRING;

extern Object * add_object1(ObjectTable ** table, void * value, Object * type, int size, int search_from);

char * copy(char * str) {
    char * copy = allocate(char,strlen(str)+1);
    strcpy(copy, str); // if only strcpy would return its result, I wouldn't need to write this out
    return copy;
}

Object * make_string(char * value) {
    return add_object(&objects, copy(value), core_types[CT_STRING]->type, strlen(value)+1);
}

Object * string_literal(char * value) {
    int first_free = 0;
    value = copy(value);
    for (int i=1; i<objects->value.count;i++) {
        if(objects[i].type == tag_obj(core_types[CT_STRING_RO]->type) && strcmp((const char *) value, (const char *) objects[i].value.str) == 0) {
//            printf("Found: %s\n", objects[i].value.str);
            return &(objects[i]);
        } else if (first_free == 0 && objects[i].refcount == GC_FREE) first_free = i;
    }
    // Not found; add
    //printf("First free: %d\n", first_free);
    return add_object1(&objects, value, core_types[CT_STRING_RO]->type, strlen(value)+1, first_free == 0 ? 1 : first_free);
}

void print_string(WORD val, Object * ctx) {
    printf("%s", as_obj(val)->value.str);
}
WORD print_string_cb(Object * ctx, WORD val) {
    print_string(val, ctx);
    return nil;
}

WORD resolve_label(WORD val, Object * ctx) {
    DictEntry * entry = lookup(ctx, val);
    if (entry == NULL) { printf("Unresolved variable %s\n", as_obj(val)->value.str); ls(ctx, tag_obj(ctx)); return val; }
    return entry->value;
}

WORD resolve_label_cb(Object * ctx, WORD val) {
    return resolve_label(val, ctx);
}

bool parse_label(int * ch, WORD * result) {
    // Ideally label should be considered after exhausting other things
    // For now, do some sanity checking
    // Don't have to test for int, which comes before this
    if (is_bracket(*ch) || *ch == '\"') return false; // It's something else

    char buffer[256]; int i=0;
    if (is_binary_expr(*ch)) {
        // binary expression. TODO merge with label below, only continuation criteria differ
        while (is_binary_expr(*ch)) {
            buffer[i++] = *ch;
            *ch=getchar();
        }
    } else {
        while (*ch != EOF && *ch != ';' && !is_whitespace_char(*ch) && !is_bracket(*ch) && !is_binary_expr(*ch)) {
            buffer[i++] = *ch;
            *ch=getchar();
        }
    }


    buffer[i++] = '\0';
    //printf("*%s*\n", buffer);
    *result = tag_obj(string_literal(buffer));
    return true;
}

void label_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("Label"), tag_obj(ct->type));
    define(ct->type, string_literal("print"), make_prim(print_string_cb));
    define(ct->type, string_literal("eval"), make_prim(resolve_label_cb));

    ct->apply = NULL;
    ct->parse = parse_label;
};

void string_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("String"), tag_obj(ct->type));
    define(ct->type, string_literal("print"), make_prim(print_string_cb));

    ct->apply = NULL; // for now at least; maybe apply string == map key?
    ct->parse = parse_label;
};

