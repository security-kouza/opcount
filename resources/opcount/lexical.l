/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

%option yylineno
%option yy_scan_string
%{

// %option bison-locations
// %option bison-bridge
// #define YYSTYPE char
// #define YYSTYPE char*

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "yacclex.h"
#include "syntactic.h"
#include "octet_string.h"
#include "reserved_word.h"

YYLTYPE yylloc = {
  0, // first_line
  0, // first_column
  1, // last_line
  0, // last_column
 -1, // first_loc
 -1, // last_loc
} ;

// extern char * strdup(const char * s) ;

extern int yylineno ;
#define line ((intptr_t)yylineno)

// static int eof = 0 ;

static octet_string * literal = NULL ;

#ifdef LEXICAL_DEBUG
static int lexical_debug = 1 ;
#else
static int lexical_debug = 0 ;
#endif /* LEXICAL_DEBUG */

#define SET_LOCATION() \
	yylloc.first_line   = yylloc.last_line ; \
	yylloc.first_column = yylloc.last_column + 1 ; \
	yylloc.last_column += yyleng ; \
	yylloc.first_loc    = yylloc.last_loc + 1 ; \
	yylloc.last_loc    += yyleng ; \
	yylloc.length       = yyleng ; \
	// fprintf(stderr,"%d.%d-%d.%d:%s\n", yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column, yytext );

#define SET_LOCATION_NEW_LINE() \
	yylloc.first_line   = yylloc.last_line ; \
	yylloc.first_column = yylloc.last_column + 1 ; \
	yylloc.last_column  = 0 ; \
	yylloc.last_line   ++ ; \
	yylloc.first_loc    = yylloc.last_loc + 1 ; \
	yylloc.last_loc    += yyleng ; \
	yylloc.length       = yyleng ; \
	// fprintf(stderr,"%d.%d-%d.%d:%s\n", yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column, yytext );

%}

alpha    [A-Za-z_$]
alnum    [A-Za-z0-9_$]
digit    [0-9]
hex      [0-9A-Fa-f]
space    [ \t\r]
ull      ([Uu][Ll]?[Ll]?|[Ll][Ll]?[Uu]?)

exponent [eE][-+]?{digit}+
mantissa ({digit}*"."{digit}+|{digit}+".")
fl       [FLfl]

sprefix  (R|L|u|U|u8R|uR|LR|UR)

%start STR1 STR2 COM1 COM2 COM3

%%

<INITIAL>{sprefix}?\"		{
					SET_LOCATION() ;
			  		BEGIN STR1;
			  		if(!literal)literal = new_octet_string(128);
				}
<STR1>\\\"			{
					SET_LOCATION() ;
					append_octet_string(literal,'"') ;
				}
<STR1>\"			{
					SET_LOCATION() ;
			  		BEGIN INITIAL;
                          		append_octet_string(literal,0) ;
			  		yylval = strdup(literal->s) ;
			  		clear_octet_string(literal) ;
			  		return(STRING) ;
				}
<STR1>\n			{
					SET_LOCATION_NEW_LINE() ;
					append_octet_string(literal,'\n') ;
				}
<STR1>.				{
					SET_LOCATION() ;
					append_octet_string(literal,yytext[0]) ;
				}
<INITIAL>{sprefix}?\'		{
					SET_LOCATION() ;
			  		BEGIN STR2;
			  		if(!literal)literal = new_octet_string(128);
				}
<STR2>\\\'			{
					SET_LOCATION() ;
					append_octet_string(literal,'\'') ;
				}
<STR2>\'			{
					SET_LOCATION() ;
			  		BEGIN INITIAL;
                          		append_octet_string(literal,0) ;
			  		yylval = strdup(literal->s) ;
			  		clear_octet_string(literal) ;
			  		return(CHAR) ;
				}
<STR2>\n			{
					SET_LOCATION_NEW_LINE() ;
					append_octet_string(literal,'\n') ;
				}
<STR2>.				{
					SET_LOCATION() ;
					append_octet_string(literal,yytext[0]) ;
				}

<INITIAL>"/*"           	{
					SET_LOCATION() ;
					BEGIN COM1;
				}
<COM1>"*/"              	{
					SET_LOCATION() ;
					BEGIN INITIAL;
				}
<COM1>\n			{
					SET_LOCATION_NEW_LINE() ;
				}
<COM1>.				{
					SET_LOCATION() ;
				}
<INITIAL>"//"           	{
					SET_LOCATION() ;
					BEGIN COM2;
				}
<COM2>\n                	{
					SET_LOCATION_NEW_LINE() ;
					BEGIN INITIAL;
				}
<COM2>.				{
					SET_LOCATION() ;
				}
<INITIAL>^"#"           	{
					SET_LOCATION() ;
					BEGIN COM3;
				}
<COM3>\n                	{
					SET_LOCATION_NEW_LINE() ;
					BEGIN INITIAL;
				}
<COM3>.				{
					SET_LOCATION(); 
				}
<INITIAL>"->"			SET_LOCATION(); return ARROW ;
<INITIAL>"--"			SET_LOCATION(); return MM ;
<INITIAL>"++"			SET_LOCATION(); return PP ;

