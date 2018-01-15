/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#include <iostream>
#include <boost/unordered_map.hpp>
#include <vector>
#include <string.h>
#include <limits.h>
#include "yacclex.h"
#include "syntactic.h"
#include "reserved_word.h"
#include "lisp.h"
#include "declaration.h"
#include "ast.h"
#include "ast2str.h"
#include <boost/format.hpp>

/*
struct declaration_stack_entry {
  const char                           * symbol                 ;
  cell                                 * declaration_specifiers ;
  cell                                 * init_declarator        ;
  int                                    type                   ;
  std::vector<declaration_stack_entry>   option                 ;
} ;
*/

using namespace std ;
using namespace boost ;
using boost::format;
using boost::io::group;

vector<const cell *>            declaration_specifiers_stack ;
vector<declaration_stack_entry> declaration_stack ;
vector<int>                     declaration_frame_stack ;

/*
unsigned long long history_date ;
unordered_map<char *, hist_entry> history ;
*/

cell * copy_ast (cell * L) {
  cell * R = NULL ;
  if(L){
    cell * loc = L->cdr ;
    cell * arg = loc->cdr ;
    if(is_literal(L)){
      R = LIST(L->car, loc->car, arg->car) ;
    }else{
      R = LIST(L->car, loc->car) ;
      for(;arg;arg=arg->cdr) {
        R = INTEGRATE(R, copy_ast((cell*)arg->car)) ;
      }
    }
  }
  return R ;
}

void free_ast (cell * L) {
  if(L){
    cell * loc = L->cdr ;
    cell * arg = loc->cdr ;
    if(! is_literal(L)) for(;arg;arg=arg->cdr) free_ast((cell*)arg->car) ;
    free(L) ;
  }
}

static int insert_ast (char * symbol, cell * A, cell * B) { // insert A into B
  cell * pre = B  ->cdr ; // location
  cell * cur = pre->cdr ; // first arg
  for(;cur;pre=cur, cur=cur->cdr){
    cell * L   = (cell*)(cur->car) ;
    if(is_literal(L)){
      cell * loc = L  ->cdr ;
      cell * arg = loc->cdr ;
      if(arg->car == symbol){

        cell * locA = A   ->cdr ;
        cell * argA = locA->cdr ;
        pre->cdr = APPEND(argA, cur->cdr) ;

	free(locA) ;
	free(A) ;
	free(arg) ;

	free(loc) ;
	free(L  ) ;
	free(cur) ;
	return 1 ;
      }
    }
  }
  return 0 ;
}

static int embed_ast (char * symbol, cell * A, cell * B) { // embed A into B
  int ret = 0 ;
  if(B){
    cell * loc = B  ->cdr ;
    cell * arg = loc->cdr ;
    if(is_literal(B)){
      if(arg->car == symbol){
        B->car = AST_PARENTHESIS ;
        arg->car = (char *)A ;
        ret = 1 ;
      }
    }else{
      for(;arg;arg=arg->cdr) {
        if((ret |= embed_ast (symbol, A, (cell*)arg->car))) break ;
      }
    }
  }
  return ret ;
}

void remove_macro(cell * A) {
  if(A){
    cell * pre = A  ->cdr ; // location
    cell * cur = pre->cdr ; // first arg
    for(;cur;pre=cur, cur=cur->cdr){
      cell * L   = (cell*)(cur->car) ;
      if(L->car == AST_STORAGE_CLASS_SPECIFIER){
        cell * loc = L  ->cdr ;
        cell * arg = loc->cdr ;
        if(arg->car == SYMBOL_MACRO){
          pre->cdr = cur->cdr ;
          free(arg) ;
          free(loc) ;
          free(L  ) ;
          free(cur) ;
          cur = pre ;
        }
      }
    }
  }
}

static void remove_extern(cell * A) {
  if(A){
    cell * pre = A  ->cdr ; // location
    cell * cur = pre->cdr ; // first arg
    for(;cur;pre=cur, cur=cur->cdr){
      cell * L   = (cell*)(cur->car) ;
      if(L->car == AST_STORAGE_CLASS_SPECIFIER){
        cell * loc = L  ->cdr ;
        cell * arg = loc->cdr ;
        if(arg->car == SYMBOL_EXTERN){
          pre->cdr = cur->cdr ;
          free(arg) ;
          free(loc) ;
          free(L  ) ;
          free(cur) ;
          cur = pre ;
        }
      }
    }
  }
}

static void remove_typedef(cell * A) {
  if(A){
    cell * pre = A  ->cdr ; // location
    cell * cur = pre->cdr ; // first arg
    for(;cur;pre=cur, cur=cur->cdr){
      cell * L   = (cell*)(cur->car) ;
      if(L->car == AST_STORAGE_CLASS_SPECIFIER){
        cell * loc = L  ->cdr ;
        cell * arg = loc->cdr ;
        if(arg->car == SYMBOL_TYPEDEF){
          pre->cdr = cur->cdr ;
          free(arg) ;
          free(loc) ;
          free(L  ) ;
          free(cur) ;
          cur = pre ;
        }
      }
    }
  }
}

