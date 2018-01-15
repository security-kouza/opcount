/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#include <iostream>
#include <vector>
#include <stdint.h>
#include <boost/unordered_map.hpp>
#include <string.h>

#include "lisp.h"
#include "ast.h"
#include "reserved_word.h"
#include "preamble.h"
#include "yacclex.h"

using namespace std   ;
using namespace boost ;

string replace_all(
  const string & a, // with
  const string & b, // in
  const string & c
){
  string result = c ;
  size_t index  = 0 ;
  size_t a_len  = a.size() ;
  size_t b_len  = b.size() ;
  while((index = result.find(a,index))!=string::npos){
    result.replace(index, a_len, b);
    index += b_len ;
  }
  return result ;
}

static string ast2str_all(cell * L) ;

static int nest = 0 ;
static inline void indent(){
  for(int i=0;i<nest;i++)cout << " " ;
}

// ===================================================================================

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

static inline string double_quote(const string & s){
  string d = "\"" ;
  for(auto c : s){
         if(unlikely(c=='"' ))d+="\\\"" ;
    else if(unlikely(c=='\\'))d+="\\\\" ;
    else d+=c;
  }
  d+="\"" ;
  return d ;
}

static inline string single_quote(const string & s){
  string d = "'" ;
  for(auto c : s){
         if(unlikely(c=='\'' ))d+="\\'" ;
    else if(unlikely(c=='\\'))d+="\\\\" ;
    else d+=c;
  }
  d+="'" ;
  return d ;
}

// ===================================================================================

static string ast2str_LITERAL(cell * L, int flag) {
  const char * s = (char *)L->element(2) ;
  switch(flag){
    case  1: return double_quote(string(s));
    case  2: return single_quote(string(s));
    default: return              string(s) ;
  }
}

static string ast2str_STRING                  (cell * L){ return ast2str_LITERAL(L,1); }
static string ast2str_CHAR                    (cell * L){ return ast2str_LITERAL(L,2); }
static string ast2str_IDENTIFIER              (cell * L){ return ast2str_LITERAL(L,0); }
static string ast2str_NEW_IDENTIFIER          (cell * L){ return ast2str_LITERAL(L,0); }
static string ast2str_NUMBER                  (cell * L){ return ast2str_LITERAL(L,0); }
static string ast2str_TYPE_QUALIFIER          (cell * L){ return ast2str_LITERAL(L,0); }
static string ast2str_STORAGE_CLASS_SPECIFIER (cell * L){ return ast2str_LITERAL(L,0); }

// ===================================================================================

static string ast2str_NULLARY(const char * const prefix){
  return string(prefix) ;
}

static string ast2str_BREAK    (cell * L){ return ast2str_NULLARY("break ;"); }
static string ast2str_CONTINUE (cell * L){ return ast2str_NULLARY("continue ;"); }
static string ast2str_NULL     (cell * L){ return ast2str_NULLARY(""); }

// ===================================================================================

static string ast2str_UNARY(
  cell * L,
  const char * const prefix,
  const char * const postfix
){
  cell * arg0 = L->element(2) ;
  string s0 = ast2str_all(arg0) ;
  string s  = string(prefix) + s0 + postfix ;
  return s ;
}

static string ast2str_ADDRESSOF          (cell * L){ return ast2str_UNARY(L,"&",""); }
static string ast2str_BRACKETS           (cell * L){ return ast2str_UNARY(L,"[","]"); }
static string ast2str_COMPOUND_STATEMENT (cell * L){ return ast2str_UNARY(L,"{\n","}"); }
static string ast2str_CURLY_BRACES       (cell * L){ return ast2str_UNARY(L,"{","}"); }
static string ast2str_DEFAULT            (cell * L){ return ast2str_UNARY(L,"default: ",""); }
static string ast2str_DEREFERENCEOF      (cell * L){ return ast2str_UNARY(L,"*",""); }
static string ast2str_EXP_STAT           (cell * L){ return ast2str_UNARY(L,"",";"); }
static string ast2str_GOTO               (cell * L){ return ast2str_UNARY(L,"goto ",";"); }
static string ast2str_INLINE             (cell * L){ return ast2str_UNARY(L,"inline ",";"); }
static string ast2str_NEG                (cell * L){ return ast2str_UNARY(L,"~",""); }
static string ast2str_NOT                (cell * L){ return ast2str_UNARY(L,"!",""); }
static string ast2str_PARENTHESIS        (cell * L){ return ast2str_UNARY(L,"(",")"); }
static string ast2str_POSTPP             (cell * L){ return ast2str_UNARY(L,"","++"); }
static string ast2str_POSTMM             (cell * L){ return ast2str_UNARY(L,"","--"); }
static string ast2str_PREMM              (cell * L){ return ast2str_UNARY(L,"--",""); }
static string ast2str_PREPP              (cell * L){ return ast2str_UNARY(L,"++",""); }
static string ast2str_RETURN             (cell * L){ return ast2str_UNARY(L,"return ",";"); }
static string ast2str_SIZEOF_TYPE        (cell * L){ return ast2str_UNARY(L,"sizeof(",")"); }
static string ast2str_SIZEOF_EXP         (cell * L){ return ast2str_UNARY(L,"sizeof(",")"); }
static string ast2str_UPLUS              (cell * L){ return ast2str_UNARY(L,"+",""); }
static string ast2str_UMINUS             (cell * L){ return ast2str_UNARY(L,"-",""); }

