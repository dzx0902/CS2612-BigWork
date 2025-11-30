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

static char* format_tokens(const char* line){
  Lexer lx; lexer_init(&lx, line);
  sb_t sb; sb_init(&sb);
  for(;;){
    Token t = lexer_next(&lx);
    if (t.type==TOK_EOF){ token_free(&t); break; }
    if (t.lexeme){ sb_append(&sb, t.lexeme); sb_append(&sb, " "); }
    else {
      switch(t.type){
        case TOK_LPAREN: sb_append(&sb, "( "); break;
        case TOK_RPAREN: sb_append(&sb, ") "); break;
        case TOK_COMMA: sb_append(&sb, ", "); break;
        case TOK_DOT: sb_append(&sb, ". "); break;
        case TOK_NOT: sb_append(&sb, "! "); break;
        case TOK_AND: sb_append(&sb, "& "); break;
        case TOK_OR: sb_append(&sb, "| "); break;
        case TOK_IMPLY: sb_append(&sb, "-> "); break;
        case TOK_IFF: sb_append(&sb, "<-> "); break;
        case TOK_FORALL: sb_append(&sb, "forall "); break;
        case TOK_EXISTS: sb_append(&sb, "exists "); break;
        default: sb_append(&sb, "? "); break;
      }
    }
    token_free(&t);
  }
  if (!sb.buf){
    char* empty = (char*)malloc(1);
    if (empty) empty[0]='\0';
    return empty;
  }
  return sb_take(&sb);
}

