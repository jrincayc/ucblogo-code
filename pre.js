const SVG = 'http://www.w3.org/2000/svg';

Module.graphics = {};

Module.graphics.logoColors = [
    { r: 0, g: 0, b: 0 },
    { r: 0, g: 0, b: 99.6108949416342 },
    { r: 0, g: 99.6108949416342, b: 0 },
    { r: 0, g: 99.6108949416342, b: 99.6108949416342 },
    { r: 99.6108949416342, g: 0, b: 0 },
    { r: 99.6108949416342, g: 0, b: 99.6108949416342 },
    { r: 99.6108949416342, g: 99.6108949416342, b: 0 },
    { r: 99.6108949416342, g: 99.6108949416342, b: 99.6108949416342 },
    { r: 60.5477988860914, g: 37.5005722133211, b: 23.0472266727703 },
    { r: 76.9542992294194, g: 53.1258106355383, b: 7.03135728999771 },
    { r: 39.0630960555428, g: 63.2822156099794, b: 25.0003814755474 },
    { r: 46.8757152666514, g: 73.0479896238651, b: 73.0479896238651 },
    { r: 99.6108949416342, g: 58.2040131227588, b: 46.485084306096 },
    { r: 56.2508583199817, g: 44.1412985427634, b: 81.2512397955291 },
    { r: 99.6108949416342, g: 63.6728465705348, b: 0 },
    { r: 71.4854657816434, g: 71.4854657816434, b: 71.4854657816434 },
];
Module.FS_runjs_getChar_buffer = [];
Module.preRun = () => {
    const PREFIX = "/share/ucblogo";
    ENV.LOGOLIB = PREFIX + "/logolib";
    ENV.LOGOHELP = PREFIX + "/helpfiles";
    ENV.CSLS = PREFIX + "/csls";

    const runjs_ops = {
        put_char(tty, val) {
            if (val === 0 || val === 10) {
                Module.runjs(UTF8ArrayToString(tty.output)); tty.output = [];
            } else {
                if (val != 0) tty.output.push(val);
            }
        },
        fsync(tty) {
            if (tty.output?.length > 0) {
                Module.runjs(UTF8ArrayToString(tty.output));
                tty.output = [];
            }
        },
    };
    const RUNJS_MAJOR = 10;
    TTY.register(FS.makedev(RUNJS_MAJOR, 0), runjs_ops);
    FS.mkdev('/dev/runjs', FS.makedev(RUNJS_MAJOR, 0));
};