static declaration_stack_entry reduce_typedef(
  declaration_stack_entry & definition_of_variable,
  declaration_stack_entry & definition_of_type
){
  char * & symbol = definition_of_type.symbol ;
  cell * dsv = copy_ast(definition_of_variable.declaration_specifiers) ;
  cell * dst = copy_ast(definition_of_type    .declaration_specifiers) ;
  cell * idv = copy_ast(definition_of_variable.init_declarator       ) ;
  cell * idt = copy_ast(definition_of_type    .init_declarator       ) ;
  remove_typedef(dsv) ;
  remove_typedef(dst) ;
  if(symbol){
    insert_ast (symbol, dst, dsv) ;
    embed_ast  (symbol, idv, idt) ;
    declaration_stack_entry ret = {
      definition_of_variable.symbol, dsv, idt, definition_of_type.type, {}
    } ;
    return ret ;
  }else{
    declaration_stack_entry ret = {
      definition_of_variable.symbol, dsv, idv, 
      definition_of_variable.type, 
      definition_of_variable.option
    } ;
    return ret ;
  }
}

char * major_declaration_specifier(cell * L) {
  for(cell * a = lisp_nthcdr(2,L) ; a ; a = a->cdr){
    cell * b = a->element(0) ;
    char * t = (char *)(b->element(0)) ;
    if(t == AST_IDENTIFIER){
      char * symbol = (char *)(b->element(2)) ;
      if(!is_reserved_word(symbol)) return symbol ;
    }else if((t == AST_STRUCT) || (t == AST_UNION) || (t == AST_ENUM)){
      cell * c      =          b->element(2)  ;
      char * symbol = (char *)(c->element(2)) ;
      return symbol ;
    }
  }
  return NULL ;
}

static int is_struct(cell * L) {
  for(cell * a = lisp_nthcdr(2,L) ; a ; a = a->cdr){
    cell * b = a->element(0) ;
    char * t = (char *)(b->element(0)) ;
    if((t == AST_STRUCT) || (t == AST_UNION) || (t == AST_ENUM)) return 1;
  }
  return 0 ;
}

declaration_stack_entry resolve_type(char * symbol) {
  symbol = (char *)unique_symbol(symbol) ;
  int i = symbol_to_declaration_stack_entry(symbol) ;
  if(i>=0){
    declaration_stack_entry dse_t = {NULL};
    declaration_stack_entry dse   = declaration_stack[i] ;
    dse = reduce_typedef(dse, dse_t) ;
    char * s  = major_declaration_specifier(dse.declaration_specifiers) ;
    if(s){
      for(
        int j = symbol_to_declaration_stack_entry(s,i) ;
        (j >= 0) && s ;
        j = symbol_to_declaration_stack_entry(s,j)
      ){
	dse_t = declaration_stack[j] ;
        dse   = reduce_typedef(dse, dse_t) ;
        s  = major_declaration_specifier(dse.declaration_specifiers) ;
        if(!s) break ;
        if(is_struct(dse.declaration_specifiers)) break ;
      }
    }
    return dse ;
  }else{
    declaration_stack_entry dse = { symbol, NULL, NULL, 0, {} } ;
    return dse ;
  }
}

int same_declaration (cell * X, cell * Y, int exact, int func){
  if(!X && !Y)                  return 1;
  if(!X)                        return 0;
  if(!Y)                        return 0;
  if(X->car == AST_PARENTHESIS) return same_declaration (X->element(2),Y,exact,func);
  if(Y->car == AST_PARENTHESIS) return same_declaration (X,Y->element(2),exact,func);
  if(X->car == AST_INIT)        return same_declaration (X->element(2),Y,exact,func);
  if(Y->car == AST_INIT)        return same_declaration (X,Y->element(2),exact,func);
  if(X->car == AST_DECLARATOR) if(X->element(2)->car == AST_NULL)
                                return same_declaration (X->element(3),Y,exact,func);
  if(Y->car == AST_DECLARATOR) if(Y->element(2)->car == AST_NULL)
                                return same_declaration (X,Y->element(3),exact,func);
  if(X->car != Y->car)          
//    if(! (((X->car == AST_IDENTIFIER) && (Y->car == AST_NEW_IDENTIFIER))||
//          ((X->car == AST_NEW_IDENTIFIER) && (Y->car == AST_IDENTIFIER)))) 
	                        return 0;

  if(func) if(X->car == AST_PARAMETER_LIST) return 1 ;

  if(is_literal(X)){
    if(exact)return X->element(2) == Y->element(2) ;
    else   return 1 ;
  }

  if(X->car == AST_DECLARATION_SPECIFIERS) exact = 1 ;

  for( X=X->pointer(2), Y=Y->pointer(2) ; X && Y ; X=X->cdr, Y=Y->cdr) {
    if(!same_declaration(X->element(0),Y->element(0),exact,func)) return 0 ;
  }
  if(X)                         return 0;
  if(Y)                         return 0;
                                return 1;
}

void declaration_specifiers_stack_push(cell * declaration_specifiers){
  declaration_specifiers_stack.push_back(declaration_specifiers) ;
}

