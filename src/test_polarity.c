#include <stdio.h>
#include "rdparser.h"
#include "syntax.h"

int main(void){
  Parser ps; parser_init(&ps, "(forall x. P(x)) -> ((forall y. Q(y)) & (exists z. R(z)))");
  Prop* p = parse_formula(&ps);
  if (!p) { printf("POLARITY_FAIL_INPUT\n"); return 1; }
  analyze_polarity(p, true);
  free_prop(p);
  return 0;
}
