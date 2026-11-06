function agent_in_sphere_adaptive_matched_filter_many_parameter(savestring)
% agent_in_sphere_adaptive_matched_filter_many_parameter(savestring)
% This function tested different update rates of the adaptive matched
% filter
% the update rates are every 1, 5, 11 or 21 frame
% the result is stored in the folder "savestring"


timer=tic;
anzahlzeitkonstanten=4;
zeitkonstanten=[1 5 11 21];
speicherstr=[savestring '/'];
%Anzahldurchlaeufe
anzahldurchlaeufe=1;
% ini Parameter
anzahlitfuerini=1;

% Allgemeine Parameter
%anzahlwiederbest=35; % zur Bestimmung von besttrans und bestrot
%gausspropfehler=0; % optionaler Fehler
anzahliterationen=2; % anzahl richtungsvektoren 20*4(hoch)anzahliterationen

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

for k=1:anzahlpunkte
     laenge=abs(sqrt(skalarprodukt(punkte(k,:)',punkte(k,:)')));
     if laenge>1
         Hinweis='Es liegt ein Punkt außerhalb der Kugel Radius 1'
     end
end
     

% trans und rot aus trajektorie
[alletrans absdrehvektoren reldrehvektoren]=trajektorie_aus_punkten(punkte);


%Schleife für durchläufe
allemittransfehler=zeros(anzahlpunkte,anzahlzeitkonstanten);
allemitrotfehler=zeros(anzahlpunkte,anzahlzeitkonstanten);
%alleohnetransfehler=zeros(anzahlpunkte,anzahlitfuerini);
%alleohnerotfehler=zeros(anzahlpunkte,anzahlitfuerini);

for j=1:anzahldurchlaeufe
   
  mitkoeff=zeros(3,5,anzahlzeitkonstanten);
  for k=1:anzahlzeitkonstanten
      mitkoeff(1,1,k)=1;
  end
% Start Trajektorie
  mittransfehler=zeros(anzahlpunkte,anzahlzeitkonstanten);
  mitrotfehler=zeros(anzahlpunkte,anzahlzeitkonstanten);
  %ohnetransfehler=zeros(anzahlpunkte,anzahlitfuerini);
  %ohnerotfehler=zeros(anzahlpunkte,anzahlitfuerini);
  akrot=zeros(anzahlpunkte,1);

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
     akrot(i)=360*abs(sqrt(skalarprodukt(bestrot,bestrot)))/(pi*2);
     
     
     %if i==zeichnen
     %   spherical_model_draw2(punkte(i,:)', absdrehvektoren(i,:)');
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
     for k=1:anzahlzeitkonstanten
        mitkoeff(:,:,k)=transfor_koeff(mitkoeff(:,:,k), vordrehmatrix);
     end
     
     
     
     
     
    % ohnekoeff=zeros(3,5);
    % ohnekoeff(1,1)=1;
     for k=1:anzahlzeitkonstanten
         if mod(i,zeitkonstanten(k))==0
            mitkoeff(:,:,k)=akkoeff;
         end
     end
     %divkoeff=bestaltkoeff-akkoeff; %findet hier eine Annäherung statt?
     %bestneukoeff
     %akkoeff
     
      
        
         
         for k=1:anzahlzeitkonstanten
     
         %[ohnekoeff ohnetrans ohnerot error]=oneit_adaptiv_matched_filter(ohnekoeff, setofpoints);
         
     
     
         [dumi mittrans mitrot error]=oneit_adaptiv_matched_filter(mitkoeff(:,:,k), setofpoints);
         
       
         mittransfehler(i,k)=normrelfehler(besttrans,mittrans);
         mitrotfehler(i,k)=normrelfehler(bestrot,mitrot);
         end
         
         
         
         
         
         %ohnetransfehler(i,k)=transfehler_div(besttrans,ohnetrans);
         %ohnerotfehler(i,k)=rotfehler_div(bestrot,ohnerot);
      
         
      
     
     %eigentlicher Algorithmus
     [akkoeff aktuelltrans aktuellrot error]=oneit_adaptiv_matched_filter(akkoeff, setofpoints);
     
  end
  allemittransfehler=allemittransfehler+mittransfehler;
  allemitrotfehler=allemitrotfehler+mitrotfehler;
  %alleohnetransfehler=alleohnetransfehler+ohnetransfehler;
  %alleohnerotfehler=alleohnerotfehler+ohnerotfehler;
  
end
allemittransfehler=allemittransfehler/anzahldurchlaeufe;
allemitrotfehler=allemitrotfehler/anzahldurchlaeufe;
%alleohnetransfehler=alleohnetransfehler/anzahldurchlaeufe;
%alleohnerotfehler=alleohnerotfehler/anzahldurchlaeufe;

dumi='ende'


save([speicherstr 'allemittransfehler'],'allemittransfehler');
save([speicherstr 'allemitrotfehler'],'allemitrotfehler');
%save([speicherstr 'alleohnetransfehler'],'alleohnetransfehler');
%save([speicherstr 'alleohnerotfehler'],'alleohnerotfehler');
save([speicherstr 'akrot'],'akrot');

zeitinsec=toc(timer);
zeitinStunden=zeitinsec/3600





