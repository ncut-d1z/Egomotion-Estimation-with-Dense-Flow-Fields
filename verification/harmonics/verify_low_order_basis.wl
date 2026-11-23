(* 仅l=0,1,2符号验证；不调用MATLAB或C++。以Module隔离，避免清空用户内核。 *)
Module[{x,y,z,theta,phi,xyz,basis,angles,paper,fromDefinition,yc,
  onSphere,moment,sphereIntegral,gram,laplacians,definitionResiduals,
  recurrence,recurrenceResiduals,dd,dyadicResiduals,printedXX,printedYY,
  n00,n10,n20,n21,n22,p00,p10,p11c,p11s,p20,p21c,p21s,p22c,p22s,result},
 xyz={x,y,z};
 basis={1/Sqrt[4 Pi],Sqrt[3/(4 Pi)]x,Sqrt[3/(4 Pi)]y,Sqrt[3/(4 Pi)]z,
  Sqrt[5/(16 Pi)](2z^2-x^2-y^2),Sqrt[15/(4 Pi)]x z,
  Sqrt[15/(4 Pi)]y z,Sqrt[15/(16 Pi)](x^2-y^2),Sqrt[15/(4 Pi)]x y};
 onSphere[p_]:=Last[PolynomialReduce[Expand[p],{x^2+y^2+z^2-1},xyz]];
 (* 单位球单项式积分：奇指数为0，偶指数用双阶乘的精确积分公式。 *)
 moment[e_List]:=If[AnyTrue[e,OddQ],0,
  4 Pi Times@@(Factorial2[#-1]& /@ e)/Factorial2[Total[e]+1]];
 sphereIntegral[p_]:=Total[(Last[#] moment[First[#]])& /@
  CoefficientRules[Expand[p],xyz]];
 gram=Table[FullSimplify[sphereIntegral[basis[[i]]basis[[j]]]],{i,9},{j,9}];
 laplacians=Table[FullSimplify[Sum[D[basis[[i]],{xyz[[j]],2}],{j,3}]],{i,9}];
 angles={Sin[theta]Cos[phi],Sin[theta]Sin[phi],Cos[theta]};
 paper=basis/.Thread[xyz->angles];
 yc[l_,m_]:=SphericalHarmonicY[l,m,theta,phi];
 (* Wolfram采用CS复基：显式去相位，输出2015正x/y及正xz/yz基。 *)
 fromDefinition={yc[0,0],-Sqrt[2]Re[yc[1,1]],-Sqrt[2]Im[yc[1,1]],yc[1,0],
  yc[2,0],-Sqrt[2]Re[yc[2,1]],-Sqrt[2]Im[yc[2,1]],
  Sqrt[2]Re[yc[2,2]],Sqrt[2]Im[yc[2,2]]};
 definitionResiduals=FullSimplify[ComplexExpand[FunctionExpand[fromDefinition]]-paper,
  Assumptions->0<theta<Pi&&Element[phi,Reals]];
 (* 与low_order_basis.cpp相同的关联Legendre递推表达，不依赖直接值输出。 *)
 p00=1;p10=z p00;p11c=x p00;p11s=y p00;
 p20=(3z p10-p00)/2;p21c=3z p11c;p21s=3z p11s;
 p22c=3(x p11c-y p11s);p22s=3(x p11s+y p11c);
 n00=1/Sqrt[4Pi];n10=Sqrt[3/(4Pi)];n20=Sqrt[5/(4Pi)];
 n21=Sqrt[5/(12Pi)];n22=Sqrt[5/(48Pi)];
 recurrence={n00 p00,n10 p11c,n10 p11s,n10 p10,n20 p20,
  n21 p21c,n21 p21s,n22 p22c,n22 p22s};
 recurrenceResiduals=onSphere/@(recurrence-basis);
 dd={{Sqrt[4Pi/9]basis[[1]]-Sqrt[4Pi/45]basis[[5]]+Sqrt[4Pi/15]basis[[8]],
       Sqrt[4Pi/15]basis[[9]],Sqrt[4Pi/15]basis[[6]]},
     {Sqrt[4Pi/15]basis[[9]],
       Sqrt[4Pi/9]basis[[1]]-Sqrt[4Pi/45]basis[[5]]-Sqrt[4Pi/15]basis[[8]],
       Sqrt[4Pi/15]basis[[7]]},
     {Sqrt[4Pi/15]basis[[6]],Sqrt[4Pi/15]basis[[7]],
       Sqrt[4Pi/9]basis[[1]]+Sqrt[16Pi/45]basis[[5]]}};
 dyadicResiduals=Map[onSphere,Outer[Times,xyz,xyz]-dd,{2}];
 printedXX=Sqrt[4Pi/9]basis[[1]]-Sqrt[4Pi/45]basis[[5]]-Sqrt[4Pi/15]basis[[8]];
 printedYY=Sqrt[4Pi/9]basis[[1]]-Sqrt[4Pi/45]basis[[5]]+Sqrt[4Pi/15]basis[[8]];
 result=<|"kernel"->$Version,"basisOrder"->{"g0","f1","f2","f3","h1","h2","h3","h4","h5"},
  "definitionResiduals"->definitionResiduals,"recurrenceResiduals"->recurrenceResiduals,
  "gramIdentity81"->(gram===IdentityMatrix[9]),"harmonicLaplacians"->laplacians,
  "dyadicResiduals"->dyadicResiduals,
  "printedXXEqualsYY"->(onSphere[printedXX-y^2]===0),
  "printedYYEqualsXX"->(onSphere[printedYY-x^2]===0),
  "allExact"->(definitionResiduals===ConstantArray[0,9]&&
    recurrenceResiduals===ConstantArray[0,9]&&gram===IdentityMatrix[9]&&
    laplacians===ConstantArray[0,9]&&dyadicResiduals===ConstantArray[0,{3,3}])|>;
 If[TrueQ[result["allExact"]],result,Failure["SymbolicIdentityFailed",result]]
]
