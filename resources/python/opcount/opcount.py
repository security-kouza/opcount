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
from ast2str import ast2str_raw
from sympy import var as formula
from sympy import var
from sympy import poly

try:
    range = xrange
except NameError:
    pass

def ast2str(A):
    code = ast2str_raw(A)
    code = code.replace('^','^^')
    code = code.replace('**','^')
    return code

def location(node):
    return "line " + str(node.lineno) + ", column " + str(node.col_offset)

def semantic_error(node, message):
    print("semantic error at ", location(node), ": ", message, file=sys.stderr)
    sys.exit(1)

def warning(node, message):
    print("warning at ", location(node), ": ", message, file=sys.stderr)

class reduced_:
    def __init__(self, ** kwargs):
        self.__dict__.update(kwargs)
        self._fields = kwargs.keys()
        self._kwargs = kwargs

    def __str__(self):
        s = ''
        for key in self._fields:
            if s != '':
              s += ', '
            s += '{x}={y}'.format(x=key,y=self.__dict__[key])
        return s

    def __repr__(self):
        return self.__str__()

def SetAttr(node, attr, value):
#    if value == 'T':
#        print("??:",node, attr, value);
    try:
        setattr(node, attr, value)
    except AttributeError:
        pass

def reduced(type_, node, **kwargs):
    kwargs['type_'] = type_
    kwargs['node']  = node
    SetAttr(node,'InferredType',type_)
    return reduced_(**kwargs)

def Reduced(type_, node, **kwargs):
    kwargs['type_'] = type_
    kwargs['node']  = node
    return reduced_(**kwargs)

def Total(X): return getattr(X,'total',0)
def Batch(X): return getattr(X,'batch',0)
def Coeff(poly, mono):
    if hasattr(poly,'coeff'):
        return poly.coeff(mono)
    else:
        return 0

def transfer_batch(X):
    X.batch = Batch(X)
    e_coeff = Coeff(X.batch,var('e'))
    b_coeff = Coeff(X.batch,var('b'))
    p_coeff = Coeff(X.batch,var('p'))
    if e_coeff != 0:
        s = X.type_[5:] + '_batch(' + str(e_coeff) + ';' \
                                    + str(b_coeff) + ')'
        X.total = formula(s) + X.total
    if p_coeff != 0:
        s = 'pairing_batch(' + str(p_coeff) + ')'
        X.total  = formula(s) + X.total
        X.total -= (p_coeff - 1) * formula('TARGET_mul')
    X.batch = 0

def transfer_bitexp(X, n):
    X.batch = Batch(X)
    e_coeff = Coeff(X.batch,var('e'))
    b_coeff = Coeff(X.batch,var('b'))
    p_coeff = Coeff(X.batch,var('p'))
    if e_coeff != 0:
        s = X.type_[5:] +                   \
            '_bitexp(' + str(n)       + ';' \
                       + str(e_coeff) + ';' \
                       + str(b_coeff) + ')'
        X.total = formula(s) + X.total
    if p_coeff != 0:
        s = "pairing_batch(" + str(p_coeff) + ")"
        X.total  = formula(s) + X.total
        X.total -= (p_coeff - 1) * formula("TARGET_mul")
    X.batch = 0

def InferredType(node, unknown='TYPE_UNKNOWN0'):
    if hasattr(node,'InferredType'):
        return node.InferredType
    else:
        return unknown

def InferredValue(node, default_value=0):
    if hasattr(node,'InferredValue'):
        return node.InferredValue
    else:
        return default_value

# -------------------------------------------------------------------------
#
# -------------------------------------------------------------------------

class NodeEditor:
    table = [ 'Module', 'Interactive', 'Expression', 'Suite',          \
              'FunctionDef', 'ClassDef', 'Return', 'Delete',           \
              'Assign', 'AugAssign', 'Print', 'For', 'While',          \
              'If', 'With', 'Raise', 'TryExcept', 'TryFinally',        \
              'Assert', 'Import', 'ImportFrom', 'Exec', 'Global',      \
              'Expr', 'Pass', 'Break', 'Continue', 'BoolOp', 'BinOp',  \
              'UnaryOp', 'Lambda', 'IfExp', 'Dict', 'Set', 'ListComp', \
              'SetComp', 'DictComp', 'GeneratorExp', 'Yield',          \
              'Compare', 'Call', 'Repr', 'Num', 'Str', 'Attribute',    \
              'Subscript', 'Name', 'List', 'Tuple', 'Load', 'Store',   \
              'Del', 'AugLoad', 'AugStore', 'Param', 'Ellipsis',       \
              'Slice', 'ExtSlice', 'Index', 'And', 'Or', 'Add', 'Sub', \
              'Mult', 'Div', 'Mod', 'Pow', 'LShift', 'RShift',         \
              'BitOr', 'BitXor', 'BitAnd', 'FloorDiv', 'Invert',       \
              'Not', 'UAdd', 'USub', 'Eq', 'NotEq', 'Lt', 'LtE', 'Gt', \
              'GtE', 'Is', 'IsNot', 'In', 'NotIn', 'ExceptHandler',    \
              'comprehension', 'attributes', 'arguments', 'keyword',   \
              'alias' ]

    def __init__(self):
        self.indent = 0

    def Print(self, *args, **kwargs):
        if kwargs.has_key('begin'):
            print(kwargs['begin'], end='')
        else:
            print(' ' * (4 * self.indent), end='')

        for a in args:
            print(a, end='')

        if kwargs.has_key('end'):
            print(end=kwargs['end'])
        else:
            print()

    def generic_visit(self, node):
        first = True
        first_child = None
        if hasattr(node, '_fields'):
            self.indent+=1
            for f in node._fields:
                child = getattr(node, f)
                if first:
                    first = False
                    first_child = self.visit(child)
                else:
                    self.visit(child)
            self.indent-=1
        if first:
            return reduced('TYPE_UNKNOWN1', node)
        else:
            return first_child

    def visit(self, node):
        if isinstance(node, ast.AST):
            name = type(node).__name__
            if name in NodeEditor.table:
                func_name = "visit_{x}".format(x=name)
                func = getattr(self, func_name, None)
                if callable(func):
                    return func(node)
            return self.generic_visit(node)
        self.indent+=1
        type_ = type(node)
        if type_ in [list, tuple, set]:
            for child in node:
                self.visit(child)
        elif type_ == dict:
            for key in node.keys():
                self.visit(key)
                self.visit(node[key])
        elif (type_ == int) or (type_ == long):
            pass
        elif type_ == str:
            pass
        elif (node == None) or \
             (node == True) or \
             (node == False):
            pass
        else:
            self.Print("Unkown node type:",type(node)," ",node)
        self.indent-=1
        return reduced('TYPE_UNKNOWN2', node)