<INITIAL>"<<"			SET_LOCATION(); return SHL ;
<INITIAL>">>"			SET_LOCATION(); return SHR ;
<INITIAL>"<="			SET_LOCATION(); return LE ;
<INITIAL>">="			SET_LOCATION(); return GE ;
<INITIAL>"=="			SET_LOCATION(); return EQ ;
<INITIAL>"!="			SET_LOCATION(); return NE ;
<INITIAL>"&&"			SET_LOCATION(); return LOGICALAND ;
<INITIAL>"||"			SET_LOCATION(); return LOGICALOR ;
<INITIAL>"+="			SET_LOCATION(); return ADDASSIGN ;
<INITIAL>"-="			SET_LOCATION(); return SUBASSIGN ;
<INITIAL>"*="			SET_LOCATION(); return MULASSIGN ;
<INITIAL>"/="			SET_LOCATION(); return DIVASSIGN ;
<INITIAL>"%="			SET_LOCATION(); return MODASSIGN ;
<INITIAL>"<<="			SET_LOCATION(); return SHLASSIGN ;
<INITIAL>">>="			SET_LOCATION(); return SHRASSIGN ;
<INITIAL>"&="			SET_LOCATION(); return ANDASSIGN ;
<INITIAL>"|="			SET_LOCATION(); return ORASSIGN ;
<INITIAL>"^="			SET_LOCATION(); return POWASSIGN ;
<INITIAL>"(+)="			SET_LOCATION(); return XORASSIGN ;
<INITIAL>"(+)"			SET_LOCATION(); return BITXOR ;
<INITIAL>"..."			SET_LOCATION(); return ELLIPSIS ;

<INITIAL>{mantissa}{exponent}?{fl}?	{
						SET_LOCATION();
						yylval=strdup(yytext);
						return NUMBER ;
					}
<INITIAL>{digit}+{exponent}{fl}?	{
						SET_LOCATION();
						yylval=strdup(yytext);
						return NUMBER ;
					}
<INITIAL>0[xX]{hex}+{ull}?	{
					SET_LOCATION();
					yylval=strdup(yytext);
					return NUMBER ;
				}
<INITIAL>{digit}+{ull}?		{
					SET_LOCATION();
					yylval=strdup(yytext);
					return NUMBER ;
				}
<INITIAL>{alpha}{alnum}*	{
					SET_LOCATION();
					yylval=(char*)unique_symbol(yytext) ;
					/* fprintf(stderr, "debug lex: %s\n", yylval) ; */
					return symbol_to_token(yylval) ;
				}
<INITIAL>{space}+		{
					SET_LOCATION();
				}
<INITIAL>\n			{
					SET_LOCATION_NEW_LINE();
				}
<INITIAL>.			{
					SET_LOCATION();
					return yytext[0] ;
				}

%%

int yywrap()
{
  return 1;
}

#ifdef LEXICAL_DEBUG
#include "operation.h"

char * yylval ;

#define CASE1(X) \
case X: printf("%02ld " # X ":\n",line); break;

#define CASE2(X) \
case X: printf("%1$02ld " # X ":'%2$s'(%2$p)\n",line,yylval); break;

int main()
{
  int t ;

  while((t = yylex()) != 0){
    switch(t){

      CASE1(ARROW) ;
      CASE1(MM) ;
      CASE1(PP) ;
      CASE1(SHL) ;
      CASE1(SHR) ;
      CASE1(LE) ;
      CASE1(GE) ;
      CASE1(EQ) ;
      CASE1(NE) ;
      CASE1(LOGICALAND) ;
      CASE1(LOGICALOR) ;
      CASE1(ADDASSIGN) ;
      CASE1(SUBASSIGN) ;
      CASE1(MULASSIGN) ;
      CASE1(DIVASSIGN) ;
      CASE1(MODASSIGN) ;
      CASE1(SHLASSIGN) ;
      CASE1(SHRASSIGN) ;
      CASE1(ANDASSIGN) ;
      CASE1(ORASSIGN) ;
      CASE1(POWASSIGN) ;
      CASE1(XORASSIGN) ;
      CASE1(BITXOR) ;

      CASE2(FUNCTION) ;
      CASE2(INLINE) ;
      CASE2(CASE) ;
      CASE2(DEFAULT) ;
      CASE2(IF) ;
      CASE2(ELSE) ;
      CASE2(SWITCH) ;
      CASE2(WHILE) ;
      CASE2(DO) ;
      CASE2(FOR) ;
      CASE2(GOTO) ;
      CASE2(CONTINUE) ;
      CASE2(BREAK) ;
      CASE2(RETURN) ;
      CASE2(SIZEOF) ;
      CASE2(ENUM) ;
      CASE2(STRUCT) ;
      CASE2(UNION) ;
      CASE2(CLASS) ;
      CASE2(AUTO) ;
      CASE2(REGISTER) ;
      CASE2(STATIC) ;
      CASE2(EXTERN) ;
      CASE2(TYPEDEF) ;
      CASE2(CONST) ;
      CASE2(VOLATILE) ;

      CASE2(STRING) ;
      CASE2(NUMBER) ;
      CASE2(IDENTIFIER) ;
      CASE2(TYPE) ;

      default        : printf("%02ld token :'%c'\n",line,t     ); break;
    }
  }
}
#endif /* LEXICAL_DEBUG */
