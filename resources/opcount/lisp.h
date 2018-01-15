/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#ifndef LISP_H
#define LISP_H

#include <stdlib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cell_s {
  char          * car ;
  struct cell_s * cdr ;
#ifdef __cplusplus
//  inline char * operator[](int n) ;
  inline cell_s * element(int n) ;
  inline cell_s * pointer(int n) ;
#endif
} cell ;

// === implementation =========================================

extern cell * lisp_list  (int dummy, ... ) ;
extern cell * lisp_nconc (int dummy, ... ) ;

inline static cell * lisp_nthcdr(int n, cell * L){
       if(!L)  return NULL ;
  else if(n<0) return NULL ;
  else if(n)   return lisp_nthcdr(n-1, L->cdr) ;
  else         return L ;
}

inline static char * lisp_nth(int n, cell * L){
  L = lisp_nthcdr(n,L) ;
  return L?L->car:NULL ;
}

#ifdef __cplusplus
// inline char   * cell_s::operator[](int n){ return lisp_nth(n,this) ; }
inline cell_s * cell_s::element(int n){ return (cell *)lisp_nth(n,this) ; }
inline cell_s * cell_s::pointer(int n){ return (cell *)lisp_nthcdr(n,this) ; }
#endif

#define ENDOFVA       ((void*)-1)
#define LIST_i(...)   lisp_list(0,##__VA_ARGS__,ENDOFVA)
#define NCONC_i(...)  lisp_nconc(0,##__VA_ARGS__,ENDOFVA)
#define NTHCDR(n,L)   lisp_nthcdr(n,L)
#define NTH(n,L)      lisp_nth(n,L)

// === interface ==============================================

#define NIL()            NULL
#define LIST(...)        LIST_i(__VA_ARGS__)
#define NCONC(...)       NCONC_i(__VA_ARGS__)
#define APPEND(...)      NCONC(__VA_ARGS__)
#define INTEGRATE(L,...) NCONC(L,LIST(__VA_ARGS__))

#ifdef __cplusplus
}


#endif

#endif /* LISP_H */
