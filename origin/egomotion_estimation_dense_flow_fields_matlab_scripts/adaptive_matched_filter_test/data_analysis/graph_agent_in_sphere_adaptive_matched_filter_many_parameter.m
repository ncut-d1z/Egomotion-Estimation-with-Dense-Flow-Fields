function graph_agent_in_sphere_adaptive_matched_filter_many_parameter(savestring)
% graph_agent_in_sphere_adaptive_matched_filter_many_parameter(savestring)
% the graph corresponding to the data generation function
% "agent_in_sphere_adaptive_matched_filter_many_parameter"
% in the folder "savestring" must be the data


speicherstr=[savestring '/'];

allemittransfehler=getflowfield([speicherstr 'allemittransfehler']);
allemitrotfehler=getflowfield([speicherstr 'allemitrotfehler']);
rottransratio=getflowfield([speicherstr 'rottransratio']);
[anzahl dumi]=size(rottransratio);
x=50:550;
anzahlzonen=3;
begin=zeros(anzahlzonen+1,1);
ende=zeros(anzahlzonen+1,1);
rotgrenze=0.02;
j=1;
begin(1)=50;
zone=0;
for i=50:550
   if (rottransratio(i,1)<rotgrenze) && (zone==0)
       ende(j)=i;
       j=j+1;
       zone=1;
   end
   if (rottransratio(i,1)>rotgrenze) && (zone==1)
       begin(j)=i;
       
       zone=0;
   end
end
ende(anzahlzonen+1)=550;



figure;
for z=1:(anzahlzonen+1)
    kurve=zeros(600,1);
    kurve(begin(z):ende(z))=allemitrotfehler(begin(z):ende(z),1);
    h1=plot(begin(z):ende(z),kurve(begin(z):ende(z)), 'k');
    hold on;
    kurve=zeros(600,1);
    kurve(begin(z):ende(z))=kurveglaetten(allemitrotfehler(begin(z):ende(z),2),2);
    h2=plot(begin(z)+2:ende(z)-2,kurve(begin(z)+2:ende(z)-2), 'g');
    hold on;
    kurve=zeros(600,1);
    kurve(begin(z):ende(z))=kurveglaetten(allemitrotfehler(begin(z):ende(z),3),5);
    h3=plot(begin(z)+5:ende(z)-5,kurve(begin(z)+5:ende(z)-5), 'b');
    hold on;
    kurve=zeros(600,1);
    kurve(begin(z):ende(z))=kurveglaetten(allemitrotfehler(begin(z):ende(z),4),10);
    h4=plot(begin(z)+10:ende(z)-10,kurve(begin(z)+10:ende(z)-10), 'm');
    hold on;
end

hold off;

ylabel('angle error in degree');
xlabel('steps in sinuscurve');
legend([h1 h2 h3 h4]);
 



figure;
kurve=allemittransfehler(:,1);
plot(x,kurve(x),'k');
hold on;
kurve=kurveglaetten(allemittransfehler(:,2),2);
plot(x,kurve(x),'g');
hold on;
kurve=kurveglaetten(allemittransfehler(:,3),5);
plot(x,kurve(x),'b');
hold on;
kurve=kurveglaetten(allemittransfehler(:,4),10);
plot(x,kurve(x),'m');
hold off;

ylabel('angle error in degree');
xlabel('steps in sinuscurve');
legend('a','b','c','d');
 
