inline("""

  range(A_, B_ = 0, C_ = 1) = {
    return (floor((A_ - B_)/C_)) ;
  }

  for_statement(A_,B_,C_,D_) = {
    A_ = eval(B_) ;
    return (A_ * C_ + D_) ;
  }

  if_statement(A_,B_,C_) = {
    return (if_prob * B_ + (1 - if_prob) * C_) ;
  }

  ni = 10;
  if_prob = 1/2 ;

""")

INTEGER    = Integer()
GROUP1     = Group1()
GROUP2     = Group2()
TARGET     = Target()

CRS0_TYPE  = (GROUP1, GROUP1, GROUP1, GROUP1)
CRS1_TYPE  = (GROUP2, GROUP2, GROUP2, GROUP2)

Sep = INTEGER # security parameter specified in PerfDB
Ord = INTEGER # group order (=q)

G   = GROUP1  # default generator for the left group
Gh  = GROUP2  # default generator for the right group
Gt  = TARGET  # default generator for the target group Gt = e(G, Gh);

def rnd():
    return INTEGER

def rnd_g():
    r = rnd()
    return G^r

def rnd_gh():
    r = rnd()
    return Gh^r

# GS-proof functions

def GS_GenCrs_0( G = GROUP1 ):
    ch = rnd()
    xi = rnd()

    Q = G^(ch)
    U = G^(xi)
    V = G^(ch*xi)
    CRS = (G, Q, U, V)

    return CRS

def GS_ComInt_0( crs = CRS0_TYPE, s = INTEGER ):
    (G, Q, U, V) = crs
    r = rnd()

    c1 = U^(s)*G^(r)
    c2 = (V*G)^(s)*Q^(r)
    comv = (c1, c2)

    return (comv, r)

# special case, s=0

def GS_ComIntZero_0( crs = CRS0_TYPE ):
    (G, Q, U, V) = crs
    r = rnd()

    c1 = G^(r)
    c2 = Q^(r)
    comv = (c1, c2)

    return (comv, r)

# default commitment for s=1, r=0

def GS_DefaultComInt_0( crs = CRS0_TYPE ):
    (G, Q, U, V) = crs
    r = 0

    c1 = U
    c2 = V*G
    comv = (c1, c2)

    return (comv, r)

def GS_GenCrs_1( G = GROUP2 ):
    ch = rnd()
    xi = rnd()

    Q  = G^(ch)
    U  = G^(xi)
    V  = G^(ch*xi)
    CRS = (G, Q, U, V)

    return CRS

def GS_ComInt_1( CRS = CRS1_TYPE, s = INTEGER ):
    (G, Q, U, V) = CRS
    r = rnd()

    c1=U^(s)*G^(r)
    c2=(V*G)^(s)*Q^(r)
    comv = (c1, c2)
    return (comv, r)

# special case, s=0

def GS_ComIntZero_1( CRS = CRS1_TYPE ):
    (G, Q, U, V) = CRS
    r = rnd()

    c1=G^(r)
    c2=Q^(r)
    comv = (c1, c2)

    return (comv, r)

def GS_DefaultComInt_1( CRS = CRS1_TYPE ):
    (G, Q, U, V) = CRS
    r = 0

    c1=U
    c2=V*G
    comv = (c1, c2)

    return (comv, r)

def GS_Pf_MSE_1(g1 = GROUP2, g2 = GROUP2, g3 = GROUP2, a = INTEGER, b = INTEGER, c = INTEGER):
    pi = (g1^a) * (g2^(-b)) * (g3^(-c))
    return pi

# special case, a=0

def GS_Pf_MSEZero_1(g1 = GROUP2, g2 = GROUP2, g3 = GROUP2, b = INTEGER, c = INTEGER):
    pi=(g2^(-b))*(g3^(-c))
    return pi

def GS_Pf_MSE_0(g1 = GROUP1, g2 = GROUP1, g3 = GROUP1, a = INTEGER, b = INTEGER, c = INTEGER):
    pi=(g1^a)*(g2^(-b))*(g3^(-c))
    return pi

