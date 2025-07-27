extern int CT_STRING_RO;
extern int CT_STRING;

// Really cheesy but effective way to communicate last label parsed was an operator
extern bool parsed_operator;

Object * make_string(char * value);
Object * string_literal(char * value);

WORD resolve_label(WORD val, Object * ctx);
WORD as_label(WORD name);

CoreType *  label_core_type(CoreType * ct, Object * ctx);
CoreType * string_core_type(CoreType * ct, Object * ctx);
