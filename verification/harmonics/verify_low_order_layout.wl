Module[{x,y,z,cs,paperBasis,sourceBasis,paperCoefficients,back,fieldResidual,
  rx,perm,legacy,oldDipole,newDipole,correctAtY,legacyAtY,result},
 cs=Array[c,9];
 paperBasis={1/Sqrt[4Pi],Sqrt[3/(4Pi)]x,Sqrt[3/(4Pi)]y,Sqrt[3/(4Pi)]z,
  Sqrt[5/(16Pi)](2z^2-x^2-y^2),Sqrt[15/(4Pi)]x z,
  Sqrt[15/(4Pi)]y z,Sqrt[15/(16Pi)](x^2-y^2),Sqrt[15/(4Pi)]x y};
 (* 直接按原kugelflfunc2含CS相位的[-l,...,+l]顺序展开。 *)
 sourceBasis={paperBasis[[1]],-paperBasis[[3]],paperBasis[[4]],-paperBasis[[2]],
  paperBasis[[9]],-paperBasis[[7]],paperBasis[[5]],-paperBasis[[6]],paperBasis[[8]]};
 (* 与C++ fromMatlabLayout的九个赋值逐项对应。 *)
 paperCoefficients={cs[[1]],-cs[[4]],-cs[[2]],cs[[3]],cs[[7]],-cs[[8]],-cs[[6]],cs[[9]],cs[[5]]};
 back={paperCoefficients[[1]],-paperCoefficients[[3]],paperCoefficients[[4]],
  -paperCoefficients[[2]],paperCoefficients[[9]],-paperCoefficients[[7]],
  paperCoefficients[[5]],-paperCoefficients[[6]],paperCoefficients[[8]]};
 fieldResidual=Expand[paperCoefficients.paperBasis-cs.sourceBasis];
 rx={{1,0,0},{0,0,-1},{0,1,0}};perm={{0,1,0},{0,0,1},{1,0,0}};
 legacy=perm.Transpose[rx].Transpose[perm];
 (* mu_old=2+z。MATLAB旧系数的m=0为sqrt(4pi/3)。 *)
 oldDipole={0,Sqrt[4Pi/3],0};newDipole=legacy.oldDipole;
 legacyAtY=FullSimplify[2+newDipole.(sourceBasis[[2;;4]]/.{x->0,y->1,z->0})];
 correctAtY=2+(rx.{0,1,0})[[3]];
 result=<|"kernel"->$Version,"allCoefficientRoundtripResiduals"->Expand[back-cs],
  "sameFieldPolynomialResidual"->fieldResidual,
  "legacyAtYFor2PlusZ"->legacyAtY,"specifiedPullbackAtYFor2PlusZ"->correctAtY,
  "allExact"->(Expand[back-cs]===ConstantArray[0,9]&&fieldResidual===0&&
    legacyAtY===1&&correctAtY===3)|>;
 If[TrueQ[result["allExact"]],result,Failure["LayoutIdentityFailed",result]]
]
