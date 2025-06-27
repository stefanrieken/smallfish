#include <stdio.h>
#include <string.h>

#include "smallfish.h"
#include "gc.h"

#include "type/int.h"
#include "type/string.h"
#include "type/expr.h"
#include "type/dict.h"
#include "type/prim.h"

ObjectTable * objects;

WORD eval_to_self(WORD val, Object * ctx) { return val; }

// search_from is a hint for a potential vacant slot
Object * add_object1(ObjectTable ** table, void * value, int type, int size, int search_from) {
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
    result->value.ptr = value;
    result->size = size;
    result->refcount = 0;

    //printf("Added object %ld, type %d, value %p\n", idx(result), result->type, result->value.ptr);
    return result;
}

Object * add_object(ObjectTable ** table, void * value, int type, int size) {
    return add_object1(table, value, type, size, 1);
}

WORD eval(WORD val, Object * ctx) {
    if (is_int(val)) return val;
    // else
    Object * obj = as_obj(val);
    return core_types[obj->type]->eval(val, ctx);
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

static inline
int add_core_type(int idx, CoreType * type) {
    core_types[idx] = type;
    return idx;
}

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
    CT_DICT = 6;

    Object * root = add_object(&objects, make_dict(nil, nil, 1), CT_DICT, sizeof(DictEntry) * 1); // can't pre-allocate more because no fill size indicator!
    define(root, string_literal("env"), tag_obj(root));

    num_core_types = 8;
    core_types = allocate(CoreType, num_core_types);
    int idx = 0;
    CT_INT       = add_core_type(idx++,    int_core_type(root));
    if (idx != CT_PRIM) { printf("ERROR: the presumed bootstrap value for CT_PRIM has changed (expected %d got %d)!\n", CT_PRIM, idx); exit(-1); }
    CT_PRIM      = add_core_type(idx++,   prim_core_type(root));
    if (idx != CT_STRING_RO) { printf("ERROR: the presumed bootstrap value for CT_STRING_RO has changed (expected %d got %d)!\n", CT_STRING_RO, idx); exit(-1); }
    CT_STRING_RO = add_core_type(idx++,  label_core_type(root));
    CT_STRING    = add_core_type(idx++, string_core_type(root));
    CT_PARR      = add_core_type(idx++,   parr_core_type(root));
    CT_EXPR      = add_core_type(idx++,   expr_core_type(root));
    if (idx != CT_DICT) { printf("ERROR: the presumed bootstrap value for CT_DICT has changed (expected %d got %d)!\n", CT_DICT, idx); exit(-1); }
    CT_DICT      = add_core_type(idx++,   dict_core_type(root));

    define(root, string_literal("gc"), make_prim(gc_cb));
    define(root, string_literal("help"), tag_obj(string_literal("Type 'env ls' to list global defintions. Type e.g. 'Int ls' to find integer methods.")));

    PERMGEN = objects[0].value.count;

    printf("Sizeof dictionary: %ld, dictheader: %ld dictentry: %ld\n", sizeof(Dictionary), sizeof(DictHeader), sizeof(DictEntry));

    printf("READY.\n> ");
    Object * expr = parse_expr('\n');
    while (expr != NULL) {
        WORD result = core_types[expr->type]->eval(tag_obj(expr), root);
        printf("["); print_val(result); printf("] Ok.\n> ");
        expr = parse_expr('\n');
    }
    
    printf("\n");
}

