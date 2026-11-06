function zellen=zellendefinieren
% zellen=zellendefinieren
% gives the direction vectors (stored in "zellen") for a simple matched filter for egomotion
% estimation

anzahlzellen=3;
zellen=zeros(anzahlzellen,10);
vektor=[1 0 0];
zellen(1,1:3)=vektor;
zellen(1,6:8)=vektor;
vektor=[0 1 0];
zellen(2,1:3)=vektor;
zellen(2,6:8)=vektor;
vektor=[0 0 1];
zellen(3,1:3)=vektor;
zellen(3,6:8)=vektor;