# ========================================================================
# declaration_stack
# ========================================================================

class declaration_stack_entry:
    def __init__(self, symbol, type_, node = None, value = None):
        self.symbol = symbol # type : str
        self.type_  = type_  # type : str
        self.node   = node   # type : ast
        self.value  = value  # type : ast
    def __str__(self):
        return "[{x},{y},{z},{w}]".format(\
          x=self.symbol,y=self.type_,z=self.node,w=self.value)

declaration_frame_stack = [0]

declaration_stack = [
    declaration_stack_entry('Integer','TYPE_INTEGER'),
    declaration_stack_entry('Group','TYPE_GROUP'),
    declaration_stack_entry('Group0','TYPE_GROUP0'),
    declaration_stack_entry('Group1','TYPE_GROUP1'),
    declaration_stack_entry('Group2','TYPE_GROUP2'),
    declaration_stack_entry('Target','TYPE_TARGET'),
    declaration_stack_entry('e','TYPE_TARGET')
]

def declaration_stack_pointer():
    return len(declaration_stack)
def declaration_frame_stack_pointer():
    return len(declaration_frame_stack)
def declaration_current_frame():
    return declaration_frame_stack[-1]
def declaration_frame_stack_push():
    declaration_frame_stack.append(declaration_stack_pointer())
def declaration_frame_stack_pop():
    if len(declaration_frame_stack) > 1:
        declaration_stack_restore(declaration_frame_stack[-1])
        declaration_frame_stack.pop()
def declaration_stack_restore(n):
    while len(declaration_stack) > n:
        declaration_stack.pop()

def in_frame(symbol):
    sp = declaration_current_frame()
    n  = len(declaration_stack)
    for i in range(sp,n):
        if symbol == declaration_stack[i].symbol:
            return 1 
    return 0

def symbol_to_declaration_stack_entry(symbol):
    global declaration_stack
    n = len(declaration_stack)
    for i in range(n-1,-1,-1):
        if symbol == declaration_stack[i].symbol:
            return i
    return -1

def declaration_stack_push(symbol, type_, node = None):
    global declaration_stack
    i = len(declaration_stack)
    e = declaration_stack_entry(symbol, type_, node)
#    print(e.__str__())
    declaration_stack.append(e)
    return i

def declaration_stack_print():
    global declaration_stack
    n = len(declaration_stack)
    print("n=",n)
    for i in range(n):
        print(declaration_stack[i].__str__())
    return

def declaration_stack_set_type(i, type_):
    global declaration_stack
    declaration_stack[i].type_ = type_
    node = declaration_stack[i].node
    SetAttr(node,'InferredType',type_)
    return type_

def declaration_stack_set_value(i, value, node=None):
    global declaration_stack
    declaration_stack[i].value = value
    if node != None:
        SetAttr(node,'InferredValue',value)
    return value

# ========================================================================
# operation
# ========================================================================

def DECLARATION_STACK_SET_TYPE(X, type_):
    X.type_ = type_
    if hasattr(X,'sp'):
        i = X.sp
        declaration_stack_set_type(i, X.type_)
    return type_

class TypeAnalyzer(NodeEditor):
    def operation_Name_New(self, node):
        symbol = node.id
        if in_frame(symbol):
            i = symbol_to_declaration_stack_entry(symbol)
            type_ = declaration_stack[i].type_
        else:
            type_ = 'TYPE_VARIABLE'
            i = declaration_stack_push(symbol, type_, node)
        value = declaration_stack[i].value
        if value != None:
            SetAttr(node,'InferredValue',value)
            return reduced(type_, node, sp=i, value=value)
        else:
            return reduced(type_, node, sp=i)
 
    def operation_Name(self, node):
        symbol = node.id
        i = symbol_to_declaration_stack_entry(symbol)
        if i>=0:
            type_ = declaration_stack[i].type_
        else:
            type_ = 'TYPE_VARIABLE'
            i = declaration_stack_push(symbol, type_, node)
        value = declaration_stack[i].value
        if value != None:
            SetAttr(node,'InferredValue',value)
            return reduced(type_, node, sp=i, value=value)
        else:
            return reduced(type_, node, sp=i)

    def operation_Assign(self, target, value, node=None):
        if node == None:
            node = target

        global declaration_stack
        """
                 -- the following expression can appear in assignment context
                 | Attribute(expr value, identifier attr, expr_context ctx)
                 | Subscript(expr value, slice slice, expr_context ctx)
                 | Name(identifier id, expr_context ctx)
                 | List(expr* elts, expr_context ctx) 
                 | Tuple(expr* elts, expr_context ctx)
        """
        if type(target) == ast.Attribute: # dot operator ?
            self.visit(target)
            if value != None:
                self.visit(value)
            return reduced('TYPE_UNKNOWN4', node)
    
        if type(target) == ast.Subscript: # list operator ?
            arg0 = self.visit(target)
            if value != None:
