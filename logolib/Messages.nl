; Nederlandse versie van het Berkeley Logo Messages-bestand (o.b.v. versie 6.2)
; Foutmeldingen -- de positie in dit bestand komt overeen met het foutnummer
; Regels die met een puntkomma beginnen tellen niet mee in de nummering
;  en mogen vrij worden toegevoegd.
Logo: fatale interne fout.
geen geheugen meer
stapeloverloop
schildpad buiten beeld
%p vindt %s geen geldige invoer
%p leverde geen uitvoer aan %p
niet genoeg invoer voor %p
%p vindt %s geen geldige invoer
te veel invoer voor %p
Je zegt niet wat er met %s moet gebeuren
te veel haakjes '('
%s heeft geen waarde
onverwachte ')'
Ik weet niet hoe ik %p moet uitvoeren
Kan geen CATCH-merkteken vinden voor %p
%p is al gedefinieerd
Bezig met stoppen...
Al aan het meeschrijven (DRIBBLE)
Bestandssysteemfout: %p
Aangenomen dat je ALSANDERS bedoelt, niet ALS
%p overschaduwd door lokale variabele in procedure-aanroep
Throw "Fout
%p is een primitief commando
Kan LEER niet gebruiken binnen een procedure
Ik weet niet hoe ik %p moet uitvoeren
%p zonder TEST
onverwachte ']'
onverwachte '}'
kon grafische omgeving niet initialiseren
Macro gaf %s terug in plaats van een lijst
Je zegt niet wat er met %s moet gebeuren
%p kan alleen binnen een procedure gebruikt worden
APPLY vindt %s geen geldige invoer
EIND midden in een meerregelige instructie in %p
Logo: geen geheugen meer.
%p
EIND midden in een meerregelige instructie
Foutieve standaardwaarde voor optionele invoer: %s
Kan UITVOER of STOP niet gebruiken binnen RUNRESULT
Aangenomen dat je '%p' bedoelt, niet %p
Kan bestand %p niet openen
Bestand %p is al open
Bestand %p is niet open
Opdrachtenlijst %s bevat meer dan één expressie
Variabelenaam %s is zowel dynamisch als in het huidige object gedefinieerd
; Niet-foutmeldingen (geen foutcode hiervoor)
Bedankt voor het gebruik van Logo.
Fijne dag verder.
Sorry, geen shell beschikbaar op de Mac.
Typ EXIT om terug te gaan naar Logo.
  in %s\n%s
Erract-lus
Bezig met pauzeren...
stopt
levert op
Bestand niet gevonden: %t\n
Kan TOETS? niet gebruiken, geen FIONREAD op dit systeem
Kan %p niet gebruiken, geen wxWidgets op dit systeem
Niet genoeg geheugen
Ik kan dat bestand niet openen
Bestand is al open
Bestand is niet open
Kenmerklijst
Welkom bij Berkeley Logo versie %t
Je moet in een procedure zitten om UITVOER of STOP te gebruiken.
Waarschuwing: niet genoeg geheugen om de garbage collector te draaien.
GC uitgeschakeld - bewaar belangrijke gegevens en stop het programma!
%s gedefinieerd\n
Maak %s %s
leer %p\neind\n\n
Kenmerklijst %s = %s\n
Geen hulp beschikbaar.\n
Geen hulp beschikbaar over %p.\n
--meer--
; Speciale Logo-woorden, vooral gebruikt in door Logo gegenereerde meldingen
; WELWAAR en NIETWAAR (officiële SuperLogo-termen) worden gegenereerd door
; predicaten en geaccepteerd door ALS etc.
welwaar
nietwaar
; Einde van een procedure
eind
; Namen van primitieven die de evaluator speciaal behandelt
; (nog steeds nodig om ze met COPYDEF te matchen bij wijzigingen hier)
uitvoer
stop
ga
merk
als
alsanders
leer
.macro
; Speciale CATCH-merktekens
topniveau
systeem
fout
; Hoe "geen waarde" wordt weergegeven in foutmeldingen
niets
; Uitvoer van SCREENMODE
tekstscherm
splitsscherm
volscherm
; Uitvoer van PENMODE
verf
gum
omgekeerd
; Uitvoer van TURTLEMODE
omslaan
hek
venster
; HELP zet infix-operatoren +-*/=<> om in deze namen
telop
verschil
vermenigvuldig
deeldoor
gelijk?
kleiner?
groter?
kleinergelijk?
grotergelijk?
ongelijk?
; Object-gerelateerd
naam
klasse
zelf
kenteken
initlijst
bestaat
