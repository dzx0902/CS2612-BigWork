#ifndef LEXER_H
#define LEXER_H
#include <stdbool.h>

typedef enum {
  TOK_IDENT,
  TOK_NUMBER,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_COMMA,
  TOK_NOT,
  TOK_AND,
  TOK_OR,
  TOK_IMPLY,
  TOK_IFF,
  TOK_FORALL,
  TOK_EXISTS,
  TOK_DOT,
  TOK_EOF,
  TOK_INVALID
} TokenType;

typedef struct {
  TokenType type;
  char* lexeme;
} Token;

typedef struct {
  const char* input;
  int length;
  int pos;
} Lexer;

void lexer_init(Lexer* lx, const char* input);
Token lexer_next(Lexer* lx);
void token_free(Token* t);

#endif
