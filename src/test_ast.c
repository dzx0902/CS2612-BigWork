#include <stdio.h>
#include "syntax.h"

int main(void){
  Term* x = new_term_variable("x");
  Term* args1[] = { x };
  UPredicate* P = new_predicate("P", 1, args1);
  Prop* atomP = new_prop_atom(P);
  Prop* forall = new_prop_quant(Prop_FORALL, "x", atomP);
  Prop* notforall = new_prop_unop(Prop_NOT, forall);
  Term* y = new_term_variable("y");
  Term* args2[] = { y };
  UPredicate* Q = new_predicate("Q", 1, args2);
  Prop* atomQ = new_prop_atom(Q);
  Prop* imply = new_prop_binop(Prop_IMPLY, notforall, atomQ);
  free_prop(imply);
  printf("AST_OK\n");
  return 0;
}
