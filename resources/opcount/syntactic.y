/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

%locations
%{
#include <stdio.h>
#include <stdint.h>
#include "lisp.h"
#include "ast.h"
#include "yacclex.h"
#include "syntactic.h"
#include "reserved_word.h"

#define TYPEDEFNAME TYPE

extern char * TYPE_ELLIPSIS ;

static YYLTYPE * location(YYLTYPE * loc){
  YYLTYPE * ret = NULL ;
  if(loc){
    ret = (YYLTYPE*)malloc(sizeof(YYLTYPE)) ;
    if(ret) *ret = *loc ;
//    fprintf(stderr,"%d\n",loc->first_line);
  }
  return ret ;
}

// extern int yylex(YYSTYPE * yylval_param,YYLTYPE * yylloc_param );
extern int yylex(void) ;
extern int yyparse(void) ;
extern char *yytext;

void yyerror(char const * msg)
{
  fprintf (stderr, "%s at line %d column %d: '%s'\n", 
    msg, yylloc.first_line, yylloc.first_column, yytext
  );
}

/*
cell * INSERTX(cell * L, cell * M){
  if(L){
    cell * lineno = L->cdr      ;
    cell * arg0   = lineno->cdr ;
    lineno->cdr   = M           ;
    M->cdr        = arg0        ;
  }
  return L ;
}

#define INSERT(L,...) INSERTX(L,LIST(__VA_ARGS__))
*/

int is_literal(cell * L) {
return (L->car == AST_NUMBER)  ||
       (L->car == AST_CHAR)    ||
       (L->car == AST_STRING)  ||
       (L->car == AST_IDENTIFIER) ||
       (L->car == AST_NEW_IDENTIFIER) ||
       (L->car == AST_STORAGE_CLASS_SPECIFIER) ||
       (L->car == AST_TYPE_QUALIFIER) ;
}

static int is_IDENTIFIER_or_TYPE(const char * symbol, cell * L) {
  int ret = 0 ;
  if(L){
    cell * lineno    = L->cdr ;
    cell * arg0      = lineno->cdr ; // declaration_specifiers 
    if(is_literal(L)){
      if((L->car == AST_IDENTIFIER) || (L->car == AST_NEW_IDENTIFIER)){
        if( arg0->car == symbol) ret = 1 ;
      }
    }else{
      for(;arg0;arg0=arg0->cdr) ret |= is_IDENTIFIER_or_TYPE(symbol, (cell*)arg0->car) ;
    }
  }
  return ret ;
}

static int is_STORAGE_CLASS_SPECIFIER(const char * symbol, cell * L) {
  int ret = 0 ;
  if(L){
    cell * lineno    = L->cdr ;
    cell * arg0      = lineno->cdr ; // declaration_specifiers 
    if(is_literal(L)){
      if(L->car == AST_STORAGE_CLASS_SPECIFIER){
        if( arg0->car == symbol) ret = 1 ;
      }
    }else{
      for(;arg0;arg0=arg0->cdr) ret |= is_STORAGE_CLASS_SPECIFIER(symbol, (cell*)arg0->car) ;
    }
  }
  return ret ;
}

int is_prohibited(cell * L) { return is_IDENTIFIER_or_TYPE(SYMBOL_PROHIBITED,L) ; }
int is_group     (cell * L) { return is_IDENTIFIER_or_TYPE(SYMBOL_GROUP     ,L) ; }
// int is_group0    (cell * L) { return is_IDENTIFIER_or_TYPE(SYMBOL_PROHIBITED,L) ; }
// int is_group1    (cell * L) { return is_IDENTIFIER_or_TYPE(SYMBOL_PROHIBITED,L) ; }

int is_typedef(cell * L) { return is_STORAGE_CLASS_SPECIFIER(SYMBOL_TYPEDEF,L) ; }
int is_macro  (cell * L) { return is_STORAGE_CLASS_SPECIFIER(SYMBOL_MACRO  ,L) ; }
int is_extern (cell * L) { return is_STORAGE_CLASS_SPECIFIER(SYMBOL_EXTERN ,L) ; }

char * init_declarator_to_symbol (cell * L) {
  char * ret = NULL ;
  char c ;
  if(L){
    cell * lineno    = L->cdr ;
    cell * arg0      = lineno->cdr ;
    if(is_literal(L)){
      if((L->car == AST_IDENTIFIER) ||
         (L->car == AST_NEW_IDENTIFIER) ) ret = arg0->car ;
    }else{
      for(;arg0;arg0=arg0->cdr) {
        ret = init_declarator_to_symbol ((cell*)arg0->car) ;
	if(ret) break ;
      }
    }
  }
  return ret ;
}

void register_symbol(cell * L, int token) {
  char * symbol = init_declarator_to_symbol (L) ;
#if 0
  cell * print_node(cell * L, int n, int recursive);
  printf("-"); print_node(L,0,1) ;
  printf("register_symbol(\"%s\",%d)\n",symbol,token);
  fprintf(stderr,"register_symbol: \"%s\" = %d\n",symbol, token) ;
#endif
  if(symbol) symbol_stack_push(symbol, token);
//  print_stack();
}

void register_symbols_raw (cell * L, int token) {
  if(L){
    cell * lineno    = L->cdr ;
    cell * arg0      = lineno->cdr ;
    for(;arg0;arg0=arg0->cdr) register_symbol((cell *)arg0->car, token) ;
  }
}

#include <unistd.h>

void register_symbols (cell * L) {
  if(L){
    cell * lineno    = L->cdr ;
    cell * arg0      = lineno->cdr ;
    cell * arg1      = arg0->cdr   ;
    cell * declaration_specifiers  = (cell *)arg0->car ;
    cell * init_declarator_list    = (cell *)arg1->car ;
    int token = is_typedef(declaration_specifiers) ? TYPEDEFNAME : IDENTIFIER ;
    register_symbols_raw ( init_declarator_list, token ) ;
  }
}

#ifdef SYNTACTIC_DEBUG

cell * print_node(cell * L, int n, int recursive){
  cell * M ;
  int i ;

  for(i=0;i<n;i++){putchar(' ');}
//  printf("%p:",L);
  if(!L){
    printf("NULL\n");
  }else{
    cell * lineno    = L->cdr ;
    cell * arg0      = lineno->cdr ;
    printf("%s line %ld ", L->car, (intptr_t)lineno->car) ;
    if(is_literal(L)){
      printf("[%s]\n", arg0->car) ;
    }else{
      printf("\n") ;
      if(recursive){
        for(;arg0;arg0=arg0->cdr)
          print_node((cell*)arg0->car,n+1,recursive);
      }
    }
  }
  return L ;
}

void accept(YYSTYPE L){
  printf("\naccepted !\n\n") ;
  print_node(L,0,1) ;
}

cell * print_node_A(cell * L){
  putchar(' '); print_node(L,0,0); return L;
}
cell * print_node_B(cell * L){
  putchar('+'); print_node(L,0,0); return L;
}

#define list(...)      print_node_A(LIST(__VA_ARGS__))
#define integrate(...) print_node_B(INTEGRATE(__VA_ARGS__))
#define insert(...)    print_node_B(INSERT(__VA_ARGS__))

int main() {
//  symbol_stack_push("integer"     ,TYPEDEFNAME) ;
//  symbol_stack_push("string"      ,TYPEDEFNAME) ;
//  symbol_stack_push("proof"       ,TYPEDEFNAME) ;
//  symbol_stack_push("group"       ,TYPEDEFNAME) ;
//  symbol_stack_push("target"      ,TYPEDEFNAME) ;
//  symbol_stack_push("crs"         ,TYPEDEFNAME) ;
//  symbol_stack_push("$identifier" ,TYPEDEFNAME) ;
//  symbol_stack_push("$ellipsis"   ,TYPEDEFNAME) ;
  return yyparse(); 
}

#else

YYSTYPE AST ;
void accept(YYSTYPE L){ AST = L ; }
#define list(...)      LIST(__VA_ARGS__)
#define integrate(...) INTEGRATE(__VA_ARGS__)
#define insert(...)    INSERT(__VA_ARGS__)

#endif

#ifdef SYNTACTIC_DEBUG
#else
#endif

char * literal(cell * L){
  cell * lineno = L->cdr ;
  cell * opname = lineno->cdr ;
  cell * arg    = opname->cdr ;
  return arg->car ;
}

%}

