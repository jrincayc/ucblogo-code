
addToLibrary({

yield_to_js__deps: ['$safeSetTimeout', '$callUserCallback'],
yield_to_js__async: true,
yield_to_js: () => {
    // emscripten_sleep() does not return a value, but we still need a |return|
    // here for stack switching support (ASYNCIFY=2). In that mode this function
    // returns a Promise instead of nothing, and that Promise is what tells the
    // wasm VM to pause the stack.
    return (globalThis.scheduler?.yield) ? 
    Asyncify.handleSleep((wakeUp) => {
        {{{ runtimeKeepalivePush() }}}
        scheduler.yield().then(()=>{
            {{{ runtimeKeepalivePop() }}}
            callUserCallback(wakeUp);
        });
        return -1;
    }) : 
    Asyncify.handleSleep((wakeUp) => safeSetTimeout(wakeUp, 0))
}

});
