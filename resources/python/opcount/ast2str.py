#! /usr/bin/python
#
#   Copyright (c) 2019 NTT corp. - All Rights Reserved
#
#   This file is part of opcount which is released under Software License
#   Agreement for Evaluation. See file LICENSE.pdf for full license details.
#

from __future__ import print_function
import sys
import ast

try:
    range = xrange
except NameError:
    pass

class Ast2Str:
    table = [
        'Module', # 'Interactive', 'Expression', 'Suite',
# ====================================================================
        'FunctionDef', 'ClassDef', 'Return', 'Delete', 'Assign',
        'AugAssign', 'Print', 'For', 'While', 'If', 'With', 'Raise',
        'TryExcept', 'TryFinally', 'Assert', 'Import', 'ImportFrom',
        'Exec', 'Global', 'Expr', 'Pass', 'Break', 'Continue',
# ====================================================================
        'BoolOp', 'BinOp', 'UnaryOp', 'Lambda', 'IfExp', 'Dict', 'Set',
        'ListComp', 'SetComp', 'DictComp', 'GeneratorExp', 'Yield',
        'Compare', 'Call', 'Repr', 'Num', 'Str', 'Attribute', 'Subscript',
        'Name', 'List', 'Tuple',
# ====================================================================
#        'Load', 'Store', 'Del', 'AugLoad', 'AugStore', 'Param',
# ====================================================================
        'Ellipsis', 'Slice', 'ExtSlice', 'Index',
        'And', 'Or',
        'Add', 'Sub', 'Mult', 'Div', 'Mod', 'Pow', 'LShift', 'RShift', 'BitOr', 'BitXor', 'BitAnd', 'FloorDiv',
        'Invert', 'Not', 'UAdd', 'USub', 
        'Eq', 'NotEq', 'Lt', 'LtE', 'Gt', 'GtE', 'Is', 'IsNot', 'In', 'NotIn',
        'comprehension', 
        'ExceptHandler', 'arguments', 'keyword', 'alias' ]

    def __init__(self):
        self.indent = 0

    def Indent(self):
        return (' ' * (4 * self.indent))

    def visit(self, node):
        if isinstance(node, ast.AST):
            name = type(node).__name__
            if name in Ast2Str.table:
                func_name = 'visit_{x}'.format(x=name)
                func = getattr(self, func_name, None)
                if callable(func):
                    return func(node)
                return self.generic_visit(node)
        elif isinstance(node, list):
            s = ''
            for x in node:
                s += self.visit(x)
            return s
        else:
            return ''

    def generic_visit(self, node):
        s = ''
        if hasattr(node, '_fields'):
            for f in node._fields:
                child = getattr(node, f)
                s += self.visit(child)
        return s

    def enter(self, node, keyword=''):
        s = ''
        if keyword != '':
            s += self.Indent()
            s += keyword
        s += ':\n'
        self.indent += 1
        s += self.visit(node)
        self.indent -= 1
        return s

    def list_expand(self, list_, **kwargs):
        s = ''
        if kwargs.has_key('first'): first = kwargs['first']
        else: first = True
        if kwargs.has_key('sep'): sep = kwargs['sep']
        else: sep = ','
        if kwargs.has_key('prefix'): prefix = kwargs['prefix']
        else: prefix = ''
        for x in list_:
            if first: first = False
            else: s += sep
            s += prefix
            s += self.visit(x)
        return s

    def statement(self, s):
        t  = ''
        t += self.Indent()
        t += s
        t += '\n'
        return t

# ====================================================================
# module
# ====================================================================

    def visit_Module(self, node):
        s = ''
        for stmt in node.body:
            s += self.visit(stmt)
        return s

