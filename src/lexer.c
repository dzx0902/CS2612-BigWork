#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static bool starts_with_bytes(Lexer* lx, const unsigned char* pat, int n) {
  if (lx->pos + n > lx->length) return false;
  return memcmp((const unsigned char*)lx->input + lx->pos, pat, n) == 0;
}

static void skip_ws(Lexer* lx) {
  while (lx->pos < lx->length) {
    unsigned char c = (unsigned char)lx->input[lx->pos];
    if (c==' '||c=='\t'||c=='\r'||c=='\n') { lx->pos++; continue; }
    if (c==0xC2 && lx->pos+1<lx->length && (unsigned char)lx->input[lx->pos+1]==0xA0) { lx->pos+=2; continue; }
    if (c==0xE3 && lx->pos+2<lx->length && (unsigned char)lx->input[lx->pos+1]==0x80 && (unsigned char)lx->input[lx->pos+2]==0x80) { lx->pos+=3; continue; }
    break;
  }
}

static Token make(TokenType tp, const char* start, int len) {
  Token t; t.type = tp; t.lexeme = NULL;
  if (start && len>0) { t.lexeme = (char*)malloc((size_t)len+1); memcpy(t.lexeme, start, (size_t)len); t.lexeme[len]='\0'; }
  return t;
}

void token_free(Token* t) {
  if (t && t->lexeme) { free(t->lexeme); t->lexeme = NULL; }
}

void lexer_init(Lexer* lx, const char* input) {
  lx->input = input; lx->length = (int)strlen(input); lx->pos = 0;
}

static Token number(Lexer* lx) {
  int start = lx->pos;
  while (lx->pos < lx->length && isdigit((unsigned char)lx->input[lx->pos])) lx->pos++;
  return make(TOK_NUMBER, lx->input + start, lx->pos - start);
}

static Token ident(Lexer* lx) {
  int start = lx->pos;
  lx->pos++;
  while (lx->pos < lx->length) {
    unsigned char c = (unsigned char)lx->input[lx->pos];
    if (isalnum(c) || c=='_') lx->pos++; else break;
  }
  int len = lx->pos - start;
  Token t = make(TOK_IDENT, lx->input + start, len);
  if (t.lexeme){
    if (strcmp(t.lexeme,"forall")==0) t.type = TOK_FORALL;
    else if (strcmp(t.lexeme,"exists")==0) t.type = TOK_EXISTS;
  }
  return t;
}

Token lexer_next(Lexer* lx) {
  skip_ws(lx);
  if (lx->pos >= lx->length) return make(TOK_EOF, NULL, 0);

  unsigned char c = (unsigned char)lx->input[lx->pos];

  if (c=='(') { lx->pos++; return make(TOK_LPAREN, "(", 1); }
  if (c==')') { lx->pos++; return make(TOK_RPAREN, ")", 1); }
  if (c==',') { lx->pos++; return make(TOK_COMMA, ",", 1); }
  if (c=='.') { lx->pos++; return make(TOK_DOT, ".", 1); }
  if (c==0xAC) { lx->pos++; return make(TOK_NOT, "¬", 1); }
  if (c=='!') { lx->pos++; return make(TOK_NOT, "!", 1); }
  if (c=='&') { lx->pos++; return make(TOK_AND, "&", 1); }
  if (c=='|') { lx->pos++; return make(TOK_OR, "|", 1); }

  if (c=='-' && lx->pos+1<lx->length && lx->input[lx->pos+1]=='>') {
    lx->pos += 2; return make(TOK_IMPLY, "->", 2);
  }
  if (c=='<' && lx->pos+2<lx->length && lx->input[lx->pos+1]=='-' && lx->input[lx->pos+2]=='>' ) {
    lx->pos += 3; return make(TOK_IFF, "<->", 3);
  }

  {
    static const unsigned char NOT_U[] = {0xC2,0xAC};
    static const unsigned char AND_U[] = {0xE2,0x88,0xA7};
    static const unsigned char OR_U[]  = {0xE2,0x88,0xA8};
    static const unsigned char IMP_U[] = {0xE2,0x86,0x92};
    static const unsigned char IFF_U[] = {0xE2,0x86,0x94};
    static const unsigned char ALL_U[] = {0xE2,0x88,0x80};
    static const unsigned char EXS_U[] = {0xE2,0x88,0x83};
    int start = lx->pos;
    if (starts_with_bytes(lx, NOT_U, 2)) { lx->pos += 2; return make(TOK_NOT, lx->input + start, 2); }
    if (starts_with_bytes(lx, AND_U, 3)) { lx->pos += 3; return make(TOK_AND, lx->input + start, 3); }
    if (starts_with_bytes(lx, OR_U, 3))  { lx->pos += 3; return make(TOK_OR,  lx->input + start, 3); }
    if (starts_with_bytes(lx, IMP_U, 3)) { lx->pos += 3; return make(TOK_IMPLY, lx->input + start, 3); }
    if (starts_with_bytes(lx, IFF_U, 3)) { lx->pos += 3; return make(TOK_IFF, lx->input + start, 3); }
    if (starts_with_bytes(lx, ALL_U, 3)) { lx->pos += 3; return make(TOK_FORALL, lx->input + start, 3); }
    if (starts_with_bytes(lx, EXS_U, 3)) { lx->pos += 3; return make(TOK_EXISTS, lx->input + start, 3); }
  }

  if (isalpha(c) || c=='_') return ident(lx);
  if (isdigit(c)) return number(lx);

  lx->pos++;
  return make(TOK_INVALID, (const char*)&c, 1);
}
