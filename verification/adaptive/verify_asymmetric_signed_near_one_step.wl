(* AN-ADAPT-NONASYM: independent mathematical one-step test on nine
   asymmetric rational unit directions, with positive and negative mean near.
   Sources: oneit_adaptiv_matched_filter.m, near_aus_koeff.m,
   kugelflfunc2.m, koeff_aus_near.m. Original files remain read-only.
   No MATLAB/Octave/C++ code generation or execution. *)
Module[{dirs,ex={1,0,0},omegaTrue={1/5,-1/7,1/11},basis,coeffBase,
  crossMat,fixture,results},
 dirs={{1,0,0},{0,1,0},{0,0,1},{-1,0,0},{0,-1,0},{0,0,-1},
   {3/5,4/5,0},{0,3/5,4/5},{4/5,0,3/5}};
 basis[d_]:=Module[{x=d[[1]],y=d[[2]],z=d[[3]]},
   {1/Sqrt[4 Pi],-Sqrt[3/(4 Pi)]y,Sqrt[3/(4 Pi)]z,
     -Sqrt[3/(4 Pi)]x,Sqrt[15/(4 Pi)]x y,
     -Sqrt[15/(4 Pi)]y z,Sqrt[5/(16 Pi)](3 z^2-1),
     -Sqrt[15/(4 Pi)]x z,Sqrt[15/(16 Pi)](x^2-y^2)}];
 coeffBase={{Sqrt[4 Pi],0,0,0,0},
   {Sqrt[4 Pi/3]/4,Sqrt[4 Pi/3]/5,-Sqrt[4 Pi/3]/3,0,0},
   {0,0,0,0,0}};
 crossMat[v_]:={{0,-v[[3]],v[[2]]},{v[[3]],0,-v[[1]]},
   {-v[[2]],v[[1]],0}};
 fixture[sign_]:=Module[{countRows=Length[dirs],coeff,flat,basisRows,nearRaw,
     meanRaw,nearAfterSign,nearN,flow,m1,m2,m3,m4,mat,rhs,
     solved,rawT,trans,omega,flowN,predicted,mse,nearNew,
     coeffNew,sourceProjection,projectionIdentity,positiveBranchProof,sv,
     result,detMat,meanN,differenceM1,d1,d2},
   coeff=sign coeffBase;
   flat=Join[{coeff[[1,1]]},coeff[[2,1;;3]],coeff[[3,1;;5]]];
   basisRows=basis/@dirs;
   nearRaw=FullSimplify[basisRows.flat];
   meanRaw=Mean[nearRaw];
   nearAfterSign=If[meanRaw<0,-nearRaw,nearRaw];
   nearN=FullSimplify[nearAfterSign/meanRaw];
   meanN=FullSimplify[Mean[nearN]];
   flow=MapThread[-#1(ex-(ex.#2)#2)-Cross[omegaTrue,#2]&,
      {nearRaw,dirs}];
   m1=IdentityMatrix[3]-Total[MapThread[#1 Outer[Times,#2,#2]&,
      {nearN,dirs}]]/countRows;
   m4=IdentityMatrix[3]-Total[Outer[Times,#,#]& /@ dirs]/countRows;
   m2=crossMat[-Total[dirs]/countRows];
   m3=crossMat[Total[MapThread[Times,{nearN,dirs}]]/countRows];
   mat=ArrayFlatten[{{m1,m2},{m3,m4}}];
   rhs=Join[-Total[flow]/countRows,
      Total[MapThread[Cross,{flow,dirs}]]/countRows];
   detMat=Det[mat];
   sv=SingularValueList[N[mat,MachinePrecision]];
   solved=LinearSolve[mat,rhs];
   rawT=solved[[1;;3]];
   trans=N[rawT/Sqrt[rawT.rawT],24];
   omega=N[solved[[4;;6]],24];
   flowN=N[flow,24];
   predicted=MapThread[-#1 N[meanRaw,24] (trans-(trans.#2)#2)-
      Cross[omega,#2]&,{N[nearN,24],N[dirs,24]}];
   mse=N[Mean[(#.#)& /@ (flowN-predicted)],20];
   nearNew=MapThread[-trans.(#1-Cross[#2,omega])/
       (1-(trans.#2)^2+10^-11)&,{flowN,N[dirs,24]}];
   coeffNew=Table[
      Total[nearNew*(N[basisRows,24][[All,j]])]/
        Total[(N[basisRows,24][[All,j]])^2],
      {j,1,9}];
   differenceM1=FullSimplify[
      m1-Total[MapThread[#1(IdentityMatrix[3]-
         Outer[Times,#2,#2])&,{nearN,dirs}]]/countRows];
   positiveBranchProof=If[sign==1,
      FullSimplify[solved-Join[{meanRaw,0,0},omegaTrue]]===
        ConstantArray[0,6],
      False];
   result=<|"sign"->sign,"directions"->N[dirs,17],
     "inputCSCoefficients3x5"->N[coeff,17],
     "oldNearReconstructed"->N[nearRaw,17],
     "oldNearMeanExact"->ToString[meanRaw,InputForm],
     "oldNearMean"->N[meanRaw,17],
     "meanAfterNormalizationExact"->meanN,
     "nearAfterSignFlip"->N[nearAfterSign,17],
     "nearNormalized"->N[nearN,17],
     "syntheticTangentialFlow"->N[flow,17],
     "M1"->N[m1,17],"M2"->N[m2,17],
     "M3"->N[m3,17],"M4"->N[m4,17],
     "rhs"->N[rhs,17],"detM"->N[detMat,17],
     "conditionNumber2"->N[Max[sv]/Min[sv],17],
     "solvedMotionSix"->N[solved,17],
     "translationNormalized"->N[trans,17],
     "rotationEstimated"->N[omega,17],
     "errorMse"->If[sign==1&&positiveBranchProof,0,N[mse,17]],
     "newNearMin"->N[Min[nearNew],17],
     "newNearMax"->N[Max[nearNew],17],
     "newNearAll"->N[nearNew,17],
     "newCoefficientsSourceNine"->N[coeffNew,17],
     "M1MinusWeightedTangent"->N[differenceM1,17],
     "M1MinusWeightedTangentEquals2I"->(
       FullSimplify[differenceM1-(1-meanN)IdentityMatrix[3]]===
          ConstantArray[0,{3,3}]),
     "matrixNonsingular"->(detMat!=0),
     "M2Nonzero"->(m2!=ConstantArray[0,{3,3}]),
     "M3Nonzero"->(m3!=ConstantArray[0,{3,3}]),
     "positiveBranchExactMotionRecovered"->positiveBranchProof,
     "negativeBranchRawMean"->(meanRaw<0)|>;
   result
 ];
 results=<|"positiveMean"->fixture[1],
   "negativeMeanSigned"->fixture[-1]|>;
 ExportString[results,"RawJSON","Compact"->True]
]