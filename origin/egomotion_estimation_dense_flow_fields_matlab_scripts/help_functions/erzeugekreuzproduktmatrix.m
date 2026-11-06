function m=erzeugekreuzproduktmatrix(v)
% m=erzeugekreuzproduktmatrix(v)
% returns a matrix "m" that the cross product between "v" and abritary
% vector x can be written as m*x

m=[0 (-1)*v(3) v(2); v(3) 0 (-1)*v(1); (-1)*v(2) v(1) 0];