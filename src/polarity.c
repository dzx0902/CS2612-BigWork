#include <stdio.h>
#include "syntax.h"
#include "sb.h"

void analyze_polarity(const Prop* prop, bool positive){
  if (!prop) return;
  switch(prop->type){
    case Prop_Unop:
      if (prop->prop.Unary_prop.op==Prop_NOT) analyze_polarity(prop->prop.Unary_prop.prop1, !positive);
      else analyze_polarity(prop->prop.Unary_prop.prop1, positive);
      break;
    case Prop_Binop:
      if (prop->prop.Binary_prop.op==Prop_IMPLY){
        analyze_polarity(prop->prop.Binary_prop.prop1, !positive);
        analyze_polarity(prop->prop.Binary_prop.prop2, positive);
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

static void append_entry(sb_t* sb, const char* quant, const char* var, bool positive){
  if (!quant || !var) return;
  if (sb->len > 0) sb_append(sb, ";");
  sb_append(sb, quant);
  sb_append(sb, " ");
  sb_append(sb, var);
  sb_append(sb, " : ");
  sb_append(sb, positive ? "positive" : "negative");
}

static void format_polarity_rec(sb_t* sb, const Prop* prop, bool positive){
  if (!prop) return;
  switch(prop->type){
    case Prop_Unop:
      if (prop->prop.Unary_prop.op==Prop_NOT) format_polarity_rec(sb, prop->prop.Unary_prop.prop1, !positive);
      else format_polarity_rec(sb, prop->prop.Unary_prop.prop1, positive);
      break;
    case Prop_Binop:
      if (prop->prop.Binary_prop.op==Prop_IMPLY){
        format_polarity_rec(sb, prop->prop.Binary_prop.prop1, !positive);
        format_polarity_rec(sb, prop->prop.Binary_prop.prop2, positive);
      } else {
        format_polarity_rec(sb, prop->prop.Binary_prop.prop1, positive);
        format_polarity_rec(sb, prop->prop.Binary_prop.prop2, positive);
      }
      break;
    case Prop_Quant:
      if (prop->prop.Quant_prop.op==Prop_FORALL) append_entry(sb, "forall", prop->prop.Quant_prop.Variable, positive);
      else if (prop->prop.Quant_prop.op==Prop_EXISTS) append_entry(sb, "exists", prop->prop.Quant_prop.Variable, positive);
      format_polarity_rec(sb, prop->prop.Quant_prop.prop1, positive);
      break;
    default:
      break;
  }
}

char* format_polarity(const Prop* prop, bool positive){
  sb_t sb; sb_init(&sb);
  format_polarity_rec(&sb, prop, positive);
  if (!sb.buf){
    char* empty = (char*)malloc(1);
    if (empty) empty[0] = '\0';
    return empty;
  }
  return sb_take(&sb);
}
