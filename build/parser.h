/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_F_2025AUTUMN_CS2612_CS2612_MATERIALS_EXPERIMENT_BIGWORK_BUILD_PARSER_H_INCLUDED
# define YY_YY_F_2025AUTUMN_CS2612_CS2612_MATERIALS_EXPERIMENT_BIGWORK_BUILD_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 11 "F:/2025Autumn/CS2612/CS2612_materials/experiment/bigwork/src/parser.y"

#include "syntax.h"
typedef struct TermList { Term** items; int n; } TermList;

#line 54 "F:/2025Autumn/CS2612/CS2612_materials/experiment/bigwork/build/parser.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    BP_IDENT = 258,                /* BP_IDENT  */
    BP_NUMBER = 259,               /* BP_NUMBER  */
    BP_LPAREN = 260,               /* BP_LPAREN  */
    BP_RPAREN = 261,               /* BP_RPAREN  */
    BP_COMMA = 262,                /* BP_COMMA  */
    BP_DOT = 263,                  /* BP_DOT  */
    BP_NOT = 264,                  /* BP_NOT  */
    BP_AND = 265,                  /* BP_AND  */
    BP_OR = 266,                   /* BP_OR  */
    BP_IMPLY = 267,                /* BP_IMPLY  */
    BP_IFF = 268,                  /* BP_IFF  */
    BP_FORALL = 269,               /* BP_FORALL  */
    BP_EXISTS = 270                /* BP_EXISTS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 27 "F:/2025Autumn/CS2612/CS2612_materials/experiment/bigwork/src/parser.y"

  int num;
  char* str;
  Term* term;
  UPredicate* pred;
  Prop* prop;
  TermList tlist;

#line 95 "F:/2025Autumn/CS2612/CS2612_materials/experiment/bigwork/build/parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_F_2025AUTUMN_CS2612_CS2612_MATERIALS_EXPERIMENT_BIGWORK_BUILD_PARSER_H_INCLUDED  */
