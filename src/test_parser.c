#include <stdio.h>
#include <string.h>
#include "rdparser.h"
#include "syntax.h"

int main(void){
  Parser ps; parser_init(&ps, "(forall x. P(x)) -> ((forall y. Q(y)) & (exists z. R(z)))");
  Prop* p = parse_formula(&ps);
  if (!p) { printf("PARSE_FAIL\n"); return 1; }
  free_prop(p);

  parser_init(&ps, "P(x) <-> Q(x) -> R(x)");
  Prop* mixed = parse_formula(&ps);
  if (!mixed) { printf("PARSE_FAIL_PRECEDENCE\n"); return 1; }
  int ok = 1;
  if (!(mixed->type==Prop_Binop && mixed->prop.Binary_prop.op==Prop_IFF)) ok = 0;
  Prop* left = ok ? mixed->prop.Binary_prop.prop1 : NULL;
  Prop* right = ok ? mixed->prop.Binary_prop.prop2 : NULL;
  if (ok && left->type!=Prop_Atom) ok = 0;
  if (ok && (!left->prop.Atomic_prop || strcmp(left->prop.Atomic_prop->name,"P")!=0)) ok = 0;
  if (ok && !(right->type==Prop_Binop && right->prop.Binary_prop.op==Prop_IMPLY)) ok = 0;
  if (ok){
    Prop* right_left = right->prop.Binary_prop.prop1;
    Prop* right_right = right->prop.Binary_prop.prop2;
    if (right_left->type!=Prop_Atom || !right_left->prop.Atomic_prop || strcmp(right_left->prop.Atomic_prop->name,"Q")!=0) ok = 0;
    if (right_right->type!=Prop_Atom || !right_right->prop.Atomic_prop || strcmp(right_right->prop.Atomic_prop->name,"R")!=0) ok = 0;
  }
  free_prop(mixed);
  if (!ok){ printf("BAD_PRECEDENCE\n"); return 1; }

  printf("PARSE_OK\n");
  return 0;
}
