function graph_agent_in_sphere_initialization_test(savestring)
% graph_agent_in_sphere_initialization_test(savestring)
% the graph corresponding to the data generation function
% "agent_in_sphere_initialization_test"
% in the folder "savestring" must be the data


speicherstr=[savestring '/'];

allemittransfehler=getflowfield([speicherstr 'allemittransfehler']);
allemitrotfehler=getflowfield([speicherstr 'allemitrotfehler']);
alleohnetransfehler=getflowfield([speicherstr 'alleohnetransfehler']);
alleohnerotfehler=getflowfield([speicherstr 'alleohnerotfehler']);
%akrot=getflowfield([speicherstr 'akrot']);
%[anzahl dumi]=size(akrot);
%x=1:anzahl;

figure;
semilogy(allemittransfehler(1:13,1), 'r-');
hold on;
semilogy(allemitrotfehler(:,1), 'b-');
hold off;

 

title('error during initialisation');
xlabel('steps in circle');
ylabel('angle error in degree');

 
