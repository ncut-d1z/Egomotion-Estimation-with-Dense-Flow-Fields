function setofpoints=erzeugerichtungsvektoren_loch(iterationszahl)
% setofpoints=erzeugerichtungsvektoren_loch(iterationszahl)
% returns 6*4^iterationszahl direction vectors (stored in the first three entries in a row of "setofpoints"), which are uniformly
% distributed over the sphere except of two regions in the upper part

p1=[0 0 1];
p2=[0 0 -1];
p3=[1 0 0];
p4=[-1 0 0];
p5=[0 1 0];
p6=[0 -1 0];

[Rueckgabeliste1]=findeDreiecke(p1,p3,p5,0,iterationszahl);
[Rueckgabeliste2]=findeDreiecke(p1,p3,p6,0,iterationszahl);
[Rueckgabeliste3]=findeDreiecke(p1,p4,p5,0,iterationszahl);
[Rueckgabeliste4]=findeDreiecke(p1,p4,p6,0,iterationszahl);

[Rueckgabeliste5]=findeDreiecke(p2,p3,p5,0,iterationszahl);
[Rueckgabeliste6]=findeDreiecke(p2,p3,p6,0,iterationszahl);
[Rueckgabeliste7]=findeDreiecke(p2,p4,p5,0,iterationszahl);
[Rueckgabeliste8]=findeDreiecke(p2,p4,p6,0,iterationszahl);

Rueckgabeliste=[Rueckgabeliste2; Rueckgabeliste3; Rueckgabeliste5; Rueckgabeliste6; Rueckgabeliste7; Rueckgabeliste8];
%Rueckgabeliste=[Rueckgabeliste1; Rueckgabeliste2; Rueckgabeliste3; Rueckgabeliste4; Rueckgabeliste5; Rueckgabeliste6; Rueckgabeliste7; Rueckgabeliste8];

[anzahl dumi]=size(Rueckgabeliste);
setofpoints=zeros(anzahl,6);
for i=1:anzahl
    setofpoints(i,1:3)=Rueckgabeliste(i,1:3);
end

%plot3(Rueckgabeliste(:,1),Rueckgabeliste(:,2),Rueckgabeliste(:,3),'.');
    
