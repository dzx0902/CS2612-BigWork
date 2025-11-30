#include <stdio.h>
#include "rdparser.h"
#include "syntax.h"

int main(void){
  Parser ps; parser_init(&ps, "(forall x. P(x)) -> ((forall y. Q(y)) & (exists z. R(z)))");
  Prop* p = parse_formula(&ps);
  if (!p) { printf("POLARITY_FAIL_INPUT\n"); return 1; }

  Parser ps2; parser_init(&ps2, "((forall a. (exists b. P(a,b))) <-> (exists c. (forall d. Q(c,d))))");
  Prop* p2 = parse_formula(&ps2);
  if (!p2) { free_prop(p); printf("POLARITY_FAIL_INPUT2\n"); return 1; }

  analyze_polarity(p, true);
  analyze_polarity(p2, true);
  free_prop(p);
  free_prop(p2);
  printf("POLARITY_OK\n");
  return 0;
}
