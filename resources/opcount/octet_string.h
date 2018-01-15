/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#ifndef _OCTET_STRING_H
#define _OCTET_STRING_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  int    a ;  /* allocated number of element */
  int    n ;  /* effective number of element */
  char * s ;  /* element vector table        */
} octet_string ;

extern octet_string * new_octet_string    (int n);
extern octet_string * copy_octet_string   (octet_string * S);
extern int            append_octet_string (octet_string * S, char c);
extern void           delete_octet_string (octet_string * S);
extern void           clear_octet_string  (octet_string * S);

#endif /* _OCTET_STRING_H */
