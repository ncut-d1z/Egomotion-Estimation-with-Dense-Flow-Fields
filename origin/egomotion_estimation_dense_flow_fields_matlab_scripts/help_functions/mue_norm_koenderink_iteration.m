function [trans rot near x]=mue_norm_koenderink_iteration(setofpoints)
% [trans rot near x]=mue_norm_koenderink_iteration(setofpoints)
% the Koenderink van Doorn algorithm without the factor "mue", which computes the ego translation ("trans") and
% ego rotation ("rot") from direction vectors and corresponding 3d flow
% vectors, both must be stored in one row of "setofpoints"
% "near" stores the values of the nearnesses
% "x" stores the number of iterations




%[trans rot near]=koenderink_iteration 
%liefert translation, rotation und nearness zur�ck
%die setofpoint k�nnen wahlweise �ber ein kugelf�rmiges Flussfeld bestimmt
%werden "fiducialpoints" oder �ber ein k�nstliches Flussfeld
%"perfectpoints"
%Die Initialisierung erfolgt nach Koenderink �ber "koenderink_initial" oder
%�ber Franz "franzeigen"
%als Parameter taucht die Grenze auf, die den Fehlerunterschied zwischen
%zwei Iterationen markiert
start='mue_norm_Koenderink'
nulldiv=0.0000000000001;
genauigkeit=1000;%Parameter
%setofpoints=fiducialpoints(Feld,Schranke);
%setofpoints=perfectpoints;
%[trans rot]=koenderink_initial(setofpoints);
[trans rot]=franzeigen(setofpoints);%(Feld,Schranke);
trans=trans/abs(sqrt(skalarprodukt(trans,trans)));

%allesenkrecht(setofpoints);% pr�ft ob Flu� und Richtung senkrecht sind
[anzahl dumi]=size(setofpoints);
near=zeros(anzahl,1);

grenze=0.0001;
absgrenze=0.0000001;
erroralt=0;
errorneu=0;
fehlerdiv=grenze*10;
x=0;
while ((fehlerdiv>grenze) && (x<200) && (fehlerdiv>absgrenze)) || (x<6)
    x=x+1;
    % near
    for i=1:anzahl
        d=transpose(setofpoints(i,1:3));
        p=transpose(setofpoints(i,4:6));
        near(i)=(-1)*skalarprodukt(trans,p-kreuzprodukt(d,rot))/((1-skalarprodukt(trans,d)^2)+nulldiv);
    end
    sum=0;
    for i=1:anzahl
        sum=sum+near(i);
    end
    if sum<0 %verhindert das falsche Vorzeichen bei der Translation
        for i=1:anzahl
            near(i)=near(i)*(-1);
        end
        trans=trans*(-1);
    end
    
    %transneu
    a=zeros(3,1);
    for i=1:anzahl
        p=transpose(setofpoints(i,4:6));
        a=a+p;
    end
    a=a/anzahl;
    
    b=zeros(3,1);
    for i=1:anzahl
         d=transpose(setofpoints(i,1:3));
         b=b+d;
    end
    b=b/anzahl;

    c=zeros(3,1);
    for i=1:anzahl
         d=transpose(setofpoints(i,1:3));
         c=c+(near(i)*skalarprodukt(trans,d))*d;
    end
    c=c/anzahl;
    
    transneu=(-1)*(a+kreuzprodukt(rot,b)-c);
    transneu=transneu/abs(sqrt(skalarprodukt(transneu,transneu)));
    
    
    %rot
    a=zeros(3,1);
    for i=1:anzahl
        d=transpose(setofpoints(i,1:3));
        p=transpose(setofpoints(i,4:6));
        a=a+kreuzprodukt(p,d);
    end
    a=a/anzahl;
    
    b=zeros(3,1);
    for i=1:anzahl
         d=transpose(setofpoints(i,1:3));
         b=b+near(i)*d;
    end
    b=b/anzahl;
    
    c=zeros(3,1);
    for i=1:anzahl
         d=transpose(setofpoints(i,1:3));
         c=c+skalarprodukt(rot,d)*d;
    end
    c=c/anzahl;
    
    rot=a+kreuzprodukt(trans,b)+c;
    trans=transneu;%erst jetzt zuweisen damit rot mit dem alten trans gerechnet wird
    %rot
    %trans
    
    %Error berechnung
    error=0;
    durchschnitt=0;
    for i=1:anzahl
        d=transpose(setofpoints(i,1:3));
        p=transpose(setofpoints(i,4:6));
        
        pstrich=((-1)*near(i)*(trans-(skalarprodukt(trans,d)*d)))-kreuzprodukt(rot,d);
        error=error+abs(skalarprodukt(p-pstrich,p-pstrich));
        durchschnitt=durchschnitt+abs(sqrt(skalarprodukt(p,p)));
    end
    error=error/anzahl;
    durchschnitt=durchschnitt/anzahl;
    errorwurzel=sqrt(error);
    %fehlerdiv
    erroralt=errorneu;
    errorneu=errorwurzel;
    fehlerdiv=abs(erroralt-errorneu);
    grenze=errorwurzel/(genauigkeit*anzahl);
    %errorwurzel% error anzeigen
end
%Koenderink_rot=rot
%Koenderink_trans=trans
Anzahliterationen=x
sum=0;
for i=1:anzahl
     sum=sum+near(i);
end
sum=sum/anzahl;

trans=trans*sum;
%norm_Koenderink_rot=rot
%norm_Koenderink_trans=trans




 