const BLON = {
    stringify: function (obj) {
        const addBrackets = (s) => `[${s}]`;
        const noAddBrackets = (s) => s;
        const helper = (addBrackets) => (val) => {
            if (typeof val === 'undefined') {
                return 'undefined';
            } else if (val === null) {
                return 'null';
            } else if (typeof val === 'boolean') {
                return val.toString();
            } else if (typeof val === 'number') {
                return val.toString();
            } else if (typeof val == 'string') {
                return `|${val
                    .replace('\\', '\\\\')
                    .replace('\n', '\\n')
                    .replace('|', '\\|')
                    }|`;
            } else if (Array.isArray(val)) {
                return addBrackets(`${val.map(v => innerHelper(v)).join(' ')
                    }`);
            } else /* object */ {
                return addBrackets(`${Object.entries(val)
                    .flat()
                    .map(v => innerHelper(v))
                    .join(' ')
                    }`);
            }
        };
        const outerHelper = helper(noAddBrackets);
        const innerHelper = helper(addBrackets);
        return outerHelper(obj);
    }
};
Module.FS_runjs_getChar_buffer_append_obj = (obj) => {
    // Module.FS_runjs_getChar_buffer
    //     .push(...intArrayFromString(BLON.stringify(obj), true));
    // Module.FS_runjs_getChar_buffer.push(10);
    let chars = BLON.stringify(obj).split("");
    chars.push("\n");
    Module.FS_runjs_getChar_buffer.push(...chars);
    document.dispatchEvent(new CustomEvent("getcharready"));
}
Module.FS_runjs_getChar_buffer_append_str = (str) => {
    let chars = str.split("");
    chars.push("\n");
    Module.FS_runjs_getChar_buffer.push(...chars);
    document.dispatchEvent(new CustomEvent("getcharready"));
}
Module.runjs = (expr) => {
    Module.FS_runjs_getChar_buffer_append_obj(eval(expr));
};
Module.env = {};
Module.graphics.pen_info = {
    x: 320,
    y: 240,
    sz: 1,
    c: 0,
    fntsz: 12,
    mode: 'normal',
};
Module.graphics.graphics_init = () => {
    const g = Module.graphics;
    // console.log("graphics_init");
    g.clear_screen();
    g.masknum = 0;
    const ld = document.getElementById('logoDrawing');
    ld.addEventListener('mousemove', (evt) => {
        g.lastmove = evt;
    });
    Module.graphics.buttonp = false;
    ld.addEventListener('mousedown', (evt) => {
        g.lastclick = evt;
        Module.graphics.buttonp = true;
        Module.ccall('mouse_down', 'void', [], []);
        evt.preventDefault();
        return false;
    }, { capture: true });
    ld.addEventListener('mouseup', (evt) => {
        Module.graphics.buttonp = false;
        evt.preventDefault();
        return false;
    }, { capture: true });
    ld.addEventListener('contextmenu', (evt) => {
        evt.preventDefault();
        return false;
    }, { capture: true });
};
Module.graphics.prepare_to_draw = () => {
    console.log("prepare_to_draw");
    Module.graphics.split_screen();
};
Module.graphics.done_drawing = () => {
    // console.log("done_drawing")
};
Module.graphics.prepare_to_exit = (v) => {
    // console.log(`prepare_to_exit(${v})`) 
};
Module.graphics.clear_screen = () => {
    // FIXME to work with mask erasing
    // console.log("clear_screen");
    if (typeof document === 'undefined') return;
    const dr = document.getElementById('logoDrawingContainer');
    const g = document.getElementById('logoDrawingElements');
    dr.removeChild(g);
    const newg = document.createElementNS(SVG, 'g')
    newg.id = 'logoDrawingElements';
    dr.appendChild(newg);
    const rect = document.createElementNS(SVG, 'rect');
    rect.setAttribute('class', 'sizer');
    newg.appendChild(rect);
    const defs = document.getElementById('logoDefs');
    defs.replaceChildren([]);
    delete Module.graphics.mask;
    Module.graphics.masknum = 0;
};
Module.graphics.prepare_to_exit = (v) => {
    // console.log(`prepare_to_exit ${[v]}`) 
};
Module.graphics.line_to = (x, y) => {
    if (typeof document === 'undefined') return;
    // console.log(`line_to ${[x, y]}`);
    const g = Module.graphics;
    const pen_info = g.pen_info;
    if (pen_info.v == 0) {
        const g = Module.graphics;
        const masknum = Module.graphics.masknum;
        const ld = document.getElementById(g.mask ? `logoMask${masknum}` : 'logoDrawingElements')
        const el = document.createElementNS(SVG, 'line');
        el.setAttribute("stroke-width", `${pen_info.sz}px`);
        if (g.mask) {
            el.setAttribute('stroke', `black`);
        }
        else {
            el.style.setProperty("mix-blend-mode", pen_info.mode);
            el.style.setProperty('stroke', `var(--logo-color-${pen_info.c})`);
        }
        el.setAttribute("x1", x + 0.5);
        el.setAttribute("y1", y + 0.5);
        el.setAttribute("x2", pen_info.x + 0.5);
        el.setAttribute("y2", pen_info.y + 0.5);
        ld.appendChild(el);
    };
    pen_info.x = x;
    pen_info.y = y;
    const turtle = document.getElementById('logoTurtleTranslated');
    turtle.setAttribute('transform', `translate(${x - 6}, ${y - 9})`);
};
Module.graphics.move_to = (x, y) => {
    if (typeof document === 'undefined') return;
    // console.log(`move_to ${[x, y]}`)
    Module.graphics.pen_info.x = x;
    Module.graphics.pen_info.y = y;
    const turtle = document.getElementById('logoTurtleTranslated');
    turtle.setAttribute('transform', `translate(${x - 6}, ${y - 9})`);
};
Module.graphics.label = (s) => {
    if (typeof document === 'undefined') return;
    // console.log(`label ${[s]}`);
    const g = Module.graphics;
    const pen_info = g.pen_info;
    const ld = document.getElementById('logoDrawingElements')
    const el = document.createElementNS(SVG, 'text');
    el.textContent = UTF8ToString(s);
    el.style.setProperty('fill', `var(--logo-color-${pen_info.c})`);
    el.setAttribute('font-size', pen_info.fntsz);
    el.setAttribute('x', pen_info.x);
    el.setAttribute('y', pen_info.y);
    ld.appendChild(el);
};
Module.graphics.set_pen_vis = (v) => {
    if (typeof document === 'undefined') return;
    Module.graphics.pen_info.v = v;
    // console.log(`set_pen_vis ${[v]}`)
};
Module.graphics.set_pen_mode = (m) => {
    // console.log(`set_pen_mode ${[m]}`) 
};
Module.graphics.set_pen_color = (c) => {
    if (typeof document === 'undefined') return;
    // console.log(`set_pen_color ${[c]}`);
    const pen_info = Module.graphics.pen_info;
    const el = document.getElementById('logoTurtleTranslated');
    pen_info.c = c;
    el.style.setProperty('stroke', `var(--logo-color-${pen_info.c})`);
};
Module.graphics.set_pen_width = (w) => {
    if (typeof document === 'undefined') return;
    // console.log(`set_pen_width ${[w]}`);
    Module.graphics.pen_info.sz = w;
};
Module.graphics.set_pen_height = (w) => {
    if (typeof document === 'undefined') return;
    // console.log(`set_pen_height ${[w]}`);
    Module.graphics.pen_info.sz = w;
    const el = document.getElementById('logoTurtle');
    el.style.setProperty('stroke-width', w);
};
// Module.graphics.set_pen_x = () => { console.log(`set_pen_x ${[]}`) };
// Module.graphics.set_pen_y = () => { console.log(`set_pen_y ${[]}`) };
Module.graphics.set_back_ground = (c) => {
    if (typeof document === 'undefined') return;
    // console.log(`set_back_ground ${[c]}`)
    const pen_info = Module.graphics.pen_info;
    const el = document.getElementById("logoBackground")
    el.style.setProperty('fill', `var(--logo-color-${c})`);
    pen_info.bg = c;
};
Module.graphics.pen_reverse = () => {
    // console.log(`pen_reverse ${[]}`)
    Module.graphics.pen_info.mode = 'difference';
};
Module.graphics.pen_erase = () => {
    // console.log(`pen_erase ${[]}`)
    // create new mask in defs
    // apply mask to current logoDrawingElements
    // when pen != erase start new logoDrawingElements
    const ld = document.getElementById('logoDrawing');
    const lde = document.getElementById('logoDrawingElements');
    const defs = document.getElementById('logoDefs');
    const masknum = Module.graphics.masknum;
    const g = Module.graphics;
    const mask = document.createElementNS(SVG, 'mask');
    mask.setAttribute('id', `logoMask${masknum}`);
    const rect = document.createElementNS(SVG, 'rect');
    rect.setAttribute('x', 0);
    rect.setAttribute('y', 0);
    rect.setAttribute('height', 480);
    rect.setAttribute('width', 640);
    rect.setAttribute('fill', 'white');
    mask.appendChild(rect);
    defs.appendChild(mask);
    lde.setAttribute('mask', `url('#logoMask${masknum}')`);
    lde.setAttribute('maskUnits', "userSpaceOnUse");
    Module.graphics.mask = mask;
};
Module.graphics.pen_down = () => {
    // console.log(`pen_down ${[]}`)
    Module.graphics.pen_info.mode = 'initial';
    if (Module.graphics.mask) {
        const dr = document.getElementById('logoDrawingContainer');
        const g = document.getElementById('logoDrawingElements');
        dr.removeChild(g);
        g.removeAttribute('id');
        const newg = document.createElementNS(SVG, 'g')
        newg.id = 'logoDrawingElements';
        dr.appendChild(newg);
        const rect = document.createElementNS(SVG, 'rect');
        rect.setAttribute('class', 'sizer');
        newg.appendChild(rect);
        newg.appendChild(g);
        delete Module.graphics.mask;
        delete Module.graphics.masknum++;
    }
};
Module.graphics.full_screen = () => { console.log(`full_screen ${[]}`) };
Module.graphics.split_screen = () => {
    console.log(`split_screen ${[]}`);
    document.getElementById('logoDrawing')
        .style.setProperty('display', 'block');
    document.getElementById('logoInputContainer')
        .style.setProperty('display', 'flex');
    window.dispatchEvent(new Event('resize'));
};
Module.graphics.text_screen = () => {
    console.log(`text_screen ${[]}`);
    document.getElementById('logoDrawing')
        .style.setProperty('display', 'none');
    document.getElementById('logoInputContainer')
        .style.setProperty('display', 'flex');
    window.dispatchEvent(new Event('resize'));
};
Module.graphics.save_pen = (p) => {
    // console.log(`save_pen ${[p]}`) 
};
Module.graphics.restore_pen = (p) => {
    // console.log(`restore_pen ${[p]}`) 
};
Module.graphics.plain_xor_pen = () => {
    // console.log(`plain_xor_pen ${[]}`) 
};
// use https://developer.mozilla.org/en-US/docs/Web/API/OscillatorNode
Module.graphics.tone = (pitch, duration) => {
    // console.log(`tone ${[pitch, duration]}`) 
};
Module.graphics.set_pen_pattern = (pat) => {
    // console.log(`set_pen_pattern ${[pat]}`) 
};
Module.graphics.get_pen_pattern = (pat) => {
    // console.log(`get_pen_pattern ${[pat]}`) 
};
Module.graphics.set_list_pen_pattern = (pat) => {
    // console.log(`set_list_pen_pattern ${[pat]}`) 
};
Module.graphics.prepare_to_draw_turtle = () => {
    // console.log(`prepare_to_draw_turtle ${[]}`) 
};
Module.graphics.web_done_drawing_turtle = () => {
    // console.log(`web_done_drawing_turtle ${[]}`) 
};
Module.graphics.logofill = () => {
    // console.log(`logofill ${[]}`) 
};
Module.graphics.set_palette = (i, r, g, b) => {
    // console.log(`set_palette ${[i, r, g, b]}`);
    const logoColors = Module.graphics.logoColors;
    const styleElement = document.getElementById('logoColors');
    const conv = n => n * 100 / 65535;
    const c = logoColors[i] = { r: conv(r), g: conv(g), b: conv(b) };
    styleElement.textContent = (
        `:root {
${logoColors.map((c, idx) => `
    --logo-color-${idx}: rgb(${c.r}%, ${c.g}%, ${c.b}%);`).join('\n')}
}
`);
}
Module.graphics.get_palette = (i, pR, pG, pB) => {
    // console.log(`get_palette ${i}}`);
    const logoColors = Module.graphics.logoColors;
    const styleElement = document.getElementById('logoColors').style;
    const { r, g, b } = logoColors[i] || { r: 0, g: 0, b: 0 };
    const setpv = (p, v) => {
        setValue(p, v * 65535 / 100, 'i32');
    }
    setpv(pR, r);
    setpv(pG, g);
    setpv(pB, b);
};
Module.graphics.erase_screen = () => {
    // console.log(`erase_screen ${[]}`) 
};
Module.graphics.draw_turtle = (heading) => {
    // console.log(`draw_turtle ${heading}`);
    const turtle = document.getElementById('logoTurtle');
    turtle.setAttribute('transform', `rotate(${heading})`);
};
Module.graphics.get_label_size = () => {
    // console.log('get_label_size');
    const pen_info = Module.graphics.pen_info;
    const theLetterM = document.getElementById('theLetterM');
    const { width } = theLetterM.getBoundingClientRect();
    return { width: Math.floor(width), height: pen_info.fntsz };
};
Module.graphics.adjust_label_height = (h) => {
    // console.log(`adjust_label_height ${h}`);
    const pen_info = Module.graphics.pen_info;
    pen_info.fntsz = h;
    const theLetterM = document.getElementById('theLetterM');
    theLetterM.style.setProperty('font-size', h + 'px');
};
Module.graphics.hide_turtle = () => {
    document.getElementById('logoTurtle').style.setProperty('display', 'none');
};
Module.graphics.show_turtle = () => {
    document.getElementById('logoTurtle').style.setProperty('display', 'initial');
};
Module.graphics.filled_begin = (color) => {
    const el = document.createElementNS(SVG, 'polygon');
    const ld = document.getElementById('logoDrawingElements')
    ld.appendChild(el);
    const pen_info = Module.graphics.pen_info;
    el.style.setProperty('fill', `var(--logo-color-${color})`);
    el.style.setProperty('stroke', `var(--logo-color-${pen_info.c})`);
    el.style.setProperty('stroke-width', pen_info.sz);
    Module.graphics.filled_poly = el;
}
Module.graphics.filled_add_point = (x, y) => {
    const svgel = document.getElementById('logoDrawing')
    const poly = Module.graphics.filled_poly;
    const point = svgel.createSVGPoint();
    point.x = x;
    point.y = y;
    Module.graphics.filled_poly.points.appendItem(point);
}
Module.graphics.filled_end = () => {
    delete Module.graphics.filled_poly;
}
Module.graphics.web_get_buttonp = () => {
    return Module.graphics.buttonp;
}
Module.graphics.web_get_button = () => {
    const b = Module.graphics.lastclick?.button;
    if (b != undefined) {
        delete Module.graphics.lastclick;
        if (b == 0) return 1;
        if (b == 1) return 3;
        if (b == 2) return 2;
    }
    return 0;
}
Module.graphics.web_get_mouse_x = () => {
    return Module.graphics.lastmove?.offsetX - 320 || 0;
}
Module.graphics.web_get_mouse_y = () => {
    return Module.graphics.lastmove?.offsetY * -1 + 240 || 0;
}
Module.graphics.web_get_click_x = () => {
    return Module.graphics.lastclick?.offsetX - 320 || 0;
}
Module.graphics.web_get_click_y = () => {
    return Module.graphics.lastclick?.offsetY * -1 + 240 || 0;
}
Module.upgradeLogoElements = () => {
    class LogoElement extends HTMLElement {
        static observedAttributes = ['id', 'class', 'style', 'hidden', 'accesskey', 'checked', 'data-logo', 'disabled', 'draggable', 'height', 'loop', 'controls', 'name', 'open', 'readonly', 'reversed', 'rows', 'selected', 'size', 'slot', 'value', 'width', 'wrap'];

        constructor() {
            super(); // Must call first
            // User code here
            Module.FS_runjs_getChar_buffer_append_obj(
                ['logoElConstructed', this.getAttribute('id')]
            );
        }

        connectedCallback() {
            Module.FS_runjs_getChar_buffer_append_obj(
                ['logoElConnected', this.getAttribute('id')]
            );
        }

        disconnectedCallback() {
            Module.FS_runjs_getChar_buffer_append_obj(
                ['logoElDisconnected', this.getAttribute('id')]
            );
        }

        adoptedCallback() {
            Module.FS_runjs_getChar_buffer_append_obj(
                ['logoElAdopted', this.getAttribute('id')]
            );
        }

        attributeChangedCallback(name, oldValue, newValue) {
            Module.FS_runjs_getChar_buffer_append_obj([
                'logoElAttributeChanged',
                this.getAttribute('id'),
                name, oldValue, newValue
            ]);
        }
    }
    customElements.define("logo-element", LogoElement);
}

