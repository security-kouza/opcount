g = Group()

def Random():
    return Integer()

def Setup():
    x = Random()
    y = g^x
    return (x,y)

def Enc(y = Group(), m = Group()):
    r  = Random()
    return (g^r, m * y^r)

def Dec(x = Integer(), c = (Group(), Group())):
    (c1,c2) = c
    return c2/c1^x
