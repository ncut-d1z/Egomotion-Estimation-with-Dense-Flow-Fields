function newsetofpoints=nonpropfehler(setofpoints, fehlerprozent)
% newsetofpoints=nonpropfehler(setofpoints, fehlerprozent)
% add an error to the flow vectors stored in the last three entries of a
% row of "setofpoints". The error is gaussian.
% "fehlerprozent" is the scale factor of the error
% "newsetofpoint" returns the new flow vectors

[anzahl dumi]=size(setofpoints);
realsetofpoints=setofpoints;
newsetofpoints=setofpoints;
durchlaenge=0;
for i=1:anzahl
    p=transpose(realsetofpoints(i,4:6));
    laenge=sqrt(skalarprodukt(p,p));
    durchlaenge=durchlaenge+laenge;
end
durchlaenge=durchlaenge/anzahl;
    

for i=1:anzahl
    d=transpose(realsetofpoints(i,1:3));
    p=transpose(realsetofpoints(i,4:6));
    p(1)=p(1)+randn*fehlerprozent*durchlaenge;%zufallsfehler
    p(2)=p(2)+randn*fehlerprozent*durchlaenge;%zufallsfehler
    p(3)=p(3)+randn*fehlerprozent*durchlaenge;%zufallsfehler
    p=p-skalarprodukt(p,d)*d;
    newsetofpoints(i,4)=p(1);
    newsetofpoints(i,5)=p(2);
    newsetofpoints(i,6)=p(3);
end
 