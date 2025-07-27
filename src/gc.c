#include <stdio.h>

#include "smallfish.h"
#include "gc.h"

#include "type/prim.h"
#include "type/string.h"
#include "type/expr.h"
#include "type/dict.h"
#include "type/block.h"

int PERMGEN;

void gc_reset() {
    for(int i=PERMGEN;i<objects->value.count;i++) {
        if (objects[i].refcount != GC_FREE) objects[i].refcount = 0;
    }
}

void gc_mark(WORD val, Object * ctx) {
    if (is_int(val)) return;

    Object * entry = as_obj(val);

    if (entry == objects) return; // Don't mark nil == object table header

    entry->refcount++;
    if (entry->refcount > 1) return; // already visited

    // Mark following type inheritance
    message1(val, STR_MARK, ctx);
}

int gc_sweep() {
    int freed = 0;
    for (int i=PERMGEN; i<objects->value.count;i++) {
        if (
            // TODO reorder core types so that all types below a certain value are not deallocatable
            objects[i].refcount == 0 && objects[i].type != tag_obj(core_types[CT_PRIM]->type) && objects[i].type != tag_obj(core_types[CT_STRING_RO]->type) // These are not deallocatable
            && objects[i].type != tag_obj(core_types[CT_STRING]->type) && objects[i].type != tag_obj(core_types[CT_BLOCK]->type) // These point to other objects
        ) {
            // printf("freeing %d\n", objects[i].type);
            free(objects[i].value.ptr);
            objects[i].refcount = GC_FREE;
            objects[i].type = 0; // TODO make a proper 'free' type; this is 'int'!
            freed++;
        }
    }
    
    return freed;
}

WORD gc_cb(WORD obj, Object * expr, Object * env) {
    gc_reset();
    // obj is just this gc function; what we want to mark is every entry level object; like 'root'
    gc_mark(tag_obj(env), env); // true? env == obj == root ?
    int result = gc_sweep();
    printf("Freed %d/%d objects.\n", result, objects[0].value.count);
    return tag_int(result);
}
