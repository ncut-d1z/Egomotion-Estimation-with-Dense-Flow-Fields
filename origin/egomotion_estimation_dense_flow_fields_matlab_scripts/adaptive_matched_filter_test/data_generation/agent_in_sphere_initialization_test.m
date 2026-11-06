function agent_in_sphere_initialization_test(savestring)
% agent_in_sphere_initialization_test(savestring)
% This function tests the initialization behavior in the adaptive matched
% filter test
% the result is stored in the folder "savestring"


timer=tic;
speicherstr=[savestring '/'];
%Anzahldurchlaeufe
anzahldurchlaeufe=1;
% ini Parameter
anzahlitfuerini=1;

% Allgemeine Parameter
%anzahlwiederbest=35; % zur Bestimmung von besttrans und bestrot
%gausspropfehler=0; % optionaler Fehler
anzahliterationen=2; % anzahl richtungsvektoren 20*4(hoch)anzahliterationen

%Trajektorie Eigenschaften
%für alle
stellen=35; 
%für Kreisbahn
kreisradius=0.6;
hoehe=0.3;
%für Spirale
maxabstand=0.7;
anzahlschleifen=3;
% für sinuskurve
amplithude=0.6;
laengebahn=1;
anzahlperioden=3;
% für kurve+sinuswelle
ksradius=0.35;
ksamplithude=0.25;
ksanzahlperioden=3;
%Trajektorie Eigenschaften Ende

% Eine Trajektorie auswählen
punkte=kreis_trajektorie(stellen, kreisradius, hoehe);
%punkte=spirale_trajektorie(stellen, maxabstand, anzahlschleifen);
%punkte=sinuskurve_trajektorie(stellen, amplithude, laengebahn, anzahlperioden);
%punkte=kreis_sinus_trajektorie(stellen, ksradius, ksamplithude, ksanzahlperioden);
[anzahlpunkte dumi]=size(punkte);

% trans und rot aus trajektorie
[alletrans absdrehvektoren reldrehvektoren]=trajektorie_aus_punkten(punkte);


%Schleife für durchläufe
allemittransfehler=zeros(anzahlpunkte,anzahlitfuerini);
allemitrotfehler=zeros(anzahlpunkte,anzahlitfuerini);
alleohnetransfehler=zeros(anzahlpunkte,anzahlitfuerini);
alleohnerotfehler=zeros(anzahlpunkte,anzahlitfuerini);

for j=1:anzahldurchlaeufe
   

% Start Trajektorie
  mittransfehler=zeros(anzahlpunkte,anzahlitfuerini);
  mitrotfehler=zeros(anzahlpunkte,anzahlitfuerini);
  ohnetransfehler=zeros(anzahlpunkte,anzahlitfuerini);
  ohnerotfehler=zeros(anzahlpunkte,anzahlitfuerini);

  akkoeff=zeros(3,5);
  akkoeff(1,1)=1;
  for i=2:(anzahlpunkte-1) 
     %durchlauf=j 
     trajektoriepunkt=i
     
     setofpoints=richtungsvektoren_icosahedron(anzahliterationen); % Generierung Richtungsvektoren
     near=getnear_in_kugel_Drehung(setofpoints, punkte(i,:)', absdrehvektoren(i,:)'); % nearberechnung
     [setofpoints durchtransfluss durchrotfluss]=erzeugeflussausnear(setofpoints, near, alletrans(i,:)', reldrehvektoren(i,:)'); %künstliches Flussfeld
     %setofpoints=propfehler(setofpoints, gausspropfehler); % Fehler hinzugabe
     
     besttrans=alletrans(i,:)';
     bestrot=reldrehvektoren(i,:)';
     %durchtransfluss
     %durchrotfluss
     
     
     % Bestberechnung
     %status='bestberechnung'
     
     %bestaltkoeff=zeros(3,5);
     %bestaltkoeff(1,1)=1;
     %for k=1:anzahlwiederbest
     %    [bestneukoeff besttrans bestrot error]=oneit_adaptiv_matched_filter(bestaltkoeff, setofpoints);
     %    bestaltkoeff=bestneukoeff;
     %   
     %end
     % Bestberechnung Ende
     
     % initest
     %status='ini berechnung'
     
     ohnekoeff=zeros(3,5);
     ohnekoeff(1,1)=1;
     mitkoeff=akkoeff;
     
     %%divkoeff=bestaltkoeff-akkoeff; %findet hier eine Annäherung statt?
     %bestneukoeff
     %akkoeff
     
     for k=1:anzahlitfuerini
        
         
         [ohnekoeff ohnetrans ohnerot error]=oneit_adaptiv_matched_filter(ohnekoeff, setofpoints);
         [mitkoeff mittrans mitrot error]=oneit_adaptiv_matched_filter(mitkoeff, setofpoints);
         
         mittransfehler(i,k)=normrelfehler(besttrans,mittrans);
         mitrotfehler(i,k)=normrelfehler(bestrot,mitrot);
         ohnetransfehler(i,k)=normrelfehler(besttrans,ohnetrans);
         ohnerotfehler(i,k)=normrelfehler(bestrot,ohnerot);
     end
     
     %eigentlicher Algorithmus
     [akkoeff aktrans akrot error]=oneit_adaptiv_matched_filter(akkoeff, setofpoints);
     
  end
  allemittransfehler=allemittransfehler+mittransfehler;
  allemitrotfehler=allemitrotfehler+mitrotfehler;
  alleohnetransfehler=alleohnetransfehler+ohnetransfehler;
  alleohnerotfehler=alleohnerotfehler+ohnerotfehler;
  
end
allemittransfehler=allemittransfehler/anzahldurchlaeufe;
allemitrotfehler=allemitrotfehler/anzahldurchlaeufe;
alleohnetransfehler=alleohnetransfehler/anzahldurchlaeufe;
alleohnerotfehler=alleohnerotfehler/anzahldurchlaeufe;

dumi='ende'


save([speicherstr 'allemittransfehler'],'allemittransfehler');
save([speicherstr 'allemitrotfehler'],'allemitrotfehler');
save([speicherstr 'alleohnetransfehler'],'alleohnetransfehler');
save([speicherstr 'alleohnerotfehler'],'alleohnerotfehler');

zeitinsec=toc(timer);
zeitinStunden=zeitinsec/3600







