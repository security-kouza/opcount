#
#   Copyright (c) 2017 NTT corp. - All Rights Reserved
#
#   This file is part of opcount which is released under Software License
#   Agreement for Evaluation. See file LICENSE.pdf for full license details.
#

ADDITIONAL_CFLAGS=-D_BSD_SOURCE -D_POSIX_SOURCE

CTAGS=ctags
YACC=bison -y
LEX=flex
CC=cc
CXX=c++
LD=c++

ifeq ($(OS),Windows_NT)
    CC=x86_64-w64-mingw32-gcc
    CXX=x86_64-w64-mingw32-g++
    LD=x86_64-w64-mingw32-g++ -static-libgcc -static-libstdc++
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CC=gcc
        CXX=g++
        LD=g++
    endif
    ifeq ($(UNAME_S),Darwin)

    endif
endif

include Makefile
