/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#include <iostream>
#include <boost/unordered_map.hpp>
#include <vector>
#include <string.h>
#include <limits.h>
#include "syntactic.h"
#include "reserved_word.h"
using namespace std ;
using namespace boost ;


// alignas		(C++11)
// alignof		(C++11)
// and			"&&"
// and_eq		"&="
// asm			insert an assembly instruction
// auto			declare a local variable
// bitand		'&'
// bitor		'|'
// bool			declare a boolean variable
// break		break out of a loop
// case			a block of code in a switch statement
// catch		handles exceptions from throw
// char			declare a character variable
// char16_t		(C++11)
// char32_t		(C++11)
// class		declare a class
// compl		'~'
// concept		(concepts TS)
// const		declare immutable data or functions that do not change data
// constexpr		(C++11)
// const_cast		cast from const variables
// continue		bypass iterations of a loop
// decltype		(C++11)
// default		default handler in a case statement
// delete		make memory available
// do			looping construct
// double		declare a double precision floating-point variable
// dynamic_cast		perform runtime casts
// else			alternate case for an if statement
// enum			create enumeration types
// explicit		only use constructors when they exactly match
// export		allows template definitions to be separated from their declarations
// extern		tell the compiler about variables defined elsewhere
// false		the boolean value of false
// float		declare a floating-point variable
// for			looping construct
// friend		grant non-member function access to private data
// goto			jump to a different part of the program
// if			execute code based off of the result of a test
// inline		optimize calls to short functions
// int			declare a integer variable
// long			declare a long integer variable
// mutable		override a const variable
// namespace		partition the global namespace by defining a scope
// new			allocate dynamic memory for a new variable
// noexcept		(C++11)
// not			'!'
// not_eq		"!="
// nullptr		(C++11)
// operator		create overloaded operator functions
// or			"||"
// or_eq		"|="
// private		declare private members of a class
// protected		declare protected members of a class
// public		declare public members of a class
// register		request that a variable be optimized for speed
// reinterpret_cast	change the type of a variable
// requires		(concepts TS)
// return		return from a function
// short		declare a short integer variable
// signed		modify variable type declarations
// sizeof		return the size of a variable or type
// static		create permanent storage for a variable
// static_assert	(C++11)
// static_cast		perform a nonpolymorphic cast
// struct		define a new structure
// switch		execute code based off of different possible values for a variable
// template		create generic functions
// this			a pointer to the current object
// thread_local		(C++11)
// throw		throws an exception
// true			the boolean value of true
// try			execute code that can throw an exception
// typedef		create a new type name from an existing type
// typeid		describes an object
// typename		declare a class or undefined type
// union		a structure that assigns multiple variables to the same memory location
// unsigned		declare an unsigned integer variable
// using		import complete or partial namespaces into the current scope
// virtual		create a function that can be overridden by a derived class
// void			declare functions or data with no associated data type
// volatile		warn the compiler about variables that can be modified unexpectedly
// wchar_t		declare a wide-character variable
// while		looping construct
// xor			"(+)"
// xor_eq		"(+)="


