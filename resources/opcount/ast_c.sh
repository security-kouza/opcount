#! /bin/sh
#
#   Copyright (c) 2017 NTT corp. - All Rights Reserved
#
#   This file is part of opcount which is released under Software License
#   Agreement for Evaluation. See file LICENSE.pdf for full license details.
#


cat <<EOS
#include "ast.h"

#define ast_node_define(name) char * name = # name

EOS

sed 's/\(^.*$\)/ast_node_define (\1) ;/' "$@"
