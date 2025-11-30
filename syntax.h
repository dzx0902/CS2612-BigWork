#ifndef FOL_H
#define FOL_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct UFunction UFunction;
typedef struct UPredicate UPredicate;
typedef struct Term Term;
typedef struct Prop Prop;
typedef enum TermType TermType;
typedef enum PropBop PropBop;
typedef enum PropUop PropUop;
typedef enum PropQuant PropQuant;
typedef enum PropType PropType;

enum TermType {
    Term_UFTerm,
    Term_ConstNum, 
    Term_VarName  
};

enum PropBop{
    Prop_AND = Term_VarName+1, 
    Prop_OR, Prop_IMPLY, Prop_IFF
};

enum PropUop{
    Prop_NOT = Prop_IFF+1 
};

enum PropQuant{
    Prop_FORALL = Prop_NOT+1,
    Prop_EXISTS,
};

enum PropType {
    Prop_Binop = Prop_EXISTS + 1,
    Prop_Unop,
    Prop_Quant,
    Prop_Atom,
};

struct UFunction{
    char* name; // 函数名
    int numArgs; // 参数数量
    Term** args; // 参数数组
};

struct UPredicate{
    char* name; // 谓词名
    int numArgs; // 参数数量
    Term** args; // 参数数组
};

struct Term{
    TermType type;
    union {
        int ConstNum;      //同时可以表示变量的编号
        char* Variable; 
        UFunction* UFTerm;
    } term;
};

struct Prop {
    PropType type;
    union {
        struct {
            PropBop op;
            Prop *prop1, *prop2;
        } Binary_prop;
        struct {
            PropUop op;
            Prop *prop1;
        } Unary_prop;
        struct {
            PropQuant op;
            char* Variable;
            Prop *prop1;
        } Quant_prop;
        UPredicate * Atomic_prop;
    } prop;
};

// AST construction helpers
Term* new_term_variable(const char* name);
Term* new_term_const(int num);
Term* new_term_function(UFunction* func);
UFunction* new_function(const char* name, int numArgs, Term** args);
UPredicate* new_predicate(const char* name, int numArgs, Term** args);
Prop* new_prop_binop(PropBop op, Prop* left, Prop* right);
Prop* new_prop_unop(PropUop op, Prop* child);
Prop* new_prop_quant(PropQuant op, const char* var, Prop* body);
Prop* new_prop_atom(UPredicate* pred);

// Memory cleanup helpers
void free_term(Term* term);
void free_function(UFunction* func);
void free_predicate(UPredicate* pred);
void free_prop(Prop* prop);

// Logical transforms
Prop* clone_prop(const Prop* prop);
Prop* expand_iff(const Prop* prop);

// Polarity analysis
void analyze_polarity(const Prop* prop, bool positive);
char* format_polarity(const Prop* prop, bool positive);

// Parser entry points (provided by Bison/Flex)
int yyparse(void);
void yyerror(const char* s);
extern Prop* g_root;

#endif