# special case, a=0

def GS_Pf_MSEZero_0(g1 = GROUP1, g2 = GROUP1, g3 = GROUP1, b = INTEGER, c = INTEGER):
    pi=(g2^(-b))*(g3^(-c))
    return pi

def GS_Pf_NonQE(
    CRS0 = CRS0_TYPE,
    CRS1 = CRS1_TYPE,
    z0 = INTEGER, z1 = INTEGER, x2 = INTEGER,
    z2 = INTEGER, rz0 = INTEGER, rz1 = INTEGER,
    rx2 = INTEGER, rz2 = INTEGER
):

    (G, Q, U, V)     = CRS0
    (Gh, Qh, Uh, Vh) = CRS1

    r = rnd()

    # General Formula for arbitrary inputs

    th1=U^(z0*(rx2-rz2)-z1*(rx2-rz2)) * G^((rx2-rz2)*(rz0-rz1)-r)
    th2=((V*G)^(z0*(rx2-rz2)-z1*(rx2-rz2)))*(Q^((rx2-rz2)*(rz0-rz1)-r))
    pi1=(Uh^(x2*(rz0-rz1)-z2*(rz0-rz1)))*Gh^r
    pi2=((Vh*Gh)^(x2*(rz0-rz1)-z2*(rz0-rz1)))*Qh^r

    lrho=(th1, th2, pi1, pi2)

    return lrho

def GS_Pf_NonQEZero(
    CRS0 = CRS0_TYPE,
    CRS1 = CRS1_TYPE,
    z0   = INTEGER, z1  = INTEGER,
    rz0  = INTEGER, rz1 = INTEGER,
    rx2  = INTEGER, rz2 = INTEGER
):
    (G, Q, U, V) = CRS0
    (Gh, Qh, Uh, Vh) = CRS1

    r = rnd()

    th1 = G^((rx2-rz2)*(rz0-rz1)-r)
    th2 = Q^((rx2-rz2)*(rz0-rz1)-r)
    pi1 = Gh^r
    pi2 = Qh^r

    lrho = (th1, th2, pi1, pi2)

    return lrho

# ElGamal Encryption functions
# As it is used as a commitment scheme, we do not use a secret-key
# or decryption function.
# We allow randomness re-use in encryption in G1.

def ElGamal_KeyGen_1( Gh = GROUP2 ):
    y  = rnd()
    Yh = Gh^(y)

    return (Yh, y)

def ElGamal_KeyGen_0( G = GROUP1 ):
    y = rnd()
    Y = G^(y)

    return (Y, y)

def ElGamal_DblEnc_1(
    x0 = INTEGER, x1 = INTEGER, 
    Gh = GROUP2, Yh0 = GROUP2, Yh1 = GROUP2
):
    r = rnd()
    Eh0=Gh^(x0)*Yh0^(r)
    Eh1=Gh^(x1)*Yh1^(r)
    Ehr=Gh^(r)
    return (Eh0, Eh1, Ehr)

def ElGamal_Enc_0(x = INTEGER, G = GROUP1, Y = GROUP1):
    r = rnd()
    E=G^(x)*Y^(r)
    Er=G^(r)
    return (E, Er)

# special case, x=0

def ElGamal_EncZero_0(G = GROUP1, Y = GROUP1):
    r  = rnd()
    E  = Y^(r)
    Er = G^(r)
    return (E, Er)

# Partially One-Time Signature Functions
# We only consider non-adaptive signing that 
# generates a one-time key internally at the time of signing.
# A message consists of 10 group elements in G0.

POSpk_TYPE = (GROUP1, GROUP2, GROUP2, GROUP2)
POSsk_TYPE = (INTEGER, INTEGER)

def POS_Keygen( Gh = GROUP2 ):
    for i in range(10):
        gami = rnd()
        Ghi = Gh^(gami)

    w = rnd()
    Ghr = Gh^w

    POSpk = (G, Gh, Ghr, Ghi)
    POSsk = (w, gami)

    return (POSpk, POSsk)