%nonassoc EMPTY            /* level 24 ;                                 */
%left     SEMICOLON ';'    /* level 23 ;                                 */
%nonassoc STATEMENT        /* level 22                                   */
                           /* level 21                                   */
%nonassoc FUNCTION MACRO ALIGNAS ALIGNOF ASM AUTO BREAK CASE CATCH CLASS CONCEPT CONST CONSTEXPR CONST_CAST CONTINUE DECLTYPE DEFAULT DELETE DO DYNAMIC_CAST ELSE ENUM EXPLICIT EXPORT EXTERN FOR FRIEND GOTO IF INLINE MUTABLE NAMESPACE NEW NOEXCEPT OPERATOR PRIVATE PROTECTED PUBLIC REGISTER REINTERPRET_CAST REQUIRES RETURN STATIC STATIC_ASSERT STATIC_CAST STRUCT SWITCH TEMPLATE THIS THREAD_LOCAL THROW TRY TYPEDEF TYPEID TYPENAME UNION USING VIRTUAL VOLATILE WHILE

%right    DECL             /* level 20                                   */
%right    INIT             /* level 19                                   */
%left     COMMA ','        /* level 18 ,                                 */
%nonassoc ELLIPSIS         /* level 17 ...                               */
                           /* level 16 = += -= *= /= %= <<= >>= &= ^= |= */
%right    ASSIGN '=' ADDASSIGN SUBASSIGN MULASSIGN DIVASSIGN MODASSIGN SHLASSIGN SHRASSIGN ANDASSIGN XORASSIGN ORASSIGN POWASSIGN
%right    TERNARY '?' ':'  /* level 15 ? :                               */
%left     LOGICALOR        /* level 14 ||                                */
%left     LOGICALAND       /* level 13 &&                                */
%left     BITOR  '|'       /* level 12 |                                 */
%left     BITXOR           /* level 11 ^                                 */
%left     BITAND '&'       /* level 10 &                                 */
%left     EQ NE            /* level  9 ==  !=                            */
%left     CMP LE GE '<' '>'/* level  8 < <= >= >                         */
%left     SHIFT SHL SHR    /* level  7 << >>                             */
%left     ADD '+' '-'      /* level  6 + -                               */
%left     MUL '*' '/' '%'  /* level  5 * / %                             */
%right    CAST             /* level  4 CAST                              */
                           /* level  3 + - ! ~ & * sizeof ++x --x        */
%right    UNARY SIZEOF '!' '~'
%right    POW '^'          /* level  2 '^'                               */
                           /* level  1 x->y x.y x++ x-- x(y) x[y]        */
%left     BIND PP MM ARROW '.'
                           /* level  0                                   */
