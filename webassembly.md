# UCB Logo Web Edition

In collaboration with Brian Harvey we are proud to present...

_UCB Logo, lovingly packaged as a Progressive Web App_

Run Logo right now in your browser -> https://n1ywb-1.github.io/ucblogo-code/ucblogo.html

Mobile users may select "Add to Home" from the browser menu to install UCB Logo
as a progressive web app, which is recommended.

Features

* OG C UCB Logo interpreter, cross-compiled to Web Assembly
* 100% compatible with UCB Logo; minor exceptions noted
* Responisive, accessible, mobile-friendly HTML UI
* Animated SVG turtle graphics and sprites
* Accessible UI
* Mobile-first design; looks great on the desktop as well
* Full offline capabilities
* Persistant emulated filesystem
* Built-in barebones text editor
* Tail-call elimination for unlimited recursion depth
* JS/Logo interoperability

Supported browsers and platforms (current versions):
* Chrome
* Firefox
* Edge
* Safari
* Node.js


Links:
* UCB Logo Web edition home: https://github.com/n1ywb-1/ucblogo-code
* UCB Logo Home: https://github.com/jrincayc/ucblogo-code/
* UCB Logo Docs: https://people.eecs.berkeley.edu/~bh/v2ch14/manual.html

Test reports and contributions welcome.

Great thanks to Brian Harvey and Joshua Cogliati.

# Introduction

It goes without saying that nearly everybody on earth today has access to a
web browser. The web is the defacto standard computing platform of the 21st
century.

While Logo cousins Snap and Scratch have flourished on the web, Logo itself has
languished somewhat. In my personal quest to hack on Logo programs on my phone I
tried many broken "Logo" apps offering little more power than turtle graphics
and basic loops, or didn't even use Logo syntax, or didn't have map/reduce, or
macros, or tail call eliminaion, etc etc etc. Toy Logos, basically. I wound up
using 1980's vintage LCSI Logos in PC and apple II emulators much to the
detriment of my battery life. It was a fun trip down memory lane but I wanted
much more.

Now thanks to Web Assembly you can run full UCB Logo right in your browser with
beautful SVG turtle graphics, high quality animations, and JavaScript
integration. 

## Genuine article

This is not some retro-nostalgic vibe-coded slop version of Logo. This is Brian
Harvey's very own C-Logo interpreter lovingly ported to Web Assembly with
Emscripten by a 30-year veteran C programmer. Map, reduce, macros, they're all
there. The terminal UI, barebones editor, and turtle graphics are a mix of C,
JS, HTML, SVG, and CSS.

## Mobile-first design

Although UCB Logo Web Edition works great on desktop browsers, most people in
the world don't own a computer, they own a phone. Priority is given to offering
an outstanding mobile user experience. Mobile users need to use their OS native
input widgets to edit tiny text with their big mushy fingers. If you've ever
tried to SSH from a phone you know what I'm talking about. They don't even have
arrow keys.

soft-keys are provided for common punction that's difficult to enter on the
on-screen keyboard.

SVG scales nicely accross the huge range of display resolutions present in
mobile devices.

## More accessible

Further because the UI uses standard HTML and SVG elements it is accessible to
people with visual impairments in a way that Logo has never been. The fonts and
drawings can be blown up to epic proportions without pixelation artifacts and it
should be screen reader compatible. The responsive UI smoothly scales.

Accessiblity is a high priority with this port so feedback is appreciated.

# New features

Web Edition features an exclusive real-time animation facility.

The turtle can be drawn with SVG.

Scalable Vector Graphics provide stunning and responsive turtle art independent
of the native display resolution. SVG turtle art can be easily printed, saved,
shared, or opened in another program.

Use the JS interop to break the limits of logo and manipulate DOM directly or
access cutting edge web APIs.

# Differences and missing features

Pausing is not supported.

The terminal is dumb as a rock, no emulation related words work yet.

The `shell` word doesn't work, but see `runjs`.

Web edition cannot access the host filesystem but includes an internally
emulated filesystem. This allows file-related words to function otherwise
normally.  The emulated filesystem should be persistent in PWA installs so you
can use `edit "mylogoprog` etc to save and load your programs but I wouldn't
100% trust it yet. Logo can also upload and download source code files. 

## xor and erase pens

Historically Logo drew bitmaps. While SVG is vastly superior in nearly every
respect, bitmaps offer some unique capabilities which logo has used to implement
an xor pen and an erasing pen. We can't provide pixel-perfect implementations of
those, but we can get close.

The xor pen is implemented with an SVG xor filter. The result looks similar to
the historical behavior but it's not pixel-perfect.

