/*

   Copyright (c) 2017 NTT corp. - All Rights Reserved

   This file is part of opcount which is released under Software License
   Agreement for Evaluation. See file LICENSE.pdf for full license details.

 */

#ifndef RESERVED_WORD_H
#define RESERVED_WORD_H
#ifdef __cplusplus
extern "C" {
#endif

extern int          expect_identifier_flag  ;

extern char * unique_symbol           (char * symbol);
extern char * new_symbol              (char * symbol);
extern int          is_reserved_word        (char * symbol);
extern int          symbol_to_token         (char * symbol);
extern int          symbol_to_stack_pointer (char * symbol);
extern int          symbol_stack_pointer    ();
extern void         symbol_stack_push       (char * symbol, int token);
extern void         symbol_stack_restore    (int n);
extern int          current_frame           ();
extern void         frame_stack_push        ();
extern void         frame_stack_pop         ();
extern void         lexer_hack_stack_push   ();
extern void         lexer_hack_stack_set    (int n);
extern void         lexer_hack_stack_pop    ();
extern void         print_stack             ();

extern char * SYMBOL_INTEGER          ;
extern char * SYMBOL_GROUP            ;
extern char * SYMBOL_GROUP0           ;
extern char * SYMBOL_GROUP1           ;
extern char * SYMBOL_DUPLEX           ;
extern char * SYMBOL_TARGET           ;
extern char * SYMBOL_LIST             ;
extern char * SYMBOL_FUNCTION         ;
extern char * SYMBOL_MACRO            ;
extern char * SYMBOL_ALIGNAS          ;
extern char * SYMBOL_ALIGNOF          ;
extern char * SYMBOL_ASM              ;
extern char * SYMBOL_AND              ;
extern char * SYMBOL_AND_EQ           ;
extern char * SYMBOL_AUTO             ;
extern char * SYMBOL_BITAND           ;
extern char * SYMBOL_BITOR            ;
extern char * SYMBOL_BOOL             ;
extern char * SYMBOL_BREAK            ;
extern char * SYMBOL_CASE             ;
extern char * SYMBOL_CATCH            ;
extern char * SYMBOL_CHAR             ;
extern char * SYMBOL_CHAR16_T         ;
extern char * SYMBOL_CHAR32_T         ;
extern char * SYMBOL_CLASS            ;
extern char * SYMBOL_COMPL            ;
extern char * SYMBOL_CONCEPT          ;
extern char * SYMBOL_CONST            ;
extern char * SYMBOL_CONSTEXPR        ;
extern char * SYMBOL_CONST_CAST       ;
extern char * SYMBOL_CONTINUE         ;
extern char * SYMBOL_DECLTYPE         ;
extern char * SYMBOL_DEFAULT          ;
extern char * SYMBOL_DELETE           ;
extern char * SYMBOL_DO               ;
extern char * SYMBOL_DOUBLE           ;
extern char * SYMBOL_DYNAMIC_CAST     ;
extern char * SYMBOL_ELSE             ;
extern char * SYMBOL_ENUM             ;
extern char * SYMBOL_EXPLICIT         ;
extern char * SYMBOL_EXPORT           ;
extern char * SYMBOL_EXTERN           ;
extern char * SYMBOL_FALSE            ;
extern char * SYMBOL_FLOAT            ;
extern char * SYMBOL_FOR              ;
extern char * SYMBOL_FRIEND           ;
extern char * SYMBOL_GOTO             ;
extern char * SYMBOL_IF               ;
extern char * SYMBOL_INLINE           ;
extern char * SYMBOL_INT              ;
extern char * SYMBOL_LONG             ;
extern char * SYMBOL_MUTABLE          ;
extern char * SYMBOL_NAMESPACE        ;
extern char * SYMBOL_NEW              ;
extern char * SYMBOL_NOEXCEPT         ;
extern char * SYMBOL_NOT              ;
extern char * SYMBOL_NOT_EQ           ;
extern char * SYMBOL_NULLPTR          ;
extern char * SYMBOL_OPERATOR         ;
extern char * SYMBOL_OR               ;
extern char * SYMBOL_OR_EQ            ;
extern char * SYMBOL_PRIVATE          ;
extern char * SYMBOL_PROHIBITED       ;
extern char * SYMBOL_PROTECTED        ;
extern char * SYMBOL_PUBLIC           ;
extern char * SYMBOL_REGISTER         ;
extern char * SYMBOL_REINTERPRET_CAST ;
extern char * SYMBOL_REQUIRES         ;
extern char * SYMBOL_RETURN           ;
extern char * SYMBOL_SHORT            ;
extern char * SYMBOL_SIGNED           ;
extern char * SYMBOL_SIZEOF           ;
extern char * SYMBOL_STATIC           ;
extern char * SYMBOL_STATIC_ASSERT    ;
extern char * SYMBOL_STATIC_CAST      ;
extern char * SYMBOL_STRUCT           ;
extern char * SYMBOL_SWITCH           ;
extern char * SYMBOL_TEMPLATE         ;
extern char * SYMBOL_THIS             ;
extern char * SYMBOL_THREAD_LOCAL     ;
extern char * SYMBOL_THROW            ;
extern char * SYMBOL_TRUE             ;
extern char * SYMBOL_TRY              ;
extern char * SYMBOL_TYPEDEF          ;
extern char * SYMBOL_TYPEID           ;
extern char * SYMBOL_TYPENAME         ;
extern char * SYMBOL_UNION            ;
extern char * SYMBOL_UNSIGNED         ;
extern char * SYMBOL_USING            ;
extern char * SYMBOL_VIRTUAL          ;
extern char * SYMBOL_VOID             ;
extern char * SYMBOL_VOLATILE         ;
extern char * SYMBOL_WCHAR_T          ;
extern char * SYMBOL_WHILE            ;
extern char * SYMBOL_XOR              ;
extern char * SYMBOL_XOR_EQ           ;

#ifdef __cplusplus
}
#endif
#endif /* RESERVED_WORD_H */
