(* Mathematical AN-G12 one-step check against the published original
   oneit_adaptiv_matched_filter.m control flow, not an Adaptive production
   implementation or CTest. Source directions are the AN-G03 archived rows.
   Uses denominators + 1e-11 for the final reconstructed nearness. *)
Module[{path,archive,dirs,n,initialNear,translationTrue={1,0,0},flow,crossMatrix,
  m1,m2,m3,m4,rhs,motion,translation,rotation,approxFlow,mse,
  updatedNear,sourceBasis,basisMatrix,updatedCoefficients,errors,result},
  path=FileNameJoin[{DirectoryName[DirectoryName[DirectoryName[$InputFileName]]],
    "docs","golden","analytic_golden_candidates.v1.json"}];
  archive=Import[path,"RawJSON"];
  dirs=SelectFirst[archive["fixtures"],#["id"]=="AN-G03"&]["expected"]["directions_ordered"];
  n=Length[dirs];initialNear=ConstantArray[1.,n];
  crossMatrix[v_]:={{0,-v[[3]],v[[2]]},{v[[3]],0,-v[[1]]},
    {-v[[2]],v[[1]],0}};
  flow=Table[-translationTrue+(translationTrue.d)d,{d,dirs}];
  m1=IdentityMatrix[3]-Total[MapThread[#1 Outer[Times,#2,#2]&,
    {initialNear,dirs}]]/n;
  m4=IdentityMatrix[3]-Total[Outer[Times,#,#]& /@ dirs]/n;
  m2=crossMatrix[-Total[dirs]/n];
  m3=crossMatrix[Total[dirs]/n];
  rhs=Join[-Total[flow]/n,Total[MapThread[Cross,{flow,dirs}]]/n];
  motion=LinearSolve[ArrayFlatten[{{m1,m2},{m3,m4}}],rhs];
  translation=Normalize[motion[[1;;3]]];
  rotation=motion[[4;;6]];
  approxFlow=MapThread[-#1(translation-(translation.#2)#2)-
    Cross[rotation,#2]&,{initialNear,dirs}];
  mse=Mean[Norm[#]^2& /@ (flow-approxFlow)];
  updatedNear=MapThread[-translation.(#1-Cross[#2,rotation])/
    (1-(translation.#2)^2+10^-11)&,{flow,dirs}];
  sourceBasis[d_]:=Module[{x=d[[1]],y=d[[2]],z=d[[3]]},
    {1/Sqrt[4 Pi],-Sqrt[3/(4 Pi)]y,Sqrt[3/(4 Pi)]z,
    -Sqrt[3/(4 Pi)]x,Sqrt[15/(4 Pi)]x y,-Sqrt[15/(4 Pi)]y z,
    Sqrt[5/(16 Pi)](3 z^2-1),-Sqrt[15/(4 Pi)]x z,
    Sqrt[15/(16 Pi)](x^2-y^2)}];
  basisMatrix=sourceBasis/@dirs;
  updatedCoefficients=Table[Total[updatedNear*basisMatrix[[All,j]]]/
    Total[basisMatrix[[All,j]]^2],{j,9}];
  errors={Max[Abs[Flatten[m1-(2/3)IdentityMatrix[3]]]],
    Max[Abs[Flatten[m4-(2/3)IdentityMatrix[3]]]],
    Max[Abs[Flatten[m2]]],Max[Abs[Flatten[m3]]],
    Max[Abs[rhs-{2/3,0,0,0,0,0}]]};
  result=<|"fixture"->"AN-G12","rows"->n,"matrixMaxAbsError"->Max[errors],
    "translation"->translation,"rotation"->rotation,
    "mse"->mse,"updatedNearMin"->Min[updatedNear],
    "updatedNearMax"->Max[updatedNear],
    "updatedCoefficientSourceNine"->updatedCoefficients,
    "updatedMonopole"->updatedCoefficients[[1]],
    "maxCoefficientDifferenceFromConstantUnitNear"->
      Max[Abs[updatedCoefficients-{Sqrt[4 Pi],0,0,0,0,0,0,0,0}]],
    "allExpectedSymmetricBlocks"->(n==20&&Max[errors]<10^-12&&
      Norm[translation-{1,0,0}]<10^-12&&Norm[rotation]<10^-12&&
      mse<10^-12),
    "notExactlyOneAfterRegularization"->(Min[updatedNear]<1&&Max[updatedNear]<1)|>;
  If[TrueQ[result["allExpectedSymmetricBlocks"]]&&
      TrueQ[result["notExactlyOneAfterRegularization"]],result,
    Failure["AN_G12_MathCandidateMismatch",result]]
]