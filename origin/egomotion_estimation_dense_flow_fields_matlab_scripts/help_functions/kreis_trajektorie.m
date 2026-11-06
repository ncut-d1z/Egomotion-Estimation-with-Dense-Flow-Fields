function punkte=kreis_trajektorie(stellen, parameter, hoehe)
% punkte=kreis_trajektorie(stellen, parameter, hoehe)
% gives trajectory points stored in "punkte" of a circel
% "stellen" gives the number of points
% "parameter" gives the radius of the circel
% "hoehe" defines the height of the circel

punkte=zeros(stellen,3);
kugelradius=1;
r=kugelradius*parameter;

for i=1:stellen
    w=2*pi*(i-1)/stellen;
    punkte(i,1)=r*cos(w);
    punkte(i,2)=r*sin(w);
    punkte(i,3)=hoehe;
end
line(punkte(:,1),punkte(:,2),punkte(:,3));