Erasing is implemented by drawing on a mask layer to reveal the background. So
nothing is really erased. Again it's close but not pixel-perfect.

UCB Logo historically supports a proprietary vector art file format by the
unfortunate name of PICT. It's basically a serialized list of turtle commands.
PICT words might still work but I haven't tried it.

# Usage

## As a web page

UCB Logo can run as a web page in most popular browsers.

## As a progressive web app

UCB Logo can also be installed as an app on most mobile devices by selecting "Add to Home" from your mobile browser menu.

Installed as an app, UCB Logo features full-offline capabilities and persistant storage on a simulated local filesystem.

# Web-specific words

* `animationstart`
* `animationstop`
* `animationframact`
* `animationtime`
* `animationdelay`
* `makesprite`
* `setturtlesprite`
* `setshaperendering`
* `runjs`
* `runLogo` (JS Function)

## Animation words

Exclusive to Web Edition is an animation facility based on the HTMl5
requestAnimationFrame API. This enables Logo to render smooth turtle-graphics
based animations at 60fps or higher on modern devices. Further, because the loop
runs asynchronously via Logo "actions", Logo remains available for live
interactive animation development.

Normally when SVG is animated the browser renders the vectors to bitmaps and
transforms them on the GPU. This is very fast. This also has the side effect of
introducing ugly pixelated visual artifacts in line-art. 

When we use turtle graphics to draw each full animation frame from scratch we
effectively de-optimize the browser's SVG rendering pipeline by forcing the
browser to rerender the SVG for each frame. This improves visual quality at the
cost of somewhat increased CPU usage. 

To set an animation frame action
```
make "animationframeact [words to draw frame]
```

To get the total time the loop has been running
```
:animationtime
```

To get the time since the last frame
```
:animationdelta
```

To start animation loop
```
animationstart
```

To stop animation loop
```
animationstop
```

Example:

```
cs pu
make "animationframeact [setheading towards mousepos fd :animationdelta * 0.2]
animationstart
```

The turtle should start smoothly chasing your mouse pointer around the screen at
a constant speed.

## Sprite words

Right now there is only one sprite, the turtle. So basically this is just an
SVG-based alternative to `setturtleshape`.

To create a new sprite, first `clearscreen` and draw it with the turtle, then

```
makesprite "spritename
```

The drawing will be captured and saved in "spritename and cleared from the
screen. This process can be repeated to prepare additional sprite shapes.

To set the turtle to a sprite shape

```
setturtlesprite "spritename
```

## SVG Rendering Control Words

Sets the SVG shape rendering mode in CSS. In practice this just
turns anti-aliasing on or off, don't get too excited.

To set the SVG shape rendering mode

```
setshaperendering "crispedges
```

Accepted words are

1. `"geometricprecision` Default; enables anti-aliasing
3. `"optimizespeed` Typically identical to 1
2. `"crispedges` Disables anti-aliasing

## Experimental Logo/JS interoperation

Call JavaScript from Logo and Logo from JavaScript or both at once using UCB
Logo's JavaScript Foreign Function Interface.

### To call JavaScript from Logo
```
runjs "|console.log('Called JS from Logo')|
```

Warning: Uses JS `eval()` so totally insecure and slow.

Communication is over standard IO so everything must be stringified. JS values
are encoded to Berkeley Logo Object Notation or BLON. It's basically Logo JSON. 

Bools, numbers, null, and undefined are stringified and vbarred. Strings are escaped and
vbarred.

JS arrays are coerced to Logo sentences (cons lists, really). 

JS objects are coerced to a list in Logo `plist` format which is a normal list
but odd indices are keys and evens are values. The ordering may or may not be
significant, it's up to the JS object. For objects with only a few keys a linear
search is probably the fastest access. Otherwise load them into a Logo property
list for keyed access; see example below.

```
ignore cascade [empty? ?] [pprop "myplist first ? first butfirst ? butfirst butfirst ?] parse runjs "|/* js func call that returns an object */|
```

No efforts are made to preserve semantics, e.g. JS `null` becomes Logo `null`,
not `NIL`. This avoids the question of whether `undefined` should also be `NIL`
or how to represent `false`. Interpretation of these values is up to the Logo
program.

The resulting string is sent to Logo over stdin. It will be immediately
evaluated by Logo, unless you do something with it, like `print` or `parse` or
`ignore` it. JS and Logo both show their Lisp heritage here.

For more control over Logo/JS interaction it's not hard to add a new builtin
word to Logo in C that wraps JavaScript using the Emscripten API. Suggestions are appreciated.

