WORD parse_expr(int * ch, char until);

void print_expr(WORD val, Object * ctx);
WORD eval_expr(WORD val, Object * ctx);

extern int CT_EXPR;
extern void expr_core_type(CoreType * ct, Object * ctx);

