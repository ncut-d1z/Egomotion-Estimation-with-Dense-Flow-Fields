function [newsetofpoints durchtransfluss durchrotfluss]=erzeugeflussausnear(setofpoints, near, theotrans, theorot)
% [newsetofpoints durchtransfluss durchrotfluss]=erzeugeflussausnear(setofpoints, near, theotrans, theorot)
% computes 3d optic flow vectors from direction vectors, the corresponding
% nearness (stored in "near"), a given translation ("theotrans") and a
% given rotation ("theorot")
% "setofpoints" must be a n*6 matrix, where the first three entries of the
% matrix in a row gives the direction vector
% the last three entries in row of "newsetofpoints" stored the 3d optical
% flow, where the first three entries are identical with "setofpoints"
% "durchtransfluss" stored the average translational flow
% "durchrotfluss" stored the average rotational flow

[anzahl dumi]=size(setofpoints);
newsetofpoints=setofpoints;
durchtransfluss=0;
durchrotfluss=0;
for i=1:anzahl
    d=transpose(setofpoints(i,1:3));
    transfluss=(-1)*near(i)*(theotrans-(skalarprodukt(theotrans,d)*d));
    rotfluss=(-1)*kreuzprodukt(theorot,d);
    p=transfluss+rotfluss;
    newsetofpoints(i,4)=p(1);
    newsetofpoints(i,5)=p(2);
    newsetofpoints(i,6)=p(3);
    durchtransfluss=durchtransfluss+abs(sqrt(skalarprodukt(transfluss,transfluss)));
    durchrotfluss=durchrotfluss+abs(sqrt(skalarprodukt(rotfluss,rotfluss)));
end
durchtransfluss=durchtransfluss/anzahl;
durchrotfluss=durchrotfluss/anzahl;


