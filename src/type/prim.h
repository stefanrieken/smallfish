extern int CT_PRIM;

WORD make_prim(void * cb);
WORD apply_prim(WORD msg, WORD obj, Object * expr, Object * ctx);

// loose definition with arbitrary number of args
typedef WORD (*PrimitiveCb)(Object * ctx, WORD obj, WORD arg1, WORD arg2, WORD arg3);

void prim_core_type(CoreType * ct, Object * ctx);
