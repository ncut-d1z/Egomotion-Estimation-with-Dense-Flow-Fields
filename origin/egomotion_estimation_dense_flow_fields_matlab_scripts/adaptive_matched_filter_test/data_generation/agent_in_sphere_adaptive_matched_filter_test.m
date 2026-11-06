function agent_in_sphere_adaptive_matched_filter_test(addederror, savestring)
% agent_in_sphere_adaptive_matched_filter_test(addederror, savestring)
% This function tested the adaptive matched filter against the unadaptive
% matched filter
% an error can be added to the artificial flow vectors by the variable
% "addederror"
% the result is stored in the folder "savestring"

timer=tic;
speicherstr=[savestring '/'];
%Anzahldurchlaeufe
anzahldurchlaeufe=8;
% ini Parameter
anzahlitfuerini=1;

% Allgemeine Parameter
%anzahlwiederbest=35; % zur Bestimmung von besttrans und bestrot
gausspropfehler=addederror; % optionaler Fehler
anzahliterationen=4; % anzahl richtungsvektoren 20*4(hoch)anzahliterationen 

zeichnen=6;
%Trajektorie Eigenschaften
%für alle
stellen=600; 
%für Kreisbahn
kreisradius=0.1;
hoehe=0.3; %auch für sinuskurve
%für Spirale
maxabstand=0.7;
anzahlschleifen=3;
% für sinuskurve
amplithude=0.5;
laengebahn=1;
anzahlperioden=2;
% für kurve+sinuswelle
ksradius=0.35;
ksamplithude=0.25;
ksanzahlperioden=3;
%Trajektorie Eigenschaften Ende

% Eine Trajektorie auswählen
%punkte=kreis_trajektorie(stellen, kreisradius, hoehe);
%punkte=spirale_trajektorie(stellen, maxabstand, anzahlschleifen);
punkte=sinuskurve_trajektorie(stellen, amplithude, laengebahn, anzahlperioden, hoehe);
%punkte=kreis_sinus_trajektorie(stellen, ksradius, ksamplithude, ksanzahlperioden);
[anzahlpunkte dumi]=size(punkte);

%for k=1:anzahlpunkte
     %laenge=abs(sqrt(skalarprodukt(punkte(k,:)',punkte(k,:)')));
     %if laenge>1
       %  Hinweis='Es liegt ein Punkt außerhalb der Kugel Radius 1'
     %end
%end
     

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
  akrot=zeros(anzahlpunkte,1);
  rottransratio=zeros(anzahlpunkte,1);

  akkoeff=zeros(3,5);
  akkoeff(1,1)=1;
  for i=2:(anzahlpunkte-1) 
     %durchlauf=j 
     trajektoriepunkt=i
     
     setofpoints=richtungsvektoren_icosahedron(anzahliterationen); % Generierung Richtungsvektoren
     near=getnear_in_kugel_Drehung(setofpoints, punkte(i,:)', absdrehvektoren(i,:)'); % nearberechnung
     [setofpoints durchtransfluss durchrotfluss]=erzeugeflussausnear(setofpoints, near, alletrans(i,:)', reldrehvektoren(i,:)'); %künstliches Flussfeld
     setofpoints=nonpropfehler(setofpoints, gausspropfehler); % Fehler hinzugabe
     besttrans=alletrans(i,:)';
     bestrot=reldrehvektoren(i,:)';
     akrot(i)=360*abs(sqrt(skalarprodukt(bestrot,bestrot)))/(pi*2);
     rottransratio(i)=durchrotfluss/durchtransfluss;
     
     
     %if i==zeichnen
       % spherical_model_draw2(punkte(i,:)', absdrehvektoren(i,:)');
     %end
     
     
   
     
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
     
     %normtheotrans=theotrans/abs(sqrt(skalarprodukt(theotrans,theotrans)))
     %normtheorot=theorot/abs(sqrt(skalarprodukt(theorot,theorot)))
     
     %normbesttrans=besttrans/abs(sqrt(skalarprodukt(besttrans,besttrans)))
     %normbestrot=bestrot/abs(sqrt(skalarprodukt(bestrot,bestrot)))
     
     
     
     
     
     % Bestberechnung Ende
     
     % initest
     %status='ini berechnung'
     
     
     vordrehmatrix=rodrigues(reldrehvektoren((i-1),:)');
     akkoeff=transfor_koeff(akkoeff, vordrehmatrix);
     
     
     
     
     
     
     ohnekoeff=zeros(3,5);
     ohnekoeff(1,1)=1;
     mitkoeff=akkoeff;
     
     %divkoeff=bestaltkoeff-akkoeff; %findet hier eine Annäherung statt?
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
     [akkoeff aktuelltrans aktuellrot error]=oneit_adaptiv_matched_filter(akkoeff, setofpoints);
     
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
save([speicherstr 'rottransratio'],'rottransratio');


zeitinsec=toc(timer);
zeitinStunden=zeitinsec/3600






