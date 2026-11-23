(* 三条独立公式路线；一般非零实四元数覆盖SO(3)，不是抽样。 *)
Module[{q0,q1,q2,q3,x,y,z,variables,scale,argument,permutation,block,
  be,p,ir,irMatrix,xyz,polynomials,tensors,direct,axes,real,imag,complexOne,
  de,term,choi,complexTwo,w,converted,oneConverted,zeroCount,irResidual,
  choiResidual,actionResidual,orthogonality,gramResidual,detResidual,
  dipoleIrResidual,dipoleChoiResidual,paperOne,paperTwo,badMatrix,rx45,result},
 variables={q0,q1,q2,q3}; scale=Total[variables^2];
 (* argument/scale才是单位旋转。三路二阶公式均齐次二次，因此可清分母。 *)
 argument={{q0^2+q1^2-q2^2-q3^2,2(q1 q2-q0 q3),2(q1 q3+q0 q2)},
  {2(q1 q2+q0 q3),q0^2-q1^2+q2^2-q3^2,2(q2 q3-q0 q1)},
  {2(q1 q3-q0 q2),2(q2 q3+q0 q1),q0^2-q1^2-q2^2+q3^2}};
 permutation={{0,1,0},{0,0,1},{1,0,0}};
 block=permutation.Transpose[argument].Transpose[permutation];
 be[i_,j_]:=block[[i+2,j+2]];
 p[i_,a_,n_]:=Which[n==-2,be[i,1]be[a,-1]+be[i,-1]be[a,1],
  n==2,be[i,1]be[a,1]-be[i,-1]be[a,-1],True,be[i,0]be[a,n]];
 ir[m_,n_,bad_:False]:=Module[{den=If[Abs[n]==2,12,4-n^2],d=Boole[m==0],u,v,del,vv},
  u=Sqrt[(4-m^2)/den];v=(1-2d)Sqrt[(1+d)(1+Abs[m])(2+Abs[m])/den]/2;
  vv=Which[m==0,p[1,1,n]+p[-1,-1,n],
   m>0,del=Boole[m==1];Sqrt[1+del]p[1,m-1,n]-(1-del)p[-1,-m+1,n],
   True,del=Boole[m==-1];(1-del)p[1,m+1,n]+Sqrt[If[bad,1-del,1+del]]p[-1,-m-1,n]];
  If[u==0,0,u p[0,m,n]]+v vv];
 irMatrix=Table[Expand[ir[m,n]],{m,-2,2},{n,-2,2}];
 (* 路A：从基函数Hessian提取对称无迹张量。绝不调用p/ir/choi。 *)
 xyz={x,y,z};
 polynomials={Sqrt[15/(4Pi)]x y,Sqrt[15/(4Pi)]y z,
  Sqrt[5/(16Pi)](2z^2-x^2-y^2),Sqrt[15/(4Pi)]x z,
  Sqrt[15/(16Pi)](x^2-y^2)};
 tensors=Table[Table[D[polynomials[[k]],xyz[[i]],xyz[[j]]]/2,{i,3},{j,3}],{k,5}];
 direct=Table[Expand[(8Pi/15)Tr[tensors[[i]].Transpose[argument].tensors[[j]].argument]],
  {i,5},{j,5}];
 (* 路C：Choi 1999式5.4/5.5及6.1/6.9/6.14，独立建立复递推。 *)
 axes=Transpose[argument];
 real={{(axes[[2,2]]+axes[[1,1]])/2,axes[[1,3]]/Sqrt[2],(axes[[2,2]]-axes[[1,1]])/2},
  {axes[[3,1]]/Sqrt[2],axes[[3,3]],-axes[[3,1]]/Sqrt[2]},
  {(axes[[2,2]]-axes[[1,1]])/2,-axes[[1,3]]/Sqrt[2],(axes[[2,2]]+axes[[1,1]])/2}};
 imag={{(axes[[2,1]]-axes[[1,2]])/2,axes[[2,3]]/Sqrt[2],-(axes[[2,1]]+axes[[1,2]])/2},
  {-axes[[3,2]]/Sqrt[2],0,-axes[[3,2]]/Sqrt[2]},
  {(axes[[2,1]]+axes[[1,2]])/2,axes[[2,3]]/Sqrt[2],(axes[[1,2]]-axes[[2,1]])/2}};
 complexOne=real+I imag;de[i_,j_]:=complexOne[[i+2,j+2]];
 term[weight_,i_,j_,a_,b_]:=If[weight==0,0,weight de[i,j]de[a,b]];
 choi[m_,n_]:=If[Abs[n]<2,
  term[Sqrt[(4-m^2)/(4-n^2)],0,0,m,n]+
   term[Sqrt[(2+m)(1+m)/(2(4-n^2))],1,0,m-1,n]+
   term[Sqrt[(2-m)(1-m)/(2(4-n^2))],-1,0,m+1,n],
  With[{sg=Sign[n],nn=n-Sign[n]},
   term[Sqrt[2(4-m^2)/12],0,sg,m,nn]+
   term[Sqrt[(2+m)(1+m)/12],1,sg,m-1,nn]+
   term[Sqrt[(2-m)(1-m)/12],-1,sg,m+1,nn]]];
 complexTwo=Table[Expand[choi[m,n]],{m,-2,2},{n,-2,2}];
 w[l_]:=Module[{matrix=ConstantArray[0,{2l+1,2l+1}]},matrix[[l+1,l+1]]=1;
  Do[matrix[[l+1-m,l+1-m]]=-I/Sqrt[2];
   matrix[[l+1-m,l+1+m]]=I(-1)^m/Sqrt[2];
   matrix[[l+1+m,l+1-m]]=1/Sqrt[2];
   matrix[[l+1+m,l+1+m]]=(-1)^m/Sqrt[2],{m,1,l}];matrix];
 converted=Expand[w[2].complexTwo.ConjugateTranspose[w[2]]];
 oneConverted=Expand[w[1].complexOne.ConjugateTranspose[w[1]]];
 (* 与C++中零基置换{2,0,1}/{2,3,1,4,0}一致，行列同时转换。 *)
 paperOne={3,1,2};paperTwo={3,4,2,5,1};
 dipoleIrResidual=Expand[block[[paperOne,paperOne]]-Transpose[argument]];
 dipoleChoiResidual=Expand[oneConverted[[paperOne,paperOne]]-Transpose[argument]];
 irResidual=Expand[(irMatrix-direct)[[paperTwo,paperTwo]]];
 choiResidual=Expand[(converted-direct)[[paperTwo,paperTwo]]];
 actionResidual=Expand[Transpose[direct].polynomials-(polynomials/.Thread[xyz->argument.xyz])];
 orthogonality=Expand[Transpose[argument].argument-scale^2 IdentityMatrix[3]];
 detResidual=Expand[Det[argument]-scale^3];
 gramResidual=Expand[Transpose[direct].direct-scale^4 IdentityMatrix[5]];
 zeroCount[v_]:=Count[Flatten[v],0];
 badMatrix=Expand[Table[ir[m,n,True],{m,-2,2},{n,-2,2}]-direct];
 rx45=FullSimplify[({ir[-1,0],ir[-1,0,True]}/scale^2)/.
  {q0->Sqrt[2+Sqrt[2]]/2,q1->Sqrt[2-Sqrt[2]]/2,q2->0,q3->0}];
 result=<|"kernel"->$Version,"domain"->"real q!=0; A=argument/(q.q); all SO(3)",
  "rotationOrthogonalityZeros"->zeroCount[orthogonality],"rotationDeterminantResidual"->detResidual,
  "dipoleIrZeros"->zeroCount[dipoleIrResidual],"dipoleChoiZeros"->zeroCount[dipoleChoiResidual],
  "quadrupoleIrZeros"->zeroCount[irResidual],"quadrupoleChoiZeros"->zeroCount[choiResidual],
  "fieldActionZeros"->zeroCount[actionResidual],"quadrupoleOrthogonalityZeros"->zeroCount[gramResidual],
  "printedMinusFactorRejected"->(zeroCount[badMatrix]<25),"x45CorrectAndPrinted"->rx45,
  "allExact"->(orthogonality===ConstantArray[0,{3,3}]&&detResidual===0&&
   dipoleIrResidual===ConstantArray[0,{3,3}]&&dipoleChoiResidual===ConstantArray[0,{3,3}]&&
   irResidual===ConstantArray[0,{5,5}]&&choiResidual===ConstantArray[0,{5,5}]&&
   actionResidual===ConstantArray[0,5]&&gramResidual===ConstantArray[0,{5,5}])|>;
 If[TrueQ[result["allExact"]],result,Failure["SymbolicIdentityFailed",result]]
]
