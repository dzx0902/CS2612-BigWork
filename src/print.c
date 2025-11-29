#include <stdio.h>
#include "print.h"
#include "sb.h"

void print_term(const Term* t){
  if (!t) { printf("?"); return; }
  switch(t->type){
    case Term_VarName:
      printf("%s", t->term.Variable?t->term.Variable:"");
      break;
    case Term_ConstNum:
      printf("%d", t->term.ConstNum);
      break;
    case Term_UFTerm:{
      UFunction* f = t->term.UFTerm;
      printf("%s(", f&&f->name?f->name:"");
      for(int i=0;i<(f?f->numArgs:0);i++){
        if (i) printf(", ");
        print_term(f->args[i]);
      }
      printf(")");
      break;
    }
    default:
      printf("?");
      break;
  }
}

void print_pred(const UPredicate* p){
  if (!p) { printf("?"); return; }
  printf("%s(", p->name?p->name:"");
  for(int i=0;i<p->numArgs;i++){
    if (i) printf(", ");
    print_term(p->args[i]);
  }
  printf(")");
}

void print_prop(const Prop* p){
  if (!p) { printf("?"); return; }
  switch(p->type){
    case Prop_Atom:
      print_pred(p->prop.Atomic_prop);
      break;
    case Prop_Unop:
      if (p->prop.Unary_prop.op==Prop_NOT){
        printf("!");
        print_prop(p->prop.Unary_prop.prop1);
      } else {
        print_prop(p->prop.Unary_prop.prop1);
      }
      break;
    case Prop_Binop:{
      printf("(");
      print_prop(p->prop.Binary_prop.prop1);
      switch(p->prop.Binary_prop.op){
        case Prop_AND: printf(" & "); break;
        case Prop_OR: printf(" | "); break;
        case Prop_IMPLY: printf(" -> "); break;
        case Prop_IFF: printf(" <-> "); break;
        default: printf(" ? "); break;
      }
      print_prop(p->prop.Binary_prop.prop2);
      printf(")");
      break;
    }
    case Prop_Quant:
      if (p->prop.Quant_prop.op==Prop_FORALL) printf("forall %s. ", p->prop.Quant_prop.Variable);
      else if (p->prop.Quant_prop.op==Prop_EXISTS) printf("exists %s. ", p->prop.Quant_prop.Variable);
      print_prop(p->prop.Quant_prop.prop1);
      break;
    default:
      printf("?");
      break;
  }
}

static void fmt_term(sb_t* sb, const Term* t){
  if (!t) { sb_append(sb, "?"); return; }
  switch(t->type){
    case Term_VarName:
      sb_append(sb, t->term.Variable?t->term.Variable:"");
      break;
    case Term_ConstNum: {
      char tmp[32];
      int n = snprintf(tmp, sizeof(tmp), "%d", t->term.ConstNum);
      sb_append_n(sb, tmp, (size_t)n);
      break;
    }
    case Term_UFTerm:{
      UFunction* f = t->term.UFTerm;
      sb_append(sb, f&&f->name?f->name:"");
      sb_append(sb, "(");
      for(int i=0;i<(f?f->numArgs:0);i++){
        if (i) sb_append(sb, ", ");
        fmt_term(sb, f->args[i]);
      }
      sb_append(sb, ")");
      break;
    }
    default:
      sb_append(sb, "?");
      break;
  }
}

static void fmt_pred(sb_t* sb, const UPredicate* p){
  if (!p) { sb_append(sb, "?"); return; }
  sb_append(sb, p->name?p->name:"");
  sb_append(sb, "(");
  for(int i=0;i<p->numArgs;i++){
    if (i) sb_append(sb, ", ");
    fmt_term(sb, p->args[i]);
  }
  sb_append(sb, ")");
}

static void fmt_prop(sb_t* sb, const Prop* p){
  if (!p) { sb_append(sb, "?"); return; }
  switch(p->type){
    case Prop_Atom:
      fmt_pred(sb, p->prop.Atomic_prop);
      break;
    case Prop_Unop:
      if (p->prop.Unary_prop.op==Prop_NOT){
        sb_append(sb, "!");
        fmt_prop(sb, p->prop.Unary_prop.prop1);
      } else {
        fmt_prop(sb, p->prop.Unary_prop.prop1);
      }
      break;
    case Prop_Binop:{
      sb_append(sb, "(");
      fmt_prop(sb, p->prop.Binary_prop.prop1);
      switch(p->prop.Binary_prop.op){
        case Prop_AND: sb_append(sb, " & "); break;
        case Prop_OR: sb_append(sb, " | "); break;
        case Prop_IMPLY: sb_append(sb, " -> "); break;
        case Prop_IFF: sb_append(sb, " <-> "); break;
        default: sb_append(sb, " ? "); break;
      }
      fmt_prop(sb, p->prop.Binary_prop.prop2);
      sb_append(sb, ")");
      break;
    }
    case Prop_Quant:
      if (p->prop.Quant_prop.op==Prop_FORALL){ sb_append(sb, "forall "); sb_append(sb, p->prop.Quant_prop.Variable); sb_append(sb, ". "); }
      else if (p->prop.Quant_prop.op==Prop_EXISTS){ sb_append(sb, "exists "); sb_append(sb, p->prop.Quant_prop.Variable); sb_append(sb, ". "); }
      fmt_prop(sb, p->prop.Quant_prop.prop1);
      break;
    default:
      sb_append(sb, "?");
      break;
  }
}