extern "C" {
// -------------------------------------------------------------
char * SYMBOL_INTEGER          = "integer"          ;
char * SYMBOL_GROUP            = "group"            ;
char * SYMBOL_PROHIBITED       = "prohibited"       ;
char * SYMBOL_TARGET           = "target"           ;
char * SYMBOL_GROUP0           = "group0"           ;
char * SYMBOL_GROUP1           = "group1"           ;
char * SYMBOL_GROUP2           = "group2"           ;
char * SYMBOL_DUPLEX           = "duplex"           ;
char * SYMBOL_LIST             = "list"             ;
char * SYMBOL_FUNCTION         = "function"         ;
char * SYMBOL_MACRO            = "macro"            ;
// -------------------------------------------------------------
char * SYMBOL_ALIGNAS          = "alignas"          ;
char * SYMBOL_ALIGNOF          = "alignof"          ;
char * SYMBOL_ASM              = "asm"              ;
char * SYMBOL_AND              = "and"              ;
char * SYMBOL_AND_EQ           = "and_eq"           ;
char * SYMBOL_AUTO             = "auto"             ;
char * SYMBOL_BITAND           = "bitand"           ;
char * SYMBOL_BITOR            = "bitor"            ;
char * SYMBOL_BOOL             = "bool"             ;
char * SYMBOL_BREAK            = "break"            ;
char * SYMBOL_CASE             = "case"             ;
char * SYMBOL_CATCH            = "catch"            ;
char * SYMBOL_CHAR             = "char"             ;
char * SYMBOL_CHAR16_T         = "char16_t"         ;
char * SYMBOL_CHAR32_T         = "char32_t"         ;
char * SYMBOL_CLASS            = "class"            ;
char * SYMBOL_COMPL            = "compl"            ;
char * SYMBOL_CONCEPT          = "concept"          ;
char * SYMBOL_CONST            = "const"            ;
char * SYMBOL_CONSTEXPR        = "constexpr"        ;
char * SYMBOL_CONST_CAST       = "const_cast"       ;
char * SYMBOL_CONTINUE         = "continue"         ;
char * SYMBOL_DECLTYPE         = "decltype"         ;
char * SYMBOL_DEFAULT          = "default"          ;
char * SYMBOL_DELETE           = "delete"           ;
char * SYMBOL_DO               = "do"               ;
char * SYMBOL_DOUBLE           = "double"           ;
char * SYMBOL_DYNAMIC_CAST     = "dynamic_cast"     ;
char * SYMBOL_ELSE             = "else"             ;
char * SYMBOL_ENUM             = "enum"             ;
char * SYMBOL_EXPLICIT         = "explicit"         ;
char * SYMBOL_EXPORT           = "export"           ;
char * SYMBOL_EXTERN           = "extern"           ;
char * SYMBOL_FALSE            = "false"            ;
char * SYMBOL_FLOAT            = "float"            ;
char * SYMBOL_FOR              = "for"              ;
char * SYMBOL_FRIEND           = "friend"           ;
char * SYMBOL_GOTO             = "goto"             ;
char * SYMBOL_IF               = "if"               ;
char * SYMBOL_INLINE           = "inline"           ;
char * SYMBOL_INT              = "int"              ;
char * SYMBOL_LONG             = "long"             ;
char * SYMBOL_MUTABLE          = "mutable"          ;
char * SYMBOL_NAMESPACE        = "namespace"        ;
char * SYMBOL_NEW              = "new"              ;
char * SYMBOL_NOEXCEPT         = "noexcept"         ;
char * SYMBOL_NOT              = "not"              ;
char * SYMBOL_NOT_EQ           = "not_eq"           ;
char * SYMBOL_NULLPTR          = "nullptr"          ;
char * SYMBOL_OPERATOR         = "operator"         ;
char * SYMBOL_OR               = "or"               ;
char * SYMBOL_OR_EQ            = "or_eq"            ;
char * SYMBOL_PRIVATE          = "private"          ;
char * SYMBOL_PROTECTED        = "protected"        ;
char * SYMBOL_PUBLIC           = "public"           ;
char * SYMBOL_REGISTER         = "register"         ;
char * SYMBOL_REINTERPRET_CAST = "reinterpret_cast" ;
char * SYMBOL_REQUIRES         = "requires"         ;
char * SYMBOL_RETURN           = "return"           ;
char * SYMBOL_SHORT            = "short"            ;
char * SYMBOL_SIGNED           = "signed"           ;
char * SYMBOL_SIZEOF           = "sizeof"           ;
char * SYMBOL_STATIC           = "static"           ;
char * SYMBOL_STATIC_ASSERT    = "static_assert"    ;
char * SYMBOL_STATIC_CAST      = "static_cast"      ;
char * SYMBOL_STRUCT           = "struct"           ;
char * SYMBOL_SWITCH           = "switch"           ;
char * SYMBOL_TEMPLATE         = "template"         ;
char * SYMBOL_THIS             = "this"             ;
char * SYMBOL_THREAD_LOCAL     = "thread_local"     ;
char * SYMBOL_THROW            = "throw"            ;
char * SYMBOL_TRUE             = "true"             ;
char * SYMBOL_TRY              = "try"              ;
char * SYMBOL_TYPEDEF          = "typedef"          ;
char * SYMBOL_TYPEID           = "typeid"           ;
char * SYMBOL_TYPENAME         = "typename"         ;
char * SYMBOL_UNION            = "union"            ;
char * SYMBOL_UNSIGNED         = "unsigned"         ;
char * SYMBOL_USING            = "using"            ;
char * SYMBOL_VIRTUAL          = "virtual"          ;
char * SYMBOL_VOID             = "void"             ;
char * SYMBOL_VOLATILE         = "volatile"         ;
char * SYMBOL_WCHAR_T          = "wchar_t"          ;
char * SYMBOL_WHILE            = "while"            ;
char * SYMBOL_XOR              = "xor"              ;
char * SYMBOL_XOR_EQ           = "xor_eq"           ;
}

