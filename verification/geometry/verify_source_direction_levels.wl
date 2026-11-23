(* Independent Mathematica check of source triangle-subdivision direction ordering.
   This is verification evidence, not a MATLAB execution or a production sampler.
   Returns per-level [N, sum_i i*d_i.x/y/z, sum_i i^2*d_i.x/y/z].
   The signatures cover all 714 rows but do not alone prove byte-for-byte row equality. *)
Module[{phi,octa,vertices,faces,ico,normalize,mid,leaf,groups,calc,sig},
  phi=N[(1+Sqrt[5])/2,30];
  normalize[v_]:=v/Sqrt[v.v];
  mid[u_,v_]:=normalize[(u+v)/2];
  leaf[u_,v_,w_,level_Integer]:=Module[{xy=mid[u,v],xz=mid[u,w],yz=mid[v,w]},
    If[level==0,{normalize[(u+v+w)/3]},
      Join[leaf[u,xz,xy,level-1],leaf[v,xy,yz,level-1],
        leaf[w,xz,yz,level-1],leaf[xy,xz,yz,level-1]]]
  ];
  octa=N[{{{0,0,1},{1,0,0},{0,1,0}},{{0,0,1},{1,0,0},{0,-1,0}},
    {{0,0,1},{-1,0,0},{0,1,0}},{{0,0,1},{-1,0,0},{0,-1,0}},
    {{0,0,-1},{1,0,0},{0,1,0}},{{0,0,-1},{1,0,0},{0,-1,0}},
    {{0,0,-1},{-1,0,0},{0,1,0}},{{0,0,-1},{-1,0,0},{0,-1,0}}},30];
  vertices=Transpose[{{0,0,0,0,-1,-1,1,1,-phi,phi,phi,-phi},
    {-1,-1,1,1,-phi,phi,phi,-phi,0,0,0,0},
    {-phi,phi,phi,-phi,0,0,0,0,-1,-1,1,1}}];
  faces={{1,4,9},{1,5,9},{1,8,5},{1,8,10},{1,10,4},
    {12,2,5},{12,2,3},{12,3,6},{12,6,9},{12,9,5},
    {11,7,10},{11,10,8},{11,8,2},{11,2,3},
    {11,3,7},{2,5,8},{10,4,7},{3,6,7},{6,7,4},{6,4,9}};
  ico=vertices[[#]]& /@ faces;
  groups=<|"octa"->octa,"occluded"->octa[[{2,3,5,6,7,8}]],"icosa"->ico|>;
  sig[ds_]:=Module[{idx=Range[Length[ds]],m=N[ds,MachinePrecision]},
    Join[{Length[m]},Flatten@Table[
      Total[N[idx,MachinePrecision]^pow*m[[All,dim]]],{pow,1,2},{dim,1,3}]]];
  calc=AssociationMap[Function[g,
    Table[sig[Join@@(leaf[#[[1]],#[[2]],#[[3]],level]& /@ groups[g])],
      {level,0,2}]],Keys[groups]];
  ExportString[calc,"RawJSON","Compact"->True]
]
