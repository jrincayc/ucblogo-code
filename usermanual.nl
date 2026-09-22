Berkeley Logo Gebruikershandleiding

 *	Copyright (C) 1993 by the Regents of the University of California
 *
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program.  If not, see <https://www.gnu.org/licenses/>.

Dit is een programma dat nog steeds in ontwikkeling is. Er ontbreekt nog van
alles, waaronder fatsoenlijke documentatie. Deze handleiding gaat ervan uit
dat je al kunt programmeren in Logo, en behandelt alleen de details van deze
nieuwe implementatie. Lees _Computer_Science_Logo_Style,_Volume_1:_
_Symbolic_Computing_ van Brian Harvey (MIT Press, 1997) voor een inleiding
tot Logo-programmeren met de nadruk op symbolisch rekenen.

Dit zijn de bijzondere kenmerken van dit Logo-dialect:

	Broncode-compatibel tussen Unix, DOS, Windows en Mac.

	Arrays met willekeurige toegang.

	Variabel aantal invoerwaarden voor zelfgedefinieerde procedures.

	Mutators voor lijststructuren (gevaarlijk).

	Pauzeren bij een fout, en andere verbeteringen aan foutafhandeling.

	Commentaar en vervolgregels; opmaak blijft behouden wanneer
	procedure-definities worden opgeslagen of bewerkt.

	Tokenisatie in Terrapin-stijl (bv. [2+3] is een lijst met één
	element) maar syntax in LCSI-stijl (geen speciale vormen behalve
	LEER). Het beste van beide werelden.

	Instructie- en expressiesjablonen als volwaardig gegevenstype (zie
	APPLY).

	Macro's.

Kenmerken die *niet* in Berkeley Logo zitten: robotica, muziek, GUI's,
animatie, parallellisme en multimedia. Koop daarvoor een commerciële versie.


GETTER/SETTER-VARIABELESYNTAX
==============================

Logo maakt onderscheid tussen PROCEDURES en VARIABELEN. Een procedure is een
reeks instructies om een berekening uit te voeren; een variabele is een
benoemde container die een gegevenswaarde bevat, zoals een getal, woord,
lijst of array.

In de traditionele Logo-syntax staat een niet-numeriek woord dat zonder
leestekens wordt getypt voor een verzoek om de procedure met die naam aan te
roepen. Een woord met een aanhalingsteken ervoor staat voor het woord zelf.
Bijvoorbeeld, in de instructie
	PRINTPERREGEL EERSTE "WOORD
worden de procedures EERSTE en PRINTPERREGEL aangeroepen, maar de procedure
WOORD niet; het woord W-O-O-R-D is de invoer voor EERSTE.

Hoe zit dat met variabelen? Er zijn twee dingen die je met een variabele kan
doen: er een waarde aan geven, en de waarde ervan opvragen. Om een variabele
een waarde te geven, biedt Logo de primitieve procedure MAAK, die twee
invoerwaarden nodig heeft: de naam van de variabele en de nieuwe waarde die
eraan toegekend moet worden. De eerste invoerwaarde, de naam van de
variabele, is gewoon een woord, en als (zoals bijna altijd het geval is) de
programmeur een waarde wil toekennen aan een specifieke variabele waarvan de
naam vooraf bekend is, wordt die invoerwaarde gequote, net als elk ander
bekend, specifiek woord:
	MAAK "MIJN.VAR EERSTE "WOORD
geeft de variabele met de naam MIJN.VAR de waarde W (de eerste letter van
WOORD).

Om de waarde van een variabele op te vragen, biedt Logo de primitieve
procedure DING, die een variabelenaam als invoer neemt en de waarde
teruggeeft van de toegankelijke variabele met die naam. Dus
	PRINTPERREGEL DING "MIJN.VAR
print W (uitgaande van de MAAK hierboven). Omdat het opvragen van de waarde
van een specifieke, bekende variabelenaam zo'n gangbare bewerking is, biedt
Logo ook een verkorte notatie die DING met een aanhalingsteken combineert:
	PRINTPERREGEL :MIJN.VAR
De dubbele punt (die Logo-veteranen "punten" noemen) vervangt DING en " uit
de eerdere versie van de instructie.

Nieuwkomers in Logo klagen vaak over al deze verplichte leestekens. Vooral
Logo-programmeurs die iets geleerd hebben over punten en aanhalingstekens
zonder ook iets over DING te leren, vragen zich af waarom een instructie
zoals
	MAAK "NIEUWE.VAR :OUDE.VAR
twee verschillende leestekens gebruikt om de twee variabelen aan te duiden.
(Na het lezen van de bovenstaande alinea's begrijp je dat eigenlijk beide
variabelenamen gequote zijn, maar dat de procedure DING wordt aangeroepen om
de waarde van OUDE.VAR op te vragen, omdat het die waarde is — niet de naam
OUDE.VAR — die MAAK nodig heeft. Het zou geen zin hebben om DING van
NIEUWE.VAR te vragen, aangezien we NIEUWE.VAR nog geen waarde hebben
gegeven.)

Hoewel Logo's leestekenregels logisch zijn zodra je ze begrijpt, vormen ze
wel een drempel voor de Logo-beginner. Waarom zou Logo dan niet zo ontworpen
kunnen zijn dat een woord zonder leesteken een procedure voorstelt als er
een procedure met die naam bestaat, of een variabele als er een variabele
met die naam bestaat? Dan zouden we kunnen zeggen
	PRINTPERREGEL MIJN.VAR
en zou Logo begrijpen dat MIJN.VAR de naam van een variabele is, niet van
een procedure. De traditionele reden om deze conventie niet te gebruiken is
dat Logo toestaat dat hetzelfde woord tegelijk een procedure én een
variabele benoemt. Dit is vooral belangrijk voor woorden die gegevenstypen
benoemen, zoals in de volgende procedure:
	LEER MEERVOUD :WOORD
	UITVOER WOORD :WOORD "S
	EIND
Hier is de naam WOORD een voor de hand liggende keuze voor de invoer van
MEERVOUD, omdat die het soort invoer beschrijft dat MEERVOUD verwacht.
Binnen de procedure gebruiken we WOORD voor Logo's primitieve procedure die
twee invoerwoorden combineert tot een nieuw, langer woord; we gebruiken
:WOORD voor de variabele die de invoer bevat, welk woord er ook daadwerkelijk
wordt meegegeven wanneer MEERVOUD wordt aangeroepen.
	? PRINTPERREGEL MEERVOUD "COMPUTER
	COMPUTERS

Als een Logo-instructie echter een ongequote woord bevat dat *niet* de naam
is van een procedure, zou Logo kunnen zoeken naar een variabele met die
naam. Dit zou een "leestekenloze" Logo mogelijk maken, ** OP VOORWAARDE DAT
GEBRUIKERS DIE ZONDER DUBBELE PUNTEN VOOR VARIABELEN WILLEN WERKEN,
VARIABELENAMEN KIEZEN DIE NIET OOK PROCEDURENAMEN ZIJN. **

Hoe zit het met het toekennen van een waarde aan een variabele? Zouden we
zonder het aanhalingsteken bij de eerste invoer van MAAK kunnen? Helaas
niet. Hoewel de eerste invoer van MAAK *meestal* een constante, bekende
variabelenaam is, is dat soms niet zo, zoals in dit voorbeeld:
	LEER OPHOGEN :VAR
	MAAK :VAR (DING :VAR)+1	; Let op: hier geen "VAR!
	EIND

	? MAAK "X 5
	? OPHOGEN "X
	? PRINTPERREGEL :X
	6
De procedure OPHOGEN neemt een variabelenaam als invoer en verandert de
waarde van die variabele. In dit voorbeeld zijn er twee variabelen: de
variabele met de naam VAR, met als waarde het woord X; en de variabele met
de naam X, waarvan de waarde verandert van 5 naar 6. Stel dat we het gedrag
van MAAK zouden veranderen zodat het woord na MAAK gebruikt wordt als de
naam van de te wijzigen variabele; dan zouden we OPHOGEN niet kunnen
schrijven:
	LEER OPHOGEN :VAR	; werkt niet!
	MAAK VAR (DING VAR)+1
	EIND
Dit zou een nieuwe waarde toekennen aan VAR, niet aan X.

Wat we wél kunnen doen, is een *alternatief* voor MAAK toestaan: een
"setter"-procedure voor een specifieke variabele. De notatie wordt dan
	? ZETFOO 7
	? PRINTPERREGEL FOO
	7
ZETFOO is een "setter-procedure" die één invoerwaarde neemt (in dit geval
de waarde 7) en die toekent aan de variabele met de naam FOO.

Berkeley Logo staat gebruikers toe om te kiezen voor óf de traditionele
notatie, waarbij dezelfde naam zowel voor een procedure als voor een
variabele gebruikt kan worden, óf de getter/setter-notatie, waarbij
variabele FOO wordt gezet met ZETFOO en opgevraagd met FOO, maar dezelfde
naam niet voor zowel procedure als variabele gebruikt kan worden.

Zo werkt die keuze: Berkeley Logo gebruikt standaard de traditionele
notatie, met procedures los van variabelen. Maar als er een variabele
bestaat met de naam AllowGetSet met als waarde WELWAAR (wat standaard het
geval is wanneer Logo opstart), dan probeert Logo, wanneer een
Logo-instructie verwijst naar een *niet-bestaande* procedure (zodat
normaliter de foutmelding "Ik weet niet hoe ik ... moet uitvoeren" zou
volgen), de volgende twee stappen:

	1.  Als de naam minstens vier tekens lang is, en de eerste drie
	tekens zijn de letters ZET (hoofd- of kleine letters), en als de
	naam in de instructie gevolgd wordt door nog een waarde, en als de
	naam zonder de ZET de naam is van een al bestaande variabele, dan
	roept Logo MAAK aan met als eerste invoer de naam zonder de ZET, en
	als tweede invoer de daaropvolgende waarde.

	2.  Als niet aan de voorwaarden van stap 1 is voldaan, maar de naam
	is wel de naam van een toegankelijke variabele, dan roept Logo DING
	aan met die naam als invoer, om de waarde van de variabele op te
	vragen.

Stap 1 vereist dat de variabele al bestaat, zodat schrijffouten in namen
van ZETxxx-primitieven (bv. ZETRICHTING) nog steeds worden opgemerkt, in
plaats van stilzwijgend een nieuwe variabele aan te maken. Het commando
GLOBAL kan gebruikt worden om een variabele aan te maken zonder er een
waarde aan te geven.

Nog één laatste punt: het commando LEER is in Logo altijd al een speciaal
geval geweest; de rest van de regel die met LEER begint wordt niet
geëvalueerd als gewone Logo-expressies. In het bijzonder veroorzaken de
dubbele punten die de namen van de invoerwaarden van de procedure markeren
geen aanroep van DING. Het zijn slechts geheugensteuntjes, die de
Logo-gebruiker eraan herinneren dat deze woorden namen van variabelen zijn.
(Je zou kunnen zeggen dat dit afwijkende gedrag van LEER bijdraagt aan de
verwarring van Logo-beginners over dubbele punten.) Voor een programmeur
die variabelen zonder dubbele punt gebruikt, zijn de dubbele punten op de
LEER-regel overbodig en betekenisloos. Berkeley Logo maakt de dubbele
punten daarom optioneel:
	LEER FOO :IN1 :IN2
en
	LEER FOO IN1 IN2
zijn allebei toegestaan.


LOGO STARTEN EN VERLATEN
=========================

Het proces om Logo te starten hangt af van je besturingssysteem:

Unix:	 Typ het woord {\tt logo} in de shell.  (De map waarin je Logo hebt
	 geïnstalleerd moet in je pad staan.)

DOS:	 Ga naar de map met Logo (waarschijnlijk C:\UCBLOGO).  Typ dan
	 UCBLOGO voor de grote-geheugenversie, of BL voor de 640K-versie.

Mac:	 Dubbelklik op het LOGO-icoon in de map "UCB Logo".

Windows: Dubbelklik op het UCBWLOGO-icoon in de map UCBLOGO.

Om Logo te verlaten, typ je het commando "totziens" (bye).

Bij het opstarten zoekt Logo naar een bestand met de naam "startup.lg" in de
systeem-Logo-bibliotheek en laadt dat, indien gevonden. Daarna zoekt het naar
"startup.lg" in de home-map van de gebruiker, of in de huidige map, afhankelijk
van het besturingssysteem, en laadt dat bestand. Deze opstartbestanden kunnen
gebruikt worden om procedures vooraf te definiëren, bv. om niet-Engelse namen
voor primitieve procedures aan te bieden.

Onder Unix, DOS of Windows: als je één of meer bestandsnamen meegeeft op de
commandoregel bij het starten van Logo, worden die bestanden geladen voordat
de interpreter begint met het lezen van commando's van je terminal. Als je
een bestand laadt dat een programma uitvoert met daarin een "totziens"-
commando, dan draait Logo dat programma en sluit af. Je kan dus zelfstandige
programma's in Logo schrijven en ze draaien met shell-/batchscripts. Om dit
te ondersteunen print Logo zijn gebruikelijke welkomst- en afscheidsberichten
niet als je bestandsargumenten meegeeft aan het logo-commando.

Als een commandoregelargument gewoon een liggend streepje is, dan worden
alle commandoregelargumenten na dat streepje niet als bestandsnamen
opgevat, maar verzameld in een lijst, één woord per argument; de begraven
variabele COMMAND.LINE bevat die lijst met argumenten, of de lege lijst als
die er niet zijn. Op mijn Linux-systeem, als de eerste regel van een
uitvoerbaar shellscript
#!/usr/local/bin/logo -
is (let op het streepje), dan kan het script commandoregelargumenten
meekrijgen en komen die allemaal terecht in :COMMAND.LINE, samen met het pad
van het script. Experimenteer ermee.

Als je je onderbrekingsteken typt (zie onderstaande tabel) stopt Logo
waarmee het bezig is en keert het terug naar het topniveau, alsof je GOOI
"TOPLEVEL had gedaan. Als je je pauzeteken typt, pauzeert Logo alsof je
PAUZE had gedaan.

               wxWidgets       Unix        DOS/Windows          Mac

topniveau        alt-S    meestal ctrl-C     ctrl-Q      command-. (punt)

pauze            alt-P    meestal ctrl-\     ctrl-W      command-, (komma)

Als je een omgevingsvariabele hebt met de naam LOGOLIB waarvan de waarde de
naam van een map is, dan gebruikt Logo die map in plaats van de standaard-
bibliotheek. Als je een procedure aanroept die niet gedefinieerd is, zoekt
Logo eerst naar een bestand in de huidige map met de naam proc.lg, waarbij
"proc" de procedurenaam in kleine letters is. Als zo'n bestand bestaat, laadt
Logo dat bestand. Als de ontbrekende procedure dan nog steeds niet
gedefinieerd is, of als zo'n bestand niet bestaat, zoekt Logo vervolgens in
de bibliotheekmap naar een bestand met de naam proc (zonder ".lg") en laadt
dat, als het bestaat. Als geen van beide bestanden een definitie voor de
procedure bevat, geeft Logo een foutmelding. Verscheidene procedures die in
de meeste Logo-versies primitief zijn, zitten in de standaardbibliotheek; als
je een andere bibliotheek gebruikt, wil je misschien een deel van of de hele
standaardbibliotheek erin opnemen.


TOKENISATIE
=============

Namen van procedures, variabelen en kenmerklijsten zijn niet
hoofdlettergevoelig. Dat geldt ook voor de speciale woorden EIND, WELWAAR en
NIETWAAR. Het gebruik van hoofd-/kleine letters in alles wat je typt blijft
echter wel behouden.

Binnen vierkante haken worden woorden alleen begrensd door spaties en
vierkante haken. [2+3] is een lijst met één woord erin. Let echter op dat de
Logo-primitieven die zo'n lijst interpreteren als een Logo-instructie of
-expressie (DOE, ALS, etc.) de lijst herparsen alsof die niet binnen haken
was getypt.

Na een aanhalingsteken buiten vierkante haken wordt een woord begrensd door
een spatie, een vierkante haak, of een haakje.

Een woord dat niet na een aanhalingsteken staat of binnen vierkante haken,
wordt begrensd door een spatie, een haak, een haakje, of een infix-operator
+-*/=<>. Merk op dat woorden die op een dubbele punt volgen in deze categorie
vallen. Merk op dat aanhalingsteken en dubbele punt geen begrenzers zijn.
Elk infix-operatorteken is op zichzelf een woord, behalve dat de
tweetekenreeksen <= >= en <> (die laatste betekent ongelijk-aan) zonder
tussenliggende spatie als één woord herkend worden.

Een woord dat bestaat uit een vraagteken gevolgd door een getal (bv. ?37)
wordt, wanneer het geherparset wordt (d.w.z. op een plek waar een
procedurenaam verwacht wordt), behandeld alsof het de reeks

	( ? 37 )

was, waardoor het getal een invoerwaarde wordt voor de ?-procedure. (Zie de
bespreking van sjablonen, verderop.) Deze speciale behandeling geldt niet
voor woorden die als gegevens gelezen worden, voor woorden waarbij na het
vraagteken geen getal volgt, of als het vraagteken met een backslash is
ontsnapt.

Een regel (een instructieregel of een regel gelezen door LEESLIJST of
LEESWOORD) kan doorlopen op de volgende regel als het laatste teken een
tilde (~) is. LEESWOORD behoudt de tilde en de regeleinde-markering;
LEESLIJST doet dat niet.

Regels gelezen met READRAWLINE lopen nooit door.

Een instructieregel of een regel gelezen door LEESLIJST (maar niet door
LEESWOORD) loopt automatisch door naar de volgende regel, alsof er een
tilde aan het eind stond, als er nog haakjes, accolades of verticale
strepen openstaan. Het is echter een fout als de vervolgregel alleen het
woord EIND bevat; dit is om oneindig doorlopende procedure-definities te
voorkomen. Regels die expliciet met een tilde worden vervolgd ontsnappen
aan deze beperking.

Als een regel die interactief op het toetsenbord wordt getypt doorloopt,
hetzij met een tilde, hetzij automatisch, dan toont Logo een tilde als
promptteken voor de vervolgregel.

Een puntkomma begint een commentaar in een instructieregel. Logo negeert
alle tekens vanaf de puntkomma tot het einde van de regel. Een tilde als
laatste teken duidt nog steeds een vervolgregel aan, maar geen voortzetting
van het commentaar. Bijvoorbeeld, als je de instructie

	printperregel "abc;commentaar ~
	def

typt, dan wordt het woord abcdef geprint. Een puntkomma heeft geen speciale
betekenis in gegevensregels die gelezen worden door LEESWOORD of LEESLIJST,
maar zo'n regel kan later opnieuw geparset worden met RUNPARSE, en dan
worden commentaren wel herkend.

De tweetekenreeks #! aan het begin van een regel begint ook een commentaar.
Unix-gebruikers kunnen daarom een bestand met Logo-commando's schrijven dat
begint met de regel

	#! /usr/local/bin/logo

(of waar je Logo-programma zich ook bevindt), en het bestand wordt dan
direct vanuit de shell uitvoerbaar.

Om een teken dat anders als begrenzer zou werken (inclusief puntkomma of
tilde) toch in een woord op te nemen, zet je er een backslash (\) voor. Als
het laatste teken van een regel een backslash is, dan wordt het
regeleinde-teken dat op de backslash volgt onderdeel van het laatste woord
op de regel, en loopt de regel door op de volgende regel. Om een backslash
in een woord op te nemen, gebruik je \\. Als de combinatie backslash-
regeleinde aan de terminal wordt ingevoerd, toont Logo een backslash als
promptteken voor de vervolgregel. Dit alles geldt zowel voor gegevensregels
gelezen met LEESWOORD of LEESLIJST als voor instructieregels.

Een regel gelezen met READRAWLINE heeft geen speciaal ontsnappingsmechanisme;
zowel backslash als verticale streep (hieronder beschreven) zijn daar
gewone tekens.

Een alternatieve notatie om anders begrenzende tekens in woorden op te
nemen, is door een groep tekens tussen verticale strepen te zetten. Alle
tekens tussen verticale strepen worden behandeld alsof het letters zijn. In
gegevens gelezen met LEESWOORD blijven de verticale strepen behouden in het
resulterende woord. In gegevens gelezen met LEESLIJST (of die het resultaat
zijn van een PARSE of RUNPARSE van een woord) verschijnen de verticale
strepen niet expliciet; alle mogelijk begrenzende tekens (inclusief
spaties, haken, haakjes en infix-operatoren) verschijnen ongemarkeerd, maar
worden getokeniseerd alsof het letters waren. Binnen verticale strepen kan
nog steeds een backslash gebruikt worden; de enige tekens die in deze
context met een backslash ontsnapt moeten worden, zijn backslash en de
verticale streep zelf.

