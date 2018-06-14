/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#include <iostream>
#include <vector>
#include <stdint.h>
#include <boost/unordered_map.hpp>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>


#include "lisp.h"
#include "ast.h"
#include "reserved_word.h"
#include "preamble.h"
#include "semantic.h"
#include "yacclex.h"
#include "declaration.h"
#include "ast2str.h"

using namespace std   ;
using namespace boost ;

#define YY_BUF_SIZE 32768

extern "C" {
  extern int yyparse(void) ;
  extern cell * AST ;

  typedef struct yy_buffer_state * YY_BUFFER_STATE ;
  extern YY_BUFFER_STATE yy_create_buffer( FILE *file, int size) ;
  extern void yy_switch_to_buffer( YY_BUFFER_STATE new_buffer) ;
  extern void yy_delete_buffer( YY_BUFFER_STATE buffer ) ;
  extern void yy_flush_buffer(YY_BUFFER_STATE buffer) ;
  extern YY_BUFFER_STATE yy_scan_string(char * str);
}

cell * yyparse(char * s){
  cell * R = NULL ;
  YY_BUFFER_STATE str_buffer = yy_scan_string(s);
  yy_switch_to_buffer(str_buffer) ;
  if(yyparse()==0)R = AST ;
  yy_flush_buffer(str_buffer) ;
  yy_delete_buffer(str_buffer) ;
  return R ;
}

static YYLTYPE dummy_location ;
cell * dummy_LOCATION = LIST(&dummy_location) ;

static string location(cell * L){
  YYLTYPE * location = (YYLTYPE *) L->element(1) ;
  string s = "line "                     ;
  s += to_string(location->first_line)   ;
  s += ", column "                       ;
  s += to_string(location->first_column) ;
  return s ;
}

static void unexpected_error(cell * L, const string & message) {
  cerr << "unexpected error at " << location(L) << ": " << message << endl
       << "abstruct syntax tree node: " << L->car << endl ;
  exit(1) ;
}

void warning(cell * L, const string & message) {
  cerr << "warning at " << location(L) << ": " << message << endl ;
}

void semantic_error(cell * L, const string & message) {
  cerr << "semantic error at " << location(L) << ": " << message << endl ;
  exit(1);
}

// ===================================================================================

struct reduced {
  char    * type    = 0 ;
  formula   total   = formula() ;
  int       left    = 0 ;
  formula   batch   = formula() ;
  int       literal = 0 ;
  long long value   = 0 ;

  inline    reduced (
    char    * type_    = 0,
    formula   total_   = formula() ,
    int       left_    = 0,
    formula   batch_   = formula(),
    int       literal_ = 0,
    long long value_   = 0
  ) : type   (type_   ),
      total  (total_  ),
      left   (left_   ),
      batch  (batch_  ),
      literal(literal_),
      value  (value_  )
  {}

  inline void transfer_batch(){
    if(batch["e"]){
      total += formula( string(type + 5) +
        "_batch(" + to_string(batch["e"]) + ","
	          + to_string(batch["b"]) + ")"
      ) ;
    }
    if(batch["p"]){
      total += formula(
        "pairing_batch(" + to_string(batch["p"]) + ")"
      ) ;
      total -= (batch["p"]-INTEGER(1)) * formula("TARGET_mul") ;
    }
    batch  = formula() ;
  }

  inline void transfer_bitexp(string n){
    if(batch["e"]){
      total += formula( string(type + 5) +
        "_bitexp(" + n                     + ","
	           + to_string(batch["e"]) + ","
	           + to_string(batch["b"]) + ")"
      ) ;
    }
    if(batch["p"]){
      total += formula(
        "pairing_batch(" + to_string(batch["p"]) + ")"
      ) ;
      total -= (batch["p"]-INTEGER(1)) * formula("TARGET_mul") ;
    }
    batch  = formula() ;
  }


} ;

inline int    is_left_value(reduced * X){
  if(!X)return 0 ;
  return X->left ; 
}
inline char * type_of(reduced * X){ 
  if(!X)return "NULL" ;
  return X->type ; 
}


// ===================================================================================
#define OPERATION_ERROR1() semantic_error(L, string(__func__) + "(" + X->type + ") error in : " + ast2str(L))
#define OPERATION_ERROR2() semantic_error(L, string(__func__) + "(" + X->type + "," + Y->type + ") error in : " + ast2str(L))
#define DEBUG(X)           warning(L, "debug: " + string(__func__) + ": " + X + " in : " + ast2str(L))

reduced * GENERATE_CRS_if_listvar(reduced*, cell_s*){ return NULL ; }
reduced * operation_ARRAY_REF(reduced*, reduced*, cell_s*){ return NULL ; }
reduced * operation_DOT(reduced*, reduced*, cell_s*){ return NULL ; }

reduced * operation_EQ(reduced * X, reduced * Y, cell_s * L){
  if(!X) return NULL ;
  if(!Y) return NULL ;
  /* <== list の場合再帰的に */
  if(X->type != Y->type){
    if((X->type != TYPE_INTEGER) && (Y->type != TYPE_INTEGER)){
      OPERATION_ERROR2() ;
    }
  }

  X->transfer_batch() ;
  Y->transfer_batch() ;

  string s = X->type + 5 ;
  s += "_eq" ;
  return new reduced(TYPE_INTEGER, X->total + Y->total + formula(s)) ;
}

reduced * operation_GS_COMMIT(reduced*, reduced*, cell_s*){ return NULL ; }
reduced * operation_GS_DUMMY_COMMIT(reduced*, reduced*, cell_s*){ return NULL ; }

reduced * operation_LOGICAL_AND(reduced * X, reduced * Y, cell_s * L){
  if(!X) return NULL ;
  if(!Y) return NULL ;
  /* <== list の場合再帰的に */
  if(!((X->type == TYPE_INTEGER)&&(Y->type == TYPE_INTEGER))) OPERATION_ERROR2() ;

  X->transfer_batch() ;
  Y->transfer_batch() ;

  string s = X->type + 5 ;
  s += "_logical_and" ;
  return new reduced(X->type, X->total + Y->total + formula(s)) ;
}

reduced * operation_ADD(reduced * X, reduced * Y, cell * L){
  if(!X) return NULL ;
  if(!Y) return NULL ;
  /* <== list の場合再帰的に */
  if(!((X->type == TYPE_INTEGER)&&(Y->type == TYPE_INTEGER))) OPERATION_ERROR2() ;

  X->transfer_batch() ;
  Y->transfer_batch() ;

  if( (X->literal) && (X->value == 0) ) return Y ;
  if( (Y->literal) && (Y->value == 0) ) return X ;

  string s = X->type + 5;
  s += "_add" ;
  return new reduced(X->type, X->total + Y->total + formula(s)) ;
}

reduced * operation_SUB(reduced * X, reduced * Y, cell * L){
  extern reduced * operation_UMINUS (reduced * X, cell * L) ;
  if(!X) return NULL ;
  if(!Y) return NULL ;
  /* <== list の場合再帰的に */
  if(!((X->type == TYPE_INTEGER)&&(Y->type == TYPE_INTEGER))) OPERATION_ERROR2() ;

  X->transfer_batch() ;
  Y->transfer_batch() ;

  if( (X->literal) && (X->value == 0) ) return operation_UMINUS(Y,L) ;
  if( (Y->literal) && (Y->value == 0) ) return X ;

  string s = X->type + 5 ;
  s += "_sub" ;
  return new reduced(X->type, X->total + Y->total + formula(s)) ;
}

reduced * operation_MUL(reduced * X, reduced * Y, cell * L){
  if(!X) return NULL ;
  if(!Y) return NULL ;
  /* X*1 */

  if(
    (Y->type == TYPE_INTEGER) && (Y->literal) && (Y->value == 1)
  ){
    if(
        (X->type == TYPE_INTEGER)||
        (X->type == TYPE_GROUP)  ||
        (X->type == TYPE_GROUP0) ||
        (X->type == TYPE_GROUP1) ||
        (X->type == TYPE_GROUP2) ||
        (X->type == TYPE_DUPLEX) ||
        (X->type == TYPE_PROHIBITED)||
        (X->type == TYPE_TARGET)
    ) return X ;
  }

  /* 1*Y */

  if(
    (X->type == TYPE_INTEGER) && (X->literal) && (X->value == 1)
  ){
    if(
        (Y->type == TYPE_INTEGER)||
        (Y->type == TYPE_GROUP)  ||
        (Y->type == TYPE_GROUP0) ||
        (Y->type == TYPE_GROUP1) ||
        (Y->type == TYPE_GROUP2) ||
        (Y->type == TYPE_DUPLEX) ||
        (Y->type == TYPE_PROHIBITED)||
        (X->type == TYPE_TARGET)
    ) return Y ;
  }

  /* <== list の場合再帰的に */
  if(!(((X->type == TYPE_INTEGER)||
        (X->type == TYPE_GROUP)  ||
        (X->type == TYPE_TARGET) ||
        (X->type == TYPE_GROUP0) ||
        (X->type == TYPE_GROUP1) ||
        (X->type == TYPE_GROUP2) ||
        (X->type == TYPE_DUPLEX) ||
        (X->type == TYPE_PROHIBITED)
       )&&(
        (X->type == Y->type)
  )))OPERATION_ERROR2() ;
  if((X->type == TYPE_TARGET) ||
     (X->type == TYPE_GROUP)  ||
     (X->type == TYPE_GROUP0) ||
     (X->type == TYPE_GROUP1) ||
     (X->type == TYPE_GROUP2) )
  {
    formula total = formula() ;
    formula batch = (X->batch["e"] + Y->batch["e"]) * formula("e") +
                    (X->batch["p"] + Y->batch["p"]) * formula("p") ;

    if(X->batch["b"] && Y->batch["b"]){
      string s = X->type + 5 ;
      s += "_mul" ;
      total = formula(s) ;
    }
    if(X->batch["b"] || Y->batch["b"]){
      batch += formula("b") ;
    }
    return new reduced(X->type, X->total + Y->total + total, 0, batch) ;
  }else{
    string s = X->type + 5 ;
    s += "_mul" ;
    return new reduced(X->type, X->total + Y->total + formula(s)) ;
  }
}

reduced * operation_DIV(reduced * X, reduced * Y, cell * L){
  if(!X) return NULL ;
  if(!Y) return NULL ;

  /* X/1 */

  if(
    (Y->type == TYPE_INTEGER) && (Y->literal) && (Y->value == 1)
  ){
    if(
        (X->type == TYPE_INTEGER)||
        (X->type == TYPE_GROUP)  ||
        (X->type == TYPE_GROUP0) ||
        (X->type == TYPE_GROUP1) ||
        (X->type == TYPE_GROUP2) ||
        (X->type == TYPE_DUPLEX) ||
        (X->type == TYPE_PROHIBITED)||
        (X->type == TYPE_TARGET)
    ) return X ;
  }

  /* 1/Y */

  if(
    (X->type == TYPE_INTEGER) && (X->literal) && (X->value == 1)
  ){
    if(
        (Y->type == TYPE_GROUP)  ||
        (Y->type == TYPE_GROUP0) ||
        (Y->type == TYPE_GROUP1) ||
        (Y->type == TYPE_GROUP2) ||
        (Y->type == TYPE_DUPLEX) ||
        (Y->type == TYPE_PROHIBITED)
    ) return Y ;
    if(Y->type == TYPE_TARGET){
      X->type = TYPE_TARGET ;
      X->batch = formula("b") ;
    }
    // if(Y->type == TYPE_INTEGER){}
  }

  /* <== list の場合再帰的に */
  if(!(((X->type == TYPE_INTEGER)||
        (X->type == TYPE_GROUP)  ||
        (X->type == TYPE_TARGET) ||
        (X->type == TYPE_GROUP0) ||
        (X->type == TYPE_GROUP1) ||
        (X->type == TYPE_GROUP2) ||
        (X->type == TYPE_DUPLEX) ||
        (X->type == TYPE_PROHIBITED)
       )&&(
        (X->type == Y->type)
  )))OPERATION_ERROR2() ;

  if((X->type == TYPE_TARGET) ||
     (X->type == TYPE_GROUP)  ||
     (X->type == TYPE_GROUP0) ||
     (X->type == TYPE_GROUP1) ||
     (X->type == TYPE_GROUP2) )
  {
    formula total = formula() ;
    formula batch = (X->batch["e"] + Y->batch["e"]) * formula("e") +
                    (X->batch["p"] + Y->batch["p"]) * formula("p") ;
    if(X->batch["b"] && Y->batch["b"]){
      string s = X->type + 5 ;
      s += "_div" ;
      total = formula(s) ;
    }
    if(X->batch["b"] || Y->batch["b"]){
      batch += formula("b") ;
    }
    return new reduced(X->type, X->total + Y->total + total, 0, batch) ;
  }else{
    string s = X->type + 5 ;
    s += "_div" ;
    return new reduced(X->type, X->total + Y->total + formula(s)) ;
  }
}

