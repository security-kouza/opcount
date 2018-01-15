/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#include "octet_string.h"

/* -------------------------------------------------------- */
/* Generic octet_string Operations                           */
/* -------------------------------------------------------- */

octet_string * new_octet_string(int a){
  octet_string * S = (octet_string*) calloc(1,sizeof(octet_string)) ;
  if(S){
    S->a = a ;
    S->n = 0 ;
    S->s = (char*) calloc(S->a, sizeof(char)) ;
    if(! S->s){
      free(S) ;
      S = NULL ;
    }
  }
  return S ;
}

octet_string * copy_octet_string(octet_string * S0){
  octet_string * S = (octet_string*) new_octet_string(S0->n);
  int i ;
  if(S){
    S->n = S0->n ;
    for(i=0;i<S->n;i++){
      S->s[i] = S0->s[i] ;
    }
  }
  return S ;
}

static int realloc_octet_string(octet_string * S){
  if(S){
    int a = ((S->a)?(S->a * 2):1) ;
    if(a > S->a){
      char * s = (char *) calloc(a, sizeof(char)) ;
      if(s){
        memcpy(s,S->s,sizeof(char) * S->a) ;
        free(S->s) ;
        S->s = s ;
        S->a = a ;
        return 1 ;
      }
    }
  }
  return 0 ;
}

int append_octet_string(octet_string * S, char c){
  if(S->n == S->a){
    if(! realloc_octet_string(S)){ return -1 ; }
  }
  S->s[S->n] = c ;
  return S->n++ ;
}

void delete_octet_string(octet_string * S){
  if(S){
    free(S->s) ;
    free(S) ;
  }
}

void clear_octet_string(octet_string * S){
  if(S->a){
    S->n    = 0 ;
    S->s[0] = 0 ;
  }
}
