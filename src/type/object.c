#include <stdio.h>

#include "../smallfish.h"
#include "../gc.h"

#include "int.h"
#include "dict.h"
#include "prim.h"
#include "string.h"
#include "bool.h"

int CT_OBJ;

WORD eval_to_self_cb(WORD val, Object * expr, Object * ctx) {
    return val;
}

WORD idx_cb(WORD val, Object * expr, Object * ctx) {
    return is_int(val) ? tag_int(0) : tag_int(val >> 1);
}

WORD obj_eq_cb(WORD val, Object * expr, Object * ctx) {
    return (val == eval(expr->value.ws[2], ctx)) ? tag_obj(core_types[CT_TRUE]->type) : tag_obj(core_types[CT_FALSE]->type);
}

WORD type_cb(WORD val, Object * expr, Object * ctx) {
    return is_int(val) ? tag_obj(core_types[CT_INT]->type) : as_obj(val)->type;
}

WORD print_obj_cb(WORD val, Object * expr, Object * ctx) {
    for(int i=1;i<ctx->size/sizeof(DictEntry); i++) {
        if (ctx->value.dict[i].value == val) {
            printf("%s", as_obj(ctx->value.dict[i].name)->value.str);
            return nil;
        }
    }
    for(int i=1;i<ctx->size/sizeof(DictEntry); i++) {
        if (ctx->value.dict[i].value == as_obj(val)->type) {
            printf("(%s)", as_obj(ctx->value.dict[i].name)->value.str);
            return nil;
        }
    }
    printf("(an Object)");
    return nil;
}

WORD mark_none_cb(WORD val, Object * expr, Object * ctx) {
    // do nothing
    return nil;
}


void obj_core_type(CoreType * ct, Object * ctx) {
    Object * type = ct->type;
    define(ctx, string_literal("Object"), tag_obj(type));
    define(type, string_literal("mark"), make_prim(mark_none_cb));
    define(type, string_literal("eval"), make_prim(eval_to_self_cb));
    define(type, string_literal("type"), make_prim(type_cb));
    define(type, string_literal("print"), make_prim(print_obj_cb));
    define(type, string_literal("idx"), make_prim(idx_cb));
    define(type, string_literal("=="), make_prim(obj_eq_cb));
}
