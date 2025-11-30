#include <stdio.h>
#include <string.h>
#include "rdparser.h"
#include "syntax.h"

static int expect_parse_ok(const char* input){
  Parser ps; parser_init(&ps, input);
  Prop* p = parse_formula(&ps);
  if (!p) return 0;
  free_prop(p);
  return 1;
}

static int expect_parse_fail(const char* input){
  Parser ps; parser_init(&ps, input);
  Prop* p = parse_formula(&ps);
  if (p){ free_prop(p); return 0; }
  return 1;
}

int main(void){
  /* Valid edge cases */
  if (!expect_parse_ok("((((P(x)))) )")) { printf("VALID_NEST_FAIL\n"); return 1; }
  if (!expect_parse_ok("forall x. exists y. forall z. (!(P(x,y) | Q(z)) -> R(f(x,y,z)))")) { printf("VALID_QUANT_FAIL\n"); return 1; }
  if (!expect_parse_ok("P(x)->Q(y)->R(z)")) { printf("RIGHT_ASSOC_IMPLY_FAIL\n"); return 1; }
  if (!expect_parse_ok("P(x)<->Q(y)<->R(z)")) { printf("CHAIN_IFF_FAIL\n"); return 1; }
  if (!expect_parse_ok("¬¬¬P(x)")) { printf("MULTI_NOT_FAIL\n"); return 1; }
  if (!expect_parse_ok("∀x.(P(x) ∧ ∃y.(Q(y) → R(x,y)))")) { printf("UNICODE_FAIL\n"); return 1; }

  /* Invalid inputs */
  const char* bad_cases[] = {
    "P(x,)",        /* trailing comma */
    "(P(x)",        /* missing right paren */
    "forall x P(x", /* missing dot */
    "f(,x)",        /* leading comma */
    "P(x y)",       /* missing comma */
    "∀.x. P(x)",    /* malformed quantifier */
    "P())",         /* extra paren */
    "()",           /* empty atom */
    "-> P(x)",      /* operator first */
    "<P(x)",        /* stray char */
  };
  for (size_t i=0;i<sizeof(bad_cases)/sizeof(bad_cases[0]);i++){
    if (!expect_parse_fail(bad_cases[i])) {
      printf("EXPECT_FAIL_BUT_OK: %s\n", bad_cases[i]);
      return 1;
    }
  }

  printf("ERROR_TEST_OK\n");
  return 0;
}
