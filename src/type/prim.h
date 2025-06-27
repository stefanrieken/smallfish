extern int CT_PRIM;

WORD make_prim(void * cb);

// loose definition with arbitrary number of args
typedef WORD (*PrimitiveCb)(Object * ctx, WORD obj, WORD arg1, WORD arg2, WORD arg3);

CoreType * prim_core_type(Object * ctx);
