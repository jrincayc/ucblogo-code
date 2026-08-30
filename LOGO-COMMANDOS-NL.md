# Nederlandse Logo-commando's — uitleg

Alle Nederlandse commando's uit deze UCBLogo-fork, met een korte uitleg wat
ze doen. `:naam` betekent "de waarde van variabele naam" (net als in het
Engels). Vetgedrukt = hoofdcommando, ernaast de korte vorm. Bron: de
officiële SuperLogo-woordenlijst (zie `NEDERLANDS.md` voor details).

## Schildpad bewegen

**`vooruit :afstand`** (`vt`)
Beweegt de schildpad `:afstand` stappen vooruit, in de richting waarin hij op dat moment kijkt.

**`achteruit :afstand`** (`at`)
Beweegt de schildpad `:afstand` stappen achteruit — precies de tegenovergestelde richting van waar hij naar kijkt.

**`rechts :graden`** (`re`)
Draait de schildpad `:graden` graden met de klok mee (draait alleen, beweegt niet).

**`links :graden`** (`li`)
Draait de schildpad `:graden` graden tegen de klok in.

**`naar :positie`**
Geeft niet zelf een draai, maar berekent welke kijkrichting nodig zou zijn om recht naar `:positie` (een lijst `[x y]`) te kijken — handig te combineren met `zetrichting`.

**`graden`**
Geeft de huidige kijkrichting van de schildpad terug (in graden, 0-360).

**`naarbegin`** (`nb`)
Zet de schildpad terug naar het midden van het scherm, kijkrichting naar boven. Hetzelfde als `zetpos [0 0]` gevolgd door `zetrichting 0`.

**`zetrichting :graden`** (`zr`)
Draait de schildpad naar een vaste, absolute kijkrichting (0 = boven, 90 = rechts, enz.) — in tegenstelling tot `rechts`/`links`, die relatief draaien.

**`zetpos :lijst`**
Verplaatst de schildpad naar een absolute positie. `:lijst` is een lijst van twee getallen: `[x y]`.

**`zetx :x`** / **`zety :y`** / **`zetxy :x :y`**
Verplaatst de schildpad naar een nieuwe x-coördinaat, y-coördinaat, of allebei tegelijk.

**`positie`**
Geeft de huidige positie van de schildpad terug, als lijst `[x y]`.

**`rand`**
Zet de schildpad in "rand"-stand (de standaardstand): loopt hij van de ene kant van het scherm af, dan verschijnt hij aan de overkant weer.

**`venster`**
Zet de schildpad in "venster"-stand: hij mag oneindig ver buiten het zichtbare scherm bewegen (geen wraparound, geen begrenzing) — het zichtbare scherm is dan maar een deel van een oneindig groot tekenvlak.

## Pen (het "potlood" waarmee de schildpad tekent)

**`penneer`** (`pn`)
Zet de pen neer — de schildpad tekent nu een lijn terwijl hij beweegt.

**`penop`** (`pp`)
Haalt de pen omhoog — de schildpad beweegt zonder te tekenen.

**`pk`**
Geeft het huidige kleurnummer van de pen terug.

**`zetpenkleur :kleur`**
Verandert de penkleur naar het opgegeven kleurnummer.

**`zetpendikte :dikte`**
Verandert hoe dik de getekende lijn is.

**`gum`**
Zet de pen in "gum"-stand: alles waar de schildpad nu overheen beweegt wordt **gewist** in plaats van getekend.

**`penomgekeerd`**
Zet de pen in "omgekeerd"-stand: nogmaals over een al getekende lijn gaan wist die lijn weer weg (werkt niet op elk systeem hetzelfde).

**`vul`**
Vult het gesloten gebied waar de schildpad nu in staat met de huidige penkleur — het gebied moet wel al door eerder getekende lijnen omsloten zijn.

## Turtle-zichtbaarheid

**`wegturtle`** (`wegt`)
Maakt de schildpad onzichtbaar (het icoontje) — het tekenen zelf blijft gewoon werken.

**`kom`**
Maakt de schildpad weer zichtbaar — het tegenovergestelde van `wegturtle`.