reduced * operation_POW(reduced * X, reduced * Y, cell * L){
  if(!X) return NULL ;
  if(!Y) return NULL ;

  /* X/1 */

  if(
    (Y->type == TYPE_INTEGER) && (Y->literal)
  ){
    if(
      ((Y->value == 1) || (Y->value == 0))
    ){
      if(
        (X->type == TYPE_INTEGER)||
        (X->type == TYPE_GROUP)  ||
        (X->type == TYPE_GROUP0) ||
        (X->type == TYPE_GROUP1) ||
        (X->type == TYPE_GROUP2) ||
        (X->type == TYPE_DUPLEX) ||
        (X->type == TYPE_PROHIBITED)||
        (X->type == TYPE_TARGET)
      ) return X ;
    }
    if(
      (Y->value < 0)
    ){
      reduced mY  = reduced (TYPE_INTEGER,formula(),0,formula(),1,-Y->value) ;
      reduced one = reduced (TYPE_INTEGER,formula(),0,formula(),1,1) ;
      return operation_DIV(&one, operation_POW(X,&mY,L), L) ;
    }
  }

  /* <== list の場合再帰的に */
  if(!(((X->type == TYPE_INTEGER)||
        (X->type == TYPE_GROUP)  ||
        (X->type == TYPE_TARGET) ||
        (X->type == TYPE_GROUP0) ||
        (X->type == TYPE_GROUP1) ||
        (X->type == TYPE_GROUP2) ||
        (X->type == TYPE_DUPLEX) ||
        (X->type == TYPE_PROHIBITED)
       )&&(
        (Y->type == TYPE_INTEGER)
  )))OPERATION_ERROR2() ;
  if((X->type == TYPE_TARGET) ||
     (X->type == TYPE_GROUP)  ||
     (X->type == TYPE_GROUP0) ||
     (X->type == TYPE_GROUP1) ||
     (X->type == TYPE_GROUP2) )
  {
    string s = Y->type + 5 ;
    s += "_mul" ;
    formula total = (X->batch["e"]) * formula(s) ;
    formula batch = (X->batch["e"] + X->batch["b"]) * formula("e") +
                    (X->batch["p"]                ) * formula("p") ;
//    cout << "operation_POW: " << X->type << endl ;
    return new reduced(X->type, X->total + Y->total + total, 0, batch) ;
  }else{
    string s = X->type + 5 ;
    s += "_pow" ;
    return new reduced(X->type, X->total + Y->total + formula(s)) ;
  }
}

reduced * operation_ASSIGN (reduced * X, reduced * Y, cell * L){
  if(!X) return NULL ;
  if(!Y) return NULL ;
  /* <== list の場合再帰的に */
  if(!is_left_value(X)){
    cout << "is_left_value(X)==0" << endl ;
    OPERATION_ERROR2() ;
  }

  /* TODO: 型チェック */
  /* TODO: 左辺値 */

  X->transfer_batch() ;
  Y->transfer_batch() ;


  string s = X->type + 5 ;
  s += "_assign" ;
  return new reduced(X->type, X->total + Y->total + formula(s),1) ;
}

reduced * operation_X_ASSIGN (
  reduced * (* operation_X)(reduced * ,reduced * ,cell *),
  reduced * X, reduced * Y, cell * L
){

  if(!X) return NULL ;
  if(!Y) return NULL ;
  /* <== list の場合再帰的に */
  if(!is_left_value(X)){
    cout << "is_left_value(X)==0" << endl ;
    OPERATION_ERROR2() ;
  }

  /* TODO: 型チェック */
  /* TODO: 左辺値 */

  reduced * Z = operation_X(X, Y, L) ;

  Z->transfer_batch() ;

  string s = Z->type + 5 ;
  s += "_assign" ;
  return new reduced(Z->type, Z->total + formula(s),1) ;
}

reduced * operation_ADDASSIGN (reduced * X, reduced * Y, cell * L){
  return operation_X_ASSIGN(operation_ADD, X, Y, L) ;
}

reduced * operation_SUBASSIGN (reduced * X, reduced * Y, cell * L){
  return operation_X_ASSIGN(operation_SUB, X, Y, L) ;
}

reduced * operation_MULASSIGN (reduced * X, reduced * Y, cell * L){
  return operation_X_ASSIGN(operation_MUL, X, Y, L) ;
}

reduced * operation_DIVASSIGN (reduced * X, reduced * Y, cell * L){
  return operation_X_ASSIGN(operation_DIV, X, Y, L) ;
}

reduced * operation_POWASSIGN (reduced * X, reduced * Y, cell * L){
  return operation_X_ASSIGN(operation_POW, X, Y, L) ;
}

reduced * operation_UMINUS (reduced * X, cell * L){
  if(!X) return NULL ;
  /* <== list の場合再帰的に */
  if(!(X->type == TYPE_INTEGER)) OPERATION_ERROR1() ;

  X->transfer_batch() ;

  if(X->literal){
    X->value = -X->value ;
    return X ;
  }else{
    string s = X->type + 5 ;
    s += "_uminus" ;
    return new reduced(X->type, X->total + formula(s)) ;
  }
}

reduced * operation_UPLUS  (reduced * X, cell * L){
  if(!X) return NULL ;
  /* <== list の場合再帰的に */
  if(!(X->type == TYPE_INTEGER)) OPERATION_ERROR1() ;

  X->transfer_batch() ;

  if(X->literal){
    return X ;
  }else{
    string s = X->type + 5 ;
    s += "_uplus" ;
    return new reduced(X->type, X->total + formula(s)) ;
  }
}

reduced * operation_NEW    (char * type, formula f = formula(), int left = 1, formula b = formula()){ return new reduced(type,f,left,b) ; }
reduced * operation_LITERAL(char * type, long long value = 0){ return new reduced(type,formula(),0,formula(),1,value) ; }

// ===================================================================================

