X        IF ARGS() > I THEN GOTO L
XX       PRINT "usage: ./hack.exe username"
XXX      PRINT CHR(X)
XL       END

L        DIM username AS STRING
LX       username = ARG(II)
LXX      DIM pwdcount AS INTEGER
LXXX     pwdcount = LIII

XC       DIM words(pwdcount) AS STRING
CI       words(I) = "airplane"
CII      words(II) = "alphabet"
CIII     words(III) = "aviator"
CIV      words(IV) = "bidirectional"
CV       words(V) = "changeme"
CVI      words(VI) = "creosote"
CVII     words(VII) = "cyclone"
CVIII    words(VIII) = "december"
CIX      words(IX) = "dolphin"
CX       words(X) = "elephant"
CXI      words(XI) = "ersatz"
CXII     words(XII) = "falderal"
CXIII    words(XIII) = "functional"
CXIV     words(XIV) = "future"
CXV      words(XV) = "guitar"
CXVI     words(XVI) = "gymnast"
CXVII    words(XVII) = "hello"
CXVIII   words(XVIII) = "imbroglio"
CXIX     words(XIX) = "january"
CXX      words(XX) = "joshua"
CXXI     words(XXI) = "kernel"
CXXII    words(XXII) = "kingfish"
CXXIII   words(XXIII) = "(\b.bb)(\v.vv)"
CXXIV    words(XXIV) = "millennium"
CXXV     words(XXV) = "monday"
CXXVI    words(XXVI) = "nemesis"
CXXVII   words(XXVII) = "oatmeal"
CXXVIII  words(XXVIII) = "october"
CXXIX    words(XXIX) = "paladin"
CXXX     words(XXX) = "pass"
CXXXI    words(XXXI) = "password"
CXXXII   words(XXXII) = "penguin"
CXXXIII  words(XXXIII) = "polynomial"
CXXXIV   words(XXXIV) = "popcorn"
CXXXV    words(XXXV) = "qwerty"
CXXXVI   words(XXXVI) = "sailor"
CXXXVII  words(XXXVII) = "swordfish"
CXXXVIII words(XXXVIII) = "symmetry"
CXXXIX   words(XXXIX) = "system"
CXL      words(XL) = "tattoo"
CXLI     words(XLI) = "thursday"
CXLII    words(XLII) = "tinman"
CXLIII   words(XLIII) = "topography"
CXLIV    words(XLIV) = "unicorn"
CXLV     words(XLV) = "vader"
CXLVI    words(XLVI) = "vampire"
CXLVII   words(XLVII) = "viper"
CXLVIII  words(XLVIII) = "warez"
CXLIX    words(XLIX) = "xanadu"
CL       words(L) = "xyzzy"
CLI      words(LI) = "zephyr"
CLII     words(LII) = "zeppelin"
CLIII    words(LIII) = "zxcvbnm"

D        DIM i AS INTEGER
DX       DIM p AS STRING

DXX      PRINT "Cracking user " + username + CHR(X)
DXXX     i = I

DXL      p = words(i)
DL       PRINT "... trying " + username + ":" + p + CHR(X)
DLX      IF CHECKPASS(username, p) THEN GOTO M
DLXX     i = i + I
DLXXX    IF i > pwdcount THEN GOTO DC
DXC      GOTO DXL

DC       DIM j AS INTEGER
DCX      DIM k AS INTEGER

DCC      i = I
DCCX     j = I
DCCXX    k = I

DCCL     p = words(i) + CHR(j+XLVII) + CHR(k+XLVII)
DCCLX    PRINT "... trying " + username + ":" + p + CHR(X)
DCCLXX   IF CHECKPASS(username, p) THEN GOTO M

CM       k = k + I
CMX      IF k < XI THEN GOTO DCCL

CMXX     j = j + I
CMXXX    IF j < XI THEN GOTO DCCXX

CMXL     i = i + I
CML      IF i > pwdcount THEN GOTO MM

CMLX     GOTO DCCX

M        PRINT "!!! cracked user " + username + ":" + p + CHR(X)
MX       END
MM       PRINT "no matches for user " + username + CHR(X)
