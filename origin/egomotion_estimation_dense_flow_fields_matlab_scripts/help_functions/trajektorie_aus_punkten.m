function [alletrans absdrehvektoren reldrehvektoren]=trajektorie_aus_punkten(punkte)
% [alletrans absdrehvektoren reldrehvektoren]=trajektorie_aus_punkten(punkte)
% computes translation vectors ("alletrans") and rotation vectors
% ("absdrehvektoren" and "reldrehvektoren") from series of points ("punkte")
% the seeing direction is from the current point to the next point
% "absdrehvektoren" are the rotation vectors from the start point and
% "reldrehvektoren" are the rotation vector from point to point

[anzahl dumi]=size(punkte);
vabs=[1; 0;0];

alletrans=zeros(anzahl,3);
absdrehvektoren=zeros(anzahl,3);
reldrehvektoren=zeros(anzahl,3);
for i=2:(anzahl-1)
    altpunkt=punkte(i-1,:)';
    akpunkt=punkte(i,:)';
    neupunkt=punkte(i+1,:)';
    
    v1=akpunkt-altpunkt;
    v2=neupunkt-akpunkt;
    
    abstrans=v2;
    absrot=drehvektor_aus_zwei_vektoren(v1,v2);
    
    absdrehvektoren(i,:)=(drehvektor_aus_zwei_vektoren(vabs,v1))';
    drehmatrix=rodrigues(absdrehvektoren(i,:)');
    
    alletrans(i,:)=((drehmatrix)'*abstrans)';
    reldrehvektoren(i,:)=((drehmatrix)'*absrot)';
end
dumi='Ende'