%nonassoc PRIME '(' ')' '[' ']' '{' '}' IDENTIFIER STRING NUMBER CHAR TYPE /* TYPEDEFNAME */
/*
%nonassoc SUPREME 
*/

%%

accept	: program
		{ accept($1); }
	;

program : /* empty */
		{ $$ = list(AST_PROG,location(&@$)) ; }
	| program func_def_statement
		{ $$ = integrate($1,$2) ; }
	| program declaration_statement
		{ $$ = integrate($1,$2) ; }
	| program inline_statement
		{ $$ = integrate($1,$2) ; }
	| program compound_statement
		{ $$ = integrate($1,$2) ; }
	;

statement_list
	:                                                   %prec EMPTY
		{ $$ = list(AST_STAT_LIST,location(&@$)) ; }
	| statement_list statement
		{ $$ = integrate($1,$2) ; }
	;

/* ======================================== */

statement
	: declaration_statement
		{ $$ = $1 ; }
	| labeled_statement
		{ $$ = $1 ; }
	| expression_statement
		{ $$ = $1 ; }
	| compound_statement
		{ $$ = $1 ; }
	| selection_statement
		{ $$ = $1 ; }
	| iteration_statement
		{ $$ = $1 ; }
	| jump_statement
		{ $$ = $1 ; }
	| inline_statement
		{ $$ = $1 ; }
        | asm_statement
		{ $$ = $1 ; }
	;

declaration_statement
	: declaration
                { $$ = $1 ;
		  register_symbols($1); }
	;

func_def_statement
	: function_definition
                { $$ = $1 ; }
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement /* K&R */
		{ $$ = list(AST_FUNC_DEF,location(&@$),$1,$2,$3,$4) ; }
	| declaration_specifiers declarator                  compound_statement
		{ $$ = list(AST_FUNC_DEF,location(&@$),$1,$2,
		    list(AST_NULL,location(&@$)),$3) ; }
	|                        declarator declaration_list compound_statement /* K&R */
		{ $$ = list(AST_FUNC_DEF,location(&@$),
		    list(AST_DECLARATION_SPECIFIERS,location(&@$)),$1,$2,$3) ; }
	|                        declarator                  compound_statement
		{ $$ = list(AST_FUNC_DEF,location(&@$),
		    list(AST_DECLARATION_SPECIFIERS,location(&@$)),$1,
		    list(AST_NULL,location(&@$)),$2);}
	;

declaration_list
	: declaration
		{ $$ = list(AST_DECLARATION_LIST,location(&@$),$1);     }
	| declaration_list declaration
                { $$ = integrate($1,$2) ; }
	;

declaration
	: declaration_specifiers                      sc                   %prec STATEMENT
		{ $$ = list(AST_DECLARATION,location(&@$),$1,list(AST_INIT_DECLARATOR_LIST,location(&@$))); }
	| declaration_specifiers init_declarator_list sc                   %prec ELSE
		{ $$ = list(AST_DECLARATION,location(&@$),$1,$2); }
	;

declaration_specifiers
	: storage_class_specifier                                           %prec STATEMENT
	        { $$ = list(AST_DECLARATION_SPECIFIERS,location(&@$),$1); }
	| declaration_specifiers storage_class_specifier                    %prec STATEMENT
                { $$ = integrate($1,$2) ; }
	| type_qualifier                                                    %prec STATEMENT
		{ $$ = list(AST_DECLARATION_SPECIFIERS,location(&@$),$1); }
	| declaration_specifiers type_qualifier                             %prec STATEMENT
                { $$ = integrate($1,$2) ; }
	| type_specifier                                                    %prec STATEMENT
		{ $$ = list(AST_DECLARATION_SPECIFIERS,location(&@$),$1);
		  lexer_hack_stack_set(1) ; }
	| declaration_specifiers type_specifier                             %prec STATEMENT
                { $$ = integrate($1,$2) ;
		  lexer_hack_stack_set(1) ; }
	;

storage_class_specifier
	: AUTO
		{ $$ = list(AST_STORAGE_CLASS_SPECIFIER,location(&@$),$1); }
	| REGISTER
		{ $$ = list(AST_STORAGE_CLASS_SPECIFIER,location(&@$),$1); }
	| STATIC
		{ $$ = list(AST_STORAGE_CLASS_SPECIFIER,location(&@$),$1); }
	| EXTERN
		{ $$ = list(AST_STORAGE_CLASS_SPECIFIER,location(&@$),$1); }
	| INLINE
		{ $$ = list(AST_STORAGE_CLASS_SPECIFIER,location(&@$),$1); }
	| VIRTUAL
		{ $$ = list(AST_STORAGE_CLASS_SPECIFIER,location(&@$),$1); }
	| FRIEND
		{ $$ = list(AST_STORAGE_CLASS_SPECIFIER,location(&@$),$1); }
	| TYPEDEF
		{ $$ = list(AST_STORAGE_CLASS_SPECIFIER,location(&@$),$1); }
	| MACRO
		{ $$ = list(AST_STORAGE_CLASS_SPECIFIER,location(&@$),$1); }
        ;

type_qualifier
	: CONST
		{ $$ = list(AST_TYPE_QUALIFIER,location(&@$),$1);     }
	| VOLATILE
		{ $$ = list(AST_TYPE_QUALIFIER,location(&@$),$1);     }
	;

type_specifier
	: struct_specifier
		{ $$ = $1 ; }
	| union_specifier
		{ $$ = $1 ; }
	| enum_specifier
		{ $$ = $1 ; }
	| type
		{ $$ = $1 ; }
	;

