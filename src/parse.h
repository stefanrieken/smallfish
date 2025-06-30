bool is_whitespace_char(int ch);
bool is_bracket(int ch);
bool is_binary_expr(int ch);

int read_non_whitespace_char(char until);
WORD parse_object(int * ch);