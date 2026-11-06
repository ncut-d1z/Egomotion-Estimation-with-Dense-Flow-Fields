function ergebnis=kugelflfunc2(l,m,theta,phi)
% ergebnis=kugelflfunc2(l,m,theta,phi)
% "ergebnis" gives the value of a single point of a spherical harmonic
% function
% "l" and "m" are the order and degree of the function
% "theta" and "phi" gives the angles of the specific point

betragm=abs(m);
werte=legendre(l,cos(theta));
lg=werte(betragm+1);

faktor=factorial(l-betragm)/factorial(l+betragm);
faktor=faktor*((2*l+1)/(4*pi));
faktor=sqrt(faktor);

if m<0
    ergebnis=sqrt(2)*faktor*lg*sin(betragm*phi);
elseif m>0
    ergebnis=sqrt(2)*faktor*lg*cos(betragm*phi);
elseif m==0
    ergebnis=faktor*lg;
end

