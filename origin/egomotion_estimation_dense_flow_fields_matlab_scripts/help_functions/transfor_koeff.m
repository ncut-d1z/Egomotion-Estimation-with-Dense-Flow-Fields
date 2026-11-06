function koeffneu=transfor_koeff(koeffalt, drehmatrix)
% koeffneu=transfor_koeff(koeffalt, drehmatrix)
% rotate the coefficients of spherical harmonics
% "drehmatrix" is the rotation matrix
% "koeffalt" are the given coefficients
% "koeffneu" are the rotated coefficients


%[A3 C3 B3 A5 C5 B5]=sphericaltravomatrices(drehmatrix);

%D3=B3*inv(A3)*C3*A3*inv(B3);

%D5=B5*inv(A5)*C5*A5*inv(B5);


[R] = SHRotate(drehmatrix, 2);
R3=R{2};
R5=R{3};

koeffneu=zeros(3,5);
koeffneu(1,1)=koeffalt(1,1);
%koeffneu(2,1:3)=(inv(D3)*koeffalt(2,1:3)')';
%koeffneu(3,1:5)=(inv(D5)*koeffalt(3,1:5)')';
koeffneu(2,1:3)=(R3*koeffalt(2,1:3)')';
koeffneu(3,1:5)=(R5*koeffalt(3,1:5)')';