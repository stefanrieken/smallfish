Object * parse_expr(char until);
void print_expr(WORD val);
int gc_mark_obj_array(Object * obj_array);
WORD eval_expr(WORD val, Object * ctx);

extern int CT_PARR;
extern CoreType * parr_core_type(Object * ctx);

extern int CT_EXPR;
extern CoreType * expr_core_type(Object * ctx);