struct_specifier
	: struct_ lc struct_declaration_list rc
		{ $$ = list(AST_STRUCT,location(&@$),$1,$3) ; }
	| struct_
		{ $$ = list(AST_STRUCT,location(&@$),$1,list(AST_NULL,location(&@$))) ; }
	;

struct_
	: STRUCT
		{ $$ = list(AST_NEW_IDENTIFIER,location(&@$),new_symbol("struct")) ;
		  lexer_hack_stack_set(1) ; }
	| STRUCT new_identifier_or_type
		{ $$ = $2 ;
		  lexer_hack_stack_set(1) ; }
	;

union_specifier
	: union_ lc struct_declaration_list rc
		{ $$ = list(AST_UNION,location(&@$),$1,$3) ; }
	| union_
		{ $$ = list(AST_UNION,location(&@$),$1,list(AST_NULL,location(&@$))) ; }
	;

union_
	: UNION
		{ $$ = list(AST_NEW_IDENTIFIER,location(&@$),new_symbol("union")) ;
		  lexer_hack_stack_set(1) ; }
	| UNION new_identifier_or_type
		{ $$ = $2 ;
		  lexer_hack_stack_set(1) ; }
	;

enum_specifier
	: enum_ lc enumerator_list rc
		{ $$ = list(AST_ENUM,location(&@$),$1,$3) ; }
	| enum_ lc enumerator_list ',' rc
		{ $$ = list(AST_ENUM,location(&@$),$1,$3) ; }
	| ENUM identifier_or_type
		{ $$ = list(AST_ENUM,location(&@$),$2,list(AST_NULL,location(&@$))) ; 
		  lexer_hack_stack_set(1) ; }
	;

enum_
	: ENUM
		{ $$ = list(AST_NEW_IDENTIFIER,location(&@$),new_symbol("enum")) ;
		  lexer_hack_stack_set(1) ; }
	| ENUM new_identifier_or_type
		{ $$ = $2 ;
		  lexer_hack_stack_set(1) ; }
	;

struct_declaration_list
	: /* empty */
		{ $$ = list(AST_STAT_LIST,location(&@$)); }
	| struct_declaration_list struct_declaration
                { $$ = integrate($1,$2) ; }
/*	| struct_declaration_list func_def_statement
		{ $$ = integrate($1,$2) ; } */
	;

init_declarator_list
	: init_declarator                                                 %prec ELSE
		{ $$ = list(AST_INIT_DECLARATOR_LIST,location(&@$),$1);     }
	| init_declarator_list ',' init_declarator                        %prec ELSE
                { $$ = integrate($1,$3) ; }
	;

init_declarator
	: declarator
		{ $$ = $1 ; }
	| declarator '=' initializer
		{ $$ = list(AST_INIT,location(&@$),$1,$3); }
	| declarator asm_statement
		{ $$ = list(AST_ASM_INIT,location(&@$),$1,$2); }
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list_opt sc          %prec STATEMENT
		{ $$ = list(AST_DECLARATION,location(&@$),$1,$2); }

	;

specifier_qualifier_list
	: declaration_specifiers
		{ $$ = $1 ; }
	;

struct_declarator_list_opt
	:
		{ $$ = list(AST_INIT_DECLARATOR_LIST,location(&@$)) ; }
	| struct_declarator_list
		{ $$ = $1 ; }
	;

struct_declarator_list
	: struct_declarator
		{ $$ = list(AST_INIT_DECLARATOR_LIST,location(&@$),$1) ; }
	| struct_declarator_list ',' struct_declarator
                { $$ = integrate($1,$3) ; }
	;

struct_declarator
	: init_declarator
		{ $$ = $1 ; }
	|            ':' constant_expression
		{ $$ = list(AST_BIT_FIELD,location(&@$),list(AST_NULL,location(&@$)),$2) ; }
	| declarator ':' constant_expression
		{ $$ = list(AST_BIT_FIELD,location(&@$),$1,$3) ; }
	;

enumerator_list
	: enumerator
		{ $$ = list(AST_INIT_DECLARATOR_LIST,location(&@$),$1);     }
	| enumerator_list ',' enumerator                             %prec COMMA
                { $$ = integrate($1,$3) ; }
	;

enumerator
	: new_identifier_or_type
		{ $$ = $1 ; }
	| new_identifier_or_type '=' constant_expression
		{ $$ = list(AST_INIT,location(&@$),$1,$3) ; }
	;

declarator
	:         direct_declarator                                  %prec STATEMENT
		{ $$ = list(AST_DECLARATOR,location(&@$),list(AST_NULL,location(&@$)),$1) ; }
	| pointer direct_declarator                                  %prec STATEMENT
		{ $$ = list(AST_DECLARATOR,location(&@$),$1,$2) ; }
	;

direct_declarator
	: new_identifier                                             %prec PRIME
		{ $$ = $1 ; }
	| lp declarator rp                                           %prec PRIME /* (decl) */
                { $$ = list(AST_PARENTHESIS,location(&@$),$2) ; }
	| direct_declarator lb rb                                    %prec BIND  /* [] */
		{ $$ = list(AST_ARRAY_DECLARATOR,location(&@$), $1, list(AST_EXP_LIST,location(&@$))) ; }
	| direct_declarator lb constant_expression rb                %prec BIND
		{ $$ = list(AST_ARRAY_DECLARATOR,location(&@$), $1, $3) ; }
	| direct_declarator lp rp                                    %prec BIND  /* () */
		{ $$ = list(AST_FUNC_DECLARATOR,location(&@$), $1, list(AST_PARAMETER_LIST,location(&@$))) ; }
	| direct_declarator lp parameter_type_list rp                %prec BIND
		{ $$ = list(AST_FUNC_DECLARATOR,location(&@$), $1, $3) ; }
	| direct_declarator lp identifier_list rp                    %prec BIND
		{ $$ = list(AST_FUNC_DECLARATOR,location(&@$), $1, $3) ; }
	;

