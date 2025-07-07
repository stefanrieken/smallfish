
extern int CT_METH;
WORD make_method(WORD args, WORD body, Object * ctx);
WORD apply_method(WORD msg, WORD obj, Object * expr, Object * caller_ctx);
extern void meth_core_type(CoreType * ct, Object * ctx);