**`zichtbaar?`**
Geeft `waar` terug als de schildpad op dit moment zichtbaar is.

**`laatzien`** (`lz`)
**Let op: dit is geen turtle-commando!** Werkt als `print`, maar bij een lijst als invoer blijven de blokhaken zichtbaar in de uitvoer (`print [1 2]` toont `1 2`, `laatzien [1 2]` toont `[1 2]`).

## Scherm

**`wegtekening`**
Wist alle getekende lijnen, maar de schildpad blijft op zijn huidige plek en richting staan.

**`wistekening`** (`wt`)
Wist alle getekende lijnen **én** zet de schildpad terug naar het midden — combinatie van `wegtekening` + `naarbegin`, een "verse start".

**`wisopdracht`** (`wo`)
Wist het tekst-/opdrachtvenster (waar je typt en de uitvoer ziet) — laat de tekening met rust.

**`wisalles`**
Wist alle eigen procedures en variabelen uit het geheugen (niet het tekenscherm!) — voorzichtig mee, dit ben je eigen `leer`-definities kwijt.

**`opdrachtvenster`** (`ov`)
Maakt het tekstvenster zo groot mogelijk (verbergt het tekenvenster grotendeels).

**`splitsvenster`**
Verdeelt het scherm zodat zowel het tekstvenster als het grootste deel van het tekenvenster zichtbaar blijven.

**`achtergrond`** (`ag`)
Geeft het huidige achtergrondkleurnummer van het tekenscherm terug.

**`zetachtergrond`** (`zag`)
Verandert de achtergrondkleur van het tekenscherm.

**`zetkleurov`**
Verandert de tekst- en achtergrondkleur van het tekst-/opdrachtvenster zelf (alleen in de wxWidgets-versie van UCBLogo).

**`zetcursor :positie`**
Verplaatst de tekstcursor naar een positie in het tekstvenster (`:positie` is een lijst `[x y]`, linksboven = `[0 0]`).

## Een procedure maken (je eigen commando's bouwen)

**`leer naam :param1 :param2 ... eind`**
Begint de definitie van een nieuwe, zelfgemaakte procedure met de gegeven naam en eventuele parameters. Alles wat je daarna typt hoort bij die procedure, tot je `eind` typt.

**`definieer "naam :tekstlijst`**
Doet hetzelfde als `leer ... eind`, maar dan in één keer via een lijst i.p.v. interactief typen — handig als je een procedure programmatisch wilt opbouwen.

**`definitie? "naam`**
Geeft `waar` terug als `naam` een bestaande, zelfgemaakte procedure is.

**`primitief? "naam`**
Geeft `waar` terug als `naam` een ingebouwd commando is (geen zelfgemaakte procedure).

**`doe :lijst`**
Voert de instructies in `:lijst` direct uit, alsof je ze zelf had getypt. Geeft een waarde terug als de lijst een uitdrukking bevat die iets teruggeeft. Handig om instructies pas op het laatste moment samen te stellen.

**`als :voorwaarde [instructies]`**
Voert de instructies tussen de blokhaken alleen uit als `:voorwaarde` waar is.

**`alsanders :voorwaarde [instructies-indien-waar] [instructies-indien-onwaar]`**
Als `:voorwaarde` waar is: voert het eerste blok uit. Zo niet: voert het tweede blok uit. (Let op: altijd **drie** argumenten nodig, in tegenstelling tot `als`.)

**`alswaar [instructies]`** / **`alsnietwaar [instructies]`**
Voert de instructies alleen uit als de meest recente `TEST`-instructie waar respectievelijk onwaar was.

**`herhaal :aantal [instructies]`**
Voert de instructies tussen de blokhaken `:aantal` keer achter elkaar uit. Het meest gebruikte commando om patronen te tekenen.

**`telherhaal`**
Geeft terug bij welke herhaling je zit binnen de dichtstbijzijnde `herhaal`, te beginnen bij 1.

**`uitvoer :waarde`** (`uv`)
Geeft `:waarde` terug als resultaat van de huidige procedure (vergelijkbaar met "return" in andere talen). Stopt de procedure direct.

**`stop`**
Stopt de huidige procedure direct, zonder een waarde terug te geven.