#    'Interactive', 'Expression', 'Suite'
#
# ====================================================================
# statement
# ====================================================================

    def visit_FunctionDef(self, node):
        d  = ''
        for decorator in node.decorator_list:
            d += self.Indent()
            d += '@'
            d += self.visit(decorator)
            d += '\n'
        s  = 'def ' + node.name + '('
        s += self.visit(node.args)
        s += ')'
        s += self.enter(node.body)
        return d + self.statement(s)

    def visit_ClassDef(self, node):
        d  = ''
        for decorator in node.decorator_list:
            d += self.Indent()
            d += '@'
            d += self.visit(decorator)
            d += '\n'
        s  = 'class ' + node.name
        if node.bases:
            s += '('
            first = True
            for base in node.bases:
                if first: first = False
                else: s += ','
                s += self.visit(base)
            s += ')'
        s += self.enter(node.body)
        return d + self.statement(s)

    def visit_Return(self, node):
        s = 'return'
        if node.value:
            s += ' '
            s += self.visit(node.value)
        return self.statement(s)

    def visit_Delete(self, node):
        s  = 'del '
        s += self.list_expand(node.targets)
        return self.statement(s)

    def visit_Assign(self, node):
        s  = ''
        for target in node.targets:
            s += self.visit(target)
            s += '='
        s += self.visit(node.value)
        return self.statement(s)

    def visit_AugAssign(self, node):
        s  = ''
        s += self.visit(node.target)
        s += self.visit(node.op)
        s += '='
        s += self.visit(node.value)
        return self.statement(s)

    def visit_Print(self, node):
        s  = 'print '
        first = True
        if node.dest:
            s += '>>'
            s += self.visit(node.dest)
            first = False
        for v in node.values:
            if first: first = False
            else: s+= ','
            s += self.visit(v)
        if not node.nl:
            s += ','
        return self.statement(s)

    def visit_For(self, node):
        s  = 'for '
        s += self.visit(node.target)
        s += ' in '
        s += self.visit(node.iter)
        s += self.enter(node.body)
        if node.orelse:
            s += self.enter(node.orelse,'else')
        return self.statement(s)

    def visit_While(self, node):
        s  = 'while '
        s += self.visit(node.test)
        s += self.enter(node.body)
        if node.orelse:
            s += self.enter(node.orelse,'else')
        return self.statement(s)

    def visit_If(self, node):
        s  = 'if '
        s += self.visit(node.test)
        s += self.enter(node.body)
        while (node.orelse and len(node.orelse) == 1 and
               isinstance(node.orelse[0], ast.If)):
            node = node.orelse[0]
            t  = 'elif '
            t += self.visit(node.test)
            s += self.enter(node.body,t)
        if node.orelse:
            s += self.enter(node.orelse,'else')
        return self.statement(s)

    def visit_With(self, node):
        s  = 'with '
        s += self.visit(node.context_expr)
        if node.optional_vars:
            s += ' as '
            s += self.visit(node.optional_vars)
        s += self.enter(node.body)
        return self.statement(s)

    def visit_Raise(self, node):
        s = 'raise '
        if node.type:
            s += self.visit(node.type)
        if node.inst:
            s += ','
            s += self.visit(node.inst)
        if node.tback:
            s += ','
            s += self.visit(node.tback)
        return self.statement(s)

    def visit_TryExcept(self, node):
        s = 'try'
        s += self.enter(node.body)
        for except_ in node.handlers:
            s += self.visit(except_)
        if node.orelse:
            s += self.enter(node.orelse,'else')
        return self.statement(s)

    def visit_TryFinally(self, node):
        s = ''
        if len(node.body) == 1 and isinstance(node.body[0], ast.TryExcept):
            s += self.visit(node.body)
        else:
            s += 'try'
            s += self.enter(node.body)
        s += self.enter(node.finalbody,'finally')
        return self.statement(s)

    def visit_Assert(self, node):
        s  = 'assert '
        s += self.visit(node.test)
        if node.msg:
            s += ','
            s += self.visit(node.msg)
        return self.statement(s)

    def visit_Import(self, node):
        s  = 'import '
        s += self.list_expand(node.names)
        return self.statement(s)

    def visit_ImportFrom(self, node):
        s  = 'from '
        s += ('.' * node.level)
        if node.module:
            s += node.module
        s += ' import '
        s += self.list_expand(node.names)
        return self.statement(s)

    def visit_Exec(self, node):
        s  = 'exec '
        s += self.visit(node.body)
        if node.globals:
            s += ' in '
            s += self.visit(node.globals)
        if node.locals:
            s += ','
            s += self.visit(node.locals)
        return self.statement(s)

    def visit_Global(self, node):
        s = 'global '
        s += self.list_expand(node.names)
        return self.statement(s)

    def visit_Expr(self, node):
        s = self.visit(node.value)
        return self.statement(s)

    def visit_Pass(self, node): return self.statement('pass')
    def visit_Break(self, node): return self.statement('break')
    def visit_Continue(self, node): return self.statement('continue')

