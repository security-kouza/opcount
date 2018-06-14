/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "lisp.h"
#include "polynomial.h"
#include <boost/unordered_map.hpp>

extern void     semantic_error        (cell * L, const std::string & message) ;
extern cell   * dummy_LOCATION ;

#define excell cell
#define declare_TYPE_(NAME) extern char * TYPE_ ## NAME ;

declare_TYPE_(NULL)          ;          // TYPE_NULL
declare_TYPE_(INTEGER)       ;          // TYPE_INTEGER
declare_TYPE_(GROUP)         ;          // TYPE_GROUP
declare_TYPE_(TARGET)        ;          // TYPE_TARGET
declare_TYPE_(PROHIBITED)    ;          // TYPE_PROHIBITED

declare_TYPE_(GROUP0)        ;          // TYPE_GROUP0
declare_TYPE_(GROUP1)        ;          // TYPE_GROUP1
declare_TYPE_(GROUP2)        ;          // TYPE_GROUP1
declare_TYPE_(DUPLEX)        ;          // TYPE_DUPLEX
declare_TYPE_(LITERAL)       ;          // TYPE_LITERAL
declare_TYPE_(IDENTIFIER)    ;          // TYPE_IDENTIFIER
declare_TYPE_(LIST_VARIABLE) ;          // TYPE_LIST_VARIABLE

declare_TYPE_(LOGICAL_AND)   ;          // TYPE_LOGICAL_AND
declare_TYPE_(LIST_LITERAL)  ;          // TYPE_LIST_LITERAL
declare_TYPE_(OPTION)        ;          // TYPE_OPTION
declare_TYPE_(IMPOSSIBLE)    ;          // TYPE_IMPOSSIBLE
declare_TYPE_(POINTER)       ;          // TYPE_POINTER
declare_TYPE_(REFERENCE)     ;          // TYPE_REFERENCE


typedef long long int integer_ ;
typedef Polynomial<std::string, integer_, integer_> INTEGER ;

typedef LinearCombination<std::string, INTEGER> formula ;
struct reduced ;

extern std::vector<char *> function_stack ;

#endif /* SEMANTIC_H */
