function graphs_bias_test(number_of_cores, iterationnumber, savestring)
% graphs_bias_test(number_of_cores, iterationnumber, savestring)
% This function generates a graph of the bias test
% "number_of_cores" defines the number of runs, stored in the main folder
% "savestring"
% "iterationnumber" defines up to which number of flow vectors the
% simulation was run, the number must fit to the "iterationnumber" during
% the data generation


speicherstr=savestring;
von=1;
bis=number_of_cores;
anzahlit=iterationnumber;
anzahlvar=3;
allywerte3=zeros(anzahlit+1,anzahlvar);
allywerte4=zeros(anzahlit+1,anzahlvar);
allywerte5=zeros(anzahlit+1,anzahlvar);
allywerte6=zeros(anzahlit+1,anzahlvar);
for k=von:bis
    str=[speicherstr '/' num2str(k) '/'];
    xwerte=getflowfield([str 'xwerte']);
    %ywerte1=getflowfield([str 'ywerte1']);
    %ywerte2=getflowfield([str 'ywerte2']);
    ywerte3=getflowfield([str 'ywerte3']);
    ywerte4=getflowfield([str 'ywerte4']);
    ywerte5=getflowfield([str 'ywerte5']);
    ywerte6=getflowfield([str 'ywerte6']);
    allywerte3=allywerte3+ywerte3;
    allywerte4=allywerte4+ywerte4;
    allywerte5=allywerte5+ywerte5;
    allywerte6=allywerte6+ywerte6;
end
anzahl=(bis-von)+1;
allywerte3=allywerte3/anzahl;
allywerte4=allywerte4/anzahl;
allywerte5=allywerte5/anzahl;
allywerte6=allywerte6/anzahl;    


anfang=3;
ende=iterationnumber+1;

figure;

faktor=100;
ygross=zeros(ende,1);
for i=anfang:ende
    ygross(i)=faktor/sqrt(xwerte(i));
end
loglog(xwerte(anfang:ende),ygross(anfang:ende),'k-');
hold on;

for i=1:3
%semilogx(xwerte,ywerte1(:,i),'b-');
%hold on;
%semilogx(xwerte,ywerte2(:,i),'b--');
%hold on;
 
loglog(xwerte(anfang:ende),allywerte3(anfang:ende,i),'r-');
hold on;
loglog(xwerte(anfang:ende),allywerte4(anfang:ende,i),'r--');
hold on;

loglog(xwerte(anfang:ende),allywerte5(anfang:ende,i),'g-');
hold on;
loglog(xwerte(anfang:ende),allywerte6(anfang:ende,i),'g--');
hold on;

xlim([100 3000000]);
ylim([0.06 200]);
end
hold off;
legend('a','b','c','d','e');

%Test spezifisch
title('Optimality test');
xlabel('Number of flow vectors');
ylabel('length error vector');
%Test spezifisch