**`pauze`**
Onderbreekt de uitvoering en geeft je een interactieve prompt binnen de lopende procedure — je kunt daar even rondkijken (lokale variabelen zijn beschikbaar) voordat je verdergaat.

**`fout`**
Geeft een lijst terug met informatie over de laatst opgevangen fout (leeg als er geen fout is geweest sinds de vorige keer dat je dit aanriep).

**`wacht :tijd`**
Wacht `:tijd` zestigste seconden voordat het volgende commando wordt uitgevoerd.

**`bewerk "naam`** (`bw`)
Opent de ingebouwde editor om een bestaande procedure aan te passen (in een tijdelijk bestand).

**`bewerkbestand "bestandsnaam`** (`bb`)
Hetzelfde als `bewerk`, maar dan met een bestand dat je zelf een naam geeft i.p.v. een tijdelijk bestand — handig als je later nog eens hetzelfde bestand wilt bewerken.

**`open "bestandsnaam`** (ook: `laad`)
Leest en voert een Logo-bestand uit (bijv. om eerder opgeslagen procedures weer in te laden).

**`opslaan "bestandsnaam`**
Bewaart al je huidige (niet-verborgen) procedures, variabelen en kenmerklijsten in een bestand.

**`totziens`**
Sluit UCBLogo af.

**`gooi "merk`** / **`vang "merk [instructies]`**
Voor foutafhandeling: `gooi` springt naar de dichtstbijzijnde `vang` met hetzelfde merk-woord; `vang` voert de instructies uit en vangt eventuele `gooi`-sprongen daarbinnen op.

**`verberg :namenlijst`**
Verbergt de opgegeven procedures/variabelen/kenmerklijsten — ze blijven gewoon werken, maar verschijnen niet meer in overzichten.

**`verbergniet :namenlijst`**
Maakt eerder verborgen items weer zichtbaar in overzichten.

**`verborg`**
Geeft een lijst van alle op dit moment verborgen items terug.

**`wis :namenlijst`**
Verwijdert de opgegeven procedures/variabelen/kenmerklijsten helemaal uit het geheugen (niet alleen verbergen — echt weg).

## Gegevens: getallen, woorden en lijsten

