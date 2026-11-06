function sinosodial_curve_in_the_sphere
% sinosodial_curve_in_the_sphere
% this function illustrate the sinosodial curve in the sphere used for the
% adaptive matched filter test


stellen=600; 
  
hoehe=0.3; %auch für sinuskurve
kreisradius=0.6;

% für sinuskurve
amplithude=0.5;
laengebahn=1;
anzahlperioden=2;
punkte=sinuskurve_trajektorie(stellen, amplithude, laengebahn, anzahlperioden, hoehe);
%punkte=kreis_trajektorie(stellen, kreisradius, hoehe);

figure;
hold on;

R=1;
quality=50;

[x,y,z]=sphere(quality);
x=R*x;
y=R*y;
z=R*z;
z(find(y<=0))=NaN;
surf(x,y,z);
axis image; 

%[X,Y,Z]=zirkelphi(1);
%line(X,Y,Z);

%[X,Y,Z]=zirkeltheta(1);
%line(X,Y,Z);

%[X,Y,Z]=zirkeltheta2(1);
%line(X,Y,Z);
 
line(punkte(:,1),punkte(:,2),punkte(:,3));
 
hold off;
title('circular trajectory within the sphere');
%title('sinusoidal trajectory within the sphere');