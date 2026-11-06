function einheitsvektor=getvektor
% einheitsvektor=getvektor
% returns a 3d unit vektor, which is randomly distributed in a spherical
% coordinate system

theta=(pi/2)-asin(2*rand(1)-1);
phi=rand(1)*2*pi;
einheitsvektor=zeros(3,1);
einheitsvektor(1)=sin(theta)*cos(phi);
einheitsvektor(2)=sin(theta)*sin(phi);
einheitsvektor(3)=cos(theta);