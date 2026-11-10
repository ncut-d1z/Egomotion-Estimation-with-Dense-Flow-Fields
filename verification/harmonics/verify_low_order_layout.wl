(* Scoped SH-01/04/05/06 verification. Mathematical translation only;
   no MATLAB or C++ execution, no higher degrees and no fitting analysis.
   Source kugelflfunc2.m blob edc0caa1f88bf91d2a189e4d3a283334f809b905;
   source SHRotate.m blob 7b73000411a9294f6cbed2e111bf74ce7fb7d4df.
   legendre(l,z) defaults to unnormalized associated Legendre WITH CS phase.
   For A=N(q)/(q.q), all second-degree expressions below use N and clear (q.q)^2.
   Existing 2026-11-09 result JSON is a historical certificate, not overwritten. *)
Module[{x,y,z,theta,phi,u,q0,q1,q2,q3,xyz,angles,csBasis,noBasis,paperBasis,
  cs,paperCoefficients,back,fieldResidual,rod,sourceValue,fromRod,fromComplex,
  rodResidual,complexResidual,q,scale,argument,b1,entry,pHelper,uHelper,
  vHelper,wHelper,mHelper,rot2,polynomials,tensors,direct2,s1,s2,
  phase1,phase2,raw1,rawCS,wWeights,pVisited={},explicit2,aa,bb,cc,dd,ee,
  ff,gg,hh,ii,tableResidual,rx,legacy,oldDipole,newDipole,legacyAtY,
  correctAtY,checks,result},
 xyz={x,y,z};angles={Sin[theta]Cos[phi],Sin[theta]Sin[phi],Cos[theta]};
 (* Solid homogeneous l=2 polynomials equal spherical functions on x.x=1. *)
 csBasis={1/Sqrt[4Pi],-Sqrt[3/(4Pi)]y,Sqrt[3/(4Pi)]z,-Sqrt[3/(4Pi)]x,
  Sqrt[15/(4Pi)]x y,-Sqrt[15/(4Pi)]y z,Sqrt[5/(16Pi)](2z^2-x^2-y^2),
  -Sqrt[15/(4Pi)]x z,Sqrt[15/(16Pi)](x^2-y^2)};
 noBasis=csBasis*{1,-1,1,-1,1,-1,1,-1,1};
 paperBasis=noBasis[[{1,4,2,3,7,8,6,9,5}]];
 rod[l_,k_]:=(-1)^k (1-u^2)^(k/2) D[(u^2-1)^l,{u,l+k}]/(2^l Factorial[l]);
 sourceValue[l_,m_]:=Sqrt[(2l+1)/(4Pi) Factorial[l-Abs[m]]/Factorial[l+Abs[m]]] *
   (rod[l,Abs[m]]/.u->Cos[theta]) *
   If[m<0,Sqrt[2]Sin[Abs[m]phi],If[m>0,Sqrt[2]Cos[m phi],1]];
 fromRod=Flatten[Table[sourceValue[l,m],{l,0,2},{m,-l,l}]];
 fromComplex=Flatten[Table[If[m<0,Sqrt[2]Im[SphericalHarmonicY[l,-m,theta,phi]],
   If[m>0,Sqrt[2]Re[SphericalHarmonicY[l,m,theta,phi]],SphericalHarmonicY[l,0,theta,phi]]],
   {l,0,2},{m,-l,l}]];
 rodResidual=FullSimplify[fromRod-(csBasis/.Thread[xyz->angles]),
   0<theta<Pi&&Element[phi,Reals]];
 complexResidual=FullSimplify[ComplexExpand[FunctionExpand[fromComplex]]-
   (csBasis/.Thread[xyz->angles]),0<theta<Pi&&Element[phi,Reals]];
 cs=Array[Unique["coefficient"]&,9];
 paperCoefficients={cs[[1]],-cs[[4]],-cs[[2]],cs[[3]],cs[[7]],-cs[[8]],-cs[[6]],cs[[9]],cs[[5]]};
 back={paperCoefficients[[1]],-paperCoefficients[[3]],paperCoefficients[[4]],
   -paperCoefficients[[2]],paperCoefficients[[9]],-paperCoefficients[[7]],
   paperCoefficients[[5]],-paperCoefficients[[6]],paperCoefficients[[8]]};
 fieldResidual=Expand[paperCoefficients.paperBasis-cs.csBasis];
 q={q0,q1,q2,q3};scale=q.q;
 argument={{q0^2+q1^2-q2^2-q3^2,2(q1 q2-q0 q3),2(q1 q3+q0 q2)},
  {2(q1 q2+q0 q3),q0^2-q1^2+q2^2-q3^2,2(q2 q3-q0 q1)},
  {2(q1 q3-q0 q2),2(q2 q3+q0 q1),q0^2-q1^2-q2^2+q3^2}};
 (* Literal SHRotate.m:33-43 assignments. Input is r=A, not the paper axes R. *)
 b1={{argument[[2,2]],argument[[3,2]],argument[[1,2]]},
     {argument[[2,3]],argument[[3,3]],argument[[1,3]]},
     {argument[[2,1]],argument[[3,1]],argument[[1,1]]}};
 entry[i_,j_]:=b1[[i+2,j+2]];
 (* P uses the previous l=1 block only. Its source column 1 is m=-1,
    column 2l-1=3 is m=+1. These formulas are not extended above l=2. *)
 pHelper[i_,l_,a_,b_]:=Module[{r1=entry[i,1],rm=entry[i,-1],r0=entry[i,0]},
   AppendTo[pVisited,{i,a,b}];
   Which[b==-l,r1 entry[a,-1]+rm entry[a,1],
     b==l,r1 entry[a,1]-rm entry[a,-1],True,r0 entry[a,b]]];
 uHelper[l_,m_,n_]:=pHelper[0,l,m,n];
 vHelper[l_,m_,n_]:=Module[{delta,p0,p1},Which[
   m==0,pHelper[1,l,1,n]+pHelper[-1,l,-1,n],
   m>0,delta=Boole[m==1];p0=pHelper[1,l,m-1,n];p1=pHelper[-1,l,-m+1,n];
     p0 Sqrt[1+delta]-p1(1-delta),
   True,delta=Boole[m==-1];p0=pHelper[1,l,m+1,n];p1=pHelper[-1,l,-m-1,n];
     p0(1-delta)+p1 Sqrt[1+delta]]];
 wHelper[l_,m_,n_]:=Throw[Failure["WCalledOutsideLowOrderProof",<||>]];
 mHelper[l_,m_,n_]:=Module[{d=Boole[m==0],den,cu,cv,cw},
   den=If[Abs[n]==l,2l(2l-1),l^2-n^2];cu=Sqrt[(l^2-m^2)/den];
   cv=Sqrt[(1+d)(l+Abs[m]-1)(l+Abs[m])/den](1-2d)/2;
   cw=-Sqrt[(l-Abs[m]-1)(l-Abs[m])/den](1-d)/2;
   If[cu!=0,cu=cu uHelper[l,m,n]];If[cv!=0,cv=cv vHelper[l,m,n]];
   If[cw!=0,cw=cw wHelper[l,m,n]];Expand[cu+cv+cw]];
 rot2=Catch[Table[mHelper[2,m,n],{m,-2,2},{n,-2,2}]];
 If[FailureQ[rot2],Return[rot2]];
 polynomials=noBasis[[5;;9]];
 tensors=Table[Table[D[polynomials[[k]],xyz[[i]],xyz[[j]]]/2,{i,3},{j,3}],{k,5}];
 direct2=Table[Expand[(8Pi/15)Tr[tensors[[i]].Transpose[argument].tensors[[j]].argument]],
   {i,5},{j,5}];
 (* Independent IR96 Table3 as already transcribed in SH_PAPER_VERIFICATION_CASES.
    Only its constant 1 becomes scale^2 when clearing quaternion denominators. *)
 {aa,bb,cc,dd,ee,ff,gg,hh,ii}=Flatten[b1];
 explicit2={{ii aa+gg cc,gg bb+hh aa,Sqrt[3]hh bb,ii bb+hh cc,ii cc-gg aa},
  {cc dd+aa ff,aa ee+bb dd,Sqrt[3]bb ee,cc ee+bb ff,cc ff-aa dd},
  {Sqrt[3]ff dd,Sqrt[3]dd ee,(3ee^2-scale^2)/2,Sqrt[3]ff ee,Sqrt[3](ff^2-dd^2)/2},
  {ii dd+gg ff,gg ee+hh dd,Sqrt[3]hh ee,ii ee+hh ff,ii ff-gg dd},
  {ii gg-cc aa,gg hh-aa bb,Sqrt[3](hh^2-bb^2)/2,ii hh-cc bb,(ii^2-gg^2-cc^2+aa^2)/2}};
 tableResidual=Expand[explicit2-direct2];
 s1=DiagonalMatrix[{-1,1,-1}];s2=DiagonalMatrix[{1,-1,1,-1,1}];
 raw1=Expand[Transpose[b1].noBasis[[2;;4]]-(noBasis[[2;;4]]/.Thread[xyz->argument.xyz])];
 phase1=Expand[Transpose[s1.b1.s1].csBasis[[2;;4]]-(csBasis[[2;;4]]/.Thread[xyz->argument.xyz])];
 phase2=Expand[Transpose[s2.rot2.s2].csBasis[[5;;9]]-(csBasis[[5;;9]]/.Thread[xyz->argument.xyz])];
 rawCS=Expand[Transpose[b1].csBasis[[2;;4]]-(csBasis[[2;;4]]/.Thread[xyz->argument.xyz])];
 wWeights=Flatten[Table[-Sqrt[(2-Abs[m]-1)(2-Abs[m])/If[Abs[n]==2,12,4-n^2]]*
   (1-Boole[m==0])/2,{m,-2,2},{n,-2,2}]];
 rx={{1,0,0},{0,0,-1},{0,1,0}};
 legacy={{rx[[2,2]],rx[[3,2]],rx[[1,2]]},{rx[[2,3]],rx[[3,3]],rx[[1,3]]},
   {rx[[2,1]],rx[[3,1]],rx[[1,1]]}};
 oldDipole={0,Sqrt[4Pi/3],0};newDipole=legacy.oldDipole;
 legacyAtY=FullSimplify[2+newDipole.(csBasis[[2;;4]]/.{x->0,y->1,z->0})];
 correctAtY=2+(rx.{0,1,0})[[3]];
 checks=<|"sourceNineEqualsRodrigues"->(rodResidual===ConstantArray[0,9]),
   "sourceNineEqualsComplexDefinition"->(complexResidual===ConstantArray[0,9]),
   "coefficientRoundtrip"->(Expand[back-cs]===ConstantArray[0,9]),
   "sameFieldAfterPaperConversion"->(fieldResidual===0),
   "matlabR1NoCSAction"->(raw1===ConstantArray[0,3]),
   "matlabR2EqualsDirect"->(Expand[rot2-direct2]===ConstantArray[0,{5,5}]),
   "IR96Table3EqualsDirect"->(tableResidual===ConstantArray[0,{5,5}]),
   "correctedCSFirstDegreeAction"->(phase1===ConstantArray[0,3]),
   "correctedCSSecondDegreeAction"->(phase2===ConstantArray[0,5]),
   "all25WWeightsExactlyZero"->(wWeights===ConstantArray[0,25]),
   "shortCircuitKeepsPreviousRowIndicesValid"->And@@(MemberQ[{-1,0,1},#[[2]]]&/@pVisited),
   "unconjugatedSourceNotSameCSAction"->(rawCS=!=ConstantArray[0,3]),
   "independentRx90Counterexample"->(legacyAtY===1&&correctAtY===3)|>;
 result=<|"kernel"->$Version,"domain"->"l=0,1,2; scalar theta in [0,Pi], phi real; q real nonzero; A=argument/(q.q); coefficient-column pullback f_new(d)=f_old(A d)",
   "allCoefficientRoundtripResiduals"->Expand[back-cs],"sameFieldPolynomialResidual"->fieldResidual,
   "RodriguesNineResiduals"->rodResidual,"complexDefinitionNineResiduals"->complexResidual,
   "matlabQuadrupoleResidual"->Expand[rot2-direct2],"IR96Table3Residual"->tableResidual,
   "phaseCorrectedDipoleActionResidual"->phase1,"phaseCorrectedQuadrupoleActionResidual"->phase2,
   "legacyAtYFor2PlusZ"->legacyAtY,"specifiedPullbackAtYFor2PlusZ"->correctAtY,
   "sourceCSPhase1"->Diagonal[s1],"sourceCSPhase2"->Diagonal[s2],
   "poleValuesNorth"->(csBasis/.{x->0,y->0,z->1}),"poleValuesSouth"->(csBasis/.{x->0,y->0,z->-1}),
   "checkCount"->Length[checks],"passedCount"->Count[Values[checks],True],"checks"->checks,
   "allExact"->And@@Values[checks]|>;
 If[TrueQ[result["allExact"]],result,Failure["SourceBasisOrRotationMismatch",result]]
]
