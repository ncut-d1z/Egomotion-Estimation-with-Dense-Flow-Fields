(* Source-formula mathematics only. No C++/MATLAB/Octave execution or replacement solver.
   koeff_aus_near.m: 5ff91a2210a20e5acf79dec6e59ddd33b55529d3
   near_aus_koeff.m: 7373c4376dfd5bbf281136ced39657d411a10ffa
   kugelflfunc2.m: edc0caa1f88bf91d2a189e4d3a283334f809b905
   Order: (0,0),(1,-1),(1,0),(1,1),(2,-2),(2,-1),(2,0),(2,1),(2,2).
   F contains source-CS polynomials, B=F*S the normalized real SH values.
   beta=S*c is explicitly NOT the original normalized-source coefficient c.
   All finite counterexample inequalities are evaluated in exact rational arithmetic. *)
Module[{poly,scale,basis,base,fb,gb,db,tb,pb,biased,f,g,d,t,p,beta,expectedBeta,
 ones,witness={2/3,-2/3,-1/3},rec,normalizedB,normalizedG,sourceC,
 stereo,distinct,fd,gd,dd,betad,witnessExact,cube,fc,gc,cubeZero,
 cone,h,fh,gh,coneZero,cross,system,mcube,mcone,gramUnit,checks,result,
 num,aa,bb,nn,discrepancy},
 poly[v_]:=With[{x=v[[1]],y=v[[2]],z=v[[3]]},
   {1,-y,z,-x,x*y,-y*z,3z^2-1,-x*z,x^2-y^2}];
 scale={1/Sqrt[4Pi],Sqrt[3/(4Pi)],Sqrt[3/(4Pi)],Sqrt[3/(4Pi)],
   Sqrt[15/(4Pi)],Sqrt[15/(4Pi)],Sqrt[5/(16Pi)],Sqrt[15/(4Pi)],Sqrt[15/(16Pi)]};
 basis[v_]:=poly[v]*scale;
 num[v_]:=N[v,MachinePrecision];
 base=DeleteDuplicates[Flatten[Table[signs*perm,
   {perm,Permutations[{1,2,2}/3]},{signs,Tuples[{-1,1},3]}],1]];
 fb=poly/@base;gb=Transpose[fb].fb;db=DiagonalMatrix[Diagonal[gb]];
 tb=Inverse[db].gb;pb=fb.Inverse[db].Transpose[fb];
 biased=Join[base,ConstantArray[{1,2,2}/3,12]];
 f=poly/@biased;g=Transpose[f].f;d=DiagonalMatrix[Diagonal[g]];
 t=Inverse[d].g;p=f.Inverse[d].Transpose[f];ones=ConstantArray[1,36];
 beta=Inverse[d].Transpose[f].ones;rec=f.beta;
 expectedBeta={1,-3/5,3/5,-3/7,9/10,-9/8,3/5,-9/10,-9/7};
 normalizedB=basis/@biased;normalizedG=FullSimplify[Transpose[normalizedB].normalizedB];
 sourceC=FullSimplify[Inverse[DiagonalMatrix[Diagonal[normalizedG]]].Transpose[normalizedB].ones];
 gramUnit=DiagonalMatrix[1/Sqrt[Diagonal[g]]].g.DiagonalMatrix[1/Sqrt[Diagonal[g]]];
 discrepancy=Tr[(t-IdentityMatrix[9]).(t-IdentityMatrix[9])];
 stereo[u_,v_]:={2u,2v,1-u^2-v^2}/(1+u^2+v^2);
 distinct=Join[base,Table[stereo[1/5+j/10^6,2/5],{j,1,12}]];
 fd=poly/@distinct;gd=Transpose[fd].fd;dd=Diagonal[gd];
 betad=(Transpose[fd].ConstantArray[1,36])/dd;
 witnessExact=Together[poly[witness].betad];
 cube=Tuples[{-1,1},3]/Sqrt[3];fc=poly/@cube;gc=FullSimplify[Transpose[fc].fc];
 cubeZero=Flatten[Position[Diagonal[gc],0]];
 cone={{2h/(1+h^2),0,(1-h^2)/(1+h^2)},
   {-2h/(1+h^2),0,(1-h^2)/(1+h^2)},
   {0,2h/(1+h^2),(1-h^2)/(1+h^2)},
   {0,-2h/(1+h^2),(1-h^2)/(1+h^2)}};
 fh=FullSimplify[poly/@cone];gh=FullSimplify[Transpose[fh].fh];
 coneZero=Flatten[Position[Diagonal[gh],0]];
 cross[v_]:={{0,-v[[3]],v[[2]]},{v[[3]],0,-v[[1]]},{-v[[2]],v[[1]],0}};
 system[dirs_]:=With[{a=IdentityMatrix[3]-Mean[Outer[Times,#,#]&/@dirs],k=cross[Mean[dirs]]},ArrayFlatten[{{a,-k},{k,a}}]];
 mcube=FullSimplify[system[cube]];mcone=FullSimplify[system[cone]];
 checks=<|
  "sourceCommon4PiOverNCancels"->(FullSimplify[((aa/nn)*4Pi)/((bb/nn)*4Pi)-aa/bb,nn>0&&bb>0]===0),
  "nineOriginalCSChannels"->(Length[poly[{1,2,2}/3]]==9),
  "base24Directions"->(Length[base]==24),
  "baseUnitExact"->And@@((#.#==1)&/@base),
  "baseFullRankNine"->(MatrixRank[fb]==9),
  "baseDiscreteGramDiagonal"->(gb===db),
  "baseCoefficientRoundtripIdentity"->(tb==IdentityMatrix[9]),
  "baseSampleOperatorIdempotent"->(pb.pb==pb),
  "biased36InputRows"->(Length[biased]==36),
  "biasedAllUnitExact"->And@@((#.#==1)&/@biased),
  "biasedNoZeroDenominators"->And@@Thread[Diagonal[g]>0],
  "biasedFullRankNine"->(MatrixRank[f]==9),
  "constantFieldRepresentableExactly"->(f.UnitVector[9,1]==ones),
  "sourceNormalizedBasisMatchesPolynomialCalculation"->(FullSimplify[sourceC*scale-beta]===ConstantArray[0,9]),
  "biasedPolynomialCoefficientsExact"->(beta===expectedBeta),
  "positiveConstantProducesNegativeWitness"->(poly[witness].beta==-9/140),
  "witnessIsAnObservedDirection"->MemberQ[base,witness],
  "biasedMinimumExact"->(Min[rec]==-9/140),
  "constantInputMeanInflates"->(Mean[rec]==383/210),
  "coefficientRoundtripNotIdentity"->(t!=IdentityMatrix[9]),
  "sampleOperatorNotIdempotent"->((p.p-p).ones!=ConstantArray[0,36]),
  "unitDiagonalNormalizedGram"->(Diagonal[gramUnit]==ConstantArray[1,9]),
  "sampleOperatorTraceNine"->(Tr[p]==9),
  "strictSpectralSpreadExact"->(discrepancy==25216/3675),
  "distinct36Directions"->(Length[DeleteDuplicates[distinct]]==36),
  "distinctUnitExact"->And@@(Together[#.#-1]===0&/@distinct),
  "distinctContainsFullRankBase"->(Take[fd,24]===fb),
  "distinctNoZeroDenominators"->And@@Thread[dd>0],
  "distinctNegativeWitnessExact"->(witnessExact<0),
  "distinctWitnessExactRationalInterval"->(-65/1000<witnessExact<-64/1000),
  "cubeRankSeven"->(MatrixRank[fc]==7),
  "cubeUndefinedChannels20And22"->(cubeZero=={7,9}),
  "cubeMotionMatrixWellConditioned"->(mcube==2IdentityMatrix[6]/3),
  "narrowConeUndefinedChannel2Minus2"->(coneZero=={5}),
  "narrowConeMotionMatrixInvertibleAtQuarter"->(Det[mcone/.h->1/4]>0)
 |>;
 result=<|"kernel"->$Version,"checkCount"->Length[checks],"passedCount"->Count[Values[checks],True],
  "allChecksPassed"->And@@Values[checks],"checks"->checks,
  "balancedControl"-><|"rows"->24,"rank"->9,"polynomialGramDiagonal"->(ToString[#,InputForm]&/@Diagonal[gb])|>,
  "biasedConstantCase"-><|"baseRows"->24,"additionalRepeatedRows"->12,"repeatedDirection"->num[{1,2,2}/3],
   "rawNearAll"->1,"rank"->9,"allDenominatorsPositive"->True,
   "polynomialCoefficientsExact"->(ToString[#,InputForm]&/@beta),
   "sourceCoefficientsApprox"->num[sourceC],"observedWitnessDirection"->num[witness],
   "negativeWitnessExact"->"-9/140","sampleMeanExact"->"383/210",
   "roundtripSpectralSpreadExact"->"25216/3675"|>,
  "distinctConstantCase"-><|"rows"->36,"distinctRows"->36,"inputFormula"->"base24 plus stereographic(1/5+j/10^6,2/5), j=1..12",
   "rawNearAll"->1,"rank"->9,"allDenominatorsPositive"->True,
   "negativeWitnessExactSign"->(witnessExact<0),"negativeWitnessApprox"->num[witnessExact],
   "exactNumeratorSign"->Sign[Numerator[witnessExact]],"exactDenominatorSign"->Sign[Denominator[witnessExact]],
   "exactNumeratorDigits"->IntegerLength[Abs[Numerator[witnessExact]]],"exactDenominatorDigits"->IntegerLength[Denominator[witnessExact]]|>,
  "lastStageDomainExamples"-><|"cubeRows"->8,"cubeRank"->7,"cubeZeroPolynomialColumnIndices"->cubeZero,
   "cubeMotionCondition2"->1,"narrowConeRows"->4,"coneIdenticallyZeroColumnIndices"->coneZero,
   "note"->"column indices are 1-based in original m order; exact zero can be tiny rather than zero in floating point; no native runtime inference"|>,
  "limits"->"Exact discrete source-formula analysis. Does not replace source with least squares, prove Adaptive iteration divergence, or change correct basis/rotation formulas. No C++/MATLAB/CTest execution, no DEC-005 approval."|>;
 If[TrueQ[result["allChecksPassed"]],ExportString[result,"RawJSON","Compact"->True],Failure["DiscreteRefitProofFailed",result]]
]
