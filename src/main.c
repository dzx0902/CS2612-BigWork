#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rdparser.h"
#include "syntax.h"
#include "lexer.h"
#include "print.h"
#include "sb.h"
#include "bison_adapter.h"

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

static char* read_file(const char* path){
  FILE* f = fopen(path, "rb");
  if (!f) return NULL;
  fseek(f, 0, SEEK_END);
  long sz = ftell(f);
  if (sz < 0) { fclose(f); return NULL; }
  fseek(f, 0, SEEK_SET);
  char* buf = (char*)malloc((size_t)sz + 1);
  if (!buf) { fclose(f); return NULL; }
  size_t rd = fread(buf, 1, (size_t)sz, f);
  fclose(f);
  buf[rd] = '\0';
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
  int opt_tokens = 0, opt_print_ast = 0, opt_print_expanded = 0, opt_bison = 0, opt_print_tree = 0;
  const char* out_text = NULL;
  const char* in_file = NULL;
  const char* out_combined = NULL;
  for(int i=1;i<argc;i++){
    if (strcmp(argv[i],"--tokens")==0) opt_tokens=1;
    else if (strcmp(argv[i],"--print-ast")==0) opt_print_ast=1;
    else if (strcmp(argv[i],"--print-expanded")==0) opt_print_expanded=1;
    else if (strcmp(argv[i],"--bison")==0) opt_bison=1;
    else if (strcmp(argv[i],"--print-tree")==0) opt_print_tree=1;
    else if (strcmp(argv[i],"--in-file")==0 && i+1<argc) { in_file = argv[++i]; }
    else if (strcmp(argv[i],"--out-text")==0 && i+1<argc) { out_text = argv[++i]; }
    else if (strcmp(argv[i],"--out-combined")==0 && i+1<argc) { out_combined = argv[++i]; }
  }
  char* input = in_file ? read_file(in_file) : read_all_stdin();
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
    if (opt_tokens) dump_tokens_line(line);
    Prop* root = NULL;
    if (opt_bison) {
      root = parse_formula_bison(line);
      if (!root) {
        Parser ps; parser_init(&ps, line);
        root = parse_formula(&ps);
      }
    } else {
      Parser ps; parser_init(&ps, line);
      root = parse_formula(&ps);
    }
    if (!root) {
      if (out_text){
        Lexer lx2; lexer_init(&lx2, line);
        sb_t sbt; sb_init(&sbt);
        for(;;){ Token t = lexer_next(&lx2); if (t.type==TOK_EOF){ token_free(&t); break; } if (t.lexeme){ sb_append(&sbt, t.lexeme); sb_append(&sbt, " "); } else {
          switch(t.type){ case TOK_LPAREN: sb_append(&sbt, "( "); break; case TOK_RPAREN: sb_append(&sbt, ") "); break; case TOK_COMMA: sb_append(&sbt, ", "); break; case TOK_DOT: sb_append(&sbt, ". "); break; case TOK_NOT: sb_append(&sbt, "! "); break; case TOK_AND: sb_append(&sbt, "& "); break; case TOK_OR: sb_append(&sbt, "| "); break; case TOK_IMPLY: sb_append(&sbt, "-> "); break; case TOK_IFF: sb_append(&sbt, "<-> "); break; case TOK_FORALL: sb_append(&sbt, "forall "); break; case TOK_EXISTS: sb_append(&sbt, "exists "); break; default: sb_append(&sbt, "? "); break; }
        } token_free(&t); }
        char* tok_str = sb_take(&sbt);
        FILE* fp = fopen(out_text, "a");
        if (fp){
          fprintf(fp, "Line: %d\n", lineNo);
          fprintf(fp, "Input: %s\n", line);
          fprintf(fp, "Tokens: %s\n", tok_str);
          fprintf(fp, "Valid: no\n");
          fprintf(fp, "Error: syntax error\n---\n");
          fclose(fp);
        }
        free(tok_str);
      } else {
        printf("PARSE_ERROR\n");
      }
      if (out_combined){
        Lexer lx3; lexer_init(&lx3, line);
        sb_t sbc; sb_init(&sbc);
        for(;;){ Token t = lexer_next(&lx3); if (t.type==TOK_EOF){ token_free(&t); break; } if (t.lexeme){ sb_append(&sbc, t.lexeme); sb_append(&sbc, " "); } else {
          switch(t.type){ case TOK_LPAREN: sb_append(&sbc, "( "); break; case TOK_RPAREN: sb_append(&sbc, ") "); break; case TOK_COMMA: sb_append(&sbc, ", "); break; case TOK_DOT: sb_append(&sbc, ". "); break; case TOK_NOT: sb_append(&sbc, "! "); break; case TOK_AND: sb_append(&sbc, "& "); break; case TOK_OR: sb_append(&sbc, "| "); break; case TOK_IMPLY: sb_append(&sbc, "-> "); break; case TOK_IFF: sb_append(&sbc, "<-> "); break; case TOK_FORALL: sb_append(&sbc, "forall "); break; case TOK_EXISTS: sb_append(&sbc, "exists "); break; default: sb_append(&sbc, "? "); break; }
        } token_free(&t); }
        char* tokc = sb_take(&sbc);
        FILE* fp2 = fopen(out_combined, "a");
        if (fp2){
          fprintf(fp2, "Line: %d\n", lineNo);
          fprintf(fp2, "Input: %s\n", line);
          fprintf(fp2, "Tokens: %s\n", tokc);
          fprintf(fp2, "Valid: no\n");
          fprintf(fp2, "Error: syntax error\n");
          fprintf(fp2, "---\n");
          fclose(fp2);
        }
        free(tokc);
      }
      free(line);
      continue;
    }
    if (opt_print_ast) { print_prop(root); printf("\n"); }
    if (opt_print_tree) { printf("AST Tree:\n"); print_prop_tree(root); }
    Prop* expanded = expand_iff(root);
    if (!out_text && !out_combined) analyze_polarity(expanded, true);
    if (out_text){
      char* ast_str = format_prop(root);
      char* exp_str = format_prop(expanded);
      char* ast_tree = format_prop_tree(root);
      char* exp_tree = format_prop_tree(expanded);
      Lexer lx2; lexer_init(&lx2, line);
      sb_t sbt; sb_init(&sbt);
      for(;;){ Token t = lexer_next(&lx2); if (t.type==TOK_EOF){ token_free(&t); break; } if (t.lexeme){ sb_append(&sbt, t.lexeme); sb_append(&sbt, " "); } else {
        switch(t.type){ case TOK_LPAREN: sb_append(&sbt, "( "); break; case TOK_RPAREN: sb_append(&sbt, ") "); break; case TOK_COMMA: sb_append(&sbt, ", "); break; case TOK_DOT: sb_append(&sbt, ". "); break; case TOK_NOT: sb_append(&sbt, "! "); break; case TOK_AND: sb_append(&sbt, "& "); break; case TOK_OR: sb_append(&sbt, "| "); break; case TOK_IMPLY: sb_append(&sbt, "-> "); break; case TOK_IFF: sb_append(&sbt, "<-> "); break; case TOK_FORALL: sb_append(&sbt, "forall "); break; case TOK_EXISTS: sb_append(&sbt, "exists "); break; default: sb_append(&sbt, "? "); break; }
      } token_free(&t); }
      char* tok_str = sb_take(&sbt);
      char pol_buf[2048]; pol_buf[0]='\0';
      collect_pol(expanded, 1, pol_buf, sizeof(pol_buf));
      FILE* fp = fopen(out_text, "a");
      if (fp){
        fprintf(fp, "Line: %d\n", lineNo);
        fprintf(fp, "Input: %s\n", line);
        fprintf(fp, "Tokens: %s\n", tok_str);
        fprintf(fp, "AST: %s\n", ast_str);
        fprintf(fp, "AST Tree:\n%s", ast_tree);
        fprintf(fp, "Expanded: %s\n", exp_str);
        fprintf(fp, "Expanded Tree:\n%s", exp_tree);
        fprintf(fp, "Valid: yes\n");
        fprintf(fp, "Polarity: %s\n", pol_buf);
        fprintf(fp, "---\n");
        fclose(fp);
      }
      free(tok_str); free(ast_str); free(exp_str); free(ast_tree); free(exp_tree);
    } else if (out_combined){
      char* ast_str = format_prop(root);
      char* exp_str = format_prop(expanded);
      char* ast_tree = format_prop_tree(root);
      char* exp_tree = format_prop_tree(expanded);
      Lexer lx4; lexer_init(&lx4, line);
      sb_t sbd; sb_init(&sbd);
      for(;;){ Token t = lexer_next(&lx4); if (t.type==TOK_EOF){ token_free(&t); break; } if (t.lexeme){ sb_append(&sbd, t.lexeme); sb_append(&sbd, " "); } else {
        switch(t.type){ case TOK_LPAREN: sb_append(&sbd, "( "); break; case TOK_RPAREN: sb_append(&sbd, ") "); break; case TOK_COMMA: sb_append(&sbd, ", "); break; case TOK_DOT: sb_append(&sbd, ". "); break; case TOK_NOT: sb_append(&sbd, "! "); break; case TOK_AND: sb_append(&sbd, "& "); break; case TOK_OR: sb_append(&sbd, "| "); break; case TOK_IMPLY: sb_append(&sbd, "-> "); break; case TOK_IFF: sb_append(&sbd, "<-> "); break; case TOK_FORALL: sb_append(&sbd, "forall "); break; case TOK_EXISTS: sb_append(&sbd, "exists "); break; default: sb_append(&sbd, "? "); break; }
      } token_free(&t); }
      char* tok_str = sb_take(&sbd);
      char pol_buf[2048]; pol_buf[0]='\0';
      collect_pol(expanded, 1, pol_buf, sizeof(pol_buf));
      FILE* fp2 = fopen(out_combined, "a");
      if (fp2){
        fprintf(fp2, "Line: %d\n", lineNo);
        fprintf(fp2, "Input: %s\n", line);
        fprintf(fp2, "Tokens: %s\n", tok_str);
        fprintf(fp2, "AST: %s\n", ast_str);
        fprintf(fp2, "AST Tree:\n%s", ast_tree);
        fprintf(fp2, "Expanded: %s\n", exp_str);
        fprintf(fp2, "Expanded Tree:\n%s", exp_tree);
        fprintf(fp2, "Valid: yes\n");
        fprintf(fp2, "Polarity: %s\n", pol_buf);
        fprintf(fp2, "---\n");
        fclose(fp2);
      }
      free(tok_str); free(ast_str); free(exp_str); free(ast_tree); free(exp_tree);
    }
    free_prop(expanded);
    free_prop(root);
    free(line);
  }
  free(input);
  return 0;
}
