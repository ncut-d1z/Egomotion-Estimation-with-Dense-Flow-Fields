function drehvektor=drehvektor_aus_zwei_vektoren(v1,v2)
% drehvektor=drehvektor_aus_zwei_vektoren(v1,v2)
% returns a rotation vector, which rotates v1 into v2

normfaktor1=abs(sqrt(skalarprodukt(v1,v1)));
normfaktor2=abs(sqrt(skalarprodukt(v2,v2)));
 
drehvektor=kreuzprodukt(v1,v2);
normfaktor3=abs(sqrt(skalarprodukt(drehvektor,drehvektor)));
if normfaktor3>0.00000000001
    drehvektor=drehvektor/normfaktor3;
end
coswinkel=skalarprodukt(v1,v2)/(normfaktor1*normfaktor2);
winkel=acos(coswinkel);

drehvektor=drehvektor*winkel;