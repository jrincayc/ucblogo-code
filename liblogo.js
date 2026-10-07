
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
},

safe_request_animation_frame__async: false,
safe_request_animation_frame: (cb, userData) => {
    function _sraf() {
        return requestAnimationFrame(
            (timeStamp) => {
                if (Asyncify.state === 0) {
                    {{{ makeDynCall('idp', 'cb') }}}(timeStamp, userData)
                }
                else {
                    console.warn('dropped frame currState != 0');
                    // FIXME update animationFrameID global
                    setTimeout(
                        ()=>Module.setValue(Module['_animationFrameID'], _sraf(), 'i32'),
                        0
                    );
                }
            }
        )
    }
    return _sraf();
},

});
