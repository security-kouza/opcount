/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#ifndef YACCLEX_H
#define YACCLEX_H
#include "lisp.h"
#ifdef __cplusplus
extern "C" {
#endif

#define YYSTYPE_IS_DECLARED
typedef void * YYSTYPE ;

#define YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  int first_loc ;
  int last_loc ;
  int length ;
  char * first_filename ;
  char * last_filename ;
} YYLTYPE ;

extern YYSTYPE yylval ;
extern YYLTYPE yylloc ;

#define YYLLOC_DEFAULT(Current, Rhs, N)                                 \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
          (Current).first_loc    = YYRHSLOC (Rhs, 1).first_loc;         \
          (Current).last_loc     = YYRHSLOC (Rhs, N).last_loc;          \
          (Current).length = (Current).last_loc-(Current).first_loc+1;  \
          (Current).first_filename= YYRHSLOC (Rhs, 1).first_filename;   \
          (Current).last_filename= YYRHSLOC (Rhs, N).last_filename;     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
          (Current).first_loc = (Current).last_loc =                    \
            YYRHSLOC (Rhs, 0).last_loc;                                 \
          (Current).length = (Current).last_loc-(Current).first_loc+1;  \
          (Current).first_filename = (Current).last_filename =          \
            YYRHSLOC (Rhs, 0).last_filename;                            \
        }                                                               \
    while (0)

extern int    is_literal                (cell * L) ;
extern int    is_typedef                (cell * L) ;
extern int    is_macro                  (cell * L) ;
extern int    is_extern                 (cell * L) ;
extern int    is_prohibited             (cell * L) ;
extern int    is_group                  (cell * L) ;

extern char * init_declarator_to_symbol (cell * L) ;

#ifdef __cplusplus
}
#endif
#endif /* YACCLEX_H */