pointer
	: '*'                                                           %prec UNARY
		{ $$ = list(AST_POINTER,location(&@$),
		    list(AST_TYPE_QUALIFIER_LIST,location(&@$)),
		    list(AST_NULL,location(&@$))
		  ) ;
		}
	| '*' type_qualifier_list                                       %prec UNARY
		{ $$ = list(AST_POINTER,location(&@$),
		    $2,
		    list(AST_NULL,location(&@$))
		  ) ;
		}
	| '*'                     pointer                               %prec UNARY
		{ $$ = list(AST_POINTER,location(&@$),
		    list(AST_TYPE_QUALIFIER_LIST,location(&@$)),
		    $2
		  ) ;
		}
	| '*' type_qualifier_list pointer                               %prec UNARY
		{ $$ = list(AST_POINTER,location(&@$),
		    $2,
		    $3
		  ) ;
		}
	| '&'                                                           %prec UNARY
		{ $$ = list(AST_REFERENCE,location(&@$),
		    list(AST_TYPE_QUALIFIER_LIST,location(&@$)),
		    list(AST_NULL,location(&@$))
		  ) ;
		}
	| '&' type_qualifier_list                                       %prec UNARY
		{ $$ = list(AST_REFERENCE,location(&@$),
		    $2,
		    list(AST_NULL,location(&@$))
		  ) ;
		}
	| '&'                     pointer                               %prec UNARY
		{ $$ = list(AST_REFERENCE,location(&@$),
		    list(AST_TYPE_QUALIFIER_LIST,location(&@$)),
		    $2
		  ) ;
		}
	| '&' type_qualifier_list pointer                               %prec UNARY
		{ $$ = list(AST_REFERENCE,location(&@$),
		    $2,
		    $3
		  ) ;
		}
	;

type_qualifier_list
	: type_qualifier
		{ $$ = list(AST_TYPE_QUALIFIER_LIST,location(&@$),$1) ; }
	| type_qualifier_list type_qualifier
                { $$ = integrate($1,$2) ; }
	;

parameter_type_list
	: parameter_list
		{ $$ = $1 ; }
	| ELLIPSIS
		{ $$ = list(AST_PARAMETER_LIST,location(&@$),
		    list(AST_PARAMETER_DECLARATION,location(&@$),
		      list(AST_IDENTIFIER,location(&@$),"$ellipsis"),
		      list(AST_NULL,location(&@$))
                    )
		  ) ;
		}
	| parameter_list ',' ELLIPSIS                                 %prec COMMA
		{ $$ = integrate($1,
		    list(AST_PARAMETER_DECLARATION,location(&@$),
		      list(AST_IDENTIFIER,location(&@$),"$ellipsis"),
		      list(AST_NULL,location(&@$))
                    )
                  ) ;
		}
	;

parameter_list
	: parameter_declaration
		{ $$ = list(AST_PARAMETER_LIST,location(&@$),$1) ; }
	| parameter_list ',' parameter_declaration                    %prec COMMA
                { $$ = integrate($1,$3) ; }
	;

parameter_declaration
	: declaration_specifiers declarator
		{ $$ = list(AST_PARAMETER_DECLARATION,location(&@$),$1,$2) ;
		  lexer_hack_stack_set(0) ; }
	| declaration_specifiers 
		{ $$ = list(AST_PARAMETER_DECLARATION,location(&@$),$1,list(AST_NULL,location(&@$))) ;
		  lexer_hack_stack_set(0) ; }
	| declaration_specifiers abstract_declarator
		{ $$ = list(AST_PARAMETER_DECLARATION,location(&@$),$1,$2) ;
		  lexer_hack_stack_set(0) ; }
	;

identifier_list
	: identifier
		{ $$ = list(AST_IDENTIFIER_LIST,location(&@$),$1) ; }
	| identifier_list ',' identifier                              %prec COMMA
                { $$ = integrate($1,$3) ; }
	;

initializer_list
	: initializer
		{ $$ = list(AST_INITIALIZER_LIST,location(&@$),$1) ; }
	| initializer_list ',' initializer                            %prec COMMA
                { $$ = integrate($1,$3) ; }
	;

initializer
	: expression
                { $$ = $1 ; }
	| identifier_or_type ':' constant_expression
		{ $$ = list(AST_LABELED_INIT,location(&@$),$1,$3) ; }
	| lc initializer_list     rc
                { $$ = list(AST_CURLY_BRACES,location(&@$),$2) ; }
	| lc initializer_list ',' rc
                { $$ = list(AST_CURLY_BRACES,location(&@$),$2) ; }
	;

/* == type_name ========================= */

type_name
	: specifier_qualifier_list
		{ $$ = $1 ; }
	| specifier_qualifier_list abstract_declarator
		{ $$ = list(AST_TYPE_NAME,location(&@$),$1,$2) ; }
	;

abstract_declarator
	: pointer
		{ $$ = $1 ; }
	|         direct_abstract_declarator
		{ $$ = $1 ; }
	| pointer direct_abstract_declarator
		{ $$ = list(AST_ABSTRACT_DECLARATOR,location(&@$),$1,$2) ; }
	;

