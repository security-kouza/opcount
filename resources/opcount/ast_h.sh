#! /bin/sh
#
#   Copyright (c) 2017 NTT corp. - All Rights Reserved
#
#   This file is part of opcount which is released under Software License
#   Agreement for Evaluation. See file LICENSE.pdf for full license details.
#


cat <<EOS
#ifndef AST_H
#define AST_H

#ifdef __cplusplus
extern "C" {
#endif

#define ast_node_declaration(name) extern char * name 

EOS

sed 's/\(^.*$\)/ast_node_declaration (\1) ;/' "$@"

cat <<EOS

#ifdef __cplusplus
}
#endif

#endif /* AST_H */
EOS
