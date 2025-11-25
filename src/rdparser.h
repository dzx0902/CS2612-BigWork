#ifndef RDPARSER_H
#define RDPARSER_H
#include "lexer.h"
#include "syntax.h"

typedef struct {
  Lexer lx;
  Token cur;
  int error;
} Parser;

void parser_init(Parser* ps, const char* input);
Prop* parse_formula(Parser* ps);

#endif
