#
#   Copyright (c) 2017 NTT corp. - All Rights Reserved
#
#   This file is part of opcount which is released under Software License
#   Agreement for Evaluation. See file LICENSE.pdf for full license details.
#

OPT=-O3
# OPT=-O0 -g

# =====================================================================

CPPFLAGS=-I/usr/local/include
CFLAGS=${CPPFLAGS} ${OPT} -std=c99 ${ADDITIONAL_CFLAGS}
CXXFLAGS=${CPPFLAGS} ${OPT} -std=c++11 -Wno-write-strings -DUSE_LONG_LONG
LDFLAGS=
#-lboost_regex
MAKEFILES=BSDmakefile GNUmakefile Makefile

# =====================================================================

.PHONY: all debug samples clean zip d

all: opcount
# all: semantic
# all: syntactic
# all: lexical 

clean:
	rm -f opcount semantic syntactic lexical *.o ast.* *.output syntactic.h y.tab.* syntactic.c lexical.c syntactic_debug.c lexical_debug.c tags lex.yy.c dump preamble.cpp *.d
	rm -f *.dot *.g0 *.g1 *.gsconv *.lp *.result *.sol *.tango

tags:
	${CTAGS} -R
zip: clean
	cd .. &&  \
	zip -9r --filesync opcount.zip opcount && \
	( mkdir -p backup ; \
	  cp opcount.zip backup/opcount-`date "+%Y-%m-%d-%H-%M-%S"`.zip )
d: all
	${CC} -E -MMD *.c >& /dev/null
	${CXX} -E -MMD *.cpp >& /dev/null

.y.o:
.l.o:
.y.h:
	${YACC} -t -v -d $<
	rm -f y.tab.c y.output 
	mv y.tab.h $*.h
.y.c:
	${YACC} -t -v -d $<
	rm -f y.tab.h y.output
	mv y.tab.c $*.c
.l.c:
	${LEX} -o$*.c $<

# =====================================================================
LEXICAL_O=octet_string.o reserved_word.o
LEXICAL_R=reserved_word.o octet_string.o 
LEXICAL_H=yacclex.h syntactic.h octet_string.h reserved_word.h
SYNTACTIC_O=${LEXICAL_O} lisp.o ast.o lexical.o 
SYNTACTIC_R=lexical.o ast.o lisp.o ${LEXICAL_R} 
SYNTACTIC_H=${LEXICAL_H} lisp.h ast.h
OPERATION_O=${SYNTACTIC_O} syntactic.o
OPERATION_R=syntactic.o ${SYNTACTIC_R}
OPERATION_H=${SYNTACTIC_H} semantic.h polynomial.h
DUMP_O=${OPERATION_O}
DUMP_R=${OPERATION_R}
DUMP_H=${OPERATION_H}
SEMANTIC_O=${DUMP_O} preamble.o declaration.o ast2str.o
SEMANTIC_R=ast2str.o declaration.o preamble.o ${DUMP_R}
SEMANTIC_H=${DUMP_H} preamble.h declaration.h ast2str.h
GSCONV_O=${SEMANTIC_O} semantic.o
GSCONV_R=semantic.o ${SEMANTIC_R}
# =====================================================================
opcount:    ${MAKEFILES} ${GSCONV_O} opcount.o
	   if [ "${LD}" = "g++" ] ; then ${LD} ${LDFLAGS} ${GSCONV_O} $@.o -o $@ ; fi
	   if [ "${LD}" = "c++" ] ; then ${LD} ${LDFLAGS} $@.o ${GSCONV_R} -o $@ ; fi
	   if [ "opt${OPT}" = "opt-O3" ] ; then strip -s $@ ; fi
semantic:  ${MAKEFILES} ${SEMANTIC_O} semantic_debug.o
	   if [ "${LD}" = "g++" ] ; then ${LD} ${LDFLAGS} ${SEMANTIC_O} $@_debug.o -o $@ ; fi
	   if [ "${LD}" = "c++" ] ; then ${LD} ${LDFLAGS} $@_debug.o ${SEMANTIC_R} -o $@ ; fi
dump:      ${MAKEFILES} ${DUMP_O} dump.o
	   ${LD} ${LDFLAGS} ${DUMP_O} $@.o -o $@
syntactic: ${MAKEFILES} ${SYNTACTIC_O} syntactic_debug.o
	   ${LD} ${LDFLAGS} ${SYNTACTIC_O} $@_debug.o -o $@
lexical:   ${MAKEFILES} ${LEXICAL_O} lexical_debug.o
	   ${LD} ${LDFLAGS} ${LEXICAL_O} $@_debug.o -o $@

# =====================================================================

lexical_debug.o: ${MAKEFILES} lexical.o
	${CC} ${CFLAGS} -c lexical.c -o $@ -DLEXICAL_DEBUG
syntactic_debug.o: ${MAKEFILES} syntactic.o
	${CC} ${CFLAGS} -c syntactic.c -o $@ -DSYNTACTIC_DEBUG
semantic_debug.o: ${MAKEFILES} semantic.o
	${CXX} ${CXXFLAGS} -c semantic.cpp -o $@ -DSEMANTIC_DEBUG

# =====================================================================
lisp.o:         ${MAKEFILES} lisp.c lisp.h
octet_string.o: ${MAKEFILES} octet_string.c octet_string.h
reserved_word.o: ${MAKEFILES} reserved_word.cpp reserved_word.h syntactic.h
ast.o:          ${MAKEFILES} ast.c ast.h
lexical.o:      ${MAKEFILES} lexical.c ${LEXICAL_H}
syntactic.o:    ${MAKEFILES} syntactic.c ${SYNTACTIC_H}
dump.o:         ${MAKEFILES} dump.cpp ${DUMP_H}
preamble.o:     ${MAKEFILES} preamble.cpp lisp.h ast.h preamble.h
semantic.o:     ${MAKEFILES} semantic.cpp ${SEMANTIC_H}
opcount.o:      ${MAKEFILES} opcount.cpp lisp.h ast.h reserved_word.h \
                semantic.h preamble.h syntactic.h
declaration.o:  ${MAKEFILES} declaration.cpp declaration.h syntactic.h reserved_word.h \
                lisp.h

# =====================================================================

lexical.c: ${MAKEFILES} lexical.l
syntactic.c: ${MAKEFILES} syntactic.y
syntactic.h: ${MAKEFILES} syntactic.y
preamble.cpp: ${MAKEFILES} dump preamble.txt
	./dump < preamble.txt > $@

# =====================================================================

ast.c: ${MAKEFILES} ast_c.sh ast.txt
	chmod 755 ./ast_c.sh
	./ast_c.sh ast.txt > ast.c
ast.h: ${MAKEFILES} ast_h.sh ast.txt
	chmod 755 ./ast_h.sh
	./ast_h.sh ast.txt > ast.h
ast.txt: ${MAKEFILES} ast_txt.sh syntactic.y
	chmod 755 ./ast_txt.sh
	./ast_txt.sh syntactic.y > ast.txt