#                i    = arg0.sp
#                arg1 = self.visit(value)
                arg1 = self.visit(value)
                if hasattr(arg1,'type_'):
                    type_ = arg1.type_
                else:
                    type_ = 'TYPE_UNKNOWN5'
#                declaration_stack_set_type(i, type_)
                if hasattr(arg1,'value'):
#                    declaration_stack_set_value(i, arg1.value, node)
                    return reduced(type_, node, value=arg1.value)
                else:
                    return reduced(type_, node)
            return reduced('TYPE_UNKNOWN5', node)

# -----------------------------------------------------------

        if type(target) == ast.Name:
            arg0 = self.operation_Name_New(target)
            if value != None:
                i     = arg0.sp
                if type(value) == str or type(value) == list:
                    arg1  = None
                    type_ = value
                else:
                    arg1 = self.visit(value)
                    if hasattr(arg1,'type_'):
                        type_ = arg1.type_
                    else:
                        type_ = 'TYPE_UNKNOWN6'
                declaration_stack_set_type(i, type_)
                if hasattr(arg1,'value'):
                    declaration_stack_set_value(i, arg1.value, node)
                    return reduced(type_, node, value=arg1.value)
                else:
                    return reduced(type_, node)
            return reduced('TYPE_UNKNOWN7', node)

        if type(target) in [ast.List, ast.Tuple]:
            if type(value) in [ast.List, ast.Tuple]:
                type_ = []
                for i in range(len(target.elts)):
                    try:
                        assign = self.operation_Assign(target.elts[i], value.elts[i])
                    except (TypeError, IndexError):
                        assign = self.operation_Assign(target.elts[i], None)
                    type_.append(assign.type_)
                reduced(type_, node)
            elif type(value) == ast.Name:
                arg1 = self.operation_Name(value)
                if hasattr(arg1,'type_'):
                    type_ = arg1.type_
                    if type(type_) == list:
#                        print("operation_Assign: ", location(node)," ", ast2str(target.elts)," = ",type_)
                        try:
                            for i in range(len(target.elts)):
                                self.operation_Assign(target.elts[i], type_[i])
                        except IndexError:
                            semantic_error(node, "bad index in: " + ast2str(node))
                        return reduced(arg1.type_, node)
                    else:
                        for i in range(len(target.elts)):
                            self.operation_Assign(target.elts[i], None)
                return reduced('TYPE_UNKNOWN8a', node)
            else:
                v = self.visit(value)
                type_ = v.type_
                if type(type_) == list:
                    try:
                        for i in range(len(target.elts)):
                            self.operation_Assign(target.elts[i], type_[i])
                    except IndexError:
                        semantic_error(node, "bad index in: " + ast2str(node))
                    return reduced(type_, node)
            return reduced('TYPE_UNKNOWN8', node)
    
        if type(target) in [list, tuple]:
            if type(value) in [list, tuple]:
                for i in range(len(target)):
                    try:
                        self.operation_Assign(target[i], value[i])
                    except (TypeError, IndexError):
                        self.operation_Assign(target[i], None)
            else:
                for i in range(len(target.elts)):
                    self.operation_Assign(target.elts[i], None)
            return reduced('TYPE_UNKNOWN9', node)
    
        print("unknown assignment: ", type(target))
        self.visit(target)
        self.visit(value)
        return None
    
    def operation_Call(self, node):
        func   = node.func
        args   = node.args

        actual = []
        dummy  = []
        if type(args) in [list, tuple]:
            for i in range(len(args)):
                actual.append(self.visit(args[i]))
        ret = self.visit(func)
        if hasattr(ret.node,'id'):
            symbol = ret.node.id
            i      = symbol_to_declaration_stack_entry(symbol)
            template_node = declaration_stack[i].node
            if template_node != None:
                if hasattr(template_node,'args'):
                    template = template_node.args
                    declaration_frame_stack_push()
                    a = len(template.args)
                    for i in range(a):
                        dummy.append(self.operation_Name_New(template.args[i]))
                    a = len(actual)
                    for i in range(a):
                        arg0 = dummy[i]
                        arg1 = actual[i]
                        j    = arg0.sp
                        if hasattr(arg1,'type_'):
                            type_ = arg1.type_
                        else:
                            type_ = 'TYPE_UNKNOWN10'
                        declaration_stack_set_type(j, type_)
                        if hasattr(arg1,'value'):
                            declaration_stack_set_value(j, type_)
                    declaration_frame_stack_pop()
            # special functions
            if (symbol == 'Integer') and (len(actual) == 1):
                value = getattr(actual[0],'value',0)
                SetAttr(node,'InferredValue',value)
                return reduced(ret.type_, node, value=value)
            if (symbol == 'product') and (len(actual) >= 1):
                type_ = getattr(actual[0],'type_',0)
                return reduced(type_, node)
    #            print("operation_Call: ",symbol)
    #            print("operation_Call: ",InferredType(template_node, 'TYPE_UNKNOWN10a'))
    #            print("operation_Call: ",ret.type_)
        return reduced(ret.type_, node)

    def operation_Add(self, X,Y,node):
        if X.type_ == 'TYPE_VARIABLE':
            DECLARATION_STACK_SET_TYPE(X,'TYPE_INTEGER')
        if Y.type_ == 'TYPE_VARIABLE':
            DECLARATION_STACK_SET_TYPE(Y,'TYPE_INTEGER')
        if hasattr(X,'value') and hasattr(Y,'value'):
            value = X.value+Y.value
            SetAttr(node,'InferredValue',value)
            return reduced(X.type_, node, value = value)
        else:
            return reduced(X.type_, node)

    def operation_Sub(self, X,Y,node):
        if X.type_ == 'TYPE_VARIABLE':
            DECLARATION_STACK_SET_TYPE(X,'TYPE_INTEGER')
        if Y.type_ == 'TYPE_VARIABLE':
            DECLARATION_STACK_SET_TYPE(Y,'TYPE_INTEGER')
        if hasattr(X,'value') and hasattr(Y,'value'):
            value = X.value-Y.value
            SetAttr(node,'InferredValue',value)
            return reduced(X.type_, node, value = value)
        else:
            return reduced(X.type_, node)

    def operation_Mult(self, X,Y,node):
        if X.type_ == 'TYPE_VARIABLE':
            if (Y.type_ != 'TYPE_VARIABLE') and \
               (Y.type_[0:12] != 'TYPE_UNKNOWN'):
                DECLARATION_STACK_SET_TYPE(X, Y.type_)
        if Y.type_ == 'TYPE_VARIABLE':
            if (X.type_ != 'TYPE_VARIABLE') and \
               (X.type_[0:12] != 'TYPE_UNKNOWN'):
                DECLARATION_STACK_SET_TYPE(Y, X.type_)
        if hasattr(X,'value') and hasattr(Y,'value'):
            value = X.value*Y.value
            SetAttr(node,'InferredValue',value)
            return reduced(X.type_, node, value = value)
        else:
            return reduced(X.type_, node)

    def operation_Div(self, X,Y,node):
        if X.type_ == 'TYPE_VARIABLE':
            if (Y.type_ != 'TYPE_VARIABLE') and \
               (Y.type_[0:12] != 'TYPE_UNKNOWN'):
                DECLARATION_STACK_SET_TYPE(X, Y.type_)
        if Y.type_ == 'TYPE_VARIABLE':
            if (X.type_ != 'TYPE_VARIABLE') and \
               (X.type_[0:12] != 'TYPE_UNKNOWN'):
                DECLARATION_STACK_SET_TYPE(Y, X.type_)
        if hasattr(X,'value') and hasattr(Y,'value'):
            value = X.value / Y.value
            SetAttr(node,'InferredValue',value)
            return reduced(X.type_, node, value = value)
        else:
            return reduced(X.type_, node)

    def operation_Pow(self, X,Y,node):
        if Y.type_ == 'TYPE_VARIABLE':
            DECLARATION_STACK_SET_TYPE(Y,'TYPE_INTEGER')
        if hasattr(X,'value') and hasattr(Y,'value'):
            value = X.value ** Y.value
            SetAttr(node,'InferredValue',value)
            return reduced(X.type_, node, value = value)
        else:
            return reduced(X.type_, node)

    def operation_Invert(self,X,node): # bit neg
        if hasattr(X,'value'):
            value = ~(X.value)
            SetAttr(node,'InferredValue',value)
            return reduced(X.type_, node, value = value)
        else:
            return reduced(X.type_, node)

    def operation_Not(self,X,node): # logical not
        if hasattr(X,'value'):
            value = not (X.value)
            SetAttr(node,'InferredValue',value)
            return reduced(X.type_, node, value = value)
        else:
            return reduced(X.type_, node)

    def operation_UAdd(self,X,node): # unary +
        if hasattr(X,'value'):
            value = (X.value)
            SetAttr(node,'InferredValue',value)
            return reduced(X.type_, node, value = value)
        else:
            return reduced(X.type_, node)

    def operation_USub(self,X,node): # unary -
        if hasattr(X,'value'):
            value = -(X.value)
            SetAttr(node,'InferredValue',value)
            return reduced(X.type_, node, value = value)
        else:
            return reduced(X.type_, node)

