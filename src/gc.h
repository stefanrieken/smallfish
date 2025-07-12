
extern int PERMGEN;

void gc_reset();
void gc_mark(WORD val, Object * ctx);
int gc_sweep();

WORD gc_cb(WORD obj, Object * expr, Object * env);
