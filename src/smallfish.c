#include <stdio.h>
#include <string.h>

#include "smallfish.h"
#include "parse.h"
#include "gc.h"

#include "type/int.h"
#include "type/object.h"
#include "type/string.h"
#include "type/parr.h"
#include "type/expr.h"
#include "type/method.h"
#include "type/block.h"
#include "type/dict.h"
#include "type/prim.h"
#include "type/bool.h"

ObjectTable * objects;

// search_from is a hint for a potential vacant slot
Object * add_object1(ObjectTable ** table, void * value, Object * type, int size, int search_from) {
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

    result->type = type == NULL ? nil : tag_obj(type);
    result->value.ptr = value;
    result->size = size;
    result->refcount = 0;

    //printf("Added object %ld, type %d, value %p\n", idx(result), result->type, result->value.ptr);
    return result;
}

Object * add_object(ObjectTable ** table, void * value, Object * type, int size) {
    return add_object1(table, value, type, size, 1);
}

WORD STR_EVAL;
WORD STR_PRINT;
WORD STR_MARK;
WORD STR_ENV;
WORD STR_APPLY;

// 1-arg message
WORD message1(WORD obj, WORD name, Object * ctx) {
    if (obj == nil) return nil;
    WORD type = is_int(obj) ? tag_obj(core_types[CT_INT]->type) : as_obj(obj)->type;
    DictEntry * entry = lookup(as_obj(type),name);
    if (entry == NULL) { printf ("Method %s not found\n", as_obj(name)->value.str); return nil; }
    if (as_obj(entry->value)->type == tag_obj(core_types[CT_PRIM]->type)) return apply_prim(entry->value, obj, NULL, ctx);
    if (as_obj(entry->value)->type == tag_obj(core_types[CT_METH]->type)) return apply_method(entry->value, obj, NULL, ctx);
    return entry->value; // allow value to be called as method
}

// Not using the above 1-arg message function, so as to allow for some (e.g. int)
// respective to really calling 'eval' for everything.
WORD eval(WORD val, Object * ctx) {
    if (is_int(val)) return val;
    // else
    Object * obj = as_obj(val);
#ifdef QUICK_EVAL
    // As you can see here, making all objects `evaluable` is a bit of overkill; it is also
    // not intrinsic to the object, but rather only applicable in one specific interpreter context.
    // From this perspective, this 'party trick' it is certainly not morally superior to encoding
    // interpreter behaviour in the interpreter itself, which is what we do here.
    //
    // (Note: this applies to evaluating arguments only. To apply methods, different bits of
    // theory and pragmatics apply; see e.g. `message` above.)
    if (as_obj(obj->type) == core_types[CT_STRING_RO]->type) return resolve_label(val, ctx);
    if (as_obj(obj->type) == core_types[CT_SEQ]->type) return doseq(val, ctx);
    if (as_obj(obj->type) == core_types[CT_EXPR]->type) return eval_expr(val, ctx);
    if (as_obj(obj->type) == core_types[CT_LAMBDA]->type) return eval_lambda(val, ctx);
    if (as_obj(obj->type) == core_types[CT_BLOCK]->type) return eval_block(val, ctx);
    if (as_obj(obj->type) == core_types[CT_PARR]->type) return eval_array(val, ctx);
    return val; // eval to self; this is also true for blocks, methods etc. (only exception MAYBE implicit binding)
#else
    // Evaluate objects using the object's class' `eval` function.
    // As evident from the above, the vast majority of types should eval to self.
    DictEntry * entry = lookup(as_obj(obj->type), STR_EVAL); // TODO add back `eval` callbacks for label, expr
    if (entry == NULL) return val; // If we don't supply a default `eval` method, e.g. with Object
    if (as_obj(entry->value)->type == tag_obj(core_types[CT_PRIM]->type)) return apply_prim(entry->value, val, NULL, ctx);
    if (as_obj(entry->value)->type == tag_obj(core_types[CT_METH]->type)) return apply_method(entry->value, val, NULL, ctx);
    return entry->value; // Umm, let's say eval func is actually int 42; return that?
#endif
}

void print_val(WORD val, Object * ctx) {
    if (val == nil) printf("(nil)");
    else message1(val, STR_PRINT, ctx);
}

// TODO this registry can only be completely constructed dynamically
// But here's a little taste of what it could be like
int num_core_types;
CoreType ** core_types;

