/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#ifndef DECLARATION_H
#define DECLARATION_H
#include "lisp.h"
#include <vector>
#include <boost/unordered_map.hpp>

enum declarator_type {
  IDENTIFIER_DECLARATOR =   1,
  STRUCT_DECLARATOR     =   2,
  UNION_DECLARATOR      =   4,
  ENUM_DECLARATOR       =   8,

  ENUM_CONST            =  16,
  TYPEDEF_DECLARATOR    =  32,
  FUNC_DEFINITION       =  64,
  FUNC_DECLARATOR       = 128,

  ARRAY_DECLARATOR      = 256,
  POINTER_DECLARATOR    = 512,
  REFERENCE_DECLARATOR  =1024,
  MACRO_DEFINITION      =2048,
} ;

/*
extern unsigned long long history_date ;

struct hist_entry : std::vector<excell *> {
  typedef std::vector<excell *> parent ;
  std::vector<unsigned long long> date ;
  hist_entry() : parent(), date() {}
  void push_back (excell * val){
    ((parent*)this)->push_back(val) ;
    date.push_back(history_date++) ;
  }
} ;

extern boost::unordered_map<char *, hist_entry> history ;
*/

struct declaration_stack_entry {
  char                                 * symbol                 ;
  cell                                 * declaration_specifiers ;
  cell                                 * init_declarator        ;
  int                                    type                   ;
  std::vector<declaration_stack_entry>   option                 ;
// ----------------------------------------------------------------------
  char                                 * true_name              ;
  cell                                 * declaration_list       ;
  cell                                 * compound_statement     ;
  int                                    chain ;
// ----------------------------------------------------------------------
} ;

extern std::vector<const cell *>            declaration_specifiers_stack ;
extern std::vector<declaration_stack_entry> declaration_stack ;
extern std::vector<int>                     declaration_frame_stack ;

extern declaration_stack_entry resolve_type(char * symbol) ;
extern int same_declaration (cell * X, cell * Y, int exp = 0, int func = 0) ;
inline int same_declaration_func(cell * X, cell * Y, int exp = 0){ return same_declaration (X, Y, exp, 1) ; }

extern void declaration_specifiers_stack_push(cell * declaration_specifiers);
extern void declaration_specifiers_stack_pop ();
extern int  symbol_to_declaration_stack_entry(char * symbol, int n = 0);
extern int  true_name_to_declaration_stack_entry (char * true_name, int n = 0);

extern int  declaration_stack_pointer        ();
extern void declaration_stack_push           (cell * init_declarator);
extern void declaration_stack_restore        (int n);
extern int  declaration_current_frame        ();
extern void declaration_frame_stack_push     ();
extern void declaration_frame_stack_pop      ();
extern std::ostream & operator<<(std::ostream& out, const declaration_stack_entry & dse);
extern void print_declaration_stack          (std::string s = "");

extern cell * copy_ast (cell * L) ;
extern void   free_ast (cell * L) ;
extern char * major_declaration_specifier(cell * L) ;

extern void remove_macro(cell * A) ;

#endif /* DECLARATION_H */
