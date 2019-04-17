import sys
import hashlib

p = 87369858268705636068130620660487437707575022223654752313117208450295069782989900711984586934672211770206061984637489066359818285270740009637007578657541483
K.<w> = GF(p^2, modulus=(x^2 + x + 1))
E = EllipticCurve(K,[0,0,0,0,1])
q = 2^160+7
c = Integer((p+1)/q)
X = 40592263294143433237494079401382247628167293705023275456095814770133530315637428394508707791046369563570471906712927865124299180709031484514789094287053963
Y = 47415637585301645093278793967072945423923480258771851494306086640586812214919223776553732139170126751983717718898763201230718883181063640345318922577502350
P = E([X, Y])

class Group:
    def __init__(self, P=[X,Y,1]):
        if type(P) == Group:
            self.point = P.point
        else:
            self.point = E(P)

    def __mul__(self,Q):
        return Group(self.point + Q.point)

    def __div__(self,Q):
        return Group(self.point - Q.point)

    def __pow__(self,x):
        return Group(x * self.point)

    def __eq__(self,Q):
        return self.point.__eq__(Q.point)

    def __ne__(self,Q):
        return self.point.__ne__(Q.point)

    def __delitem__(self, key):
        self.point.__delitem__(key)

    def __getitem__(self, key):
        return self.point.__getitem__(key)

    def __setitem__(self, key, value):
        self.point.__setitem__(key, value)

    def __repr__(self):
        return str(self.point)

    def __str__(self):
        return str(self.point)

    def _distortion(self):
        return Group([self[0]*w, self[1], self[2]])

gT = Group().point.tate_pairing(Group()._distortion().point,q,2)

class Target:

    def __init__(self, value = gT):
        if type(value) == Target:
            self.value = value.value
        else:
            self.value = value

    def __mul__(self,y):
        return Target(self.value * y.value)

    def __div__(self,y):
        return Target(self.value / y.value)

    def __pow__(self,y):
        return Target(self.value ^ y)

    def __eq__(self,Q):
        return self.value.__eq__(Q.value)

    def __ne__(self,Q):
        return self.value.__ne__(Q.value)

    def __delitem__(self, key):
        self.value.__delitem__(key)

    def __getitem__(self, key):
        return self.value.__getitem__(key)

    def __setitem__(self, key, value):
        self.value.__setitem__(key, value)

    def __str__(self):
        return str(self.value)

    def __repr__(self):
        return str(self.value)

def e(P,Q):
    return Target(P.point.tate_pairing(Q._distortion().point,q,2))

def Random():
    return Integer(randrange(0,q-1))

def Hash2point(ID = str()):
    Y = Mod(Integer('0x' + hashlib.sha256(ID).hexdigest()),p)
    X = (Y^2 - 1)^((2*p-1)/3)
    P = E([X, Y])
    Q = c * P
    return Group(Q)

g1 = Group()
g2 = Group()

# =========================================================================
#
# =========================================================================

def Setup():
    x = Random()
    y = g2^x
    return [x,y]

def Keygen(ID = str(), x = Integer()):
    return Hash2point(ID)^x

def Enc(m = Target(), ID = str(), y = Group() ):
    r  = Random()
    c1 = g1^r
    c2 = m * e(Hash2point(ID),y^r)
    return [c1,c2]

def Dec(c = [Group(), Target()], d_ID = Group() ):
    c1 = c[0]
    c2 = c[1]
    m = c2/e(c1,d_ID)
    return m

m  = Target()^Random()
(x,y) = Setup()
d_ID = Keygen('Alice',x)
c = Enc(m,'Alice',y)
md = Dec(c,d_ID) 
print(m == md)