Examples:
```
? pr runjs "|(()=>true)()|
true
? pr runjs "|(()=>null)()|
null
? pr runjs "|(()=>undefined)()|
undefined
? pr runjs "|(()=>123)()|
123
? pr runjs "|(()=>123.456)()|
123.456
? pr runjs "|(()=>['hello', 'world'])()|
|hello| |world|
? show parse runjs "|(()=>['hello', 'world'])()|
[hello world]
? pr  runjs "|(()=>({hello: "world", foo: {poo: "bar", boo: ['bee', 42]}}))()|
|hello| |world| |foo| [|poo| |bar| |boo| [|bee| 42]]
? show runparse runjs "|(()=>({hello: "world", foo: {poo: "bar", boo: ['bee', 42]}}))()|
[hello world foo [poo bar boo [bee 42]]]
? ignore cascade [empty? ?] [pprop "pplist first ? first butfirst ? butfirst butfirst ?] parse runjs "|(()=>({hello: "world", foo: {poo: "bar", boo: ['bee', 42]}}))()|
? popl "pplist
Pprop "pplist "foo [poo bar boo [bee 42]]
Pprop "pplist "hello "world
```

### To call Logo from Javascript

```javascript
runLogo('pr [Called Logo from JS]')
```

Logo output

```
? Called Logo from JS
?
```

Setting the optional second argument to true causes Logo to echo the command

```javascript
runLogo('pr [Called Logo from JS]', true)
```

Logo output

```
? pr [Called Logo from JS]
Called Logo from JS
? 
```

No marshaling is provided as of now. 

Warning: Also uses eval, the logo version. Logo commands are sent via stdin to the main eval loop. Totally insecure.

### Ouroboros

I don't know why you would want to do this.

```
? ignore runjs "|setTimeout(()=>runLogo('pr [called Logo from JS from Logo]'),0)|
? called Logo from JS from Logo
?
```

Or this.


