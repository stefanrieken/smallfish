
extern int PERMGEN;

void gc_reset();
void gc_mark(WORD val);
int gc_sweep();

void gc_mark_none(Object * obj);

WORD gc_cb(Object * ctx, WORD obj);
