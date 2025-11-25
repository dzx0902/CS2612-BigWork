#ifndef SB_H
#define SB_H
#include <stddef.h>

typedef struct {
  char* buf;
  size_t len;
  size_t cap;
} sb_t;

void sb_init(sb_t* s);
void sb_free(sb_t* s);
void sb_append(sb_t* s, const char* str);
void sb_append_n(sb_t* s, const char* str, size_t n);
char* sb_take(sb_t* s);

#endif