Object * root;
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
    CT_PRIM = 2;
    CT_STRING_RO = 3;
    CT_DICT = 5;

    root = make_dict(nil, NULL); // can't pre-allocate more because no fill size indicator!

    num_core_types = 16;
    core_types = allocate(CoreType *, num_core_types);

    // Initialize stub core type class objects
    for (int i=0;i<num_core_types;i++) {
        core_types[i] = allocate(CoreType, 1);
        core_types[i]->type = make_dict(nil, NULL);
    }

    STR_EVAL = tag_obj(string_literal("eval"));
    STR_PRINT = tag_obj(string_literal("print"));
    STR_MARK = tag_obj(string_literal("mark"));

    int idx = 0;

    CT_OBJ  = idx; obj_core_type(core_types[idx++], root);
    for (int i=1;i<num_core_types;i++) {
        set_parent(core_types[i]->type, core_types[CT_OBJ]->type);
    }

    CT_INT  = idx; int_core_type(core_types[idx++], root);

    if (idx != CT_PRIM) { printf("ERROR: the presumed bootstrap value for CT_PRIM has changed (expected %d got %d)!\n", CT_PRIM, idx); exit(-1); }
    CT_PRIM = idx; prim_core_type(core_types[idx++], root);

    if (idx != CT_STRING_RO) { printf("ERROR: the presumed bootstrap value for CT_STRING_RO has changed (expected %d got %d)!\n", CT_STRING_RO, idx); exit(-1); }
    CT_STRING_RO = idx; label_core_type(core_types[idx++], root);
    CT_STRING    = idx; string_core_type(core_types[idx++], root);

    if (idx != CT_DICT) { printf("ERROR: the presumed bootstrap value for CT_DICT has changed (expected %d got %d)!\n", CT_DICT, idx); exit(-1); }
    CT_DICT      = idx; dict_core_type(core_types[idx++], root);

    CT_PARR      = idx; parr_core_type(core_types[idx++], root);
    CT_SEQ       = idx; seq_core_type(core_types[idx++], root); // Make parsing a sequence have precedence over expr; MAYBE reduce 1-expr sequence to expr in parser
    CT_EXPR      = idx; expr_core_type(core_types[idx++], root);
    CT_METH      = idx; meth_core_type(core_types[idx++], root);
    CT_LAMBDA    = idx; lambda_core_type(core_types[idx++], root);
    CT_BLOCK     = idx; block_core_type(core_types[idx++], root);

    CT_BOOL  = idx;  bool_core_type(core_types[idx++], root);
    CT_TRUE  = idx;  true_core_type(core_types[idx++], root);
    CT_FALSE = idx; false_core_type(core_types[idx++], root);

    if (idx > num_core_types) { printf("ERROR: core_types out of bounds!\n"); exit(-1); }

    // Back-fix dict type for all classes
    root->type = tag_obj(core_types[CT_DICT]->type);
    for(int i=0;i<num_core_types;i++) {
        if (core_types[i]->type->type == nil) {
            core_types[i]->type->type = tag_obj(core_types[CT_DICT]->type);
        }
    }
    // Smooth over some problems by giving the target entry of 'nil' a type
    objects[0].type=tag_obj(core_types[CT_OBJ]->type);

    define(root, string_literal("env"), tag_obj(root));
    define(root, string_literal("gc"), make_prim(gc_cb));
    define(root, string_literal("help"), tag_obj(string_literal("Type 'ls' to list global defintions. Type e.g. 'Int.ls' to find integer methods.")));

    STR_EVAL = tag_obj(string_literal("eval"));
    STR_PRINT = tag_obj(string_literal("print"));
    STR_MARK = tag_obj(string_literal("mark"));
    STR_ENV = tag_obj(string_literal("env"));
    STR_APPLY = tag_obj(string_literal("apply"));

    PERMGEN = objects[0].value.count;
    printf("READY.\n> ");
    int ch = read_non_whitespace_char(EOF);
    WORD result;
    while(parse_seq(&ch, &result, '\n')) { // TODO adjust parse_expr to return proper end value
        result = eval(result, root);
        printf("["); print_val(result, root); printf("] Ok.\n> ");
        ch = read_non_whitespace_char(EOF);
    }

    printf("\n");
}