direct_abstract_declarator
	: lp abstract_declarator rp                            %prec PRIME /* ( ... ) */
                { $$ = list(AST_PARENTHESIS,location(&@$),$2) ; }
	|                            lb rb                     %prec BIND  /* [] */
		{ $$ = list(AST_ARRAY_DECLARATOR,location(&@$),
		    list(AST_NULL,location(&@$)),
		    list(AST_EXP_LIST,location(&@$))
		  ) ;
		}
	|                            lb constant_expression rb %prec BIND  /* [ exp ] */
		{ $$ = list(AST_ARRAY_DECLARATOR,location(&@$),
		    list(AST_NULL,location(&@$)),
		    $2
		  ) ;
		}
	| direct_abstract_declarator lb rb                     %prec BIND /* [] */
		{ $$ = list(AST_ARRAY_DECLARATOR,location(&@$),
		    $1,
		    list(AST_EXP_LIST,location(&@$))
		  ) ;
		}
	| direct_abstract_declarator lb constant_expression rb %prec BIND /* [ exp ] */
		{ $$ = list(AST_ARRAY_DECLARATOR,location(&@$),
		    $1,
		    $3
		  ) ;
		}
	|                            lp rp                     %prec BIND /* () */
                { $$ = list(AST_FUNC_DECLARATOR,location(&@$),
		    list(AST_NULL,location(&@$)),
		    list(AST_PARAMETER_LIST,location(&@$))
                  ) ;
		}
	|                            lp parameter_type_list rp %prec BIND /* (...) */
                { $$ = list(AST_FUNC_DECLARATOR,location(&@$),
		    list(AST_NULL,location(&@$)),
		    $2
                  ) ;
		}
	| direct_abstract_declarator lp rp                     %prec BIND /* () */
                { $$ = list(AST_FUNC_DECLARATOR,location(&@$),
		    $1,
		    list(AST_PARAMETER_LIST,location(&@$))
                  ) ;
		}
	| direct_abstract_declarator lp parameter_type_list rp %prec BIND /* (...) */
                { $$ = list(AST_FUNC_DECLARATOR,location(&@$),
		    $1,
		    $3
                  ) ;
		}
	;

/* -- */

/* =================================================================================== */
/* =================================================================================== */
/* =================================================================================== */

labeled_statement
	: identifier_or_type ':' statement                  %prec STATEMENT
                { $$ = list(AST_LABEL,location(&@$),$1,$3) ; }
	| CASE constant_expression ':' statement            %prec STATEMENT
                { $$ = list(AST_CASE,location(&@$),$2,$4) ; }
	| DEFAULT ':' statement                             %prec STATEMENT
                { $$ = list(AST_DEFAULT,location(&@$),$3) ; }
	;

compound_statement
	: lc statement_list rc                              %prec STATEMENT
		{ $$ = list(AST_COMPOUND_STATEMENT,location(&@$),$2) ; }
	;

selection_statement
	: IF parentheses_expression statement               %prec STATEMENT
                { $$ = list(AST_IF,location(&@$),$2,$3) ; }
	| IF parentheses_expression statement
          ELSE                      statement               %prec ELSE
                { $$ = list(AST_IF_ELSE,location(&@$),$2,$3,$5) ; }
	| SWITCH parentheses_expression statement           %prec STATEMENT
                { $$ = list(AST_SWITCH,location(&@$),$2,$3) ; }
        ;

iteration_statement
	: WHILE lp expression_opt rp statement              %prec STATEMENT
                { $$ = list(AST_WHILE,location(&@$),$3,$5) ; }
	| DO statement WHILE lp expression_opt rp sc        %prec STATEMENT
                { $$ = list(AST_DO,location(&@$),$2,$5) ; }
	| FOR lp expression_opt sc expression_opt sc expression_opt rp
	  statement                                         %prec STATEMENT
                { $$ = list(AST_FOR,location(&@$),$3,$5,$7,$9) ; }
        ;

expression_statement
	: expression_opt sc                                 %prec STATEMENT
                { $$ = list(AST_EXP_STAT,location(&@$),$1) ; }
	;

jump_statement
	: GOTO identifier_or_type sc                        %prec STATEMENT
                { $$ = list(AST_GOTO,location(&@$),$2); }
	| CONTINUE sc                                       %prec STATEMENT
                { $$ = list(AST_CONTINUE,location(&@$)); }
	| BREAK sc                                          %prec STATEMENT
                { $$ = list(AST_BREAK,location(&@$)); }
	| RETURN expression_opt sc                          %prec STATEMENT
                { $$ = list(AST_RETURN,location(&@$),$2) ; }
	;

inline_statement
	: INLINE string sc                                  %prec STATEMENT
                { $$ = list(AST_INLINE,location(&@$),$2) ; }
	;

asm_statement
	: ASM lp string rp                                          %prec STATEMENT
                { $$ = list(AST_ASM,location(&@$),
		    $3,list(AST_LIST,location(&@$)), list(AST_LIST,location(&@$)), list(AST_LIST,location(&@$))) ; }
	| ASM lp string ':' asm_in_out_opt rp                       %prec STATEMENT
                { $$ = list(AST_ASM,location(&@$),
		    $3,$5, list(AST_LIST,location(&@$)), list(AST_LIST,location(&@$))) ; }
	| ASM lp string ':' asm_in_out_opt ':' asm_in_out_opt rp    %prec STATEMENT
                { $$ = list(AST_ASM,location(&@$),
		    $3,$5,$7, list(AST_LIST,location(&@$))) ; }
	| ASM lp string ':' asm_in_out_opt 
	                ':' asm_in_out_opt ':' expression_opt rp %prec STATEMENT
                { $$ = list(AST_ASM,location(&@$),
		    $3,$5,$7,$9) ; }
	;

