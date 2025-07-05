#include <stdio.h>

#include "../smallfish.h"
#include "../gc.h"

#include "int.h"
#include "dict.h"
#include "prim.h"
#include "string.h"

int CT_OBJ;

WORD type_cb(Object * ctx, WORD val) {
    return is_int(val) ? tag_obj(core_types[CT_INT]->type) : as_obj(val)->type;
}

void print_obj_cb(Object * ctx, WORD val) {
    for(int i=1;i<ctx->size/sizeof(DictEntry); i++) {
        if (ctx->value.dict[i].value == val) {
            printf("%s", as_obj(ctx->value.dict[i].name)->value.str);
            return;
        }
    }
    for(int i=1;i<ctx->size/sizeof(DictEntry); i++) {
        if (ctx->value.dict[i].value == as_obj(val)->type) {
            printf("(%s)", as_obj(ctx->value.dict[i].name)->value.str);
            return;
        }
    }
    printf("(an Object)");
}

void mark_none_cb(Object * ctx, WORD val) {
    // do nothing
}


void obj_core_type(CoreType * ct, Object * ctx) {
    Object * type = ct->type;
    define(ctx, string_literal("Object"), tag_obj(type));
    define(type, string_literal("mark"), make_prim(mark_none_cb));
    define(type, string_literal("type"), make_prim(type_cb));
    define(type, string_literal("print"), make_prim(print_obj_cb));
}