typedef struct {
  char * symbol ;
  int          token  ;
} symbol_stack_entry ;

static vector<symbol_stack_entry> symbol_stack ;
static vector<int>                lexer_hack_stack = {0} ;


static unordered_map<char *, int> reserved_word = {
  {SYMBOL_INTEGER         ,TYPE            }, // <-
  {SYMBOL_GROUP           ,TYPE            }, // <-
  {SYMBOL_GROUP0          ,TYPE            }, // <-
  {SYMBOL_GROUP1          ,TYPE            }, // <-
  {SYMBOL_GROUP2          ,TYPE            }, // <-
  {SYMBOL_DUPLEX          ,TYPE            }, // <-
  {SYMBOL_TARGET          ,TYPE            }, // <-
  {SYMBOL_LIST            ,TYPE            }, // <-
  {SYMBOL_FUNCTION        ,FUNCTION        },
  {SYMBOL_MACRO           ,MACRO           },
  {SYMBOL_ALIGNAS         ,ALIGNAS         },
  {SYMBOL_ALIGNOF         ,ALIGNOF         },
  {SYMBOL_ASM             ,ASM             },
  {SYMBOL_AND             ,LOGICALAND      }, // <-
  {SYMBOL_AND_EQ          ,ANDASSIGN       }, // <-
  {SYMBOL_AUTO            ,AUTO            },
  {SYMBOL_BITAND          ,BITAND          }, // <-
  {SYMBOL_BITOR           ,BITOR           }, // <-
  {SYMBOL_BOOL            ,TYPE            }, // <-
  {SYMBOL_BREAK           ,BREAK           },
  {SYMBOL_CASE            ,CASE            },
  {SYMBOL_CATCH           ,CATCH           },
  {SYMBOL_CHAR            ,TYPE            }, // <-
  {SYMBOL_CHAR16_T        ,TYPE            }, // <-
  {SYMBOL_CHAR32_T        ,TYPE            }, // <-
  {SYMBOL_CLASS           ,CLASS           },
  {SYMBOL_COMPL           ,'~'             }, // <-
  {SYMBOL_CONCEPT         ,CONCEPT         },
  {SYMBOL_CONST           ,CONST           },
  {SYMBOL_CONSTEXPR       ,CONSTEXPR       },
  {SYMBOL_CONST_CAST      ,CONST_CAST      },
  {SYMBOL_CONTINUE        ,CONTINUE        },
  {SYMBOL_DECLTYPE        ,DECLTYPE        },
  {SYMBOL_DEFAULT         ,DEFAULT         },
  {SYMBOL_DELETE          ,DELETE          },
  {SYMBOL_DO              ,DO              },
  {SYMBOL_DOUBLE          ,TYPE            }, // <-
  {SYMBOL_DYNAMIC_CAST    ,DYNAMIC_CAST    },
  {SYMBOL_ELSE            ,ELSE            },
  {SYMBOL_ENUM            ,ENUM            },
  {SYMBOL_EXPLICIT        ,EXPLICIT        },
  {SYMBOL_EXPORT          ,EXPORT          },
  {SYMBOL_EXTERN          ,EXTERN          },
  {SYMBOL_FALSE           ,IDENTIFIER      }, // <-
  {SYMBOL_FLOAT           ,TYPE            }, // <-
  {SYMBOL_FOR             ,FOR             },
  {SYMBOL_FRIEND          ,FRIEND          },
  {SYMBOL_GOTO            ,GOTO            },
  {SYMBOL_IF              ,IF              },
  {SYMBOL_INLINE          ,INLINE          },
  {SYMBOL_INT             ,TYPE            }, // <-
  {SYMBOL_LONG            ,TYPE            }, // <-
  {SYMBOL_MUTABLE         ,MUTABLE         },
  {SYMBOL_NAMESPACE       ,NAMESPACE       },
  {SYMBOL_NEW             ,NEW             },
  {SYMBOL_NOEXCEPT        ,NOEXCEPT        },
  {SYMBOL_NOT             ,'!'             }, // <-
  {SYMBOL_NOT_EQ          ,NE              }, // <-
  {SYMBOL_NULLPTR         ,IDENTIFIER      }, // <-
  {SYMBOL_OPERATOR        ,OPERATOR        },
  {SYMBOL_OR              ,LOGICALOR       }, // <-
  {SYMBOL_OR_EQ           ,ORASSIGN        }, // <-
  {SYMBOL_PRIVATE         ,PRIVATE         },
  {SYMBOL_PROHIBITED      ,TYPE            },
  {SYMBOL_PROTECTED       ,PROTECTED       },
  {SYMBOL_PUBLIC          ,PUBLIC          },
  {SYMBOL_REGISTER        ,REGISTER        },
  {SYMBOL_REINTERPRET_CAST,REINTERPRET_CAST},
  {SYMBOL_REQUIRES        ,REQUIRES        },
  {SYMBOL_RETURN          ,RETURN          },
  {SYMBOL_SHORT           ,TYPE            }, // <-
  {SYMBOL_SIGNED          ,TYPE            }, // <-
  {SYMBOL_SIZEOF          ,SIZEOF          },
  {SYMBOL_STATIC          ,STATIC          },
  {SYMBOL_STATIC_ASSERT   ,STATIC_ASSERT   },
  {SYMBOL_STATIC_CAST     ,STATIC_CAST     },
  {SYMBOL_STRUCT          ,STRUCT          },
  {SYMBOL_SWITCH          ,SWITCH          },
  {SYMBOL_TEMPLATE        ,TEMPLATE        },
  {SYMBOL_THIS            ,THIS            },
  {SYMBOL_THREAD_LOCAL    ,THREAD_LOCAL    },
  {SYMBOL_THROW           ,THROW           },
  {SYMBOL_TRUE            ,IDENTIFIER      }, // <-
  {SYMBOL_TRY             ,TRY             },
  {SYMBOL_TYPEDEF         ,TYPEDEF         },
  {SYMBOL_TYPEID          ,TYPEID          },
  {SYMBOL_TYPENAME        ,TYPENAME        },
  {SYMBOL_UNION           ,UNION           },
  {SYMBOL_UNSIGNED        ,TYPE            }, // <-
  {SYMBOL_USING           ,USING           },
  {SYMBOL_VIRTUAL         ,VIRTUAL         },
  {SYMBOL_VOID            ,TYPE            }, // <-
  {SYMBOL_VOLATILE        ,VOLATILE        },
  {SYMBOL_WCHAR_T         ,TYPE            }, // <-
  {SYMBOL_WHILE           ,WHILE           },
  {SYMBOL_XOR             ,BITXOR          }, // <-
  {SYMBOL_XOR_EQ          ,XORASSIGN       }, // <-
} ;










