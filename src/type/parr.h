
// Shared function to print pointer array, expression
void print_list(WORD val, Object * ctx);

void gc_mark_obj_array(Object * obj_array, Object * ctx);
WORD mark_array_cb(Object * ctx, WORD val);
extern int CT_PARR;
extern void parr_core_type(CoreType * ct, Object * ctx);

