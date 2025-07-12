extern int CT_PRIM;

// NOTE: ctx is really only relevant as an argument
// because of lazy evaluation.
typedef WORD (*PrimitiveCb)(WORD obj, Object * expr, Object * ctx);

WORD make_prim(PrimitiveCb cb);
WORD apply_prim(WORD meth, WORD obj, Object * expr, Object * ctx);

void prim_core_type(CoreType * ct, Object * ctx);