# ====================================================================
# visit_X
# ====================================================================
    def visit_Subscript(self, node):
#        NodeViewer().visit(node)
        head  = self.visit(node.value)
        index = self.visit(node.slice)
        if hasattr(index,'value'):
            if type(head.type_)==list:
                return reduced(head.type_[index.value], node)
            else:
                return reduced('TYPE_UNKNOWN:visit_Subscript:a', node)
        else:
            return reduced('TYPE_UNKNOWN:visit_Subscript:b', node)

    def visit_List(self, node):
        type_ = []
        for i in range(len(node.elts)):
            element = self.visit(node.elts[i])
            type_.append(element.type_)
        return reduced(type_, node)

    def visit_Tuple(self, node):
        return self.visit_List(node)

    def visit_Name(self, node):
        return self.operation_Name(node)

    def visit_Num(self, node):
        setattr(node,'InferredValue',node.n)
        return reduced('TYPE_INTEGER', node, value=node.n)

    def visit_Str(self, node):
        return reduced('TYPE_STRING', node)

    def visit_Call(self, node):
        return self.operation_Call(node)

    def visit_Attribute(self, node):
        if hasattr(node,'value'):
            t = type(node.value) 
            if (t == ast.Name) or (t == ast.Attribute):
                return self.visit(node.value)
        return self.generic_visit(node)

    def visit_BinOp(self, node):
        Op = type(node.op)
        X = self.visit(node.left)
        Y = self.visit(node.right)

        if   Op == ast.Add : return self.operation_Add(X,Y,node)
        elif Op == ast.Sub : return self.operation_Sub(X,Y,node)
        elif Op == ast.Mult: return self.operation_Mult(X,Y,node)
        elif Op == ast.Div : return self.operation_Div(X,Y,node)
        elif Op == ast.Pow : return self.operation_Pow(X,Y,node)
        else               : return reduced('TYPE_UNKNOWN12', node)

    def visit_UnaryOp(self, node):
        Op = type(node.op)
        X  = self.visit(node.operand)
        if   Op == ast.Invert: return self.operation_Invert(X,node)
        elif Op == ast.Not   : return self.operation_Not(X,node)
        elif Op == ast.UAdd  : return self.operation_UAdd(X,node)
        elif Op == ast.USub  : return self.operation_USub(X,node)
        else                 : return reduced('TYPE_UNKNOWN12a', node)

    def visit_Assign(self, node):
        if (type(node.targets) == list) and (len(node.targets) == 1):
            targets = node.targets[0]
            value   = node.value
            return self.operation_Assign(targets, value, node)
        else:
            print("unknown assignment targets:", node.targets)
        return reduced('TYPE_UNKNOWN13', node)

    def visit_FunctionDef(self, node):
        symbol = node.name
        type_  = 'TYPE_VARIABLE'
        declaration_stack_push(symbol, type_, node)
        declaration_frame_stack_push()
        self.visit(node.args)
        self.visit(node.body)
        declaration_frame_stack_pop()
        type_ = declaration_stack[-1].type_
        return reduced(type_, node)

    def visit_Return(self, node):
        ret = self.visit(node.value)
        i = declaration_current_frame() - 1
