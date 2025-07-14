extern int CT_BLOCK;

WORD eval_block(WORD block, Object * ctx);
bool parse_block(int * ch, WORD * result);
void block_core_type(CoreType * ct, Object * ctx);
void sequence_core_type(CoreType * ct, Object * ctx);

