function punkte=sinuskurve_trajektorie(stellen, amplithude, laenge, anzahlperioden, hoehe)
% punkte=sinuskurve_trajektorie(stellen, amplithude, laenge, anzahlperioden, hoehe)
% gives trajectory points stored in "punkte" of a sinosodial curve
% "stellen" gives the number of points
% "amplithude" gives the amplithude of the curve
% "laenge" gives the length of the curve
% "anzahlperioden" gives the number of periodes
% "hoehe" defines the height of the circel


punkte=zeros(stellen,3);

for i=1:stellen
    w=2*pi*(i-1)*anzahlperioden/stellen;
    punkte(i,1)=laenge*(i-1)/stellen-laenge/2;
    punkte(i,2)=amplithude*sin(w);
    punkte(i,3)=hoehe;
end
line(punkte(:,1),punkte(:,2),punkte(:,3));




