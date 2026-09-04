# Nederlandse commando's in UCBLogo

Deze fork voegt Nederlandse aliassen toe aan UCBLogo — puur additief, alle
Engelse commando's blijven gewoon werken. Bron: de officiële **"Woordenlijst
van procedures Nederlands-Engels"** uit de SuperLogo-documentatie (1994,
Addo Stuur / A.W. Bruna Informatica, gebaseerd op Comenius Logo — software
gratis te downloaden op [archive.org](https://archive.org/details/superlogo-voor-windows)),
gedecompileerd uit `SLOGO.HLP` met [helpdeco](https://github.com/rofl0r/helpdeco).
Dit is dus geen gok/reconstructie meer, maar de authentieke, complete lijst
uit het product zelf.

Voor volledig Nederlandse foutmeldingen en systeemteksten: gebruik de
losse **UCBLogo-NL**-app (gebruikt `logolib/Messages.nl`) i.p.v. de gewone
UCBLogo-app, en zet `usealternatenames` aan: `make "usealternatenames "true`.

## Procedures & besturing

| Nederlands | Engels |
|---|---|
| `leer ... eind` | `to ... end` |
| `doe` | `run` *(voert een lijst als instructies uit)* |
| `als` | `if` |
| `alswaar` | `iftrue` (`ift`) |
| `alsnietwaar` | `iffalse` (`iff`) |
| `alsanders` | `ifelse` *(niet in de officiële SuperLogo-lijst, eigen toevoeging)* |
| `herhaal` | `repeat` |
| `telherhaal` | `repcount` |
| `uitvoer` / `uv` | `output` / `op` |
| `stop` | `stop` *(zelfde woord)* |
| `wacht` | `wait` |
| `totziens` | `bye` |
| `gooi` | `throw` |
| `vang` | `catch` |
| `bewerk` / `bw` | `edit` / `ed` |
| `bb` / `bewerkbestand` | `editfile` |
| `open` / `laad` | `load` *(`open` is de officiële SuperLogo-naam)* |
| `opslaan` | `save` |
| `maak` | `make` |
| `lokaal` | `local` |
| `definieer` | `define` |
| `definitie?` | `defined?` |
| `primitief?` | `primitive?` |

## Turtle & tekenscherm

| Nederlands | Engels |
|---|---|
| `vooruit` / `vt` | `forward` / `fd` |
| `achteruit` / `at` | `back` / `bk` |
| `rechts` / `re` | `right` / `rt` |
| `links` / `li` | `left` / `lt` |
| `naar` | `towards` |
| `graden` | `heading` |
| `penneer` / `pn` | `pendown` / `pd` |
| `penop` / `pp` | `penup` / `pu` |
| `pk` | `pencolor` |
| `zetpenkleur` | `setpencolor` |
| `zetpendikte` | `setpensize` |
| `penomgekeerd` | `penreverse` |
| `gum` | `penerase` |
| `zetrichting` / `zr` | `setheading` / `seth` |
| `zetpos` | `setpos` |
| `zetx` / `zety` / `zetxy` | `setx` / `sety` / `setxy` |
| `positie` | `pos` |
| `xcoordinaat` | `xcor` |
| `ycoordinaat` | `ycor` |
| `wegturtle` / `wegt` | `hideturtle` / `ht` |
| `kom` | `showturtle` / `st` |
| `laatzien` / `lz` | `show` *(print met behoud van haakjes — géén turtle-commando!)* |
| `zichtbaar?` | `shown?` |
| `naarbegin` / `nb` | `home` |
| `rand` | `wrap` |
| `venster` | `window` |
| `splitsvenster` | `splitscreen` |
| `opdrachtvenster` / `ov` | `textscreen` |
| `wegtekening` | `clean` *(wist tekening, laat turtle staan)* |
| `wistekening` / `wt` | `clearscreen` / `cs` *(wist tekening + turtle naar huis)* |
| `wisopdracht` / `wo` | `cleartext` *(wist het tekst/opdrachtvenster)* |
| `wisalles` | `erall` *(wist procedures/variabelen)* |
| `wis` | `erase` |
| `vul` | `fill` |
| `zag` / `zetachtergrond` | `setbackground` |
| `achtergrond` / `ag` | `background` |
| `teken` | `char` |

## Bestanden

| Nederlands | Engels |
|---|---|
| `wb` / `wisbestand` | `erasefile` |
| `openschrijf` | `openwrite` |
| `openlees` | `openread` |
| `opentoevoeg` | `openappend` |
| `openbijwerk` | `openupdate` |
| `zetschrijf` | `setwrite` |
| `zetlees` | `setread` |
| `sluit` | `close` |
| `zetcursor` | `setcursor` |
| `zetkleurov` | `settc` *(tekstkleur)* |

## Gegevens: getallen, woorden, lijsten, kenmerken

| Nederlands | Engels |
|---|---|
| `ding` | `thing` |
| `eerste` / `laatste` | `first` / `last` |
| `element` | `item` |
| `me` / `mineerste` | `butfirst` *(`bf`)* |
| `minlaatste` / `ml` | `butlast` *(`bl`)* |
| `plaatservoor` | `fput` |
| `plaatserachter` | `lput` |
| `lijst` / `lijst?` | `list` / `list?` |
| `woord` / `woord?` | `word` / `word?` |
| `samen` / `sa` | `sentence` / `se` |
| `erbij?` | `member?` |
| `leeg?` | `empty?` |
| `gelijk?` | `equal?` |
| `kleiner?` / `groter?` | `less?` / `greater?` |
| `getal?` | `number?` |
| `naam?` | `name?` |
| `namen` | `names` |
| `niet` / `of` / `en` | `not` / `or` / `and` |
| `aantal` | `count` |
| `neem` | `ask` |
| `neemkenmerk` | `gprop` |
| `plaatskenmerk` | `pprop` |
| `wiskenmerk` | `remprop` |
| `kenmerklijst` | `plist` |
| `verberg` | `bury` |
| `verbergniet` | `unbury` |
| `verborg` | `buried` |

## Rekenen

| Nederlands | Engels |
|---|---|
| `gok` | `random` |
| `telop` | `sum` |
| `vermenigvuldig` | `product` |
| `deeldoor` | `quotient` |
| `verschil` | `difference` |
| `macht` | `power` |
| `wortel` | `sqrt` |
| `sinus` / `cosinus` / `tangens` / `arctangens` | `sin` / `cos` / `tan` / `arctan` |
| `absolutewaarde` | `abs` |
| `afronding` | `round` |
| `mod` | `modulo` |
| `integer` | `int` |
| `pauze` | `pause` |
| `fout` | `error` |

## In-/uitvoer

| Nederlands | Engels |
|---|---|
| `print` | `print` *(zelfde woord)* |
| `printperregel` | `print` |
| `printtekst` | `text` |
| `printnaarov` | `type` |
| `printdefinitie` | `po` |
| `leesteken` | `readchar` |
| `leeswoord` / `lw` | `readword` / `rw` |
| `leeslijst` / `ll` | `readlist` / `rl` |
| `toets?` | `key?` |

## Foutmeldingen & reservewoorden (alleen met `Messages.nl` / `usealternatenames`)

| Nederlands | Engels |
|---|---|
| `welwaar` / `nietwaar` | `true` / `false` |
| `eind` | `end` |

## Niet vertaald: geen UCBLogo-equivalent

SuperLogo's Windows-schil had een uitgebreidere media/object-API dan
UCBLogo's eenvoudigere turtle-graphics (bitmaps, geluid, aanpasbare
turtle-vormen, een venstersysteem met knoppenbalken). Zo'n 110
SuperLogo-commando's hebben daardoor geen zinnig UCBLogo-doel, bijvoorbeeld:
`openbitmap`, `openafbeelding`, `opslaanbitmap`, `maakturtle`, `zetvorm`,
`speelgeluid`, `speeltoon`, `muis`, `knoppen`, `balkterug`, `zetmuiswijzer`,
`voegtoe` (`append`, bestaat niet als los UCBLogo-commando), `welke` (`who`,
object-georiënteerd, alleen relevant met `--enable-objects`).
