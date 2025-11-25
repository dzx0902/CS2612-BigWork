#include <stdlib.h>
#include <string.h>
#include "rdparser.h"

static void next(Parser* ps){ token_free(&ps->cur); ps->cur = lexer_next(&ps->lx); }
static int match(Parser* ps, TokenType tp){ if (ps->cur.type==tp){ next(ps); return 1; } return 0; }
static void fail(Parser* ps){ ps->error=1; }

static Term* parse_term(Parser* ps);
static Term** parse_term_list(Parser* ps, int* n);
static UPredicate* parse_predicate(Parser* ps);
static Prop* parse_atom_or_paren(Parser* ps);
static Prop* parse_quantified(Parser* ps);
static Prop* parse_unary(Parser* ps);
static Prop* parse_and(Parser* ps);
static Prop* parse_or(Parser* ps);
static Prop* parse_imply(Parser* ps);

void parser_init(Parser* ps, const char* input){ lexer_init(&ps->lx, input); ps->cur = lexer_next(&ps->lx); ps->error=0; }

static Term* parse_term(Parser* ps){
  if (ps->cur.type==TOK_IDENT){
    char* name = ps->cur.lexeme ? strdup(ps->cur.lexeme) : NULL;
    next(ps);
    if (ps->cur.type==TOK_LPAREN){
      next(ps);
      int n=0; Term** args = parse_term_list(ps,&n);
      if (!match(ps,TOK_RPAREN)){ fail(ps); }
      UFunction* f = new_function(name?name:"", n, args);
      free(name);
      return new_term_function(f);
    } else {
      Term* v = new_term_variable(name?name:"");
      free(name);
      return v;
    }
  }
  if (ps->cur.type==TOK_NUMBER){
    int val = atoi(ps->cur.lexeme?ps->cur.lexeme:"0");
    next(ps);
    return new_term_const(val);
  }
  fail(ps);
  return NULL;
}

static Term** parse_term_list(Parser* ps, int* n){
  Term** arr = NULL; int cap=0; *n=0;
  Term* t = parse_term(ps);
  if (!t) return NULL;
  cap=4; arr=(Term**)malloc(sizeof(Term*)*cap); arr[(*n)++]=t;
  while (ps->cur.type==TOK_COMMA){
    next(ps);
    t = parse_term(ps);
    if (!t){ fail(ps); break; }
    if (*n>=cap){ cap*=2; arr=(Term**)realloc(arr,sizeof(Term*)*cap); }
    arr[(*n)++]=t;
  }
  return arr;
}

static UPredicate* parse_predicate(Parser* ps){
  if (ps->cur.type!=TOK_IDENT) { fail(ps); return NULL; }
  char* name = ps->cur.lexeme ? strdup(ps->cur.lexeme) : NULL;
  next(ps);
  if (!match(ps,TOK_LPAREN)) { free(name); fail(ps); return NULL; }
  int n=0; Term** args = parse_term_list(ps,&n);
  if (!match(ps,TOK_RPAREN)) { free(name); fail(ps); }
  UPredicate* p = new_predicate(name?name:"", n, args);
  free(name);
  return p;
}

static Prop* parse_atom_or_paren(Parser* ps){
  if (ps->cur.type==TOK_LPAREN){
    next(ps);
    Prop* f = parse_imply(ps);
    if (!match(ps,TOK_RPAREN)) { fail(ps); }
    return f;
  }
  UPredicate* pr = parse_predicate(ps);
  if (!pr) return NULL;
  return new_prop_atom(pr);
}

static Prop* parse_quantified(Parser* ps){
  if (ps->cur.type==TOK_FORALL || ps->cur.type==TOK_EXISTS){
    PropQuant q = ps->cur.type==TOK_FORALL ? Prop_FORALL : Prop_EXISTS;
    next(ps);
    if (ps->cur.type!=TOK_IDENT) { fail(ps); return NULL; }
    char* var = ps->cur.lexeme ? strdup(ps->cur.lexeme) : NULL;
    next(ps);
    if (!match(ps,TOK_DOT)) { free(var); fail(ps); return NULL; }
    Prop* body = parse_imply(ps);
    Prop* res = new_prop_quant(q, var?var:"", body);
    free(var);
    return res;
  }
  return parse_atom_or_paren(ps);
}

static Prop* parse_unary(Parser* ps){
  if (ps->cur.type==TOK_NOT){ next(ps); Prop* u = parse_unary(ps); return new_prop_unop(Prop_NOT, u); }
  return parse_quantified(ps);
}

static Prop* parse_and(Parser* ps){
  Prop* left = parse_unary(ps);
  while (ps->cur.type==TOK_AND){ next(ps); Prop* right = parse_unary(ps); left = new_prop_binop(Prop_AND, left, right); }
  return left;
}

static Prop* parse_or(Parser* ps){
  Prop* left = parse_and(ps);
  while (ps->cur.type==TOK_OR){ next(ps); Prop* right = parse_and(ps); left = new_prop_binop(Prop_OR, left, right); }
  return left;
}

static Prop* parse_imply(Parser* ps){
  Prop* left = parse_or(ps);
  while (ps->cur.type==TOK_IMPLY || ps->cur.type==TOK_IFF){
    TokenType tp = ps->cur.type; next(ps); Prop* right = parse_or(ps);
    if (tp==TOK_IMPLY) left = new_prop_binop(Prop_IMPLY, left, right);
    else left = new_prop_binop(Prop_IFF, left, right);
  }
  return left;
}

Prop* parse_formula(Parser* ps){
  Prop* p = parse_imply(ps);
  if (ps->cur.type!=TOK_EOF) fail(ps);
  if (ps->error) { free_prop(p); return NULL; }
  return p;
}
