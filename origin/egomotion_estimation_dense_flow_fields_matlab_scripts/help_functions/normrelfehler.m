function fehler=normrelfehler(theovektor,berechvektor)
% "fehler" returns the angle between the correct vector "theovektor" and
% the error prone vector "berechvector"

a=theovektor/sqrt(skalarprodukt(theovektor,theovektor));
b=berechvektor/sqrt(skalarprodukt(berechvektor,berechvektor));
c=skalarprodukt(a,b);
fehler=abs(acos(c)*(360/(pi*2)));


 