#        print("visit_Return:",i,",",ast2str(node))
        if i > 0:
            if hasattr(ret,'type_'):
                type_ = ret.type_
            else:
                type_ = 'TYPE_UNKNOWN14'
            declaration_stack_set_type(i, type_)
            setattr(node,'InferredType',type_)
        return ret

    def visit_arguments(self, node):
        a = len(node.args)
        d = len(node.defaults)
        type_ = []
        for i in range(a-d):
            element = self.operation_Name_New(node.args[i])
            type_.append(element.type_)
        for i in range(d):
            targets = node.args[a-d+i]
            value   = node.defaults[i]
            assign  = self.operation_Assign(targets, value)
            type_.append(assign.type_)
        return reduced(type_, node)

# ==================================================================
# NodeViewer
# ==================================================================

class NodeViewer(NodeEditor):

    def generic_visit(self, node):
        self.Print(type(node).__name__, ":",end='')
        if hasattr(node, 'InferredType'):
            self.Print(" InferredType = ", node.InferredType, begin='', end='')
        if hasattr(node, 'InferredValue'):
            self.Print(" InferredValue = ", node.InferredValue, begin='', end='')
        self.Print(begin='')
        if hasattr(node, '_fields'):
            self.indent+=1
            for name in node._fields:
                child = getattr(node, name)
                self.Print(name,"=",child)
                self.visit(child)
            self.indent-=1
        return None

# ==================================================================
# OpCount
# ==================================================================

function_stack = []

class OpCount(NodeEditor):
    def generic_visit(self, node):
        total = 0
        batch = 0
        if hasattr(node, '_fields'):
            self.indent+=1
            for name in node._fields:
                child = self.visit(getattr(node, name))
                transfer_batch(child)
                if hasattr(child,'total'):
#                    print("child.total=",child.total,end=' ')
#                    print("type(child.total)=",type(child.total))
#                    print("total=",total,end=' ')
#                    print("type(total)=",type(total))
                    total = child.total + total
                else:
#                    self.Print("?")
                    pass
            self.indent-=1
        if hasattr(node,'InferredType'):
            type_ = node.InferredType
        else:
            type_ = 'TYPE_UNKNOWN15'
#        self.Print(total)
        if hasattr(node,'InferredValue'):
            value = node.InferredValue
            return Reduced(type_, node, value=value, total=total)
        else:
            return Reduced(type_, node, total=total)

    def visit(self, node):
        if isinstance(node, ast.AST):
            name = type(node).__name__
            if name in NodeEditor.table:
                func_name = "visit_{x}".format(x=name)
                func = getattr(self, func_name, None)
                if callable(func):
                    return func(node)
            return self.generic_visit(node)
        self.indent+=1
        total = 0
        type_ = type(node)
        if type_ in [list, tuple, set]:
            for child_node in node:
                child = self.visit(child_node)
                transfer_batch(child)
#                NodeViewer().visit(child_node)
#                print("type(child.total)=",type(child.total))
#                print("type(total)=",type(total))
                total = child.total + total
        elif type_ == dict:
            for key in node.keys():
                child = self.visit(key)
                transfer_batch(child)
                total = child.total + total
                child = self.visit(node[key])
                transfer_batch(child)
                total = child.total + total
        elif (type_ == int) or (type_ == long):
            pass
        elif type_ == str:
            pass
        elif (node == None) or \
             (node == True) or \
             (node == False):
            pass
        else:
            self.Print("Unkown node type:",type(node),node)
        self.indent-=1
        return Reduced('TYPE_UNKNOWN2', node, total=total)

    def operation_Add(self,X,Y,node,**kwargs):
#        self.Print("operation_Add:", X.value,",", Y.value)
        if (X.type_ != 'TYPE_INTEGER') or \
           (Y.type_ != 'TYPE_INTEGER'):
            return Reduced(X.type_, node, total=0)
        transfer_batch(X)
        transfer_batch(Y)
        s = X.type_[5:] + '_add'
        total = formula(s) + Total(X) + Total(Y) 
        return Reduced(X.type_, node, total=total, batch=0, **kwargs)

    def operation_Sub(self,X,Y,node,**kwargs):
#        self.Print("operation_Sub:", X.type_,",", Y.type_)
        if (X.type_ != 'TYPE_INTEGER') or \
           (Y.type_ != 'TYPE_INTEGER'):
            return Reduced(X.type_, node, total=0)
        transfer_batch(X)
        transfer_batch(Y)
        s = X.type_[5:] + '_sub'
        total = formula(s) + Total(X) + Total(Y)
#        print("operation_Sub:", total)
        return Reduced(X.type_, node, total=total, batch=0, **kwargs)

    def operation_Mult(self,X,Y,node,**kwargs):
