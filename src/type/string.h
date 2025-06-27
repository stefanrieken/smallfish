extern int CT_STRING_RO;
extern int CT_STRING;

Object * make_string(char * value);
Object * string_literal(char * value);

CoreType *  label_core_type(Object * ctx);
CoreType * string_core_type(Object * ctx);
