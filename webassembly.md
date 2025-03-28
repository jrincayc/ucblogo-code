# UCB Logo Web Assembly Port

Created March 25, 2025
By Jeff Laughlin, n1ywb-1

## Introduction

This document describes how ucblogo was ported to the Web Assembly platform
using the Emscripten SDK.

It goes without saying today that very nearly everybody on earth has access to a
web browser. The web is the defacto standard computing platform of the 21st
century.

Web Assembly gives us the opportunity to port ucblogo to the Web. This effort is
vital to maintaining the projects status as the defacto standard implementation
of Logo. Hardly anybody uses "computers" anymore; it's all phones and tablets.

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
environment. WASM programs must periodically yield execution to the browser so
it can process events and run other coroutines. But a tight loop which fails to
yield to the browser's even loop will freeze up the browser and you're gonna
have a bad time. I'm pretty sure Brendan copied this model from Classic MacOS; it's
a well known fact that he copied MacOS's event model.

The WASM community has developed a standard mechanism to facilitate cooperative
context switching known as JSPI or "JavaScript Promise Integration". JSPI
essentially allows blocking synchronous C code to magically become nonblocking
asynchronous, much like Gevent for Python from back in the day. 

Blocking IO calls yield automatically, so IO bound programs or very short
programs often need no changes. 

CPU-bound programs must yield from time to time, effectively becoming
co-routines.

Programs may explicitly yield by calling `emscripten_sleep(0)`, again like classic
MacOS.

We've defined this in a macro as thus

#ifdef __EMSCRIPTEN__
#define EMSLEEP(ticks) (emscripten_sleep(ticks))
#else
#define EMSLEEP(ticks) (0)
#endif

EMSLEEP needs to be sprinkled around the code. It's particularly important to
call it periodically from within long running loops, like parsing and evaluation
loops.

The suggested way to do this is periodically check the wall clock and call sleep
if enough time has elapsed. Browsers complain if they are blocked for more than
about 20ms, due to the impact long delays have on responsiveness and user
experience.

I've found that even though emscripten yields on blocking IO it doesn't
necessarily yield long enough or often enough for the browser to complete it's
part of the IO operation and update it's display. I've found it helpful to yield
after print to ensure the output is painted to the users screen promptly.

## Building

### Prerequisites

* https://emscripten.org/docs/getting_started/downloads.html
* Python >= 3.6

Emscripten will install it's own version of clang called emcc.

Also emscrpten likes to compile C as C++ for some reason, but it doesn't seem to
cause us issues. (Is this right?)

We are using autotools with Emscripten but it's a little hacky at the moment.

Note: Our makehelp target doesn't work with emscripten. It's generally
recommended to NOT use wasm for stuff like that anyway. TBD how to update
autotools. 

### Quick Start


```sh
. ~/emsdk/emsdk_env.sh
./emscript.sh
emrun ucblogo.html
```

When you run ./configure you need to pass in a few CFLAGS

    CFLAGS="-std=c90 -Wno-int-conversion"

You must also pass a few flags into configure itself. Also you must wrap the
call in a call to emconfigure.

    emconfigure ./configure --disable-docs --disable-x11 --disable-wx --prefix=/ --datadir=/logolib --enable-wasm 

Then the usual make clean/make except again we wrap make with emmake

    emmake make clean && emmake make

This doesn't perform the final linking step at the moment; so do it manually.
Emscripten final linkage must be performed with the C++ linker.

    em++ -O2 -Wno-write-strings -Wno-unused-variable \
    ucblogo-coms.o ucblogo-error.o ucblogo-eval.o ucblogo-files.o ucblogo-graphics.o \
    ucblogo-init.o ucblogo-intern.o ucblogo-libloc.o ucblogo-lists.o \
    ucblogo-logodata.o ucblogo-main.o ucblogo-math.o ucblogo-mem.o ucblogo-paren.o \
    ucblogo-parse.o ucblogo-print.o ucblogo-wrksp.o   ucblogo-term.o \
    ucblogo-nographics.o \
    --embed-file logolib \
    --embed-file tests \
    --pre-js pre.js \
    -s JSPI \
    -s ASYNCIFY_STACK_SIZE=100000 \
    -s ALLOW_MEMORY_GROWTH \
    -s ENVIRONMENT=web,webview,worker,node,shell \
    -s STRICT_JS \
    -s EMIT_PRODUCERS_SECTION \
    -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='$addOnExit' \
    --emrun \
    --shell-file shell.html \
    -o ucblogo.html

For debugging, include the following

    -g3 -gsource-map -O0 \
    -fsanitize=undefined \
    -fsanitize=address \
    -fno-sanitize=alignment \
    -s ASYNCIFY_ADVISE 

you can also use 

    -s ASSERTIONS=1 \
    
These are available but cannot be used with address sanitizer

    -s STACK_OVERFLOW_CHECK=1 \
    -s SAFE_HEAP=2 \

Additional possibly helpful flags

    -s EXCEPTION_DEBUG=1  \
    -s ASYNCIFY_ADVISE=1 \
    -s ASYNCIFY_DEBUG=1 

You might also try -Og to speed up debugging builds without over-optimizing it.

## Runnning

Once compiled to WASM, ucblogo can be run in any WASM VM including node.js and
any modern web browser.

### Node

Under node the --experimental-wasm-stack-switching turns on JSPI

    node --experimental-wasm-stack-switching  ./ucblogo.js

Logo terminal input is currently broken in node, but you can still use it to run
unit tests from the console.

I don't recommend using node for debugging, it's built in debugger sucks so much
that people usually use Chrome dev tools to debug node programs. At that point you might as
well just run it in the browser anyway.

### Browser

First you need to enable Experimental WebAssembly JavaScript Promise Integration
(JSPI) in chrome://flags/ 

JSPI is expected to be enabled by default soon, like months not years. 

Once the program is compiled to ucblogo.html one simply serves the output files
on a web server and loads it in a browser.

Emscripten includes a utility called `emrun` which will automatically start a
little web server, open the program in a browser window, and pipe the javascript
console output back to the terminal session. This requires that the --emrun flag
was passed to the linker.

emrun ucblogo.html

The debugging experience is a mixed bag. On the one hand Chrome Devtools is a
pretty amazing debugger. OTOH there is a huge impedance mismatch between C and
the web and even with source maps enabled the WASM VM is a stack machine so
unless you are a FORTH developer good luck examining it's heavily-optimized
assembly code. And errors that occur in JS don't necessarily include a C
backtrace, or maybe the trace doesn't have line numbers.

Also to get decent performance from C programs in WASM it's important to crank
up the optmizations. This obviously causes bugs to bubble up to the surface while simultaneously making them harder to fix.

In the face of a difficult bug on WASM, I highly recommend attempting to
reproduce and fix it in the native host environment, then port the fix to the
WASM build.

Logo bugs can sometimes be forced to the surface by setting a tiny SEG_SIZE,
like 8. The resultant GC thrashing has a fuzzing effect and will make existing
bugs crash logo earlier and more often, which is good for debugging hard to
reproduce bugs.

## Future Directions

WASM-GC looks interesting, but LLVM hasn't picked it up yet.

Once the initial port is complete the next step will be getting turtle graphics
working in some kind of UI. There are a few options here.

Unfortunately wx-widgets-wasm seems abandoned.

SDL might be a good option.

My preferred option is SVG. It's scalable; for free.

There is also a woefully incomplete xlib-wasm things

Could also run in a tiny Linux vm but adds a lot of overhead
