function [theta phi]=winkelausvektor(vektor)
% [theta phi]=winkelausvektor(vektor)
% compute the angles "theta" and "phi" of a spherical coordinate system
% from a vector ("vektor")

phi=atan2(vektor(2),vektor(1));
theta=acos(vektor(3));