#define define_is_(X) \
static int is_ ## X(declaration_stack_entry & dse){                         \
  YYLTYPE & loc = dummy_location ;                                          \
  static cell * label_3 = LIST(AST_IDENTIFIER,&loc,unique_symbol( #X ));    \
  static cell * X ## _  = LIST(AST_DECLARATION_SPECIFIERS,&loc,label_3);    \
  static cell * label_5 = LIST(AST_NULL,&loc);                              \
  static cell * label_6 = LIST(AST_NEW_IDENTIFIER,&loc,unique_symbol("x")); \
  static cell * x_      = LIST(AST_DECLARATOR,&loc,label_5,label_6);        \
  return same_declaration(X ## _, dse.declaration_specifiers) &&            \
         same_declaration(x_    , dse.init_declarator       ) ;             \
}                                                                           \
static int is_ ## X ## _function (declaration_stack_entry & dse){           \
  YYLTYPE & loc = dummy_location ;                                          \
  static cell * label_3 = LIST(AST_IDENTIFIER,&loc,unique_symbol( #X ));    \
  static cell * X ## _  = LIST(AST_DECLARATION_SPECIFIERS,&loc,label_3);    \
  static cell * label_5 = LIST(AST_NULL,&loc);                              \
  static cell * label_6 = LIST(AST_NEW_IDENTIFIER,&loc,unique_symbol("x")); \
  static cell * label_7 = LIST(AST_FUNC_DECLARATOR,&loc,label_6,LIST(AST_PARAMETER_LIST,&loc)); \
  static cell * x_      = LIST(AST_DECLARATOR,&loc,label_5,label_7);        \
  return same_declaration(X ## _, dse.declaration_specifiers) &&            \
         same_declaration_func(x_, dse.init_declarator      ) ;             \
}

define_is_(integer)    ; // static int is_integer() ;
define_is_(group)      ; // static int is_group() ;
define_is_(target)     ; // static int is_target() ;
define_is_(list)       ; // static int is_list() ;
define_is_(prohibited) ; // static int is_prohibited() ;

define_is_(group0)     ; // static int is_group0() ;
define_is_(group1)     ; // static int is_group1() ;
define_is_(group2)     ; // static int is_group2() ;
define_is_(duplex)     ; // static int is_duplex() ;

#define define_TYPE_(X) char * TYPE_ ## X = "TYPE_" # X

define_TYPE_(NULL)          ;          // TYPE_NULL
define_TYPE_(INTEGER)       ;          // TYPE_INTEGER
define_TYPE_(GROUP)         ;          // TYPE_GROUP
define_TYPE_(TARGET)        ;          // TYPE_TARGET
define_TYPE_(PROHIBITED)    ;          // TYPE_PROHIBITED

define_TYPE_(GROUP0)        ;          // TYPE_GROUP0
define_TYPE_(GROUP1)        ;          // TYPE_GROUP1
define_TYPE_(GROUP2)        ;          // TYPE_GROUP2
define_TYPE_(DUPLEX)        ;          // TYPE_DUPLEX
define_TYPE_(LITERAL)       ;          // TYPE_LITERAL
define_TYPE_(IDENTIFIER)    ;          // TYPE_IDENTIFIER
define_TYPE_(LIST_VARIABLE) ;          // TYPE_LIST_VARIABLE

define_TYPE_(LOGICAL_AND)   ;          // TYPE_LOGICAL_AND
define_TYPE_(LIST_LITERAL)  ;          // TYPE_LIST_LITERAL
define_TYPE_(OPTION)        ;          // TYPE_OPTION
define_TYPE_(IMPOSSIBLE)    ;          // TYPE_IMPOSSIBLE
define_TYPE_(POINTER)       ;          // TYPE_POINTER
define_TYPE_(REFERENCE)     ;          // TYPE_REFERENCE

// ===================================================================================

extern reduced * process_all(cell * L) ;

static reduced * process_LITERAL(cell * L, int flag) {
  const char * s = (char *)L->element(2) ;
  return operation_LITERAL(TYPE_LITERAL) ;
}

static reduced * process_STRING                  (cell * L){ return process_LITERAL(L,1); }
static reduced * process_CHAR                    (cell * L){ return process_LITERAL(L,2); }

static reduced * process_IDENTIFIER_RAW (cell * L, int push_back){
  char * name = (char *)L->element(2);
  int i = symbol_to_declaration_stack_entry(name) ;
  reduced * R = NULL ;
  if(i>=0){
    char * true_name = declaration_stack[i].true_name ;
    /* */ if(is_integer   (declaration_stack[i])){         R = operation_NEW(TYPE_INTEGER) ;
    }else if(is_group     (declaration_stack[i])){         R = operation_NEW(TYPE_GROUP ,formula(),1,formula("b"));
    }else if(is_prohibited(declaration_stack[i])){         R = operation_NEW(TYPE_PROHIBITED) ;
    }else if(is_group0    (declaration_stack[i])){         R = operation_NEW(TYPE_GROUP0,formula(),1,formula("b"));
    }else if(is_group1    (declaration_stack[i])){         R = operation_NEW(TYPE_GROUP1,formula(),1,formula("b"));
    }else if(is_group2    (declaration_stack[i])){         R = operation_NEW(TYPE_GROUP2,formula(),1,formula("b"));
    }else if(is_duplex    (declaration_stack[i])){         R = operation_NEW(TYPE_DUPLEX) ;
    }else if(is_target    (declaration_stack[i])){         R = operation_NEW(TYPE_TARGET,formula(),1,formula("b"));
    }else if(is_list      (declaration_stack[i])){         R = operation_NEW(TYPE_LIST_VARIABLE) ;
    }else if(declaration_stack[i].type & FUNC_DEFINITION){ R = operation_NEW(TYPE_LIST_VARIABLE) ;
    }else{                                                 R = operation_NEW(TYPE_LIST_VARIABLE) ;
    }
  }else{
    R = operation_NEW(TYPE_LIST_VARIABLE) ;
  }
  return R ;
}

static reduced * process_IDENTIFIER      (cell * L){ return process_IDENTIFIER_RAW(L, 0); }
static reduced * process_NEW_IDENTIFIER  (cell * L){ return process_IDENTIFIER_RAW(L, 1); }

static reduced * process_NUMBER (cell * L){
  process_LITERAL(L,0); 
  char * name = (char *)L->element(2);

/*
#ifndef USE_LONG_LONG
  reduced * R = (reduced *)LIST(TYPE_INTEGER , L->element(1), new INTEGER(integer_(name)) ) ;
#else
  reduced * R = (reduced *)LIST(TYPE_INTEGER , L->element(1), new INTEGER(strtoll(name,NULL,0)) ) ;
#endif
*/

  reduced * R = operation_LITERAL(TYPE_INTEGER, strtoll(name,NULL,0)) ;
  return R ;
}

static reduced * process_TYPE_QUALIFIER          (cell * L){ return process_LITERAL(L,0); }
static reduced * process_STORAGE_CLASS_SPECIFIER (cell * L){ return process_LITERAL(L,0); }

// ===================================================================================

static reduced * process_NULLARY(const char * const prefix){
  formula total = formula() ;
  return operation_NEW(TYPE_NULL, total, 0) ;
}

static reduced * process_NULLARY_OPERATION(
  cell * L,
  const char * const prefix,
  reduced * (*operation_X)(cell * L)
){
  return operation_X(L) ;
}

static reduced * process_BREAK     (cell * L){ return process_NULLARY("break ;"); }
static reduced * process_CONTINUE  (cell * L){ return process_NULLARY("continue ;"); }
static reduced * process_NULL      (cell * L){ return process_NULLARY(""); }

// ===================================================================================

static reduced * process_UNARY(
  cell * L,
  const char * const prefix,
  const char * const postfix
){
  cell * arg0 = L->element(2) ;
  reduced * left  = process_all(arg0) ;
  if(left){
    left->transfer_batch() ;
    return left ;
  }else{
    return operation_NEW(TYPE_NULL, formula(), 0) ;
  }
}

static reduced * process_UNARY_OPERATION(
  cell * L,
  const char * const prefix,
  const char * const postfix,
  reduced * (*operation_X)(reduced * X, cell * L),
  int check = 1
){
  cell * arg0 = L->element(2) ;

  reduced * A = process_all(arg0) ;
  reduced * R = operation_X(A, L) ;
  if(check) if(!R) semantic_error (L, 
    string("invalid operands: ")
    + prefix + " (" + type_of(A) + " " // + s0
    + ") "
    + postfix
  );
  return R ;
}

static reduced * process_ADDRESSOF         (cell * L){ return process_UNARY(L,"&",""); }

static int brackets_expand = 1 ;

static reduced * process_BRACKETS (cell * L) { return process_UNARY(L,"[","]"); }

static reduced * process_COMPOUND_STATEMENT(cell * L){
  declaration_frame_stack_push();
  reduced * R = process_UNARY(L,"{\n","}");
  declaration_frame_stack_pop();
  return R ;
}

static reduced * process_CURLY_BRACES (cell * L){ return process_UNARY(L,"{","}"); }
static reduced * process_DEFAULT      (cell * L){ return process_UNARY(L,"default: ",""); }
static reduced * process_DEREFERENCEOF(cell * L){ return process_UNARY(L,"*",""); }
static reduced * process_EXP_STAT     (cell * L){
  reduced * R = process_UNARY(L,"",";"); 
  R->transfer_batch() ;
  return R ;
}
static reduced * process_GOTO         (cell * L){ return process_UNARY(L,"goto ",";"); }

static reduced * process_INLINE       (cell * L){
  cell * arg0 = L->element(2) ;
  char * str  = (char *) arg0->element(2) ;
  cout << str << endl ;
  return process_UNARY(L,"inline ",";");
}

static reduced * process_NEG          (cell * L){ return process_UNARY(L,"~",""); }
static reduced * process_NOT          (cell * L){ return process_UNARY(L,"!",""); }

static reduced * process_PARENTHESIS  (cell * L){
  cell * arg0 = L->element(2) ;
  reduced * R = process_all(arg0) ;
  return R ;
}

static reduced * process_POSTPP       (cell * L){ return process_UNARY(L,"","++"); }
static reduced * process_POSTMM       (cell * L){ return process_UNARY(L,"","--"); }
static reduced * process_PREMM        (cell * L){ return process_UNARY(L,"--",""); }
static reduced * process_PREPP        (cell * L){ return process_UNARY(L,"++",""); }
static reduced * process_RETURN       (cell * L){ return process_UNARY(L,"return ",";"); }
static reduced * process_SIZEOF_TYPE  (cell * L){ return process_UNARY(L,"sizeof(",")"); }
static reduced * process_SIZEOF_EXP   (cell * L){ return process_UNARY(L,"sizeof(",")"); }
static reduced * process_UPLUS(cell * L){
  return process_UNARY_OPERATION(L,"+","",operation_UPLUS,0) ;
}
static reduced * process_UMINUS(cell * L){
  return process_UNARY_OPERATION(L,"-","",operation_UMINUS,0) ;
}

// ===================================================================================

static reduced * process_BINARY(
  cell * L,
  const char * const prefix,
  const char * const infix,
  const char * const postfix
){
  cell * arg0 = L->element(2) ;
  cell * arg1 = L->element(3) ;
  formula total = formula() ;
  reduced * left  = process_all(arg0) ;
  reduced * right = process_all(arg1) ;
  if(left) total += left->total ;
  if(right)total += right->total ;
  return operation_NEW(TYPE_NULL, total, 0) ;
}

enum process_XARY_OPERATION_flag {
  POP_BACK       =  1, // tweak declaration stack to initialize a variable
  PUSH_CRS       =  2, // push crs
  DOT_FLAG       =  4, // push crs and resolve .xxx identifier
  SUPPRESS       =  8, // suppress brackets expand
  DUMMYCOM       = 16, // dummy commit
  PUSH_REDUCE    = 32, // push reduce_history
  REFLECT_REDUCE = 64, // reflect reduce_history to reconstruct_stack
  WITHOUT_REDUCE = 32,
  WITH_REDUCE    = 32 + 64,
  SKIPERROR      = 128
} ;

static reduced * process_BINARY_OPERATION_CORE(
  cell * L,
  const char * const prefix,
  const char * const infix,
  const char * const postfix,
  reduced * (*operation_X)(reduced * X, reduced * Y, cell * L),
  int flag = 0
){
  int pop_back = flag & POP_BACK ;
  int push_crs = flag & PUSH_CRS ;
  int dot      = flag & DOT_FLAG ;
  int suppress = flag & SUPPRESS ;
  int dummycom = flag & DUMMYCOM ;
  int skiperror= flag & SKIPERROR;

  int save_brackets_expand = brackets_expand ;

  cell * arg0 = L->element(2) ;
  cell * arg1 = L->element(3) ;

  reduced * A = process_all(arg0) ;
  declaration_stack_entry dse ;
  if(pop_back){
    dse = declaration_stack.back() ;
    declaration_stack.pop_back() ;
  }
  if(dot) push_crs = 1 ;
//  if(push_crs) crs_stack.push_back({GENERATE_CRS_if_listvar(A,L), dummycom});
  if(suppress) brackets_expand = 0 ;
  reduced * B = process_all(arg1) ;
  if(suppress) brackets_expand = save_brackets_expand ;

  if(pop_back){
    declaration_stack.push_back(dse) ;
  }
  reduced * R = operation_X(A, B, L) ;
//  if(push_crs) crs_stack.pop_back();

  if(skiperror) return R ;
  else if(R)    return R ;
  else
  semantic_error (L, 
    string("invalid operands: ")
    + prefix + " (" + type_of(A) + " " // + s0
    + ") "
    + infix  + " (" + type_of(B) + " " // + s1
    + ") "
    + postfix
  );
  return NULL ;
}

static reduced * process_BINARY_OPERATION(
  cell * L,
  const char * const prefix,
  const char * const infix,
  const char * const postfix,
  reduced * (*operation_X)(reduced * X, reduced * Y, cell * L),
  int flag = 0
){
  int push_reduce    = flag & PUSH_REDUCE    ;
  int reflect_reduce = flag & REFLECT_REDUCE ;
  string t = "" ;

  reduced * R = process_BINARY_OPERATION_CORE(L, prefix, infix, postfix, operation_X, flag) ;
  return R ;
}

static reduced * process_TERNARY_OPERATION(
  cell * L,
  const char * const prefix,
  const char * const infix1,
  const char * const infix2,
  const char * const postfix,
  reduced * (*operation_X)(reduced * X, reduced * Y, reduced * Z, cell * L),
  int flag = 0
){
  int pop_back = flag & POP_BACK ; // unused
  int push_crs = flag & PUSH_CRS ;
  int dot      = flag & DOT_FLAG ; // unused
  int suppress = flag & SUPPRESS ;
  int dummycom = flag & DUMMYCOM ;

  int save_brackets_expand = brackets_expand ;

  cell * arg0 = L->element(2) ;
  cell * arg1 = L->element(3) ;
  cell * arg2 = L->element(4) ;

  reduced * A = process_all(arg0) ;

//  if(push_crs) crs_stack.push_back({GENERATE_CRS_if_listvar(A,L),dummycom});
  if(suppress) brackets_expand = 0 ;
  reduced * B = process_all(arg1) ;
  if(suppress) brackets_expand = save_brackets_expand ;

  reduced * C = process_all(arg2) ;

  reduced * R = operation_X(A, B, C, L) ;
//  if(push_crs) crs_stack.pop_back();

  if(R) return R ;
  else semantic_error (L,
    string("invalid operands: ")
    + prefix + " (" + type_of(A) + " " // + s0
    + ") "
    + infix1 + " (" + type_of(B) + " " // + s1
    + ") "
    + infix2 + " (" + type_of(C) + " " // + s2
    + ") "
    + postfix
  );
  return NULL ;
}

/*
AST_STAT_LIST
AST_DECLARATION
AST_DECLARATION_SPECIFIERS
AST_IDENTIFIER
AST_INIT_DECLARATOR_LIST
AST_INIT
AST_DECLARATOR
AST_NULL
AST_NEW_IDENTIFIER
AST_POW
AST_IDENTIFIER
AST_NUMBER
AST_DECLARATION
AST_DECLARATION_SPECIFIERS
AST_IDENTIFIER
AST_INIT_DECLARATOR_LIST
AST_INIT
AST_DECLARATOR
AST_NULL
AST_NEW_IDENTIFIER
AST_POW
AST_IDENTIFIER
AST_NUMBER
*/

static reduced * process_ABSTRACT_DECLARATOR(cell * L){ return process_BINARY(L,"","",""); }
static reduced * process_ARRAY_DECLARATOR   (cell * L){ return process_BINARY(L,"","[","]"); }

static reduced * process_ARRAY_REF          (cell * L){
  return process_BINARY_OPERATION(L,"","[","]",operation_ARRAY_REF); 
}

static reduced * process_ASSIGN(cell * L){
  return process_BINARY_OPERATION(L,"","=","",operation_ASSIGN, WITHOUT_REDUCE) ;
}

static reduced * process_INIT  (cell * L){
  return process_BINARY_OPERATION(L,"","=","",operation_ASSIGN, WITHOUT_REDUCE | POP_BACK) ;
//  return NULL ;
}

static reduced * process_DECLARATION (cell * L){
  cell * loc  = L   ->cdr ;
  cell * arg0 = loc ->cdr ;
  cell * arg1 = arg0->cdr ;
  cell * declaration_specifiers = (cell*)arg0->car ;

  int sp = declaration_stack_pointer();
  declaration_specifiers_stack_push(declaration_specifiers);
  reduced * R = process_BINARY(L,"","",";");
  declaration_specifiers_stack_pop();

  if( is_typedef(declaration_specifiers) ){
    int n = declaration_stack_pointer();
    for(int i=sp;i<n;i++) declaration_stack[i].type |= TYPEDEF_DECLARATOR ;
  }

  return R ;
}

static reduced * process_DECLARATOR     (cell * L){
  const char * const prefix  = "" ;
  const char * const infix   = "" ;
  const char * const postfix = "" ;
  cell * arg0 = L->element(2) ;
  cell * arg1 = L->element(3) ;
               process_all(arg0) ; 
  reduced * R = process_all(arg1) ;
  if(arg0->car == AST_NULL) return R ;
  else                      return NULL ; // pointer/reference
}

static reduced * process_FUNC_DECLARATOR(cell * L){
  const char * const prefix  = "" ;
  const char * const infix   = "(" ;
  const char * const postfix = ")" ;
  cell * arg0 = L->element(2) ;
  cell * arg1 = L->element(3) ;

  int sp = declaration_stack_pointer();

  declaration_frame_stack_push();
  process_all(arg0) ; 
  process_all(arg1) ; 
  int n = declaration_stack_pointer();
  for(int i=sp;i<n;i++){
    declaration_stack[sp-1].option.push_back(declaration_stack[i]) ;
  }
  declaration_frame_stack_pop();

  return NULL ;
}

static reduced * process_ADD(cell * L){ return process_BINARY_OPERATION(L,"","+","",operation_ADD) ; }
static reduced * process_SUB(cell * L){ return process_BINARY_OPERATION(L,"","-","",operation_SUB) ; }
static reduced * process_MUL(cell * L){ return process_BINARY_OPERATION(L,"","*","",operation_MUL) ; }
static reduced * process_DIV(cell * L){ return process_BINARY_OPERATION(L,"","/","",operation_DIV) ; }
static reduced * process_POW(cell * L){ return process_BINARY_OPERATION(L,"","^","",operation_POW) ; }
static reduced * process_EQ (cell * L){ return process_BINARY_OPERATION(L,"","==","",operation_EQ) ; }
static reduced * process_NE (cell * L){ return process_BINARY_OPERATION(L,"","!=","",operation_EQ) ; }
static reduced * process_DOT(cell * L){ return process_BINARY_OPERATION(L,"",".","",operation_DOT, DOT_FLAG) ; }
static reduced * process_LOGICALAND(cell * L){ return process_BINARY_OPERATION(L,"","&&","",operation_LOGICAL_AND) ; }
static reduced * process_ARROW          (cell * L){ return process_BINARY(L,"","->",""); }
// static reduced * process_ADDASSIGN      (cell * L){ return process_BINARY(L,"","+=",""); }
// static reduced * process_SUBASSIGN      (cell * L){ return process_BINARY(L,"","-=",""); }
// static reduced * process_MULASSIGN      (cell * L){ return process_BINARY(L,"","*=",""); }
// static reduced * process_DIVASSIGN      (cell * L){ return process_BINARY(L,"","/=",""); }

static reduced * process_ADDASSIGN(cell * L){
  return process_BINARY_OPERATION(L,"","=","",operation_ADDASSIGN, WITHOUT_REDUCE) ;
}
static reduced * process_SUBASSIGN(cell * L){
  return process_BINARY_OPERATION(L,"","=","",operation_SUBASSIGN, WITHOUT_REDUCE) ;
}
static reduced * process_MULASSIGN(cell * L){
  return process_BINARY_OPERATION(L,"","=","",operation_MULASSIGN, WITHOUT_REDUCE) ;
}
static reduced * process_DIVASSIGN(cell * L){
  return process_BINARY_OPERATION(L,"","=","",operation_DIVASSIGN, WITHOUT_REDUCE) ;
}

static reduced * process_MODASSIGN      (cell * L){ return process_BINARY(L,"","%=",""); }
static reduced * process_SHLASSIGN      (cell * L){ return process_BINARY(L,"","<<=",""); }
static reduced * process_SHRASSIGN      (cell * L){ return process_BINARY(L,"",">>=",""); }
static reduced * process_ANDASSIGN      (cell * L){ return process_BINARY(L,"","&=",""); }
static reduced * process_XORASSIGN      (cell * L){ return process_BINARY(L,"","(+)=",""); }
static reduced * process_ORASSIGN       (cell * L){ return process_BINARY(L,"","|=",""); }

static int is_group_variant(string & type){
  return (type == "group") || (type == "prohibited") ;
}

static reduced * process_PARAMETER_DECLARATION(cell * L){
  cell * declaration_specifiers = L->element(2) ;
  cell * declarator             = L->element(3) ;
  if(declarator->car == AST_DECLARATOR){
    declaration_specifiers_stack_push(declaration_specifiers);
    declaration_stack_push(declarator) ;
    declaration_specifiers_stack_pop();
  }
  cell * arg0 = L->element(2) ;
  cell * arg1 = L->element(3) ;
  process_all(arg0) ;
  process_all(arg1) ;
  return NULL ;
}

static reduced * process_POWASSIGN      (cell * L){ return process_BINARY(L,"","^=",""); }
static reduced * process_BITAND         (cell * L){ return process_BINARY(L,"","&",""); }
static reduced * process_BITOR          (cell * L){ return process_BINARY(L,"","|",""); }
static reduced * process_BITXOR         (cell * L){ return process_BINARY(L,"","(+)",""); }
static reduced * process_DO             (cell * L){ return process_BINARY(L,"do ","while(",");"); }

static cell * first_ast(cell * L, char * name){
  if(L->car == name) return L ;
  if(is_literal(L))  return NULL ;
  for(cell * arg = L->pointer(2); arg; arg=arg->cdr){
    cell * t = first_ast((cell *)(arg->car), name) ;
    if(t) return t ;
  }
  return NULL ;
}

cell * convert_return (cell * L, char * id, char * lb) {
  cell * R = NULL ;
  if(L){
    cell * loc = L  ->cdr ;
    cell * arg = loc->cdr ;
    if(is_literal(L)){
      R = LIST(L->car, loc->car, arg->car) ;
    }else if(L->car == AST_RETURN){
      cell * right = convert_return((cell*)arg->car, id, lb) ;
      cell * left  = LIST(AST_IDENTIFIER, loc->car, id) ;
      cell * label = LIST(AST_IDENTIFIER, loc->car, lb) ;
      cell * exp0  = LIST(AST_ASSIGN    , loc->car, left, right) ;
      cell * exp1  = LIST(AST_EXP_LIST  , loc->car, exp0) ;
      cell * stat0 = LIST(AST_EXP_STAT  , loc->car, exp1) ;
      cell * stat1 = LIST(AST_GOTO      , loc->car, label) ;
      cell * stat2 = LIST(AST_STAT_LIST , loc->car, stat0, stat1) ;
      R = LIST(AST_COMPOUND_STATEMENT   , loc->car, stat2) ;
    }else{
      R = LIST(L->car, loc->car) ;
      for(;arg;arg=arg->cdr) {
        R = INTEGRATE(R, convert_return((cell*)arg->car, id, lb)) ;
      }
    }
  }
  return R ;
}

cell * convert_return2 (cell * L, char * id){
  cell * R = NULL ;
  if(L){
    cell * loc = L  ->cdr ;
    cell * arg = loc->cdr ;
    if(is_literal(L)){
      R = LIST(L->car, loc->car, arg->car) ;
    }else if(L->car == AST_RETURN){
      cell * right = convert_return2((cell*)arg->car, id) ;
      cell * left  = LIST(AST_IDENTIFIER, loc->car, id) ;
      cell * exp0  = LIST(AST_ASSIGN    , loc->car, left, right) ;
      cell * exp1  = LIST(AST_EXP_LIST  , loc->car, exp0) ;
      cell * stat0 = LIST(AST_EXP_STAT  , loc->car, exp1) ;
      cell * left2 = LIST(AST_IDENTIFIER, loc->car, id) ;
      cell * exp2  = LIST(AST_EXP_LIST  , loc->car, left2) ;
      cell * stat1 = LIST(AST_RETURN    , loc->car, exp2) ;
      cell * stat2 = LIST(AST_STAT_LIST , loc->car, stat0, stat1) ;
      R = LIST(AST_COMPOUND_STATEMENT   , loc->car, stat2) ;
    }else{
      R = LIST(L->car, loc->car) ;
      for(;arg;arg=arg->cdr) {
        R = INTEGRATE(R, convert_return2((cell*)arg->car, id)) ;
      }
    }
  }
  return R ;
}

cell * replace_func(cell * L, char * id){
  cell * R = NULL ;
  if(L){
    cell * loc = L  ->cdr ;
    cell * arg = loc->cdr ;
    if(is_literal(L)){
      R = LIST(L->car, loc->car, arg->car) ;
    }else if(L->car == AST_FUNC_DECLARATOR){
      R = LIST(AST_NEW_IDENTIFIER, loc->car, id) ;
    }else{
      R = LIST(L->car, loc->car) ;
      for(;arg;arg=arg->cdr) {
        R = INTEGRATE(R, replace_func((cell*)arg->car, id)) ;
      }
    }
  }
  return R ;
}

cell * convert_to_macro (cell * L, int i){
#define dse (declaration_stack[i])
  cell * loc            = L->cdr;
  cell * name           = L->element(2) ;
  cell * expression_opt = L->element(3) ;
  char * base           = init_declarator_to_symbol(dse.init_declarator) ;
  char * id             = new_symbol(base) ;
  char * lb             = new_symbol(base) ;
  cell * R              = LIST(AST_STAT_LIST, loc->car) ;
  { // return variable
    cell * declaration_specifiers ;

    cell * dse_ds = copy_ast(dse.declaration_specifiers) ;
    remove_macro( dse_ds ) ;
    string s = ast2str( dse_ds ) ;

    if(s==""){
      cell * type_specifier =  LIST(AST_IDENTIFIER, loc->car, unique_symbol("list")) ;
      declaration_specifiers = LIST(AST_DECLARATION_SPECIFIERS,loc->car, type_specifier) ;
      free_ast(dse_ds) ;
    }else{
      declaration_specifiers = dse_ds ;
    }

    cell * init_declarator        = replace_func(dse.init_declarator, id) ;
    cell * init_declarator_list   = LIST(AST_INIT_DECLARATOR_LIST,loc->car, init_declarator) ;
    cell * declaration            = LIST(AST_DECLARATION,loc->car, declaration_specifiers, init_declarator_list) ;
    R                             = INTEGRATE(R,declaration) ;
  }
  { // arguments
    cell * init_declarator= dse.init_declarator ;
    cell * parameter_list = first_ast(init_declarator, AST_PARAMETER_LIST) ;
    cell * exp_list       = first_ast(expression_opt , AST_EXP_LIST      ) ;
    cell * var            = parameter_list->pointer(2) ;
    cell * val            = exp_list      ->pointer(2) ;
    cell * loc            = L->cdr ;
    for(;var && val;var=var->cdr,val=val->cdr){
      cell * parameter_declaration  = var->element(0) ;
      cell * declaration_specifiers = copy_ast(parameter_declaration->element(2)) ;
      cell * declarator             = copy_ast(parameter_declaration->element(3)) ;
      cell * initializer            = copy_ast(val->element(0)) ;
      cell * init_declarator        = LIST(AST_INIT, loc->car, declarator, initializer) ;
      cell * init_declarator_list   = LIST(AST_INIT_DECLARATOR_LIST,loc->car, init_declarator) ;
      cell * declaration            = LIST(AST_DECLARATION,loc->car, declaration_specifiers, init_declarator_list) ;
      R                             = INTEGRATE(R,declaration) ;
    }
    if(var) semantic_error(L, string("too few arguments to " ) + ast2str(init_declarator));
    if(val) semantic_error(L, string("too many arguments to ") + ast2str(init_declarator));
  }
  { // body
    cell * L = dse.compound_statement ;
    cell * loc = L->cdr ;
    cell * body  = convert_return (L, id, lb) ;
    cell * ret   = LIST(AST_IDENTIFIER, loc->car, id) ;
    cell * label = LIST(AST_IDENTIFIER, loc->car, lb) ;
    cell * exp0  = LIST(AST_EXP_LIST  , loc->car, ret) ;
    cell * stat0 = LIST(AST_EXP_STAT  , loc->car, exp0) ;
    cell * tail  = LIST(AST_LABEL, loc->car, label, stat0) ;
    R = INTEGRATE(R, body) ;
    R = INTEGRATE(R, tail) ;
  }
  R = LIST(AST_COMPOUND_STATEMENT, loc->car, R) ;
  R = LIST(AST_PARENTHESIS       , loc->car, R) ;
  return R ;
#undef dse
}

static reduced * macro_expand(cell * L, int i){
  cell * loc            = L->element(1) ;
  cell * name           = L->element(2) ;
  cell * expression_opt = L->element(3) ;
  cell * t = convert_to_macro (L, i) ;
  reduced * R = process_all(t) ;
  free_ast(t) ;
  return R ;
}

static reduced * func_expand(cell * L, int i){
#define dse (declaration_stack[i])
  cell * name           = L->element(2) ;
  cell * expression_opt = L->element(3) ;
  cell * init_declarator= dse.init_declarator ;

  formula total = formula() ;

  { // arguments
    cell * exp_list       = first_ast(expression_opt,AST_EXP_LIST) ;
    cell * val            = exp_list->pointer(2) ;
    cell * loc            = L->cdr ;
    int n = dse.option.size() ;
    int j ;
    for(j=0; val && (j<n) ; val=val->cdr, j++){
      declaration_stack.push_back(dse.option[j]) ;
      cell * var = LIST(AST_IDENTIFIER, loc->car, dse.option[j].symbol) ;
      reduced * left  = process_all(var) ;
      declaration_stack.pop_back() ;
      reduced * right = process_all((cell*)val->car) ;
      reduced * R = operation_ASSIGN(left, right, L) ;
      if(R){
        // R->transfer_batch() ; (already done by assign)
        total += R->total ;
      }
      /* <-- integrate */
    }
    if(j<n) semantic_error(L, string("too few arguments to " ) + ast2str(init_declarator));
    if(val) semantic_error(L, string("too many arguments to ") + ast2str(init_declarator));
  }

  string s = "call_" ;
  s += dse.symbol ;
  total += formula(s) ;

  reduced * R = NULL ;
  { // function body
         if(is_integer_function   (declaration_stack[i])){ R = operation_NEW(TYPE_INTEGER,total) ; }
    else if(is_group_function     (declaration_stack[i])){ R = operation_NEW(TYPE_GROUP ,total,0,formula("b")) ; }
    else if(is_prohibited_function(declaration_stack[i])){ R = operation_NEW(TYPE_PROHIBITED,total) ; }
    else if(is_group0_function    (declaration_stack[i])){ R = operation_NEW(TYPE_GROUP0,total,0,formula("b")) ; }
    else if(is_group1_function    (declaration_stack[i])){ R = operation_NEW(TYPE_GROUP1,total,0,formula("b")) ; }
    else if(is_group2_function    (declaration_stack[i])){ R = operation_NEW(TYPE_GROUP2,total,0,formula("b")) ; }
    else if(is_duplex_function    (declaration_stack[i])){ R = operation_NEW(TYPE_DUPLEX,total) ; }
    else if(is_target_function    (declaration_stack[i])){ R = operation_NEW(TYPE_TARGET,total,0,formula("b")) ; }
    else                                                 { R = operation_NEW(TYPE_NULL,total) ; }
  }
  return R ;
#undef dse
}

static int is_function_(cell * prog, int i){
#define dse (declaration_stack[i])
  cell * declaration_specifiers = first_ast(prog, AST_DECLARATION_SPECIFIERS) ;
  cell * init_declarator        = first_ast(prog, AST_DECLARATOR) ;
  return same_declaration(declaration_specifiers, dse.declaration_specifiers, 1) &&
         same_declaration(init_declarator       , dse.init_declarator       , 1) ;
#undef dse
}

static int is_pairing(int i){
  static cell * prog = yyparse("target e(group1 g1, group2 g2) ;") ;
  return is_function_(prog, i) ;
}

static int is_Repeat(int i){
  static cell * prog = yyparse("group Repeat(integer n, group g) ;") ;
  return is_function_(prog, i) ;
}

static int is_product(int i){
  static cell * prog = yyparse("target product(integer n, target t) ;") ;
  return is_function_(prog, i) ;
}

static int is_bitexp(int i){
  static cell * prog = yyparse("group bitexp(integer n, group g) ;") ;
  return is_function_(prog, i) ;
}

/*

// =================================================================================
// is_function_X
// =================================================================================


static int is_function_(char * declaration, int i){
#define dse (declaration_stack[i])
  cell * prog = yyparse(declaration) ;
  cell * declaration_specifiers = first_ast(prog, AST_DECLARATION_SPECIFIERS) ;
  cell * init_declarator        = first_ast(prog, AST_DECLARATOR) ;
  return same_declaration(declaration_specifiers, dse.declaration_specifiers, 1) &&
         same_declaration(init_declarator       , dse.init_declarator       , 1) ;
#undef dse
}

static int is_COM        (int i){ return is_function_("list COM(list proof) ;", i) ; }
static int is_PI         (int i){ return is_function_("list PI (list proof) ;", i) ; }
static int is_pairing    (int i){ return is_function_("target e(group g0, group g1) ;", i) ; }
static int is_option_crs (int i){ return is_function_("list option_crs() ;", i) ; }
static int is_GS_proofwi (int i){ return is_function_("list GS_proofwi(list crs, list predicate) ;", i) ; }
static int is_GS_proofzk (int i){ return is_function_("list GS_proofzk(list crs, list predicate) ;", i) ; }
static int is_GS_verifywi(int i){ return is_function_("list GS_verifywi(list crs, list predicate, list proof) ;", i) ; }
static int is_GS_verifyzk(int i){ return is_function_("list GS_verifyzk(list crs, list predicate, list proof) ;", i) ; }
static int is_GS_commit  (int i){ return is_function_("list GS_commit (list crs, group g) ;", i) ; }
static int is_GS_commit_g(int i){ return is_function_("list GS_commit_g(list crs) ;", i) ; }
static int is_GS_verify2 (int i){ return is_function_("list GS_verify2(list proof) ;", i) ; }
static int is_GS_setupB2 (int i){ return is_function_("list GS_setupB2(group g) ;", i) ; }
static int is_GS_setupH2 (int i){ return is_function_("list GS_setupH2(group g) ;", i) ; }
static int is_statistics (int i){ return is_function_("list statistics(list x) ;", i) ; }
static int is_setweight  (int i){ return is_function_("list setweight(integer n, list group_variables) ;", i) ; }
static int is_setpriority(int i){ return is_function_("list setpriority(integer n, list group_variables) ;", i) ; }
static int is_exclude    (int i){ return is_function_("list exclude(list X, list Y) ;", i) ; }

// =================================================================================
// do_function_X
// =================================================================================


static reduced * do_nullary_function_(cell * L, reduced* (*func)(cell * L)){
  cell * exp_list = first_ast(L, AST_EXP_LIST) ;
  cell * val      = exp_list->pointer(2) ;
  if(val) semantic_error(L, string("too many arguments: ") + ast2str(L));
  return process_NULLARY_OPERATION(L,"",func) ;
}
static reduced * do_unary_function_  (cell * L, reduced* (*func)(reduced * X, cell * L)){
  cell * exp_list = first_ast(L, AST_EXP_LIST) ;
  cell * val      = exp_list->pointer(2) ;
  if(!val    ) semantic_error(L, string("too few arguments: " ) + ast2str(L));
  if(val->cdr) semantic_error(L, string("too many arguments: ") + ast2str(L));
  return process_UNARY_OPERATION(L,"","",func,0) ;
}
static reduced* do_binary_function_ (
  cell* L,reduced* (*func)(reduced* X, reduced* Y, cell* L), int flag=0
){
  cell * exp_list = first_ast(L, AST_EXP_LIST) ;
  cell * val      = exp_list->pointer(2) ;
  if(!val         ) semantic_error(L, string("too few arguments: " ) + ast2str(L));
  if(!(val->cdr)  ) semantic_error(L, string("too few arguments: " ) + ast2str(L));
  if(val->cdr->cdr) semantic_error(L, string("too many arguments: ") + ast2str(L));
  return process_BINARY_OPERATION(L,"",",","",func,flag) ;
}

static reduced* do_binary_function_crs_push (cell* L,reduced* (*func)(reduced* X, reduced* Y, cell* L)){
  cell * exp_list = first_ast(L, AST_EXP_LIST) ;
  cell * val      = exp_list->pointer(2) ;
  if(!val         ) semantic_error(L, string("too few arguments: " ) + ast2str(L));
  if(!(val->cdr)  ) semantic_error(L, string("too few arguments: " ) + ast2str(L));
  if(val->cdr->cdr) semantic_error(L, string("too many arguments: ") + ast2str(L));
  return process_BINARY_OPERATION(L,"",",","",func, PUSH_CRS | SUPPRESS) ;
}

static reduced* do_ternary_function_crs_push (cell* L,reduced* (*func)(reduced* X, reduced* Y, reduced * Z, cell* L)){
  cell * exp_list = first_ast(L, AST_EXP_LIST) ;
  cell * val      = exp_list->pointer(2) ;
  if(!val              ) semantic_error(L, string("too few arguments: " ) + ast2str(L));
  if(!(val->cdr)       ) semantic_error(L, string("too few arguments: " ) + ast2str(L));
  if(!(val->cdr->cdr)  ) semantic_error(L, string("too few arguments: " ) + ast2str(L));
  if(val->cdr->cdr->cdr) semantic_error(L, string("too many arguments: ") + ast2str(L));
  return process_TERNARY_OPERATION(L,"",",",",","",func, PUSH_CRS | SUPPRESS | DUMMYCOM ) ;
}

static reduced* do_option_crs (cell* L){return do_nullary_function_(L,operation_OPTION_CRS);}
static reduced* do_COM        (cell* L){return do_unary_function_  (L,operation_COM);}
static reduced* do_PI         (cell* L){return do_unary_function_  (L,operation_PI);}
static reduced* do_pairing    (cell* L){return do_binary_function_ (L,operation_PAIRING   );}
static reduced* do_GS_commit  (cell* L){return do_binary_function_ (L,operation_GS_COMMIT );}
static reduced* do_GS_commit_g(cell* L){return do_unary_function_ (L,operation_GS_COMMIT_G);}
static reduced* do_GS_proofwi (cell* L){return do_binary_function_crs_push (L,operation_GS_PROOFWI);}
static reduced* do_GS_proofzk (cell* L){return do_binary_function_crs_push (L,operation_GS_PROOFZK);}
static reduced* do_GS_verifywi(cell* L){return do_ternary_function_crs_push(L,operation_GS_VERIFYWI);}
static reduced* do_GS_verifyzk(cell* L){return do_ternary_function_crs_push(L,operation_GS_VERIFYZK);}

static reduced* do_GS_setupB2 (cell* L){return do_unary_function_  (L,operation_GS_SETUPB2);}
static reduced* do_GS_setupH2 (cell* L){return do_unary_function_  (L,operation_GS_SETUPH2);}
static reduced* do_GS_verify2 (cell* L){return do_unary_function_  (L,operation_GS_VERIFY2);}
static reduced* do_statistics (cell* L){return do_unary_function_  (L,operation_STATISTICS);}
static reduced* do_setweight  (cell* L){return do_binary_function_ (L,operation_SETWEIGHT,SKIPERROR);}
static reduced* do_setpriority(cell* L){return do_binary_function_ (L,operation_SETPRIORITY,SKIPERROR);}
static reduced* do_exclude    (cell* L){return do_binary_function_ (L,operation_EXCLUDE,SKIPERROR);}

// =================================================================================
// function_X
// =================================================================================

static reduced * do_function_(cell * L, reduced* (*func)(cell * L)){
  cell   * arg0 = L->element(2) ;
  cell   * arg1 = L->pointer(3) ? L->element(3) : L ;
  process_all(arg0) ;
  reduced * R    = func(arg1) ;
  return R ;
}

static reduced * option_crs(cell * L){ return do_function_(L, do_option_crs) ; }
static reduced * pairing   (cell * L){ return do_function_(L, do_pairing   ) ; }
static reduced * COM       (cell * L){ return do_function_(L, do_COM       ) ; }
static reduced * PI        (cell * L){ return do_function_(L, do_PI        ) ; }
static reduced * GS_commit (cell * L){ return do_function_(L, do_GS_commit ) ; }
static reduced * GS_commit_g(cell * L){ return do_function_(L, do_GS_commit_g ) ; }

static reduced * do_function_with_expand(cell * L, reduced* (*func)(cell * L)){
  reduced * R  = do_function_(L, func) ; 
  return R ;
}
static reduced * GS_proofwi (cell * L){ return do_function_with_expand(L, do_GS_proofwi ) ; }
static reduced * GS_proofzk (cell * L){ return do_function_with_expand(L, do_GS_proofzk ) ; }
static reduced * GS_verifywi(cell * L){ return do_function_with_expand(L, do_GS_verifywi) ; }
static reduced * GS_verifyzk(cell * L){ return do_function_with_expand(L, do_GS_verifyzk) ; }

// ------------------------------------------------------------------------------------------
static reduced * GS_setupB2(cell * L){ return do_function_(L, do_GS_setupB2) ; }
static reduced * GS_setupH2(cell * L){ return do_function_(L, do_GS_setupH2) ; }
static reduced * GS_verify2(cell * L){ return do_function_with_expand(L, do_GS_verify2) ; }
static reduced * statistics(cell * L){ return do_function_with_expand(L, do_statistics) ; }
static reduced * setweight (cell * L){ return do_function_with_expand(L, do_setweight ) ; }
static reduced * setpriority(cell * L){ return do_function_with_expand(L, do_setpriority) ; }
static reduced * exclude   (cell * L){ return do_function_with_expand(L, do_exclude) ; }

// =================================================================================
//  FUNC_CALL
// =================================================================================
*/

static reduced * pairing(cell * L, int i){
#define dse (declaration_stack[i])
  cell * name           = L->element(2) ;
  cell * expression_opt = L->element(3) ;
  cell * init_declarator= dse.init_declarator ;

  formula total = formula() ;

  { // arguments
    cell * exp_list       = first_ast(expression_opt,AST_EXP_LIST) ;
    cell * val            = exp_list->pointer(2) ;
    cell * loc            = L->cdr ;
    int n = dse.option.size() ;
    int j ;
    for(j=0; val && (j<n) ; val=val->cdr, j++){
      declaration_stack.push_back(dse.option[j]) ;
      cell * var = LIST(AST_IDENTIFIER, loc->car, dse.option[j].symbol) ;
      reduced * left  = process_all(var) ;
      declaration_stack.pop_back() ;
      reduced * right = process_all((cell*)val->car) ;
      reduced * R = operation_ASSIGN(left, right, L) ;
      if(R){
        // R->transfer_batch() ; (already done by assign)
        total += R->total ;
      }
      /* <-- integrate */
    }
    if(j<n) semantic_error(L, string("too few arguments to " ) + ast2str(init_declarator));
    if(val) semantic_error(L, string("too many arguments to ") + ast2str(init_declarator));
  }
  reduced * R = operation_NEW(TYPE_TARGET,total,0,formula("b") + formula("p")) ;
//  reduced * R = operation_NEW(TYPE_TARGET,total,0,formula("p")) ;
  return R ;
#undef dse
}

static reduced * Repeat(cell * L, int i){
#define dse (declaration_stack[i])
  cell * name           = L->element(2) ;
  cell * expression_opt = L->element(3) ;
  cell * init_declarator= dse.init_declarator ;
  reduced * R ;

  formula total = formula() ;

  { // arguments
    cell * exp_list       = first_ast(expression_opt,AST_EXP_LIST) ;
    cell * val            = exp_list->pointer(2) ;
    cell * loc            = L->cdr ;
    int n = dse.option.size() ;
    int j ;
    for(j=0; val && (j<n) ; val=val->cdr, j++){
      declaration_stack.push_back(dse.option[j]) ;
      cell * var = LIST(AST_IDENTIFIER, loc->car, dse.option[j].symbol) ;
      reduced * left  = process_all(var) ;
      declaration_stack.pop_back() ;
      reduced * right = process_all((cell*)val->car) ;
      R = operation_ASSIGN(left, right, L) ;
      if(R){
        // R->transfer_batch() ; (already done by assign)
        total += R->total ;
      }
      /* <-- integrate */
    }
    if(j<n) semantic_error(L, string("too few arguments to " ) + ast2str(init_declarator));
    if(val) semantic_error(L, string("too many arguments to ") + ast2str(init_declarator));
  }

  {
    // ast2str
    cell * exp_list       = first_ast(expression_opt,AST_EXP_LIST) ;
    cell * arg1           = exp_list->element(2) ;
    // cell * arg2           = exp_list->element(3) ;
    // cell * arg3           = exp_list->element(4) ;

    string s  = "call_Repeat(" ;
           s += ast2str(arg1) ;
           s += "," ;
           s += (string)total ;
           s += ")" ;
    total = formula(s) ;
    R = operation_NEW(R->type, total) ;
    return R ;
  }

#undef dse
}

static reduced * product(cell * L, int i){
#define dse (declaration_stack[i])
//  cell * name           = L->element(2) ;
  cell * expression_opt = L->element(3) ;

  { // arguments
    cell * init_declarator= dse.init_declarator ;
    cell * exp_list       = first_ast(expression_opt,AST_EXP_LIST) ;
    cell * val            = exp_list->pointer(2) ;
    cell * loc            = L->cdr ;
    int n = dse.option.size() ;
    int j ;
    for(j=0; val && (j<n) ; val=val->cdr, j++){
      declaration_stack.push_back(dse.option[j]) ;
      cell * var = LIST(AST_IDENTIFIER, loc->car, dse.option[j].symbol) ;
    }
    if(j<n) semantic_error(L, string("too few arguments to " ) + ast2str(init_declarator));
    if(val) semantic_error(L, string("too many arguments to ") + ast2str(init_declarator));
  }

  {
    cell    * exp_list = first_ast(expression_opt,AST_EXP_LIST) ;
    cell    * arg1     = exp_list->element(2) ;
    cell    * arg2     = exp_list->element(3) ;
    string    a1       = ast2str(arg1) ;
    reduced * right    = process_all(arg2) ;
    formula total = formula() ;
    formula batch = formula() ;

    char    * e ;
    integer_  n ;
    INTEGER   N ;

    errno = 0 ;
    n = strtol(a1.c_str(), &e, 0) ;
    if (errno != ERANGE) {
      if (*e == '\0') N = INTEGER(n) ;
      else            N = INTEGER(string("(") + a1 + ")") ;
    }else if (n == LONG_MAX){
      semantic_error(L, string("too large number ") + a1) ;
    }else if (n == LONG_MIN){
      semantic_error(L, string("too small number ") + a1) ;
    }

    total = N * right->total ;
    batch = (N * (right->batch["e"])) * formula("e") +
            (N * (right->batch["p"])) * formula("p") ;

    if(N){
      if(right->batch["b"]){
        string s = right->type + 5 ;
        s += "_mul" ;
        total += (N - INTEGER(1)) * formula(s) ;
        batch += formula("b") ;
      }
    }

    return operation_NEW(right->type,total,0,batch) ;
  }

#undef dse
}

static reduced * bitexp(cell * L, int i){
#define dse (declaration_stack[i])
//  cell * name           = L->element(2) ;
  cell * expression_opt = L->element(3) ;

  { // arguments
    cell * init_declarator= dse.init_declarator ;
    cell * exp_list       = first_ast(expression_opt,AST_EXP_LIST) ;
    cell * val            = exp_list->pointer(2) ;
    cell * loc            = L->cdr ;
    int n = dse.option.size() ;
    int j ;
    for(j=0; val && (j<n) ; val=val->cdr, j++){
      declaration_stack.push_back(dse.option[j]) ;
      cell * var = LIST(AST_IDENTIFIER, loc->car, dse.option[j].symbol) ;
    }
    if(j<n) semantic_error(L, string("too few arguments to " ) + ast2str(init_declarator));
    if(val) semantic_error(L, string("too many arguments to ") + ast2str(init_declarator));
  }

  {
    cell    * exp_list = first_ast(expression_opt,AST_EXP_LIST) ;
    cell    * arg1     = exp_list->element(2) ;
    cell    * arg2     = exp_list->element(3) ;
    string    a1       = ast2str(arg1) ;
    reduced * right    = process_all(arg2) ;
    right->transfer_bitexp(a1) ;
    return right ;
  }

#undef dse
}

static reduced * process_FUNC_CALL(cell * L){
  string s = ast2str(L->element(2));
  int i = symbol_to_declaration_stack_entry((char*)s.c_str()) ;
  reduced * R = NULL ;
  if(i>=0){
    if(declaration_stack[i].type & MACRO_DEFINITION){
      R = macro_expand(L, i) ;
    }else if(declaration_stack[i].type & FUNC_DEFINITION){
      R = func_expand(L, i) ;
    }else if(declaration_stack[i].type & FUNC_DECLARATOR){
           if(is_pairing(i)) R = pairing    (L, i) ;
      else if(is_Repeat (i)) R = Repeat     (L, i) ;
      else if(is_product(i)) R = product    (L, i) ;
      else if(is_bitexp (i)) R = bitexp     (L, i) ;
      else                   R = func_expand(L, i) ;
/*
      else if(is_COM        (i)) R = COM        (L) ;
      else if(is_PI         (i)) R = PI         (L) ;
      else if(is_GS_commit  (i)) R = GS_commit  (L) ;
      else if(is_GS_commit_g(i)) R = GS_commit_g(L) ;
      else if(is_option_crs (i)) R = option_crs (L) ;
      else if(is_GS_proofwi (i)) R = GS_proofwi (L) ;
      else if(is_GS_proofzk (i)) R = GS_proofzk (L) ;
      else if(is_GS_verifywi(i)) R = GS_verifywi(L) ;
      else if(is_GS_verifyzk(i)) R = GS_verifyzk(L) ;
      else if(is_GS_setupB2 (i)) R = GS_setupB2 (L) ; // obsolete
      else if(is_GS_setupH2 (i)) R = GS_setupH2 (L) ; // obsolete
      else if(is_GS_verify2 (i)) R = GS_verify2 (L) ; // obsolete
      else if(is_statistics (i)) R = statistics (L) ;
      else if(is_setweight  (i)) R = setweight  (L) ;
      else if(is_setpriority(i)) R = setpriority(L) ;
      else if(is_exclude    (i)) R = exclude    (L) ;
*/
    }else{
      semantic_error(L, s + " is not a function") ;
    }
  }else{
    R = process_BINARY(L,"","(",")");
  }
  return R ;
}

static reduced * process_GE          (cell * L){ return process_BINARY(L,"",">=",""); }
static reduced * process_GT          (cell * L){ return process_BINARY(L,"",">",""); }
static reduced * process_IF          (cell * L){
//  return process_BINARY(L,"if","","");

  const cell * const loc  = L   ->cdr ;
  const cell * const arg0 = loc ->cdr ;
  const cell * const arg1 = arg0->cdr ;

  formula total = formula() ;
  reduced * left   = process_all((cell*)arg0->car) ;
  reduced * right  = process_all((cell*)arg1->car) ;

  if(left   )total += left->total ;
  if(right  )total += right->total ;

  {
    // ast2str
    extern string replace_all(
      const string & a, // with
      const string & b, // in
      const string & c
    );

    string s  = "if_statement(\"" ;
           s += replace_all("\"","\\\"",ast2str((cell*)arg0->car)) ;
           s += "\"," ;
           s += (string)total ;
           s += "," ;
           s += "0" ;
           s += ")" ;

    total = formula(s) ;

    return operation_NEW(TYPE_NULL, total) ;

  }
}

static reduced * process_LE          (cell * L){ return process_BINARY(L,"","<=",""); }
static reduced * process_LOGICALOR   (cell * L){ return process_BINARY(L,"","||",""); }
static reduced * process_LT          (cell * L){ return process_BINARY(L,"","<",""); }
static reduced * process_MOD         (cell * L){ return process_BINARY(L,"","%",""); }

// static reduced * process_NE          (cell * L){ return process_BINARY(L,"","!=",""); }

static reduced * process_POINTER     (cell * L){ return process_BINARY(L,"*","",""); }
static reduced * process_REFERENCE   (cell * L){ return process_BINARY(L,"&","",""); }
static reduced * process_SHL         (cell * L){ return process_BINARY(L,"","<<",""); }
static reduced * process_SHR         (cell * L){ return process_BINARY(L,"",">>",""); }
static reduced * process_WHILE       (cell * L){ return process_BINARY(L,"while(",")",""); }
static reduced * process_TYPE_NAME   (cell * L){ return process_BINARY(L,"","",""); }
static reduced * process_CAST        (cell * L){ return process_BINARY(L,"(",")",""); }

static reduced * process_LABEL       (cell * L){
  const char * const prefix  = ""   ;
  const char * const infix   = ": " ;
  const char * const postfix = ""   ;
  cell * arg0 = L->element(2) ;
  cell * arg1 = L->element(3) ;
  process_all(arg0) ;
  reduced * R = process_all(arg1) ;
  return R ;
}

static reduced * process_LABELED_INIT(cell * L){ return process_BINARY(L,"",": ",""); }
static reduced * process_SWITCH      (cell * L){ return process_BINARY(L,"switch","",""); }
static reduced * process_CASE        (cell * L){ return process_BINARY(L,"case ",": ",""); }
static reduced * process_BIT_FIELD   (cell * L){ return process_BINARY(L,"",": ",""); }

static reduced * process_SU(
  cell * L,
  const char * const prefix,
  const char * const infix,
  const char * const postfix,
  int                declarator_type
){
  cell * loc  = L   ->cdr ;
  cell * arg0 = loc ->cdr ;
  cell * arg1 = arg0->cdr ;

  cell * struct_                 = (cell*)arg0->car ;
  cell * struct_declaration_list = (cell*)arg1->car ;

  cell * declaration_specifiers  = LIST(AST_DECLARATION_SPECIFIERS,&dummy_location,L) ;

  string s = prefix ;

  int sp = declaration_stack_pointer();

  declaration_specifiers_stack_push(declaration_specifiers);
  declaration_stack_push(struct_);
  declaration_stack[sp].type = declarator_type ;
  declaration_specifiers_stack_pop ();

  process_all(struct_) ;
  declaration_frame_stack_push() ;
  process_all(struct_declaration_list) ;
  int n = declaration_stack_pointer();
  for(int i=sp+1;i<n;i++) declaration_stack[sp].option.push_back(declaration_stack[i]);
  declaration_frame_stack_pop() ;
  return NULL ;
}

static reduced * process_STRUCT(cell * L){
  return process_SU(L,"struct ","{\n","}",STRUCT_DECLARATOR); }
static reduced * process_UNION (cell * L){
  return process_SU(L,"union ","{\n","}",UNION_DECLARATOR); }

static reduced * process_E(
  cell * L,
  const char * const prefix,
  const char * const infix,
  const char * const postfix
){
  cell * loc  = L   ->cdr ;
  cell * arg0 = loc ->cdr ;
  cell * arg1 = arg0->cdr ;

  cell * enum_           = (cell*)arg0->car ;
  cell * enumerator_list = (cell*)arg1->car ;

  cell * declaration_specifiers = LIST(AST_DECLARATION_SPECIFIERS,&dummy_location,L);

  int sp = declaration_stack_pointer();

  declaration_specifiers_stack_push(declaration_specifiers);
  declaration_stack_push(enum_);
  declaration_stack[sp].type = ENUM_DECLARATOR ;
  declaration_specifiers_stack_pop ();

  process_all(enum_) ;
  declaration_specifiers_stack_push(declaration_specifiers) ;

  process_all(enumerator_list) ;
  int n = declaration_stack_pointer();
  for(int i=sp+1;i<n;i++) declaration_stack[i].type = ENUM_CONST ;
  declaration_specifiers_stack_pop();
  return NULL ;
}

static reduced * process_ENUM (cell * L){ return process_E(L,"enum ","{","}"); }

// ===================================================================================

static reduced * process_TERNARY(
  const cell * const L,
  const char * const prefix,
  const char * const infix1,
  const char * const infix2,
  const char * const postfix
){
  const cell * const loc  = L   ->cdr ;
  const cell * const arg0 = loc ->cdr ;
  const cell * const arg1 = arg0->cdr ;
  const cell * const arg2 = arg1->cdr ;

  formula total = formula() ;
  reduced * left   = process_all((cell*)arg0->car) ;
  reduced * middle = process_all((cell*)arg1->car) ;
  reduced * right  = process_all((cell*)arg2->car) ;
  if(left  )total += left->total ;
  if(middle)total += middle->total ;
  if(right )total += right->total ;
  return operation_NEW(TYPE_NULL, total, 0) ;
}

static reduced * process_IF_ELSE(cell * L) {
// return process_TERNARY(L,"if","","else","");

  const cell * const loc  = L   ->cdr ;
  const cell * const arg0 = loc ->cdr ;
  const cell * const arg1 = arg0->cdr ;
  const cell * const arg2 = arg1->cdr ;

  formula total = formula() ;
  formula right_total = formula() ;
  reduced * left   = process_all((cell*)arg0->car) ;
  reduced * middle = process_all((cell*)arg1->car) ;
  reduced * right  = process_all((cell*)arg2->car) ;

  if(left   )total += left->total ;
  if(middle )total += middle->total ;
  if(right  )right_total += right->total ;

  {
    // ast2str
    extern string replace_all(
      const string & a, // with
      const string & b, // in
      const string & c
    );

    string s  = "if_statement(\"" ;
           s += replace_all("\"","\\\"",ast2str((cell*)arg0->car)) ;
           s += "\"," ;
           s += (string)total ;
           s += "," ;
           s += (string)right_total ;
           s += ")" ;

    total = formula(s) ;

    return operation_NEW(TYPE_NULL, total) ;

  }
}

static reduced * process_TERNARY(cell * L){ return process_TERNARY(L,"","?",":",""); }

// ===================================================================================

/*
static reduced * process_QUATERNARY(
  const cell * const L,
  const char * const prefix,
  const char * const infix1,
  const char * const infix2,
  const char * const infix3,
  const char * const postfix
){
  const cell * const loc  = L   ->cdr ;
  const cell * const arg0 = loc ->cdr ;
  const cell * const arg1 = arg0->cdr ;
  const cell * const arg2 = arg1->cdr ;
  const cell * const arg3 = arg2->cdr ;

  formula total = formula() ;
  reduced * left   = process_all((cell*)arg0->car) ;
  reduced * middle = process_all((cell*)arg1->car) ;
  reduced * middle2= process_all((cell*)arg2->car) ;
  reduced * right  = process_all((cell*)arg3->car) ;

  if(left   )total += left->total ;
  if(middle )total += middle->total ;
  if(middle2)total += middle2->total ;
  if(right  )total += right->total ;
  return operation_NEW(TYPE_NULL, total, 0) ;
}
*/

static reduced * process_FOR     (cell * L){
//  return process_QUATERNARY(L,"for(",";",";",")",""); 

  const cell * const loc  = L   ->cdr ;
  const cell * const arg0 = loc ->cdr ;
  const cell * const arg1 = arg0->cdr ;
  const cell * const arg2 = arg1->cdr ;
  const cell * const arg3 = arg2->cdr ;

  formula total = formula() ;
  reduced * left   = process_all((cell*)arg0->car) ;
  reduced * middle = process_all((cell*)arg1->car) ;
  reduced * middle2= process_all((cell*)arg2->car) ;
  reduced * right  = process_all((cell*)arg3->car) ;

  if(left   )total += left->total ;
  if(middle )total += middle->total ;
  if(middle2)total += middle2->total ;
  if(right  )total += right->total ;

  {
    // ast2str
    extern string replace_all(
      const string & a, // with
      const string & b, // in
      const string & c
    );

    string s  = "for_statement(\"" ;
           s += replace_all("\"","\\\"",ast2str((cell*)arg0->car)) ;
           s += "\",\"" ;
           s += replace_all("\"","\\\"",ast2str((cell*)arg1->car)) ;
           s += "\",\"" ;
           s += replace_all("\"","\\\"",ast2str((cell*)arg2->car)) ;
           s += "\"," ;
           s += (string)total ;
           s += ")" ;

    total = formula(s) ;

    return operation_NEW(TYPE_NULL, total) ;

  }
}

std::vector<char *> function_stack ;

static reduced * process_FUNC_DEF(cell * L){
  cell * loc  = L   ->cdr ;
  cell * arg0 = loc ->cdr ;
  cell * arg1 = arg0->cdr ;
  cell * arg2 = arg1->cdr ;
  cell * arg3 = arg2->cdr ;

  cell * declaration_specifiers = (cell*)arg0->car ;
  cell * declarator             = (cell*)arg1->car ;
  cell * declaration_list       = (cell*)arg2->car ; /* K&R */ 
  cell * compound_statement     = (cell*)arg3->car ;

  int sp = declaration_stack_pointer() ;
  declaration_specifiers_stack_push(declaration_specifiers) ;
  declaration_stack_push(declarator) ;
#define dse (declaration_stack[sp])
  for(int i = dse.chain; i != -1; i = declaration_stack[i].chain){
    if(declaration_stack[i].compound_statement){
      semantic_error(L, string("duplicate function definition: ") + dse.symbol + "()" ) ;
    }
  }

  dse.declaration_list   = declaration_list ;
  dse.compound_statement = compound_statement ;

  declaration_frame_stack_push();




  reduced * T0 = process_all(declaration_specifiers) ;
  reduced * T1 = process_all(declarator            ) ;
  for(auto i : declaration_stack[sp].option) declaration_stack.push_back(i) ;
  reduced * T2 = process_all(declaration_list      ) ;

  char * return_symbol = unique_symbol((char*)(string(dse.true_name) + "$return").c_str()) ;

  cell * body  = convert_return2 ((cell*)arg3->car, return_symbol) ;
  reduced * T3 = process_all(body) ;
  free_ast(body) ;

  declaration_frame_stack_pop();

  int is_macro_ = is_macro(declaration_specifiers) ;
  {
//   declaration_stack_entry & dse = declaration_stack.back() ; // <== BUG CODE
    if(is_macro_)dse.type |= MACRO_DEFINITION ;
    else         dse.type |= FUNC_DEFINITION  ;
  }

  formula total = formula();
  if(T0)total += T0->total ;
  if(T1)total += T1->total ;
  if(T2)total += T2->total ;
  if(T3)total += T3->total ;
  cout << "call_" << dse.symbol << " = " << total << " ; "
//       << "print(\"" << dse.symbol << " = \", call_" << dse.symbol << ") ;"
       << endl ;

  function_stack.push_back(dse.symbol) ;

#undef dse
  declaration_specifiers_stack_pop();

  return operation_NEW(TYPE_NULL) ;
}

// ===================================================================================

// ===================================================================================

static reduced * process_N_ARY(cell * L){
  cell    * loc   = L  ->cdr ;
  cell    * arg   = loc->cdr ;
  formula   total = formula() ;
  for(;arg;arg=arg->cdr){
    reduced * R = process_all((cell*)arg->car) ;
    if(R) total += R->total ;
  }
  return operation_NEW(TYPE_NULL, total, 0) ;
}

static reduced * process_LIST                   (cell * L) { return process_N_ARY(L); }
static reduced * process_PROG                   (cell * L) { return process_N_ARY(L); }
static reduced * process_DECLARATION_LIST       (cell * L) { return process_N_ARY(L); }
static reduced * process_STAT_LIST              (cell * L) { return process_N_ARY(L); }
static reduced * process_DECLARATION_SPECIFIERS (cell * L) { return process_N_ARY(L); }
static reduced * process_TYPE_QUALIFIER_LIST (cell * L) {
  return process_DECLARATION_SPECIFIERS (L);
}
static reduced * process_SPECIFIER_QUALIFIER_LIST(cell * L) {
  return process_DECLARATION_SPECIFIERS (L);
}

static reduced * process_EXP_LIST(cell * L) {
  cell * loc = L  ->cdr ;
  cell * arg = loc->cdr ;
  int single = 0 ;
  if(arg) if(! (arg->cdr)) single = 1 ;
  reduced * R = NULL ;
  if(single){
    R = process_all((cell*)arg->car) ;
    if(R)R->transfer_batch() ;
  }else{
    formula total ;
    int     left  = 1 ;
    for(;arg;arg=arg->cdr){
      reduced * T = process_all((cell*)arg->car) ;
      if(T){
        T->transfer_batch() ;
        total += T->total ;
	left  &= T->left  ;
      }
    }
    R = new reduced(TYPE_LIST_LITERAL, total, left) ;
  }
  return R ;
}

static reduced * process_PARAMETER_LIST       (cell * L) { return process_EXP_LIST(L); }
static reduced * process_IDENTIFIER_LIST      (cell * L) { return process_EXP_LIST(L); }
static reduced * process_INITIALIZER_LIST     (cell * L) { return process_EXP_LIST(L); }

/*
static reduced * process_INIT_DECLARATOR_LIST (cell * L) {
  cell * loc = L  ->cdr ;
  cell * arg = loc->cdr ;
  for(;arg;arg=arg->cdr){
    declaration_stack_push((cell*)arg->car) ;
    process_all((cell*)arg->car) ;
  }
  return NULL ;
}
*/

static reduced * process_INIT_DECLARATOR_LIST (cell * L) {
  cell    * loc   = L  ->cdr ;
  cell    * arg   = loc->cdr ;
  formula   total = formula() ;
  for(;arg;arg=arg->cdr){
    declaration_stack_push((cell*)arg->car) ;
    reduced * R = process_all((cell*)arg->car) ;
    if(R) total += R->total ;
  }
  return operation_NEW(TYPE_NULL, total, 0) ;
}

static unordered_map<char const * const, reduced * (*)(cell *)> table = {
  {AST_ABSTRACT_DECLARATOR,       process_ABSTRACT_DECLARATOR},
  {AST_ADD,                       process_ADD},
  {AST_ADDASSIGN,                 process_ADDASSIGN},
  {AST_ADDRESSOF,                 process_ADDRESSOF},
  {AST_ANDASSIGN,                 process_ANDASSIGN},
  {AST_ARRAY_DECLARATOR,          process_ARRAY_DECLARATOR},
  {AST_ARRAY_REF,                 process_ARRAY_REF},
  {AST_ARROW,                     process_ARROW},
  {AST_ASM,                       process_LIST},
  {AST_ASM_INIT,                  process_LIST},
  {AST_ASM_IN_OUT,                process_LIST},
  {AST_ASSIGN,                    process_ASSIGN},
  {AST_BITAND,                    process_BITAND},
  {AST_BIT_FIELD,                 process_BIT_FIELD},
  {AST_BITOR,                     process_BITOR},
  {AST_BITXOR,                    process_BITXOR},
  {AST_BREAK,                     process_BREAK},
  {AST_CASE,                      process_CASE},
  {AST_CAST,                      process_CAST},
  {AST_CHAR,                      process_CHAR},
  {AST_COMPOUND_STATEMENT,        process_COMPOUND_STATEMENT},
  {AST_CONTINUE,                  process_CONTINUE},
  {AST_CURLY_BRACES,              process_CURLY_BRACES},
  {AST_DECLARATION,               process_DECLARATION},
  {AST_DECLARATION_LIST,          process_DECLARATION_LIST},
  {AST_DECLARATION_SPECIFIERS,    process_DECLARATION_SPECIFIERS},
  {AST_DECLARATOR,                process_DECLARATOR},
  {AST_DEFAULT,                   process_DEFAULT},
  {AST_DEREFERENCEOF,             process_DEREFERENCEOF},
  {AST_DIV,                       process_DIV},
  {AST_DIVASSIGN,                 process_DIVASSIGN},
  {AST_DO,                        process_DO},
  {AST_DOT,                       process_DOT},
  {AST_ENUM,                      process_ENUM},
  {AST_EQ,                        process_EQ},
  {AST_EXP_LIST,                  process_EXP_LIST},
  {AST_EXP_STAT,                  process_EXP_STAT},
  {AST_FOR,                       process_FOR},
  {AST_FUNC_CALL,                 process_FUNC_CALL},
  {AST_FUNC_DECLARATOR,           process_FUNC_DECLARATOR},
  {AST_FUNC_DEF,                  process_FUNC_DEF},
  {AST_GE,                        process_GE},
  {AST_GOTO,                      process_GOTO},
  {AST_GT,                        process_GT},
  {AST_IDENTIFIER,                process_IDENTIFIER},
  {AST_IDENTIFIER_LIST,           process_IDENTIFIER_LIST},
  {AST_IF,                        process_IF},
  {AST_IF_ELSE,                   process_IF_ELSE},
  {AST_INIT,                      process_INIT},
  {AST_INITIALIZER_LIST,          process_INITIALIZER_LIST},
  {AST_INIT_DECLARATOR_LIST,      process_INIT_DECLARATOR_LIST},
  {AST_INLINE,                    process_INLINE},
  {AST_LABEL,                     process_LABEL},
  {AST_LABELED_INIT,              process_LABELED_INIT},
  {AST_LE,                        process_LE},
  {AST_LIST,                      process_LIST},
  {AST_LOGICALAND,                process_LOGICALAND},
  {AST_LOGICALOR,                 process_LOGICALOR},
  {AST_LT,                        process_LT},
  {AST_MOD,                       process_MOD},
  {AST_MODASSIGN,                 process_MODASSIGN},
  {AST_MUL,                       process_MUL},
  {AST_MULASSIGN,                 process_MULASSIGN},
  {AST_NE,                        process_NE},
  {AST_NEG,                       process_NEG},
  {AST_NEW_IDENTIFIER,            process_NEW_IDENTIFIER},
  {AST_NOT,                       process_NOT},
  {AST_NULL,                      process_NULL},
  {AST_NUMBER,                    process_NUMBER},
  {AST_ORASSIGN,                  process_ORASSIGN},
  {AST_PARAMETER_DECLARATION,     process_PARAMETER_DECLARATION},
  {AST_PARAMETER_LIST,            process_PARAMETER_LIST},
  {AST_PARENTHESIS,               process_PARENTHESIS},
  {AST_POINTER,                   process_POINTER},
  {AST_POSTMM,                    process_POSTMM},
  {AST_POSTPP,                    process_POSTPP},
  {AST_POW,                       process_POW},
  {AST_POWASSIGN,                 process_POWASSIGN},
  {AST_PREMM,                     process_PREMM},
  {AST_PREPP,                     process_PREPP},
  {AST_PROG,                      process_PROG},
  {AST_REFERENCE,                 process_REFERENCE},
  {AST_RETURN,                    process_RETURN},
  {AST_SHL,                       process_SHL},
  {AST_SHLASSIGN,                 process_SHLASSIGN},
  {AST_SHR,                       process_SHR},
  {AST_SHRASSIGN,                 process_SHRASSIGN},
  {AST_SIZEOF_EXP,                process_SIZEOF_EXP},
  {AST_SIZEOF_TYPE,               process_SIZEOF_TYPE},
  {AST_BRACKETS,                  process_BRACKETS},
  {AST_STAT_LIST,                 process_STAT_LIST},
  {AST_STORAGE_CLASS_SPECIFIER,   process_STORAGE_CLASS_SPECIFIER},
  {AST_STRING,                    process_STRING},
  {AST_STRUCT,                    process_STRUCT},
  {AST_SUB,                       process_SUB},
  {AST_SUBASSIGN,                 process_SUBASSIGN},
  {AST_SWITCH,                    process_SWITCH},
  {AST_TERNARY,                   process_TERNARY},
  {AST_TYPE_NAME,                 process_TYPE_NAME},
  {AST_TYPE_QUALIFIER,            process_TYPE_QUALIFIER},
  {AST_TYPE_QUALIFIER_LIST,       process_TYPE_QUALIFIER_LIST},
  {AST_UMINUS,                    process_UMINUS},
  {AST_UNION,                     process_UNION},
  {AST_UPLUS,                     process_UPLUS},
  {AST_WHILE,                     process_WHILE},
  {AST_XORASSIGN,                 process_XORASSIGN},
} ;


reduced * process_all(cell * L) {
  static int n = 0 ;
//  for(int i=0;i<n;i++) cout << "  " ;
//  cout << L->car << endl ;
  n++ ;
  auto r = table.find(L->car) ;
  if(r != table.end()){
    reduced * R = r->second(L) ;
    n-- ;
//    for(int i=0;i<n;i++) cout << "  " ;
//    if(R) cout << R->total << " , " << R->batch << endl ;
//    else cout << "NULL" << endl ;
    return R ;
  }else{
    unexpected_error(L, "unknown node symbol") ;
    return NULL ;
  }
}

extern "C" {
#include "syntactic.h"
}

#ifdef SEMANTIC_DEBUG

extern "C" int yyparse(void) ;
extern "C" cell * AST ;
extern "C" FILE * yyin ;

int main(int argc, char **argv) {
  yyin  = fdopen(0,"r") ;
  yy_switch_to_buffer(yy_create_buffer(yyin,YY_BUF_SIZE));
  if(yyparse() == 0){
    cell * A = AST ;
//    process_all(PREAMBLE) ;
    process_all(A) ;
  }else{
    exit(1) ; // syntactic error
  }
  return EXIT_SUCCESS ;
}

#endif