// ===================================================================================

static string ast2str_BINARY(
  cell * L,
  const char * const prefix,
  const char * const infix,
  const char * const postfix
){
  cell * arg0 = L->element(2) ;
  cell * arg1 = L->element(3) ;

  string s0 = ast2str_all(arg0) ;
  string s1 = ast2str_all(arg1) ;
  string s  = string(prefix) + s0 + infix + s1 + postfix ;
  return s ;
}

static string ast2str_ABSTRACT_DECLARATOR (cell * L){ return ast2str_BINARY(L,"","",""); }
static string ast2str_ARRAY_DECLARATOR (cell * L){ return ast2str_BINARY(L,"","[","]"); }
static string ast2str_ARRAY_REF    (cell * L){ return ast2str_BINARY(L,"","[","]"); }
static string ast2str_ASSIGN       (cell * L){ return ast2str_BINARY(L,"","=",""); }
static string ast2str_ADD          (cell * L){ return ast2str_BINARY(L,"","+",""); }
static string ast2str_SUB          (cell * L){ return ast2str_BINARY(L,"","-",""); }
static string ast2str_MUL          (cell * L){ return ast2str_BINARY(L,"","*",""); }
static string ast2str_DIV          (cell * L){ return ast2str_BINARY(L,"","/",""); }
static string ast2str_DECLARATION  (cell * L){ return ast2str_BINARY(L,""," ",";"); }
static string ast2str_DECLARATOR   (cell * L){ return ast2str_BINARY(L,"","",""); }
static string ast2str_FUNC_DECLARATOR (cell * L){ return ast2str_BINARY(L,"","(",")"); }
static string ast2str_POW          (cell * L){ return ast2str_BINARY(L,"","^",""); }
static string ast2str_EQ           (cell * L){ return ast2str_BINARY(L,"","==",""); }
static string ast2str_LOGICALAND   (cell * L){ return ast2str_BINARY(L,"","&&",""); }
static string ast2str_DOT          (cell * L){ return ast2str_BINARY(L,"",".",""); }
static string ast2str_ARROW        (cell * L){ return ast2str_BINARY(L,"","->",""); }
static string ast2str_ADDASSIGN    (cell * L){ return ast2str_BINARY(L,"","+=",""); }
static string ast2str_SUBASSIGN    (cell * L){ return ast2str_BINARY(L,"","-=",""); }
static string ast2str_MULASSIGN    (cell * L){ return ast2str_BINARY(L,"","*=",""); }
static string ast2str_DIVASSIGN    (cell * L){ return ast2str_BINARY(L,"","/=",""); }
static string ast2str_MODASSIGN    (cell * L){ return ast2str_BINARY(L,"","%=",""); }
static string ast2str_SHLASSIGN    (cell * L){ return ast2str_BINARY(L,"","<<=",""); }
static string ast2str_SHRASSIGN    (cell * L){ return ast2str_BINARY(L,"",">>=",""); }
static string ast2str_ANDASSIGN    (cell * L){ return ast2str_BINARY(L,"","&=",""); }
static string ast2str_XORASSIGN    (cell * L){ return ast2str_BINARY(L,"","(+)=",""); }
static string ast2str_ORASSIGN     (cell * L){ return ast2str_BINARY(L,"","|=",""); }
static string ast2str_PARAMETER_DECLARATION (cell * L){ return ast2str_BINARY(L,""," ",""); }
static string ast2str_POWASSIGN    (cell * L){ return ast2str_BINARY(L,"","^=",""); }
static string ast2str_BITAND       (cell * L){ return ast2str_BINARY(L,"","&",""); }
static string ast2str_BITOR        (cell * L){ return ast2str_BINARY(L,"","|",""); }
static string ast2str_BITXOR       (cell * L){ return ast2str_BINARY(L,"","(+)",""); }
static string ast2str_DO           (cell * L){ return ast2str_BINARY(L,"do ","while(",");");}
static string ast2str_FUNC_CALL    (cell * L){ return ast2str_BINARY(L,"","(",")"); }
static string ast2str_GE           (cell * L){ return ast2str_BINARY(L,"",">=",""); }
static string ast2str_GT           (cell * L){ return ast2str_BINARY(L,"",">",""); }
static string ast2str_IF           (cell * L){ return ast2str_BINARY(L,"if","",""); }
static string ast2str_LE           (cell * L){ return ast2str_BINARY(L,"","<=",""); }
static string ast2str_LOGICALOR    (cell * L){ return ast2str_BINARY(L,"","||",""); }
static string ast2str_LT           (cell * L){ return ast2str_BINARY(L,"","<",""); }
static string ast2str_MOD          (cell * L){ return ast2str_BINARY(L,"","%",""); }
static string ast2str_NE           (cell * L){ return ast2str_BINARY(L,"","!=",""); }
static string ast2str_POINTER      (cell * L){ return ast2str_BINARY(L,"*","",""); }
static string ast2str_REFERENCE    (cell * L){ return ast2str_BINARY(L,"&","",""); }
static string ast2str_SHL          (cell * L){ return ast2str_BINARY(L,"","<<",""); }
static string ast2str_SHR          (cell * L){ return ast2str_BINARY(L,"",">>",""); }
static string ast2str_WHILE        (cell * L){ return ast2str_BINARY(L,"while(",")",""); }
static string ast2str_TYPE_NAME    (cell * L){ return ast2str_BINARY(L,"","",""); }
static string ast2str_CAST         (cell * L){ return ast2str_BINARY(L,"(",")",""); }
static string ast2str_LABEL        (cell * L){ return ast2str_BINARY(L,"",": ",""); }
static string ast2str_LABELED_INIT (cell * L){ return ast2str_BINARY(L,"",": ",""); }
static string ast2str_SWITCH       (cell * L){ return ast2str_BINARY(L,"switch","",""); }
static string ast2str_CASE         (cell * L){ return ast2str_BINARY(L,"case ",": ",""); }
static string ast2str_INIT         (cell * L){ return ast2str_BINARY(L,"","=",""); }
static string ast2str_BIT_FIELD    (cell * L){ return ast2str_BINARY(L,"",": ",""); }

