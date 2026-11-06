function [theotrans theorot setofpoints realsetofpoints near durchlaenge]=fixed_kugel_symmetric_perfectpoints_randomnear(iteration,fehlerprozent,rotzutrans,Parameter,theotrans,theorot)
% [theotrans theorot setofpoints realsetofpoints near durchlaenge]=fixed_kugel_symmetric_perfectpoints_randomnear(iteration,fehlerprozent,rotzutrans,Parameter,theotrans,theorot)
% computes direction and optical flow vectors from a given ego translation
% ("theotrans") and a given ego rotation ("theorot")
% the direction vectors are uniformly distributed over the sphere, with two
% occluded regions, the number of direction vectors are 6*4^"iteration"
% "realsetofpoints" stores the perfect optical flow, where in "setofpoints"
% an error are added to the flow vectors
% the error is uniformly gaussian and scaled with "fehlerprozent"
% "rotzutrans" and "Parameter" are not used
% "near" stored the nearnesses, which are computed randomly between a
% maximum and minimum
% "durchlaenge" stores the average length of the flow vectors


norot=0;
%anzahl=5000;
%fehlerprozent=0.0;
minentf=1;
maxentf=4;
entfdurch=(maxentf-minentf)/2+minentf;
 

%[theotrans,theorot]=gettransandrotvektor(drehachseegovektor,nummer);
%theotrans=getvektor;
%if norot==1
%    theorot=zeros(3,1);
%else
%    theorot=rotzutrans*getvektor;
%end
%transunnorm=transpose([4 3 6]);
%grosst=abs(sqrt(skalarprodukt(transunnorm,transunnorm)));
%theotrans=transunnorm/grosst;
%theorot=transpose([0 0 3]);

setofpoints=erzeugerichtungsvektoren_loch(iteration);%mit Loch
[anzahl, dumi]=size(setofpoints); 
realsetofpoints=setofpoints;
%near=zeros(anzahl,1);

%grossr=abs(sqrt(skalarprodukt(theorot,theorot)));
%transzurot=(grosst/((maxentf-minentf)/2+minentf))/grossr

%near=getnear_in_kugel(setofpoints, Parameter);


for i=1:anzahl
    d=transpose(setofpoints(i,1:3));
    entf=(rand*(maxentf-minentf)+minentf)/entfdurch;
    near=1/entf;
    p=((-1)*near*(theotrans-(skalarprodukt(theotrans,d)*d)))-kreuzprodukt(theorot,d);
    realsetofpoints(i,4)=p(1);
    realsetofpoints(i,5)=p(2);
    realsetofpoints(i,6)=p(3);
end

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
    setofpoints(i,4)=p(1);
    setofpoints(i,5)=p(2);
    setofpoints(i,6)=p(3);
end