# ====================================================================
# expression
# ====================================================================

    def visit_BoolOp(self, node):
        op = self.visit(node.op)
        s  = '('
        first = True
        for v in node.values:
          if first: first = False
          else: s += op
          s += self.visit(v)
        s += ')'
        return s

    def visit_BinOp(self, node):
        return '(' + self.visit(node.left)          \
                   + self.visit(node.op)            \
                   + self.visit(node.right) + ')'

    def visit_UnaryOp(self, node):
        return '(' + self.visit(node.op)            \
                   + self.visit(node.operand) + ')'

    def visit_Lambda(self, node):
        return '(lambda ' + self.visit(node.args) + ':' \
                          + self.visit(node.body) + ')'

    def visit_IfExp(self, node):
        return '(' + self.visit(node.body) + ' if ' \
                   + self.visit(node.test) + ' else ' \
                   + self.visit(node.orelse) + ')'

    def visit_Dict(self, node):
        s  = '{'
        first = True
        for i in range(len(node.keys)):
            if first: first = False
            else: s += ','
            s += node.keys[i] + ':' + node.values[i]
        s += '}'
        return s

    def visit_Set(self, node):
        return '{' + self.list_expand(node.elts) + '}'

    def visit_ListComp(self, node):
        s  = '['
        s += self.visit(node.elt)
        for gen in node.generators:
            s += self.visit(gen)
        s += ']'
        return s

    def visit_SetComp(self, node):
        s  = '{'
        s += self.visit(node.elt)
        for gen in node.generators:
            s += self.visit(gen)
        s += '}'
        return s

    def visit_DictComp(self, node):
        s  = '{'
        s += self.visit(node.key)
        s += ':'
        s += self.visit(node.value)
        for gen in node.generators:
            s += self.visit(gen)
        s += '}'
        return s
 
    def visit_GeneratorExp(self, node):
        s  = '('
        s += self.visit(node.elt)
        for gen in node.generators:
            s += self.visit(gen)
        s += ')'
        return s

    def visit_Yield(self, node):
        s = '(yield'
        if node.value:
            s += ' '
            s += self.visit(node.value)
        return s + ')'

    def visit_Compare(self, node):
        s  = '('
        s += self.visit(node.left)
        for i in range(len(node.ops)):
            s += self.visit(node.ops[i])
            s += self.visit(node.comparators[i])
        s += ')'
        return s

    def visit_Call(self, node):
        s  = self.visit(node.func)
        s += '('
        s += self.list_expand(node.args)
        first = (len(node.args)==0)
        s += self.list_expand(node.keywords, first=first)
        first = first and (len(node.keywords)==0)
        if node.starargs:
            s += self.list_expand([node.starargs], first=first, prefix='*')
            first = False
        if node.kwargs:
            s += self.list_expand([node.kwargs], first=first, prefix='**')
        s += ')'
        return s

    def visit_Repr(self, node):
        return '`' + self.visit(node.value) + '`'

    def visit_Num(self, node):
        return str(node.n)

    def visit_Str(self, node):
        return repr(node.s)

    def visit_Attribute(self, node):
        s  = self.visit(node.value)
        s += '.'
        s += node.attr
        return s

    def visit_Subscript(self, node):
        s  = self.visit(node.value)
        s += '['
        s += self.visit(node.slice)
        s += ']'
        return s

    def visit_Name(self, node):
        return node.id

    def visit_List(self, node):
        return '[' + self.list_expand(node.elts) + ']'

    def visit_Tuple(self, node):
        if len(node.elts) == 1:
            return '(' + self.visit(node.elts[0]) + ',)'
        else:
            return '(' + self.list_expand(node.elts) + ')'
