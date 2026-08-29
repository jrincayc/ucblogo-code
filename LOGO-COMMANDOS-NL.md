# Nederlandse Logo-commando's — uitleg

Alle Nederlandse commando's uit deze UCBLogo-fork, met een korte uitleg wat
ze doen. `:naam` betekent "de waarde van variabele naam" (net als in het
Engels). Vetgedrukt = hoofdcommando, ernaast de korte vorm.

## Schildpad bewegen

**`vooruit :afstand`** (`vt`)
Beweegt de schildpad `:afstand` stappen vooruit, in de richting waarin hij op dat moment kijkt.

**`achteruit :afstand`** (`at`)
Beweegt de schildpad `:afstand` stappen achteruit — precies de tegenovergestelde richting van waar hij naar kijkt.

**`rechts :graden`** (`re`)
Draait de schildpad `:graden` graden met de klok mee (draait alleen, beweegt niet).

**`links :graden`** (`li`)
Draait de schildpad `:graden` graden tegen de klok in.

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

## Pen (het "potlood" waarmee de schildpad tekent)

**`penneer`** (`pn`)
Zet de pen neer — de schildpad tekent nu een lijn terwijl hij beweegt.

**`penop`** (`pp`)
Haalt de pen omhoog — de schildpad beweegt zonder te tekenen.

**`penkleur`**
Geeft het huidige kleurnummer van de pen terug.

**`zetpenkleur :kleur`**
Verandert de penkleur naar het opgegeven kleurnummer.

**`zetpendikte :dikte`**
Verandert hoe dik de getekende lijn is.

**`wegturtle`** (`wegt`) / **`laatzien`** (`lz`)
Maakt de schildpad onzichtbaar / weer zichtbaar (het tekenen zelf blijft gewoon werken, dit is puur het icoontje).

## Scherm

**`wegtekening`**
Wist alle getekende lijnen, maar de schildpad blijft op zijn huidige plek en richting staan.

**`wisscherm`** (`ws`)
Wist alle getekende lijnen **én** zet de schildpad terug naar het midden — een "verse start".

**`wisalles`**
Wist alle eigen procedures en variabelen uit het geheugen (niet het tekenscherm!) — voorzichtig mee, dit ben je eigen `leer`-definities kwijt.

## Een procedure maken (je eigen commando's bouwen)

**`leer naam :param1 :param2 ... eind`**
Begint de definitie van een nieuwe, zelfgemaakte procedure met de gegeven naam en eventuele parameters. Alles wat je daarna typt hoort bij die procedure, tot je `eind` typt.

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

**`wacht :tijd`**
Wacht `:tijd` zestigste seconden voordat het volgende commando wordt uitgevoerd.

**`bewerk "naam`** (`bw`)
Opent de ingebouwde editor om een bestaande procedure aan te passen.

**`laad "bestandsnaam`**
Leest en voert een Logo-bestand uit (bijv. om eerder opgeslagen procedures weer in te laden).

**`totziens`**
Sluit UCBLogo af.

**`gooi "merk`** / **`vang "merk [instructies]`**
Voor foutafhandeling: `gooi` springt naar de dichtstbijzijnde `vang` met hetzelfde merk-woord; `vang` voert de instructies uit en vangt eventuele `gooi`-sprongen daarbinnen op.

## Gegevens: getallen, woorden en lijsten

**`maak "naam :waarde`**
Maakt een variabele aan (of past 'm aan) met de gegeven naam en waarde. Let op de `"` vóór de naam!

**`lokaal "naam`**
Maakt een variabele die alleen binnen de huidige procedure bestaat.

**`ding "naam`**
Geeft de waarde van de variabele met die naam terug. (`:naam` is eigenlijk een snelle schrijfwijze voor `ding "naam`.)

**`eerste :iets`** / **`laatste :iets`**
Bij een woord: het eerste/laatste letterteken. Bij een lijst: het eerste/laatste element.

**`element :index :iets`**
Het `:index`-de teken (bij een woord) of element (bij een lijst) — tellend vanaf 1.

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

**`mod :a :b`**
De rest bij deling van `:a` door `:b`.

## Invoer/uitvoer

**`print :iets`**
Toont `:iets` op het scherm (zelfde woord als Engels, geen aparte vertaling nodig).

**`leesteken`**
Leest en geeft één letterteken terug dat de gebruiker typt.

**`leeswoord`** (`lw`)
Leest een hele regel in en geeft die terug als één woord.

**`leeslijst`** (`ll`)
Leest een hele regel in en geeft die terug als een lijst van woorden.

**`toets?`**
Geeft `waar` terug als er een toets is ingedrukt die nog niet gelezen is.

## Voorbeeld: een eenvoudige procedure

```logo
leer vierkant :zijde
herhaal 4 [vooruit :zijde rechts 90]
eind

wisscherm
vierkant 100
```
