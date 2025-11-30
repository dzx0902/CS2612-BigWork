#include <stdio.h>
#include <string.h>
#include "rdparser.h"
#include "syntax.h"

static int expect_polarity(const char* input, const char* expected){
  Parser ps; parser_init(&ps, input);
  Prop* root = parse_formula(&ps);
  if (!root) return 0;
  Prop* expanded = expand_iff(root);
  free_prop(root);
  if (!expanded) return 0;
  char* pol = format_polarity(expanded, true);
  free_prop(expanded);
  const char* actual = pol ? pol : "";
  int ok = strcmp(actual, expected)==0;
  if (pol) free(pol);
  return ok;
}

int main(void){
  if (!expect_polarity("(forall x. P(x)) -> ((forall y. Q(y)) & (exists z. R(z)))",
                       "forall x : negative;forall y : positive;exists z : positive")){
    printf("POLARITY_EXPECT_FAIL1\n");
    return 1;
  }
  if (!expect_polarity("((forall a. (exists b. P(a,b))) <-> (exists c. (forall d. Q(c,d))))",
                       "forall a : negative;exists b : negative;exists c : positive;forall d : positive;"
                       "exists c : negative;forall d : negative;forall a : positive;exists b : positive")){
    printf("POLARITY_EXPECT_FAIL2\n");
    return 1;
  }
  printf("POLARITY_OK\n");
  return 0;
}
