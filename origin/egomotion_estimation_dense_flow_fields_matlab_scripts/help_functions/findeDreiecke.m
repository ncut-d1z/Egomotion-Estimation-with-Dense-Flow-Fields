function [Rueckgabeliste]=findeDreiecke(x,y,z,iteration,ende)
% [Rueckgabeliste]=findeDreiecke(x,y,z,iteration,ende)
% this is a rekursive function, which fragments the triangle "x", "y", "z"
% in four new triangles, until "iteration" equals "ende"
% the rows of "Rueckgabeliste" stores direction vectors pointing to the
% middle of each triangle


xy=(x+y)/2;
xy=xy/sqrt(xy*xy');
xz=(x+z)/2;
xz=xz/sqrt(xz*xz');
yz=(y+z)/2;
yz=yz/sqrt(yz*yz');

if iteration<ende
   iteration=iteration+1;
   [listeA]=findeDreiecke(x,xz,xy,iteration,ende);
   [listeB]=findeDreiecke(y,xy,yz,iteration,ende);
   [listeC]=findeDreiecke(z,xz,yz,iteration,ende);
   [listeD]=findeDreiecke(xy,xz,yz,iteration,ende);
   
   Rueckgabeliste=[listeA; listeB; listeC; listeD];
else
   p=(x+y+z)/3;
   p=p/sqrt(skalarprodukt(p,p));
   Rueckgabeliste=[p];
end
   
