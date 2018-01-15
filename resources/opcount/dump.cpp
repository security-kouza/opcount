/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#include "lisp.h"
#include "ast.h"
#include "reserved_word.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

using namespace std ;

static cell * process_all(cell * L) ;

// =====================================================================
// real process
// =====================================================================

#ifdef __CYGWIN__
static string my_to_string( int value ){
  if(value){
    string m = "" ;
    string s = "" ;
    if(value<0){
      value = -value ;
      m = "-" ;
    }
    while(value){
      static char const * const t[] = {
        "0", "1", "2", "3", "4",
        "5", "6", "7", "8", "9"
      } ;
      s = t[value % 10] + s ;
      value /= 10 ;
    }
    return m+s ;
  }else{
    return "0" ;
  }
}
#define to_string my_to_string 
#endif

static char * new_label(){
  static int count = 0 ;
  return strdup((string("label_") + to_string(count++)).c_str()) ;
}

// =====================================================================
// stub process
// =====================================================================

static cell * process_LITERAL (cell * L){
  cell * lineno = L->cdr ;
  cell * arg0   = lineno->cdr ;
  char * label  = new_label() ;
  printf("static cell * %s = LIST(%s,&loc,unique_symbol(\"%s\"));\n",
    label, L->car, arg0->car) ;
  return LIST(label) ;
}

static cell * process_N_ARY(cell * L){
  cell * lineno = L->cdr ;
  cell * arg0   = lineno->cdr ;
  cell * ret0   = NULL ;
  for(;arg0;arg0=arg0->cdr)
    ret0 = APPEND(ret0, process_all((cell*)arg0->car)) ;
  char * label ;
  if(L->car == AST_PROG){
    label  = "PREAMBLE" ;
  }else{
    label  = new_label() ;
    printf("static ") ;
  } 
  printf("cell * %s = LIST(%s,&loc", label, L->car) ;
  for(;ret0;ret0=ret0->cdr) printf(",%s", ret0->car) ;
  printf(");\n");
  return LIST(label) ;
}

// =====================================================================
// dispatch process
// =====================================================================

static cell * process_all(cell * L) {
  if(
      (L->car == AST_NUMBER)                  ||
      (L->car == AST_CHAR)                    ||
      (L->car == AST_STRING)                  ||
      (L->car == AST_IDENTIFIER)              ||
      (L->car == AST_NEW_IDENTIFIER)          ||
      (L->car == AST_STORAGE_CLASS_SPECIFIER) ||
      (L->car == AST_TYPE_QUALIFIER)
  )    return process_LITERAL(L) ;
  else return process_N_ARY  (L) ;
}

extern "C" {
#include "syntactic.h"
}

extern "C" int yyparse(void) ;
extern "C" cell * AST ;

int main(int argc, char **argv) {
  if(yyparse() == 0){
    printf("%s",
      "#include \"lisp.h\"\n"
      "#include \"ast.h\"\n"
      "#include \"preamble.h\"\n"
      "#include \"yacclex.h\"\n"
      "#include \"reserved_word.h\"\n"
      "static YYLTYPE loc = {\n"
      "   0, // first_line\n"
      "   0, // first_column\n"
      "   1, // last_line\n"
      "   0, // last_column\n"
      "  -1, // first_loc\n"
      "  -1, // last_loc\n"
      "} ;\n"
    ) ;
    process_all(AST) ;
  }else{
    // syntactic error
    exit(1) ;
  }
  return EXIT_SUCCESS ;
}
