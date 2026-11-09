(* Mathematical audit only; no MATLAB/C++/CTest execution or solver replacement.
   oneit_adaptiv_matched_filter.m source blob: 48e51bcd52fb8f27cf35b3fce38b28507af36e55.
   Original CS field r=a+b*y: C00=a*Sqrt[4 Pi], C1,-1=-b*Sqrt[4 Pi/3].
   Mean zero is classified before division, not evaluated as a source runtime.
   SVDs use 70-digit arithmetic; diagnostic comparisons are not DEC-005 approval. *)
Module[{a,b,h,x,rr,ss,dr,ds,cross,system,rhs,num,dirs,raw,scaleMat,cancelMat,gamma,cancelKappa,growthRatio,coneDirs,coneMat,lmin,lmid,lmax,omega,flow,sol,pureSol,checks,shearRows,coneRows,sv,f,result},
 cross[v_]:={{0,-v[[3]],v[[2]]},{v[[3]],0,-v[[1]]},{-v[[2]],v[[1]],0}};
 system[d_,mu_]:=ArrayFlatten[{{IdentityMatrix[3]-Mean[MapThread[#1 Outer[Times,#2,#2]&,{mu,d}]],cross[-Mean[d]]},{cross[Mean[MapThread[Times,{mu,d}]]],IdentityMatrix[3]-Mean[Outer[Times,#,#]&/@d]}}];
 rhs[d_,p_]:=Join[-Mean[p],Mean[MapThread[Cross,{p,d}]]];
 num[v_]:=N[v,MachinePrecision];
 dirs={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
 raw=(a+b #[[2]])&/@dirs;
 scaleMat=FullSimplify[system[dirs,ConstantArray[1,6]]];
 cancelMat=FullSimplify[system[dirs,raw/a]];
 gamma=b/(2a);cancelKappa=((Sqrt[gamma^2+4]+gamma)/2)^2;
 growthRatio=FullSimplify[cancelKappa/gamma^2,a>0&&b>0];
 coneDirs={{2h/(1+h^2),0,(1-h^2)/(1+h^2)},{-2h/(1+h^2),0,(1-h^2)/(1+h^2)},{0,2h/(1+h^2),(1-h^2)/(1+h^2)},{0,-2h/(1+h^2),(1-h^2)/(1+h^2)}};
 coneMat=FullSimplify[system[coneDirs,ConstantArray[1,4]]];
 lmin=2h^4/(1+h^2)^2;lmid=4h^2/(1+h^2)^2;lmax=2/(1+h^2)^2;
 omega={1/5,-1/7,1/11};flow=Table[-a({1,0,0}-d[[1]]d)-Cross[omega,d],{d,dirs}];
 sol=FullSimplify[LinearSolve[scaleMat,rhs[dirs,flow]]];
 pureSol=FullSimplify[LinearSolve[scaleMat,rhs[dirs,(-Cross[omega,#])&/@dirs]]];
 checks=<|
 "rawPositiveBranch"->(FullSimplify[rr/ss-rr/Abs[ss],ss>0]===0),
 "rawNegativeBranch"->(FullSimplify[-rr/ss-rr/Abs[ss],ss<0]===0),
 "finitePerturbationBothSigns"->And@@Table[Together[(rr+dr)/(sign*(ss+ds))-rr/(sign*ss)-(dr-sign*(rr/(sign*ss))*ds)/(sign*(ss+ds))]===0,{sign,{-1,1}}],
 "sixAxisUnitDirections"->And@@((#.#==1)&/@dirs),
 "rawMeanEqualsA"->(Simplify[Mean[raw]]===a),
 "nonzeroFieldWithZeroMean"->(Mean[raw/.{a->0,b->1}]==0&&(raw/.{a->0,b->1})!=ConstantArray[0,6]),
 "CSReconstruction"->And@@Table[FullSimplify[a Sqrt[4Pi]/Sqrt[4Pi]+(-b Sqrt[4Pi/3])*(-Sqrt[3/(4Pi)]d[[2]])-(a+b d[[2]])]===0,{d,dirs}],
 "scaleMatrix"->(scaleMat==2IdentityMatrix[6]/3),
 "scaleRawMotion"->(sol==Join[{a,0,0},omega]),
 "pureRotationZeroTranslation"->(pureSol==Join[{0,0,0},omega]),
 "cancellationConstantDeterminant"->(Factor[Det[cancelMat]]===64/729),
 "cancellationConstantEigenvalues"->(Factor[CharacteristicPolynomial[cancelMat,x]-(x-2/3)^6]===0),
 "cancellationGramPolynomial"->(Together[CharacteristicPolynomial[Transpose[cancelMat].cancelMat,x]-(x-4/9)^2*(x^2-(4/9)*(2+gamma^2)*x+16/81)^2]===0),
 "cancellationAsymptoticGrowth"->(Limit[growthRatio,a->0,Direction->"FromAbove",Assumptions->b>0]===1),
 "signedCancellationIndicator"->FullSimplify[0<3a/(2a+b)<1,0<a<b],
 "coneUnitDirections"->And@@(Together[#.#-1]===0&/@coneDirs),
 "coneSymmetricMatrix"->(coneMat===Transpose[coneMat]),
 "coneSpectrumPolynomial"->(Together[CharacteristicPolynomial[coneMat,x]-(x-lmin)^2*(x-lmid)^2*(x-lmax)^2]===0),
 "coneSpectrumOrdering"->FullSimplify[0<lmin<=lmid<=lmax,0<h<=1/2],
 "coneConditionFormula"->(Together[lmax/lmin-h^-4]===0),
 "coneDeterminant"->(Together[Det[coneMat]-256h^12/(1+h^2)^12]===0),
 "coneWeakMode"->(FullSimplify[coneMat.{1,0,0,0,-1,0}-lmin {1,0,0,0,-1,0}]===ConstantArray[0,6]),
 "coneLimitRankTwo"->(MatrixRank[coneMat/.h->0]===2)|>;
 shearRows=Table[sv=SingularValueList[N[cancelMat/.{a->aa,b->1},70]];f=N[cancelKappa/.{a->aa,b->1},60];
 <|"a"->num[aa],"b"->1,"det"->num[64/729],"conditionFormula"->num[f],"conditionSVD"->num[Max[sv]/Min[sv]],"relativeDifference"->num[Abs[(Max[sv]/Min[sv])/f-1]],"cancellationRatio"->num[3aa/(2aa+1)]|>,{aa,{1/10,1/1000,1/1000000}}];
 coneRows=Table[sv=SingularValueList[N[coneMat/.h->hh,70]];f=hh^-4;
 <|"h"->num[hh],"rawMean"->1,"conditionExact"->f,"conditionSVD"->num[Max[sv]/Min[sv]],"relativeDifference"->num[Abs[(Max[sv]/Min[sv])/f-1]],"lambdaMin"->num[lmin/.h->hh],"lambdaMax"->num[lmax/.h->hh]|>,{hh,{1/10,1/100,1/1000}}];
 checks=Join[checks,<|"shearSVDAgrees"->(Max[Lookup[shearRows,"relativeDifference"]]<10^-35),"coneSVDAgrees"->(Max[Lookup[coneRows,"relativeDifference"]]<10^-35)|>];
 result=<|"kernel"->$Version,"checkCount"->Length[checks],"passedCount"->Count[Values[checks],True],"checks"->checks,"allChecksPassed"->And@@Values[checks],"scaleFamily"-><|"condition2"->1,"rawMotion"->{"a",0,0,"1/5","-1/7","1/11"},"angleWithTransverseNoise"->"atan(abs(eta)/a)","atAZero"->"source normalization undefined"|>,"signedCancellationFamily"-><|"detExact"->"64/729","eigenvalues"->"2/3 repeated six times","conditionFormula"->"((sqrt(gamma^2+4)+abs(gamma))/2)^2; gamma=b/(2a)","samples"->shearRows|>,"positiveConeFamily"-><|"conditionExact"->"h^-4 for 0<h<=1/2","rawNear"->1,"rankAtHZero"->2,"eigenvaluesEachTwice"->{"2h^4/(1+h^2)^2","4h^2/(1+h^2)^2","2/(1+h^2)^2"},"samples"->coneRows|>|>;
 ExportString[result,"RawJSON","Compact"->True]
]