asm_in_out_opt
	:
		{ $$ = list(AST_LIST, location(&@$)) ; }
	| asm_in_out_list
		{ $$ = $1 ; }
	;


asm_in_out_list
	: asm_in_out
		{ $$ = list(AST_LIST, location(&@$), $1) ; }
	| asm_in_out_list ',' asm_in_out
		{ $$ = integrate($1,$3) ; }
	;

asm_in_out
	: string lp expression rp
		{ $$ = list(AST_ASM_IN_OUT, location(&@$), $1, $3) ; }
	;

/* ===================================================================== */

expression_opt
	:                                                   %prec EMPTY
		{ $$ = list(AST_EXP_LIST,location(&@$)) ; }
	| expression_list                                   %prec COMMA
		{ $$ = $1 ; }
	;

expression_list
	: expression                                        %prec TERNARY
		{ $$ = list(AST_EXP_LIST,location(&@$),$1) ; }
	| expression_list ',' expression                    %prec COMMA
		{ $$ = integrate($1,$3) ; }
	;

expression
        : expression '=' expression                         %prec ASSIGN
                { $$ = list(AST_ASSIGN,location(&@$),$1,$3) ; }
        | expression POWASSIGN expression                   %prec ASSIGN
                { $$ = list(AST_POWASSIGN,location(&@$),$1,$3) ; }
        | expression ORASSIGN expression                    %prec ASSIGN
                { $$ = list(AST_ORASSIGN,location(&@$),$1,$3) ; }
        | expression XORASSIGN expression                   %prec ASSIGN
                { $$ = list(AST_XORASSIGN,location(&@$),$1,$3) ; }
        | expression ANDASSIGN expression                   %prec ASSIGN
                { $$ = list(AST_ANDASSIGN,location(&@$),$1,$3) ; }
        | expression SHRASSIGN expression                   %prec ASSIGN
                { $$ = list(AST_SHRASSIGN,location(&@$),$1,$3) ; }
        | expression SHLASSIGN expression                   %prec ASSIGN
                { $$ = list(AST_SHLASSIGN,location(&@$),$1,$3) ; }
        | expression MODASSIGN expression                   %prec ASSIGN
                { $$ = list(AST_MODASSIGN,location(&@$),$1,$3) ; }
        | expression DIVASSIGN expression                   %prec ASSIGN
                { $$ = list(AST_DIVASSIGN,location(&@$),$1,$3) ; }
        | expression MULASSIGN expression                   %prec ASSIGN
                { $$ = list(AST_MULASSIGN,location(&@$),$1,$3) ; }
        | expression SUBASSIGN expression                   %prec ASSIGN
                { $$ = list(AST_SUBASSIGN,location(&@$),$1,$3) ; }
        | expression ADDASSIGN expression                   %prec ASSIGN
                { $$ = list(AST_ADDASSIGN,location(&@$),$1,$3) ; }
        | constant_expression
                { $$ = $1 ; }
        ;

constant_expression
        : expression '?' expression ':' expression          %prec TERNARY
                { $$ = list(AST_TERNARY,location(&@$),$1,$3,$5) ; }
        | expression LOGICALOR expression                   %prec LOGICALOR
                { $$ = list(AST_LOGICALOR,location(&@$),$1,$3) ; }
        | expression LOGICALAND expression                  %prec LOGICALAND
                { $$ = list(AST_LOGICALAND,location(&@$),$1,$3) ; }
        | expression '|' expression                         %prec BITOR
                { $$ = list(AST_BITOR,location(&@$),$1,$3) ; }
        | expression BITXOR expression                      %prec BITXOR
                { $$ = list(AST_BITXOR,location(&@$),$1,$3) ; }
        | expression '&' expression                         %prec BITAND
                { $$ = list(AST_BITAND,location(&@$),$1,$3) ; }
        | expression NE expression                          %prec EQ
                { $$ = list(AST_NE,location(&@$),$1,$3) ; }
        | expression EQ expression                          %prec EQ
                { $$ = list(AST_EQ,location(&@$),$1,$3) ; }
        | expression GE expression                          %prec CMP
                { $$ = list(AST_GE,location(&@$),$1,$3) ; }
        | expression '>' expression                         %prec CMP
                { $$ = list(AST_GT,location(&@$),$1,$3) ; }
        | expression LE expression                          %prec CMP
                { $$ = list(AST_LE,location(&@$),$1,$3) ; }
        | expression '<' expression                         %prec CMP
                { $$ = list(AST_LT,location(&@$),$1,$3) ; }
        | expression SHR expression                         %prec SHIFT
                { $$ = list(AST_SHR,location(&@$),$1,$3) ; }
        | expression SHL expression                         %prec SHIFT
                { $$ = list(AST_SHL,location(&@$),$1,$3) ; }
        | expression '-' expression                         %prec ADD
                { $$ = list(AST_SUB,location(&@$),$1,$3) ; }
        | expression '+' expression                         %prec ADD
                { $$ = list(AST_ADD,location(&@$),$1,$3) ; }
        | expression '%' expression                         %prec MUL
                { $$ = list(AST_MOD,location(&@$),$1,$3) ; }
        | expression '/' expression                         %prec MUL
                { $$ = list(AST_DIV,location(&@$),$1,$3) ; }
        | expression '*' expression                         %prec MUL
                { $$ = list(AST_MUL,location(&@$),$1,$3) ; }
        | lp type_name rp expression                        %prec CAST
                { $$ = list(AST_CAST,location(&@$),$2,$4) ; }
        | SIZEOF lp type_name rp                            %prec UNARY
                { $$ = list(AST_SIZEOF_TYPE,location(&@$),$3) ; }
        | SIZEOF expression                                 %prec UNARY
                { $$ = list(AST_SIZEOF_EXP,location(&@$),$2) ; }
        | '!' expression                                    %prec UNARY
                { $$ = list(AST_NOT,location(&@$),$2) ; }
        | '~' expression                                    %prec UNARY
                { $$ = list(AST_NEG,location(&@$),$2) ; }
        | '-' expression                                    %prec UNARY
                { $$ = list(AST_UMINUS,location(&@$),$2) ; }
        | '+' expression                                    %prec UNARY
                { $$ = list(AST_UPLUS,location(&@$),$2) ; }
        | '&' expression                                    %prec UNARY
                { $$ = list(AST_ADDRESSOF,location(&@$),$2) ; }
        | '*' expression                                    %prec UNARY
                { $$ = list(AST_DEREFERENCEOF,location(&@$),$2) ; }
        | PP expression                                     %prec UNARY
                { $$ = list(AST_PREPP,location(&@$),$2) ; }
        | MM expression                                     %prec UNARY
                { $$ = list(AST_PREMM,location(&@$),$2) ; }
        | expression '^' expression                         %prec POW
                { $$ = list(AST_POW,location(&@$),$1,$3) ; }
	| expression ARROW identifier_or_type               %prec BIND
                { $$ = list(AST_ARROW,location(&@$),$1,$3) ; }
        | expression '.' identifier_or_type                 %prec BIND
                { $$ = list(AST_DOT,location(&@$),$1,$3) ; }