#        self.Print("operation_Mult:", X.type_,",", Y.type_)

        X_value = kwargs['X_value']
        Y_value = kwargs['Y_value']

        # X*1

        if (Y.type_ == 'TYPE_INTEGER') and (Y_value == 1):
            if (X.type_ == 'TYPE_INTEGER') or \
               (X.type_ == 'TYPE_GROUP')   or \
               (X.type_ == 'TYPE_GROUP0')  or \
               (X.type_ == 'TYPE_GROUP1')  or \
               (X.type_ == 'TYPE_GROUP2')  or \
               (X.type_ == 'TYPE_TARGET'):
                return X

        # 1*Y

        if (X.type_ == 'TYPE_INTEGER') and (X_value == 1):
            if (Y.type_ == 'TYPE_INTEGER') or \
               (Y.type_ == 'TYPE_GROUP')   or \
               (Y.type_ == 'TYPE_GROUP0')  or \
               (Y.type_ == 'TYPE_GROUP1')  or \
               (Y.type_ == 'TYPE_GROUP2')  or \
               (Y.type_ == 'TYPE_TARGET'):
                return Y

        # if not (((X.type_ == 'TYPE_INTEGER') or  \
        #          (X.type_ == 'TYPE_GROUP')   or  \
        #          (X.type_ == 'TYPE_GROUP0')  or  \
        #          (X.type_ == 'TYPE_GROUP1')  or  \
        #          (X.type_ == 'TYPE_GROUP2')  or  \
        #          (X.type_ == 'TYPE_TARGET')) and \
        #         (X.type_ == Y.type_)):
        #     OPERATION_ERROR2()

        if (X.type_ == 'TYPE_TARGET') or \
           (X.type_ == 'TYPE_GROUP') or \
           (X.type_ == 'TYPE_GROUP0') or \
           (X.type_ == 'TYPE_GROUP1') or \
           (X.type_ == 'TYPE_GROUP2'):
            X.batch = Batch(X)
            Y.batch = Batch(Y)
            X_b_coeff = Coeff(X.batch,var('b'))
            X_e_coeff = Coeff(X.batch,var('e'))
            X_p_coeff = Coeff(X.batch,var('p'))
            Y_b_coeff = Coeff(Y.batch,var('b'))
            Y_e_coeff = Coeff(Y.batch,var('e'))
            Y_p_coeff = Coeff(Y.batch,var('p'))
            total = 0
            batch = (X_e_coeff + Y_e_coeff) * formula('e') + \
                    (X_p_coeff + Y_p_coeff) * formula('p')
            if (X_b_coeff != 0) and (Y_b_coeff != 0):
                s = X.type_[5:] + '_mul'
                total = formula(s)
            if (X_b_coeff != 0) or (Y_b_coeff != 0):
                batch += formula('b')
            return Reduced(X.type_, node, \
                     total=Total(X)+Total(Y)+total, batch=batch, **kwargs)
        elif X.type_ == 'TYPE_INTEGER':
            s = X.type_[5:] + '_mul'
            total = formula(s)
            return Reduced(X.type_, node, \
                     total=Total(X)+Total(Y)+formula(s), batch=0, **kwargs)
        else:
            warning(node, 'cannot infer type of ' + ast2str(X.node))
            return Reduced(X.type_, node, \
                     total=Total(X)+Total(Y), batch=0, **kwargs)

    def operation_Div(self,X,Y,node,**kwargs):
#        self.Print("operation_Div:", X.type_,",", Y.type_)
        X_value = kwargs['X_value']
        Y_value = kwargs['Y_value']

        # X/1

        if (Y.type_ == 'TYPE_INTEGER') and (Y_value == 1):
            if (X.type_ == 'TYPE_INTEGER') or \
               (X.type_ == 'TYPE_GROUP')   or \
               (X.type_ == 'TYPE_GROUP0')  or \
               (X.type_ == 'TYPE_GROUP1')  or \
               (X.type_ == 'TYPE_GROUP2')  or \
               (X.type_ == 'TYPE_TARGET'):
                return X

        # 1/Y

        if (X.type_ == 'TYPE_INTEGER') and (X_value == 1):
            if (Y.type_ == 'TYPE_GROUP')   or \
               (Y.type_ == 'TYPE_GROUP0')  or \
               (Y.type_ == 'TYPE_GROUP1')  or \
               (Y.type_ == 'TYPE_GROUP2'):
                return Y
            if (Y.type_ == 'TYPE_TARGET'):
                X.type_ = 'TYPE_TARGET'
                X.batch = formula('b')
            # if (X.type_ == 'TYPE_INTEGER'):

        # if not (((X.type_ == 'TYPE_INTEGER') or  \
        #          (X.type_ == 'TYPE_GROUP')   or  \
        #          (X.type_ == 'TYPE_GROUP0')  or  \
        #          (X.type_ == 'TYPE_GROUP1')  or  \
        #          (X.type_ == 'TYPE_GROUP2')  or  \
        #          (X.type_ == 'TYPE_TARGET')) and \
        #         (X.type_ == Y.type_)):
        #     OPERATION_ERROR2()

        if (X.type_ == 'TYPE_TARGET') or \
           (X.type_ == 'TYPE_GROUP') or \
           (X.type_ == 'TYPE_GROUP0') or \
           (X.type_ == 'TYPE_GROUP1') or \
           (X.type_ == 'TYPE_GROUP2'):
            X.batch = Batch(X)
            Y.batch = Batch(Y)
            X_b_coeff = Coeff(X.batch,var('b'))
            X_e_coeff = Coeff(X.batch,var('e'))
            X_p_coeff = Coeff(X.batch,var('p'))
            Y_b_coeff = Coeff(Y.batch,var('b'))
            Y_e_coeff = Coeff(Y.batch,var('e'))
            Y_p_coeff = Coeff(Y.batch,var('p'))
            total = 0
            batch = (X_e_coeff + Y_e_coeff) * formula('e') + \
                    (X_p_coeff + Y_p_coeff) * formula('p')
            if (X_b_coeff != 0) and (Y_b_coeff != 0):
                s = X.type_[5:] + '_div'
                total = formula(s)
            if (X_b_coeff != 0) or (Y_b_coeff != 0):
                batch += formula('b')
            return Reduced(X.type_, node, \
                     total=Total(X)+Total(Y)+total, batch=batch, **kwargs)
        elif X.type_ == 'TYPE_INTEGER':
            s = X.type_[5:] + '_div'
            total = formula(s)
            return Reduced(X.type_, node, \
                     total=Total(X)+Total(Y)+formula(s), batch=0, **kwargs)
        else:
            warning(node, 'cannot infer type of ' + ast2str(X.node))
            return Reduced(X.type_, node, \
                     total=Total(X)+Total(Y), batch=0, **kwargs)

    def operation_Pow(self,X,Y,node,**kwargs):
