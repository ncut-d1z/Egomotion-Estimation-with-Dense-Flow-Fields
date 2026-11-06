function near=getnear_in_kugel_Drehung(setofpoints, ort, drehvektor)
% near=getnear_in_kugel_Drehung(setofpoints, ort, drehvektor)
% returns the nearnesses ("near") from a point within a sphere
% the nearnesses are computed corresponding to direction vectors stored in
% the first three entries of the rows of "setofpoints"
% "ort" gives the point within the sphere
% the direction vectors are rotated with the rotation vector "drehvektor"

[anzahl dumi]=size(setofpoints);

drehmatrix=rodrigues(drehvektor);
near=zeros(anzahl,1);
for i=1:anzahl
    n=transpose(setofpoints(i,1:3));
    n=drehmatrix*n;
    b=skalarprodukt(n,ort);
    c=skalarprodukt(ort,ort);
    wurzel=b*b-c+1;
    entf=(-1)*b+sqrt(wurzel);
    near(i)=1/entf; %eins durch entfernung
end