#!/bin/bash -v

# NOTE: We can't fully support STANDALONE_WASM until WASI supports longjmp
# This only an issue for WASI runtimes like Wasmer or Wasmtime

# wasm mdarray unit test crashes with OOB error with -O > 0
# Future: use -mtail-call (is the interpreter even recursive?)

export CFLAGS="-O0 -g -std=gnu90 -Wno-comment -Wno-typedef-redefinition -fsanitize=undefined -fsanitize=address"
export CXXFLAGS="-O0 -g -fsanitize=undefined -fsanitize=address"
# export EMCC_DEBUG=1

# actually compiles slower with -j > 1... and I'm on a quad-core i7
emconfigure ./configure --disable-docs --disable-x11 --disable-wx --prefix=/ --datadir=/logolib --enable-objects --enable-wasm \
&& emmake make clean \
&& emmake make \
&& em++ $CXXFLAGS \
-o ucblogo.html \
ucblogo-coms.o ucblogo-error.o ucblogo-eval.o ucblogo-files.o ucblogo-graphics.o \
ucblogo-init.o ucblogo-intern.o ucblogo-libloc.o ucblogo-lists.o \
ucblogo-logodata.o ucblogo-main.o ucblogo-math.o ucblogo-mem.o ucblogo-paren.o \
ucblogo-parse.o ucblogo-print.o ucblogo-wrksp.o   ucblogo-term.o \
ucblogo-nographics.o ucblogo-obj.o \
--embed-file logolib \
--pre-js pre.js \
--embed-file tests \
-s JSPI \
-s ASYNCIFY_ADVISE \
-s ASYNCIFY_STACK_SIZE=100000 \
-s ALLOW_MEMORY_GROWTH \
--emrun \
-s ENVIRONMENT=web,webview,worker,node,shell \
-s STRICT_JS \
-s EMIT_PRODUCERS_SECTION \
-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='$addOnExit' \
-fsanitize=undefined \
-fsanitize=address \
--shell-file shell.html \
--save-temps
# -fno-sanitize=alignment \
# -s EXIT_RUNTIME=1 \
# -s EXPORT_NAME=ucblogo \
# -s STANDALONE_WASM \
# -s ASSERTIONS=1 \
# -s STACK_OVERFLOW_CHECK=1 \
# -s SAFE_HEAP=2 \




#--embed-file ucblogo.wasm.map \
#-s EXCEPTION_DEBUG=1  \
#-s ASYNCIFY_ADVISE=1 \
#-s ASYNCIFY_DEBUG=1 


# Doesn't work with asyncify
# Does it work with JSPI?
# -fsanitize=address 

# redundant with -gsource-maps
# --profiling-funcs \

# This is future but node isn't quite there
# -s MODULARIZE -s EXPORT_ES6 

# Must enable experimental wasm stack swtiching in chrome://flags
# Run tests by 
# 1. typing into the dialog box: load "tests/test.lg"
# 2. Click OK (adds line to buffer)
# 3. Click Cancel (executes lines in buffer)
# 4. Watch output pane
# python3 -m http.server 8080 & xdg-open http://0.0.0.0:8080/ucblogo.html

# Or use node; it uses the same V8 engine as chrome
node --experimental-wasm-stack-switching  ./ucblogo.js tests/test.lg

# or use emrun
# link with --emrun then run
# emrun