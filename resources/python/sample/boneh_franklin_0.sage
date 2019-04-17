def Random():
    return Integer()

def Hash2point(ID = str()):
    return Group()

g1 = Group()
g2 = Group()

def Setup():
    x = Random()
    y = g2^x
    return (x,y)

def Keygen(ID = str(), x = Integer()):
    return Hash2point(ID)^x

def Enc(m = Target(), ID = str(), y = Group() ):
    r  = Random()
    c1 = g1^r
    c2 = m * e(Hash2point(ID),y^r)
    return (c1,c2)

def Dec(c = (Group(), Target()), d_ID = Group() ):
    (c1,c2) = c
    m = c2/e(c1,d_ID)
    return m
