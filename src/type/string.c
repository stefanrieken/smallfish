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

// Returns a literal object for either Label or String.
// If the same value occurs as both Label and String,
// the result is two objects pointing to one value.
// This is allowed because String literals aren't GC'ed.
Object * string_literal(char * value) {
    int first_free = 0;
    for (int i=1; i<objects->value.count;i++) {
        if(objects[i].type == tag_obj(core_types[CT_STRING_RO]->type) &&
            strcmp((const char *) value, (const char *) objects[i].value.str) == 0) {
//            printf("Found: %s\n", objects[i].value.str);
            return &(objects[i]);
        } else if (first_free == 0 && objects[i].refcount == GC_FREE) first_free = i;
    }

    // Not found; add
    value = copy(value); // 'value' argument is either a stack value or a C literal; so make an heap based copy
    //printf("First free: %d\n", first_free);
    return add_object1(&objects, value, core_types[CT_STRING_RO]->type, strlen(value)+1, first_free == 0 ? 1 : first_free);
}

WORD print_label_cb(Object * ctx, WORD val) {
    printf("%s", as_obj(val)->value.str);
    return nil;
}
WORD print_string_cb(Object * ctx, WORD val) {
    // Find the value in the pointed-to Label
    printf("\"%s\"", as_obj(val)->value.obj->value.str);
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
    define(ct->type, string_literal("print"), make_prim(print_label_cb));
    define(ct->type, string_literal("eval"), make_prim(resolve_label_cb));

    ct->apply = NULL;
    ct->parse = parse_label;
};

bool parse_string(int * ch, WORD * result) {
    // Ideally label should be considered after exhausting other things
    // For now, do some sanity checking
    // Don't have to test for int, which comes before this
    if (*ch != '\"') return false; // It's something else
    *ch=getchar();

    char buffer[256]; int i=0;
    while (*ch != EOF && *ch != '\"') {
        // TODO escape sequences
        buffer[i++] = *ch;
        *ch=getchar();
    }
    *ch=getchar();

    buffer[i++] = '\0';
    //printf("*%s*\n", buffer);

    // Store (immutable) strings as indirect pointers to labels / string literals;
    // this also prepares for 'rope' style strings. The advantage of doing this over
    // Label and String pointing to the same data, is that now you can easily get from
    // a String to its underlying Label. This allows for things like quoted-string args.
    *result = tag_obj(add_object(&objects, string_literal(buffer), core_types[CT_STRING]->type, 0));
    return true;
}

// So a String (for now) is an indirect pointer to a Label.
// We don't GC labels as a rule; but if we ever change that rule,
// we must mark them as in use.
WORD mark_string_cb(Object * ctx, WORD val) {
    gc_mark(tag_obj(as_obj(val)->value.obj), ctx);
    return nil;
}

// Derive underlying Label from String, or just return Label as-is
WORD as_label(WORD name) {
    if (as_obj(name)->type == tag_obj(core_types[CT_STRING]->type)) {
//        printf("Deriving Label from String\n");
        name = tag_obj(as_obj(name)->value.obj);
    }
    return name;
}

void string_core_type(CoreType * ct, Object * ctx) {
    define(ctx, string_literal("String"), tag_obj(ct->type));
    define(ct->type, string_literal("print"), make_prim(print_string_cb));

    ct->apply = NULL; // for now at least; maybe apply string == map key?
    ct->parse = parse_string;
};