Tekens die tussen verticale strepen zijn ingevoerd, blijven voor altijd
speciaal, zelfs als het woord of de lijst waarin ze staan later opnieuw
geparset wordt met PARSE of RUNPARSE. Tekens die na een backslash getypt
zijn, worden enigszins anders behandeld: wanneer een gequote woord met een
ge-backslashte teken geherparset wordt, verliest dat teken zijn speciale
karakter en gedraagt het zich daarna alsof het normaal getypt was. Dit
onderscheid is alleen van belang als je een Logo-expressie opbouwt uit
losse delen, om later te DOEn, en je haakjes wil gebruiken. Bijvoorbeeld,

	PRINTPERREGEL DOE (SA "\( 2 "+ 3 "\))

print 5, maar

	DOE (SA "MAAK ""|(| 2)

maakt een variabele aan met als naam open-haakje. (Beide voorbeelden zouden
mislukken als verticale strepen en backslashes verwisseld werden.)

Een teken dat met een backslash is ingevoerd is GELIJK? aan hetzelfde
teken zonder backslash, maar kan onderscheiden worden met het VBARRED?-
predicaat. (VBARRED? geeft echter alleen WELWAAR voor tekens waarvoor
speciale behandeling nodig is: witruimte, haakjes, haken, infix-operatoren,
backslash, verticale streep, tilde, aanhalingsteken, vraagteken, dubbele
punt en puntkomma.)


GEGEVENSSTRUCTUUR-PRIMITIEVEN
================================

CONSTRUCTORS
------------

WOORD woord1 woord2
(WOORD woord1 woord2 woord3 ...)

	geeft een woord terug dat gevormd wordt door de invoerwaarden aan
	elkaar te plakken.

LIJST ding1 ding2
(LIJST ding1 ding2 ding3 ...)

	geeft een lijst terug waarvan de elementen de invoerwaarden zijn, die
	elk gegevenstype in Logo mogen zijn (woord, lijst of array).

SAMEN ding1 ding2
SA ding1 ding2
(SAMEN ding1 ding2 ding3 ...)
(SA ding1 ding2 ding3 ...)

	geeft een lijst terug waarvan de elementen de invoerwaarden zijn, als
	die geen lijsten zijn, of de elementen van de invoerwaarden, als die
	wel lijsten zijn.

PLAATSERVOOR ding lijst

	geeft een lijst terug gelijk aan de tweede invoerwaarde met één extra
	element, de eerste invoerwaarde, aan het begin. Als de tweede
	invoerwaarde een woord is, moet de eerste invoerwaarde een woord van
	één letter zijn, en is PLAATSERVOOR gelijk aan WOORD.

PLAATSERACHTER ding lijst

	geeft een lijst terug gelijk aan de tweede invoerwaarde met één extra
	element, de eerste invoerwaarde, aan het eind. Als de tweede
	invoerwaarde een woord is, moet de eerste invoerwaarde een woord van
	één letter zijn, en is PLAATSERACHTER gelijk aan WOORD met de
	invoerwaarden in omgekeerde volgorde.

ARRAY grootte
(ARRAY grootte oorsprong)

	geeft een array terug met "grootte" elementen (moet een positief
	geheel getal zijn), die elk in eerste instantie een lege lijst zijn.
	Array-elementen kunnen geselecteerd worden met ITEM en gewijzigd met
	SETITEM. Het eerste element van de array is element nummer 1, tenzij
	een invoerwaarde "oorsprong" (moet een geheel getal zijn) is
	meegegeven; in dat geval heeft het eerste element van de array dat
	getal als index. (Meestal wordt 0 als oorsprong gebruikt, als er al
	iets gebruikt wordt.) Arrays worden geprint door PRINTPERREGEL en
	verwanten, en kunnen ingetypt worden binnen accolades; geef een
	oorsprong aan met {a b c}@0.

MDARRAY groottelijst					(bibliotheekprocedure)
(MDARRAY groottelijst oorsprong)

	geeft een meerdimensionale array terug. De eerste invoerwaarde moet
	een lijst zijn van één of meer positieve gehele getallen. De tweede
	invoerwaarde, indien aanwezig, moet een los geheel getal zijn dat op
	elke dimensie van de array van toepassing is. Bv.: (MDARRAY [3 5] 0)
	geeft een tweedimensionale array terug waarvan de elementen lopen
	van [0 0] tot [2 4].

LISTTOARRAY lijst
(LISTTOARRAY lijst oorsprong)

	geeft een array terug van dezelfde grootte als de invoerlijst,
	waarvan de elementen de elementen van de invoerlijst zijn.

ARRAYTOLIST array

	geeft een lijst terug waarvan de elementen de elementen van de
	invoerarray zijn. Het eerste element van de uitvoer is het eerste
	element van de array, ongeacht de oorsprong van de array.

COMBINE ding1 ding2					(bibliotheekprocedure)

	als ding2 een woord is, geeft dit WOORD ding1 ding2 terug. Als ding2
	een lijst is, geeft dit PLAATSERVOOR ding1 ding2 terug.

REVERSE lijst						(bibliotheekprocedure)

	geeft een lijst terug waarvan de elementen de elementen van de
	invoerlijst zijn, in omgekeerde volgorde.

GENSYM							(bibliotheekprocedure)

	geeft elke keer dat het aangeroepen wordt een uniek woord terug. De
	woorden hebben de vorm G1, G2, enzovoort.


SELECTORS
---------

EERSTE ding

	als de invoerwaarde een woord is, geeft dit het eerste teken van het
	woord terug. Als de invoerwaarde een lijst is, geeft dit het eerste
	element van de lijst terug. Als de invoerwaarde een array is, geeft
	dit de oorsprong van de array terug (dat wil zeggen, de INDEX VAN
	het eerste element van de array).

FIRSTS lijst

	geeft een lijst terug met de EERSTE van elk element van de
	invoerlijst. Het is een fout als een element van de invoerlijst leeg
	is. (De invoerwaarde zelf mag leeg zijn; dan is de uitvoer ook leeg.)
	Dit zou geschreven kunnen worden als

		leer firsts :lijst
		uitvoer map "eerste :lijst
		eind

	maar wordt als primitief aangeboden om de iteratie-hulpmiddelen MAP,
	MAP.SE en FOREACH te versnellen.

		leer transpose :matrix
		als leeg? eerste :matrix [uv []]
		uv plaatsvoor firsts :matrix transpose bfs :matrix
		eind

LAATSTE woordoflijst

	als de invoerwaarde een woord is, geeft dit het laatste teken van
	het woord terug. Als de invoerwaarde een lijst is, geeft dit het
	laatste element van de lijst terug.

MINEERSTE woordoflijst
ME woordoflijst

	als de invoerwaarde een woord is, geeft dit een woord terug met alle
	tekens behalve het eerste. Als de invoerwaarde een lijst is, geeft
	dit een lijst terug met alle elementen behalve het eerste.

BUTFIRSTS lijst
BFS lijst

	geeft een lijst terug met de MINEERSTE van elk element van de
	invoerlijst. Het is een fout als een element van de invoerlijst leeg
	is of een array is. (De invoerwaarde zelf mag leeg zijn; dan is de
	uitvoer ook leeg.) Dit zou geschreven kunnen worden als

		leer butfirsts :lijst
		uitvoer map "mineerste :lijst
		eind

	maar wordt als primitief aangeboden om de iteratie-hulpmiddelen MAP,
	MAP.SE en FOREACH te versnellen.

MINLAATSTE woordoflijst
ML woordoflijst

	als de invoerwaarde een woord is, geeft dit een woord terug met alle
	tekens behalve het laatste. Als de invoerwaarde een lijst is, geeft
	dit een lijst terug met alle elementen behalve het laatste.

ELEMENT index ding

	als "ding" een woord is, geeft dit het index-de teken van het woord
	terug. Als "ding" een lijst is, geeft dit het index-de element van de
	lijst terug. Als "ding" een array is, geeft dit het index-de element
	van de array terug. "Index" begint bij 1 voor woorden en lijsten; de
	startindex van een array wordt opgegeven bij het aanmaken van de
	array.

MDITEM indexlijst array				(bibliotheekprocedure)

	geeft het element van de meerdimensionale "array" terug dat
	geselecteerd wordt door de lijst met getallen "indexlijst".

PICK lijst						(bibliotheekprocedure)

	geeft een willekeurig gekozen element van de invoerlijst terug.

REMOVE ding lijst					(bibliotheekprocedure)

	geeft een kopie van "lijst" terug waaruit elk element gelijk aan
	"ding" verwijderd is.

REMDUP lijst						(bibliotheekprocedure)

	geeft een kopie van "lijst" terug waaruit dubbele elementen
	verwijderd zijn. Als twee of meer elementen van de invoerwaarde
	gelijk zijn, is het meest rechtse van die elementen degene die in de
	uitvoer overblijft.

QUOTED ding						(bibliotheekprocedure)

	geeft zijn invoerwaarde terug als die een lijst is; geeft zijn
	invoerwaarde terug met een aanhalingsteken ervoor als die een woord
	is.


MUTATORS
--------

SETITEM index array waarde

	commando. Vervangt het index-de element van "array" door de nieuwe
	"waarde". Zorgt ervoor dat de resulterende array niet circulair is,
	d.w.z. "waarde" mag geen lijst of array zijn die "array" bevat.

MDSETITEM indexlijst array waarde			(bibliotheekprocedure)

	commando. Vervangt het element van "array" dat gekozen wordt door
	"indexlijst" door de nieuwe "waarde".

.SETFIRST lijst waarde

	commando. Verandert het eerste element van "lijst" in "waarde".

	WAARSCHUWING: primitieven waarvan de naam met een punt begint zijn
	GEVAARLIJK. Gebruik door niet-experts wordt afgeraden. Het gebruik
	van .SETFIRST kan leiden tot circulaire lijststructuren, waardoor
	sommige Logo-primitieven in oneindige lussen terechtkomen, en tot
	onverwachte veranderingen in andere gegevensstructuren die opslag
	delen met de lijst die gewijzigd wordt.

.SETBF lijst waarde

	commando. Verandert de mineerste van "lijst" in "waarde".

	WAARSCHUWING: primitieven waarvan de naam met een punt begint zijn
	GEVAARLIJK. Gebruik door niet-experts wordt afgeraden. Het gebruik
	van .SETBF kan leiden tot circulaire lijststructuren, waardoor
	sommige Logo-primitieven in oneindige lussen terechtkomen; tot
	onverwachte veranderingen in andere gegevensstructuren die opslag
	delen met de lijst die gewijzigd wordt; of tot Logo-crashes en
	coredumps als de mineerste van een lijst zelf geen lijst is.

.SETITEM index array waarde

	commando. Verandert het index-de element van "array" in "waarde",
	net als SETITEM, maar zonder op circulariteit te controleren.

	WAARSCHUWING: primitieven waarvan de naam met een punt begint zijn
	GEVAARLIJK. Gebruik door niet-experts wordt afgeraden. Het gebruik
	van .SETITEM kan leiden tot circulaire arrays, waardoor sommige
	Logo-primitieven in oneindige lussen terechtkomen.

PUSH stapelnaam ding					(bibliotheekprocedure)

	commando. Voegt "ding" toe aan de stapel die de waarde is van de
	variabele met de naam "stapelnaam". Deze variabele moet een lijst
	als waarde hebben; de beginwaarde moet de lege lijst zijn. Nieuwe
	elementen worden vooraan de lijst toegevoegd.

POP stapelnaam						(bibliotheekprocedure)

	geeft het meest recent met PUSH toegevoegde element van de stapel
	terug die de waarde is van de variabele met de naam "stapelnaam", en
	verwijdert dat element van de stapel.

QUEUE wachtrijnaam ding				(bibliotheekprocedure)

	commando. Voegt "ding" toe aan de wachtrij die de waarde is van de
	variabele met de naam "wachtrijnaam". Deze variabele moet een lijst
	als waarde hebben; de beginwaarde moet de lege lijst zijn. Nieuwe
	elementen worden achteraan de lijst toegevoegd.

DEQUEUE wachtrijnaam					(bibliotheekprocedure)

	geeft het langst wachtende, met QUEUE toegevoegde element van de
	wachtrij terug die de waarde is van de variabele met de naam
	"wachtrijnaam", en verwijdert dat element uit de wachtrij.


PREDICATES
----------

WOORD? ding

	geeft WELWAAR terug als de invoerwaarde een woord is, anders
	NIETWAAR.

LIJST? ding

	geeft WELWAAR terug als de invoerwaarde een lijst is, anders
	NIETWAAR.

ARRAY? ding

	geeft WELWAAR terug als de invoerwaarde een array is, anders
	NIETWAAR.

LEEG? ding

	geeft WELWAAR terug als de invoerwaarde het lege woord of de lege
	lijst is, anders NIETWAAR.

GELIJK? ding1 ding2
ding1 = ding2

	geeft WELWAAR terug als de invoerwaarden gelijk zijn, anders
	NIETWAAR. Twee getallen zijn gelijk als ze dezelfde numerieke waarde
	hebben. Twee niet-numerieke woorden zijn gelijk als ze dezelfde
	tekens in dezelfde volgorde bevatten. Als er een variabele bestaat
	met de naam CASEIGNOREDP waarvan de waarde WELWAAR is, dan wordt een
	hoofdletter als hetzelfde beschouwd als de bijbehorende kleine
	letter. (Dit is standaard het geval.) Twee lijsten zijn gelijk als
	hun elementen gelijk zijn. Een array is alleen gelijk aan zichzelf;
	twee apart aangemaakte arrays zijn nooit gelijk, ook al zijn hun
	elementen gelijk. (Het is belangrijk om te kunnen weten of twee
	expressies dezelfde array als waarde hebben, omdat arrays muteerbaar
	zijn; als bijvoorbeeld twee variabelen dezelfde array als waarde
	hebben, dan verandert SETITEM op de één ook de ander.)

NOTEQUAL? ding1 ding2
ding1 <> ding2

	geeft NIETWAAR terug als de invoerwaarden gelijk zijn, anders
	WELWAAR. Zie GELIJK? voor de betekenis van gelijkheid voor
	verschillende gegevenstypen.

BEFORE? woord1 woord2

	geeft WELWAAR terug als woord1 in de ASCII-sorteervolgorde vóór
	woord2 komt (voor woorden van letters, in alfabetische volgorde).
	Hoofdlettergevoeligheid wordt bepaald door de waarde van
	CASEIGNOREDP. Merk op dat als de invoerwaarden getallen zijn, het
	resultaat kan afwijken van dat van LESSP; bijvoorbeeld, BEFORE? 3 12
	is onwaar, omdat 3 na 1 in de sorteervolgorde komt.

.EQ ding1 ding2

	geeft WELWAAR terug als de twee invoerwaarden hetzelfde gegeven zijn,
	zodat het toepassen van een mutator op de één ook de ander
	verandert. Geeft anders NIETWAAR terug, zelfs als de invoerwaarden
	in waarde gelijk zijn.
	WAARSCHUWING: primitieven waarvan de naam met een punt begint zijn
	GEVAARLIJK. Gebruik door niet-experts wordt afgeraden. Het gebruik
	van mutators kan leiden tot circulaire gegevensstructuren, oneindige
	lussen, of Logo-crashes.

ERBIJ? ding1 ding2

	als "ding2" een lijst of een array is, geeft dit WELWAAR terug als
	"ding1" GELIJK? is aan een element van "ding2", anders NIETWAAR. Als
	"ding2" een woord is, geeft dit WELWAAR terug als "ding1" een woord
	van één teken is dat GELIJK? is aan een teken van "ding2", anders
	NIETWAAR.

SUBSTRING? ding1 ding2

	als "ding1" of "ding2" een lijst of een array is, geeft dit NIETWAAR
	terug. Als "ding2" een woord is, geeft dit WELWAAR terug als "ding1"
	GELIJK? is aan een deelreeks van "ding2", anders NIETWAAR.

GETAL? ding

	geeft WELWAAR terug als de invoerwaarde een getal is, anders
	NIETWAAR.

VBARRED? teken
BACKSLASHEDP teken                              (bibliotheekprocedure)
BACKSLASHED? teken                              (bibliotheekprocedure)

	geeft WELWAAR terug als het invoerteken oorspronkelijk in Logo werd
	ingevoerd tussen verticale strepen (|) om de gebruikelijke speciale
	syntactische betekenis ervan te voorkomen, anders NIETWAAR. (Geeft
	alleen WELWAAR terug als het teken een ge-backslashte spatie, tab,
	regeleinde, of een van ()[]+-*/=<>":;\~?| is.)

	De namen BACKSLASHEDP en BACKSLASHED? zitten in de Logo-bibliotheek
	voor achterwaartse compatibiliteit met de vroegere namen van dit
	primitief, hoewel het *niet* WELWAAR teruggeeft voor tekens die
	oorspronkelijk met backslashes zijn ingevoerd.


QUERIES
-------

AANTAL ding

	geeft het aantal tekens in de invoerwaarde terug, als de
	invoerwaarde een woord is; geeft het aantal elementen in de
	invoerwaarde terug, als het een lijst of een array is. (Bij een
	array is dit niet per se de index van het laatste element, dat hangt
	af van de oorsprong van de array.)

ASCII teken

	geeft het gehele getal (tussen 0 en 255) terug dat het invoerteken
	voorstelt in de ASCII-code. Interpreteert stuurcodes als
	vbarred-leestekens, en geeft de tekencode terug voor het
	bijbehorende leesteken zonder verticale strepen. (Vergelijk
	RAWASCII.)

RAWASCII teken

	geeft het gehele getal (tussen 0 en 255) terug dat het invoerteken
	voorstelt in de ASCII-code. Interpreteert stuurcodes als zichzelf.
	Om de ASCII-code van een willekeurige toetsaanslag te achterhalen,
	gebruik je RAWASCII RC.

TEKEN geheelgetal

	geeft het teken terug dat in de ASCII-code voorgesteld wordt door de
	invoerwaarde, die een geheel getal tussen 0 en 255 moet zijn.

MEMBER ding1 ding2

	als "ding2" een woord of lijst is en ERBIJ? met deze invoerwaarden
	WELWAAR zou teruggeven, geeft dit het deel van "ding2" terug vanaf
	het eerste voorkomen van "ding1" tot het einde. Als ERBIJ? NIETWAAR
	zou teruggeven, geeft dit het lege woord of de lege lijst terug,
	afhankelijk van het type van "ding2". Het is een fout als "ding2"
	een array is.

LOWERCASE woord

	geeft een kopie van het invoerwoord terug, maar met alle
	hoofdletters veranderd in de bijbehorende kleine letter.

UPPERCASE woord

	geeft een kopie van het invoerwoord terug, maar met alle kleine
	letters veranderd in de bijbehorende hoofdletter.

STANDOUT ding

	geeft een woord terug dat er bij het printen uitziet als de
	invoerwaarde, maar weergegeven in standout-modus (vetgedrukt, video
	omgekeerd, of wat jouw versie ook doet voor standout). Het woord
	bevat machine-specifieke magische tekens aan het begin en einde;
	daartussen staat de geprinte vorm (alsof weergegeven met TYPE) van de
	invoerwaarde. De uitvoer is altijd een woord, zelfs als de
	invoerwaarde van een ander type is, maar het mag spaties en andere
	opmaaktekens bevatten. Let op: een woord dat door STANDOUT wordt
	teruggegeven terwijl Logo op de ene machine draait, zal
	waarschijnlijk niet het gewenste effect hebben als het op een ander
	type machine geprint wordt.

	In de klassieke Macintosh-versie is de manier waarop standout werkt
	niet te combineren met het gebruik van tekens waarvan de ASCII-code
	groter is dan 127. Je hebt daarom een keuze te maken: de instructie
		CANINVERSE 0
	schakelt standout uit, maar schakelt de weergave van ASCII-codes
	boven 127 in, en de instructie
		CANINVERSE 1
	herstelt de standaardsituatie waarin standout is ingeschakeld en de
	extra grafische tekens niet geprint kunnen worden.

PARSE woord

	geeft de lijst terug die zou ontstaan als het invoerwoord was
	ingevoerd als antwoord op een LEESLIJST-bewerking. Dat wil zeggen,
	PARSE LEESWOORD heeft dezelfde waarde als LEESLIJST voor dezelfde
	gelezen tekens.

RUNPARSE woordoflijst

	geeft de lijst terug die zou ontstaan als het invoerwoord of de
	invoerlijst was ingevoerd als instructieregel; tekens zoals
	infix-operatoren en haakjes zijn afzonderlijke elementen van de
	uitvoer. Merk op dat sublijsten van een geruntparste lijst niet zelf
	geruntparst worden.


COMMUNICATIE
=============

TRANSMITTERS
------------

Let op: als er een variabele bestaat met de naam PRINTDEPTHLIMIT met een
niet-negatieve gehele waarde, dan worden complexe lijst- en
arraystructuren alleen tot de toegestane diepte geprint. Dat wil zeggen,
elementen van elementen van... van elementen worden maar tot zover
toegestaan. De elementen die weggelaten worden omdat ze net voorbij de
dieptelimiet liggen, worden elk aangegeven met puntjes, dus een te diepe
lijst met twee elementen print als [... ...].

Als er een variabele bestaat met de naam PRINTWIDTHLIMIT met een
niet-negatieve gehele waarde, dan worden alleen de eerste zoveel elementen
van een array of lijst geprint. Eén reeks puntjes vervangt alle
ontbrekende gegevens binnen de structuur. De breedtelimiet geldt ook voor
het aantal geprinte tekens in een woord, behalve dat een PRINTWIDTHLIMIT
tussen 0 en 9 behandeld wordt alsof het 10 was toen toegepast op woorden.
Deze limiet geldt niet alleen voor het geprinte gegeven op het hoogste
niveau, maar voor elke substructuur erin.

Als er een variabele bestaat met de naam FULLPRINTP waarvan de waarde
WELWAAR is, dan worden woorden die gemaakt zijn met een backslash of
verticale streep (om tekens op te nemen die anders niet als deel van een
woord behandeld zouden worden) geprint mét de backslashes of verticale
strepen zichtbaar, zodat het geprinte resultaat door Logo teruggelezen kan
worden om dezelfde waarde te produceren. Als FULLPRINTP WELWAAR is, print
het lege woord (hoe het ook gemaakt is) als ||. (Anders print het als
helemaal niets.)

PRINTPERREGEL ding
PR ding
(PRINTPERREGEL ding1 ding2 ...)
(PR ding1 ding2 ...)

	commando. Print de invoerwaarde(n) naar de huidige schrijfstroom
	(aanvankelijk het scherm). Alle invoerwaarden worden op één regel
	geprint, gescheiden door spaties, eindigend met een regeleinde. Als
	een invoerwaarde een lijst is, worden er geen vierkante haken
	omheen geprint, maar rond sublijsten wel. Rond arrays worden altijd
	accolades geprint.

PRINTNAAROV ding
(PRINTNAAROV ding1 ding2 ...)

	commando. Print de invoerwaarde(n) zoals PRINTPERREGEL, behalve dat
	er geen regeleinde-teken aan het eind geprint wordt en meerdere
	invoerwaarden niet door spaties gescheiden worden. Let op: printen
	naar de terminal is normaliter "regel-gebufferd"; dat wil zeggen, de
	tekens die je print met PRINTNAAROV verschijnen pas echt op het
	scherm zodra er óf een regeleinde-teken geprint wordt (bijvoorbeeld
	door PRINTPERREGEL of LAATZIEN), óf Logo probeert te lezen van het
	toetsenbord (op verzoek van je programma, of na een instructie-
	prompt). Deze buffering maakt het programma veel sneller dan wanneer
	elk teken meteen zou verschijnen, en in de meeste gevallen is het
	effect niet storend. Om programma's die veel gepositioneerde tekst
	tonen met PRINTNAAROV tegemoet te komen, dwingt Logo het printen af
	wanneer CURSOR of SETCURSOR wordt aangeroepen. Dit lost de meeste
	bufferingsproblemen op. Toch kan het soms nodig zijn de gebufferde
	tekens expliciet af te dwingen te printen; dit kan met het
	WACHT-commando. WACHT 0 dwingt het printen af zonder daadwerkelijk
	te wachten.

LAATZIEN ding
(LAATZIEN ding1 ding2 ...)

	commando. Print de invoerwaarde(n) zoals PRINTPERREGEL, behalve dat
	als een invoerwaarde een lijst is, die tussen vierkante haken
	geprint wordt.


RECEIVERS
---------

LEESLIJST
LL
RL

	leest een regel van de leesstroom (aanvankelijk het toetsenbord) en
	geeft die regel terug als een lijst. De regel wordt opgesplitst in
	elementen alsof die tussen vierkante haken in een instructie was
	getypt. Als de leesstroom een bestand is en het einde van het
	bestand bereikt wordt, geeft LEESLIJST het lege woord terug (niet de
	lege lijst). LEESLIJST verwerkt backslash-, verticale-streep- en
	tildetekens in de leesstroom; de uitvoerlijst bevat deze tekens niet,
	maar ze hebben wel hun gebruikelijke effect gehad. LEESLIJST
	behandelt een puntkomma echter niet als commentaarteken.

LEESWOORD
LW
RW

	leest een regel van de leesstroom en geeft die regel terug als een
	woord. De uitvoer is een los woord, zelfs als de regel spaties,
	haken, etc. bevat. Als de leesstroom een bestand is en het einde van
	het bestand bereikt wordt, geeft LEESWOORD de lege lijst terug (niet
	het lege woord). LEESWOORD verwerkt backslash-, verticale-streep- en
	tildetekens in de leesstroom. Bij een tilde die gebruikt wordt voor
	regelvoortzetting bevat het uitvoerwoord WEL de tilde en de
	regeleinde-tekens, zodat het gebruikersprogramma precies kan zien wat
	de gebruiker heeft ingevoerd. Verticale strepen in de regel blijven
	ook behouden in de uitvoer. Backslash-tekens blijven niet behouden
	in de uitvoer.

READRAWLINE

	leest een regel van de leesstroom en geeft die regel terug als een
	woord. De uitvoer is een los woord, zelfs als de regel spaties,
	haken, etc. bevat. Als de leesstroom een bestand is en het einde van
	het bestand bereikt wordt, geeft READRAWLINE de lege lijst terug
	(niet het lege woord). READRAWLINE geeft precies de reeks tekens
	terug zoals die in de regel voorkomen, zonder speciale betekenis
	voor backslash, verticale streep, tilde, of enig ander opmaakteken.

LEESTEKEN
RC

	leest één teken van de leesstroom en geeft dat teken terug als een
	woord. Als de leesstroom een bestand is en het einde van het bestand
	bereikt wordt, geeft LEESTEKEN de lege lijst terug (niet het lege
	woord). Als de leesstroom het toetsenbord is, wordt het echoën
	uitgeschakeld zodra LEESTEKEN wordt aangeroepen, en blijft dat zo
	totdat LEESLIJST of LEESWOORD wordt aangeroepen of een Logo-prompt
	geprint wordt. Backslash-, verticale-streep- en tildetekens hebben in
	deze context geen speciale betekenis.

READCHARS getal
RCS getal

	leest "getal" tekens van de leesstroom en geeft die tekens terug als
	een woord. Als de leesstroom een bestand is en het einde van het
	bestand bereikt wordt, geeft READCHARS de lege lijst terug (niet het
	lege woord). Als de leesstroom een terminal is, wordt het echoën
	uitgeschakeld zodra READCHARS wordt aangeroepen, en blijft dat zo
	totdat LEESLIJST of LEESWOORD wordt aangeroepen of een Logo-prompt
	geprint wordt. Backslash-, verticale-streep- en tildetekens hebben in
	deze context geen speciale betekenis.

SHELL commando
(SHELL commando woordvlag)

	Onder Unix geeft dit het resultaat terug van het uitvoeren van
	"commando" als shellcommando. (Het commando wordt naar /bin/sh
	gestuurd, niet naar csh of andere alternatieven.) Als het commando
	een letterlijke lijst is in de instructieregel, en je wil een
	backslash-teken naar de shell sturen, moet je \\ gebruiken om de
	backslash intact door Logo's lezer heen te krijgen. De uitvoer is een
	lijst met één element voor elke regel die het shellcommando
	genereert. Normaliter wordt elke zo'n regel voorgesteld door een
	lijst in de uitvoer, alsof de regel gelezen was met LEESLIJST. Als
	een tweede invoerwaarde is meegegeven, ongeacht de waarde ervan,
	wordt elke regel voorgesteld door een woord in de uitvoer, alsof die
	gelezen was met LEESWOORD. Voorbeeld:

			leer weekdag
			uitvoer eerste eerste shell [date]
			eind

	Dit is "eerste eerste" om het eerste woord van de eerste (en enige)
	regel van de shell-uitvoer eruit te halen.

	Onder MacOS X werkt SHELL zoals onder Unix. SHELL is niet
	beschikbaar onder Mac Classic.

	Onder DOS is SHELL een commando, geen operatie; het stuurt zijn
	invoer naar een DOS-commandoprocessor, maar verzamelt het resultaat
	van het commando niet.

	Onder Windows gedraagt de wxWidgets-versie van Logo zich als onder
	Unix (behalve dat DOS-stijl-commando's begrepen worden; gebruik
	"dir" in plaats van "ls"). De niet-wxWidgets-versie gedraagt zich
	als de DOS-versie.


BESTANDSTOEGANG
-----------------

SETPREFIX tekenreeks

	commando. Stelt een voorvoegsel in dat gebruikt wordt als impliciet
	begin van bestandsnamen in OPENLEES, OPENSCHRIJF, OPENTOEVOEG,
	OPENBIJWERK, OPEN en OPSLAAN. Logo plaatst het juiste
	scheidingsteken (schuine streep voor Unix, backslash voor
	DOS/Windows, dubbele punt voor MacOS Classic) tussen het voorvoegsel
	en de door de gebruiker ingevoerde bestandsnaam. De invoerwaarde
	voor SETPREFIX moet een woord zijn, tenzij het de lege lijst is, om
	aan te geven dat er geen voorvoegsel moet zijn.

PREFIX

	geeft het huidige bestandsvoorvoegsel terug, of [] als er geen
	voorvoegsel is. Zie SETPREFIX.

OPENLEES bestandsnaam

	commando. Opent het genoemde bestand om te lezen. De leespositie
	staat aanvankelijk aan het begin van het bestand.

OPENSCHRIJF bestandsnaam

	commando. Opent het genoemde bestand om te schrijven. Als het
	bestand al bestond, wordt de oude versie verwijderd en een nieuw,
	leeg bestand aangemaakt.

	OPENSCHRIJF, maar niet de andere OPEN-varianten, accepteert als
	invoerwaarde een lijst met twee elementen, waarvan het eerste
	element een variabelenaam moet zijn en het tweede een positief
	geheel getal. Er wordt dan een tekenbuffer van de opgegeven grootte
	aangemaakt. Wanneer een ZETSCHRIJF wordt gedaan met dezelfde lijst
	(in de zin van .EQ, geen kopie, dus je moet zoiets doen als
		? maak "buf [foo 100]
		? openschrijf :buf
		? zetschrijf :buf
		    [...]
		? sluit :buf
	en niet gewoon
		? openschrijf [foo 100]
		? zetschrijf [foo 100]
	enzovoort), worden de geprinte tekens in de buffer opgeslagen; als
	een SLUIT gedaan wordt met dezelfde lijst als invoerwaarde, worden
	de tekens uit de buffer (behandeld als één lang woord, ook als er
	spaties en regeleindes in zitten) de waarde van de opgegeven
	variabele.

OPENTOEVOEG bestandsnaam

	commando. Opent het genoemde bestand om te schrijven. Als het
	bestand al bestaat, wordt de schrijfpositie aanvankelijk ingesteld
	op het eind van het oude bestand, zodat nieuw geschreven gegevens
	eraan worden toegevoegd.

OPENBIJWERK bestandsnaam

	commando. Opent het genoemde bestand om te lezen en te schrijven. De
	lees- en schrijfpositie worden aanvankelijk op het eind van het oude
	bestand gezet, als dat er is. Let op: elk geopend bestand heeft maar
	één positie, voor zowel lezen als schrijven. Als een bestand dat
	geopend is om bij te werken tegelijk LEZER en SCHRIJVER is, dan
	beïnvloedt SETREADPOS ook WRITEPOS en andersom. Ook als je
	afwisselend hetzelfde bestand leest en schrijft, moet je SETREADPOS
	doen tussen een schrijf- en een leesactie, en SETWRITEPOS tussen een
	lees- en een schrijfactie.

SLUIT bestandsnaam

	commando. Sluit het genoemde bestand. Als het bestand op dat moment
	de lezer of schrijver was, wordt de lezer of schrijver veranderd
	naar het toetsenbord of scherm, alsof ZETLEES [] of ZETSCHRIJF []
	gedaan was.

ALLOPEN

	geeft een lijst terug waarvan de elementen de namen zijn van alle
	momenteel geopende bestanden. Deze lijst bevat niet het
	dribble-bestand, indien aanwezig.

CLOSEALL						(bibliotheekprocedure)

	commando. Sluit alle geopende bestanden. Afkorting voor
	FOREACH ALLOPEN [SLUIT ?]

WISBESTAND bestandsnaam
WB bestandsnaam

	commando. Wist (verwijdert) het genoemde bestand, dat op dat moment
	niet geopend mag zijn.

DRIBBLE bestandsnaam

	commando. Maakt een nieuw bestand aan met de opgegeven naam, zoals
	OPENSCHRIJF, en begint alles wat van het toetsenbord gelezen of naar
	de terminal geschreven wordt in dat bestand op te nemen. Dat wil
	zeggen, dit schrijven komt bovenop het schrijven naar WRITER. De
	bedoeling is een transcript te maken van een Logo-sessie, inclusief
	dingen als promptteken en interacties.

NODRIBBLE

	commando. Stopt het kopiëren van informatie naar het dribble-bestand,
	en sluit het bestand.

ZETLEES bestandsnaam

	commando. Maakt het genoemde bestand de leesstroom, gebruikt voor
	LEESLIJST, etc. Het bestand moet al geopend zijn met OPENLEES of
	OPENBIJWERK. Als de invoerwaarde de lege lijst is, wordt de
	leesstroom weer het toetsenbord, zoals gebruikelijk. Het veranderen
	van de leesstroom sluit het bestand dat voorheen de leesstroom was
	niet, dus het is mogelijk om tussen bestanden te wisselen.

ZETSCHRIJF bestandsnaam

	commando. Maakt het genoemde bestand de schrijfstroom, gebruikt voor
	PRINTPERREGEL, etc. Het bestand moet al geopend zijn met
	OPENSCHRIJF, OPENTOEVOEG of OPENBIJWERK. Als de invoerwaarde de lege
	lijst is, wordt de schrijfstroom weer het scherm, zoals gebruikelijk.
	Het veranderen van de schrijfstroom sluit het bestand dat voorheen
	de schrijfstroom was niet, dus het is mogelijk om tussen bestanden
	te wisselen.

	Als de invoerwaarde een lijst is, moet het eerste element een
	variabelenaam zijn, en het tweede en laatste element een positief
	geheel getal; er wordt dan een buffer van dat aantal tekens
	toegewezen, die de schrijfstroom wordt. Als dezelfde lijst (gelijk
	in de .EQ-zin, geen kopie) gebruikt is als invoerwaarde voor
	OPENSCHRIJF, dan wordt de al toegewezen buffer gebruikt, en kan de
	schrijver naar en van deze buffer gewisseld worden, waarbij alle
	tekens opgeteld worden zoals in een bestand. Als dezelfde lijst
	gebruikt wordt als invoerwaarde voor SLUIT, wordt de inhoud van de
	buffer (als een ongeparst woord, dat regeleinde-tekens mag bevatten)
	de waarde van de genoemde variabele. Voor compatibiliteit met
	eerdere versies: als de lijst nog niet geopend was toen de
	ZETSCHRIJF gedaan werd, wordt die impliciet geopend, maar de eerste
	ZETSCHRIJF na deze sluit 'm impliciet weer, waarbij de variabele
	ingesteld wordt en de toegewezen buffer vrijgegeven wordt.

READER

	geeft de naam terug van het huidige leesstroom-bestand, of de lege
	lijst als de leesstroom de terminal is.

WRITER

	geeft de naam terug van het huidige schrijfstroom-bestand, of de
	lege lijst als de schrijfstroom het scherm is.

SETREADPOS tekenpositie

	commando. Zet de bestandswijzer van het leesstroom-bestand zo dat de
	volgende LEESLIJST, etc., begint met lezen bij het tekenpositie-de
	teken in het bestand, tellend vanaf 0. (Dat wil zeggen, SETREADPOS 0
	begint met lezen vanaf het begin van het bestand.) Betekenisloos als
	de leesstroom het scherm is.

SETWRITEPOS tekenpositie

	commando. Zet de bestandswijzer van het schrijfstroom-bestand zo dat
	de volgende PRINTPERREGEL, etc., begint met schrijven bij het
	tekenpositie-de teken in het bestand, tellend vanaf 0. (Dat wil
	zeggen, SETWRITEPOS 0 begint met schrijven vanaf het begin van het
	bestand.) Betekenisloos als de schrijfstroom het scherm is.

READPOS

	geeft de bestandspositie terug van het huidige leesstroom-bestand.

WRITEPOS

	geeft de bestandspositie terug van het huidige schrijfstroom-bestand.

EOF?

	predicaat, geeft WELWAAR terug als er geen tekens meer te lezen
	zijn in het leesstroom-bestand, anders NIETWAAR.

FILE? bestandsnaam					(bibliotheekprocedure)

	predicaat, geeft WELWAAR terug als er een bestand met de opgegeven
	naam bestaat en gelezen kan worden, anders NIETWAAR.


TERMINALTOEGANG
-----------------

TOETS?

	predicaat, geeft WELWAAR terug als er tekens klaarstaan om gelezen
	te worden van de leesstroom. Als de leesstroom een bestand is, komt
	dit overeen met NIET EOF?. Als de leesstroom de terminal is, wordt
	het echoën uitgeschakeld en de terminal in CBREAK-modus gezet (teken
	voor teken in plaats van regel voor regel). Dit blijft zo totdat er
	een regel-modus-leesactie wordt aangevraagd (bv. LEESLIJST). Het
	Unix-besturingssysteem vergeet eventuele wachtende tekens zodra het
	van modus wisselt, dus de eerste TOETS?-aanroep geeft altijd NIETWAAR
	terug.

WISOPDRACHT
WO
CT

	commando. Wist het tekstvenster.

SETCURSOR vector

	commando. De invoerwaarde is een lijst van twee getallen, de x- en
	y-coördinaten van een positie in het tekstvenster (oorsprong in de
	linkerbovenhoek, positieve richting is zuidoost). De tekstcursor
	wordt naar de gevraagde positie verplaatst. Dit commando dwingt ook
	het onmiddellijk printen van eventuele gebufferde tekens af.

CURSOR

	geeft een lijst terug met de huidige x- en y-coördinaten van de
	tekstcursor. Logo kan in de war raken over de huidige cursorpositie
	als je bijvoorbeeld een lange regel typt die omslaat, of als je
	programma escape-codes print die het scherm op vreemde wijze
	beïnvloeden.

SETMARGINS vector

	commando. De invoerwaarde moet een lijst van twee getallen zijn,
	zoals bij SETCURSOR. Het effect is dat het scherm gewist wordt en
	daarna alle verdere printacties naar beneden en naar rechts
	verschoven worden volgens de opgegeven marges. Specifiek: elke keer
	dat een regeleinde-teken geprint wordt (expliciet of impliciet) typt
	Logo x_margin spaties, en bij elke aanroep van SETCURSOR worden de
	marges bij de invoer-x- en y-coördinaten opgeteld. (CURSOR rapporteert
	de cursorpositie relatief aan de marges, zodat deze verschuiving
	onzichtbaar is voor Logo-programma's.) Het doel van dit commando is
	om de weergave van terminalschermen in collegezalen met ontoereikende
	tv-monitoren te ondersteunen, waarbij de boven- en linkerrand van het
	scherm wegvallen.

ZETKLEUROV voorgrond achtergrond
SETTC voorgrond achtergrond

	commando (alleen wxWidgets). De invoerwaarden zijn kleurnummers, of
	RGB-kleurenlijsten, zoals bij turtle graphics. De voorgrond- en
	achtergrondkleuren van het tekstvenster (opdrachtvenster/splitscreen)
	worden naar de opgegeven waarden veranderd. De verandering
	beïnvloedt zowel al geprinte tekst als toekomstige tekst; er is
	maar één tekstkleur voor het hele venster.

	commando (alleen niet-wxWidgets Windows en DOS extended). De
	invoerwaarden zijn kleurnummers, zoals bij turtle graphics.
	Toekomstig printen naar het tekstvenster gebruikt de opgegeven
	kleuren voor voorgrond (de geprinte tekens) en achtergrond (de
	ruimte onder die tekens). Het gebruik van STANDOUT herstelt de
	standaard tekstvensterkleuren. In de DOS-extended-versie
	(ucblogo.exe) zijn kleuren in opdrachtvenster-modus beperkt tot de
	nummers 0-7, en geldt de kleuring alleen voor tekst die door het
	programma geprint wordt, niet voor het echoën van tekst die door de
	gebruiker getypt wordt. Geen van beide beperkingen geldt voor het
	tekstgedeelte van splitscreen-modus, dat intern eigenlijk als
	graphics getekend wordt.

INCREASEFONT
DECREASEFONT

	commando (alleen wxWidgets). Vergroot of verklein het lettertype dat
	gebruikt wordt in het tekst- en bewerkvenster naar de eerstvolgende
	beschikbare grootte.

SETTEXTSIZE hoogte

	commando (alleen wxWidgets). Stelt de "puntgrootte" in van het
	lettertype dat gebruikt wordt in het tekst- en bewerkvenster, op de
	opgegeven gehele invoerwaarde. De gewenste grootte is mogelijk niet
	beschikbaar; in dat geval wordt de dichtstbijzijnde beschikbare
	grootte gebruikt. Let op: er is maar een zwak verband tussen deze
	gehele getallen en pixelgroottes. Onze ruwe schatting is dat het
	aantal pixels in hoogte ongeveer 1,5 keer de puntgrootte is, maar dit
	verschilt per lettertype. Zie SETLABELHEIGHT voor een andere aanpak
	die voor het grafisch venster gebruikt wordt.

TEXTSIZE

	(alleen wxWidgets) geeft de "puntgrootte" terug van het lettertype
	dat gebruikt wordt in het tekst- en bewerkvenster. Zie SETTEXTSIZE
	voor een bespreking van lettertypegrootte. Zie LABELSIZE voor een
	andere aanpak die voor het grafisch venster gebruikt wordt.

SETFONT lettertypenaam

	commando (alleen wxWidgets). Stelt de lettertypefamilie in die in
	alle vensters gebruikt wordt, op de door de invoerwaarde genoemde
	familie. Probeer 'Courier' of 'Monospace' als waarschijnlijke
	mogelijkheden. Niet elke computer heeft dezelfde lettertypes
	geïnstalleerd. Het is verstandig om bij monospace-lettertypes te
	blijven (waarin alle tekens dezelfde breedte hebben).

SETTEXTFONT lettertypenaam

	commando (alleen wxWidgets). Stelt de lettertypefamilie in die in
	het tekstvenster gebruikt wordt, op de door de invoerwaarde genoemde
	familie. Probeer 'Courier' of 'Monospace' als waarschijnlijke
	mogelijkheden. Niet elke computer heeft dezelfde lettertypes
	geïnstalleerd. Het is verstandig om bij monospace-lettertypes te
	blijven (waarin alle tekens dezelfde breedte hebben).


FONT

	(alleen wxWidgets) geeft de naam terug van de lettertypefamilie die
	in alle vensters gebruikt wordt.


REKENEN
=========

NUMERIEKE BEWERKINGEN
------------------------

TELOP getal1 getal2
(TELOP getal1 getal2 getal3 ...)
getal1 + getal2

	geeft de som van de invoerwaarden terug.

VERSCHIL getal1 getal2
getal1 - getal2

	geeft het verschil van de invoerwaarden terug. Een minteken betekent
	infix-verschil in dubbelzinnige gevallen (als het voorafgegaan wordt
	door een complete expressie), tenzij het voorafgegaan wordt door een
	spatie en gevolgd door iets zonder spatie. (Zie ook MINUS.)

MINUS getal
- getal

	geeft het negatieve van de invoerwaarde terug. Een minteken betekent
	unair minus als het vorige token een infix-operator of open haakje
	is, of als het voorafgegaan wordt door een spatie en gevolgd door
	iets zonder spatie. Er is een verschil in bindingssterkte tussen de
	twee vormen:

		MINUS 3 + 4	betekent	-(3+4)
		- 3 + 4		betekent	(-3)+4

VERMENIGVULDIG getal1 getal2
(VERMENIGVULDIG getal1 getal2 getal3 ...)
getal1 * getal2

	geeft het product van de invoerwaarden terug.

DEELDOOR getal1 getal2
(DEELDOOR getal)
getal1 / getal2

	geeft het quotiënt van de invoerwaarden terug. Het quotiënt van twee
	gehele getallen is een geheel getal dan en slechts dan als het
	deeltal een veelvoud van de deler is. (Met andere woorden, DEELDOOR
	5 2 is 2.5, geen 2, maar DEELDOOR 4 2 is 2, geen 2.0 — het doet het
	juiste.) Met één invoerwaarde geeft DEELDOOR het omgekeerde van de
	invoerwaarde terug.

REMAINDER getal1 getal2

	geeft de rest terug bij het delen van "getal1" door "getal2"; beide
	moeten gehele getallen zijn en het resultaat is een geheel getal met
	hetzelfde teken als getal1.

MOD getal1 getal2

	geeft de rest terug bij het delen van "getal1" door "getal2"; beide
	moeten gehele getallen zijn en het resultaat is een geheel getal met
	hetzelfde teken als getal2.

INTEGER getal

	geeft de invoerwaarde terug zonder het gedeelte na de komma, d.w.z.
	een geheel getal met hetzelfde teken als de invoerwaarde, waarvan de
	absolute waarde het grootste gehele getal is dat kleiner dan of
	gelijk is aan de absolute waarde van de invoerwaarde.

AFRONDING getal

	geeft het dichtstbijzijnde gehele getal bij de invoerwaarde terug.

WORTEL getal

	geeft de vierkantswortel van de invoerwaarde terug, die niet-negatief
	moet zijn.

MACHT getal1 getal2

	geeft "getal1" tot de macht "getal2" terug. Als getal1 negatief is,
	moet getal2 een geheel getal zijn.

EXP getal

	geeft e (2,718281828+) tot de macht van de invoerwaarde terug.

LOG10 getal

	geeft de briggse (10-tallige) logaritme van de invoerwaarde terug.

LN getal

	geeft de natuurlijke logaritme van de invoerwaarde terug.

SINUS graden

	geeft de sinus van de invoerwaarde terug, opgevat in graden.

RADSIN radialen

	geeft de sinus van de invoerwaarde terug, opgevat in radialen.

COSINUS graden

	geeft de cosinus van de invoerwaarde terug, opgevat in graden.

RADCOS radialen

	geeft de cosinus van de invoerwaarde terug, opgevat in radialen.

ARCTANGENS getal
(ARCTANGENS x y)

	geeft de arctangens, in graden, van de invoerwaarde terug. Met twee
	invoerwaarden geeft dit de arctangens van y/x terug, als x niet nul
	is, of 90 of -90 afhankelijk van het teken van y, als x nul is.

RADARCTAN getal
(RADARCTAN x y)

	geeft de arctangens, in radialen, van de invoerwaarde terug. Met
	twee invoerwaarden geeft dit de arctangens van y/x terug, als x niet
	nul is, of pi/2 of -pi/2 afhankelijk van het teken van y, als x nul
	is.

	De expressie 2*(RADARCTAN 0 1) kan gebruikt worden om de waarde van
	pi te krijgen.

ISEQ van tot						(bibliotheekprocedure)

	geeft een lijst terug van de gehele getallen van VAN tot en met TOT.

		? laatzien iseq 3 7
		[3 4 5 6 7]
		? laatzien iseq 7 3
		[7 6 5 4 3]

RSEQ van tot aantal					(bibliotheekprocedure)

	geeft een lijst terug van AANTAL gelijk verdeelde rationale getallen
	tussen VAN en TOT, inclusief.

		? laatzien rseq 3 5 9
		[3 3.25 3.5 3.75 4 4.25 4.5 4.75 5]
		? laatzien rseq 3 5 5
		[3 3.5 4 4.5 5]


PREDICATES
----------

KLEINER? getal1 getal2
getal1 < getal2

	geeft WELWAAR terug als de eerste invoerwaarde strikt kleiner is dan
	de tweede.

GROTER? getal1 getal2
getal1 > getal2

	geeft WELWAAR terug als de eerste invoerwaarde strikt groter is dan
	de tweede.

LESSEQUAL? getal1 getal2
getal1 <= getal2

	geeft WELWAAR terug als de eerste invoerwaarde kleiner dan of gelijk
	is aan de tweede.

GREATEREQUAL? getal1 getal2
getal1 >= getal2

	geeft WELWAAR terug als de eerste invoerwaarde groter dan of gelijk
	is aan de tweede.


WILLEKEURIGE GETALLEN
------------------------

GOK getal
(GOK begin eind)

	met één invoerwaarde geeft dit een willekeurig niet-negatief geheel
	getal terug kleiner dan de invoerwaarde, die een positief geheel
	getal moet zijn.

	Met twee invoerwaarden geeft GOK een willekeurig geheel getal terug
	groter dan of gelijk aan de eerste invoerwaarde, en kleiner dan of
	gelijk aan de tweede invoerwaarde. Beide invoerwaarden moeten gehele
	getallen zijn, en de eerste moet kleiner zijn dan de tweede. (GOK 0
	9) is gelijk aan GOK 10; (GOK 3 8) is gelijk aan (GOK 6)+3.

RERANDOM
(RERANDOM zaadgetal)

	commando. Maakt de resultaten van GOK reproduceerbaar. Normaliter is
	de reeks willekeurige getallen elke keer dat Logo gebruikt wordt
	anders. Als je herhaaldelijk dezelfde reeks pseudowillekeurige
	getallen nodig hebt, bv. om een programma te debuggen, zeg dan
	RERANDOM voor de eerste aanroep van GOK. Als je meer dan één
	herhaalbare reeks nodig hebt, kan je RERANDOM een geheel getal als
	invoerwaarde meegeven; elke mogelijke invoerwaarde selecteert een
	unieke reeks getallen.


PRINTOPMAAK
-------------

FORM getal breedte precisie

	geeft een woord terug met een printbare weergave van "getal",
	eventueel voorafgegaan door spaties (en daardoor geen getal voor het
	uitvoeren van rekenkundige bewerkingen), met minstens "breedte"
	tekens, inclusief precies "precisie" cijfers na de komma. (Als
	"precisie" 0 is, komt er geen komma in de uitvoer.)

	Als debug-functie print (FORM getal -1 opmaak) het zwevendekomma-
	getal "getal" volgens de C printf-"opmaak", om het volgende mogelijk
	te maken:

		leer hex :getal
		uv form :getal -1 "|%08X %08X|
		eind

	om het exacte resultaat van zwevendekomma-bewerkingen te
	achterhalen. De precies benodigde opmaak kan machine-afhankelijk
	zijn.


BITSGEWIJZE BEWERKINGEN
--------------------------

BITAND getal1 getal2
(BITAND getal1 getal2 getal3 ...)

	geeft de bitsgewijze EN van de invoerwaarden terug, die gehele
	getallen moeten zijn.

BITOR getal1 getal2
(BITOR getal1 getal2 getal3 ...)

	geeft de bitsgewijze OF van de invoerwaarden terug, die gehele
	getallen moeten zijn.

BITXOR getal1 getal2
(BITXOR getal1 getal2 getal3 ...)

	geeft de bitsgewijze EXCLUSIEVE OF van de invoerwaarden terug, die
	gehele getallen moeten zijn.

BITNOT getal

	geeft de bitsgewijze NIET van de invoerwaarde terug, die een geheel
	getal moet zijn.

ASHIFT getal1 getal2

	geeft "getal1" terug, rekenkundig naar links verschoven met "getal2"
	bits. Als getal2 negatief is, verschuift het naar rechts met
	tekenuitbreiding. De invoerwaarden moeten gehele getallen zijn.

LSHIFT getal1 getal2

	geeft "getal1" terug, logisch naar links verschoven met "getal2"
	bits. Als getal2 negatief is, verschuift het naar rechts met
	nulopvulling. De invoerwaarden moeten gehele getallen zijn.


LOGISCHE BEWERKINGEN
=======================

EN waarheidswaarde1 waarheidswaarde2
(EN wh1 wh2 wh3 ...)

	geeft WELWAAR terug als alle invoerwaarden WELWAAR zijn, anders
	NIETWAAR. Alle invoerwaarden moeten WELWAAR of NIETWAAR zijn.
	(Vergelijking is niet hoofdlettergevoelig, ongeacht de waarde van
	CASEIGNOREDP. Dat wil zeggen, "welwaar", "Welwaar" of "WELWAAR" zijn
	allemaal hetzelfde.) Een invoerwaarde mag een lijst zijn; die wordt
	dan opgevat als een expressie om te DOEn, en die expressie moet een
	WELWAAR- of NIETWAAR-waarde opleveren. Lijst-expressies worden van
	links naar rechts geëvalueerd; zodra een NIETWAAR-waarde gevonden
	wordt, worden de overige invoerwaarden niet meer onderzocht.
	Voorbeeld:
		MAAK "RESULTAAT EN [NIET (:X = 0)] [(1 / :X) > .5]
	om deling door nul te voorkomen als het eerste deel onwaar is.

OF waarheidswaarde1 waarheidswaarde2
(OF wh1 wh2 wh3 ...)

	geeft WELWAAR terug als een van de invoerwaarden WELWAAR is, anders
	NIETWAAR. Alle invoerwaarden moeten WELWAAR of NIETWAAR zijn.
	(Vergelijking is niet hoofdlettergevoelig, ongeacht de waarde van
	CASEIGNOREDP. Dat wil zeggen, "welwaar", "Welwaar" of "WELWAAR" zijn
	allemaal hetzelfde.) Een invoerwaarde mag een lijst zijn; die wordt
	dan opgevat als een expressie om te DOEn, en die expressie moet een
	WELWAAR- of NIETWAAR-waarde opleveren. Lijst-expressies worden van
	links naar rechts geëvalueerd; zodra een WELWAAR-waarde gevonden
	wordt, worden de overige invoerwaarden niet meer onderzocht.
	Voorbeeld:
		ALS OF :X=0 [een.lange.berekening] [...]
	om de lange berekening te voorkomen als aan de eerste voorwaarde
	voldaan is.

NIET waarheidswaarde

	geeft WELWAAR terug als de invoerwaarde NIETWAAR is, en omgekeerd.
	De invoerwaarde mag een lijst zijn; die wordt dan opgevat als een
	expressie om te DOEn, en die expressie moet een WELWAAR- of
	NIETWAAR-waarde opleveren.


GRAPHICS
==========

Berkeley Logo biedt traditionele Logo-turtlegraphics met één turtle.
Meerdere turtles, dynamische turtles en botsingsdetectie worden niet
ondersteund. Dit is het meest hardware-afhankelijke deel van Logo; sommige
functies bestaan misschien op de ene machine maar niet op de andere. Toch
is het doel geweest om Logo-programma's zo overdraagbaar mogelijk te
maken, in plaats van optimaal gebruik te maken van de mogelijkheden van
elke machine. Logo probeert met name het scherm zo te schalen dat de
turtlecoördinaten [-100 -100] en [100 100] op het grafisch venster passen,
en dat de beeldverhouding 1:1 is.

Het midden van het grafisch venster (dat afhankelijk van de gebruikte
machine al dan niet het volledige scherm is) is turtlepositie [0 0].
Positieve X is naar rechts; positieve Y is omhoog. Richtingen (hoeken)
worden gemeten in graden, met de klok mee vanaf de positieve Y-as. (Dit
wijkt af van de gebruikelijke wiskundige conventie om hoeken tegen de klok
in te meten vanaf de positieve X-as.) De turtle wordt weergegeven als een
gelijkbenige driehoek; de werkelijke turtlepositie is het middelpunt van
de basis (de korte zijde). De turtle wordt echter één stap achter zijn
werkelijke positie getekend, zodat de weergave van de basis van de
turtle-driehoek geen lijn verbergt die er loodrecht op getekend is (zoals
zou gebeuren na het tekenen van een vierkant).

Kleuren zijn natuurlijk hardware-afhankelijk. Logo biedt echter
gedeeltelijke hardware-onafhankelijkheid door kleurnummers 0 tot en met 7
op alle computers gelijk te interpreteren:

	0  zwart	1  blauw	2  groen	3  cyaan
	4  rood		5  magenta	6  geel		7  wit

Waar mogelijk biedt Logo extra, door de gebruiker instelbare kleuren; hoe
veel er beschikbaar zijn hangt af van de hardware- en
besturingssysteemomgeving. Als er minstens 16 kleuren beschikbaar zijn,
probeert Logo eenduidige beginwaarden te geven voor de kleuren 8-15:

	 8  bruin	 9  lichtbruin	10  bosgroen	11  aquagroen
	12  zalm	13  paars	14  oranje	15  grijs

Logo begint met een zwarte achtergrond en een witte pen.


TURTLEBEWEGING
-----------------

VOORUIT afstand
VT afstand

	beweegt de turtle vooruit, in de richting waarin die kijkt, over de
	opgegeven afstand (gemeten in turtlestappen).

ACHTERUIT afstand
AT afstand

	beweegt de turtle achteruit, d.w.z. precies tegenovergesteld aan de
	richting waarin die kijkt, over de opgegeven afstand. (De richting
	van de turtle verandert niet.)

LINKS graden
LI graden

	draait de turtle tegen de klok in over de opgegeven hoek, gemeten in
	graden (1/360 van een cirkel).

RECHTS graden
RE graden

	draait de turtle met de klok mee over de opgegeven hoek, gemeten in
	graden (1/360 van een cirkel).

ZETPOS positie

	beweegt de turtle naar een absolute positie in het grafisch venster.
	De invoerwaarde is een lijst van twee getallen, de X- en
	Y-coördinaten.

ZETXY xcoord ycoord

	beweegt de turtle naar een absolute positie in het grafisch venster.
	De twee invoerwaarden zijn getallen, de X- en Y-coördinaten.

ZETX xcoord

	beweegt de turtle horizontaal van zijn oude positie naar een nieuwe
	absolute horizontale coördinaat. De invoerwaarde is de nieuwe
	X-coördinaat.

ZETY ycoord

	beweegt de turtle verticaal van zijn oude positie naar een nieuwe
	absolute verticale coördinaat. De invoerwaarde is de nieuwe
	Y-coördinaat.

ZETRICHTING graden
ZR graden

	draait de turtle naar een nieuwe absolute richting. De invoerwaarde
	is een getal, de richting in graden met de klok mee vanaf de
	positieve Y-as.

NAARBEGIN
NB

	beweegt de turtle naar het midden van het scherm. Gelijk aan
	ZETPOS [0 0] ZETRICHTING 0.

ARC hoek straal

	tekent een boog van een cirkel, met de turtle in het midden, met de
	opgegeven straal, beginnend bij de richting van de turtle en zich
	met de klok mee uitstrekkend over de opgegeven hoek. De turtle
	beweegt niet.

TURTLEBEWEGINGSQUERIES
-------------------------

POSITIE

	geeft de huidige positie van de turtle terug, als een lijst van twee
	getallen, de X- en Y-coördinaten.

XCOR							(bibliotheekprocedure)

	geeft een getal terug, de X-coördinaat van de turtle.

YCOR							(bibliotheekprocedure)

	geeft een getal terug, de Y-coördinaat van de turtle.

GRADEN

	geeft een getal terug, de richting van de turtle in graden.

NAAR positie

	geeft een getal terug, de richting waarin de turtle zou moeten
	kijken zodat die vanaf zijn huidige positie zou wijzen naar de
	positie die als invoerwaarde is meegegeven.

SCRUNCH

	geeft een lijst terug met twee getallen, de X- en Y-scrunchfactoren,
	zoals gebruikt door SETSCRUNCH. (Let wel op dat SETSCRUNCH twee
	getallen als invoerwaarden neemt, niet één lijst met getallen.)


TURTLE- EN VENSTERBESTURING
------------------------------

KOM
ST

	maakt de turtle zichtbaar.

WEGTURTLE
WEGT
HT

	maakt de turtle onzichtbaar. Het is een goed idee om dit te doen
	terwijl je middenin een ingewikkelde tekening zit, omdat het
	verbergen van de turtle het tekenen aanzienlijk versnelt.

WEGTEKENING

	wist alle lijnen die de turtle in het grafisch venster getekend
	heeft. De toestand van de turtle (positie, richting, penmodus, etc.)
	verandert niet.

WISTEKENING
WT
CS

	wist het grafisch venster en stuurt de turtle naar zijn
	beginpositie en -richting. Als NAARBEGIN en WEGTEKENING samen.

RAND

	vertelt de turtle om in randmodus te gaan: vanaf nu, als de turtle
	gevraagd wordt om voorbij de rand van het grafisch venster te
	bewegen, "wikkelt" die zich om en verschijnt weer aan de
	tegenoverliggende kant van het venster. De bovenrand wikkelt naar de
	onderrand, en de linkerrand wikkelt naar de rechterrand. (Het
	venster is dus topologisch gelijk aan een torus.) Dit is de
	beginmodus van de turtle. Vergelijk VENSTER en FENCE.

VENSTER

	vertelt de turtle om in venstermodus te gaan: vanaf nu, als de
	turtle gevraagd wordt om voorbij de rand van het grafisch venster te
	bewegen, beweegt die buiten beeld. Het zichtbare grafisch venster
	wordt beschouwd als slechts een deel van een oneindig grafisch vlak;
	de turtle kan zich overal op dat vlak bevinden. (Als je de turtle
	kwijtraakt, brengt NAARBEGIN 'm terug naar het midden van het
	venster.) Vergelijk RAND en FENCE.

FENCE

	vertelt de turtle om in hekmodus te gaan: vanaf nu, als de turtle
	gevraagd wordt om voorbij de rand van het grafisch venster te
	bewegen, beweegt die zo ver als kan en stopt dan bij de rand met een
	foutmelding "buiten bereik". Vergelijk RAND en VENSTER.

VUL

	vult een gebied van het grafisch venster dat de turtle bevat en
	begrensd wordt door eerder getekende lijnen. Dit is niet overdraagbaar;
	het werkt niet op alle machines, en werkt mogelijk niet overal exact
	hetzelfde.

FILLED kleur instructies

	voert de instructies uit, en onthoudt daarbij alle punten die
	bezocht worden door turtlebewegingscommando's, beginnend *en
	eindigend* bij de beginpositie van de turtle. Tekent vervolgens
	(zonder rekening te houden met de penmodus) de resulterende veelhoek,
	in de huidige penkleur, en vult de veelhoek met de opgegeven kleur,
	die een kleurnummer of een RGB-lijst mag zijn. De instructielijst mag
	geen andere aanroep van FILLED bevatten.

LABEL tekst

	neemt een woord of lijst als invoerwaarde, en print die in het
	grafisch venster, beginnend bij de positie van de turtle.

SETLABELHEIGHT hoogte

	commando (alleen wxWidgets). Neemt een positief geheel getal als
	invoerwaarde en probeert de lettertypegrootte zo in te stellen dat
	de tekenhoogte (inclusief staarten) dat aantal turtlestappen is. Dit
	verschilt van het aantal schermpixels als SETSCRUNCH gebruikt is.
	Merk ook op dat SETSCRUNCH de lettertypegrootte verandert om te
	proberen deze hoogte in turtlestappen te behouden. Let op dat de
	bijbehorende queryoperatie voor dit commando LABELSIZE heet, niet
	LABELHEIGHT, omdat die zowel de breedte als de hoogte van tekens in
	het huidige lettertype vertelt.

OPDRACHTVENSTER
OV
TS

	herschikt de grootte en positie van vensters om de beschikbare
	ruimte in het tekstvenster (het venster voor interactie met Logo) te
	maximaliseren. De details verschillen per machine. Vergelijk
	SPLITSVENSTER en FULLSCREEN.

FULLSCREEN
FS

	herschikt de grootte en positie van vensters om de beschikbare
	ruimte in het grafisch venster te maximaliseren. De details
	verschillen per machine. Vergelijk SPLITSVENSTER en OPDRACHTVENSTER.

	Omdat er een tekstvenster moet zijn om te kunnen printen (inclusief
	het printen van de Logo-prompt), schakelt Logo automatisch van
	fullscreen naar splitsvenster zodra er iets geprint wordt.

	In de DOS-versie raak je bij het wisselen van fullscreen naar
	splitsvenster het deel van de tekening kwijt dat verborgen was door
	het tekstvenster. [Deze ontwerpkeuze komt voort uit geheugenschaarste,
	waardoor het extra geheugen om een onzichtbaar deel van een tekening
	te onthouden te duur leek.]

SPLITSVENSTER
SS

	herschikt de grootte en positie van vensters om wat ruimte voor
	tekstinteractie mogelijk te maken, terwijl het grootste deel van het
	grafisch venster zichtbaar blijft. De details verschillen per
	machine. Vergelijk OPDRACHTVENSTER en FULLSCREEN.

SETSCRUNCH xschaal yschaal

	past de beeldverhouding en schaling van de grafische weergave aan.
	Na gebruik van dit commando wordt elke verdere turtlebeweging
	aangepast door de horizontale en verticale omvang van de beweging te
	vermenigvuldigen met de twee getallen die als invoerwaarden zijn
	meegegeven. Bijvoorbeeld, na de instructie "SETSCRUNCH 2 1" beweegt
	een beweging met richting 45 graden twee keer zo ver horizontaal als
	verticaal. Als je vierkanten niet vierkant uitkomen, probeer dit.
	(Je kan de beeldverhouding ook expres verkeerd instellen om een
	ellips te tekenen.)

	Alleen in wxWidgets verandert SETSCRUNCH ook de grootte van het
	tekstlettertype dat gebruikt wordt voor het LABEL-commando, om te
	proberen de hoogte van tekens geschaald te houden met de verticale
	turtlestapgrootte.

	Voor alle moderne computers, voor DOS-machines, worden de
	schaalfactoren aanvankelijk ingesteld volgens wat de hardware zegt
	dat de beeldverhouding is, maar de hardware liegt soms. Voor DOS
	worden de door SETSCRUNCH ingestelde waarden onthouden in een
	bestand (genaamd SCRUNCH.DAT) en automatisch toegepast bij het
	begin van een Logo-sessie.

REFRESH

	(commando) vertelt Logo om de bewegingen van de turtle te onthouden,
	zodat die gebruikt kunnen worden voor hoge-resolutie-printen
	(wxWidgets) of om het grafisch venster te vernieuwen als het
	verplaatst, van grootte veranderd, of overlapt wordt (niet-wxWidgets).
	Dit is de standaardinstelling.

NOREFRESH

	(commando) vertelt Logo om de bewegingen van de turtle niet te
	onthouden, wat nuttig kan zijn om tijd en geheugen te besparen als je
	programma interactief of geanimeerd is, in plaats van een statische
	tekening te maken die je later wil printen (wxWidgets). In
	niet-wxWidgets-versies kan het gebruik van NOREFRESH voorkomen dat
	Logo de grafische afbeelding herstelt nadat het venster verplaatst,
	van grootte veranderd, of overlapt is.


TURTLE- EN VENSTERQUERIES
----------------------------

ZICHTBAAR?

	geeft WELWAAR terug als de turtle getoond wordt (zichtbaar is),
	NIETWAAR als de turtle verborgen is. Zie KOM en WEGTURTLE.

SCREENMODE

	geeft het woord TEXTSCREEN, SPLITSCREEN of FULLSCREEN terug,
	afhankelijk van de huidige schermmodus.

TURTLEMODE

	geeft het woord WRAP, FENCE of WINDOW terug, afhankelijk van de
	huidige turtlemodus.

LABELSIZE

	(alleen wxWidgets) geeft een lijst terug van twee positieve gehele
	getallen, de breedte en hoogte van tekens die door LABEL getoond
	worden, gemeten in turtlestappen (wat verschilt van schermpixels als
	SETSCRUNCH gebruikt is). Er is geen SETLABELSIZE omdat de breedte en
	hoogte van een lettertype niet los van elkaar instelbaar zijn, dus
	het omgekeerde van deze bewerking is SETLABELHEIGHT, dat maar één
	getal neemt voor de gewenste hoogte.


PEN- EN ACHTERGRONDBESTURING
-------------------------------

De turtle draagt een pen waarmee tekeningen gemaakt kunnen worden. Op elk
moment kan de pen OMHOOG staan (in welk geval het bewegen van de turtle
niets verandert aan wat er op het grafisch scherm staat) of OMLAAG (in
welk geval de turtle een spoor achterlaat). Als de pen omlaag staat, kan
die in een van drie modi werken: PAINT (zodat die lijnen tekent als de
turtle beweegt), ERASE (zodat die lijnen wist die eerder op of langs dat
pad getekend waren), of REVERSE (zodat die de status van elk punt langs
het pad van de turtle omkeert).

PENNEER
PN
PD

	zet de positie van de pen op OMLAAG, zonder de modus te veranderen.

PENOP
PP
PU

	zet de positie van de pen op OMHOOG, zonder de modus te veranderen.

PENPAINT
PPT

	zet de positie van de pen op OMLAAG en de modus op PAINT.

GUM
PE

	zet de positie van de pen op OMLAAG en de modus op ERASE.

PENOMGEKEERD
PX

	zet de positie van de pen op OMLAAG en de modus op REVERSE. (Dit kan
	op systeemafhankelijke manieren interacteren met het gebruik van
	kleur.)

ZETPENKLEUR kleurnummer.of.rgblijst
SETPC kleurnummer.of.rgblijst

	stelt de penkleur in op het opgegeven nummer, dat een niet-negatief
	geheel getal moet zijn. Er zijn beginwaarden voor de eerste 16
	kleuren:

	 0  zwart	 1  blauw	 2  groen	 3  cyaan
	 4  rood	 5  magenta	 6  geel	 7  wit
	 8  bruin	 9  lichtbruin	10  bosgroen	11  aquagroen
	12  zalm	13  paars	14  oranje	15  grijs

	maar andere kleuren kunnen aan nummers toegekend worden met het
	PALETTE-commando. Als alternatief stelt dit de penkleur in op de
	opgegeven RGB-waarden (een lijst van drie niet-negatieve getallen
	kleiner dan 100, die het procentuele verzadigingsaandeel van rood,
	groen en blauw in de gewenste kleur opgeven).

SETPALETTE kleurnummer rgblijst

	stelt de werkelijke kleur in die bij een opgegeven nummer hoort, als
	de hardware en het besturingssysteem dat toestaan. Kleurnummer moet
	een geheel getal groter dan of gelijk aan 8 zijn. (Logo probeert de
	eerste 8 kleuren constant te houden.) De tweede invoerwaarde is een
	lijst van drie niet-negatieve getallen kleiner dan 100, die het
	procentuele verzadigingsaandeel van rood, groen en blauw in de
	gewenste kleur opgeven.

ZETPENDIKTE grootte

	stelt de dikte van de pen in. De invoerwaarde is óf een los positief
	geheel getal, óf een lijst van twee positieve gehele getallen (voor
	horizontale en verticale dikte). Sommige versies negeren het tweede
	getal, en hebben altijd een vierkante pen.

SETPENPATTERN patroon

	stelt hardware-afhankelijke peneigenschappen in. Dit commando is
	niet gegarandeerd compatibel tussen implementaties op verschillende
	machines.

SETPEN lijst						(bibliotheekprocedure)

	stelt de positie, modus, dikte en hardware-afhankelijke eigenschappen
	van de pen in volgens de informatie in de invoerlijst, die afkomstig
	moet zijn van een eerdere aanroep van PEN.

ZETACHTERGROND kleurnummer.of.rgblijst
SETBG kleurnummer.of.rgblijst

	stelt de achtergrondkleur van het scherm in via slotnummer of
	RGB-waarden. Zie ZETPENKLEUR voor details.


PENQUERIES
------------

PENDOWN?

	geeft WELWAAR terug als de pen omlaag staat, NIETWAAR als die
	omhoog staat.

PENMODE

	geeft een van de woorden PAINT, ERASE of REVERSE terug, afhankelijk
	van de huidige penmodus.

PENKLEUR
PK

	geeft een kleurnummer terug, een niet-negatief geheel getal dat met
	een bepaalde kleur geassocieerd is, of een lijst van RGB-waarden als
	zo'n lijst de meest recente invoerwaarde voor ZETPENKLEUR was. Er
	zijn beginwaarden voor de eerste 16 kleuren:

	 0  zwart	 1  blauw	 2  groen	 3  cyaan
	 4  rood	 5  magenta	 6  geel	 7  wit
	 8  bruin	 9  lichtbruin	10  bosgroen	11  aquagroen
	12  zalm	13  paars	14  oranje	15  grijs

	maar andere kleuren kunnen aan nummers toegekend worden met het
	PALETTE-commando.

PALETTE kleurnummer

	geeft een lijst terug van drie niet-negatieve getallen kleiner dan
	100, die het procentuele verzadigingsaandeel van rood, groen en
	blauw aangeven in de kleur die bij het opgegeven nummer hoort.

PENSIZE

	geeft een lijst terug van twee positieve gehele getallen, die de
	horizontale en verticale dikte van de turtlepen aangeven. (In
	sommige implementaties, waaronder wxWidgets, zijn de twee getallen
	altijd gelijk.)

PENPATTERN

	geeft systeemspecifieke peninformatie terug.

PEN							(bibliotheekprocedure)

	geeft een lijst terug met de positie, modus, dikte en
	hardware-specifieke eigenschappen van de pen, voor gebruik door
	SETPEN.

ACHTERGROND
AG
BG

	geeft de achtergrondkleur van het grafisch venster terug, hetzij als
	slotnummer, hetzij als RGB-lijst, afhankelijk van hoe die ingesteld
	is. (Zie PENKLEUR.)


TEKENINGEN OPSLAAN EN LADEN
------------------------------

SAVEPICT bestandsnaam

	commando. Schrijft een bestand met de opgegeven naam met daarin de
	toestand van het grafisch venster, inclusief eventuele afwijkende
	kleurpaletinstellingen, in Logo's interne formaat. Deze tekening kan
	met LOADPICT weer op het scherm gezet worden. Het formaat is niet
	overdraagbaar tussen platformen, en ook niet leesbaar door andere
	programma's. Zie EPSPICT om Logo-graphics naar andere programma's te
	exporteren.

LOADPICT bestandsnaam

	commando. Leest het opgegeven bestand, dat geschreven moet zijn door
	een SAVEPICT-commando, en herstelt het grafisch venster en de
	kleurpaletinstellingen naar de waarden die in het bestand
	opgeslagen zijn. Elke eerdere tekening op het scherm wordt gewist.

EPSPICT bestandsnaam

	commando. Schrijft een bestand met de opgegeven naam, met daarin een
	Encapsulated-Postscript-representatie (EPS) van de toestand van het
	grafisch venster. Dit bestand kan geïmporteerd worden in andere
	programma's die het EPS-formaat begrijpen. Beperkingen: de tekening
	mag geen VUL, GUM, of PENOMGEKEERD gebruiken; zulke instructies
	worden genegeerd bij de vertaling naar Postscript-vorm.


MUISQUERIES
-------------

MOUSEPOS

	geeft de coördinaten van de muis terug, mits die zich binnen het
	grafisch venster bevindt, in turtlecoördinaten. Als de muis buiten
	het grafisch venster is, wordt de laatste positie binnen het venster
	teruggegeven. Uitzondering: als een muisknop binnen het grafisch
	venster ingedrukt wordt en vastgehouden terwijl de muis buiten het
	venster gesleept wordt, wordt de muispositie teruggegeven alsof het
	venster groot genoeg was om die te bevatten.

CLICKPOS

	geeft de coördinaten terug waar de muis zich bevond toen een
	muisknop het laatst werd ingedrukt, mits die positie binnen het
	grafisch venster lag, in turtlecoördinaten. (alleen wxWidgets)

BUTTON?

	geeft WELWAAR terug als een muisknop ingedrukt is en de muis zich
	boven het grafisch venster bevindt. Zodra de knop ingedrukt is,
	blijft BUTTON? waar totdat de knop losgelaten wordt, ook als de muis
	buiten het grafisch venster gesleept wordt.

BUTTON

	geeft 0 terug als er sinds de laatste aanroep van BUTTON geen
	muisknop is ingedrukt binnen het Logo-venster. Anders geeft dit een
	geheel getal tussen 1 en 3 terug dat aangeeft welke knop het laatst
	ingedrukt is. Normaliter betekent 1 links, 2 rechts, en 3 midden,
	maar besturingssystemen kunnen dit anders instellen.


WORKSPACEBEHEER
=================

PROCEDURE-DEFINITIE
----------------------

LEER procnaam :invoer1 :invoer2 ...				(speciale vorm)

	commando. Bereidt Logo voor om een procedure-definitie te
	accepteren. De procedure krijgt de naam "procnaam" en er mag nog
	geen procedure met die naam bestaan. De invoerwaarden heten
	"invoer1" enzovoort. Elk aantal invoerwaarden is toegestaan,
	inclusief geen. Namen van procedures en invoerwaarden zijn niet
	hoofdlettergevoelig.

	Anders dan elke andere Logo-procedure neemt LEER als invoerwaarden
	de daadwerkelijk in de instructieregel getypte woorden, alsof ze
	allemaal gequote waren, in plaats van de resultaten van het
	evalueren van expressies om de invoerwaarden te leveren. (Dat is wat
	"speciale vorm" betekent.)

	Deze versie van Logo staat een variabel aantal invoerwaarden voor
	een procedure toe. Na de procedurenaam komen vier soorten dingen,
	*in deze volgorde*:

	    1.   0 of meer VERPLICHTE invoerwaarden    :FOO :FROBOZZ
	    2.   0 of meer OPTIONELE invoerwaarden    [:BAZ 87] [:THINGO 5+9]
	    3.   0 of 1 REST-invoerwaarde              [:GARPLY]
	    4.   0 of 1 STANDAARD-aantal               5

	Elke procedure heeft een MINIMUM, STANDAARD en MAXIMUM aantal
	invoerwaarden. (Dat laatste kan oneindig zijn.)

	Het MINIMUM aantal invoerwaarden is het aantal verplichte
	invoerwaarden, die als eerste moeten komen. Een verplichte
	invoerwaarde wordt aangegeven met de notatie

			:invoernaam

	Na alle verplichte invoerwaarden mogen er nul of meer optionele
	invoerwaarden komen, elk voorgesteld door de volgende notatie:

			[:invoernaam standaardwaarde.expressie]

	Wanneer de procedure wordt aangeroepen, en er geen echte
	invoerwaarden zijn meegegeven voor deze optionele invoerwaarden,
	worden de standaardwaarde-expressies geëvalueerd om waarden voor de
	bijbehorende invoernamen in te stellen. De invoerwaarden worden van
	links naar rechts verwerkt, dus een standaardwaarde-expressie kan
	gebaseerd zijn op eerdere invoerwaarden. Voorbeeld:

			leer proc :inlijst [:startwaarde eerste :inlijst]

	Als de procedure aangeroepen wordt met

			proc [a b c]

	dan heeft de variabele INLIJST de waarde [A B C] en de variabele
	STARTWAARDE de waarde A. Als de procedure aangeroepen wordt met

			(proc [a b c] "x)

	dan heeft INLIJST de waarde [A B C] en STARTWAARDE de waarde X.

	Na alle verplichte en optionele invoerwaarden mag er één enkele
	"rest"-invoerwaarde komen, voorgesteld door de volgende notatie:

			[:invoernaam]

	Dit is een rest-invoerwaarde in plaats van een optionele
	invoerwaarde omdat er geen standaardwaarde-expressie is. Er kan
	hoogstens één rest-invoerwaarde zijn. Wanneer de procedure
	aangeroepen wordt, is de waarde van deze invoernaam een lijst met
	alle daadwerkelijk meegegeven invoerwaarden die niet gebruikt zijn
	voor verplichte of optionele invoerwaarden. Voorbeeld:

			leer proc :in1 [:in2 "foo] [:in3 "baz] [:in4]

	Als deze procedure aangeroepen wordt met

			proc "x

	dan heeft IN1 de waarde X, IN2 de waarde FOO, IN3 de waarde BAZ, en
	IN4 de waarde [] (de lege lijst). Als die aangeroepen wordt met

			(proc "a "b "c "d "e)

	dan heeft IN1 de waarde A, IN2 de waarde B, IN3 de waarde C, en IN4
	de waarde [D E].

	Het MAXIMUM aantal invoerwaarden voor een procedure is oneindig als
	er een rest-invoerwaarde is opgegeven; anders is het het aantal
	verplichte invoerwaarden plus het aantal optionele invoerwaarden.

	Het STANDAARD aantal invoerwaarden voor een procedure, dat is het
	aantal invoerwaarden dat geaccepteerd wordt als de aanroep niet
	tussen haakjes staat, is normaliter gelijk aan het minimumaantal.
	Als je een ander standaardaantal wil, kan je dat aangeven door het
	gewenste standaardaantal als laatste ding op de LEER-regel te
	zetten. Voorbeeld:

			leer proc :in1 [:in2 "foo] [:in3] 3

	Deze procedure heeft een minimum van één invoerwaarde, een
	standaardaantal van drie invoerwaarden, en een oneindig maximum.

	Logo reageert op het LEER-commando door de procedure-definitiemodus
	te betreden. Het promptteken verandert van "?" naar ">" en alle
	instructies die je typt worden deel van de definitie, totdat je een
	regel typt met alleen het woord EIND.

DEFINIEER procnaam tekst

	commando. Definieert een procedure met de naam "procnaam" en tekst
	"tekst". Als er al een procedure met dezelfde naam bestaat, vervangt
	de nieuwe definitie de oude. De invoerwaarde tekst moet een lijst
	zijn waarvan de elementen lijsten zijn. Het eerste element is een
	lijst met invoerwaarden; die ziet eruit als een LEER-regel maar
	zonder het woord LEER, zonder de procedurenaam, en zonder de dubbele
	punten voor de invoernamen. Met andere woorden, de elementen van
	deze eerste sublijst zijn woorden voor de namen van verplichte
	invoerwaarden en lijsten voor de namen van optionele of
	rest-invoerwaarden. De overige sublijsten van de invoerwaarde tekst
	vormen samen de body van de procedure, met één sublijst voor elke
	instructieregel van de body. (Er is geen EIND-regel in de
	invoerwaarde tekst.) Het is een fout om een primitieve procedure te
	herdefiniëren, tenzij de variabele REDEFP de waarde WELWAAR heeft.

PRINTTEKST procnaam

	geeft de tekst van de procedure met de naam "procnaam" terug in de
	vorm die DEFINIEER verwacht: een lijst van lijsten, waarvan de eerste
	de invoerwaarden van de procedure beschrijft en de rest de regels
	van de body ervan zijn. De tekst geeft geen opmaakinformatie terug
	die gebruikt werd toen de procedure gedefinieerd werd, zoals
	vervolgregels en extra spaties.

FULLTEXT procnaam

	geeft een weergave van de procedure "procnaam" terug waarin de
	opmaakinformatie behouden blijft. Als de procedure gedefinieerd is
	met LEER, BEWERK, of OPEN, dan is de uitvoer een lijst van woorden.
	Elk woord stelt één complete regel van de definitie voor in de vorm
	die LEESWOORD teruggeeft, inclusief extra spaties en vervolgregels.
	Het laatste element van de uitvoer stelt de EIND-regel voor. Als de
	procedure gedefinieerd is met DEFINIEER, dan is de uitvoer een lijst
	van lijsten. Als deze lijsten geprint worden, één per regel, ziet
	het resultaat eruit als een definitie met LEER. Let op: de uitvoer
	van FULLTEXT is niet geschikt om als invoerwaarde voor DEFINIEER te
	gebruiken!

COPYDEF nieuwenaam oudenaam

	commando. Maakt "nieuwenaam" een procedure identiek aan "oudenaam".
	Die laatste mag een primitief zijn. Als "nieuwenaam" al gedefinieerd
	was, gaat de vorige definitie ervan verloren. Als "nieuwenaam" al
	een primitief was, is de herdefinitie niet toegestaan tenzij de
	variabele REDEFP de waarde WELWAAR heeft.

	Let op: Logo-dialecten verschillen in de volgorde van invoerwaarden
	voor COPYDEF. Dit dialect gebruikt de "MAAK-volgorde", niet de
	"NAME-volgorde".


VARIABELE-DEFINITIE
----------------------

MAAK variabelenaam waarde

	commando. Kent de waarde "waarde" toe aan de variabele met de naam
	"variabelenaam", die een woord moet zijn. Variabelenamen zijn niet
	hoofdlettergevoelig. Als er al een variabele met dezelfde naam
	bestaat, wordt de waarde van die variabele veranderd. Zo niet, dan
	wordt een nieuwe globale variabele aangemaakt.

NAME waarde variabelenaam				(bibliotheekprocedure)

	commando. Hetzelfde als MAAK maar met de invoerwaarden in
	omgekeerde volgorde.

LOKAAL variabelenaam
LOKAAL variabelenamenlijst
(LOKAAL variabelenaam1 variabelenaam2 ...)

	commando. Accepteert als invoerwaarden één of meer woorden, of een
	lijst met woorden. Voor elk van deze woorden wordt een variabele
	aangemaakt, met dat woord als naam. De variabelen zijn lokaal aan de
	op dat moment lopende procedure. Logo-variabelen volgen dynamische
	scoperegels; een variabele die lokaal is aan een procedure is
	beschikbaar voor elke subprocedure die door die procedure aangeroepen
	wordt. De door LOKAAL aangemaakte variabelen hebben geen
	beginwaarde; ze moeten een waarde krijgen (bv. met MAAK) voordat de
	procedure probeert hun waarde te lezen.

LOCALMAKE variabelenaam waarde				(bibliotheekprocedure)

	commando. Maakt de genoemde variabele lokaal, zoals LOKAAL, en kent
	er de opgegeven waarde aan toe, zoals MAAK.

DING variabelenaam
:gequote.variabelenaam

	geeft de waarde terug van de variabele waarvan de naam de
	invoerwaarde is. Als er meer dan één zo'n variabele is, wordt de
	meest binnenste lokale variabele met die naam gekozen. De
	dubbele-punt-notatie is geen afkorting voor DING, maar voor de
	combinatie

				ding "

	zodat :FOO betekent DING "FOO.

GLOBAL variabelenaam
GLOBAL variabelenamenlijst
(GLOBAL variabelenaam1 variabelenaam2 ...)

	commando. Accepteert als invoerwaarden één of meer woorden, of een
	lijst met woorden. Voor elk van deze woorden wordt een globale
	variabele aangemaakt, met dat woord als naam. De enige reden waarom
	dit nodig is, is dat je misschien de "setter"-notatie ZETXYZ wil
	gebruiken voor een variabele XYZ die nog geen waarde heeft; GLOBAL
	"XYZ maakt dat toegestaan. Let op: als er op dat moment een lokale
	variabele met dezelfde naam bestaat, zorgt dit commando er *niet*
	voor dat Logo de globale waarde gebruikt in plaats van de lokale.


KENMERKLIJSTEN
-----------------

Let op: namen van kenmerklijsten zijn altijd niet-hoofdlettergevoelig.
Namen van afzonderlijke kenmerken zijn wel of niet hoofdlettergevoelig,
afhankelijk van de waarde van CASEIGNOREDP, die standaard WELWAAR is.

In principe is elke mogelijke naam de naam van een kenmerklijst, die
aanvankelijk leeg is. Logo geeft dus nooit een foutmelding "zo'n
kenmerklijst bestaat niet", zoals het wel zou doen voor niet-gedefinieerde
procedure- of variabelenamen. Maar de primitieve procedures die met
"alle" kenmerklijsten te maken hebben (CONTENTS, PLISTS, etc.) tonen
alleen de niet-lege. Een kenmerklijst "wissen" (zie WIS hieronder)
betekent 'm leeg maken, waarbij alle kenmerken eruit verwijderd worden.

PLAATSKENMERK kenmerklijstnaam kenmerknaam waarde

	commando. Voegt een kenmerk toe aan de kenmerklijst "kenmerklijstnaam"
	met naam "kenmerknaam" en waarde "waarde".

NEEMKENMERK kenmerklijstnaam kenmerknaam

	geeft de waarde terug van het kenmerk "kenmerknaam" in de
	kenmerklijst "kenmerklijstnaam", of de lege lijst als zo'n kenmerk
	niet bestaat.

WISKENMERK kenmerklijstnaam kenmerknaam

	commando. Verwijdert het kenmerk met de naam "kenmerknaam" uit de
	kenmerklijst met de naam "kenmerklijstnaam".

KENMERKLIJST kenmerklijstnaam

	geeft een lijst terug waarvan de oneven-genummerde elementen de
	namen zijn, en de even-genummerde elementen de waarden, van de
	kenmerken in de kenmerklijst met de naam "kenmerklijstnaam". De
	uitvoer is een kopie van de daadwerkelijke kenmerklijst; het later
	veranderen van kenmerken verandert niet magisch een eerder door
	KENMERKLIJST teruggegeven lijst.


PREDICATES
----------

PROCEDURE? naam

	geeft WELWAAR terug als de invoerwaarde de naam van een procedure is.

PRIMITIEF? naam

	geeft WELWAAR terug als de invoerwaarde de naam is van een
	primitieve procedure (een die ingebouwd is in Logo). Let op: sommige
	van de in dit document beschreven procedures zijn
	bibliotheekprocedures, geen primitieven.

DEFINITIE? naam

	geeft WELWAAR terug als de invoerwaarde de naam is van een
	zelfgedefinieerde procedure, inclusief een bibliotheekprocedure.

NAAM? naam

	geeft WELWAAR terug als de invoerwaarde de naam is van een
	variabele.

PLIST? naam

	geeft WELWAAR terug als de invoerwaarde de naam is van een
	*niet-lege* kenmerklijst. (In principe is elk woord de naam van een
	kenmerklijst; als je er nog geen kenmerken in gezet hebt, geeft
	KENMERKLIJST van die naam een lege lijst terug, in plaats van een
	foutmelding.)


QUERIES
-------

Let op: alle procedures waarvan de invoerwaarde aangeduid wordt als
"inhoudslijst" accepteren een los woord (opgevat als procedurenaam), een
lijst met woorden (opgevat als namen van procedures), of een lijst van
drie lijsten zoals beschreven bij het CONTENTS-commando hierboven.

CONTENTS

	geeft een "inhoudslijst" terug, d.w.z. een lijst van drie lijsten met
	respectievelijk de namen van gedefinieerde procedures, variabelen,
	en kenmerklijsten. Deze lijst bevat alle onbegraven benoemde
	elementen in de workspace.

BURIED

	geeft een inhoudslijst terug met alle begraven benoemde elementen in
	de workspace.

TRACED

	geeft een inhoudslijst terug met alle getraceerde benoemde elementen
	in de workspace.

STEPPED

	geeft een inhoudslijst terug met alle doorgestapte benoemde
	elementen in de workspace.

PROCEDURES

	geeft een lijst terug van de namen van alle onbegraven
	zelfgedefinieerde procedures in de workspace. Let op, dit is een
	lijst van namen, geen inhoudslijst. (Procedures die een inhoudslijst
	als invoerwaarde vereisen, accepteren deze lijst echter wel.)

PRIMITIVES

	geeft een lijst terug van de namen van alle primitieve procedures in
	de workspace. Let op, dit is een lijst van namen, geen inhoudslijst.
	(Procedures die een inhoudslijst als invoerwaarde vereisen,
	accepteren deze lijst echter wel.)

NAMES

	geeft een inhoudslijst terug bestaande uit een lege lijst (wat geen
	procedurenamen aangeeft), gevolgd door een lijst van alle onbegraven
	variabelenamen in de workspace.

PLISTS

	geeft een inhoudslijst terug bestaande uit twee lege lijsten (wat
	geen procedures of variabelen aangeeft), gevolgd door een lijst van
	alle onbegraven niet-lege kenmerklijsten in de workspace.

NAMELIST variabelenaam					(bibliotheekprocedure)
NAMELIST variabelenamenlijst

	geeft een inhoudslijst terug bestaande uit een lege lijst, gevolgd
	door een lijst van de naam of namen die als invoerwaarde meegegeven
	zijn. Dit is nuttig in combinatie met workspacebeheer-procedures die
	een inhoudslijst als invoerwaarde vereisen.

PLLIST kenmerklijstnaam				(bibliotheekprocedure)
PLLIST kenmerklijstnamenlijst

	geeft een inhoudslijst terug bestaande uit twee lege lijsten, gevolgd
	door een lijst van de naam of namen die als invoerwaarde meegegeven
	zijn. Dit is nuttig in combinatie met workspacebeheer-procedures die
	een inhoudslijst als invoerwaarde vereisen.

ARITY procedurenaam

	geeft een lijst terug van drie getallen: het minimum-, standaard- en
	maximumaantal invoerwaarden voor de procedure met de opgegeven naam.
	Het is een fout als zo'n procedure niet bestaat. Een maximum van -1
	betekent dat het aantal invoerwaarden onbeperkt is.

NODES

	geeft een lijst terug van twee getallen. Het eerste geeft het aantal
	geheugen-nodes weer dat op dat moment in gebruik is. Het tweede
	toont het maximumaantal nodes dat in gebruik is geweest sinds de
	laatste aanroep van NODES. (Een node is een klein blok
	computergeheugen zoals gebruikt door Logo. Elk getal gebruikt één
	node. Elk niet-numeriek woord gebruikt één node, plus wat
	niet-node-geheugen voor de tekens in het woord. Elke array kost één
	node, plus wat niet-node-geheugen, evenals het geheugen dat nodig is
	voor de elementen ervan. Elke lijst vereist één node per element,
	evenals het geheugen binnen de elementen.) Als je het
	geheugengebruik van een algoritme wil volgen, is het het beste om GC
	aan te roepen aan het begin van elke iteratie, omdat het maximum
	anders opslag zou meetellen die niet gebruikt wordt maar nog niet
	opgeruimd is.


INSPECTIE
-----------

PRINTDEFINITIE inhoudslijst
PO inhoudslijst

	commando. Print naar de schrijfstroom de definities van alle
	procedures, variabelen en kenmerklijsten die genoemd worden in de
	invoerwaarde inhoudslijst.

POALL							(bibliotheekprocedure)

	commando. Print alle onbegraven definities in de workspace.
	Afkorting voor PO CONTENTS.

POPS							(bibliotheekprocedure)

	commando. Print de definities van alle onbegraven procedures in de
	workspace. Afkorting voor PO PROCEDURES.

PONS							(bibliotheekprocedure)

	commando. Print de definities van alle onbegraven variabelen in de
	workspace. Afkorting voor PO NAMES.

POPLS							(bibliotheekprocedure)

	commando. Print de inhoud van alle onbegraven niet-lege
	kenmerklijsten in de workspace. Afkorting voor PO PLISTS.

PON variabelenaam					(bibliotheekprocedure)
PON variabelenamenlijst

	commando. Print de definities van de genoemde variabele(n).
	Afkorting voor PO NAMELIST variabelenaam(lijst).

POPL kenmerklijstnaam					(bibliotheekprocedure)
POPL kenmerklijstnamenlijst

	commando. Print de definities van de genoemde kenmerklijst(en).
	Afkorting voor PO PLLIST kenmerklijstnaam(lijst).

POT inhoudslijst

	commando. Print de titelregels van de genoemde procedures en de
	definities van de genoemde variabelen en kenmerklijsten. Voor
	kenmerklijsten wordt de hele lijst op één regel getoond in plaats
	van als een reeks PLAATSKENMERK-instructies zoals bij PO.

POTS							(bibliotheekprocedure)

	commando. Print de titelregels van alle onbegraven procedures in de
	workspace. Afkorting voor POT PROCEDURES.


WORKSPACEBESTURING
---------------------

WIS inhoudslijst
ER inhoudslijst

	commando. Verwijdert uit de workspace de procedures, variabelen en
	kenmerklijsten die in de invoerwaarde genoemd worden. Primitieve
	procedures mogen niet gewist worden, tenzij de variabele REDEFP de
	waarde WELWAAR heeft.

ERALL

	commando. Verwijdert alle onbegraven procedures, variabelen en
	kenmerklijsten uit de workspace. Afkorting voor WIS CONTENTS.

ERPS

	commando. Verwijdert alle onbegraven procedures uit de workspace.
	Afkorting voor WIS PROCEDURES.

ERNS

	commando. Verwijdert alle onbegraven variabelen uit de workspace.
	Afkorting voor WIS NAMES.

ERPLS

	commando. Verwijdert alle onbegraven kenmerklijsten uit de
	workspace. Afkorting voor WIS PLISTS.

ERN variabelenaam					(bibliotheekprocedure)
ERN variabelenamenlijst

	commando. Verwijdert uit de workspace de variabele(n) die in de
	invoerwaarde genoemd worden. Afkorting voor WIS NAMELIST
	variabelenaam(lijst).

ERPL kenmerklijstnaam					(bibliotheekprocedure)
ERPL kenmerklijstnamenlijst

	commando. Verwijdert uit de workspace de kenmerklijst(en) die in de
	invoerwaarde genoemd worden. Afkorting voor WIS PLLIST
	kenmerklijstnaam(lijst).

VERBERG inhoudslijst

	commando. Begraaft de procedures, variabelen en kenmerklijsten die
	in de invoerwaarde genoemd worden. Een begraven element wordt niet
	opgenomen in de lijsten die CONTENTS, PROCEDURES, VARIABLES en
	PLISTS teruggeven, maar wel in de lijst die BURIED teruggeeft. Dit
	betekent onder andere dat begraven dingen niet geprint worden door
	POALL of opgeslagen door OPSLAAN.

BURYALL							(bibliotheekprocedure)

	commando. Afkorting voor VERBERG CONTENTS.

BURYNAME variabelenaam					(bibliotheekprocedure)
BURYNAME variabelenamenlijst

	commando. Afkorting voor VERBERG NAMELIST variabelenaam(lijst).

VERBERGNIET inhoudslijst

	commando. Haalt de procedures, variabelen en kenmerklijsten die in
	de invoerwaarde genoemd worden uit hun begraven staat. Dat wil
	zeggen, de genoemde elementen komen weer in beeld in CONTENTS, etc.

UNBURYALL						(bibliotheekprocedure)

	commando. Afkorting voor VERBERGNIET BURIED.

UNBURYNAME variabelenaam				(bibliotheekprocedure)
UNBURYNAME variabelenamenlijst

	commando. Afkorting voor VERBERGNIET NAMELIST variabelenaam(lijst).

BURIED? inhoudslijst

	geeft WELWAAR terug als de eerste procedure, variabele of
	kenmerklijst genoemd in de inhoudslijst begraven is, anders NIETWAAR.
	Alleen het eerste ding in de lijst wordt getest; het meest gangbare
	gebruik is met een woord als invoerwaarde dat een procedure noemt,
	maar een inhoudslijst is toegestaan zodat je BURIED? [[] [VARIABELE]]
	of BURIED? [[] [] [KENMERKLIJST]] kan doen.

TRACE inhoudslijst

	commando. Markeert de genoemde elementen voor tracering. Er wordt
	een bericht geprint elke keer dat een getraceerde procedure
	aangeroepen wordt, met de daadwerkelijke invoerwaarden, en elke keer
	dat een getraceerde procedure STOPt of UITVOERt. Er wordt een
	bericht geprint elke keer dat een nieuwe waarde aan een getraceerde
	variabele toegekend wordt met MAAK. Er wordt een bericht geprint
	elke keer dat een nieuw kenmerk gegeven wordt aan een getraceerde
	kenmerklijst met PLAATSKENMERK.

UNTRACE inhoudslijst

	commando. Schakelt tracering uit voor de genoemde elementen.

TRACED? inhoudslijst

	geeft WELWAAR terug als de eerste procedure, variabele of
	kenmerklijst genoemd in de inhoudslijst getraceerd wordt, anders
	NIETWAAR. Alleen het eerste ding in de lijst wordt getest; het meest
	gangbare gebruik is met een woord als invoerwaarde dat een procedure
	noemt, maar een inhoudslijst is toegestaan zodat je
	TRACED? [[] [VARIABELE]] of TRACED? [[] [] [KENMERKLIJST]] kan doen.

STEP inhoudslijst

	commando. Markeert de genoemde elementen om te doorstappen. Elke
	keer dat een doorgestapte procedure aangeroepen wordt, wordt elke
	instructieregel in de procedure-body geprint voordat die uitgevoerd
	wordt, en wacht Logo tot de gebruiker een regeleinde typt op de
	terminal. Er wordt een bericht geprint elke keer dat een
	doorgestapte variabelenaam "overschaduwd" wordt doordat een lokale
	variabele met dezelfde naam aangemaakt wordt, hetzij als
	procedure-invoerwaarde, hetzij door het LOKAAL-commando.

UNSTEP inhoudslijst

	commando. Schakelt doorstappen uit voor de genoemde elementen.

STEPPED? inhoudslijst

	geeft WELWAAR terug als de eerste procedure, variabele of
	kenmerklijst genoemd in de inhoudslijst doorgestapt wordt, anders
	NIETWAAR. Alleen het eerste ding in de lijst wordt getest; het meest
	gangbare gebruik is met een woord als invoerwaarde dat een procedure
	noemt, maar een inhoudslijst is toegestaan zodat je
	STEPPED? [[] [VARIABELE]] of STEPPED? [[] [] [KENMERKLIJST]] kan
	doen.

BEWERK inhoudslijst
BW inhoudslijst
(BEWERK)
(BW)

	commando. Als het aangeroepen wordt met een invoerwaarde, schrijft
	BEWERK de definities van de genoemde elementen naar een tijdelijk
	bestand en bewerkt dat bestand, met een editor die afhankelijk is
	van het platform dat je gebruikt. In wxWidgets, en in de
	MacOS-Classic-versie, zit er een editor ingebouwd in Logo. In de
	niet-wxWidgets-versies voor Unix, MacOS X, Windows en DOS gebruikt
	Logo je favoriete editor, zoals bepaald door de EDITOR-
	omgevingsvariabele. Als je geen EDITOR-variabele hebt, bewerkt het
	de definities met jove. Als het aangeroepen wordt zonder
	invoerwaarde, bewerkt BEWERK hetzelfde bestand dat overbleef van een
	vorige BEWERK- of BEWERKBESTAND-instructie. Als je de editor
	verlaat, leest Logo de herziene definities en past de workspace
	daarop aan. Het is geen fout als de invoerwaarde namen bevat
	waarvoor nog geen eerdere definitie bestaat.

	Als er een variabele LOADNOISILY bestaat waarvan de waarde WELWAAR
	is, dan printen LEER-commando's in het tijdelijke bestand na het
	verlaten van de editor "PROCNAAM gedefinieerd" (waarbij PROCNAAM de
	naam is van de procedure die gedefinieerd wordt); als LOADNOISILY
	NIETWAAR of niet gedefinieerd is, worden LEER-commando's in het
	bestand stilzwijgend uitgevoerd.

	Als er een omgevingsvariabele TEMP bestaat, gebruikt Logo de waarde
	ervan als de map om het tijdelijke bestand voor het bewerken in te
	schrijven.

	Bij wijze van uitzondering kan het BEWERK-commando gebruikt worden
	zonder de standaardinvoerwaarde en zonder haakjes, mits er niets op
	volgt in de instructieregel.

BEWERKBESTAND bestandsnaam
BB bestandsnaam

	commando. Start de Logo-editor, zoals BEWERK, maar in plaats van een
	tijdelijk bestand te bewerken, bewerkt het het bestand dat door de
	invoerwaarde opgegeven is. Als je de editor verlaat, leest Logo het
	herziene bestand, net als bij BEWERK.

	BEWERKBESTAND onthoudt ook de bestandsnaam, zodat een volgend
	BEWERK-commando zonder invoerwaarde hetzelfde bestand opnieuw
	bewerkt.

	BEWERKBESTAND is bedoeld als alternatief voor OPEN en OPSLAAN. Je
	kan zelf een workspacebestand bijhouden, waarbij je de volgorde
	waarin definities voorkomen bepaalt, commentaar in het bestand
	bewaart, enzovoort.

EDALL							(bibliotheekprocedure)

	commando. Afkorting voor BEWERK CONTENTS.

EDPS							(bibliotheekprocedure)

	commando. Afkorting voor BEWERK PROCEDURES.

EDNS							(bibliotheekprocedure)

	commando. Afkorting voor BEWERK NAMES.

EDPLS							(bibliotheekprocedure)

	commando. Afkorting voor BEWERK PLISTS.

EDN variabelenaam					(bibliotheekprocedure)
EDN variabelenamenlijst

	commando. Afkorting voor BEWERK NAMELIST variabelenaam(lijst).

EDPL kenmerklijstnaam					(bibliotheekprocedure)
EDPL kenmerklijstnamenlijst

	commando. Afkorting voor BEWERK PLLIST kenmerklijstnaam(lijst).

OPSLAAN bestandsnaam

	commando. Slaat de definities op van alle onbegraven procedures,
	variabelen en niet-lege kenmerklijsten, in het genoemde bestand.
	Gelijk aan

			leer opslaan :bestandsnaam
			lokaal "oudeschrijver
			maak "oudeschrijver writer
			openschrijf :bestandsnaam
			zetschrijf :bestandsnaam
			poall
			zetschrijf :oudeschrijver
			sluit :bestandsnaam
			eind

	Bij wijze van uitzondering kan OPSLAAN gebruikt worden zonder
	invoerwaarde en zonder haakjes, als het het laatste ding op de
	commandoregel is. In dat geval wordt de bestandsnaam van het meest
	recente OPEN- of OPSLAAN-commando gebruikt. (Het is een fout als er
	nog geen eerdere OPEN of OPSLAAN geweest is.)

SAVEL inhoudslijst bestandsnaam			(bibliotheekprocedure)

	commando. Slaat de definities op van de procedures, variabelen en
	kenmerklijsten die opgegeven zijn door "inhoudslijst", in het
	bestand met de naam "bestandsnaam".

OPEN bestandsnaam
LAAD bestandsnaam

	commando. Leest instructies uit het genoemde bestand en voert ze
	uit. Het bestand mag procedure-definities met LEER bevatten, en die
	worden geaccepteerd, ook als er al een procedure met dezelfde naam
	bestaat. Als het bestand een lijstwaarde toekent aan een variabele
	met de naam STARTUP, dan wordt die lijst als instructielijst
	uitgevoerd nadat het bestand geladen is. Als er een variabele
	LOADNOISILY bestaat waarvan de waarde WELWAAR is, dan printen
	LEER-commando's in het bestand "PROCNAAM gedefinieerd" (waarbij
	PROCNAAM de naam is van de procedure die gedefinieerd wordt); als
	LOADNOISILY NIETWAAR of niet gedefinieerd is, worden LEER-commando's
	in het bestand stilzwijgend uitgevoerd.

CSLSLOAD naam

	commando. Laadt het genoemde bestand, zoals OPEN, maar vanuit de map
	met de Computer Science Logo Style-programma's in plaats van de map
	van de huidige gebruiker.

HELP naam
(HELP)

	commando. Print informatie uit de referentiehandleiding over de
	primitieve procedure die door de invoerwaarde genoemd wordt. Zonder
	invoerwaarde toont het een lijst van alle primitieven waarover hulp
	beschikbaar is. Als er een omgevingsvariabele LOGOHELP bestaat, dan
	wordt de waarde daarvan gebruikt als de map om hulpbestanden in te
	zoeken, in plaats van de standaardmap.

	Als HELP aangeroepen wordt met de naam van een gedefinieerde
	procedure waarvoor geen hulpbestand bestaat, print het de titelregel
	van de procedure, gevolgd door regels uit de procedure-body die met
	een puntkomma beginnen, en stopt zodra een regel zonder puntkomma
	gezien wordt.

	Bij wijze van uitzondering kan het HELP-commando gebruikt worden
	zonder de standaardinvoerwaarde en zonder haakjes, mits er niets op
	volgt in de instructieregel.

SETEDITOR pad

	commando. Vertelt Logo om het opgegeven programma als editor te
	gebruiken in plaats van de standaardeditor. De vorm van een pad
	hangt af van je besturingssysteem.

SETLIBLOC pad

	commando. Vertelt Logo om de opgegeven map als bibliotheek te
	gebruiken in plaats van de standaardmap. (Let op: veel Logo-
	"primitieve" procedures bevinden zich eigenlijk in de bibliotheek,
	dus die kunnen onbeschikbaar worden als je nieuwe bibliotheek ze
	niet bevat!) De vorm van een pad hangt af van je besturingssysteem.

SETHELPLOC pad

	commando. Vertelt Logo om in de opgegeven map te zoeken naar de
	informatie die het HELP-commando levert, in plaats van de
	standaardmap. De vorm van een pad hangt af van je besturingssysteem.

SETCSLSLOC pad

	commando. Vertelt Logo om de opgegeven map te gebruiken voor het
	CSLSLOAD-commando, in plaats van de standaardmap. De vorm van een
	pad hangt af van je besturingssysteem.

SETTEMPLOC pad

	commando. Vertelt Logo om tijdelijke editorbestanden in de opgegeven
	map te schrijven in plaats van in de standaardmap. Je moet
	schrijfrechten hebben voor deze map. De vorm van een pad hangt af
	van je besturingssysteem.

GC
(GC iets)

	commando. Draait de garbage collector, waarbij ongebruikte nodes
	teruggewonnen worden. Logo doet dit toch al wanneer nodig, maar je
	wil dit commando misschien gebruiken om precies te bepalen wanneer
	Logo dat doet. De getallen die door de NODES-bewerking teruggegeven
	worden zijn met name niet erg zinvol tenzij er garbage opgeruimd is.
	Een andere reden om GC te gebruiken is dat een garbage collection een
	merkbaar deel van een seconde kost, en je collecties misschien wil
	plannen vóór of na een tijdkritische animatie. Als het aangeroepen
	wordt met een invoerwaarde (van welke waarde dan ook), draait GC een
	volledige garbage collection, inclusief GCTWA (Garbage Collect Truly
	Worthless Atoms, wat betekent dat het woorden uit Logo's geheugen
	verwijdert die vroeger procedure- of variabelenamen waren maar dat
	niet meer zijn); zonder invoerwaarde doet GC een generationele
	garbage collection, wat betekent dat alleen recent aangemaakte nodes
	onderzocht worden. (Dat laatste is meestal goed genoeg.)

.SETSEGMENTSIZE getal

	commando. Stelt het aantal nodes in dat Logo in één keer van het
	besturingssysteem toewijst, op getal, dat een positief geheel getal
	moet zijn. De naam heeft een punt ervoor omdat er nare dingen
	gebeuren als je een getal gebruikt dat te klein of te groot is voor
	je computer. De beginwaarde is 16.000 voor de meeste systemen, maar
	is kleiner voor op de 68000 gebaseerde Macs. Het groter maken
	versnelt berekeningen (door het aantal garbage collections te
	verminderen) ten koste van het toewijzen van meer geheugen dan
	nodig.


CONTROLESTRUCTUREN
=====================

Let op: in de volgende beschrijvingen mag een "instructielijst" een lijst
of een woord zijn. In dat laatste geval wordt het woord in lijstvorm
geparst voordat het gedraaid wordt. Dus DOE LEESWOORD of DOE LEESLIJST
werkt. Het eerste heeft een lichte voorkeur omdat het een vervolgregel
(met ~) toestaat die op de eerste regel een commentaar (met ;) bevat.

Een invoerwaarde "wh" (waarheidswaarde) moet het woord WELWAAR, het woord
NIETWAAR, of een lijst zijn. Als het een lijst is, moet het een
Logo-expressie zijn, die geëvalueerd wordt om een waarde op te leveren die
WELWAAR of NIETWAAR moet zijn. De vergelijkingen met WELWAAR en NIETWAAR
zijn altijd niet-hoofdlettergevoelig.

Een doe-lijst kan bestaan uit óf één expressie (die een waarde oplevert),
óf nul of meer instructies (die iets doen, in plaats van een waarde terug
te geven), afhankelijk van de context:

	PRINTPERREGEL ALSANDERS :X<0 ["NEGATIEF] ["POSITIEF]  ; één waarde in elk geval
	HERHAAL 4 [PRINTPERREGEL "A PRINTPERREGEL "B]  ; twee instructies


DOE instructielijst

	commando of operatie. Voert de Logo-instructies in de invoerlijst
	uit; geeft een waarde terug als de lijst een expressie bevat die een
	waarde teruggeeft.

RUNRESULT instructielijst

	voert de instructies in de invoerwaarde uit; geeft een lege lijst
	terug als die instructies geen uitvoer opleveren, of een lijst
	waarvan het enige element de uitvoer is van het draaien van de
	invoerwaarde-instructielijst. Nuttig om commando-of-operatie-
	controlestructuren te bedenken:

		lokaal "resultaat
		maak "resultaat runresult [iets]
		als leeg? :resultaat [stop]
		uitvoer eerste :resultaat

HERHAAL getal instructielijst

	commando. Voert de "instructielijst" herhaaldelijk uit, "getal"
	keer.

FOREVER instructielijst

	commando. Voert de "instructielijst" herhaaldelijk uit, totdat iets
	binnen de instructielijst (zoals STOP of GOOI) 'm laat stoppen.

TELHERHAAL

	geeft de herhalingsteller terug van de meest binnenste huidige
	HERHAAL of FOREVER, beginnend bij 1. Als er geen HERHAAL of FOREVER
	actief is, geeft dit -1 terug.

	De afkorting # kan gebruikt worden voor TELHERHAAL, tenzij de
	HERHAAL zich binnen de sjabloon-invoerwaarde van een hogere-orde-
	procedure zoals FOREACH bevindt; in dat geval heeft # een andere
	betekenis.

ALS wh instructielijst
(ALS wh instructielijst1 instructielijst2)

	commando. Als de eerste invoerwaarde de waarde WELWAAR heeft, dan
	voert ALS de tweede invoerwaarde uit. Als de eerste invoerwaarde de
	waarde NIETWAAR heeft, doet ALS niets. (Als een derde invoerwaarde
	meegegeven is, gedraagt ALS zich als ALSANDERS, zoals hieronder
	beschreven.) Het is een fout als de eerste invoerwaarde niet WELWAAR
	of NIETWAAR is.

	Voor compatibiliteit met eerdere versies van Logo: als een
	ALS-instructie niet tussen haakjes staat, maar het eerste ding op de
	instructieregel na de tweede invoerwaarde-expressie een letterlijke
	lijst is (d.w.z. een lijst tussen vierkante haken), dan wordt ALS
	behandeld alsof het ALSANDERS was, maar wordt er een
	waarschuwingsbericht gegeven. Als deze afwijkende ALS in een
	procedure-body voorkomt, wordt de waarschuwing maar één keer gegeven,
	de eerste keer dat de procedure in elke Logo-sessie aangeroepen
	wordt.

ALSANDERS wh instructielijst1 instructielijst2

	commando of operatie. Als de eerste invoerwaarde de waarde WELWAAR
	heeft, dan voert ALSANDERS de tweede invoerwaarde uit. Als de eerste
	invoerwaarde de waarde NIETWAAR heeft, dan voert ALSANDERS de derde
	invoerwaarde uit. ALSANDERS geeft een waarde terug als de
	instructielijst een expressie bevat die een waarde teruggeeft.

TEST wh

	commando. Onthoudt de invoerwaarde, die WELWAAR of NIETWAAR moet
	zijn, voor gebruik door latere ALSWAAR- of ALSNIETWAAR-instructies.
	Het effect van TEST is lokaal aan de procedure waarin het gebruikt
	wordt; elke bijbehorende ALSWAAR of ALSNIETWAAR moet in dezelfde
	procedure of een subprocedure staan.

ALSWAAR instructielijst
ALSW instructielijst

	commando. Voert de invoerwaarde uit als de meest recente
	TEST-instructie een WELWAAR-invoerwaarde had. De TEST moet in
	dezelfde procedure of een superprocedure geweest zijn.

ALSNIETWAAR instructielijst
ALSNW instructielijst

	commando. Voert de invoerwaarde uit als de meest recente
	TEST-instructie een NIETWAAR-invoerwaarde had. De TEST moet in
	dezelfde procedure of een superprocedure geweest zijn.

STOP

	commando. Beëindigt het draaien van de procedure waarin het
	voorkomt. De besturing gaat terug naar de context waarin die
	procedure aangeroepen werd. De gestopte procedure geeft geen waarde
	terug.

UITVOER waarde
UV waarde

	commando. Beëindigt het draaien van de procedure waarin het
	voorkomt. Die procedure geeft de waarde "waarde" terug aan de
	context waarin die aangeroepen werd. Laat je niet in de war brengen:
	UITVOER zelf is een commando, maar de procedure die UITVOER
	aanroept is een operatie.

VANG label instructielijst

	commando of operatie. Voert de tweede invoerwaarde uit. Geeft een
	waarde terug als die instructielijst een waarde teruggeeft. Als er,
	tijdens het uitvoeren van de instructielijst, een GOOI-instructie
	uitgevoerd wordt met een label gelijk aan de eerste invoerwaarde
	(niet-hoofdlettergevoelige vergelijking), dan wordt het uitvoeren van
	de instructielijst onmiddellijk beëindigd. In dat geval geeft VANG
	een waarde terug als er een waarde-invoerwaarde aan GOOI meegegeven
	is. Het label moet een woord zijn.

	Als het label het woord ERROR is, dan heeft elke foutconditie die
	optreedt tijdens het uitvoeren van de instructielijst hetzelfde
	effect als GOOI "ERROR, in plaats van een foutmelding te printen en
	terug te keren naar het topniveau. VANG geeft geen waarde terug als
	er een fout gevangen is. Ook is, tijdens het uitvoeren van de
	instructielijst, de variabele ERRACT tijdelijk ongebonden. (Als er
	een fout optreedt terwijl ERRACT een waarde heeft, wordt die waarde
	opgevat als een instructielijst om uit te voeren na het printen van
	de foutmelding. Meestal is de waarde van ERRACT, indien aanwezig, de
	lijst [PAUZE].)

GOOI label
(GOOI label waarde)

	commando. Moet gebruikt worden binnen het bereik van een VANG met
	een gelijk label. Beëindigt het uitvoeren van de instructielijst van
	de VANG. Als GOOI met maar één invoerwaarde gebruikt wordt, geeft de
	bijbehorende VANG geen waarde terug. Als GOOI met twee invoerwaarden
	gebruikt wordt, levert de tweede een uitvoerwaarde voor de VANG.

	GOOI "TOPLEVEL kan gebruikt worden om alle lopende procedures en
	interactieve pauzes te beëindigen, en terug te keren naar de
	topniveau-instructieprompt. Het typen van het systeem-
	onderbrekingsteken (alt-S voor wxWidgets; anders normaal control-C
	voor Unix, control-Q voor DOS, of command-punt voor Mac) heeft
	hetzelfde effect.

	GOOI "ERROR kan gebruikt worden om een foutconditie te genereren.
	Als de fout niet gevangen wordt, print het een bericht (GOOI "ERROR)
	met de gebruikelijke aanduiding van waar de fout (in dit geval de
	GOOI) optrad. Als een tweede invoerwaarde gebruikt wordt samen met
	een label ERROR, dan wordt die tweede invoerwaarde gebruikt als de
	tekst van de foutmelding in plaats van het standaardbericht. Ook is
	in dat geval de aangeduide locatie voor de fout niet de locatie van
	de GOOI, maar de locatie waar de procedure met de GOOI erin
	aangeroepen werd. Dit stelt zelfgedefinieerde procedures in staat om
	foutmeldingen te genereren alsof ze primitieven waren. Let op: in
	dit geval geeft de bijbehorende VANG "ERROR, indien aanwezig, geen
	waarde terug, omdat de tweede invoerwaarde van GOOI niet als
	retourwaarde beschouwd wordt.

	GOOI "SYSTEM verlaat Logo onmiddellijk, terug naar het
	besturingssysteem, zonder het gebruikelijke afscheidsbericht te
	printen en zonder een tijdelijk editorbestand van BEWERK te
	verwijderen.

FOUT

	geeft een lijst terug die de zojuist gevangen fout beschrijft, indien
	van toepassing. Als er geen fout gevangen is sinds het laatste
	gebruik van FOUT, wordt de lege lijst teruggegeven. De foutenlijst
	bevat vier elementen: een geheel getal dat het type fout aangeeft,
	de tekst van de foutmelding (als één woord inclusief spaties), de
	naam van de procedure waarin de fout optrad, en de instructieregel
	waarop de fout optrad.

PAUZE

	commando of operatie. Betreedt een interactieve pauze. De gebruiker
	wordt om instructies gevraagd, zoals op het topniveau, maar met een
	prompt die de naam bevat van de procedure waarin PAUZE aangeroepen
	werd. Lokale variabelen van die procedure zijn beschikbaar tijdens
	de pauze. PAUZE geeft een waarde terug als de pauze beëindigd wordt
	door een CONTINUE met een invoerwaarde.

	Als de variabele ERRACT bestaat, en er treedt een foutconditie op,
	dan wordt de inhoud van die variabele uitgevoerd als
	instructielijst. Meestal krijgt ERRACT de waarde [PAUZE], zodat er
	een interactieve pauze betreden wordt als er een fout optreedt. Dit
	stelt de gebruiker in staat om waarden van lokale variabelen te
	controleren op het moment van de fout.

	Het typen van het systeem-stopteken (alt-S voor wxWidgets; anders
	normaal control-\ voor Unix, control-W voor DOS, of command-komma
	voor Mac) betreedt ook een pauze.

CONTINUE waarde
CO waarde
(CONTINUE)
(CO)

	commando. Beëindigt de huidige interactieve pauze, en gaat terug
	naar de context van de PAUZE-aanroep die 'm begon. Als CONTINUE een
	invoerwaarde meekrijgt, wordt die waarde gebruikt als de uitvoer
	van de PAUZE. Zo niet, dan geeft de PAUZE geen waarde terug.

	Bij wijze van uitzondering kan het CONTINUE-commando gebruikt worden
	zonder de standaardinvoerwaarde en zonder haakjes, mits er niets op
	volgt in de instructieregel.

WACHT tijd

	commando. Vertraagt verdere uitvoering met "tijd" zestigsten van een
	seconde. Zorgt er ook voor dat eventuele gebufferde tekens bestemd
	voor de terminal onmiddellijk geprint worden. WACHT 0 kan gebruikt
	worden om deze bufferleging te bereiken zonder daadwerkelijk te
	wachten.

TOTZIENS

	commando. Verlaat Logo; keert terug naar het besturingssysteem.

.MAYBEOUTPUT waarde					(speciale vorm)

	werkt als UITVOER, behalve dat de expressie die de invoerwaarde
	levert misschien geen waarde teruggeeft, in welk geval het effect
	is als STOP. Dit is bedoeld voor gebruik in controlestructuur-
	definities, voor gevallen waarin je niet weet of een expressie een
	waarde oplevert of niet. Voorbeeld:

		leer aanroep :functie [:invoerwaarden] 2
		.maybeoutput apply :functie :invoerwaarden
		eind

		? (aanroep "printperregel "a "b "c)
		a b c
		? printperregel (aanroep "woord "a "b "c)
		abc

	Dit is een alternatief voor RUNRESULT. Het is snel en makkelijk in
	gebruik, ten koste van een uitzondering op Logo's evaluatieregels.
	(Normaliter zou het een fout moeten zijn als de expressie die
	verondersteld wordt een invoerwaarde te leveren geen waarde heeft.)

GOTO woord

	commando. Zoekt naar een TAG-commando met dezelfde invoerwaarde in
	dezelfde procedure, en gaat verder met het draaien van de procedure
	vanaf de locatie van die TAG. Het is betekenisloos om GOTO buiten een
	procedure te gebruiken.

TAG gequote.woord

	commando. Doet niets. De invoerwaarde moet een letterlijk woord zijn
	na een aanhalingsteken ("), niet het resultaat van een berekening.
	Tags worden gebruikt door het GOTO-commando.

IGNORE waarde						(bibliotheekprocedure)

	commando. Doet niets. Wordt gebruikt wanneer een expressie
	geëvalueerd wordt voor een neveneffect en de daadwerkelijke waarde
	onbelangrijk is.

` lijst							(bibliotheekprocedure)

	geeft een lijst terug die gelijk is aan de invoerwaarde, maar met
	bepaalde vervangingen. Als een element van de invoerlijst het woord
	"," (komma) is, dan moet het volgende element een instructielijst
	zijn die een uitvoerwaarde oplevert als die gedraaid wordt. Die
	uitvoerwaarde vervangt de komma en de instructielijst. Als een
	element van de invoerlijst het woord ",@" (komma-apenstaartje) is,
	dan moet het volgende element een instructielijst zijn die een lijst
	teruggeeft als die gedraaid wordt. De elementen van die lijst
	vervangen de ,@ en de instructielijst. Voorbeeld:

		laatzien `[foo baz ,[me [a b c]] garply ,@[me [a b c]]]

	print

		[foo baz [b c] garply b c]

	Een woord dat begint met , of ,@ wordt behandeld alsof de rest van
	het woord een lijst van één woord was, bv. ,:FOO is gelijk aan
	,[:FOO].

	Een woord dat begint met ", (quote komma) of :, (dubbele-punt komma)
	wordt een woord dat begint met " of :, maar waarbij het resultaat
	van het uitvoeren van de vervanging (of het eerste woord ervan, als
	het resultaat een lijst is) komt te staan voor wat na de komma
	stond.

	Backquotes kunnen genest worden. Vervanging gebeurt alleen voor
	komma's op dezelfde diepte als de backquote waarin ze voorkomen:

		? laatzien `[a `[b ,[1+2] ,[foo ,[1+3] d] e] f]
		[a ` [b , [1+2] , [foo 4 d] e] f]

		?maak "naam1 "x
		?maak "naam2 "y
		? laatzien `[a `[b ,:,:naam1 ,",:naam2 d] e]
		[a ` [b , [:x] , ["y] d] e]

FOR forcontrole instructielijst			(bibliotheekprocedure)

	commando. De eerste invoerwaarde moet een lijst zijn met drie of vier
	elementen: (1) een woord, dat gebruikt wordt als naam van een lokale
	variabele; (2) een woord of lijst dat als door DOE geëvalueerd wordt
	om een getal te bepalen, de beginwaarde van de variabele; (3) een
	woord of lijst dat geëvalueerd wordt om een getal te bepalen, de
	grenswaarde van de variabele; (4) een optioneel woord of lijst dat
	geëvalueerd wordt om de stapgrootte te bepalen. Als het vierde
	element ontbreekt, is de stapgrootte 1 of -1, afhankelijk van of de
	grenswaarde respectievelijk groter of kleiner is dan de beginwaarde.

	De tweede invoerwaarde is een instructielijst. Het effect van FOR is
	om die instructielijst herhaaldelijk uit te voeren, waarbij elke
	keer een nieuwe waarde aan de controlevariabele (die genoemd wordt
	door het eerste element van de forcontrole-lijst) toegekend wordt.
	Eerst wordt de beginwaarde aan de controlevariabele toegekend. Dan
	wordt de waarde vergeleken met de grenswaarde. FOR is klaar wanneer
	het teken van (huidig - grens) hetzelfde is als het teken van de
	stapgrootte. (Als er geen expliciete stapgrootte opgegeven is, wordt
	de instructielijst altijd minstens één keer uitgevoerd. Een
	expliciete stapgrootte kan leiden tot een FOR met nul herhalingen,
	bv. FOR [I 1 0 1] ...) Anders wordt de instructielijst uitgevoerd,
	dan wordt de stap opgeteld bij de huidige waarde van de
	controlevariabele, en gaat FOR terug naar de vergelijkingsstap.

		? for [i 2 7 1.5] [print :i]
		2
		3.5
		5
		6.5
		?

DO.WHILE instructielijst whexpressie			(bibliotheekprocedure)

	commando. Evalueert de "instructielijst" herhaaldelijk zolang de
	geëvalueerde "whexpressie" WELWAAR blijft. Evalueert eerst de eerste
	invoerwaarde, dus de "instructielijst" wordt altijd minstens één
	keer uitgevoerd. De "whexpressie" moet een expressielijst zijn
	waarvan de waarde bij evaluatie WELWAAR of NIETWAAR is.

WHILE whexpressie instructielijst			(bibliotheekprocedure)

	commando. Evalueert de "instructielijst" herhaaldelijk zolang de
	geëvalueerde "whexpressie" WELWAAR blijft. Evalueert eerst de eerste
	invoerwaarde, dus de "instructielijst" wordt misschien wel helemaal
	niet uitgevoerd. De "whexpressie" moet een expressielijst zijn
	waarvan de waarde bij evaluatie WELWAAR of NIETWAAR is.

DO.UNTIL instructielijst whexpressie			(bibliotheekprocedure)

	commando. Evalueert de "instructielijst" herhaaldelijk zolang de
	geëvalueerde "whexpressie" NIETWAAR blijft. Evalueert eerst de
	eerste invoerwaarde, dus de "instructielijst" wordt altijd minstens
	één keer uitgevoerd. De "whexpressie" moet een expressielijst zijn
	waarvan de waarde bij evaluatie WELWAAR of NIETWAAR is.

UNTIL whexpressie instructielijst			(bibliotheekprocedure)

	commando. Evalueert de "instructielijst" herhaaldelijk zolang de
	geëvalueerde "whexpressie" NIETWAAR blijft. Evalueert eerst de
	eerste invoerwaarde, dus de "instructielijst" wordt misschien wel
	helemaal niet uitgevoerd. De "whexpressie" moet een expressielijst
	zijn waarvan de waarde bij evaluatie WELWAAR of NIETWAAR is.

CASE waarde clausules					(bibliotheekprocedure)

	commando of operatie. De tweede invoerwaarde is een lijst van
	lijsten (clausules); elke clausule is een lijst waarvan het eerste
	element óf een lijst met waarden óf het woord ELSE is, en waarvan de
	mineerste een Logo-expressie of -instructie is. CASE onderzoekt de
	clausules op volgorde. Als een clausule begint met het woord ELSE
	(hoofd- of kleine letters), dan wordt de mineerste van die clausule
	geëvalueerd en geeft CASE de waarde ervan terug, indien aanwezig.
	Als de eerste invoerwaarde van CASE een element is van het eerste
	element van een clausule, dan wordt de mineerste van die clausule
	geëvalueerd en geeft CASE de waarde ervan terug, indien aanwezig.
	Als geen van deze voorwaarden voldaan is, gaat CASE door naar de
	volgende clausule. Als geen enkele clausule voldoet, doet CASE
	niets. Voorbeeld:

		leer klinker? :letter
		uitvoer case :letter [ [[a e i o u] "true] [else "false] ]
		eind

COND clausules						(bibliotheekprocedure)

	commando of operatie. De invoerwaarde is een lijst van lijsten
	(clausules); elke clausule is een lijst waarvan het eerste element
	óf een expressie is waarvan de waarde WELWAAR of NIETWAAR is, óf het
	woord ELSE, en waarvan de mineerste een Logo-expressie of -instructie
	is. COND onderzoekt de clausules op volgorde. Als een clausule
	begint met het woord ELSE (hoofd- of kleine letters), dan wordt de
	mineerste van die clausule geëvalueerd en geeft CASE de waarde ervan
	terug, indien aanwezig. Anders wordt het eerste element van de
	clausule geëvalueerd; de resulterende waarde moet WELWAAR of
	NIETWAAR zijn. Als die WELWAAR is, dan wordt de mineerste van die
	clausule geëvalueerd en geeft COND de waarde ervan terug, indien
	aanwezig. Als de waarde NIETWAAR is, gaat COND door naar de volgende
	clausule. Als geen enkele clausule voldoet, doet COND niets.
	Voorbeeld:

		leer evens :getallen	; selecteer even getallen uit een lijst
		uv cond [ [[leeg? :getallen] []]
		          [[evenp eerste :getallen]  ; ervan uitgaand dat EVENP gedefinieerd is
		           plaatsvoor eerste :getallen evens mineerste :getallen]
		          [else evens mineerste :getallen] ]
		eind


SJABLOONGEBASEERDE ITERATIE
------------------------------

De procedures in deze sectie zijn iteratie-hulpmiddelen gebaseerd op het
idee van een "sjabloon." Dit is een veralgemenisering van een
instructielijst of expressielijst waarin "sleuven" beschikbaar zijn
waarin het hulpmiddel wisselende gegevens invoegt. Er zijn vier
verschillende vormen van sjabloon te gebruiken.

De meest gebruikte vorm voor een sjabloon is de "expliciete-sleuf"-vorm,
of "vraagteken"-vorm. Voorbeeld:

	? laatzien map [? * ?] [2 3 4 5]
	[4 9 16 25]
	?

In dit voorbeeld evalueerde het MAP-hulpmiddel het sjabloon [? * ?]
herhaaldelijk, waarbij telkens één van de elementen van de gegevenslijst
[2 3 4 5] werd ingevuld voor de vraagtekens. Dezelfde waarde werd gebruikt
voor elk vraagteken binnen één evaluatie. Sommige hulpmiddelen staan toe
dat meer dan één gegeven parallel ingevuld wordt; in die gevallen worden
de sleuven aangeduid met ?1 voor het eerste gegeven, ?2 voor het tweede,
enzovoort:

	? laatzien (map [(woord ?1 ?2 ?1)] [a b c] [d e f])
	[ada beb cfc]
	?

Als het sjabloon het gegevensnummer wil berekenen, is de vorm (? 1) gelijk
aan ?1, dus (? ?1) betekent het gegeven waarvan het nummer in gegeven
nummer 1 staat. Sommige hulpmiddelen staan extra sleufaanduidingen toe,
zoals getoond in de afzonderlijke beschrijvingen.

De tweede vorm van sjabloon is de "benoemde-procedure"-vorm. Als het
sjabloon een woord is in plaats van een lijst, wordt het opgevat als de
naam van een procedure. Die procedure moet een aantal invoerwaarden
accepteren gelijk aan het aantal parallelle gegevenssleuven dat het
hulpmiddel levert; de procedure wordt op alle beschikbare gegevens
toegepast, op volgorde. Dat wil zeggen, als gegevens ?1 tot en met ?3
beschikbaar zijn, is het sjabloon "PROC gelijk aan [PROC ?1 ?2 ?3].

	? laatzien (map "woord [a b c] [d e f])
	[ad be cf]
	?

	leer dotprod :a :b	; inproduct van vectoren
	uv apply "telop (map "vermenigvuldig :a :b)
	eind

De derde vorm van sjabloon is de "benoemde-sleuf"- of "lambda"-vorm. Deze
vorm wordt aangeduid door een sjabloonlijst met meer dan één element,
waarvan het eerste element zelf een lijst is. Het eerste element wordt
opgevat als een lijst met namen; er worden lokale variabelen aangemaakt
met die namen, die op volgorde de beschikbare gegevens als waarde
krijgen. Het aantal namen moet gelijk zijn aan het aantal beschikbare
gegevens. Deze vorm is vooral nodig wanneer het ene iteratie-hulpmiddel
gebruikt moet worden binnen de sjabloonlijst van een ander, en de
?-notatie dubbelzinnig zou zijn in het binnenste sjabloon. Voorbeeld:

	leer matmul :m1 :m2 [:tm2 transpose :m2]	; vermenigvuldig twee matrices
	uitvoer map [[rij] map [[kol] dotprod :rij :kol] :tm2] :m1
	eind

De vierde vorm is "procedure-tekst"-vorm, een variant van de lambda-vorm.
In deze vorm bevat de sjabloonlijst minstens twee elementen, die allemaal
lijsten zijn. Dit is de vorm die gebruikt wordt door de primitieven
DEFINIEER en PRINTTEKST, en APPLY accepteert deze zodat de tekst van een
gedefinieerde procedure als sjabloon gebruikt kan worden.

Let op: de vierde vorm van sjabloon wordt anders geïnterpreteerd dan de
andere, doordat Logo het beschouwt als een onafhankelijke gedefinieerde
procedure voor de doeleinden van UITVOER en STOP. Bijvoorbeeld, de
volgende twee instructies zijn identiek:

	? printperregel apply [[x] :x+3] [5]
	8
	? printperregel apply [[x] [uitvoer :x+3]] [5]
	8

hoewel de eerste instructie in benoemde-sleuf-vorm staat en de tweede in
procedure-tekst-vorm. De benoemde-sleuf-vorm kan begrepen worden als het
Logo vertellen om de expressie :x+3 te evalueren in plaats van de hele
aanroep van apply, waarbij de variabele x tijdelijk de waarde 5 krijgt. De
procedure-tekst-vorm kan begrepen worden als het aanroepen van de
procedure

	leer foo :x
	uitvoer :x+3
	eind

met invoerwaarde 5, maar zonder de procedure daadwerkelijk een naam te
geven. Als het gebruik van UITVOER in deze twee voorbeelden verwisseld
werd, zouden we fouten krijgen:

	? printperregel apply [[x] uitvoer :x+3] [5]
	Ik kan uitvoer alleen binnen een procedure gebruiken
	? printperregel apply [[x] [:x+3]] [5]
	Je zegt niet wat er met 8 moet gebeuren

De benoemde-sleuf-vorm kan gebruikt worden met STOP of UITVOER binnen een
procedure, om de omvattende procedure te stoppen.


De volgende iteratie-hulpmiddelen zijn uitgebreide versies van die in
Appendix B van het boek _Computer_Science_Logo_Style,_Volume_3:_
_Advanced_Topics_ van Brian Harvey [MIT Press, 1987]. De uitbreidingen
zijn vooral om een variabel aantal invoerwaarden mogelijk te maken.


APPLY sjabloon invoerlijst

	commando of operatie. Voert het "sjabloon" uit, waarbij de sleuven
	gevuld worden met de elementen van "invoerlijst." Het aantal
	elementen in "invoerlijst" moet een toegestaan aantal sleuven zijn
	voor "sjabloon." Het is niet toegestaan om het primitief LEER als
	sjabloon toe te passen, maar al het andere mag wel. APPLY geeft terug
	wat "sjabloon" teruggeeft, indien van toepassing.

INVOKE sjabloon invoerwaarde				(bibliotheekprocedure)
(INVOKE sjabloon invoerwaarde1 invoerwaarde2 ...)

	commando of operatie. Precies zoals APPLY, behalve dat de
	invoerwaarden als afzonderlijke expressies geleverd worden in plaats
	van in een lijst.

FOREACH gegevens sjabloon				(bibliotheekprocedure)
(FOREACH gegevens1 gegevens2 ... sjabloon)

	commando. Evalueert de sjabloonlijst herhaaldelijk, één keer voor elk
	element van de gegevenslijst. Als er meer dan één gegevenslijst
	meegegeven is, moeten ze allemaal even lang zijn. (De
	gegevens-invoerwaarden mogen woorden zijn, in welk geval het
	sjabloon één keer per teken geëvalueerd wordt.)

	In een sjabloon stelt het symbool ?REST het deel van de
	gegevens-invoerwaarde voor rechts van het element dat op dat moment
	als ?-sleufvuller gebruikt wordt. Dat wil zeggen, als de
	gegevens-invoerwaarde [A B C D E] is en het sjabloon geëvalueerd
	wordt met ? vervangen door B, dan zou ?REST vervangen worden door
	[C D E]. Als er meerdere parallelle sleuven gebruikt worden, dan
	hoort (?REST 1) bij ?1, enzovoort.

	In een sjabloon stelt het symbool # de positie voor in de
	gegevens-invoerwaarde van het element dat op dat moment als
	?-sleufvuller gebruikt wordt. Dat wil zeggen, als de
	gegevens-invoerwaarde [A B C D E] is en het sjabloon geëvalueerd
	wordt met ? vervangen door B, dan zou # vervangen worden door 2.

MAP sjabloon gegevens					(bibliotheekprocedure)
(MAP sjabloon gegevens1 gegevens2 ...)

	geeft een woord of lijst terug, afhankelijk van het type van de
	gegevens-invoerwaarde, van dezelfde lengte als die
	gegevens-invoerwaarde. (Als er meer dan één gegevens-invoerwaarde
	meegegeven is, heeft de uitvoer hetzelfde type als gegevens1.) Elk
	element van de uitvoer is het resultaat van het evalueren van de
	sjabloonlijst, waarbij de sleuven gevuld worden met de bijbehorende
	elementen van de gegevens-invoerwaarde(n). (Alle gegevens-
	invoerwaarden moeten even lang zijn.) In het geval van een
	woord-uitvoer moeten de resultaten van de sjabloonevaluatie woorden
	zijn, en worden ze samengevoegd met WOORD.

	In een sjabloon stelt het symbool ?REST het deel van de
	gegevens-invoerwaarde voor rechts van het element dat op dat moment
	als ?-sleufvuller gebruikt wordt. Dat wil zeggen, als de
	gegevens-invoerwaarde [A B C D E] is en het sjabloon geëvalueerd
	wordt met ? vervangen door B, dan zou ?REST vervangen worden door
	[C D E]. Als er meerdere parallelle sleuven gebruikt worden, dan
	hoort (?REST 1) bij ?1, enzovoort.

	In een sjabloon stelt het symbool # de positie voor in de
	gegevens-invoerwaarde van het element dat op dat moment als
	?-sleufvuller gebruikt wordt. Dat wil zeggen, als de
	gegevens-invoerwaarde [A B C D E] is en het sjabloon geëvalueerd
	wordt met ? vervangen door B, dan zou # vervangen worden door 2.

MAP.SE sjabloon gegevens				(bibliotheekprocedure)
(MAP.SE sjabloon gegevens1 gegevens2 ...)

	geeft een lijst terug gevormd door de sjabloonlijst herhaaldelijk te
	evalueren en de resultaten samen te voegen met SAMEN. Dat wil
	zeggen, de elementen van de uitvoer zijn de elementen van de
	resultaten van de evaluaties. De uitvoerlijst kan daarom van een
	andere lengte zijn dan de gegevens-invoerwaarde(n). (Als het
	resultaat van een evaluatie de lege lijst is, draagt het niets bij
	aan de uiteindelijke uitvoer.) De gegevens-invoerwaarden mogen
	woorden of lijsten zijn.

	In een sjabloon stelt het symbool ?REST het deel van de
	gegevens-invoerwaarde voor rechts van het element dat op dat moment
	als ?-sleufvuller gebruikt wordt. Dat wil zeggen, als de
	gegevens-invoerwaarde [A B C D E] is en het sjabloon geëvalueerd
	wordt met ? vervangen door B, dan zou ?REST vervangen worden door
	[C D E]. Als er meerdere parallelle sleuven gebruikt worden, dan
	hoort (?REST 1) bij ?1, enzovoort.

	In een sjabloon stelt het symbool # de positie voor in de
	gegevens-invoerwaarde van het element dat op dat moment als
	?-sleufvuller gebruikt wordt. Dat wil zeggen, als de
	gegevens-invoerwaarde [A B C D E] is en het sjabloon geëvalueerd
	wordt met ? vervangen door B, dan zou # vervangen worden door 2.

FILTER whsjabloon gegevens				(bibliotheekprocedure)

	geeft een woord of lijst terug, afhankelijk van het type van de
	gegevens-invoerwaarde, met een subset van de elementen (voor een
	lijst) of tekens (voor een woord) van de invoerwaarde. Het sjabloon
	wordt één keer per element of teken van de gegevens geëvalueerd, en
	moet een WELWAAR- of NIETWAAR-waarde opleveren. Als de waarde
	WELWAAR is, wordt het bijbehorende invoerbestanddeel in de uitvoer
	opgenomen.

		? print filter "vowelp "elephant
		eea
		?

	In een sjabloon stelt het symbool ?REST het deel van de
	gegevens-invoerwaarde voor rechts van het element dat op dat moment
	als ?-sleufvuller gebruikt wordt. Dat wil zeggen, als de
	gegevens-invoerwaarde [A B C D E] is en het sjabloon geëvalueerd
	wordt met ? vervangen door B, dan zou ?REST vervangen worden door
	[C D E].

	In een sjabloon stelt het symbool # de positie voor in de
	gegevens-invoerwaarde van het element dat op dat moment als
	?-sleufvuller gebruikt wordt. Dat wil zeggen, als de
	gegevens-invoerwaarde [A B C D E] is en het sjabloon geëvalueerd
	wordt met ? vervangen door B, dan zou # vervangen worden door 2.

FIND whsjabloon gegevens				(bibliotheekprocedure)

	geeft het eerste bestanddeel van de gegevens-invoerwaarde terug (het
	eerste element van een lijst, of het eerste teken van een woord)
	waarvoor de waarde die het evalueren van het sjabloon met dat
	bestanddeel in de sleuf oplevert WELWAAR is. Als er geen zo'n
	bestanddeel is, wordt de lege lijst teruggegeven.

	In een sjabloon stelt het symbool ?REST het deel van de
	gegevens-invoerwaarde voor rechts van het element dat op dat moment
	als ?-sleufvuller gebruikt wordt. Dat wil zeggen, als de
	gegevens-invoerwaarde [A B C D E] is en het sjabloon geëvalueerd
	wordt met ? vervangen door B, dan zou ?REST vervangen worden door
	[C D E].

	In een sjabloon stelt het symbool # de positie voor in de
	gegevens-invoerwaarde van het element dat op dat moment als
	?-sleufvuller gebruikt wordt. Dat wil zeggen, als de
	gegevens-invoerwaarde [A B C D E] is en het sjabloon geëvalueerd
	wordt met ? vervangen door B, dan zou # vervangen worden door 2.

REDUCE sjabloon gegevens				(bibliotheekprocedure)

	geeft het resultaat terug van het toepassen van het sjabloon om de
	elementen van de gegevens-invoerwaarde te accumuleren. Het sjabloon
	moet een functie met twee sleuven zijn. Meestal is het een
	associatieve functienaam zoals TELOP. Als de gegevens-invoerwaarde
	maar één bestanddeel heeft (element in een lijst of teken in een
	woord), is de uitvoer dat bestanddeel. Anders wordt het sjabloon
	eerst toegepast met ?1 gevuld met het voorlaatste bestanddeel en ?2
	met het laatste bestanddeel. Dan, als er meer bestanddelen zijn,
	wordt het sjabloon toegepast met ?1 gevuld met het volgende
	bestanddeel naar links en ?2 met het resultaat van de vorige
	evaluatie. Dit proces gaat door totdat alle bestanddelen gebruikt
	zijn. De gegevens-invoerwaarde mag niet leeg zijn.

	Let op: als het sjabloon, zoals TELOP, de naam is van een procedure
	die willekeurig veel invoerwaarden kan accepteren, is het efficiënter
	om APPLY te gebruiken in plaats van REDUCE. Dat laatste is geschikt
	voor associatieve procedures die geschreven zijn om precies twee
	invoerwaarden te accepteren:

		leer max :a :b
		uitvoer alsanders :a > :b [:a] [:b]
		eind

		printperregel reduce "max [...]

	Als alternatief kan REDUCE gebruikt worden om MAX te schrijven als
	een procedure die elk aantal invoerwaarden accepteert, zoals TELOP
	dat doet:

		leer max [:invoerwaarden] 2
		als leeg? :invoerwaarden ~
		   [(gooi "error [niet genoeg invoer voor max])]
		uitvoer reduce [alsanders ?1 > ?2 [?1] [?2]] :invoerwaarden
		eind

CROSSMAP sjabloon lijstvanlijsten			(bibliotheekprocedure)
(CROSSMAP sjabloon gegevens1 gegevens2 ...)

	geeft een lijst terug met de resultaten van sjabloonevaluaties. Elke
	gegevenslijst levert een sleuf in het sjabloon; het aantal sleuven is
	gelijk aan het aantal gegevenslijst-invoerwaarden. Als bijzonder
	geval geldt: als er maar één gegevenslijst-invoerwaarde meegegeven
	is, wordt die lijst opgevat als een lijst van gegevenslijsten, en
	levert elk van de elementen ervan waarden voor een sleuf. CROSSMAP
	verschilt van MAP doordat het, in plaats van elementen parallel uit
	de gegevens-invoerwaarden te nemen, alle mogelijke combinaties van
	elementen van de gegevens-invoerwaarden neemt, die niet even lang
	hoeven te zijn.

		? laatzien (crossmap [woord ?1 ?2] [a b c] [1 2 3 4])
		[a1 a2 a3 a4 b1 b2 b3 b4 c1 c2 c3 c4]
		?

	Voor compatibiliteit met de versie in de eerste editie van CSLS mogen
	CROSSMAP-sjablonen de notatie :1 gebruiken in plaats van ?1 om
	sleuven aan te duiden.

CASCADE eindtest sjabloon startwaarde			(bibliotheekprocedure)
(CASCADE eindtest sj1 sw1 sj2 sw2 ...)
(CASCADE eindtest sj1 sw1 sj2 sw2 ... eindsjabloon)

	geeft het resultaat terug van het herhaaldelijk toepassen van een
	sjabloon (of meerdere sjablonen, zoals hieronder uitgelegd), waarbij
	een opgegeven waarde de sleuf vult de eerste keer, en het resultaat
	van elke toepassing de sleuf vult voor de volgende toepassing.

	In het eenvoudigste geval heeft CASCADE drie invoerwaarden. De
	tweede invoerwaarde is een expressiesjabloon met één sleuf. Dat
	sjabloon wordt een aantal keren geëvalueerd (misschien nul). Bij de
	eerste evaluatie wordt de sleuf gevuld met de derde invoerwaarde; bij
	latere evaluaties wordt de sleuf gevuld met het resultaat van de
	vorige evaluatie. Het aantal evaluaties wordt bepaald door de eerste
	invoerwaarde. Dat mag óf een niet-negatief geheel getal zijn, in
	welk geval het sjabloon dat aantal keren geëvalueerd wordt, óf een
	predicaat-expressiesjabloon, in welk geval het herhaaldelijk
	geëvalueerd wordt (met dezelfde sleufvuller die gebruikt zal worden
	voor de evaluatie van de tweede invoerwaarde), en de
	CASCADE-evaluatie doorgaat zolang de predicaatwaarde NIETWAAR is.
	(Met andere woorden, het predicaatsjabloon geeft de stopvoorwaarde
	aan.)

	Als het sjabloon nul keer geëvalueerd wordt, is de uitvoer van
	CASCADE de derde invoerwaarde (startwaarde). Anders is de uitvoer de
	waarde die geproduceerd is door de laatste sjabloonevaluatie.

	CASCADE-sjablonen mogen het symbool # bevatten om het aantal keren
	voor te stellen dat het sjabloon geëvalueerd is. Deze sleuf wordt
	gevuld met 1 voor de eerste evaluatie, 2 voor de tweede, enzovoort.

		? laatzien cascade 5 [plaatserachter # ?] []
		[1 2 3 4 5]
		? laatzien cascade [vowelp eerste ?] [me ?] "spring
		ing
		? laatzien cascade 5 [# * ?] 1
		120
		?

	Meerdere gecascadeerde resultaten kunnen parallel berekend worden
	door extra sjabloon-startwaarde-paren als invoerwaarden aan CASCADE
	mee te geven. In dat geval hebben alle sjablonen (inclusief het
	eindtest-sjabloon, indien gebruikt) meerdere sleuven, met een aantal
	sleuven gelijk aan het aantal invoerwaardeparen. In elke evaluatieronde
	stelt ?2, bijvoorbeeld, het resultaat voor van het evalueren van het
	tweede sjabloon in de vorige ronde. Als het totale aantal
	invoerwaarden (inclusief de eerste eindtest-invoerwaarde) oneven is,
	dan is de uitvoer van CASCADE de uiteindelijke waarde van het eerste
	sjabloon. Als het totale aantal invoerwaarden even is, dan is de
	laatste invoerwaarde een sjabloon dat één keer geëvalueerd wordt,
	nadat aan de eindtest voldaan is, om de uitvoer van CASCADE te
	bepalen.

		leer fibonacci :n
		uitvoer (cascade :n [?1 + ?2] 1 [?1] 0)
		eind

		leer varkenslatijn :woord
		uitvoer (cascade [vowelp eerste ?] ~
				[woord me ? eerste ?] ~
				:woord ~
				[woord ? "ay])
		eind

CASCADE.2 eindtest sj1 sw1 sj2 sw2			(bibliotheekprocedure)

	geeft het resultaat terug van het aanroepen van CASCADE met dezelfde
	invoerwaarden. Het enige verschil is dat het standaardaantal
	invoerwaarden vijf is in plaats van drie.

TRANSFER eindtest sjabloon inmand			(bibliotheekprocedure)

	geeft het resultaat terug van herhaalde evaluatie van het sjabloon.
	Het sjabloon wordt één keer geëvalueerd voor elk element van de
	lijst "inmand." TRANSFER houdt een "uitmand" bij die aanvankelijk de
	lege lijst is. Na elke evaluatie van het sjabloon wordt de
	resulterende waarde de nieuwe uitmand.

	In het sjabloon stelt het symbool ?IN het huidige element uit de
	inmand voor; het symbool ?OUT stelt de hele huidige uitmand voor.
	Andere sleufsymbolen mogen niet gebruikt worden.

	Als de eerste (eindtest-)invoerwaarde een lege lijst is, gaat de
	evaluatie door totdat alle elementen van de inmand gebruikt zijn. Zo
	niet, dan moet de eerste invoerwaarde een predicaat-
	expressiesjabloon zijn, en gaat de evaluatie door totdat óf de
	waarde van dat sjabloon WELWAAR is, óf de inmand op is.


MACRO'S
=========

.MACRO procnaam :invoer1 :invoer2 ...				(speciale vorm)
.DEFMACRO procnaam tekst

	Een macro is een speciaal soort procedure waarvan de uitvoer
	geëvalueerd wordt als Logo-instructies in de context van de
	aanroeper van de macro. .MACRO is precies zoals LEER, behalve dat de
	nieuwe procedure een macro wordt; .DEFMACRO is precies zoals
	DEFINIEER, met dezelfde uitzondering.

	Macro's zijn nuttig om nieuwe controlestructuren te bedenken die
	vergelijkbaar zijn met HERHAAL, ALS, enzovoort. Zulke
	controlestructuren kunnen bijna, maar niet helemaal, nagebootst
	worden door gewone Logo-procedures. Hier is bijvoorbeeld een gewone
	procedure-versie van HERHAAL:

		leer mijn.herhaal :getal :instructies
		als :getal=0 [stop]
		doe :instructies
		mijn.herhaal :getal-1 :instructies
		eind

	Deze versie werkt prima voor de meeste doeleinden, bv.,

		mijn.herhaal 5 [printperregel "hallo]

	Maar het werkt niet als de uit te voeren instructies UITVOER, STOP,
	of LOKAAL bevatten. Bekijk bijvoorbeeld deze procedure:

		leer voorbeeld
		printperregel [Raad mijn geheime woord. Je krijgt drie pogingen.]
		herhaal 3 [printnaarov "|?? | ~
			  als leeswoord = "geheim [pr "Juist! stop]]
		printperregel [Helaas, het woord was "geheim"!]
		eind

	Deze procedure werkt zoals geschreven, maar als MIJN.HERHAAL gebruikt
	wordt in plaats van HERHAAL, werkt het niet, omdat de STOP dan
	MIJN.HERHAAL zou stoppen in plaats van VOORBEELD, zoals gewenst.

	De oplossing is om van MIJN.HERHAAL een macro te maken. In plaats van
	de berekening daadwerkelijk uit te voeren, moet een macro een lijst
	teruggeven met Logo-instructies. De inhoud van die lijst wordt
	geëvalueerd alsof die op de plek van de aanroep van de macro stond.
	Hier is een macroversie van HERHAAL:

		.macro mijn.herhaal :getal :instructies
		als :getal=0 [uitvoer []]
		uitvoer samen :instructies ~
				(lijst "mijn.herhaal :getal-1 :instructies)
		eind

	Elke macro is een operatie -- het moet altijd iets teruggeven. Zelfs
	in het basisgeval geeft MIJN.HERHAAL een lege instructielijst terug.
	Om te laten zien hoe MIJN.HERHAAL werkt, nemen we het voorbeeld

		mijn.herhaal 5 [printperregel "hallo]

	Voor dit voorbeeld geeft MIJN.HERHAAL de instructielijst terug

		[printperregel "hallo mijn.herhaal 4 [printperregel "hallo]]

	Logo voert deze instructies dan uit in plaats van de oorspronkelijke
	aanroep van MIJN.HERHAAL; dit print "hallo" één keer en roept een
	volgende herhaling aan.

	De zojuist getoonde techniek, hoewel redelijk makkelijk te begrijpen,
	heeft het nadeel dat het traag is, omdat elke herhaling een
	instructielijst moet opbouwen om te evalueren. Een andere aanpak is
	om van MIJN.HERHAAL een macro te maken die precies zo werkt als de
	niet-macroversie, tenzij de te herhalen instructies UITVOER of STOP
	bevatten:

		.macro mijn.herhaal :getal :instructies
		vang "herhaal.vanglabel ~
		      [op herhaal.klaar runresult [herhaal1 :getal :instructies]]
		op []
		end

		leer herhaal1 :getal :instructies
		als :getal=0 [gooi "herhaal.vanglabel]
		doe :instructies
		.maybeoutput herhaal1 :getal-1 :instructies
		eind

		leer herhaal.klaar :herhaal.resultaat
		als leeg? :herhaal.resultaat [op [stop]]
		op lijst "uitvoer quoted eerste :herhaal.resultaat
		eind

	Als de instructies geen STOP of UITVOER bevatten, dan bereikt
	HERHAAL1 zijn basisgeval en roept het GOOI aan. Als resultaat geeft
	MIJN.HERHAAL's laatste instructieregel een lege lijst terug, dus de
	evaluatie van het macroresultaat door de aanroeper doet niets. Maar
	als er een STOP of UITVOER gebeurt, dan geeft HERHAAL.KLAAR een
	STOP- of UITVOER-instructie terug die uitgevoerd wordt in de context
	van de aanroeper.

	De macro-definiërende commando's hebben namen die met een punt
	beginnen, omdat macro's een geavanceerde functie van Logo zijn; het
	is makkelijk om in de problemen te komen door een macro te
	definiëren die niet stopt, of door de instructielijst niet correct
	op te bouwen.

	Lisp-gebruikers moeten opmerken dat Logo-macro's GEEN speciale
	vormen zijn. Dat wil zeggen, de invoerwaarden voor de macro worden
	normaal geëvalueerd, zoals bij elke andere Logo-procedure. Het is
	alleen de uitvoer van de macro die ongebruikelijk behandeld wordt.

	Hier is nog een voorbeeld:

		.macro localmake :naam :waarde
		output (list "local		~
			     word "" :naam	~
			     "apply		~
			     ""maak		~
			     (list :naam :waarde))
		end

	Het wordt zo gebruikt:

		leer probeer
		localmake "garply "hallo
		printperregel :garply
		eind

	LOCALMAKE geeft de lijst terug

		[local "garply apply "maak [garply hallo]]

	De reden voor het gebruik van APPLY is om te voorkomen dat beslist
	moet worden of de tweede invoerwaarde van MAAK een aanhalingsteken
	ervoor nodig heeft. (In dit geval zou dat wel zo zijn -- MAAK
	"GARPLY "HALLO -- maar het aanhalingsteken zou fout zijn als de
	waarde een lijst was.)

	Het is vaak handig om de `-functie te gebruiken om de
	instructielijst op te bouwen:

		.macro localmake :naam :waarde
		op `[local ,[word "" :naam] apply "maak [,[:naam] ,[:waarde]]]
		end

	Aan de andere kant is ` behoorlijk traag, omdat het boomrecursief is
	en in Logo geschreven.

MACRO? naam

	geeft WELWAAR terug als de invoerwaarde de naam van een macro is.

MACROEXPAND expressie					(bibliotheekprocedure)

	neemt als invoerwaarde een Logo-expressie die een macro aanroept
	(d.w.z. die begint met de naam van een macro) en geeft de
	Logo-expressie terug waarin de macro de invoerexpressie zou
	vertalen.

		.macro localmake :naam :waarde
		op `[local ,[word "" :naam] apply "maak [,[:naam] ,[:waarde]]]
		end

		? laatzien macroexpand [localmake "pi 3.14159]
		[local "pi apply "maak [pi 3.14159]]


FOUTAFHANDELING
==================

Als er een fout optreedt, doorloopt Logo de volgende stappen. Eerst, als
er een beschikbare variabele bestaat met de naam ERRACT, neemt Logo de
waarde ervan als instructielijst en voert de instructies uit. De operatie
FOUT mag (één keer) binnen de instructies gebruikt worden om de
foutconditie te onderzoeken. Als de instructielijst PAUZE aanroept, wordt
de foutmelding geprint voordat de pauze plaatsvindt. Bepaalde fouten zijn
"herstelbaar"; voor zo'n fout geldt dat, als de instructielijst een waarde
teruggeeft, die waarde gebruikt wordt in plaats van de expressie die de
fout veroorzaakte. (Als ERRACT PAUZE aanroept en de gebruiker vervolgens
CONTINUE met een invoerwaarde aanroept, wordt die invoerwaarde de uitvoer
van PAUZE en dus de uitvoer van de ERRACT-instructielijst.)

Het is mogelijk dat een ERRACT-instructielijst een ongeschikte waarde
oplevert, of geen waarde waar er wel één nodig is. Daardoor zou dezelfde
foutconditie door dit mechanisme voor altijd kunnen terugkeren. Om dat
gevaar te vermijden: als dezelfde foutconditie twee keer achter elkaar
optreedt vanuit een ERRACT-instructielijst zonder gebruikersinteractie,
wordt het bericht "Erract-lus" geprint en gaat de besturing terug naar het
topniveau. "Zonder gebruikersinteractie" betekent dat als ERRACT PAUZE
aanroept en de gebruiker een onjuiste waarde geeft, dit lus-preventie-
mechanisme niet in werking treedt en de gebruiker het opnieuw mag
proberen.

Tijdens het uitvoeren van de ERRACT-instructielijst is ERRACT lokaal
ongebonden, zodat een fout in de ERRACT-instructies zelf geen lus
veroorzaakt. Met name veroorzaakt een fout tijdens een pauze geen
pauze-binnen-een-pauze, tenzij de gebruiker tijdens de pauze de waarde
[PAUZE] opnieuw aan ERRACT toekent. Maar zo'n fout keert niet terug naar
het topniveau; die blijft binnen de oorspronkelijke pauzelus.

Als er geen beschikbare ERRACT-waarde is, behandelt Logo de fout door
intern een GOOI "ERROR te genereren. (Een gebruikersprogramma kan ook zelf
opzettelijk een foutconditie genereren door GOOI aan te roepen.) Als deze
gooi niet gevangen wordt door een VANG "ERROR in het gebruikersprogramma,
wordt die uiteindelijk gevangen, hetzij door de topniveau-instructielus,
hetzij door een pauzelus, die de foutmelding print. Een aanroep van VANG
"ERROR in een gebruikersprogramma ontbindt ERRACT lokaal, zodat het effect
is dat welke van ERRACT en VANG "ERROR het meest lokaal is, voorrang
krijgt.

Als er een zwevendekomma-overloop optreedt tijdens een rekenkundige
bewerking, of een wiskundige functie met twee invoerwaarden (zoals MACHT)
wordt aangeroepen met een ontoelaatbare combinatie van invoerwaarden, dan
verwijst het bericht "vindt ... geen geldige invoer" naar de tweede
operand, maar moet dat opgevat worden als de combinatie.


FOUTCODES
-----------

Hier zijn de numerieke codes die verschijnen als eerste element van de
lijst die FOUT teruggeeft wanneer een fout gevangen wordt, met de
bijbehorende berichten. Sommige berichten hebben mogelijk twee
verschillende codes, afhankelijk van of de fout herstelbaar is (d.w.z. of
er een vervangende waarde geleverd kan worden via het ERRACT-mechanisme)
in de specifieke context. Sommige berichten zijn waarschuwingen in plaats
van fouten; die worden niet gevangen. Fouten 0 en 32 zijn zo ernstig dat
Logo onmiddellijk afsluit.

  0	Fatale interne fout (kan niet gevangen worden)
  1	Geen geheugen meer
  2	Stapeloverloop
  3	Schildpad buiten beeld
  4	PROC vindt GEGEVEN geen geldige invoer (niet herstelbaar)
  5	PROC leverde geen uitvoer aan PROC
  6	Niet genoeg invoer voor PROC
  7	PROC vindt GEGEVEN geen geldige invoer (herstelbaar)
  8	Te veel binnen ()'s
  9	Je zegt niet wat er met GEGEVEN moet gebeuren
 10	')' niet gevonden
 11	VAR heeft geen waarde
 12	Onverwachte ')'
 13	Ik weet niet hoe ik PROC moet uitvoeren (herstelbaar)
 14	Kan geen catch-merkteken vinden voor GOOILABEL
 15	PROC is al gedefinieerd
 16	Gestopt
 17	Al aan het meeschrijven (dribble)
 18	Bestandssysteemfout
 19	Aangenomen dat je ALSANDERS bedoelt, niet ALS (alleen waarschuwing)
 20	VAR overschaduwd door lokale variabele in procedure-aanroep (alleen waarschuwing)
 21	Throw "Fout
 22	PROC is een primitief
 23	Kan LEER niet gebruiken binnen een procedure
 24	Ik weet niet hoe ik PROC moet uitvoeren (niet herstelbaar)
 25	ALSWAAR/ALSNIETWAAR zonder TEST
 26	Onverwachte ']'
 27	Onverwachte '}'
 28	Kon grafische omgeving niet initialiseren
 29	Macro gaf WAARDE terug in plaats van een lijst
 30	Je zegt niet wat er met WAARDE moet gebeuren
 31	Kan alleen STOP of UITVOER binnen een procedure gebruiken
 32	APPLY vindt SLECHTDING geen geldige invoer
 33	EIND midden in een meerregelige instructie
 34	Echt geen geheugen meer (kan niet gevangen worden)
 35	door de gebruiker gegenereerd foutbericht (GOOI "ERROR [bericht])
 36	EIND midden in een meerregelige instructie
 37	Foutieve standaardwaarde-expressie voor optionele invoer: EXPR
 38	Kan UITVOER of STOP niet gebruiken binnen RUNRESULT
 39 	Aangenomen dat je 'VT 100' bedoelt, niet VT100 (of vergelijkbaar)
 40	Ik kan bestand BESTANDSNAAM niet openen
 41	Bestand BESTANDSNAAM is al open
 42	Bestand BESTANDSNAAM is niet open
 43	Opdrachtenlijst [EXPR EXPR] bevat meer dan één expressie.


SPECIALE VARIABELEN
======================

Logo onderneemt speciale actie als een van de volgende variabelenamen
bestaat. Ze volgen de normale scoperegels, dus een procedure kan er
lokaal één instellen om het bereik van het effect te beperken.
Aanvankelijk bestaat geen enkele variabele, behalve ALLOWGETSET,
CASEIGNOREDP en UNBURYONEDIT, die WELWAAR en begraven zijn.

ALLOWGETSET						(variabele)

	als WELWAAR, geeft dit aan dat een poging om een niet-bestaande
	procedure te gebruiken opgevat moet worden als een impliciete getter-
	of setterprocedure (setter als de eerste drie letters van de naam
	SET zijn) voor een variabele met dezelfde naam (zonder de SET, waar
	van toepassing).

BUTTONACT						(variabele)

	als niet leeg, moet dit een instructielijst zijn die geëvalueerd
	wordt elke keer dat een muisknop ingedrukt wordt. Let op dat de
	gebruiker de knop misschien al losgelaten heeft voordat de
	instructies geëvalueerd worden. BUTTON geeft nog steeds terug welke
	knop het laatst ingedrukt is. CLICKPOS geeft de positie van de
	muiscursor terug op het moment dat de knop ingedrukt werd; dit kan
	afwijken van MOUSEPOS als de gebruiker de muis beweegt na het
	klikken.

	Let op dat het mogelijk is dat de gebruiker een knop indrukt tijdens
	het evalueren van de instructielijst. Als dit je programma in de
	war zou brengen, voorkom dit door BUTTONACT tijdelijk op de lege
	lijst te zetten. Een makkelijke manier om dat te doen is de
	volgende:

		maak "buttonact [knop.actie]

		leer knop.actie [:buttonact []]
		... ; wat je ook wil dat de knop doet
		eind

CASEIGNOREDP						(variabele)

	als WELWAAR, geeft dit aan dat kleine letters en hoofdletters als
	gelijk beschouwd moeten worden door GELIJK?, BEFORE?, ERBIJ?, etc.
	Logo maakt deze variabele aanvankelijk WELWAAR, en begraaft 'm.

COMMANDLINE						(variabele)

	bevat alle tekst die na een liggend streepje op de commandoregel
	staat waarmee Logo gestart is.

ERRACT							(variabele)

	een instructielijst die uitgevoerd wordt in het geval van een fout.
	Heeft meestal de waarde [PAUZE] om interactief debuggen mogelijk te
	maken.

FULLPRINTP						(variabele)

	als WELWAAR, dan worden woorden die gemaakt zijn met een backslash of
	verticale streep (om tekens op te nemen die anders niet als deel van
	een woord behandeld zouden worden) geprint mét de backslashes of
	verticale strepen zichtbaar, zodat het geprinte resultaat door Logo
	teruggelezen kan worden om dezelfde waarde te produceren. Als
	FULLPRINTP WELWAAR is, print het lege woord (hoe het ook gemaakt is)
	als ||. (Anders print het als helemaal niets.)

KEYACT							(variabele)

	als niet leeg, moet dit een instructielijst zijn die geëvalueerd
	wordt elke keer dat een toets op het toetsenbord ingedrukt wordt. De
	instructielijst kan LEESTEKEN gebruiken om te achterhalen welke
	toets ingedrukt is. Let op dat alleen toetsen die tekens opleveren
	meetellen; alleen SHIFT of CONTROL indrukken zorgt er niet voor dat
	KEYACT geëvalueerd wordt.

	Let op dat het mogelijk is dat de gebruiker een toets indrukt
	tijdens het evalueren van de instructielijst. Als dit je programma
	in de war zou brengen, voorkom dit door KEYACT tijdelijk op de lege
	lijst te zetten. Een makkelijke manier om dat te doen is de
	volgende:

		maak "keyact [toets.actie]

		leer toets.actie [:keyact []]
		... ; wat je ook wil dat de toets doet
		eind

LOADNOISILY						(variabele)

	als WELWAAR, print dit de namen van procedures die gedefinieerd
	worden tijdens het laden vanuit een bestand (inclusief het
	tijdelijke bestand dat door BEWERK gemaakt is).

PRINTDEPTHLIMIT						(variabele)

	als een niet-negatief geheel getal, geeft dit de maximale diepte aan
	van sublijststructuur die geprint wordt door PRINTPERREGEL, etc.

PRINTWIDTHLIMIT						(variabele)

	als een niet-negatief geheel getal, geeft dit het maximumaantal
	elementen aan in elke lijst die geprint wordt door PRINTPERREGEL,
	etc.

REDEFP							(variabele)

	als WELWAAR, staat dit toe dat primitieven gewist (WIS) of
	geherdefinieerd (COPYDEF) worden.

STARTUP							(variabele)

	als hier een lijstwaarde aan toegekend wordt in een bestand dat door
	OPEN geladen wordt, wordt die waarde na het laden uitgevoerd als
	instructielijst.

UNBURYONEDIT						(variabele)

	als WELWAAR, zorgt dit ervoor dat elke procedure die gedefinieerd
	wordt tijdens BEWERK of OPEN uit zijn begraven staat gehaald wordt,
	zodat die door een latere OPSLAAN opgeslagen wordt. Bestanden die
	procedures willen definiëren én begraven, moeten dat in die volgorde
	doen.

USEALTERNATENAMES					(variabele)

	als WELWAAR, zorgt dit ervoor dat Logo niet-Engelse woorden genereert
	(uit het Messages-bestand) in plaats van TRUE, FALSE, END, etc.


Logo biedt de volgende begraven variabelen die door programma's gebruikt
kunnen worden:

LOGOVERSION						(variabele)

	een reëel getal dat het Logo-versienummer aangeeft, bv. 5.5

LOGOPLATFORM						(variabele)

	één van de volgende woorden: wxWidgets, X11, Windows, of
	Unix-Nographics.


INTERNATIONALISATIE
======================

Berkeley Logo heeft beperkte ondersteuning voor niet-Engelstalige
gebruikers. Helaas is er geen Unicode-ondersteuning, en ASCII-codes met de
hoge bit aan werken in sommige contexten wel en in andere niet.

Als je Berkeley Logo wil vertalen voor gebruik met een andere taal, zijn
er drie hoofdzaken die je moet doen:
	1. Primitieve namen
	2. Foutmeldingen (en andere berichten)
	3. Documentatie

Voor primitieve namen is het makkelijkst om een opstartbestand te leveren
dat aliassen definieert voor de Engelse primitieve namen, met COPYDEF:
	COPYDEF "VOORUIT "FORWARD
Dit zou meestal genoeg moeten zijn, tenzij de naam in jouw taal voor de
ene primitief net zo gespeld wordt als de Engelse naam van een andere
primitief. In dat geval moet je REDEFP aanzetten en ervoor zorgen dat je
de niet-botsende naam kopieert vóórdat je de botsende overschrijft!

"Primitieven" die eigenlijk in de Logo-bibliotheek zitten, kunnen
natuurlijk gewoon vervangen of aangevuld worden met Logo-procedures en
bestandsnamen in de eigen taal.

Logo-programma's zullen er natuurlijk nog steeds niet uitzien als je
eigen taal als de woordvolgorde drastisch anders is, vooral als je
werkwoorden niet vóór hun lijdend voorwerp zet.

Voor foutmeldingen is er een bestand met de naam Messages in de
logolib-map, met teksten van berichten, één per regel. Je kan dit
vervangen door een bestand voor je eigen taal. Voeg geen regels toe,
verwijder ze niet, en verander de volgorde niet; Logo vindt berichten op
basis van regelnummer. De reeksen %p, %s en %t in deze berichten stellen
variabele delen van het bericht voor en mogen niet vertaald worden. (%p
PRINT het variabele deel, terwijl %s het LAATZIEN -- dat wil zeggen, het
verschil zit 'm erin of er wel of geen haken rond een lijst getoond
worden. %t betekent dat het variabele deel een C-tekenreeks is in plaats
van een Logo-object.) Als je de volgorde van twee variabele delen wil
veranderen (geen enkel herordenbaar bericht heeft er meer dan twee), zou
je bijvoorbeeld de regel
	%p vindt %s geen geldige invoer
vervangen door
	%+s is een waardeloze invoer voor %p
Het plusteken vertelt de berichtenprinter om de volgorde om te draaien; je
moet de volgorde van %p en %s, als beide gebruikt worden, ook omdraaien
om overeen te komen. Het plusteken komt direct na het eerste
procentteken in het bericht, wat niet per se aan het begin van de regel
hoeft te staan. De reeks \n in een bericht stelt een regeleinde voor; laat
je niet voor de gek houden door te denken dat de "n" deel is van het
volgende woord.

Sommige berichten komen twee keer voor in het bestand; dat is geen fout.
De twee spaties vóór "hoe" in "Ik weet niet  hoe" zijn ook geen fout. Het
bericht dat alleen "%p" bevat is voor door de gebruiker geleverde
foutmeldingen in GOOI "ERROR. Het bericht "  in %s\n%s" is het deel van
alle foutmeldingen dat aangeeft waar de fout optrad als die binnen een
procedure was; je wil misschien het woord "in" naar je eigen taal
veranderen. "%s gedefinieerd\n" is wat OPEN print voor elke gedefinieerde
procedure als de variabele LOADNOISILY WELWAAR is. "to %p\nend\n\n" is
wat BEWERK in het tijdelijke bestand zet als je vraagt om een procedure
te bewerken die nog niet bestaat.

Ook in het Messages-bestand staan regels die elk maar één woord bevatten;
het eerste daarvan is het woord "true". Sommige van deze woorden worden
door Logo herkend in gebruikersinvoer; sommige worden door Logo
gegenereerd; sommige allebei. Bijvoorbeeld, de woorden TRUE en FALSE
worden herkend als booleaanse waarden door ALS en ALSANDERS, en worden
ook door Logo gegenereerd als uitvoer van de primitieve predicaten zoals
GELIJK?. Het woord END wordt herkend als het einde van een
procedure-definitie, en kan gegenereerd worden wanneer Logo een
procedure-body reconstrueert voor PO of BEWERK. Ik heb in deze alinea
hoofdletters gebruikt voor het leesgemak, maar de woorden in het
Messages-bestand moeten in kleine letters staan.

Als je deze vervangt door niet-Engelse woorden, zal Logo zowel de Engelse
namen als je alternatieve namen *herkennen*. Als je bijvoorbeeld het woord
"true" vervangt door "welwaar", dan begrijpt Logo beide:
	ALS "TRUE [PRINTPERREGEL "JA]
	ALS "WELWAAR [PRINTPERREGEL "JA]

De variabele UseAlternateNames bepaalt of Logo andere-taal-namen zal
*genereren* -- bijvoorbeeld, of predicaatfuncties de andere-taal-varianten
van TRUE en FALSE teruggeven. Deze variabele is standaard NIETWAAR, wat
betekent dat de Engelse woorden gegenereerd worden.

Je wilt misschien dat Engels-benoemde predicaatfuncties Engels TRUE en
FALSE genereren, terwijl andere-taal-benoemde predicaten de alternatieve
woorden genereren. Dit kan gedaan worden door UseAlternateNames op
NIETWAAR te laten staan, en in plaats van de andere-taal-predicaten met
COPYDEF te definiëren, het zo te doen:

	to french.boolean :bool
	if equalp :bool "true [output "vrai]
	if equalp :bool "false [output "faux]
	output :bool	; zou niet moeten gebeuren
	end

	to make.french.predicate :french :english :arity
	define :french `[[[inputs] ,[:arity]]
	                 [output french.boolean
				    apply ,[word "" :english] :inputs]]
	end

	? make.french.predicate "egal? "equal? 2
	? pr egal? 3 4
	faux
	? pr egal? 4 4
	vrai
	? pr equal? 3 4
	false
	? pr equal? 4 4
	true

De derde invoerwaarde van make.french.predicate is het aantal
invoerwaarden dat het predicaat verwacht. Deze oplossing is niet helemaal
perfect, omdat de infix-predicaten (=, <, >) nog steeds in het Engels
zullen printen. Als je wil dat die andere-taal-woorden genereren, zet dan
in plaats daarvan UseAlternateNames op WELWAAR.

Sommige van de woorden in dit deel van het Messages-bestand zijn namen
van Logo-primitieven (OUTPUT, STOP, GOTO, TAG, IF, IFELSE, TO, .MACRO).
Om deze namen te vertalen, moet je COPYDEF gebruiken zoals eerder
beschreven, naast het veranderen van de namen in Messages. Wees
consistent in deze twee stappen. Vergeet de punt in ".macro" niet!

Voor documentatie zijn er twee soorten: deze handleiding en de
hulpbestanden. Die laatste worden automatisch gegenereerd uit deze
handleiding als je een Unix-systeem hebt, dus in dat geval hoef je alleen
deze handleiding te vertalen, met behoud van het formaat. (De
automatische hulpbestandengenerator let op dingen als hoofdletters,
tabs, streepjes en is-gelijk-tekens aan het begin van regels.) Het
programma makehelp.c heeft mogelijk aanpassing nodig omdat een paar van
de primitieve namen bijzondere gevallen zijn (bv. LOG10 is de enige naam
met cijfers erin).

Als je geen Unix-hulpmiddelen hebt, kan je elk hulpbestand afzonderlijk
vertalen. Een punt in een primitieve naam wordt in de bestandsnaam
voorgesteld als een D; er zijn geen bestanden voor vraagtekens, omdat het
HELP-commando zoekt naar het bestand genoemd naar de bijbehorende
primitief die eindigt op P.
