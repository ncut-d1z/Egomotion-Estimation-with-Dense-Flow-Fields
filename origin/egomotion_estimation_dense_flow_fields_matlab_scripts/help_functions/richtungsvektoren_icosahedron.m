function setofpoints=richtungsvektoren_icosahedron(iterationszahl)
% setofpoints=richtungsvektoren_icosahedron(iterationszahl)
% returns 20*4^iterationszahl direction vectors (stored in the first three entries in a row of "setofpoints"), which are uniformly
% distributed over the sphere

anzahl=20*4^iterationszahl;


phi=(1+sqrt(5))/2;

V1=[0;0;0;0;-1;-1;1;1;-phi;phi;phi;-phi;];
V2=[-1;-1;1;1;-phi;phi;phi;-phi;0;0;0;0;];
V3=[-phi;phi;phi;-phi;0;0;0;0;-1;-1;1;1;];
        
F= [1 4 9;1 5 9;1 8 5;1 8 10;1 10 4; 12 2 5; 12 2 3; 12 3 6; 12 6 9; 12 9 5; 11 7 10; 11 10 8; 11 8 2; 11 2 3; 11 3 7; 2 5 8; 10 4 7; 3 6 7; 6 7 4; 6 4 9; ];

p1=[V1(F(1,1)) V2(F(1,1)) V3(F(1,1))];
p2=[V1(F(1,2)) V2(F(1,2)) V3(F(1,2))]; 
p3=[V1(F(1,3)) V2(F(1,3)) V3(F(1,3))];
Rueckgabeliste=findeDreiecke(p1,p2,p3,0,iterationszahl);
for i=2:20
    p1=[V1(F(i,1)) V2(F(i,1)) V3(F(i,1))];
    p2=[V1(F(i,2)) V2(F(i,2)) V3(F(i,2))]; 
    p3=[V1(F(i,3)) V2(F(i,3)) V3(F(i,3))];

    Rueckgabeliste=[Rueckgabeliste; findeDreiecke(p1,p2,p3,0,iterationszahl)];
end

setofpoints=zeros(anzahl,6);
for i=1:anzahl
    setofpoints(i,1:3)=Rueckgabeliste(i,1:3);
end
    
