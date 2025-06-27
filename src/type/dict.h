Dictionary * make_dict(WORD type, WORD parent, int num_entries);
DictEntry * define(Object * dict, Object * name, WORD value);

Object * make_class(Object * ctx, char * name, WORD type, WORD parent);

DictEntry * lookup(Object * dict, WORD name);

void ls(Object * ctx, WORD val);

void gc_mark_dict(Object * dict);

extern int CT_DICT;
extern CoreType * dict_core_type(Object * ctx);
