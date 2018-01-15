/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#ifndef AST2STR_H
#define AST2STR_H
#include "lisp.h"
extern std::string ast2str(cell * L) ;
extern std::string AST2STR(cell * L) ;
static inline std::ostream & operator<<(std::ostream& out, cell * L){return out << AST2STR(L);}
#endif /* AST2STR_H */
