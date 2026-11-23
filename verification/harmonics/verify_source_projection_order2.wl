(* Only SH source per-coefficient projection; exact rationals and symbolic normalization. *)
Module[{x,y,z,directions,near,sourceBasis,values,grams,source,expected,residuals,result},
  directions={{1,0,0},{0,1,0},{0,0,1},{3/5,4/5,0},
    {0,3/5,4/5},{4/5,0,3/5},{1/3,2/3,2/3}};
  near={2,-1,4,1/2,-3,5/4,11/2};
  sourceBasis={1/Sqrt[4 Pi],-Sqrt[3/(4 Pi)] y,Sqrt[3/(4 Pi)] z,
    -Sqrt[3/(4 Pi)] x,Sqrt[15/(4 Pi)] x y,-Sqrt[15/(4 Pi)] y z,
    Sqrt[5/(16 Pi)] (3 z^2-1),-Sqrt[15/(4 Pi)] x z,
    Sqrt[15/(16 Pi)] (x^2-y^2)};
  values=Table[sourceBasis/.Thread[{x,y,z}->d],{d,directions}];
  grams=FullSimplify[Transpose[values].values];
  source=Table[FullSimplify[
    Sum[near[[i]] values[[i,j]],{i,Length[directions]}]/
    Sum[values[[i,j]]^2,{i,Length[directions]}]],{j,9}];
  expected={37 Sqrt[Pi]/14,-19 Sqrt[3 Pi]/55,361 Sqrt[3 Pi]/220,
    -154 Sqrt[3 Pi]/95,4935 Sqrt[15 Pi]/7082,-1695 Sqrt[15 Pi]/5416,
    25530 Sqrt[5 Pi]/44797,-3075 Sqrt[15 Pi]/3541,
    4360 Sqrt[15 Pi]/15349};
  residuals=FullSimplify[source-expected];
  result=<|"source"->"koeff_aus_near.m", "scope"->"fixed ordnung=2",
    "directions"->directions,"near"->near,"reference"->expected,
    "residuals"->residuals,"denominatorsPositive"->And@@(Positive/@Diagonal[grams]),
    "discreteBasisHasCrossCoupling"->TrueQ[FullSimplify[grams[[2,3]]!=0]],
    "crossGramNumerical"->N[grams[[2,3]],17],
    "allExact"->(residuals===ConstantArray[0,9]&&
      And@@(Positive/@Diagonal[grams])&&TrueQ[FullSimplify[grams[[2,3]]!=0]])|>;
  If[TrueQ[result["allExact"]],result,Failure["SourceProjectionReferenceMismatch",result]]
]