function logostop() {
    Module.FS_runjs_getChar_buffer_append_obj('logoStop');
}

function logoRun(runliststr, echo) {
    if (echo) {
        Module.FS_runjs_getChar_buffer_append_str(`pr [${runliststr}] ${runliststr}`);
    } else {
        Module.FS_runjs_getChar_buffer_append_str(runliststr);
    }
}

Module.uploadFile = async file => {
    FS.writeFile(file.name, new Uint8Array(await file.arrayBuffer()));
};
Module.uploadFileButtonClicked = async evt => {
    try {
        await Promise.all(Array.from(evt.target.files).map(async f => {
            await Module.uploadFile(f);
            logoRun(`load "|${f.name.replace('|', '\\|')}|`, true);
        }));
    }
    catch (error) {
        window.alert(`Error uploading files ${error.toString()}`);
        throw error;
    }
};
Module.downloadSessionClicked = async (event) => {
    logoRun(`save "/tmp/ucblogo_session.lgo`, true);
    await new Promise(res => setTimeout(res, 0));
    const contents = FS.readFile('/tmp/ucblogo_session.lgo');
    var stringContents = new TextDecoder().decode(contents);
    const blob = new Blob(stringContents.split(''), { type: "text/plain;charset=UTF-8" });
    let element = document.createElement('a');
    element.setAttribute('download', 'ucblogo_session.lgo');
    element.setAttribute('href', URL.createObjectURL(blob));
    element.click();
};
Module.downloadDrawingClicked = async (event) => {
    const domparser = new DOMParser();
    const drawing = domparser.parseFromString(
        document.getElementById('logoDrawing').outerHTML,
        'text/xml'
    );
    const logoColors = drawing.createElement('style');
    const fixColor = (el, attr) => {
        const val = el.style.getPropertyValue(attr);
        const colorMatch = val.match(/var\(--logo-color-(\d+)\)/);
        if (colorMatch) {
            const color = colorMatch[1];
            const c = Module.graphics.logoColors[color] || 7;
            el.setAttribute(attr, `rgb(${c.r}%, ${c.g}%, ${c.b}%)`);
            el.style.removeProperty(attr);
        }
    };
    for (const el of drawing.querySelectorAll('*')) {
        fixColor(el, 'stroke');
        fixColor(el, 'fill');
    }
    logoColors.innerHTML = document.getElementById('logoColors').innerHTML;
    drawing.querySelector('svg').appendChild(logoColors);
    drawing.querySelector('style').textContent += '* { font-family: monospace }'
    const markup =
        '<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'
        + drawing.querySelector('svg').outerHTML
    const blob = new Blob(
        markup.split(''),
        { type: "image/svg+xml;charset=UTF-8" }
    );
    let element = document.createElement('a');
    element.setAttribute('download', 'ucblogo_drawing.svg');
    element.setAttribute('href', URL.createObjectURL(blob));
    element.click();
};
Module.commandHistory = [];
Module.commandHistoryIdx = -1;
Module.currentCommand = null;
Module.handleKeyDown = evt => {
    if (evt.key == "ArrowDown") {
        if (Module.commandHistoryIdx == -1) {
            document.getElementById('logoInputText').value = Module.currentCommand;
            return;
        }
        document.getElementById('logoInputText').value = Module.commandHistory[Module.commandHistoryIdx];
        --Module.commandHistoryIdx;
    }
    else if (evt.key == "ArrowUp") {
        if (Module.commandHistoryIdx == Module.commandHistory.length - 1) {
            return;
        }
        if (Module.commandHistoryIdx == -1) {
            Module.currentCommand = document.getElementById('logoInputText').value;
        }
        ++Module.commandHistoryIdx;
        document.getElementById('logoInputText').value = Module.commandHistory[Module.commandHistoryIdx];
    }
};