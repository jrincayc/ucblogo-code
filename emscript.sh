#!/bin/bash -v

# NOTE: We can't fully support STANDALONE_WASM until WASI supports longjmp
# This only an issue for WASI runtimes like Wasmer or Wasmtime

# wasm mdarray unit test crashes with OOB error with -O > 0
# Future: use -mtail-call (is the interpreter even recursive?)

export CFLAGS="-O3 -std=gnu90 -Wno-comment -Wno-typedef-redefinition"
export CXXFLAGS="-O3"
# export CFLAGS="-g -O3 -std=gnu90 -Wno-comment -Wno-typedef-redefinition -fsanitize-address -fsanitize-undefined"
# export CXXFLAGS="-g -O3 -fsanitize-address -fsanitize-undefined"
export LDFLAGS="-O3 -ferror-limit=0"
export LDFLAGS="$LDFLAGS -s JSPI=0"
export LDFLAGS="$LDFLAGS -s ASYNCIFY=1"
export LDFLAGS="$LDFLAGS -s ASYNCIFY_STACK_SIZE=10000000"
# export LDFLAGS="$LDFLAGS -g"
# export LDFLAGS="$LDFLAGS -gsource-map"
# export LDFLAGS="$LDFLAGS -fsanitize=address -fsanitize=undefined"
# export LDFLAGS="$LDFLAGS -s EXCEPTION_DEBUG=1"
# export LDFLAGS="$LDFLAGS -s LIBRARY_DEBUG=1"
# export LDFLAGS="$LDFLAGS -s SYSCALL_DEBUG=1"
# export LDFLAGS="$LDFLAGS -s ASSERTIONS=1"
# export LDFLAGS="$LDFLAGS -s STACK_OVERFLOW_CHECK=1"
# export LDFLAGS="$LDFLAGS -s SAFE_HEAP=2"
# export EMCC_DEBUG=1

# actually compiles slower with -j > 1... and I'm on a quad-core i7
emconfigure ./configure --disable-docs --disable-x11 --disable-wx \
--prefix=`pwd`/dist \
--disable-objects --enable-wasm \
&& emmake make clean \
&& emmake make ucblogo-node.js \
&& node ./ucblogo-node.js tests/test.lg
# && wasm-opt ucblogo-node.wasm --spill-pointers -o ucblogo-node.wasm \
# && emmake make ucblogo.html

# -s ASYNCIFY_ADVISE \
# --save-temps
# -s EXIT_RUNTIME=1 \
# -s EXPORT_NAME=ucblogo \
# -s STANDALONE_WASM \

    # -s ASYNCIFY_ADVISE=1 \
    # -s ASYNCIFY_DEBUG=1  \

# Doesn't work with asyncify
# Does it work with JSPI?

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

# or use emrun
# link with --emrun then run
# emrun