```
> runLogo(`ignore runjs "|console.log('called JS from Logo from JS')|`, true)
called JS from Logo from JS
```

# Technical details

This describes how ucblogo was ported to the Web Assembly platform
using the Emscripten SDK.

## Background

In order to be able to run non-javascript software in web browsers, the Web has
grown a standardized byte-code virtual stack machine more or less equivalent to
the Java Virtual Machine. This VM specification is known as Web Assembly, or WASM.

WASM is a low-level virtual machine that does 3 main things

1. Execute WASM bytecode on the virtual processor
2. Expose virtual memory to WASM programs
3. Facilitate interaction between WASM programs and the browser (HTML/JS/CSS
   etc)

WASM does NOT provide an operating system or a runtime or libraries or really
much of anything else. As a platform it's similar to bare-metal. This was
intentional to enable it to be a generic computing platform capable of running
arbitrary software including full system emulators and desktop operating
systems.

In the case of ucblogo we want to be able to run it in a browser without a
heavyweight operating system. Further while WASM is very low level, the browser
provides most of the facilities of a traditional OS; how else would chromebooks
work? But the browser functionality is hidden behind the JavaScript Web API
which is completely different from the POSIX API used by ucblogo. 

Enter Emscripten; a system for porting UNIX software to the web. The Emscripten
SDK includes a C/C++->WASM compiler based on clang/llvm and a fairly robust
implementation of POSIX as a runtime library on top of the web browser's native
API. This allows most UNIX programs to be ported to the web with minor changes.

## Asyncify

There is one major wrinkle; the web browser is a cooperative-multitasking
environment; all programs must periodically yield to the browser event loop. But
C programs execute synchronously. Without accounting for this long running C
programs will freeze up the browser and you're gonna have a bad time. It's well
known fact that JS copied this model from Classic MacOS.

But there is another major wrinkle; UCB Logo uses a generationl mark & sweep
garbage collector to manage it's memory. This requires access to the stack and
VM registers to scan for pointers. Sadly the registers are opaque to C and the
stack is opaque to both C and WASM.

Asyncify solves both of these problems for us. 

Firstly, Asyncify magically transforms synchronous C programs into asynchronous
callbacks so your C function can `await` a JavaScript promise without blocking.

To do this Asyncify implements a shadow C stack in WASM linear memory, where the
heap lives. This is addition to the opaque WASM stack. Asyncify unwinds and
saves the stack whenever an async call is made. This includes spilling WASM
registers into the c shadow stack.

This has the upshot of giving us a mechanism to scan these otherwise opaque
memory regions.

Still Logo must ensure that it does an await every 50ms minimum no matter what
it's doing in order to YIELD to the browser event loop and not block user
interaction or cause unresponsive page alerts.

Logo must also yield after DOM mutations, like printing to the terminal or
drawing turtle graphics, so the browser can render the changes. Logo must also
yield to receive incoming events from JavaScript.

YIELD needs to be sprinkled around the code. It's particularly important to call
it periodically from within long running loops, like parsing and evaluation
loops.

The suggested way to do this is periodically check the wall clock and YIELD if
we've passed some deadline, say 5ms. But checking the wall clock from WASM is
pretty expensive so you don't want to do it often. But since you can't know
exactly how long Logo will take between clock checks, you have to check it
pretty often.

UCB Logo uses a heuristic approach whereby we call YIELD frequently, but we make
those calls practically free by not actually yelding most of the time. Most
calls to YIELD just decrement a counter and return. YIELD only checks the wall
clock when the counter reaches zero, and then only yields if the deadline has
passed. The result is reliable yielding with practically zero runtime cost.

On supporting browsers Logo uses the new `scheduler.yield` rather than
`setTimeout`; this results in a significant peformance improvement.

Logo users can force a YIELD with the `wait` word.

## Developer Instructions

### Prerequisites

* Emscripten >= 4.5 https://emscripten.org/docs/getting_started/downloads.html
* Python >= 3.6
* Node.js >= 20

Emscripten will install it's own version of clang called emcc.

Also emscrpten likes to compile C as C++ for some reason, but it doesn't seem to
cause us issues. (Is this right?)

We are using autotools with Emscripten but it's a little hacky at the moment.

Note: Our makehelp target doesn't work with emscripten. It's generally
recommended to NOT use wasm for stuff like that anyway. TBD how to update
autotools. 

### Building

The short version

```sh
cd ucblogo-code
. ~/emsdk/emsdk_env.sh
npm install
./emscript.sh
emrun ucblogo.html
```

### Runnning

Once compiled to WASM, ucblogo can be run in any WASM VM including node.js and
any modern web browser.

#### Node


    node ./ucblogo-node.js

Logo terminal input is currently broken in node, but you can still use it to run
unit tests from the console.

To debug in node I recommend using the VSCode node debug plugin from MS official. You can also use Chrome dev tools to remotely debug Node but it leaves some things to be desired.

To debug the web app install the Chrome DWARF plugin and be sure to build ucblogo.debug.wasm. Always split debug symbols into a separate file. The combined file is too bid and the touchy DWARF plugin will silently fail to load them.

#### Browser

Once the program is compiled to ucblogo.html one simply serves the output files
on a web server and loads it in a browser.

Emscripten includes a utility called `emrun` which will automatically start a
little web server, open the program in a browser window, and pipe the javascript
console output back to the terminal session. This requires that the --emrun flag
was passed to the linker.

```
emrun ucblogo.html
```

There's also an experimental AI generated develoment server; it just serves up 
the local dir an includes CORS headers.

```
node webserver.js
```

The debugging experience is a mixed bag. On the one hand Chrome Devtools is a
pretty amazing debugger. OTOH there is a huge impedance mismatch between C and
the web. And errors that occur in JS don't necessarily include a C
backtrace, or maybe the trace doesn't have line numbers.

Also to get decent performance from C programs in WASM it's important to crank
up the optmizations. This obviously causes bugs to bubble up to the surface 
while simultaneously making them harder to fix.

In the face of a difficult bug on WASM, I highly recommend attempting to
reproduce and fix it in the native host environment, then port the fix to the
WASM build.

Logo bugs can sometimes be forced to the surface by setting a tiny SEG_SIZE,
like 8. The resultant GC thrashing has a fuzzing effect and will make existing
bugs crash logo earlier and more often, which is good for debugging hard to
reproduce bugs.

See also `address-sanitizer.txt` for how to turn on LLVM sanitizers.

See also `contrib/gdb.rc` for some useful GDB macros.

### Deployment

Tag the release
```
git tag -m pXX pXX
```

Rebuild to include tag in version
```
./emscript.sh
```

Copy files to deployment dir, e.g.

```
cp {*.js,*.wasm,*.html,*.map,*.ico,*.png,*.pdf,*.json} ~/ucblogo-code-gh-pages
```

Commit and push


## Future Directions

WASM-GC looks interesting, but LLVM hasn't picked it up yet.

Objective Logo would enable better JS object integration; they're both 
prototypal!

The UI/UX could use a lot of work, it's currently very MVP.

A hosted web service to backup and share logo programs.

Expose more JS APIs, like sensors.

Unicode would be nice.