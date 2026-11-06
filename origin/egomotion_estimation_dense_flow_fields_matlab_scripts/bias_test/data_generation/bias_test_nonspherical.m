function bias_test_nonspherical(corenumber, iterationnumber, savestring)
% bias_test_nonspherical(corenumber, iterationnumber, savestring)
% This function is designed to run on many cores in parallel
% "corenumber" initialize the random generator for this run
% the main folder is defined by "savestring", in "savestring" are generated
% a subfolder for every run, where the name of the subfolder is the
% "corenumber"
% "iterationnumber" defines up to which number of flow vectors the
% simulation runs
% in this test the flow vector errors depend not on the length of the
% vectors and the flow vectors are not spherically distributed
% three variances of the gaussian error are tested: 0.6, 2.4 and 9.6


zahl=corenumber;
RandStream.setDefaultStream(RandStream('mt19937ar','seed', zahl));

fixedtrans=getvektor;
fixedrot=getvektor;


RandStream.setDefaultStream(RandStream('mt19937ar','seed', zahl));
timer=tic;

anzahlvar=3; %die Anzahl der getesteten varianzen


durchlaeufe=1;% 25 %Anzahl Durchläufe, bei allen Tests gleich
% Fehlerprozent, Parameter und iteration sind die getesteten Parameter
Parameter=0;
%iteration=2;% 8*4^iterationen
allefehlerprozent=[0.6 2.4 9.6];

%Ende Parameter

%Test spezifisch
zahlinstr=num2str(zahl);
speicherstr=[savestring '/' zahlinstr '/'];
anzahlpunkte=iterationnumber; %9
%Test spezifisch


xwerte=zeros(anzahlpunkte+1,1);
ywerte1=zeros(anzahlpunkte+1,anzahlvar);
ywerte2=zeros(anzahlpunkte+1,anzahlvar);
ywerte3=zeros(anzahlpunkte+1,anzahlvar);
ywerte4=zeros(anzahlpunkte+1,anzahlvar);
ywerte5=zeros(anzahlpunkte+1,anzahlvar);
ywerte6=zeros(anzahlpunkte+1,anzahlvar);

for v=1:anzahlvar
  for i=0:anzahlpunkte
    
    
    %Test spezifisch
    %x Paprameter berechenen
    iteration=i;
    xwerte(i+1)=(8*4^iteration)*(6/8);  
    %Test spezifisch
    
    fehler=zeros(6,1);
    for j=1:durchlaeufe
        Varianz=v
        Fortschritt=i
        testlauf=j
        rotzutrans=1;
        [theotrans theorot setofpoints realsetofpoints near durchlaenge]=fixed_kugel_symmetric_perfectpoints_randomnear(i,allefehlerprozent(v),rotzutrans,Parameter,fixedtrans,fixedrot);
        [franztrans franzrot]=franzeigen(setofpoints);
        [koentrans1 koenrot1 near x]=norm_koenderink_iteration(setofpoints);
        [koentrans2 koenrot2 near x]=mue_norm_koenderink_iteration(setofpoints);
        
        fehler(1)=fehler(1)+transfehler_div(theotrans,franztrans);
        fehler(2)=fehler(2)+transfehler_div(theorot,franzrot);
        fehler(3)=fehler(3)+transfehler_div(theotrans,koentrans1);
        fehler(4)=fehler(4)+transfehler_div(theorot,koenrot1);
        fehler(5)=fehler(5)+transfehler_div(theotrans,koentrans2);
        fehler(6)=fehler(6)+transfehler_div(theorot,koenrot2);
    
    
    end
    ywerte1(i+1,v)=fehler(1)/durchlaeufe;
    ywerte2(i+1,v)=fehler(2)/durchlaeufe;
    ywerte3(i+1,v)=fehler(3)/durchlaeufe;
    ywerte4(i+1,v)=fehler(4)/durchlaeufe;
    ywerte5(i+1,v)=fehler(5)/durchlaeufe;
    ywerte6(i+1,v)=fehler(6)/durchlaeufe;
 
  end
end
save([speicherstr 'xwerte'],'xwerte');
save([speicherstr 'ywerte1'],'ywerte1');
save([speicherstr 'ywerte2'],'ywerte2');
save([speicherstr 'ywerte3'],'ywerte3');
save([speicherstr 'ywerte4'],'ywerte4');
save([speicherstr 'ywerte5'],'ywerte5');
save([speicherstr 'ywerte6'],'ywerte6');

zeitinsec=toc(timer);
zeitinStunden=zeitinsec/3600

