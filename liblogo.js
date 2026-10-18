
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

webLogoSleep__deps: ['$safeSetTimeout'],
webLogoSleep__async: true,
webLogoSleep: (ms) => {
// emscripten_sleep() does not return a value, but we still need a |return|
// here for stack switching support (ASYNCIFY=2). In that mode this function
// returns a Promise instead of nothing, and that Promise is what tells the
// wasm VM to pause the stack.
return Asyncify.handleSleep((wakeUp) => {
    Module.WEB_WAIT_WAKEUP = wakeUp;
    Module.WEB_WAIT_TIMEOUT_ID = safeSetTimeout(()=>{
        Module.WEB_WAIT_TIMEOUT_ID = 0;
        Module.WEB_WAIT_WAKEUP = undefined;
        wakeUp();
    }, ms);
});
},

peek_uievent: () => {
    return Module.uievents.length;
},

get_uievent: (evtptr) => {
    if (Module.uievents.length === 0) {
        return 0;
    }
    const evt = Module.uievents.pop();
    setValue(evtptr, evt.type, 'i32');
    switch (evt.type) {
        case EVT_T_MOUSEDOWN:
        case EVT_T_MOUSEUP:
        case EVT_T_MOUSEMOVE:
            setValue(evtptr + 4, evt.button, 'i32');
            setValue(evtptr + 4 + 4, evt.x, 'i32');
            setValue(evtptr + 4 + 4 + 4, evt.y, 'i32');
            break;
        case EVT_T_KEYDOWN:
        case EVT_T_KEYUP:
            setValue(evtptr + 4, evt.key, 'i32');
            break;
    }
   return 1;
},

});
