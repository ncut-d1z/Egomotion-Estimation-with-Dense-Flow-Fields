function graph_agent_in_sphere_adaptive_matched_filter_test(savestring)
% graph_agent_in_sphere_adaptive_matched_filter_test(savestring)
% the graph corresponding to the data generation function
% "agent_in_sphere_adaptive_matched_filter_test"
% in the folder "savestring" must be the data


speicherstr=[savestring '/'];

allemittransfehler=getflowfield([speicherstr 'allemittransfehler']);
allemitrotfehler=getflowfield([speicherstr 'allemitrotfehler']);
alleohnetransfehler=getflowfield([speicherstr 'alleohnetransfehler']);
alleohnerotfehler=getflowfield([speicherstr 'alleohnerotfehler']);
rottransratio=getflowfield([speicherstr 'rottransratio']);
[anzahl dumi]=size(rottransratio);
x=50:550;
anzahlzonen=3;
begin=zeros(anzahlzonen+1,1);
ende=zeros(anzahlzonen+1,1);
rotgrenze=0.01;
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
[AX,H1,H2] = plotyy(begin(1):ende(1),alleohnerotfehler(begin(1):ende(1),1),x,rottransratio(x,1));
hold on;
h=plot(begin(1):ende(1),allemitrotfehler(begin(1):ende(1),1), 'r');
hold on;

for z=2:(anzahlzonen+1)
    plot(begin(z):ende(z),alleohnerotfehler(begin(z):ende(z),1));
    hold on;
    plot(begin(z):ende(z),allemitrotfehler(begin(z):ende(z),1), 'r');
    hold on;
end

hold off;

ymin=0.01;
ymax=100;
set(AX(1), 'YScale', 'log');
set(AX(2), 'YScale', 'log');
set(AX(2), 'ylim',   [ymin ymax]);
set(AX(1), 'YTickMode','auto');
set(AX(2), 'YTickMode','auto');
set(get(AX(1),'Ylabel'),'String','angle error in degree'); 
set(get(AX(2),'Ylabel'),'String','ratio between rotational and translational flow');
set(AX(1),'FontSize',20);
set(AX(2),'FontSize',20);

title('rotation error');
xlabel('steps in sinuscurve');
legend([H1 H2 h]);
%legend('boxoff');



figure;
%plotyy(x,alleohnerotfehler(:,1),x,akrot(:,1));
[AX,H1,H2] = plotyy(x,alleohnetransfehler(x,1),x,rottransratio(x,1));
hold on;
plot(x,allemittransfehler(x,1), 'r');
%hold on;
%plot(akrotfak(:,1), 'k');
hold off;

ymin=0.01;
ymax=100;
set(AX(1), 'YScale', 'log');
set(AX(2), 'YScale', 'log');
set(AX(2), 'ylim',   [ymin ymax]);
set(AX(1), 'YTickMode','auto');
set(AX(2), 'YTickMode','auto');
set(get(AX(1),'Ylabel'),'String','angle error in degree'); 
set(get(AX(2),'Ylabel'),'String','ratio between rotational and translational flow');
set(AX(1),'FontSize',20);
set(AX(2),'FontSize',20);

title('translation error');
xlabel('steps in sinuscurve');
legend('a','b','c');
%ylabel('euclid difference');

 
