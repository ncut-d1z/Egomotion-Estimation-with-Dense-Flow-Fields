function [neukoeff trans rot error]=oneit_adaptiv_matched_filter(altkoeff, setofpoints)
% [neukoeff trans rot error]=oneit_adaptiv_matched_filter(altkoeff, setofpoints)
% this is one iteration of a modified version of the Koenderink van Doorn
% algorithm, where the computations are done on a spherical harmonic
% expansion of the nearnesses
% "altkoeff" are given coefficients of the spherical harmonics
% "neukoeff" are the updated coefficients of the spherical harmonics after
% one iteration
% "setofpoints" stores the direction and the corresponding flow vectors in
% a row
% "trans" is the ego translation
% "rot" is the ego rotation
% "error" is the error done by this algorithm


[anzahl dumi]=size(setofpoints);

% near aus koeff
near=near_aus_koeff(altkoeff, setofpoints);
sum=0;
for i=1:anzahl
    sum=sum+near(i);
end
sum=sum/anzahl;
if sum<0 %verhindert das falsche Vorzeichen bei der Translation
   near=near*(-1);
end
near=near/sum;



%Rechteseite
%flusstransberechnung 
a=zeros(3,1);
for i=1:anzahl
    p=transpose(setofpoints(i,4:6));
    a=a-p;
end
flusstrans=a/anzahl;
   
%flussrotberechnung 
a=zeros(3,1);
for i=1:anzahl
    d=transpose(setofpoints(i,1:3));
    p=transpose(setofpoints(i,4:6));
    a=a+kreuzprodukt(p,d);
end
flussrot=a/anzahl;
   
%vektorgesamt
vekgesamt=[flusstrans; flussrot];
  
   
%Linkeseite
%m1
m=zeros(3,3);
for i=1:anzahl
    d=transpose(setofpoints(i,1:3));
    m=m+near(i)*(d*d');
end
m1=eye(3,3)-m/anzahl;

%m4
m=zeros(3,3);
for i=1:anzahl
    d=transpose(setofpoints(i,1:3));
    m=m+(d*d');
end
m4=eye(3,3)-m/anzahl;
    
%m2
a=zeros(3,1);
for i=1:anzahl
    d=transpose(setofpoints(i,1:3));
    a=a-d;%hier minus
end
a=a/anzahl;
m2=erzeugekreuzproduktmatrix(a);
   
%m3
a=zeros(3,1);
for i=1:anzahl
    d=transpose(setofpoints(i,1:3));
    a=a+near(i)*d;%hier plus
end
a=a/anzahl;
m3=erzeugekreuzproduktmatrix(a);
    
%M gesamt
mgesamt=[m1 m2; m3 m4];
invmgesamt=real(inv(mgesamt));
%Determinante=det(mgesamt)
    
%Eigenbewegung
eigenbewegung=invmgesamt*vekgesamt;

%Zuordnung
    
transneu=eigenbewegung(1:3,1);
normfaktor=abs(sqrt(skalarprodukt(transneu,transneu)));
transneu=transneu/normfaktor;
rotneu=eigenbewegung(4:6,1);
    
trans=transneu;
rot=rotneu;

%Error berechnung
error=0;
for i=1:anzahl
    d=transpose(setofpoints(i,1:3));
    p=transpose(setofpoints(i,4:6));
        
    pstrich=((-1)*near(i)*sum*(trans-(skalarprodukt(trans,d)*d)))-kreuzprodukt(rot,d);%hier taucht sum wieder auf da beide normiert
    error=error+abs(skalarprodukt(p-pstrich,p-pstrich));
end
error=error/anzahl;
oneit_error=sqrt(error);

%near Neuberechnung
nulldiv=0.00000000001;
for i=1:anzahl
    d=transpose(setofpoints(i,1:3));
    p=transpose(setofpoints(i,4:6));
    near(i)=(-1)*skalarprodukt(trans,p-kreuzprodukt(d,rot))/((1-skalarprodukt(trans,d)^2)+nulldiv);
end

%Koeffberechnung
ordnung=2;
neukoeff=koeff_aus_near(near, setofpoints, ordnung);