static string ast2str_SUE(
  cell * L,
  const char * const prefix,
  const char * const infix,
  const char * const postfix
){
  cell * loc  = L   ->cdr ;
  cell * arg0 = loc ->cdr ;
  cell * arg1 = arg0->cdr ;

  cell * struct_                 = (cell*)arg0->car ;
  cell * struct_declaration_list = (cell*)arg1->car ;

  string s0 = ast2str_all(struct_) ;
  string s1 = ast2str_all(struct_declaration_list) ;
  string s  = string(prefix) + s0 + infix + s1 + postfix ;
  return s ;
}

static string ast2str_STRUCT(cell * L){ return ast2str_SUE(L,"struct ","{\n","}"); }
static string ast2str_UNION (cell * L){ return ast2str_SUE(L,"union ","{\n","}"); }
static string ast2str_ENUM  (cell * L){ return ast2str_SUE(L,"enum ","{","}"); }

// ===================================================================================

static string ast2str_TERNARY(
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

  string s0 = ast2str_all((cell*)arg0->car) ;
  string s1 = ast2str_all((cell*)arg0->car) ;
  string s2 = ast2str_all((cell*)arg0->car) ;

  string s  = prefix + s0 + infix1 + s1 + infix2 + s2 + postfix ;
  return s ;
}

static string ast2str_IF_ELSE(cell * L){ return ast2str_TERNARY(L,"if","","else",""); }
static string ast2str_TERNARY(cell * L){ return ast2str_TERNARY(L,"","?",":",""); }

