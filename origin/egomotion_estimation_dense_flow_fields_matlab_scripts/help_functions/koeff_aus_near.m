function koeff=koeff_aus_near(near, setofpoints, ordnung)
% koeff=koeff_aus_near(near, setofpoints, ordnung)
% compute the coefficients of the spherical harmonics
% given the nearnesses ("near") correponding to direction vectors
% stored in the first three entries of a row of "setofpoints"
% "ordnung" gives the order to which the koefficients are computed
% the rows of "koeff" store the order of the coefficient, where the coloums
% store the degree of the coefficients


[anzahl dumi]=size(setofpoints);

alletheta=zeros(anzahl,1);
allephi=zeros(anzahl,1);
for i=1:anzahl %Winkelberechnung
    vektor=setofpoints(i,1:3);
    [theta phi]=winkelausvektor(vektor);
    
    alletheta(i)=theta;
    allephi(i)=phi;
end

a=zeros(ordnung+1,ordnung*2+1);
b=zeros(ordnung+1,ordnung*2+1);
for l=1:(ordnung+1)
    for m=1:((l-1)*2+1)
        lreal=l-1;
        mreal=(m-lreal)-1;
        
        a(l,m)=0;
        b(l,m)=0;
        for i=1:anzahl
            a(l,m)=a(l,m)+kugelflfunc2(lreal,mreal,alletheta(i),allephi(i))*near(i);%relle Kugelflächenfunktionen
            b(l,m)=b(l,m)+kugelflfunc2(lreal,mreal,alletheta(i),allephi(i))*kugelflfunc2(lreal,mreal,alletheta(i),allephi(i));%Normierung
        end
        a(l,m)=(a(l,m)/anzahl)*4*pi;
        b(l,m)=(b(l,m)/anzahl)*4*pi;
        a(l,m)=a(l,m)/b(l,m);
    
 
    end
end
koeff=a;



