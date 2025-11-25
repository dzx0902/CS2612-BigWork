#include <stdio.h>
#include "rdparser.h"
#include "syntax.h"

int main(void){
  Parser ps; parser_init(&ps, "(forall x. P(x)) -> ((forall y. Q(y)) & (exists z. R(z)))");
  Prop* p = parse_formula(&ps);
  if (!p) { printf("PARSE_FAIL\n"); return 1; }
  free_prop(p);
  printf("PARSE_OK\n");
  return 0;
}
