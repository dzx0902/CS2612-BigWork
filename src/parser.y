%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syntax.h"
extern Prop* bison_root;
void yyerror(const char* s);
int yylex(void);
%}

%code requires {
#include "syntax.h"
typedef struct TermList { Term** items; int n; } TermList;
}

%code {
static TermList tlist_new(){ TermList l; l.items=NULL; l.n=0; return l; }
static TermList tlist_append(TermList l, Term* t){
  if (!t) return l;
  int cap = l.items? l.n : 0;
  if (l.n==cap){ int newcap = cap? cap*2 : 4; Term** nitems = (Term**)malloc(sizeof(Term*)*newcap); for(int i=0;i<l.n;i++) nitems[i]=l.items[i]; free(l.items); l.items=nitems; }
  l.items[l.n++] = t;
  return l;
}
}

%union {
  int num;
  char* str;
  Term* term;
  UPredicate* pred;
  Prop* prop;
  TermList tlist;
}

%token <str> BP_IDENT
%token <num> BP_NUMBER
%token BP_LPAREN BP_RPAREN BP_COMMA BP_DOT
%token BP_NOT BP_AND BP_OR BP_IMPLY BP_IFF BP_FORALL BP_EXISTS

%type <prop> Formula Imply Or And Unary Quantified AtomOrParen
%type <pred> Predicate
%type <tlist> TermList
%type <term> Term

%right BP_NOT
%left BP_AND
%left BP_OR
%left BP_IMPLY
%left BP_IFF

%%

Formula: Imply { $$ = $1; bison_root = $$; }
       ;

Imply: Or { $$ = $1; }
     | Imply BP_IMPLY Or { $$ = new_prop_binop(Prop_IMPLY, $1, $3); }
     | Imply BP_IFF Or { $$ = new_prop_binop(Prop_IFF, $1, $3); }
     ;

Or: And { $$ = $1; }
  | Or BP_OR And { $$ = new_prop_binop(Prop_OR, $1, $3); }
  ;

And: Unary { $$ = $1; }
   | And BP_AND Unary { $$ = new_prop_binop(Prop_AND, $1, $3); }
   ;

Unary: BP_NOT Unary { $$ = new_prop_unop(Prop_NOT, $2); }
     | Quantified { $$ = $1; }
     | AtomOrParen { $$ = $1; }
     ;

Quantified: BP_FORALL BP_IDENT BP_DOT Formula { $$ = new_prop_quant(Prop_FORALL, $2, $4); free($2); }
          | BP_EXISTS BP_IDENT BP_DOT Formula { $$ = new_prop_quant(Prop_EXISTS, $2, $4); free($2); }
          ;

AtomOrParen: Predicate { $$ = new_prop_atom($1); }
           | BP_LPAREN Formula BP_RPAREN { $$ = $2; }
           ;

Predicate: BP_IDENT BP_LPAREN TermList BP_RPAREN { $$ = new_predicate($1, $3.n, $3.items); free($1); }
         ;

TermList: Term { TermList l = tlist_new(); $$ = tlist_append(l, $1); }
        | TermList BP_COMMA Term { $$ = tlist_append($1, $3); }
        ;

Term: BP_IDENT { $$ = new_term_variable($1); free($1); }
    | BP_NUMBER { $$ = new_term_const($1); }
    | BP_IDENT BP_LPAREN TermList BP_RPAREN { UFunction* f = new_function($1, $3.n, $3.items); $$ = new_term_function(f); free($1); }
    ;

%%

void yyerror(const char* s){ (void)s; }
