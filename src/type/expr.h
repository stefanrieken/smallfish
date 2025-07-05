WORD parse_expr(int * ch, char until);

void print_expr(WORD val, Object * ctx);
void gc_mark_obj_array(Object * obj_array, Object * ctx);
WORD mark_array_cb(Object * ctx, WORD val);
WORD eval_expr(WORD val, Object * ctx);

extern int CT_PARR;
extern void parr_core_type(CoreType * ct, Object * ctx);

extern int CT_EXPR;
extern void expr_core_type(CoreType * ct, Object * ctx);

extern int CT_METH;
WORD make_method(WORD args, WORD body);
WORD apply_method(WORD msg, WORD obj, Object * expr, Object * caller_ctx);
extern void meth_core_type(CoreType * ct, Object * ctx);