def POS_Sign(
    POSpk = POSpk_TYPE,
    POSsk = POSsk_TYPE,
    Mi = GROUP1
):
    (G, Gh, Ghr, Ghi) = POSpk
    (w, gami)         = POSsk

    # One-time key generation

    al = rnd()
    Ah = Gh^(-al)

    # Signature generation

    ro = rnd()
    Z  = G^(al-ro*w)
    R  = G^(ro) * product(10, Mi^(-gami))

    sigma = (Ah, Z, R)
    return sigma

# //integer POS_Verif(list POSpk, group1 Mi, list sigma)
# //{
# //	group1 G, Md;
# //	group2 Gh, Ghr, Ghi;
# //	group2 Ah;
# //	group1 Z, R;
# //	target Mt, Lt, Rt;
# //
# //
# //	// Parse the arguments
# //	(G, Gh, Ghr, Ghi)=POSpk;
# //	// Parse signature
# //	(Ah, Z, R)=sigma;
# //	
# //	Mt = 1;
# //	for(i=1;i<10;i++){	
# //		Mt = Mt * e(Mi, Ghi); //M_i^{-gamma_i}//
# //	}
# //
# //	Lt=e(G, Ah);
# //	Rt=e(Z, Gh)*e(R, Ghr)*Mt;
# //		
# //   if_statement(Lt=Rt){
# //   return 1;
# //  }
# //
# //}

##########################################################################

def tSPSGen( ord = INTEGER, G = GROUP1, Gh = GROUP2 ):

    (Y2, y2)  = ElGamal_KeyGen_0(G)
    (Yh0, y0) = ElGamal_KeyGen_1(Gh)
    (Yh1, y1) = ElGamal_KeyGen_1(Gh)

    x0 = rnd()
    x1 = 0
    x2 = 0

    CRS00 = GS_GenCrs_0(G)
    CRS10 = GS_GenCrs_0(G)
    CRS11 = GS_GenCrs_1(Gh)

    (comx00, rx00)   = GS_ComInt_0(CRS00, x0)
    # for x1
    (comx10, rx10)   = GS_ComIntZero_0(CRS00)
    (comy00, ry00)   = GS_ComInt_0(CRS00, y0)
    # for x2
    (comhx21, rhx21) = GS_ComIntZero_1(CRS11)
    (comy01, ry01)   = GS_ComInt_0(CRS10, y0)
    (comy11, ry11)   = GS_ComInt_0(CRS10, y1)
    (comhy21, rhy21) = GS_ComInt_1(CRS11, y2)

    # (com10, r10)   = GS_ComInt_0(CRS00, 1)
    # (com11, r11)   = GS_ComInt_0(CRS10, 1)
    # (comh11, rh11) = GS_ComInt_1(CRS11, 1)

    (POSpk, POSsk) = POS_Keygen(Gh)

    pk = (G, Gh, CRS00, CRS10, CRS11, Yh0, Yh1, Y2, comx00, comx10, comy00, comhx21, comy01, comy11, comhy21, POSpk)
    sk = (x0, y0, y1, y2, rx00, rx10, ry00, rx21, ry01, ry11, ry21)

    return (pk,sk)

#########################

##
#### Signing 
##

Com0_TYPE   = (GROUP1, GROUP1)
Com1_TYPE   = (GROUP2, GROUP2)

# pk = (G, Gh, 
#       CRS00, CRS10, CRS11, 
#       Yh0, Yh1, Y2, 
#       comx00, comx10, comy00, 
#       comhx21, comy01, comy11, comhy21, 
#       POSpk)

tSPSpk_TYPE = (GROUP1, GROUP2,                              \
               CRS0_TYPE, CRS0_TYPE, CRS1_TYPE,             \
               GROUP2, GROUP2, GROUP1,                      \
               Com0_TYPE, Com0_TYPE, Com0_TYPE,             \
               Com1_TYPE, Com0_TYPE, Com0_TYPE, Com1_TYPE,  \
               POSpk_TYPE)

