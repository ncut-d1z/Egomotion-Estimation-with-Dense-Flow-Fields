function error=transfehler_div(theotrans, berechtrans)
% error=transfehler_div(theotrans, berechtrans)
% gives the error ("error") between the vectors "theotrans" and
% "berechtrans"
% the error is the pointswise squared error, where the length of
% "berechtrans" are normed to "theotrans"

theonorm=abs(sqrt(skalarprodukt(theotrans,theotrans)));
berechnorm=abs(sqrt(skalarprodukt(berechtrans,berechtrans)));

%Umnormierung von berechtrans
berechtrans=berechtrans*(theonorm/berechnorm);

div=theotrans-berechtrans;

error=abs(sqrt(skalarprodukt(div,div)));