static void emit_json_record(FILE* stream, int lineNo, const char* input, const char* tokens,
                             const char* ast, const char* expanded, const char* polarity,
                             bool valid, const char* error_msg){
  fprintf(stream,
          "{\"line\":%d,\"input\":\"%s\",\"tokens\":\"%s\",\"ast\":\"%s\",\"expanded\":\"%s\","
          "\"polarity\":\"%s\",\"valid\":%s,\"error\":\"%s\"}\n",
          lineNo,
          input ? input : "",
          tokens ? tokens : "",
          ast ? ast : "",
          expanded ? expanded : "",
          polarity ? polarity : "",
          valid ? "true" : "false",
          error_msg ? error_msg : "");
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
  int opt_tokens = 0, opt_print_ast = 0, opt_print_expanded = 0, opt_json = 0, opt_bison = 0, opt_print_tree = 0;
  const char* out_path = NULL;
  const char* out_text = NULL;
  const char* in_file = NULL;
  const char* out_combined = NULL;
  for(int i=1;i<argc;i++){
    if (strcmp(argv[i],"--tokens")==0) opt_tokens=1;
    else if (strcmp(argv[i],"--print-ast")==0) opt_print_ast=1;
    else if (strcmp(argv[i],"--print-expanded")==0) opt_print_expanded=1;
    else if (strcmp(argv[i],"--json")==0) opt_json=1;
    else if (strcmp(argv[i],"--out")==0 && i+1<argc) { out_path = argv[++i]; }
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
    if (!opt_json) printf("== Line %d ==\n", lineNo);
    if (opt_tokens && !opt_json) dump_tokens_line(line);
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
      char* tok_str = (out_text || out_combined || opt_json) ? format_tokens(line) : NULL;
      if (out_text){
        FILE* fp = fopen(out_text, "a");
        if (fp){
          fprintf(fp, "Line: %d\n", lineNo);
          fprintf(fp, "Input: %s\n", line);
          fprintf(fp, "Tokens: %s\n", tok_str?tok_str:"");
          fprintf(fp, "Valid: no\n");
          fprintf(fp, "Error: syntax error\n---\n");
          fclose(fp);
        }
      } else if (!opt_json) {
        printf("PARSE_ERROR\n");
      }
      if (out_combined){
        FILE* fp2 = fopen(out_combined, "a");
        if (fp2){
          fprintf(fp2, "Line: %d\n", lineNo);
          fprintf(fp2, "Input: %s\n", line);
          fprintf(fp2, "Tokens: %s\n", tok_str?tok_str:"");
          fprintf(fp2, "Valid: no\n");
          fprintf(fp2, "Error: syntax error\n");
          fprintf(fp2, "JSON: ");
          emit_json_record(fp2, lineNo, line, tok_str?tok_str:"", "", "", "", false, "syntax error");
          fprintf(fp2, "---\n");
          fclose(fp2);
        }
      }
      if (opt_json){
        FILE* fp = out_path ? fopen(out_path, "a") : NULL;
        FILE* stream = fp ? fp : out;
        emit_json_record(stream, lineNo, line, tok_str?tok_str:"", "", "", "", false, "syntax error");
        if (fp) fclose(fp);
      }
      if (tok_str) free(tok_str);
      free(line);
      continue;
    }
    if (opt_print_ast && !opt_json) { print_prop(root); printf("\n"); }
    if (opt_print_tree && !opt_json) { printf("AST Tree:\n"); print_prop_tree(root); }
    Prop* expanded = expand_iff(root);
    if (opt_print_expanded && !opt_json) { print_prop(expanded); printf("\n"); }
    if (opt_print_tree && !opt_json) { printf("Expanded Tree:\n"); print_prop_tree(expanded); }
    if (opt_json){
      char* ast_str = format_prop(root);
      char* exp_str = format_prop(expanded);
      char* tok_str = format_tokens(line);
      char* pol_buf = format_polarity(expanded, true);
      FILE* fp = out_path ? fopen(out_path, "a") : NULL;
      FILE* stream = fp ? fp : out;
      emit_json_record(stream, lineNo, line, tok_str, ast_str, exp_str, pol_buf, true, "");
      if (fp) fclose(fp);
      free(tok_str); free(ast_str); free(exp_str); free(pol_buf);
    } else if (out_text){
      char* ast_str = format_prop(root);
      char* exp_str = format_prop(expanded);
      char* ast_tree = format_prop_tree(root);
      char* exp_tree = format_prop_tree(expanded);
      char* tok_str = format_tokens(line);
      char* pol_buf = format_polarity(expanded, true);
      FILE* fp = fopen(out_text, "a");
      if (fp){
        fprintf(fp, "Line: %d\n", lineNo);
        fprintf(fp, "Input: %s\n", line);
        fprintf(fp, "Tokens: %s\n", tok_str?tok_str:"");
        fprintf(fp, "AST: %s\n", ast_str?ast_str:"");
        fprintf(fp, "AST Tree:\n%s", ast_tree?ast_tree:"");
        fprintf(fp, "Expanded: %s\n", exp_str?exp_str:"");
        fprintf(fp, "Expanded Tree:\n%s", exp_tree?exp_tree:"");
        fprintf(fp, "Valid: yes\n");
        fprintf(fp, "Polarity: %s\n", pol_buf?pol_buf:"");
        fprintf(fp, "---\n");
        fclose(fp);
      }
      free(tok_str); free(ast_str); free(exp_str); free(ast_tree); free(exp_tree); free(pol_buf);
    } else if (out_combined){
      char* ast_str = format_prop(root);
      char* exp_str = format_prop(expanded);
      char* ast_tree = format_prop_tree(root);
      char* exp_tree = format_prop_tree(expanded);
      char* tok_str = format_tokens(line);
      char* pol_buf = format_polarity(expanded, true);
      FILE* fp2 = fopen(out_combined, "a");
      if (fp2){
        fprintf(fp2, "Line: %d\n", lineNo);
        fprintf(fp2, "Input: %s\n", line);
        fprintf(fp2, "Tokens: %s\n", tok_str?tok_str:"");
        fprintf(fp2, "AST: %s\n", ast_str?ast_str:"");
        fprintf(fp2, "AST Tree:\n%s", ast_tree?ast_tree:"");
        fprintf(fp2, "Expanded: %s\n", exp_str?exp_str:"");
        fprintf(fp2, "Expanded Tree:\n%s", exp_tree?exp_tree:"");
        fprintf(fp2, "Valid: yes\n");
        fprintf(fp2, "Polarity: %s\n", pol_buf?pol_buf:"");
        fprintf(fp2, "JSON: ");
        emit_json_record(fp2, lineNo, line, tok_str?tok_str:"", ast_str?ast_str:"", exp_str?exp_str:"", pol_buf?pol_buf:"", true, "");
        fprintf(fp2, "---\n");
        fclose(fp2);
      }
      free(tok_str); free(ast_str); free(exp_str); free(ast_tree); free(exp_tree); free(pol_buf);
    } else {
      analyze_polarity(expanded, true);
    }
    free_prop(expanded);
    free_prop(root);
    free(line);
  }
  free(input);
  return 0;
}