# sk = (x0, y0, y1, y2, 
#       rx00, rx10, ry00, rx21, 
#       ry01, ry11, ry21)

tSPSsk_TYPE = (INTEGER, INTEGER, INTEGER, INTEGER, \
               INTEGER, INTEGER, INTEGER, INTEGER, \
               INTEGER, INTEGER, INTEGER)


def tSPSSign( pk = tSPSpk_TYPE, sk = tSPSsk_TYPE, Mi = GROUP1 ):

    (G, Gh, CRS00, CRS10, CRS11, Yh0, Yh1, Y2, comx00, comx10, comy00, comhx21, comy01, comy11, comhy21, POSpk) = pk
    (x0, y0, y1, y2, rx00, rx10, ry00, rhx21, ry01, ry11, rhy21) = sk
    (G, Q0, U0, V0) = CRS00
    (G, Q1, U1, V1) = CRS10
    (Gh, Qh1, Uh1, Vh1) = CRS11
 	
    z0 = x0
    z1 = x0
    z2 = 0

    x2 = 0

    # ElGamal Enc of z0, z1, z2 #

    (Ehz0, Ehz1, Ehs) = ElGamal_DblEnc_1(z0, z1, Gh, Yh0, Yh1)
    (Ez2, Et) = ElGamal_EncZero_0(G, Y2)

    # Commit of z0, z1, z2 #

    (comz00, rz00)   = GS_ComInt_0(CRS00, z0)
    (comz01, rz01)   = GS_ComInt_0(CRS10, z0)
    (comz11, rz11)   = GS_ComInt_0(CRS10, z1)

    # for z2 #

    (comhz21, rhz21) = GS_ComIntZero_1(CRS11)

#    (com10, r10)     = GS_DefaultComInt_0(CRS00)
#    (com11, r11)     = GS_DefaultComInt_0(CRS10)

    # GS proofs #

    rhoh00 = GS_Pf_MSE_1(Gh, Gh, Ah, rz00, rx00, rx10)
    rhoh01 = GS_Pf_MSEZero_1(Ehz0, Gh, Ehs, rz00, ry00)
    rhoh11 = GS_Pf_MSEZero_1(Ehz0, Gh, Ehs, rz01, ry01)
    rhoh12 = GS_Pf_MSEZero_1(Ehz1, Gh, Ehs, rz11, ry11)
    rho13  = GS_Pf_MSEZero_0(Ez2, G, Et, rhz21, rhy21)

    lrho10 = GS_Pf_NonQEZero(CRS10, CRS11, z0, z1, rz01, rz11, rhx21, rhz21)

    # POS signing #

    (Ah, Z, R) = POS_Sign(POSpk, POSsk, Mi)

    sigma = (Ah, Z, R, Ehz0, Ehz1, Ehs, Ez2, Et, comz00, comz01, comz11, comhz21, rhoh00, rhoh01, lrho10, rhoh11, rhoh12, rho13)
    return sigma

##
#### Verification
####

#     Ah,      Z,      R,   Ehz0,   Ehz1,     Ehs, 
#    Ez2,     Et, comz00, comz01, comz11, comhz21,
# rhoh00, rhoh01, lrho10,
# rhoh11, rhoh12,  rho13

tSPSSig_TYPE = (                                                      \
    GROUP2,    GROUP1,    GROUP1,    GROUP2,    GROUP2,    GROUP2,    \
    GROUP1,    GROUP1,    Com0_TYPE, Com0_TYPE, Com0_TYPE, Com1_TYPE, \
    GROUP2,    GROUP2,   (GROUP1,    GROUP1,    GROUP2,    GROUP2),   \
    GROUP2,    GROUP2,    GROUP1                                      \
)          

