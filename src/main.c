#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rdparser.h"
#include "syntax.h"
#include "lexer.h"
#include "print.h"
#include "sb.h"

static char* read_all_stdin(void) {
  size_t cap = 4096; size_t len = 0; char* buf = (char*)malloc(cap);
  for(;;){
    if (len + 1024 > cap) { cap *= 2; buf = (char*)realloc(buf, cap); }
    size_t r = fread(buf + len, 1, 1024, stdin);
    len += r;
    if (r < 1024) break;
  }
  buf[len] = '\0';
  return buf;
}

static void collect_pol(const Prop* p, int pos, char* buf, size_t bufsize){
  if (!p) return;
  switch(p->type){
    case Prop_Unop:
      if (p->prop.Unary_prop.op==Prop_NOT) collect_pol(p->prop.Unary_prop.prop1, !pos, buf, bufsize);
      else collect_pol(p->prop.Unary_prop.prop1, pos, buf, bufsize);
      break;
    case Prop_Binop:
      if (p->prop.Binary_prop.op==Prop_IMPLY){ collect_pol(p->prop.Binary_prop.prop1, 0, buf, bufsize); collect_pol(p->prop.Binary_prop.prop2, 1, buf, bufsize); }
      else { collect_pol(p->prop.Binary_prop.prop1, pos, buf, bufsize); collect_pol(p->prop.Binary_prop.prop2, pos, buf, bufsize); }
      break;
    case Prop_Quant: {
      const char* q = p->prop.Quant_prop.op==Prop_FORALL?"forall":"exists";
      char tmp[256];
      int n = snprintf(tmp, sizeof(tmp), "%s %s : %s", q, p->prop.Quant_prop.Variable, pos?"positive":"negative");
      size_t m = strlen(buf);
      if (m>0 && m < bufsize-1) { buf[m++]=';'; buf[m]='\0'; }
      if (m < bufsize-1) strncat(buf, tmp, bufsize-1-m);
      collect_pol(p->prop.Quant_prop.prop1, pos, buf, bufsize);
      break;
    }
    default:
      break;
  }
}

static void dump_tokens_line(const char* line){
  Lexer lx; lexer_init(&lx, line);
  for(;;){ Token t = lexer_next(&lx); if (t.type==TOK_EOF) { token_free(&t); break; }
    switch(t.type){
      case TOK_IDENT: printf("IDENT(%s) ", t.lexeme); break;
      case TOK_NUMBER: printf("NUMBER(%s) ", t.lexeme); break;
      case TOK_LPAREN: printf("( "); break;
      case TOK_RPAREN: printf(") "); break;
      case TOK_COMMA: printf(", "); break;
      case TOK_DOT: printf(". "); break;
      case TOK_NOT: printf("NOT "); break;
      case TOK_AND: printf("AND "); break;
      case TOK_OR: printf("OR "); break;
      case TOK_IMPLY: printf("IMPLY "); break;
      case TOK_IFF: printf("IFF "); break;
      case TOK_FORALL: printf("FORALL "); break;
      case TOK_EXISTS: printf("EXISTS "); break;
      default: printf("INVALID(%s) ", t.lexeme?t.lexeme:""); break;
    }
    token_free(&t);
  }
  printf("\n");
}

int main(int argc, char** argv){
  int opt_tokens = 0, opt_print_ast = 0, opt_print_expanded = 0, opt_json = 0;
  const char* out_path = NULL;
  for(int i=1;i<argc;i++){
    if (strcmp(argv[i],"--tokens")==0) opt_tokens=1;
    else if (strcmp(argv[i],"--print-ast")==0) opt_print_ast=1;
    else if (strcmp(argv[i],"--print-expanded")==0) opt_print_expanded=1;
    else if (strcmp(argv[i],"--json")==0) opt_json=1;
    else if (strcmp(argv[i],"--out")==0 && i+1<argc) { out_path = argv[++i]; }
  }
  char* input = read_all_stdin();
  const char* p = input; int lineNo=0;
  while (*p){
    const char* start = p;
    while (*p && *p!='\n') p++;
    size_t len = (size_t)(p - start);
    while (len>0 && start[len-1]=='\r') len--;
    char* line = (char*)malloc(len+1); memcpy(line,start,len); line[len]='\0';
    if (*p=='\n') p++;
    lineNo++;
    if (len==0) { free(line); continue; }
    FILE* out = stdout;
    printf("== Line %d ==\n", lineNo);
    if (opt_tokens && !opt_json) dump_tokens_line(line);
    Parser ps; parser_init(&ps, line);
    Prop* root = parse_formula(&ps);
    if (!root) { if (!opt_json) printf("PARSE_ERROR\n"); free(line); continue; }
    if (opt_print_ast && !opt_json) { print_prop(root); printf("\n"); }
    Prop* expanded = expand_iff(root);
    if (opt_print_expanded && !opt_json) { print_prop(expanded); printf("\n"); }
    if (!opt_json) analyze_polarity(expanded, true);
    if (opt_json){
      char* ast_str = format_prop(root);
      char* exp_str = format_prop(expanded);
      Lexer lx2; lexer_init(&lx2, line);
      sb_t sbt; sb_init(&sbt);
      for(;;){ Token t = lexer_next(&lx2); if (t.type==TOK_EOF){ token_free(&t); break; } if (t.lexeme){ sb_append(&sbt, t.lexeme); sb_append(&sbt, " "); } else {
        switch(t.type){ case TOK_LPAREN: sb_append(&sbt, "( "); break; case TOK_RPAREN: sb_append(&sbt, ") "); break; case TOK_COMMA: sb_append(&sbt, ", "); break; case TOK_DOT: sb_append(&sbt, ". "); break; case TOK_NOT: sb_append(&sbt, "! "); break; case TOK_AND: sb_append(&sbt, "& "); break; case TOK_OR: sb_append(&sbt, "| "); break; case TOK_IMPLY: sb_append(&sbt, "-> "); break; case TOK_IFF: sb_append(&sbt, "<-> "); break; case TOK_FORALL: sb_append(&sbt, "forall "); break; case TOK_EXISTS: sb_append(&sbt, "exists "); break; default: sb_append(&sbt, "? "); break; }
      } token_free(&t); }
      char* tok_str = sb_take(&sbt);
      char pol_buf[2048]; pol_buf[0]='\0';
      collect_pol(expanded, 1, pol_buf, sizeof(pol_buf));
      FILE* fp = out_path ? fopen(out_path, "a") : NULL;
      FILE* stream = fp ? fp : out;
      fprintf(stream, "{\"line\":%d,\"input\":\"%s\",\"tokens\":\"%s\",\"ast\":\"%s\",\"expanded\":\"%s\",\"polarity\":\"%s\"}\n", lineNo, line, tok_str, ast_str, exp_str, pol_buf);
      if (fp) fclose(fp);
      free(tok_str); free(ast_str); free(exp_str);
    }
    free_prop(expanded);
    free_prop(root);
    free(line);
  }
  free(input);
  return 0;
}
