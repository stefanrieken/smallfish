extern int CT_EXPR;
extern int CT_SEQ;

bool parse_expr(int * ch, WORD * result, char until);
bool parse_seq(int * ch, WORD * result, char until);

void print_expr(WORD val, Object * ctx);
WORD eval_expr(WORD val, Object * ctx);
WORD doseq(WORD block, Object * ctx);

extern void expr_core_type(CoreType * ct, Object * ctx);
extern void seq_core_type(CoreType * ct, Object * ctx);