static unordered_map<string, char *> unique_symbol_table = {
  {SYMBOL_INTEGER         ,SYMBOL_INTEGER         },
  {SYMBOL_GROUP           ,SYMBOL_GROUP           },
  {SYMBOL_GROUP0          ,SYMBOL_GROUP0          },
  {SYMBOL_GROUP1          ,SYMBOL_GROUP1          },
  {SYMBOL_GROUP2          ,SYMBOL_GROUP2          },
  {SYMBOL_DUPLEX          ,SYMBOL_DUPLEX          },
  {SYMBOL_TARGET          ,SYMBOL_TARGET          },
  {SYMBOL_LIST            ,SYMBOL_LIST            },
  {SYMBOL_FUNCTION        ,SYMBOL_FUNCTION        },
  {SYMBOL_MACRO           ,SYMBOL_MACRO           },
  {SYMBOL_ALIGNAS         ,SYMBOL_ALIGNAS         },
  {SYMBOL_ALIGNOF         ,SYMBOL_ALIGNOF         },
  {SYMBOL_ASM             ,SYMBOL_ASM             },
  {SYMBOL_AND             ,SYMBOL_AND             },
  {SYMBOL_AND_EQ          ,SYMBOL_AND_EQ          },
  {SYMBOL_AUTO            ,SYMBOL_AUTO            },
  {SYMBOL_BITAND          ,SYMBOL_BITAND          },
  {SYMBOL_BITOR           ,SYMBOL_BITOR           },
  {SYMBOL_BOOL            ,SYMBOL_BOOL            },
  {SYMBOL_BREAK           ,SYMBOL_BREAK           },
  {SYMBOL_CASE            ,SYMBOL_CASE            },
  {SYMBOL_CATCH           ,SYMBOL_CATCH           },
  {SYMBOL_CHAR            ,SYMBOL_CHAR            },
  {SYMBOL_CHAR16_T        ,SYMBOL_CHAR16_T        },
  {SYMBOL_CHAR32_T        ,SYMBOL_CHAR32_T        },
  {SYMBOL_CLASS           ,SYMBOL_CLASS           },
  {SYMBOL_COMPL           ,SYMBOL_COMPL           },
  {SYMBOL_CONCEPT         ,SYMBOL_CONCEPT         },
  {SYMBOL_CONST           ,SYMBOL_CONST           },
  {SYMBOL_CONSTEXPR       ,SYMBOL_CONSTEXPR       },
  {SYMBOL_CONST_CAST      ,SYMBOL_CONST_CAST      },
  {SYMBOL_CONTINUE        ,SYMBOL_CONTINUE        },
  {SYMBOL_DECLTYPE        ,SYMBOL_DECLTYPE        },
  {SYMBOL_DEFAULT         ,SYMBOL_DEFAULT         },
  {SYMBOL_DELETE          ,SYMBOL_DELETE          },
  {SYMBOL_DO              ,SYMBOL_DO              },
  {SYMBOL_DOUBLE          ,SYMBOL_DOUBLE          },
  {SYMBOL_DYNAMIC_CAST    ,SYMBOL_DYNAMIC_CAST    },
  {SYMBOL_ELSE            ,SYMBOL_ELSE            },
  {SYMBOL_ENUM            ,SYMBOL_ENUM            },
  {SYMBOL_EXPLICIT        ,SYMBOL_EXPLICIT        },
  {SYMBOL_EXPORT          ,SYMBOL_EXPORT          },
  {SYMBOL_EXTERN          ,SYMBOL_EXTERN          },
  {SYMBOL_FALSE           ,SYMBOL_FALSE           },
  {SYMBOL_FLOAT           ,SYMBOL_FLOAT           },
  {SYMBOL_FOR             ,SYMBOL_FOR             },
  {SYMBOL_FRIEND          ,SYMBOL_FRIEND          },
  {SYMBOL_GOTO            ,SYMBOL_GOTO            },
  {SYMBOL_IF              ,SYMBOL_IF              },
  {SYMBOL_INLINE          ,SYMBOL_INLINE          },
  {SYMBOL_INT             ,SYMBOL_INT             },
  {SYMBOL_LONG            ,SYMBOL_LONG            },
  {SYMBOL_MUTABLE         ,SYMBOL_MUTABLE         },
  {SYMBOL_NAMESPACE       ,SYMBOL_NAMESPACE       },
  {SYMBOL_NEW             ,SYMBOL_NEW             },
  {SYMBOL_NOEXCEPT        ,SYMBOL_NOEXCEPT        },
  {SYMBOL_NOT             ,SYMBOL_NOT             },
  {SYMBOL_NOT_EQ          ,SYMBOL_NOT_EQ          },
  {SYMBOL_NULLPTR         ,SYMBOL_NULLPTR         },
  {SYMBOL_OPERATOR        ,SYMBOL_OPERATOR        },
  {SYMBOL_OR              ,SYMBOL_OR              },
  {SYMBOL_OR_EQ           ,SYMBOL_OR_EQ           },
  {SYMBOL_PRIVATE         ,SYMBOL_PRIVATE         },
  {SYMBOL_PROHIBITED      ,SYMBOL_PROHIBITED      },
  {SYMBOL_PROTECTED       ,SYMBOL_PROTECTED       },
  {SYMBOL_PUBLIC          ,SYMBOL_PUBLIC          },
  {SYMBOL_REGISTER        ,SYMBOL_REGISTER        },
  {SYMBOL_REINTERPRET_CAST,SYMBOL_REINTERPRET_CAST},
  {SYMBOL_REQUIRES        ,SYMBOL_REQUIRES        },
  {SYMBOL_RETURN          ,SYMBOL_RETURN          },
  {SYMBOL_SHORT           ,SYMBOL_SHORT           },
  {SYMBOL_SIGNED          ,SYMBOL_SIGNED          },
  {SYMBOL_SIZEOF          ,SYMBOL_SIZEOF          },
  {SYMBOL_STATIC          ,SYMBOL_STATIC          },
  {SYMBOL_STATIC_ASSERT   ,SYMBOL_STATIC_ASSERT   },
  {SYMBOL_STATIC_CAST     ,SYMBOL_STATIC_CAST     },
  {SYMBOL_STRUCT          ,SYMBOL_STRUCT          },
  {SYMBOL_SWITCH          ,SYMBOL_SWITCH          },
  {SYMBOL_TEMPLATE        ,SYMBOL_TEMPLATE        },
  {SYMBOL_THIS            ,SYMBOL_THIS            },
  {SYMBOL_THREAD_LOCAL    ,SYMBOL_THREAD_LOCAL    },
  {SYMBOL_THROW           ,SYMBOL_THROW           },
  {SYMBOL_TRUE            ,SYMBOL_TRUE            },
  {SYMBOL_TRY             ,SYMBOL_TRY             },
  {SYMBOL_TYPEDEF         ,SYMBOL_TYPEDEF         },
  {SYMBOL_TYPEID          ,SYMBOL_TYPEID          },
  {SYMBOL_TYPENAME        ,SYMBOL_TYPENAME        },
  {SYMBOL_UNION           ,SYMBOL_UNION           },
  {SYMBOL_UNSIGNED        ,SYMBOL_UNSIGNED        },
  {SYMBOL_USING           ,SYMBOL_USING           },
  {SYMBOL_VIRTUAL         ,SYMBOL_VIRTUAL         },
  {SYMBOL_VOID            ,SYMBOL_VOID            },
  {SYMBOL_VOLATILE        ,SYMBOL_VOLATILE        },
  {SYMBOL_WCHAR_T         ,SYMBOL_WCHAR_T         },
  {SYMBOL_WHILE           ,SYMBOL_WHILE           },
  {SYMBOL_XOR             ,SYMBOL_XOR             },
  {SYMBOL_XOR_EQ          ,SYMBOL_XOR_EQ          },
} ;