char* format_term(const Term* t){ sb_t sb; sb_init(&sb); fmt_term(&sb, t); return sb_take(&sb); }
char* format_pred(const UPredicate* p){ sb_t sb; sb_init(&sb); fmt_pred(&sb, p); return sb_take(&sb); }
char* format_prop(const Prop* p){ sb_t sb; sb_init(&sb); fmt_prop(&sb, p); return sb_take(&sb); }

static void indent(int d){ for(int i=0;i<d;i++) printf("  "); }

static void print_prop_tree_rec(const Prop* p, int d){
  if (!p){ indent(d); printf("?\n"); return; }
  switch(p->type){
    case Prop_Atom: {
      indent(d);
      printf("- ");
      char* s = format_pred(p->prop.Atomic_prop);
      printf("ATOM %s\n", s?s:"");
      if (s) free(s);
      break;
    }
    case Prop_Unop: {
      indent(d);
      printf("- UNOP %s\n", p->prop.Unary_prop.op==Prop_NOT?"NOT":"?");
      print_prop_tree_rec(p->prop.Unary_prop.prop1, d+1);
      break;
    }
    case Prop_Binop: {
      indent(d);
      const char* op = "?";
      switch(p->prop.Binary_prop.op){
        case Prop_AND: op = "AND"; break;
        case Prop_OR: op = "OR"; break;
        case Prop_IMPLY: op = "IMPLY"; break;
        case Prop_IFF: op = "IFF"; break;
        default: break;
      }
      printf("- BINOP %s\n", op);
      print_prop_tree_rec(p->prop.Binary_prop.prop1, d+1);
      print_prop_tree_rec(p->prop.Binary_prop.prop2, d+1);
      break;
    }
    case Prop_Quant: {
      indent(d);
      const char* q = p->prop.Quant_prop.op==Prop_FORALL?"FORALL":(p->prop.Quant_prop.op==Prop_EXISTS?"EXISTS":"?");
      printf("- QUANT %s %s\n", q, p->prop.Quant_prop.Variable?p->prop.Quant_prop.Variable:"");
      print_prop_tree_rec(p->prop.Quant_prop.prop1, d+1);
      break;
    }
    default:
      indent(d); printf("- ?\n");
      break;
  }
}

void print_prop_tree(const Prop* p){ print_prop_tree_rec(p, 0); }

static void fmt_indent(sb_t* sb, int d){ for(int i=0;i<d;i++) sb_append(sb, "  "); }

static void fmt_tree_rec(sb_t* sb, const Prop* p, int d){
  if (!p){ fmt_indent(sb,d); sb_append(sb, "?\n"); return; }
  switch(p->type){
    case Prop_Atom: {
      fmt_indent(sb,d);
      sb_append(sb, "- ");
      char* s = format_pred(p->prop.Atomic_prop);
      sb_append(sb, "ATOM "); sb_append(sb, s?s:""); sb_append(sb, "\n");
      if (s) free(s);
      break;
    }
    case Prop_Unop: {
      fmt_indent(sb,d);
      sb_append(sb, "- UNOP ");
      if (p->prop.Unary_prop.op==Prop_NOT) sb_append(sb, "NOT"); else sb_append(sb, "?");
      sb_append(sb, "\n");
      fmt_tree_rec(sb, p->prop.Unary_prop.prop1, d+1);
      break;
    }
    case Prop_Binop: {
      fmt_indent(sb,d);
      const char* op = "?";
      switch(p->prop.Binary_prop.op){ case Prop_AND: op="AND"; break; case Prop_OR: op="OR"; break; case Prop_IMPLY: op="IMPLY"; break; case Prop_IFF: op="IFF"; break; default: break; }
      sb_append(sb, "- BINOP "); sb_append(sb, op); sb_append(sb, "\n");
      fmt_tree_rec(sb, p->prop.Binary_prop.prop1, d+1);
      fmt_tree_rec(sb, p->prop.Binary_prop.prop2, d+1);
      break;
    }
    case Prop_Quant: {
      fmt_indent(sb,d);
      const char* q = p->prop.Quant_prop.op==Prop_FORALL?"FORALL":(p->prop.Quant_prop.op==Prop_EXISTS?"EXISTS":"?");
      sb_append(sb, "- QUANT "); sb_append(sb, q); sb_append(sb, " "); sb_append(sb, p->prop.Quant_prop.Variable?p->prop.Quant_prop.Variable:""); sb_append(sb, "\n");
      fmt_tree_rec(sb, p->prop.Quant_prop.prop1, d+1);
      break;
    }
    default:
      fmt_indent(sb,d); sb_append(sb, "- ?\n");
      break;
  }
}

char* format_prop_tree(const Prop* p){ sb_t sb; sb_init(&sb); fmt_tree_rec(&sb, p, 0); return sb_take(&sb); }
