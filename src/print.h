#ifndef PRINT_H
#define PRINT_H
#include "syntax.h"

void print_term(const Term* t);
void print_pred(const UPredicate* p);
void print_prop(const Prop* p);

char* format_term(const Term* t);
char* format_pred(const UPredicate* p);
char* format_prop(const Prop* p);

#endif
