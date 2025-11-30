#include <stdlib.h>
#include <string.h>
#include "syntax.h"

static char* dupstr(const char* s){ size_t n=strlen(s); char* d=(char*)malloc(n+1); memcpy(d,s,n+1); return d; }

static Term* clone_term(const Term* t){
  if (!t) return NULL;
  switch(t->type){
    case Term_VarName: return new_term_variable(t->term.Variable?t->term.Variable:"");
    case Term_ConstNum: return new_term_const(t->term.ConstNum);
    case Term_UFTerm: {
      UFunction* f = t->term.UFTerm;
      int n = f?f->numArgs:0;
      Term** args = NULL;
      if (n>0 && f->args){ args = (Term**)malloc(sizeof(Term*)*n); for(int i=0;i<n;i++) args[i]=clone_term(f->args[i]); }
      UFunction* nf = new_function(f&&f->name?f->name:"", n, args);
      if (args) free(args);
      return new_term_function(nf);
    }
    default: return NULL;
  }
}

static UPredicate* clone_pred(const UPredicate* p){
  if (!p) return NULL;
  int n = p->numArgs;
  Term** args = NULL;
  if (n>0 && p->args){ args = (Term**)malloc(sizeof(Term*)*n); for(int i=0;i<n;i++) args[i]=clone_term(p->args[i]); }
  UPredicate* np = new_predicate(p->name?p->name:"", n, args);
  if (args) free(args);
  return np;
}

Prop* clone_prop(const Prop* prop){
  if (!prop) return NULL;
  switch(prop->type){
    case Prop_Atom:
      return new_prop_atom(clone_pred(prop->prop.Atomic_prop));
    case Prop_Unop:
      return new_prop_unop(prop->prop.Unary_prop.op, clone_prop(prop->prop.Unary_prop.prop1));
    case Prop_Binop:
      return new_prop_binop(prop->prop.Binary_prop.op, clone_prop(prop->prop.Binary_prop.prop1), clone_prop(prop->prop.Binary_prop.prop2));
    case Prop_Quant:
      return new_prop_quant(prop->prop.Quant_prop.op, prop->prop.Quant_prop.Variable?prop->prop.Quant_prop.Variable:"", clone_prop(prop->prop.Quant_prop.prop1));
    default:
      return NULL;
  }
}

Prop* expand_iff(const Prop* prop){
  if (!prop) return NULL;
  if (prop->type==Prop_Binop && prop->prop.Binary_prop.op==Prop_IFF){
    Prop* a = expand_iff(prop->prop.Binary_prop.prop1);
    Prop* b = expand_iff(prop->prop.Binary_prop.prop2);
    Prop* ab = new_prop_binop(Prop_IMPLY, clone_prop(a), clone_prop(b));
    Prop* ba = new_prop_binop(Prop_IMPLY, clone_prop(b), clone_prop(a));
    Prop* res = new_prop_binop(Prop_AND, ab, ba);
    free_prop(a);
    free_prop(b);
    return res;
  }
  switch(prop->type){
    case Prop_Atom:
      return clone_prop(prop);
    case Prop_Unop:
      return new_prop_unop(prop->prop.Unary_prop.op, expand_iff(prop->prop.Unary_prop.prop1));
    case Prop_Binop:
      return new_prop_binop(prop->prop.Binary_prop.op, expand_iff(prop->prop.Binary_prop.prop1), expand_iff(prop->prop.Binary_prop.prop2));
    case Prop_Quant:
      return new_prop_quant(prop->prop.Quant_prop.op, prop->prop.Quant_prop.Variable?prop->prop.Quant_prop.Variable:"", expand_iff(prop->prop.Quant_prop.prop1));
    default:
      return NULL;
  }
}
