function field=getflowfield(filename)
% field=getflowfield(filename)
% returns the first multidimensional matrix stored in the file "filename"

x=load(filename);
s=fieldnames(x);
str=char(s(1));
field=getfield(x,str);