void declaration_specifiers_stack_pop(){
  declaration_specifiers_stack.pop_back() ;
}

int symbol_to_declaration_stack_entry (char * symbol, int n){
  symbol = unique_symbol(symbol) ;
  if(n==0) n = declaration_stack.size();
  for(int i = n-1; i>=0; i--){
    if(symbol == declaration_stack[i].symbol) return i ;
  }
  return -1 ;
}

int true_name_to_declaration_stack_entry (char * true_name, int n){
  true_name = unique_symbol(true_name) ;
  if(n==0) n = declaration_stack.size();
  for(int i = n-1; i>=0; i--){
    if(true_name == declaration_stack[i].true_name) return i ;
  }
  return -1 ;
}

int declaration_stack_pointer(){ return declaration_stack.size() ; }
int declaration_frame_stack_pointer(){ return declaration_frame_stack.size() ; }

// extern "C" char * init_declarator_to_symbol (const cell * L) ;

static int in_frame(char * symbol){
  char * u  = unique_symbol(symbol) ;
  int    sp = declaration_current_frame() ;
  int    n  = declaration_stack.size() ;
  for(int i = sp; i<n; i++) if(u == declaration_stack[i].symbol) return 1 ;
  return 0 ;
}

int is_func(cell * L) {
  int ret = 0 ;
  if(L){
    cell * lineno    = L->cdr ;
    cell * arg0      = lineno->cdr ;
    if(L->car == AST_FUNC_DECLARATOR        ){
      ret = 1 ;
    }else if(is_literal(L)){
      ret = 0 ;
    }else{
      for(;arg0;arg0=arg0->cdr){
	if(is_func((cell*)arg0->car)){
	  ret = 1 ;
	  break ;
	}
      }
    }
  }
  return ret ;
}

void declaration_stack_push(cell * init_declarator){
  extern void semantic_error(cell * L, const string & message) ;
//  cout << init_declarator << endl ;
  char * symbol = init_declarator_to_symbol(init_declarator) ; //<-=====================
  int id_is_func = is_func(init_declarator);

  cell * declaration_specifiers = (cell *)declaration_specifiers_stack.back() ;
  int    chain = -1 ;

  if(in_frame(symbol)){
    if(! is_extern(declaration_specifiers) && ! id_is_func)
      semantic_error(init_declarator, string("duplicate declaration : ") + symbol) ;
    int i = symbol_to_declaration_stack_entry (symbol) ;

    cell * A = copy_ast(declaration_specifiers) ;
    cell * B = copy_ast(declaration_stack[i].declaration_specifiers) ;
    remove_extern(A) ;
    remove_extern(B) ;
    if(!(same_declaration(A,B) && same_declaration(init_declarator, declaration_stack[i].init_declarator))) {
      semantic_error(init_declarator, string("duplicate declaration : ") + symbol) ;
    }
    free_ast(A) ;
    free_ast(B) ;

    chain = i ;
  }

  declaration_stack_entry e = {symbol, declaration_specifiers, init_declarator, 0, {}, NULL, NULL, NULL, chain } ;

  if(id_is_func) e.type |= FUNC_DECLARATOR ;
  e.true_name = new_symbol(symbol) ;

  declaration_stack.push_back(e);
//  print_declaration_stack("push") ; // debug
}

void declaration_stack_restore(int n){
  while(declaration_stack.size() > n) declaration_stack.pop_back() ;
}

int declaration_current_frame(){
  if(declaration_frame_stack.size()>0){
    return declaration_frame_stack.back() ;
  }else{
    return 0 ;
  }
}

void declaration_frame_stack_push(){
  declaration_frame_stack.push_back(declaration_stack_pointer()) ;
}

void declaration_frame_stack_pop(){
  if(declaration_frame_stack.size()>0){
    declaration_stack_restore(declaration_frame_stack.back()) ;
    declaration_frame_stack.pop_back();
  }
//  print_declaration_stack("pop"); // debug
}

ostream & operator<<(ostream& out, const declaration_stack_entry & dse){
  return
    out << dse.symbol
        << "|" << dse.true_name
	<< "|" << ast2str(dse.declaration_specifiers) << " " << ast2str(dse.init_declarator)
	<< "|opt : " << dse.option.size()
	<< "|type: " << format("%04X") % dse.type
	<< "|addr: " << format("%p") % &dse
	<< "|" // << dse.compound_statement
    ;
}

void print_declaration_stack(string s){
  cout << "=== " << s << " begin ===" << endl ;
  int n = declaration_frame_stack_pointer() - 1;
  int i = declaration_stack_pointer()       - 1;

  for( ; i>=0;i--){
    while((n>=0)&&(declaration_frame_stack[n] > i)){
      cout << "-- " << declaration_frame_stack[n] << " --" << endl ;
      n-- ;
    }
    if(n>=0){
      cout << declaration_frame_stack[n] << ":" << i << ":" << declaration_stack[i] << endl ;
    }else{
      cout << "0" << ":" << i << ":" << declaration_stack[i] << endl ;
    }
  }
  cout << "--- " << s << " end ---" << endl ;
}
