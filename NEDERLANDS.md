# Nederlandse commando's in UCBLogo

Deze fork voegt Nederlandse aliassen toe aan UCBLogo — puur additief, alle
Engelse commando's blijven gewoon werken. Bron: [SuperLogo](https://archive.org/details/superlogo-voor-windows)
(1994, Addo Stuur / A.W. Bruna Informatica), aangevuld waar nodig.

Voor volledig Nederlandse foutmeldingen en systeemteksten: gebruik de
losse **UCBLogo-NL**-app (gebruikt `logolib/Messages.nl`) i.p.v. de gewone
UCBLogo-app, en zet `usealternatenames` aan: `make "usealternatenames "true`.

## Procedures & besturing

| Nederlands | Engels |
|---|---|
| `leer ... eind` | `to ... end` |
| `als` | `if` |
| `alsanders` | `ifelse` |
| `alswaar` | `iftrue` (`ift`) |
| `alsnietwaar` | `iffalse` (`iff`) |
| `herhaal` | `repeat` |
| `telherhaal` | `repcount` |
| `uitvoer` / `uv` | `output` / `op` |
| `stop` | `stop` *(zelfde woord)* |
| `wacht` | `wait` |
| `totziens` | `bye` |
| `gooi` | `throw` |
| `vang` | `catch` |
| `bewerk` / `bw` | `edit` / `ed` |
| `laad` | `load` |
| `maak` | `make` |
| `lokaal` | `local` |

## Turtle & tekenscherm

| Nederlands | Engels |
|---|---|
| `vooruit` / `vt` | `forward` / `fd` |
| `achteruit` / `at` | `back` / `bk` |
| `rechts` / `re` | `right` / `rt` |
| `links` / `li` | `left` / `lt` |
| `penneer` / `pn` | `pendown` / `pd` |
| `penop` / `pp` | `penup` / `pu` |
| `penkleur` | `pencolor` / `pc` |
| `zetpenkleur` | `setpencolor` |
| `zetpendikte` | `setpensize` |
| `zetrichting` / `zr` | `setheading` / `seth` |
| `zetpos` | `setpos` |
| `zetx` / `zety` / `zetxy` | `setx` / `sety` / `setxy` |
| `positie` | `pos` |
| `wegturtle` / `wegt` | `hideturtle` / `ht` |
| `laatzien` / `lz` | `showturtle` / `st` |
| `naarbegin` / `nb` | `home` |
| `wegtekening` | `clean` *(wist tekening, laat turtle staan)* |
| `wisscherm` / `ws` | `clearscreen` / `cs` *(wist tekening + turtle naar huis)* |
| `wisalles` | `erall` *(wist procedures/variabelen)* |

## Data: lijsten, woorden, getallen

| Nederlands | Engels |
|---|---|
| `ding` | `thing` |
| `eerste` / `laatste` | `first` / `last` |
| `element` | `item` |
| `lijst` / `lijst?` | `list` / `list?` |
| `woord` / `woord?` | `word` / `word?` |
| `samen` / `sa` | `sentence` / `se` |
| `erbij?` | `member?` |
| `leeg?` | `empty?` |
| `gelijk?` | `equal?` |
| `kleiner?` / `groter?` | `less?` / `greater?` |
| `getal?` | `number?` |
| `niet` / `of` / `en` | `not` / `or` / `and` |
| `gok` | `random` |
| `telop` | `sum` |
| `vermenigvuldig` | `product` |
| `deeldoor` | `quotient` |
| `verschil` | `difference` |
| `macht` | `power` |
| `wortel` | `sqrt` |
| `sinus` / `cosinus` / `arctangens` | `sin` / `cos` / `arctan` |
| `afronding` | `round` |
| `mod` | `modulo` |

## In-/uitvoer

| Nederlands | Engels |
|---|---|
| `print` | `print` *(zelfde woord)* |
| `leesteken` | `readchar` |
| `leeswoord` / `lw` | `readword` / `rw` |
| `leeslijst` / `ll` | `readlist` / `rl` |
| `toets?` | `key?` |

## Niet (bewust) vertaald

- **`doe`** en **`kom`** — mogelijke extra structuurwoorden uit Comenius
  Logo (waar SuperLogo op gebaseerd is), maar geen UCBLogo-equivalent
  gevonden dat er zeker genoeg bij past.
- **`open`** — in SuperLogo hoort dit eerder bij `openread`-achtige
  bestand-als-stream-commando's dan bij `load`.
