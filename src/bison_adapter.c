#include <stdlib.h>
#include <string.h>
#include "bison_adapter.h"
#include "lexer.h"

#ifdef HAVE_BISON_PARSER
#include "parser.h"

static Lexer g_bison_lexer;
Prop* bison_root = NULL;

int yylex(void){
  Token t = lexer_next(&g_bison_lexer);
  switch(t.type){
    case TOK_IDENT: yylval.str = t.lexeme ? strdup(t.lexeme) : NULL; token_free(&t); return BP_IDENT; 
    case TOK_NUMBER: yylval.num = t.lexeme ? atoi(t.lexeme) : 0; token_free(&t); return BP_NUMBER;
    case TOK_LPAREN: token_free(&t); return BP_LPAREN;
    case TOK_RPAREN: token_free(&t); return BP_RPAREN;
    case TOK_COMMA: token_free(&t); return BP_COMMA;
    case TOK_DOT: token_free(&t); return BP_DOT;
    case TOK_NOT: token_free(&t); return BP_NOT;
    case TOK_AND: token_free(&t); return BP_AND;
    case TOK_OR: token_free(&t); return BP_OR;
    case TOK_IMPLY: token_free(&t); return BP_IMPLY;
    case TOK_IFF: token_free(&t); return BP_IFF;
    case TOK_FORALL: token_free(&t); return BP_FORALL;
    case TOK_EXISTS: token_free(&t); return BP_EXISTS;
    case TOK_EOF: token_free(&t); return 0;
    default: token_free(&t); return 0;
  }
}

Prop* parse_formula_bison(const char* input){
  lexer_init(&g_bison_lexer, input);
  bison_root = NULL;
  if (yyparse()==0) return bison_root;
  return NULL;
}

#else

Prop* parse_formula_bison(const char* input){ (void)input; return NULL; }

#endif
