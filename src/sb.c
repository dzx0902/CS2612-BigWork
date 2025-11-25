#include <stdlib.h>
#include <string.h>
#include "sb.h"

static void sb_grow(sb_t* s, size_t need){
  if (s->cap == 0) s->cap = 128;
  while (s->len + need + 1 > s->cap) s->cap *= 2;
  s->buf = (char*)realloc(s->buf, s->cap);
}

void sb_init(sb_t* s){ s->buf=NULL; s->len=0; s->cap=0; }
void sb_free(sb_t* s){ if (s->buf) free(s->buf); s->buf=NULL; s->len=0; s->cap=0; }

void sb_append(sb_t* s, const char* str){ size_t n = str?strlen(str):0; if (n==0) return; sb_grow(s,n); memcpy(s->buf+s->len,str,n); s->len+=n; s->buf[s->len]='\0'; }
void sb_append_n(sb_t* s, const char* str, size_t n){ if (n==0) return; sb_grow(s,n); memcpy(s->buf+s->len,str,n); s->len+=n; s->buf[s->len]='\0'; }

char* sb_take(sb_t* s){ char* r = s->buf; s->buf=NULL; s->len=0; s->cap=0; return r; }
