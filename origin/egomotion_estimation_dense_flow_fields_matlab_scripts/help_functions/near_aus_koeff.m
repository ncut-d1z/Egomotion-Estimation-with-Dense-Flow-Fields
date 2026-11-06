function nearneu=near_aus_koeff(koeff, setofpoints)
% nearneu=near_aus_koeff(koeff, setofpoints)
% compute the nearnesses ("nearneu") correponding to direction vectors
% stored in the first three entries of a row of "setofpoints" given the
% coefficients of the spherical harmonics
% the rows of "koeff" give the order of the coefficient, where the coloums
% give the degree of the coefficients


[anzahl dumi]=size(setofpoints);
[dumi1 dumi2]=size(koeff);
ordnung=dumi1-1;

alletheta=zeros(anzahl,1);
allephi=zeros(anzahl,1);
nearneu=zeros(anzahl,1);

for i=1:anzahl %Winkelberechnung
    vektor=setofpoints(i,1:3);
    [theta phi]=winkelausvektor(vektor);
    
    alletheta(i)=theta;
    allephi(i)=phi;
end

for i=1:anzahl
 
    nearneu(i)=0;
    
    for l=1:(ordnung+1)
        
          for m=1:((l-1)*2+1)
            lreal=l-1;
            mreal=(m-lreal)-1;
        
            nearneu(i)=nearneu(i)+koeff(l,m)*kugelflfunc2(lreal,mreal,alletheta(i),allephi(i));
         
          end
        
    end
end




