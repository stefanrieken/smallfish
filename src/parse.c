#include <stdbool.h>
#include <stdio.h>

#include "smallfish.h"

// TODO when removing old parser, return this to a local,
// static inline
bool is_whitespace_char(int ch) {
    return ch == ' ' || ch == '\r' || ch == '\n'|| ch == '\t';
}

bool is_bracket(int ch) {
    return ch == '(' || ch == ')' || ch == '{'|| ch == '}' || ch == '['|| ch == ']';
}

bool is_binary_expr(int ch) {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%'
        || ch == '<' || ch == '>' || ch == '&' || ch == '|' || ch == '^'
        || ch == '!' || ch == '~';
}

int read_non_whitespace_char(char until) {
    int ch = getchar();
    if (ch == EOF || ch == until) return ch;
    while(is_whitespace_char(ch)) ch = getchar();
    return ch;
}

// Call all CoreType parsers until one succeeds
WORD parse_object(int * ch) {
    WORD result;
    for(int i=0; i< num_core_types; i++) {
        if (core_types[i]->parse != NULL && core_types[i]->parse(ch, &result)) return result;
    }
    printf("Parse error '%c'\n", *ch); exit(-1);
    return nil;
}
