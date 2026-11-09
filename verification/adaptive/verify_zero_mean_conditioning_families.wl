(* Mathematical audit only. No MATLAB/Octave/C++ execution or solver replacement.
   Source: oneit_adaptiv_matched_filter.m, blob 48e51bcd52fb8f27cf35b3fce38b28507af36e55.
   All raw-nearness fields below are represented by original CS harmonics l<=1:
   r=a+b*y => C00=a*Sqrt[4 Pi], C1,-1=-b*Sqrt[4 Pi/3].
   Numerical SVDs supplement exact characteristic-polynomial identities;
   their diagnostic threshold is not an approved DEC-005 tolerance. *)
Module[{a,b,h,x,rr,ss,dr,ds,cross,system,rhs,dirs,raw,mu,scaleMat,
 cancelMat,gamma,cancelGramPoly,cancelExpectedPoly,cancelKappa,
 coneDirs,coneMat,lmin,lmid,lmax,weak,omega,flow,scaleSolution,
 pureRotationSolution,normChecks,checks,shearRows,coneRows,sv,formula,
 shearErrors,coneErrors,result},
 cross[v_]:={{0,-v[[3]],v[[2]]},{v[[3]],0,-v[[1]]},{-v[[2]],v[[1]],0}};
 system[d_,m_]:=ArrayFlatten[{
   {IdentityMatrix[3]-Mean[MapThread[#1 Outer[Times,#2,#2]&,{m,d}]],cross[-Mean[d]]},
   {cross[Mean[MapThread[Times,{m,d}]]],IdentityMatrix[3]-Mean[Outer[Times,#,#]&/@d]}}];
 rhs[d_,p_]:=Join[-Mean[p],Mean[MapThread[Cross,{p,d}]]];
 dirs={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
 raw=(a+b #[[2]])&/@dirs;mu=raw/a; (* a>0 branch; a=0 intentionally not evaluated *)
 scaleMat=FullSimplify[system[dirs,ConstantArray[1,6]]];
 cancelMat=FullSimplify[system[dirs,mu]];
 gamma=b/(2 a);
 cancelGramPoly=CharacteristicPolynomial[Transpose[cancelMat].cancelMat,x];
 cancelExpectedPoly=(x-4/9)^2*(x^2-(4/9)*(2+gamma^2)*x+16/81)^2;
 cancelKappa=((Sqrt[gamma^2+4]+gamma)/2)^2; (* a>0,b>=0 *)
 coneDirs={{2h/(1+h^2),0,(1-h^2)/(1+h^2)},
   {-2h/(1+h^2),0,(1-h^2)/(1+h^2)},
   {0,2h/(1+h^2),(1-h^2)/(1+h^2)},
   {0,-2h/(1+h^2),(1-h^2)/(1+h^2)}};
 coneMat=FullSimplify[system[coneDirs,ConstantArray[1,4]]];
 lmin=2h^4/(1+h^2)^2;lmid=4h^2/(1+h^2)^2;lmax=2/(1+h^2)^2;
 weak={1,0,0,0,-1,0};
 omega={1/5,-1/7,1/11};
 flow=Table[-a({1,0,0}-d[[1]]d)-Cross[omega,d],{d,dirs}];
 scaleSolution=FullSimplify[LinearSolve[scaleMat,rhs[dirs,flow]]];
 pureRotationSolution=FullSimplify[LinearSolve[scaleMat,rhs[dirs,(-Cross[omega,#])&/@dirs]]];
 normChecks={
   FullSimplify[rr/ss-rr/Abs[ss],ss>0]===0,
   FullSimplify[-rr/ss-rr/Abs[ss],ss<0]===0,
   And@@Table[Together[(rr+dr)/(sign*(ss+ds))-rr/(sign*ss)-
     (dr-sign*(rr/(sign*ss))*ds)/(sign*(ss+ds))]===0,{sign,{-1,1}}]};
 checks=<|
   "sourcePositiveNormalization"->normChecks[[1]],
   "sourceNegativeNormalizationUsesRawNearOverAbsMean"->normChecks[[2]],
   "finitePerturbationIdentityBothFixedSigns"->normChecks[[3]],
   "sixAxisUnitDirections"->And@@((#.#==1)&/@dirs),
   "meanRawExactlyA"->(Simplify[Mean[raw]]===a),
   "exactZeroMeanNonzeroInputExists"->(Mean[raw/.{a->0,b->1}]==0&&raw/.{a->0,b->1}!=ConstantArray[0,6]),
   "CSCoefficientReconstructsRawField"->And@@Table[FullSimplify[
     a Sqrt[4Pi]/Sqrt[4Pi]+(-b Sqrt[4Pi/3])*(-Sqrt[3/(4Pi)]d[[2]])-
     (a+b d[[2]])]===0,{d,dirs}],
   "scaleMatrixTwoThirdsIdentity"->(scaleMat==2IdentityMatrix[6]/3),
   "scaleRawMotionRecovered"->(scaleSolution==Join[{a,0,0},omega]),
   "pureRotationHasZeroRawTranslation"->(pureRotationSolution==Join[{0,0,0},omega]),
   "cancellationDeterminantConstant"->(Factor[Det[cancelMat]]===64/729),
   "cancellationEigenvaluesDoNotRevealIllConditioning"->(Factor[CharacteristicPolynomial[cancelMat,x]-(x-2/3)^6]===0),
   "cancellationGramPolynomialExact"->(Together[cancelGramPoly-cancelExpectedPoly]===0),
   "cancellationConditionGrowth"->(Limit[cancelKappa/gamma^2,a->0,Direction->"FromAbove",Assumptions->b>0]===1),
   "positiveNearCannotCancel"->FullSimplify[3a/(2a+b)>0&&3a/(2a+b)<1,0<a<b],
   "coneDirectionsUnit"->And@@(Together[#.#-1]===0&/@coneDirs),
   "coneMatrixSymmetric"->(coneMat===Transpose[coneMat]),
   "coneExactSpectrumPolynomial"->(Together[CharacteristicPolynomial[coneMat,x]-(x-lmin)^2*(x-lmid)^2*(x-lmax)^2]===0),
   "coneSpectrumOrdered"->FullSimplify[0<lmin<=lmid<=lmax,0<h<=1/2],
   "coneConditionExactlyHMinus4"->(Together[lmax/lmin-h^-4]===0),
   "coneDeterminantExact"->(Together[Det[coneMat]-256h^12/(1+h^2)^12]===0),
   "coneWeakMode"->(FullSimplify[coneMat.weak-lmin weak]===ConstantArray[0,6]),
   "coneLimitRankTwo"->(MatrixRank[coneMat/.h->0]===2)
 | >;
 shearRows=Table[
   sv=SingularValueList[N[cancelMat/.{a->aa,b->1},70]];
   formula=N[cancelKappa/.{a->aa,b->1},60];
   <|"a"->N[aa],"b"->1,"determinant"->N[64/729],
     "conditionFormula"->N[formula],"conditionSVD"->N[Max[sv]/Min[sv]],
     "relativeDifference"->N[Abs[(Max[sv]/Min[sv])/formula-1]],
     "meanAbsRaw"->N[(2aa+1)/3],"cancellationRatio"->N[3aa/(2aa+1)]|>,
   {aa,{1/10,1/1000,1/1000000}}];
 coneRows=Table[
   sv=SingularValueList[N[coneMat/.h->hh,70]];
   formula=hh^-4;
   <|"h"->N[hh],"rawMean"->1,"conditionExact"->formula,
     "conditionSVD"->N[Max[sv]/Min[sv]],
     "relativeDifference"->N[Abs[(Max[sv]/Min[sv])/formula-1]],
     "lambdaMin"->N[lmin/.h->hh],"lambdaMax"->N[lmax/.h->hh]|>,
   {hh,{1/10,1/100,1/1000}}];
 shearErrors=Lookup[shearRows,"relativeDifference"];coneErrors=Lookup[coneRows,"relativeDifference"];
 checks=Join[checks,<|"highPrecisionShearSVDAgrees"->(Max[shearErrors]<10^-35),
   "highPrecisionConeSVDAgrees"->(Max[coneErrors]<10^-35)|>];
 result=<|"kernel"->$Version,"checkCount"->Length[checks],
   "passedCount"->Count[Values[checks],True],"checks"->checks,
   "allChecksPassed"->And@@Values[checks],
   "scaleFamily"-><|"mean"->"a>0","condition2"->1,"solvedMotion"->{"a",0,0,"1/5","-1/7","1/11"},
     "translationNoiseAngle"->"atan(abs(eta)/a)","sourceAtAZero"->"undefined mean normalization"|>,
   "signedCancellationFamily"-><|"conditionFormula"->"((sqrt(gamma^2+4)+abs(gamma))/2)^2; gamma=b/(2a)",
     "determinantExact"->"64/729","allEigenvalues"->"2/3 (six)","samples"->shearRows|>,
   "positiveNarrowConeFamily"-><|"conditionExact"->"h^-4 for 0<h<=1/2","allRawNear"->1,
     "eigenvaluesEachTwice"->{"2h^4/(1+h^2)^2","4h^2/(1+h^2)^2","2/(1+h^2)^2"},
     "rankAtZero"->2,"samples"->coneRows|>,
   "limits"->"Exact source-formula algebra and high-precision Wolfram SVD only; no production Adaptive/MATLAB/C++/CTest; no DEC-005 approval."|>;
 If[TrueQ[result["allChecksPassed"]],ExportString[result,"RawJSON","Compact"->True],Failure["MeanConditioningProofFailed",result]]
]