/*	| expression '.' lb identifier_or_type rb           %prec BIND
                { $$ = list(AST_DOT,location(&@$),$1,list(AST_BRACKETS,location(&@$),$4)) ; } */
	| expression '.' parentheses_expression             %prec BIND
                { $$ = list(AST_DOT,location(&@$),$1,$3) ; }
        | expression lb expression rb                       %prec BIND /* exp [ exp ] */
                { $$ = list(AST_ARRAY_REF,location(&@$),$1,$3) ; }
        | expression lp expression_opt rp                   %prec BIND /* exp ( exp ) */
                { $$ = list(AST_FUNC_CALL,location(&@$),$1,$3) ; }
        | expression PP                                     %prec BIND
                { $$ = list(AST_POSTPP,location(&@$),$1) ; }
        | expression MM                                     %prec BIND
                { $$ = list(AST_POSTMM,location(&@$),$1) ; }
	| prime_expression
                { $$ = $1 ; }
	;

prime_expression
	: string                                            %prec PRIME
                { $$ = $1 ; }
	| number                                            %prec PRIME
                { $$ = $1 ; }
	| identifier                                        %prec PRIME
                { $$ = $1 ; }
	| parentheses_expression                            %prec PRIME
                { $$ = $1 ; }
        ;

string
	: STRING                                            %prec PRIME
                { $$ = list(AST_STRING,location(&@$),$1) ; }
	| string STRING                                     %prec PRIME
                { $$ = integrate($1,$2) ; }
	;

number
	: NUMBER                                            %prec PRIME
                { $$ = list(AST_NUMBER,location(&@$),$1) ; }
	| CHAR
		{ $$ = list(AST_CHAR,location(&@$),$1) ; }
	;

identifier_or_type
	: identifier                                        %prec PRIME
		{ $$ = $1 ; }
	| type                                              %prec PRIME
		{ $$ = $1 ; }
	;

new_identifier_or_type
	: new_identifier                                        %prec PRIME
		{ $$ = $1 ; }
	| new_type                                              %prec PRIME
		{ $$ = $1 ; }
	;


identifier
	: IDENTIFIER                                        %prec PRIME
		{ $$ = list(AST_IDENTIFIER,location(&@$),$1); 
                  /* fprintf(stderr, "identifier: %s (%p)\n", $1, $1); */
		}
	;

new_identifier
	: IDENTIFIER                                        %prec PRIME
		{ $$ = list(AST_NEW_IDENTIFIER,location(&@$),$1); }
	;

type
	: TYPE                                              %prec PRIME
                { $$ = list(AST_IDENTIFIER,location(&@$),$1) ; }       /* lexer hack */
	;

new_type
	: TYPE                                              %prec PRIME
                { $$ = list(AST_NEW_IDENTIFIER,location(&@$),$1) ; }       /* lexer hack */
	;

parentheses_expression
	: lb expression_opt rb                              %prec PRIME /* [ exp ] */
                { $$ = list(AST_BRACKETS,location(&@$),$2) ; }
        | lp expression_opt rp                              %prec PRIME /* ( exp ) */
                { $$ = list(AST_PARENTHESIS,location(&@$),$2) ; }
	| lp compound_statement rp                          %prec PRIME /* ({ stat }) */
                { $$ = list(AST_PARENTHESIS,location(&@$),$2) ; }
	;

lp	: '('	{ lexer_hack_stack_push() ; }
	;

rp	: ')'	{ lexer_hack_stack_pop() ; }
	;

lb	: '['	{ lexer_hack_stack_push() ; }
	;

rb	: ']'	{ lexer_hack_stack_pop() ; }
	;

lc	: '{'	{ frame_stack_push();
                  lexer_hack_stack_push() ; }
	;

rc	: '}'	{ lexer_hack_stack_pop() ;
		  frame_stack_pop(); }
	;

sc	: ';'	{ lexer_hack_stack_set(0) ; }
	;

%%
