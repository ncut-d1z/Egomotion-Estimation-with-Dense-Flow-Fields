function ergebnis=kreuzprodukt(a,b)
% ergebnis=kreuzprodukt(a,b)
% "ergebnis" stored the cross product between the vectors "a" and "b"

ergebnis=zeros(3,1);

ergebnis(1)=a(2)*b(3)-b(2)*a(3);
ergebnis(2)=a(3)*b(1)-b(3)*a(1);
ergebnis(3)=a(1)*b(2)-b(1)*a(2);
