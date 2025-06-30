#include <stdio.h>
#include <string.h>

#include "smallfish.h"
#include "parse.h"
#include "gc.h"

#include "type/int.h"
#include "type/string.h"
#include "type/expr.h"
#include "type/dict.h"
#include "type/prim.h"

ObjectTable * objects;

WORD eval_to_self(WORD val, Object * ctx) { return val; }
WORD apply_to_self(WORD msg, WORD obj, Object * args, Object * ctx) { return msg; }

// search_from is a hint for a potential vacant slot
Object * add_object1(ObjectTable ** table, void * value, int type, Object * type1, int size, int search_from) {
    Object * result = NULL;
 
    for (int i=search_from; i<(*table)->value.count;i++) {
        if((*table)[i].refcount == GC_FREE) {
            result = &((*table)[i]); break;
        }
    }

    if (result == NULL) {
        if ((*table)->value.count == (*table)->size) {
            *table = reallocate(*table, Object, (*table)->size + 1024);
            (*table)->size += 1024;
        }
        result = &((*table)[(*table)->value.count]);
        (*table)->value.count++;
    }

    result->type = type;
    result->type1 = tag_obj(type1);
    result->value.ptr = value;
    result->size = size;
    result->refcount = 0;

    //printf("Added object %ld, type %d, value %p\n", idx(result), result->type, result->value.ptr);
    return result;
}

Object * add_object(ObjectTable ** table, void * value, int type, Object * type1, int size) {
    return add_object1(table, value, type, type1, size, 1);
}

WORD STR_EVAL;

WORD eval(WORD val, Object * ctx) {
    if (is_int(val)) return val;
    // else
    Object * obj = as_obj(val);
    DictEntry * entry = lookup(as_obj(obj->type1), STR_EVAL);
    if (entry == NULL) return val;
    if (as_obj(entry->value)->type1 == tag_obj(core_types[CT_PRIM]->type)) return apply_prim(entry->value, val, NULL, ctx);
    if (as_obj(entry->value)->type1 == tag_obj(core_types[CT_METH]->type)) return apply_method(entry->value, val, NULL, ctx);
    return entry->value; // Umm, let's say eval func is actually int 42; return that?

//    return core_types[obj->type]->eval(val, ctx);
}

void print_val(WORD val) {
    if (is_obj(val)) {
        Object * obj = as_obj(val);
        if (obj == objects) printf("(nil)");
        else core_types[obj->type]->print(val);
    } else {
        printf("%d", as_int(val));
    }
}

// TODO this registry can only be completely constructed dynamically
// But here's a little taste of what it could be like
int num_core_types;
CoreType ** core_types;

int main (int argc, char ** argv) {
    objects = allocate(Object, 1024);
    objects[0].size=1024; // ONLY for this entry, count size in num entries
    objects[0].value.count = 1; // this header counts too
    for(int i=1;i<objects[0].size;i++) objects[i].refcount=GC_FREE;

    // Register core types. See commented out CoreTypeEnum
    // in selfish.h for a more fine grained sketch of base classes

    // Predict indices for bootstrapping purposes.
    // (Do not just move these types to the front of thelist,
    // as the list is in order of GC complexity)
    CT_PRIM = 1;
    CT_STRING_RO = 2;
    CT_DICT = 4;

    Object * root = add_object(&objects, make_dict(nil, 1), CT_DICT, NULL, sizeof(DictEntry) * 1); // can't pre-allocate more because no fill size indicator!

    num_core_types = 8;
    core_types = allocate(CoreType *, num_core_types);

    // Initialize stub core type class objects
    for (int i=0;i<num_core_types;i++) {
        core_types[i] = allocate(CoreType, 1);
        core_types[i]->type = add_object(&objects, make_dict(nil, 1), CT_DICT, NULL, sizeof(DictEntry) * 1);
    }

    int idx = 0;

    CT_INT  = idx; int_core_type(core_types[idx++], root);

    if (idx != CT_PRIM) { printf("ERROR: the presumed bootstrap value for CT_PRIM has changed (expected %d got %d)!\n", CT_PRIM, idx); exit(-1); }
    CT_PRIM = idx; prim_core_type(core_types[idx++], root);

    if (idx != CT_STRING_RO) { printf("ERROR: the presumed bootstrap value for CT_STRING_RO has changed (expected %d got %d)!\n", CT_STRING_RO, idx); exit(-1); }
    CT_STRING_RO = idx; label_core_type(core_types[idx++], root);
    CT_STRING    = idx; string_core_type(core_types[idx++], root);

    if (idx != CT_DICT) { printf("ERROR: the presumed bootstrap value for CT_DICT has changed (expected %d got %d)!\n", CT_DICT, idx); exit(-1); }
    CT_DICT      = idx; dict_core_type(core_types[idx++], root);

    CT_PARR      = idx; parr_core_type(core_types[idx++], root);
    CT_EXPR      = idx; expr_core_type(core_types[idx++], root);
    CT_METH      = idx; meth_core_type(core_types[idx++], root);

    if (idx > num_core_types) { printf("ERROR: core_types out of bounds!\n"); exit(-1); }

    // Back-fix dict type for all classes
    root->type1 = tag_obj(core_types[CT_DICT]->type);
    for(int i=0;i<num_core_types;i++) core_types[i]->type->type1 = tag_obj(core_types[CT_DICT]->type);

    define(root, string_literal("env"), tag_obj(root));
    define(root, string_literal("gc"), make_prim(gc_cb));
    define(root, string_literal("help"), tag_obj(string_literal("Type 'env ls' to list global defintions. Type e.g. 'Int ls' to find integer methods.")));

    STR_EVAL = tag_obj(string_literal("eval"));

    PERMGEN = objects[0].value.count;
    printf("READY.\n> ");
    int ch = read_non_whitespace_char('\n');
    WORD result = parse_expr(&ch, '\n');
    while(result != nil) { // TODO adjust parse_expr to return proper end value
        result = eval(result, root);
        printf("["); print_val(result); printf("] Ok.\n> ");
        ch = read_non_whitespace_char('\n');
        result = parse_expr(&ch, '\n');
    }

    printf("\n");
}