#        self.Print("operation_Pow:", X.type_,",", Y.type_)
#        self.Print("operation_Pow:", kwargs['X_value'],",", kwargs['Y_value'])
        X_value = kwargs['X_value']
        Y_value = kwargs['Y_value']

        if (type(Y_value) == int):
            if (Y_value == 1) or (Y_value == 0):
                if (X.type_ == 'TYPE_INTEGER') or \
                   (X.type_ == 'TYPE_GROUP')   or \
                   (X.type_ == 'TYPE_GROUP0')  or \
                   (X.type_ == 'TYPE_GROUP1')  or \
                   (X.type_ == 'TYPE_GROUP2')  or \
                   (X.type_ == 'TYPE_TARGET'):
                    return X
            if (Y_value < 0):
                mY  = Reduced('TYPE_INTEGER', node.right, total=0, batch=0)
                one = Reduced('TYPE_INTEGER', node.right, total=0, batch=0)
                Pow_kwargs = {}
                Pow_kwargs['X_value'] =  X_value
                Pow_kwargs['Y_value'] = -Y_value
                Div_kwargs = {}
                Div_kwargs['X_value'] = 1
                Div_kwargs['Y_value'] = 'unknown'
                return self.operation_Div(one,\
                  self.operation_Pow(X,mY,node,**Pow_kwargs),\
                  node, **Div_kwargs)

        if (X.type_ == 'TYPE_TARGET') or \
           (X.type_ == 'TYPE_GROUP')  or \
           (X.type_ == 'TYPE_GROUP0') or \
           (X.type_ == 'TYPE_GROUP1') or \
           (X.type_ == 'TYPE_GROUP2'):
            X.batch = Batch(X)
            Y.batch = Batch(Y)
            X_b_coeff = Coeff(X.batch,var('b'))
            X_e_coeff = Coeff(X.batch,var('e'))
            X_p_coeff = Coeff(X.batch,var('p'))
            Y_b_coeff = Coeff(Y.batch,var('b'))
            Y_e_coeff = Coeff(Y.batch,var('e'))
            Y_p_coeff = Coeff(Y.batch,var('p'))
            s = X.type_[5:] + '_mul'
            total = X_e_coeff * formula(s)
            batch = (X_e_coeff + X_b_coeff) * formula('e') + \
                    (X_p_coeff            ) * formula('p')
            return Reduced(X.type_, node, \
                     total=Total(X)+Total(Y)+total, batch=batch, **kwargs)
        elif X.type_ == 'TYPE_INTEGER':
            s = X.type_[5:] + '_pow'
            return Reduced(X.type_, node, \
                     total=Total(X)+Total(Y)+formula(s), batch=0, **kwargs)
        else:
            warning(node, 'cannot infer type of ' + ast2str(X.node))
            return Reduced(X.type_, node, \
                     total=Total(X)+Total(Y), batch=0, **kwargs)

    def visit_BinOp(self, node):
        op = type(node.op)
        X = self.visit(node.left)
        Y = self.visit(node.right)
        kwargs = {}

        if hasattr(node,'InferredType'):
            type_ = node.InferredType
        else:
            type_ = X.type_

        if hasattr(node,'InferredValue'):
            kwargs['value'] = node.InferredValue

        if hasattr(node.left,'InferredValue'):
            kwargs['X_value'] = node.left.InferredValue
        else:
            kwargs['X_value'] = 'unknown'

        if hasattr(node.right,'InferredValue'):
            kwargs['Y_value'] = node.right.InferredValue
        else:
            kwargs['Y_value'] = 'unknown'

        if   op == ast.Add  : return self.operation_Add(X,Y,node,**kwargs)
        elif op == ast.Sub  : return self.operation_Sub(X,Y,node,**kwargs)
        elif op == ast.Mult : return self.operation_Mult(X,Y,node,**kwargs)
        elif op == ast.Div  : return self.operation_Div(X,Y,node,**kwargs)
        elif op == ast.Pow  : return self.operation_Pow(X,Y,node,**kwargs)

        return Reduced(type_, node, total = Total(X) + Total(Y), **kwargs)

    def operation_Name(self, node):
        total = 0
        batch = 0
        if hasattr(node,'InferredType'):
            type_ = node.InferredType
        else:
            type_ = 'TYPE_UNKNOWN16'

        if (type_ == 'TYPE_GROUP')  or \
           (type_ == 'TYPE_GROUP0') or \
           (type_ == 'TYPE_GROUP1') or \
           (type_ == 'TYPE_GROUP2') or \
           (type_ == 'TYPE_TARGET'):
            batch = formula('b')

        if hasattr(node,'InferredValue'):
            value = node.InferredValue
            return Reduced(type_, node, value=value, total=total, batch=batch, id=node.id)
        else:
            return Reduced(type_, node, total=total, batch=batch, id=node.id)

    def visit_Name(self, node):
        return self.operation_Name(node)

    def visit_Attribute(self, node):
        if hasattr(node,'value'):
            t = type(node.value) 
            if (t == ast.Name) or (t == ast.Attribute):
                return self.visit(node.value)
        return self.generic_visit(node)

    def bitexp(self, node):
        args   = node.args
        if len(args) == 2:
            a1    = ast2str(args[0])
            right = self.visit(args[1])
            transfer_bitexp(right, a1)
            return right
        else:
            semantic_error(node,\
              "too few or many arguments :" + ast2str(node))

    def inline(self, node):
        args   = node.args
        if len(args) == 1 and type(args[0]) == ast.Str:
            print(args[0].s)
            return Reduced('TYPE_INLINE', node, total=0, batch=0)
        else:
            semantic_error(node,\
              "invalid arguments for inline:" + ast2str(node))

    def product(self, node):
        args   = node.args
        if len(args) == 2:
            n = ast2str(args[0])
            n = n.replace(",",";")
            n = n.replace(" ","")
            n = n.replace("_sage_const_","")
            N = int(n)
            right = self.visit(args[1])

            e_coeff = Coeff(right.batch,var('e'))
            b_coeff = Coeff(right.batch,var('b'))
            p_coeff = Coeff(right.batch,var('p'))

            total = N * right.total
            batch = (N * e_coeff) * formula('e') + \
                    (N * p_coeff) * formula('p')

            if N != 0:
                if b_coeff != 0:
                    s = right.type_[5:] + '_mul'
                    total += (N - 1) * formula(s)
                    batch += formula('b')

            R = Reduced(right.type_, node, total=total, batch=batch)
            return R
        else:
            semantic_error(node,\
              "invalid arguments for product:" + ast2str(node))

    def visit_Call(self, node):
        func   = node.func
        args   = node.args
        ret    = self.visit(func)
        total  = 0
        batch  = 0
        type_  = InferredType(node)

        if not hasattr(ret,'id'):
            if type(args) in [list, tuple]:
                for i in range(len(args)):
                    child = self.visit(args[i])
                    transfer_batch(child)
                    total = child.total + total
        else:
            symbol = ret.node.id
    
            if (symbol == 'bitexp'): return self.bitexp(node)
            if (symbol == 'inline'): return self.inline(node)
            if (symbol == 'product'): return self.product(node)
    
            if type(args) in [list, tuple]:
                for i in range(len(args)):
                    child = self.visit(args[i])
                    transfer_batch(child)
                    total = child.total + total
                    # <-- process_Assign
    
            if (symbol == 'e'):
                return Reduced('TYPE_TARGET', node, total=total, \
                         batch=formula('b') + formula('p'))
            else:
                if (symbol != 'Integer')  and \
                   (symbol != 'Group')    and \
                   (symbol != 'Group0')   and \
                   (symbol != 'Group1')   and \
                   (symbol != 'Group2')   and \
                   (symbol != 'Target'):
                    total = formula('call_' + symbol) + total
                if (type_ == 'TYPE_GROUP')  or \
                   (type_ == 'TYPE_GROUP0') or \
                   (type_ == 'TYPE_GROUP1') or \
                   (type_ == 'TYPE_GROUP2') or \
                   (type_ == 'TYPE_TARGET'):
                    batch = formula('b')

        return Reduced(type_, node, total=total, batch=batch)

    def visit_FunctionDef(self, node):
        global function_stack
        symbol = node.name
        args = self.visit(node.args)
        body = self.visit(node.body)
        transfer_batch(body)
        total = body.total
        if (symbol[0] != '_') and \
           (symbol != 'e'):
            print("call_" + symbol + " = " + str(total).replace(';',',') + ';')
            function_stack.append(symbol)
        return Reduced('TYPE_FUNCTIONDEF', node, total=0, batch=0)

    def visit_If(self, node):
        body = self.visit(node.body)
        transfer_batch(body)
        total = body.total
        # elifs.
        if (node.orelse and len(node.orelse) == 1 and \
               isinstance(node.orelse[0], ast.If)):
            orelse = self.visit(node.orelse[0])
            transfer_batch(orelse)
            orelse = orelse.total
        # final else
        elif node.orelse:
            orelse = self.visit(node.orelse)
            transfer_batch(orelse)
            orelse = orelse.total
        else:
            orelse = 0
        t  = ast2str(node.test)
        t  = t.replace(",",";")
        t  = t.replace(" ","")
        s  = 'if_statement("'
        s += t
        s += '";'
        s += str(total)
        s += ';'
        s += str(orelse)
        s += ")"
        total = formula(s)
        return Reduced('TYPE_IF', node, total=total, batch=0)

    def visit_For(self, node):
        body = self.visit(node.body)
        transfer_batch(body)
        total = body.total
        if node.orelse:
            orelse = self.visit(node.orelse)
            transfer_batch(orelse)
            orelse = orelse.total
        else:
            orelse = 0
        t  = ast2str(node.target)
        t  = t.replace(",",";")
        t  = t.replace(" ","")
        t  = t.replace("_sage_const_","")
        u  = ast2str(node.iter)
        u  = u.replace(",",";")
        u  = u.replace(" ","")
        u  = u.replace("_sage_const_","")
        s  = 'for_statement("'
        s += t
        s += '";"'
        s += u
        s += '";'
        s += str(total).replace(' ','')
        s += ';'
        s += str(orelse).replace(' ','')
        s += ")"
        total = formula(s)
        return Reduced('TYPE_FOR', node, total=total, batch=0)

# =========================================================================

def main():
    n = len(sys.argv)
    i = 0
    Print = False
    for i in range(1,n):
        filename = sys.argv[i]
        if filename == '-t': break
        if filename == '-p':
            Print = True
            continue
        f = open(filename, 'rb')
        print(f.read())
        f.close()

    data = sys.stdin.read()
    sys.stdin.close()
    try:
        A = ast.parse(data)
    except SyntaxError as E:
        message = 'syntactic error: line {x}, offset {y}, text {s}'.format(x=E.lineno,y=E.offset,s=E.text)
        print(message, file=sys.stderr)
        sys.exit(1)  # syntactic error
    TypeAnalyzer().visit(A)
#    NodeViewer().visit(A)
    OpCount().visit(A)

    for j in range(i+1,n):
        filename = sys.argv[j]
        if filename == '-p':
            Print = True
            continue
        f = open(filename, 'rb')
        print(f.read())
        f.close()

    if Print:
        for f in function_stack:
            print('print("' + f + ' = ", eval(call_' + f + '));')

if __name__ == "__main__":
    main()
