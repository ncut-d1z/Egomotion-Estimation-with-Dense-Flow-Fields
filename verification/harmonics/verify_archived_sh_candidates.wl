(* AN-G06/07/08: archived MATLAB-CS m-order is independent of canonical paper order.
   Symbolic checks only, not executable C++/MATLAB or approved Golden. *)
Module[{x,y,z,sourceBasis,atX,expected06,constant,oldCoefficients,
  transformedCoefficients,rotatedField,expectedField,r3,r5,rotated,
  dipoleResidual,quadrupoleResidual,fieldResidual,result},
  sourceBasis={
    1/Sqrt[4 Pi],-Sqrt[3/(4 Pi)] y,Sqrt[3/(4 Pi)] z,-Sqrt[3/(4 Pi)] x,
    Sqrt[15/(4 Pi)] x y,-Sqrt[15/(4 Pi)] y z,
    Sqrt[5/(16 Pi)] (3 z^2-1),-Sqrt[15/(4 Pi)] x z,
    Sqrt[15/(16 Pi)] (x^2-y^2)};
  expected06={1/(2 Sqrt[Pi]),0,0,-Sqrt[3/(4 Pi)],0,0,
    -Sqrt[5/(16 Pi)],0,Sqrt[15/(16 Pi)]};
  atX=FullSimplify[sourceBasis/.{x->1,y->0,z->0}];
  constant=FullSimplify[Sqrt[4 Pi] sourceBasis[[1]]];
  r3={{0,0,-1},{0,1,0},{1,0,0}};
  r5={{-1,0,0,0,0},{0,0,0,-1,0},{0,0,1,0,0},
    {0,1,0,0,0},{0,0,0,0,-1}};
  rotated[f_]:=Expand[f/.{x->-y,y->x,z->z}];
  dipoleResidual=Table[
    FullSimplify[rotated[sourceBasis[[j+1]]]-
      Sum[r3[[i,j]]sourceBasis[[i+1]],{i,1,3}]],{j,1,3}];
  quadrupoleResidual=Table[
    FullSimplify[rotated[sourceBasis[[j+4]]]-
      Sum[r5[[i,j]]sourceBasis[[i+4]],{i,1,5}]],{j,1,5}];
  oldCoefficients={1,1,2,3,1,2,3,4,5};
  transformedCoefficients={1,-3,2,1,-1,-4,3,2,-5};
  rotatedField=rotated[oldCoefficients.sourceBasis];
  expectedField=transformedCoefficients.sourceBasis;
  fieldResidual=FullSimplify[Expand[rotatedField-expectedField]];
  result=<|"kernel"->$Version,
    "AN_G06_9Residuals"->FullSimplify[atX-expected06],
    "AN_G07_constantOne"->constant,
    "AN_G08_R3_3Residuals"->dipoleResidual,
    "AN_G08_R5_5Residuals"->quadrupoleResidual,
    "AN_G08_9CoefficientsFieldResidual"->fieldResidual,
    "allExact"->(atX===expected06&&constant===1&&
       dipoleResidual===ConstantArray[0,3]&&
       quadrupoleResidual===ConstantArray[0,5]&&fieldResidual===0)|>;
  If[TrueQ[result["allExact"]],result,Failure["ArchivedSphericalHarmonicsMismatch",result]]
]