**`maak "naam :waarde`**
Maakt een variabele aan (of past 'm aan) met de gegeven naam en waarde. Let op de `"` vóór de naam!

**`lokaal "naam`**
Maakt een variabele die alleen binnen de huidige procedure bestaat.

**`ding "naam`**
Geeft de waarde van de variabele met die naam terug. (`:naam` is eigenlijk een snelle schrijfwijze voor `ding "naam`.)

**`naam? "iets`**
Geeft `waar` terug als `iets` de naam is van een bestaande variabele.

**`namen`**
Geeft een lijst van alle huidige (niet-verborgen) variabelenamen terug.

**`eerste :iets`** / **`laatste :iets`**
Bij een woord: het eerste/laatste letterteken. Bij een lijst: het eerste/laatste element.

**`me`** (ook: `mineerste`)
Geeft alles terug **behalve** het eerste teken (woord) of eerste element (lijst) — het tegenovergestelde stuk van `eerste`.

**`minlaatste`** (`ml`)
Geeft alles terug **behalve** het laatste teken (woord) of laatste element (lijst).

**`element :index :iets`**
Het `:index`-de teken (bij een woord) of element (bij een lijst) — tellend vanaf 1.

**`plaatservoor :iets :lijst`**
Zet `:iets` vooraan `:lijst` (of voor een teken bij een woord).

**`plaatserachter :iets :lijst`**
Zet `:iets` achteraan `:lijst` (of achter een teken bij een woord).

**`lijst :a :b`**
Maakt een nieuwe lijst met `:a` en `:b` als elementen.

**`lijst? :iets`**
Geeft `waar` terug als `:iets` een lijst is, anders `onwaar`.

**`woord :a :b`**
Plakt `:a` en `:b` aan elkaar tot één woord.

**`woord? :iets`**
Geeft `waar` terug als `:iets` een los woord is (geen lijst), anders `onwaar`.

**`samen :a :b`** (`sa`)
Maakt een lijst van `:a` en `:b` — anders dan `lijst`, worden lijst-argumenten "uitgepakt" i.p.v. genest.

**`aantal :iets`**
Geeft het aantal letters (bij een woord) of elementen (bij een lijst) terug.

**`erbij? :item :lijst`**
Geeft `waar` terug als `:item` in `:lijst` voorkomt.

**`leeg? :iets`**
Geeft `waar` terug als `:iets` een leeg woord of lege lijst is.

**`gelijk? :a :b`**
Geeft `waar` terug als `:a` en `:b` gelijk zijn.

**`kleiner? :a :b`** / **`groter? :a :b`**
Vergelijkt twee getallen.

**`getal? :iets`**
Geeft `waar` terug als `:iets` een getal is.

**`niet :voorwaarde`**
Keert `waar`/`onwaar` om.

**`of :a :b`** / **`en :a :b`**
Logische OF / EN van twee (of meer) voorwaarden.

**`neemkenmerk "lijstnaam "kenmerknaam`**
Haalt de waarde van één kenmerk op uit een kenmerklijst.

**`plaatskenmerk "lijstnaam "kenmerknaam :waarde`**
Voegt een kenmerk (naam + waarde) toe aan een kenmerklijst.

**`wiskenmerk "lijstnaam "kenmerknaam`**
Verwijdert één kenmerk uit een kenmerklijst.

**`kenmerklijst "lijstnaam`**
Geeft alle kenmerken (om en om naam, waarde, naam, waarde, ...) van een kenmerklijst terug.

## Rekenen

**`gok :max`**
Geeft een willekeurig geheel getal terug tussen 0 en `:max` (exclusief).

**`telop :a :b`** / **`vermenigvuldig :a :b`** / **`deeldoor :a :b`** / **`verschil :a :b`**
Optellen, vermenigvuldigen, delen, aftrekken.

**`macht :grondtal :exponent`**
`:grondtal` tot de macht `:exponent`.

**`wortel :getal`**
Vierkantswortel.

**`sinus :graden`** / **`cosinus :graden`** / **`arctangens :getal`**
Goniometrische functies, hoek in graden.

**`afronding :getal`**
Rondt af naar het dichtstbijzijnde hele getal.

**`integer :getal`**
Hakt het deel na de komma eraf (naar nul toe afgerond, geen normale afronding).

**`mod :a :b`**
De rest bij deling van `:a` door `:b`.

**`teken :code`**
Geeft het letterteken terug dat bij een ASCII-code (0–255) hoort.

## Invoer/uitvoer

**`print :iets`**
Toont `:iets` op het scherm (zelfde woord als Engels, geen aparte vertaling nodig).

**`printperregel :iets`**
Hetzelfde als `print`.

**`printnaarov :iets`**
Print zoals `print`, maar zonder regeleinde erna en zonder spaties tussen meerdere waarden — handig om tekst "aan elkaar te plakken" op het scherm.

**`printtekst "naam`**
Geeft de tekst van een procedure terug als lijst, in het formaat dat `definieer` verwacht — handig om een procedure programmatisch te kopiëren/aanpassen.

**`printdefinitie :namenlijst`**
Print de volledige definitie van de opgegeven procedures/variabelen/kenmerklijsten op het scherm.

**`leesteken`**
Leest en geeft één letterteken terug dat de gebruiker typt.

**`leeswoord`** (`lw`)
Leest een hele regel in en geeft die terug als één woord.

**`leeslijst`** (`ll`)
Leest een hele regel in en geeft die terug als een lijst van woorden.

**`toets?`**
Geeft `waar` terug als er een toets is ingedrukt die nog niet gelezen is.

## Bestanden

**`wisbestand "bestandsnaam`** (`wb`)
Verwijdert het opgegeven bestand van schijf.

## Voorbeeld: een eenvoudige procedure

```logo
leer vierkant :zijde
herhaal 4 [vooruit :zijde rechts 90]
eind

wistekening
vierkant 100
```

## Voorbeeld: `doe` gebruiken

```logo
maak "opdracht [vooruit 50 rechts 90]
herhaal 4 [doe :opdracht]
```
