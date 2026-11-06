function neuekurve=kurveglaetten(kurve, fenster)
% neuekurve=kurveglaetten(kurve, fenster)
% computes the mean of the vector "kurve" at every position with the window
% "fenster"

[anzahl dumi]=size(kurve);

neuekurve=zeros(anzahl,1);
for i=(fenster+1):(anzahl-fenster)
    neuekurve(i)=mean(kurve((i-fenster):(i+fenster),1));
end