static vector<int> frame_stack ;

extern "C" {


  char * unique_symbol(char * symbol){
    string   s = symbol ;
    char * & r = unique_symbol_table[s] ;
    if(!r) r = strdup(symbol) ;
//    fprintf(stderr, "unique_symbol(): %s (%p)\n", r, r);
    return r ;
  }

  char * new_symbol(char * symbol){
    static unordered_map<string, int> table ;
//    return unique_symbol((char *) ("$" + string(symbol) + "$" + to_string(table[symbol]++)).c_str());
    return unique_symbol((char *) (string(symbol) + "$" + to_string(table[symbol]++)).c_str());
  }

  int is_reserved_word(char * symbol){
    char * u     = unique_symbol(symbol) ;
    auto         r     = reserved_word.find(u) ;
    return r != reserved_word.end() ;
  }

  int symbol_to_token(char * symbol){
    int          token = IDENTIFIER ;
    char * u     = unique_symbol(symbol) ;
    auto         r     = reserved_word.find(u) ;
    int & expected_identifier_flag = lexer_hack_stack.back() ;
//    printf("expected_identifier_flag = %d\n", expected_identifier_flag) ;
    if(r != reserved_word.end()){
      token = r->second ;
    }else{
      for(int i = symbol_stack.size()-1; i>=0; i--){
        if(u == symbol_stack[i].symbol){
	  token = symbol_stack[i].token ;
          if((token == TYPE) && expected_identifier_flag){
	    token = IDENTIFIER ;
          }
	  break ;
	}
      }
    }
//    print_stack();
//    printf("symbol_to_token(%s) = %d\n",symbol,token) ;
    if(token == IDENTIFIER) expected_identifier_flag = 0 ;
    return token ;
  }

  int symbol_to_stack_pointer(char * symbol){
    char * u = unique_symbol(symbol) ;
    auto r = reserved_word.find(u) ;
    if(r != reserved_word.end()){
      return -2 ;
    }else{
      for(int i = symbol_stack.size()-1; i>=0; i--){
        if(u == symbol_stack[i].symbol) return i ;
      }
      return -1 ;
    }
  }

  int symbol_stack_pointer(){ return symbol_stack.size() ; }

  void symbol_stack_push(char * symbol, int token){
    symbol_stack_entry e = {unique_symbol(symbol), token} ;
    symbol_stack.push_back(e);
  }

  void symbol_stack_restore(int n){
    while(symbol_stack.size() > n) symbol_stack.pop_back() ;
  }

  int current_frame(){
    if(frame_stack.size()>0){
      return frame_stack.back() ;
    }else{
      return 0 ;
    }
  }

  void frame_stack_push(){
    frame_stack.push_back(symbol_stack_pointer()) ;
  }

  void frame_stack_pop(){
    if(frame_stack.size()>0){
      symbol_stack_restore(frame_stack.back()) ;
      frame_stack.pop_back();
    }
  }

  void lexer_hack_stack_push(){
    lexer_hack_stack.push_back(0) ;
  }

  void lexer_hack_stack_set(int n){
    if(lexer_hack_stack.size()>0){
      lexer_hack_stack.back() = n ;
    }
  }

  void lexer_hack_stack_pop(){
    if(lexer_hack_stack.size()>0){
      lexer_hack_stack.pop_back();
    }
  }

  void print_stack(){
    cout << "-------------" << endl ;
    for(int i = symbol_stack.size()-1; i>=0; i--){
      cout << i << ":" << symbol_stack[i].symbol << "," << symbol_stack[i].token << endl ;
    }
    cout << "-------------" << endl ;
    for(int i = frame_stack.size()-1; i>=0; i--){
      cout << frame_stack[i] << endl ;
    }
  }

}
