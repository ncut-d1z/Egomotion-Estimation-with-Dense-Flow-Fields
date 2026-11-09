(* Read-only Mathematica validation of all 714 ordered source-derived directions.
   Compare every XYZ coordinate to the frozen candidate JSON, not just counts
   or row-weighted signatures. Run this file via Get["...wl"] in Wolfram Language.
   No MATLAB execution, no production sampler, and no Golden approval. *)
Module[{referencePath,stored,phi,octa,vertices,faces,icosa,normalize,mid,leaf,
   groups,results={},actual,expected,delta,maximumError=0.,failedCoordinates=0,
   sampleCount=0,maxNormError=0.,category,level,result},
  referencePath=FileNameJoin[{DirectoryName[$InputFileName],
     "source_direction_levels_0_2.v1.json"}];
  stored=Import[referencePath,"RawJSON"];
  phi=N[(1+Sqrt[5])/2,30];
  normalize[v_]:=v/Sqrt[v.v];
  mid[u_,v_]:=normalize[(u+v)/2];
  leaf[u_,v_,w_,depth_Integer]:=Module[{xy=mid[u,v],xz=mid[u,w],yz=mid[v,w]},
    If[depth==0,{normalize[(u+v+w)/3]},
      Join[leaf[u,xz,xy,depth-1],leaf[v,xy,yz,depth-1],
        leaf[w,xz,yz,depth-1],leaf[xy,xz,yz,depth-1]]]];
  octa=N[{{{0,0,1},{1,0,0},{0,1,0}},{{0,0,1},{1,0,0},{0,-1,0}},
    {{0,0,1},{-1,0,0},{0,1,0}},{{0,0,1},{-1,0,0},{0,-1,0}},
    {{0,0,-1},{1,0,0},{0,1,0}},{{0,0,-1},{1,0,0},{0,-1,0}},
    {{0,0,-1},{-1,0,0},{0,1,0}},{{0,0,-1},{-1,0,0},{0,-1,0}}},30];
  vertices=Transpose[{{0,0,0,0,-1,-1,1,1,-phi,phi,phi,-phi},
    {-1,-1,1,1,-phi,phi,phi,-phi,0,0,0,0},
    {-phi,phi,phi,-phi,0,0,0,0,-1,-1,1,1}}];
  (* This is the complete ORIGINAL F order: 20 rows; omitting {11,3,7}
     falsely creates only 19 faces and must fail the shape check. *)
  faces={{1,4,9},{1,5,9},{1,8,5},{1,8,10},{1,10,4},
    {12,2,5},{12,2,3},{12,3,6},{12,6,9},{12,9,5},
    {11,7,10},{11,10,8},{11,8,2},{11,2,3},{11,3,7},
    {2,5,8},{10,4,7},{3,6,7},{6,7,4},{6,4,9}};
  icosa=vertices[[#]]& /@ faces;
  groups=<|"octa"->octa,"occluded"->octa[[{2,3,5,6,7,8}]],
    "icosa"->icosa|>;
  If[Length[faces]!=20 || Length[icosa]!=20,
    Return[Failure["OriginalFaceTableMismatch",<|"faceCount"->Length[faces]|>]]];
  Do[
    actual=N[Join@@(leaf[#[[1]],#[[2]],#[[3]],level]& /@ groups[category]),
      MachinePrecision];
    expected=stored["levels"][category][ToString[level]]["directions_ordered"];
    If[Dimensions[actual]=!=Dimensions[expected],
      Return[Failure["OrderedDirectionShapeMismatch",
        <|"category"->category,"level"->level,
          "actualShape"->Dimensions[actual],
          "expectedShape"->Dimensions[expected]|>]]];
    delta=Abs[actual-expected];
    maximumError=Max[maximumError,Max[Flatten[delta]]];
    failedCoordinates+=Count[Flatten[delta],v_/;v>10^-12];
    sampleCount+=Length[actual];
    maxNormError=Max[maxNormError,
      Max[Abs[(#.#-1)& /@ actual]]];
    AppendTo[results,<|"category"->category,"level"->level,
      "rows"->Length[actual],"maxAbsCoordinateDifference"->Max[Flatten[delta]],
      "coordinatesAbove1eMinus12"->Count[Flatten[delta],v_/;v>10^-12]|>],
    {category,{"octa","occluded","icosa"}},{level,0,2}];
  result=<|"source"->"Original four MATLAB geometric functions (read-only)",
    "candidateReference"->referencePath,
    "faceCount"->Length[faces],"rows"->sampleCount,
    "coordinateComparisons"->3 sampleCount,
    "globalMaxAbsCoordinateDifference"->maximumError,
    "coordinatesAbove1eMinus12"->failedCoordinates,
    "maxSquaredNormError"->maxNormError,
    "perLevel"->results,
    "allComparedWithin1eMinus12"->(sampleCount==714 &&
      failedCoordinates==0&&maximumError<=10^-12)|>;
  If[TrueQ[result["allComparedWithin1eMinus12"]],result,
    Failure["FullOrderedDirectionComparisonFailed",result]]
]