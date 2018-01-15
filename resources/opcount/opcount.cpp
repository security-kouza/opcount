/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#include <iostream>
#include <fstream>

#include "lisp.h"
#include "semantic.h"
#include "preamble.h"

#define YY_BUF_SIZE 32768

extern "C" {
  extern int yyparse(void) ;
  extern cell * AST ;
  extern FILE * yyin ;

  typedef struct yy_buffer_state * YY_BUFFER_STATE ;
  extern YY_BUFFER_STATE yy_create_buffer( FILE *file, int size) ;
  extern void yy_switch_to_buffer( YY_BUFFER_STATE new_buffer) ;
  extern void yy_delete_buffer( YY_BUFFER_STATE buffer ) ;
  extern void yy_flush_buffer(YY_BUFFER_STATE buffer) ;
  extern YY_BUFFER_STATE yy_scan_string(char * str);
}

using namespace std ;
using namespace boost ;

extern reduced * process_all(cell * L) ;

#define OPCOUNT_VERSION "0.1"

int main(int argc, char ** argv){

  int RET = EXIT_SUCCESS ;
  int print = 0 ;
  int i ;

  for(i=1; i<argc; i++){
    if(string(argv[i]) == "-t"){ i++ ; break ; }
    if(string(argv[i]) == "-p"){
      print = 1 ; 
    }else if(string(argv[i]) == "-v"){
      cout << "OPCOUNT_VERSION = " << OPCOUNT_VERSION << " ;" << endl ;
    }else{
      try{
        std::ifstream ifs ;
        ifs.exceptions(std::ifstream::failbit) ;
        ifs.open(argv[i]) ;
        try{
          char c = ifs.get();
          while (ifs.good()) {
            std::cout << c ;
            c = ifs.get();
          }
        }catch (...){ }
      }catch (...){
        std::cerr << "I/O error, file name: " << argv[i] << endl ;
        RET = EXIT_FAILURE ;
      }
    }
  }

  yyin = fdopen(0,"r") ;
  yy_switch_to_buffer(yy_create_buffer(yyin,YY_BUF_SIZE));
  if(yyparse() != 0){
    exit(1) ; // syntactic error
  }
  {
    cell * A = AST ;
    process_all(PREAMBLE) ;
    process_all(A) ;
  }

  for(; i<argc; i++){
    if(string(argv[i]) == "-p"){
      print = 1 ; 
    }else if(string(argv[i]) == "-v"){
      cout << "OPCOUNT_VERSION = " << OPCOUNT_VERSION << " ;" << endl ;
    }else{
      try{
        std::ifstream ifs ;
        ifs.exceptions(std::ifstream::failbit) ;
        ifs.open(argv[i]) ;
        try{
          char c = ifs.get();
          while (ifs.good()) {
            std::cout << c ;
            c = ifs.get();
          }
        }catch (...){ }
      }catch (...){
        std::cerr << "I/O error, file name: " << argv[i] << endl ;
        RET = EXIT_FAILURE ;
      }
    }
  }

  if(print){
    cout << endl ;
    for(auto f : function_stack){
      cout << "print(\"" << f << " = \", eval(call_" << f << "));" << endl ;
    }
  }

  return RET ;
}