# ===================================================================
# expr_context
# ===================================================================
#        'Load', 'Store', 'Del', 'AugLoad', 'AugStore', 'Param',
# ===================================================================
# slice
# ===================================================================

    def visit_Ellipsis(self, node):
        return '...'

    def visit_Slice(self, node):
        s  = ''
        if node.lower: s += self.visit(node.lower)
        s += ':'
        if node.upper: s += self.visit(node.upper)
        if node.step:  s += ':' + self.visit(node.step)
        return s

    def visit_ExtSlice(self, node):
        return self.list_expand(node.dims)

    def visit_Index(self, node):
        return self.visit(node.value)

# ===================================================================
# boolop
# ===================================================================

    def visit_Or(self, node): return ' or '
    def visit_And(self, node): return ' and '

# ===================================================================
# operator
# ===================================================================

    def visit_Add(self, node): return '+'
    def visit_Sub(self, node): return '-'
    def visit_Mult(self, node): return '*'
    def visit_Div(self, node): return '/'
    def visit_Mod(self, node): return '%'
    def visit_LShift(self, node): return '<<'
    def visit_RShift(self, node): return '>>'
    def visit_BitOr(self, node): return '|'
    def visit_BitXor(self, node): return '^'
    def visit_BitAnd(self, node): return '&'
    def visit_FloorDiv(self, node): return '//'
    def visit_Pow(self, node): return '**'

# ===================================================================
# unaryop
# ===================================================================

    def visit_Invert(self, node): return '~'
    def visit_Not(self, node): return 'not '
    def visit_UAdd(self, node): return '+'
    def visit_USub(self, node): return '-'

# ===================================================================
# compop
# ===================================================================

    def visit_Eq(self, node): return '=='
    def visit_NotEq(self, node): return '!='
    def visit_Lt(self, node): return '<'
    def visit_LtE(self, node): return '<='
    def visit_Gt(self, node): return '>'
    def visit_GtE(self, node): return '>='
    def visit_Is(self, node): return ' is '
    def visit_IsNot(self, node): return ' is not '
    def visit_In(self, node): return ' in '
    def visit_NotIn(self, node): return ' not in '

# ===================================================================
# comprehension
# ===================================================================

    def visit_comprehension(self, node):
        s  = ' for '
        s += self.visit(node.target)
        s += ' in '
        s += self.visit(node.iter)
        for if_clause in node.ifs:
            s += ' if '
            s += self.visit(if_clause)
        return s

# ===================================================================
# others
# ===================================================================

    def visit_ExceptHandler(self, node):
        s = 'except'
        if node.type:
            s += ' '
            s += self.visit(node.type)
        if node.name:
            s += ' as '
            s += self.visit(node.name)
        s += self.enter(node.body)
        return self.statement(s) # <- for convenience

    def visit_arguments(self, node):
        a = len(node.args)
        d = len(node.defaults)
        first = True
        s = ''
        for i in range(a-d):
            if first: first=False
            else: s += ','
            s += self.visit(node.args[i])
        for i in range(d):
            if first: first=False
            else: s += ','
            s += self.visit(node.args[a-d+i])
            s += '='
            s += self.visit(node.defaults[i])
        if node.vararg:
            if first: first = False
            else: s += ','
            s += '*'
            s += node.vararg
        if node.kwarg:
            if first: first = False
            else: s += ','
            s += '**'
            s += node.kwarg
        return s

    def visit_keyword(self, node):
        return node.arg + '=' + self.visit(node.value)

    def visit_alias(self, node):
        s = node.name
        if node.asname:
            s += ' as '
            s += node.asname
        return s

def ast2str_raw(node):
    return Ast2Str().visit(node)

def main():
    data = sys.stdin.read()
    sys.stdin.close()
    try:
        A = ast.parse(data)
    except SyntaxError as E:
        message = 'syntactic error: line {x}, offset {y}, text {s}'.format(x=E.lineno,y=E.offset,s=E.text)
        print(message, file=sys.stderr)
        sys.exit(1)  # syntactic error
    print(ast2str_raw(A))

if __name__ == "__main__":
    main()
