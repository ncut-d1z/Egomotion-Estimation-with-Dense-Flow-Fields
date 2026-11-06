function [trans rot]=franzeigen(setofpoints)
% [trans rot]=franzeigen(setofpoints)
% a simple matched filter, which computes the ego translation ("trans") and
% ego rotation ("rot") from direction vectors and corresponding 3d flow
% vectors, both must be stored in one row of "setofpoints"


%setofpoints=fiducialpoints(Feld,Schranke);
%[perfekttrans perfektrot setofpoints]=perfectpoints;
[anzahl dumi]=size(setofpoints);
zellen=zellendefinieren;
[anzahlzellen dumi]=size(zellen);
for i=1:anzahl
    d=transpose(setofpoints(i,1:3));
    p=transpose(setofpoints(i,4:6));
    for j=1:anzahlzellen
        zellentrans=transpose(zellen(j,1:3));%m�ssen normiert sein
        zellenrot=transpose(zellen(j,6:8));%m�ssen normiert sein
        
        vorzugtrans=(-1)*(zellentrans-(skalarprodukt(zellentrans,d)*d));
        zellen(j,5)=zellen(j,5)+skalarprodukt(vorzugtrans,vorzugtrans);%gesamtnormierung
        zellen(j,4)=zellen(j,4)+skalarprodukt(vorzugtrans,p);%aufsummierung der zellantwort
        
        vorzugrot=(-1)*kreuzprodukt(zellenrot,d);
        zellen(j,10)=zellen(j,10)+skalarprodukt(vorzugrot,vorzugrot);%gesamtnormierung
        zellen(j,9)=zellen(j,9)+skalarprodukt(vorzugrot,p);%aufsummierung der zellantwort
    end
end
trans=zeros(3,1);
rot=zeros(3,1);
for j=1:anzahlzellen
    trans=trans+(transpose(zellen(j,1:3))*zellen(j,4))/zellen(j,5);
    rot=rot+(transpose(zellen(j,6:8))*zellen(j,9))/zellen(j,10); 
end
trans=(trans*3)/anzahlzellen;
rot=(rot*3)/anzahlzellen;

%trans=trans/abs(sqrt(skalarprodukt(trans,trans)));

%franzrot=rot
%franztrans=trans
%fehlertrans=sqrt(skalarprodukt(perfekttrans-trans,perfekttrans-trans))/sqrt(skalarprodukt(perfekttrans,perfekttrans))
%fehlerrot=sqrt(skalarprodukt(perfektrot-rot,perfektrot-rot))/sqrt(skalarprodukt(perfektrot,perfektrot)) 