// ===================================================================================

static string ast2str_QUATERNARY(
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

  string s0 = ast2str_all((cell*)arg0->car) ;
  string s1 = ast2str_all((cell*)arg0->car) ;
  string s2 = ast2str_all((cell*)arg0->car) ;
  string s3 = ast2str_all((cell*)arg0->car) ;

  string s  = prefix + s0 + infix1 + s1 + infix2 + s2 + infix3 + s3 + postfix ;
  return s ;
}

static string ast2str_FOR     (cell * L){
  return ast2str_QUATERNARY(L,"for(",";",";",")","");
}

static string ast2str_FUNC_DEF(cell * L){
  cell * loc  = L   ->cdr ;
  cell * arg0 = loc ->cdr ;
  cell * arg1 = arg0->cdr ;
  cell * arg2 = arg1->cdr ;
  cell * arg3 = arg2->cdr ;

  cell * declaration_specifiers = (cell*)arg0->car ;
  cell * declarator             = (cell*)arg1->car ;
  cell * declaration_list       = (cell*)arg2->car ; /* K&R */ 
  cell * compound_statement     = (cell*)arg3->car ;

  string s0 = ast2str_all((cell*)arg0->car) ;
  string s1 = ast2str_all((cell*)arg0->car) ;
  string s2 = ast2str_all((cell*)arg0->car) ;
  string s3 = ast2str_all((cell*)arg0->car) ;
  string s ;

  if(declaration_specifiers->car == AST_NULL) s = string("")+s0+""+s1+"\n"+s2+""+s3+""  ;
  else                                        s = string("")+s0+" "+s1+"\n"+s2+""+s3+"" ;

  return s ;
}

// ===================================================================================

static string ast2str_LIST(cell * L) {
  cell * loc = L  ->cdr ;
  cell * arg = loc->cdr ;
  string s = "" ;
  for(;arg;arg=arg->cdr){
    if(s == "") s =         ast2str_all((cell*)arg->car) ;
    else        s += ", " + ast2str_all((cell*)arg->car) ; 
  }
  s = string(L->car+4) + "(" + s + ")" ;
  return s ;
}

static string ast2str_PROG(cell * L) {
  cell * loc = L  ->cdr ;
  cell * arg = loc->cdr ;
  string s = "" ;
  for(;arg;arg=arg->cdr){
    if(s == "") s  =        ast2str_all((cell*)arg->car) ;
    else        s += "\n" + ast2str_all((cell*)arg->car) ;
  }
  return s ;
}

static string ast2str_DECLARATION_LIST (cell * L) {
  cell * loc = L  ->cdr ;
  cell * arg = loc->cdr ;
  string s = "" ;
  for(;arg;arg=arg->cdr){
    s += "  " ;
    s += ast2str_all((cell*)arg->car) ;
    s += "\n" ;
  }
  return s ;
}

/*
extern string replace_all(
  const string & a, // with
  const string & b, // in
  const string & c
);
*/

static string ast2str_STAT_LIST (cell * L) {
  cell * loc = L  ->cdr ;
  cell * arg = loc->cdr ;
  string s = "", t ;
  for(;arg;arg=arg->cdr){
    t = ast2str_all((cell*)arg->car) ;
    t = "  " + replace_all("\n","\n  ",t) ;
    s += t + "\n" ;
  }
  return s ;
}

static string ast2str_DECLARATION_SPECIFIERS (cell * L) {
  cell * loc = L  ->cdr ;
  cell * arg = loc->cdr ;
  string s = "" ;
  for(;arg;arg=arg->cdr){
    if(s == "") s =                ast2str_all((cell*)arg->car) ;
    else        s += string(" ") + ast2str_all((cell*)arg->car) ;
  }
  return s ;
}

static string ast2str_TYPE_QUALIFIER_LIST (cell * L) {
  return ast2str_DECLARATION_SPECIFIERS (L);
}
static string ast2str_SPECIFIER_QUALIFIER_LIST(cell * L) {
  return ast2str_DECLARATION_SPECIFIERS (L);
}

static string ast2str_EXP_LIST(cell * L) {
  cell * loc = L  ->cdr ;
  cell * arg = loc->cdr ;
  string s = "" ;
  for(;arg;arg=arg->cdr){
    if(s == "") s =         ast2str_all((cell*)arg->car) ;
    else        s += ", " + ast2str_all((cell*)arg->car) ;
  }
  return s ;
}

