#include <stdio.h>
#include "rdparser.h"
#include "syntax.h"

int main(void){
  Parser ps; parser_init(&ps, "P(x) <-> Q(x)");
  Prop* p = parse_formula(&ps);
  if (!p) { printf("EXPAND_FAIL_INPUT\n"); return 1; }
  Prop* e = expand_iff(p);
  free_prop(p);
  if (!e) { printf("EXPAND_FAIL\n"); return 1; }
  free_prop(e);
  printf("EXPAND_OK\n");
  return 0;
}