def tSPSVerify(
  sigma = tSPSSig_TYPE, 
  pk    = tSPSpk_TYPE,
  Mi    = GROUP1
):
    (G, Gh, CRS00, CRS10, CRS11, Yh0, Yh1, Y2, comx00, comx10, \
      comy00, comhx21, comy01, comy11, comhy21, POSpk) = pk

    (G, Q0, U0, V0)     = CRS00
    (G, Q1, U1, V1)     = CRS10
    (Gh, Qh1, Uh1, Vh1) = CRS11

    (C1x00, C2x00) = comx00
    (C1x10, C2x10) = comx10
    (C1y00, C2y00) = comy00
    (D1x21, D2x21) = comhx21
    (C1y01, C2y01) = comy01
    (C1y11, C2y11) = comy11
    (D1y21, D2y21) = comhy21

    (G, Gh, Ghr, Ghi) = POSpk

    (Ah, Z, R, Ehz0, Ehz1, Ehs, Ez2, Et, comz00, comz01, comz11, \
      comhz21, rhoh00, rhoh01, lrho10, rhoh11, rhoh12, rho13) = sigma

    (C1z00, C2z00) = comz00
    (C1z01, C2z01) = comz01
    (C1z11, C2z11) = comz11
    (D1z21, D2z21) = comhz21

    (thet101, thet102, pih101, pih102) = lrho10

    # [1] generation #

    (com10, r10)   = GS_DefaultComInt_0(CRS00)
    (com11, r11)   = GS_DefaultComInt_0(CRS10)
    (comh11, rh11) = GS_DefaultComInt_1(CRS11)

    (C110, C210) = com10
    (C111, C211) = com11
    (D111, D211) = comh11

    # Verification equations #

# //	target Mt;
# //	Mt = 1;
# //	for(i=1;i<10;i++){	
# //		Mt = Mt * e(Mi, Ghi); //M_i^{-gamma_i}//
# //	}

    b = (
        1==e(G, Ah)*e(Z, Gh)*e(R,Ghr)*product(10, e(Mi,Ghi)) and
        1==e(C1z00^(-1), Gh)*e(C1x00, Gh)*e(C1z10, Ah)*e(G, rhoh00) and
        1==e(C2z00^(-1), Gh)*e(C2x00, Gh)*e(C2z10, Ah)*e(Q0, rhoh00) and
        1==e(C110^(-1), Ehz0)*e(C1z00, Gh)*e(C1y00, Ehs)*e(G, rhoh01) and
        1==e(C210^(-1), Ehz0)*e(C2z00, Gh)*e(C2y00, Ehs)*e(Q0, rhoh01) and
        1==e(C1z01^(-1) * (C1z11), D1x21) * e(C1z01* C1z11^(-1), D1z21)*
        e(G, pih101)*e(thet101, Gh) and
        1==e(C2z01^(-1) * C2z11, D1x21) * e(C2z01 * C2z11^(-1), D1z21)*
        e(Q1, pih101)*e(thet102, Gh) and
        1==e(C1z01^(-1) * C1z11, D2x21) * e(C1z01 * C1z11^(-1), D2z21)*
        e(G, pih102)*e(thet101, Qh1) and
        1==e(C2z01^(-1) * C2z11, D2x21) * e(C2z01 * C2z11^(-1), D2z21)*
        e(Q1, pih102)*e(thet102, Qh1) and
        1==e(C111^(-1), Ehz0)*e(C1z01, Gh)*e(C1y01, Ehs)*e(G, rhoh11) and
        1==e(C211^(-1), Ehz0)*e(C2z01, Gh)*e(C2y01, Ehs)*e(Q1, rhoh11) and
        1==e(C111^(-1), Ehz1)*e(C1z11, Gh)*e(C1y11, Ehs)*e(G, rhoh12) and
        1==e(C211^(-1), Ehz1)*e(C2z11, Gh)*e(C2y11, Ehs)*e(Q1, rhoh12) and
        1==e(Ez2^(-1), D111)*e(G, D1z21)*e(Et, D1y21)*e(rho13, Gh) and
        1==e(Ez2^(-1), D211)*e(G, D2z21)*e(Et, D2y21)*e(rho13, Qh1)
    )
    return b