static string ast2str_PARAMETER_LIST       (cell * L) { return ast2str_EXP_LIST(L); }
static string ast2str_IDENTIFIER_LIST      (cell * L) { return ast2str_EXP_LIST(L); }
static string ast2str_INITIALIZER_LIST     (cell * L) { return ast2str_EXP_LIST(L); }
static string ast2str_INIT_DECLARATOR_LIST (cell * L) { return ast2str_EXP_LIST(L); }

static unordered_map<char const * const, string(*)(cell *)> table = {
  {AST_ABSTRACT_DECLARATOR,       ast2str_ABSTRACT_DECLARATOR},
  {AST_ADD,                       ast2str_ADD},
  {AST_ADDASSIGN,                 ast2str_ADDASSIGN},
  {AST_ADDRESSOF,                 ast2str_ADDRESSOF},
  {AST_ANDASSIGN,                 ast2str_ANDASSIGN},
  {AST_ARRAY_DECLARATOR,          ast2str_ARRAY_DECLARATOR},
  {AST_ARRAY_REF,                 ast2str_ARRAY_REF},
  {AST_ARROW,                     ast2str_ARROW},
  {AST_ASM,                       ast2str_LIST},
  {AST_ASM_INIT,                  ast2str_LIST},
  {AST_ASM_IN_OUT,                ast2str_LIST},
  {AST_ASSIGN,                    ast2str_ASSIGN},
  {AST_BITAND,                    ast2str_BITAND},
  {AST_BIT_FIELD,                 ast2str_BIT_FIELD},
  {AST_BITOR,                     ast2str_BITOR},
  {AST_BITXOR,                    ast2str_BITXOR},
  {AST_BREAK,                     ast2str_BREAK},
  {AST_CASE,                      ast2str_CASE},
  {AST_CAST,                      ast2str_CAST},
  {AST_CHAR,                      ast2str_CHAR},
  {AST_COMPOUND_STATEMENT,        ast2str_COMPOUND_STATEMENT},
  {AST_CONTINUE,                  ast2str_CONTINUE},
  {AST_CURLY_BRACES,              ast2str_CURLY_BRACES},
  {AST_DECLARATION,               ast2str_DECLARATION},
  {AST_DECLARATION_LIST,          ast2str_DECLARATION_LIST},
  {AST_DECLARATION_SPECIFIERS,    ast2str_DECLARATION_SPECIFIERS},
  {AST_DECLARATOR,                ast2str_DECLARATOR},
  {AST_DEFAULT,                   ast2str_DEFAULT},
  {AST_DEREFERENCEOF,             ast2str_DEREFERENCEOF},
  {AST_DIV,                       ast2str_DIV},
  {AST_DIVASSIGN,                 ast2str_DIVASSIGN},
  {AST_DO,                        ast2str_DO},
  {AST_DOT,                       ast2str_DOT},
  {AST_ENUM,                      ast2str_ENUM},
  {AST_EQ,                        ast2str_EQ},
  {AST_EXP_LIST,                  ast2str_EXP_LIST},
  {AST_EXP_STAT,                  ast2str_EXP_STAT},
  {AST_FOR,                       ast2str_FOR},
  {AST_FUNC_CALL,                 ast2str_FUNC_CALL},
  {AST_FUNC_DECLARATOR,           ast2str_FUNC_DECLARATOR},
  {AST_FUNC_DEF,                  ast2str_FUNC_DEF},
  {AST_GE,                        ast2str_GE},
  {AST_GOTO,                      ast2str_GOTO},
  {AST_GT,                        ast2str_GT},
  {AST_IDENTIFIER,                ast2str_IDENTIFIER},
  {AST_IDENTIFIER_LIST,           ast2str_IDENTIFIER_LIST},
  {AST_IF,                        ast2str_IF},
  {AST_IF_ELSE,                   ast2str_IF_ELSE},
  {AST_INIT,                      ast2str_INIT},
  {AST_INITIALIZER_LIST,          ast2str_INITIALIZER_LIST},
  {AST_INIT_DECLARATOR_LIST,      ast2str_INIT_DECLARATOR_LIST},
  {AST_INLINE,                    ast2str_INLINE},
  {AST_LABEL,                     ast2str_LABEL},
  {AST_LABELED_INIT,              ast2str_LABELED_INIT},
  {AST_LE,                        ast2str_LE},
  {AST_LIST,                      ast2str_LIST},
  {AST_LOGICALAND,                ast2str_LOGICALAND},
  {AST_LOGICALOR,                 ast2str_LOGICALOR},
  {AST_LT,                        ast2str_LT},
  {AST_MOD,                       ast2str_MOD},
  {AST_MODASSIGN,                 ast2str_MODASSIGN},
  {AST_MUL,                       ast2str_MUL},
  {AST_MULASSIGN,                 ast2str_MULASSIGN},
  {AST_NE,                        ast2str_NE},
  {AST_NEG,                       ast2str_NEG},
  {AST_NEW_IDENTIFIER,            ast2str_NEW_IDENTIFIER},
  {AST_NOT,                       ast2str_NOT},
  {AST_NULL,                      ast2str_NULL},
  {AST_NUMBER,                    ast2str_NUMBER},
  {AST_ORASSIGN,                  ast2str_ORASSIGN},
  {AST_PARAMETER_DECLARATION,     ast2str_PARAMETER_DECLARATION},
  {AST_PARAMETER_LIST,            ast2str_PARAMETER_LIST},
  {AST_PARENTHESIS,               ast2str_PARENTHESIS},
  {AST_POINTER,                   ast2str_POINTER},
  {AST_POSTMM,                    ast2str_POSTMM},
  {AST_POSTPP,                    ast2str_POSTPP},
  {AST_POW,                       ast2str_POW},
  {AST_POWASSIGN,                 ast2str_POWASSIGN},
  {AST_PREMM,                     ast2str_PREMM},
  {AST_PREPP,                     ast2str_PREPP},
  {AST_PROG,                      ast2str_PROG},
  {AST_REFERENCE,                 ast2str_REFERENCE},
  {AST_RETURN,                    ast2str_RETURN},
  {AST_SHL,                       ast2str_SHL},
  {AST_SHLASSIGN,                 ast2str_SHLASSIGN},
  {AST_SHR,                       ast2str_SHR},
  {AST_SHRASSIGN,                 ast2str_SHRASSIGN},
  {AST_SIZEOF_EXP,                ast2str_SIZEOF_EXP},
  {AST_SIZEOF_TYPE,               ast2str_SIZEOF_TYPE},
  {AST_BRACKETS,                  ast2str_BRACKETS},
  {AST_STAT_LIST,                 ast2str_STAT_LIST},
  {AST_STORAGE_CLASS_SPECIFIER,   ast2str_STORAGE_CLASS_SPECIFIER},
  {AST_STRING,                    ast2str_STRING},
  {AST_STRUCT,                    ast2str_STRUCT},
  {AST_SUB,                       ast2str_SUB},
  {AST_SUBASSIGN,                 ast2str_SUBASSIGN},
  {AST_SWITCH,                    ast2str_SWITCH},
  {AST_TERNARY,                   ast2str_TERNARY},
  {AST_TYPE_NAME,                 ast2str_TYPE_NAME},
  {AST_TYPE_QUALIFIER,            ast2str_TYPE_QUALIFIER},
  {AST_TYPE_QUALIFIER_LIST,       ast2str_TYPE_QUALIFIER_LIST},
  {AST_UMINUS,                    ast2str_UMINUS},
  {AST_UNION,                     ast2str_UNION},
  {AST_UPLUS,                     ast2str_UPLUS},
  {AST_WHILE,                     ast2str_WHILE},
  {AST_XORASSIGN,                 ast2str_XORASSIGN},
} ;

static string ast2str_all(cell * L) {
  auto r = table.find(L->car) ;
  if(r != table.end()) return r->second(L) ;
  else                 return ast2str_LIST(L) ;
}

string ast2str(cell * L) { return ast2str_all(L) ; }
string AST2STR(cell * L) {
  if(!L)return "" ;
  cell * loc = L  ->cdr ;
  cell * arg = loc->cdr ;
  string s = "" ;
  if(is_literal(L)){
    cell * loc = L->cdr ;
    cell * arg = loc->cdr ;
    s = string(arg->car) ;
  }else{
    for(;arg;arg=arg->cdr){
      if(s == "") s  =       AST2STR((cell*)arg->car) ;
      else        s += "," + AST2STR((cell*)arg->car) ;
    }
  }
  return (string(L->car+4) + "(" + s + ")") ;
}
