extern int CT_INT;

// Note: this result is NOT tagged!
WORD parse_int_with_radix(int * ch, int radix);
CoreType * int_core_type(CoreType * ct, Object * ctx);
