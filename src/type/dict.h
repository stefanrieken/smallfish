Object * make_dict(WORD parent, Object * type);
DictEntry * define(Object * dict, Object * name, WORD value);

Object * make_class(Object * ctx, char * name, Object * type, WORD parent);

DictEntry * lookup(Object * dict, WORD name);

void ls(WORD val, Object * ctx, bool as_parent);

void set_parent(Object * obj, Object * parent);

void gc_mark_dict(Object * dict);

extern int CT_DICT;
extern void dict_core_type(CoreType * ct, Object * ctx);
