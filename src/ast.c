#include <stdlib.h>
#include <string.h>
#include "syntax.h"

static char* dupstr(const char* s){ size_t n=strlen(s); char* d=(char*)malloc(n+1); memcpy(d,s,n+1); return d; }

Term* new_term_variable(const char* name){
  Term* t = (Term*)malloc(sizeof(Term));
  t->type = Term_VarName;
  t->term.Variable = dupstr(name);
  return t;
}

Term* new_term_const(int num){
  Term* t = (Term*)malloc(sizeof(Term));
  t->type = Term_ConstNum;
  t->term.ConstNum = num;
  return t;
}

UFunction* new_function(const char* name, int numArgs, Term** args){
  UFunction* f = (UFunction*)malloc(sizeof(UFunction));
  f->name = dupstr(name);
  f->numArgs = numArgs;
  f->args = NULL;
  if (numArgs > 0 && args){
    f->args = (Term**)malloc(sizeof(Term*)*numArgs);
    for(int i=0;i<numArgs;i++) f->args[i]=args[i];
  }
  return f;
}

Term* new_term_function(UFunction* func){
  Term* t = (Term*)malloc(sizeof(Term));
  t->type = Term_UFTerm;
  t->term.UFTerm = func;
  return t;
}

UPredicate* new_predicate(const char* name, int numArgs, Term** args){
  UPredicate* p = (UPredicate*)malloc(sizeof(UPredicate));
  p->name = dupstr(name);
  p->numArgs = numArgs;
  p->args = NULL;
  if (numArgs > 0 && args){
    p->args = (Term**)malloc(sizeof(Term*)*numArgs);
    for(int i=0;i<numArgs;i++) p->args[i]=args[i];
  }
  return p;
}

Prop* new_prop_binop(PropBop op, Prop* left, Prop* right){
  Prop* p = (Prop*)malloc(sizeof(Prop));
  p->type = Prop_Binop;
  p->prop.Binary_prop.op = op;
  p->prop.Binary_prop.prop1 = left;
  p->prop.Binary_prop.prop2 = right;
  return p;
}

Prop* new_prop_unop(PropUop op, Prop* child){
  Prop* p = (Prop*)malloc(sizeof(Prop));
  p->type = Prop_Unop;
  p->prop.Unary_prop.op = op;
  p->prop.Unary_prop.prop1 = child;
  return p;
}

Prop* new_prop_quant(PropQuant op, const char* var, Prop* body){
  Prop* p = (Prop*)malloc(sizeof(Prop));
  p->type = Prop_Quant;
  p->prop.Quant_prop.op = op;
  p->prop.Quant_prop.Variable = dupstr(var);
  p->prop.Quant_prop.prop1 = body;
  return p;
}

Prop* new_prop_atom(UPredicate* pred){
  Prop* p = (Prop*)malloc(sizeof(Prop));
  p->type = Prop_Atom;
  p->prop.Atomic_prop = pred;
  return p;
}

void free_term(Term* term){
  if (!term) return;
  if (term->type == Term_VarName && term->term.Variable) free(term->term.Variable);
  if (term->type == Term_UFTerm) free_function(term->term.UFTerm);
  free(term);
}

void free_function(UFunction* func){
  if (!func) return;
  if (func->args){
    for(int i=0;i<func->numArgs;i++) free_term(func->args[i]);
    free(func->args);
  }
  if (func->name) free(func->name);
  free(func);
}

void free_predicate(UPredicate* pred){
  if (!pred) return;
  if (pred->args){
    for(int i=0;i<pred->numArgs;i++) free_term(pred->args[i]);
    free(pred->args);
  }
  if (pred->name) free(pred->name);
  free(pred);
}

void free_prop(Prop* prop){
  if (!prop) return;
  switch(prop->type){
    case Prop_Binop:
      free_prop(prop->prop.Binary_prop.prop1);
      free_prop(prop->prop.Binary_prop.prop2);
      break;
    case Prop_Unop:
      free_prop(prop->prop.Unary_prop.prop1);
      break;
    case Prop_Quant:
      if (prop->prop.Quant_prop.Variable) free(prop->prop.Quant_prop.Variable);
      free_prop(prop->prop.Quant_prop.prop1);
      break;
    case Prop_Atom:
      free_predicate(prop->prop.Atomic_prop);
      break;
    default:
      break;
  }
  free(prop);
}
