#include <stdio.h>
#include "syntax.h"

void analyze_polarity(Prop* prop, bool positive){
  if (!prop) return;
  switch(prop->type){
    case Prop_Unop:
      if (prop->prop.Unary_prop.op==Prop_NOT) analyze_polarity(prop->prop.Unary_prop.prop1, !positive);
      else analyze_polarity(prop->prop.Unary_prop.prop1, positive);
      break;
    case Prop_Binop:
      if (prop->prop.Binary_prop.op==Prop_IMPLY){
        analyze_polarity(prop->prop.Binary_prop.prop1, false);
        analyze_polarity(prop->prop.Binary_prop.prop2, true);
      } else {
        analyze_polarity(prop->prop.Binary_prop.prop1, positive);
        analyze_polarity(prop->prop.Binary_prop.prop2, positive);
      }
      break;
    case Prop_Quant: {
      const char* sign = positive ? "positive" : "negative";
      if (prop->prop.Quant_prop.op==Prop_FORALL) printf("forall %s : %s\n", prop->prop.Quant_prop.Variable, sign);
      else if (prop->prop.Quant_prop.op==Prop_EXISTS) printf("exists %s : %s\n", prop->prop.Quant_prop.Variable, sign);
      analyze_polarity(prop->prop.Quant_prop.prop1, positive);
      break;
    }
    case Prop_Atom:
      break;
    default:
      break;
  }
}
