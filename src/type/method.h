
extern int CT_METH;
extern int CT_LAMBDA;

WORD make_method(WORD args, WORD body, Object * ctx);
WORD apply_method(WORD msg, WORD obj, Object * expr, Object * caller_ctx);

WORD make_lambda(WORD args, WORD body, Object * ctx);
WORD eval_lambda(WORD lambda, Object * caller_ctx);

void meth_core_type(CoreType * ct, Object * ctx);
void lambda_core_type(CoreType * ct, Object * ctx);
