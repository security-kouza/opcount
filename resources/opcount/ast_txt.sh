#! /bin/sh
#
#   Copyright (c) 2017 NTT corp. - All Rights Reserved
#
#   This file is part of opcount which is released under Software License
#   Agreement for Evaluation. See file LICENSE.pdf for full license details.
#


perl -ne '
  while(
    /\blist\((AST_[0-9A-Za-z_]*)\s*,\s*location/
  ){
    print($1,"\n");
    s/\blist\(AST_[0-9A-Za-z_]*\s*,\s*location//;
  }' "$@" | sort | uniq
