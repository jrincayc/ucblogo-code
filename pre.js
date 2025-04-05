const SVG = 'http://www.w3.org/2000/svg';

Module.graphics = {};

Module.graphics.logoColors = [
{r: 0, g: 0, b: 0},
{r: 0, g: 0, b: 99.6108949416342},
{r: 0, g: 99.6108949416342, b: 0},
{r: 0, g: 99.6108949416342, b: 99.6108949416342},
{r: 99.6108949416342, g: 0, b: 0},
{r: 99.6108949416342, g: 0, b: 99.6108949416342},
{r: 99.6108949416342, g: 99.6108949416342, b: 0},
{r: 99.6108949416342, g: 99.6108949416342, b: 99.6108949416342},
{r: 60.5477988860914, g: 37.5005722133211, b: 23.0472266727703},
{r: 76.9542992294194, g: 53.1258106355383, b: 7.03135728999771},
{r: 39.0630960555428, g: 63.2822156099794, b: 25.0003814755474},
{r: 46.8757152666514, g: 73.0479896238651, b: 73.0479896238651},
{r: 99.6108949416342, g: 58.2040131227588, b: 46.485084306096},
{r: 56.2508583199817, g: 44.1412985427634, b: 81.2512397955291},
{r: 99.6108949416342, g: 63.6728465705348, b: 0},
{r: 71.4854657816434, g: 71.4854657816434, b: 71.4854657816434},
];

Module.preRun = () => {
    const PREFIX = "/share/ucblogo";
    ENV.LOGOLIB = PREFIX + "/logolib";
    ENV.LOGOHELP = PREFIX + "/helpfiles";
    ENV.CSLS = PREFIX + "/csls";
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
    console.log("graphics_init");
    g.clear_screen();
    g.masknum = 0;
    const ld = document.getElementById('logoDrawing');
    ld.addEventListener('mousemove', (evt) => {
        g.lastmove = evt;
    });
    ld.addEventListener('click', (evt) => {
        g.lastclick = evt;
    });
};
Module.graphics.prepare_to_draw = () => { console.log("prepare_to_draw") };
Module.graphics.done_drawing = () => { console.log("done_drawing") };
Module.graphics.prepare_to_exit = (v) => { console.log(`prepare_to_exit(${v})`) };
Module.graphics.clear_screen = () => {
    // FIXME to work with mask erasing
    console.log("clear_screen");
    if (typeof document === 'undefined') return;
    const dr = document.getElementById('logoDrawingContainer');
    const g = document.getElementById('logoDrawingElements');
    dr.removeChild(g);
    const newg = document.createElementNS(SVG, 'g')
    newg.id = 'logoDrawingElements';
    dr.appendChild(newg);
    const rect = document.createElementNS(SVG, 'rect');
    rect.setAttribute('style', 'height: 100%; width: 100%;');
    newg.appendChild(rect);
    const defs = document.getElementById('logoDefs');
    defs.replaceChildren([]);
    delete Module.graphics.mask;
    Module.graphics.masknum = 0;
};
Module.graphics.prepare_to_draw = () => { console.log(`prepare_to_draw ${[]}`) };
Module.graphics.done_drawing = () => { console.log(`done_drawing ${[]}`) };
Module.graphics.prepare_to_exit = (v) => { console.log(`prepare_to_exit ${[v]}`) };
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
        el.setAttribute("stroke-width", pen_info.sz);
        if (g.mask) {
            el.setAttribute('stroke', `black`);
        }
        else {
            el.style.setProperty("mix-blend-mode", pen_info.mode);
            el.style.setProperty('stroke', `var(--logo-color-${pen_info.c})`);
        }
        el.setAttribute("x1", x);
        el.setAttribute("y1", y);
        el.setAttribute("x2", pen_info.x);
        el.setAttribute("y2", pen_info.y);
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
    console.log(`label ${[s]}`);
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
    console.log(`set_pen_vis ${[v]}`)
};
Module.graphics.set_pen_mode = (m) => { console.log(`set_pen_mode ${[m]}`) };
Module.graphics.set_pen_color = (c) => {
    if (typeof document === 'undefined') return;
    console.log(`set_pen_color ${[c]}`);
    const pen_info = Module.graphics.pen_info;
    const el = document.getElementById('logoTurtleTranslated');
    pen_info.c = c;
    el.style.setProperty('stroke', `var(--logo-color-${pen_info.c})`);
};
Module.graphics.set_pen_width = (w) => {
    if (typeof document === 'undefined') return;
    console.log(`set_pen_width ${[w]}`);
    Module.graphics.pen_info.sz = w;
};
Module.graphics.set_pen_height = (w) => {
    if (typeof document === 'undefined') return;
    console.log(`set_pen_height ${[w]}`);
    Module.graphics.pen_info.sz = w;
    const el = document.getElementById('logoTurtle');
    el.style.setProperty('stroke-width', w);
};
// Module.graphics.set_pen_x = () => { console.log(`set_pen_x ${[]}`) };
// Module.graphics.set_pen_y = () => { console.log(`set_pen_y ${[]}`) };
Module.graphics.set_back_ground = (c) => {
    if (typeof document === 'undefined') return;
    console.log(`set_back_ground ${[c]}`)
    const pen_info = Module.graphics.pen_info;
    const el = document.getElementById("logoBackground")
    el.style.setProperty('fill', `var(--logo-color-${c})`);
    pen_info.bg = c;
};
Module.graphics.pen_reverse = () => { 
    console.log(`pen_reverse ${[]}`) 
    Module.graphics.pen_info.mode = 'difference';
};
Module.graphics.pen_erase = () => { 
    console.log(`pen_erase ${[]}`) 
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
    console.log(`pen_down ${[]}`)
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
        rect.setAttribute('style', 'height: 100%; width: 100%;');
        newg.appendChild(rect);
        newg.appendChild(g);
        delete Module.graphics.mask;
        delete Module.graphics.masknum++;
    }
};
Module.graphics.full_screen = () => { console.log(`full_screen ${[]}`) };
Module.graphics.split_screen = () => { console.log(`split_screen ${[]}`) };
Module.graphics.text_screen = () => { console.log(`text_screen ${[]}`) };
Module.graphics.save_pen = (p) => { console.log(`save_pen ${[p]}`) };
Module.graphics.restore_pen = (p) => { console.log(`restore_pen ${[p]}`) };
Module.graphics.plain_xor_pen = () => { console.log(`plain_xor_pen ${[]}`) };
// use https://developer.mozilla.org/en-US/docs/Web/API/OscillatorNode
Module.graphics.tone = (pitch, duration) => { console.log(`tone ${[pitch, duration]}`) };
Module.graphics.set_pen_pattern = (pat) => { console.log(`set_pen_pattern ${[pat]}`) };
Module.graphics.get_pen_pattern = (pat) => { console.log(`get_pen_pattern ${[pat]}`) };
Module.graphics.set_list_pen_pattern = (pat) => { console.log(`set_list_pen_pattern ${[pat]}`) };
Module.graphics.prepare_to_draw_turtle = () => { console.log(`prepare_to_draw_turtle ${[]}`) };
Module.graphics.web_done_drawing_turtle = () => { console.log(`web_done_drawing_turtle ${[]}`) };
Module.graphics.logofill = () => { console.log(`logofill ${[]}`) };
Module.graphics.set_palette = (i, r, g, b) => { 
    console.log(`set_palette ${[i, r, g, b]}`);
    const logoColors = Module.graphics.logoColors;
    const styleElement = document.getElementById('logoColors');
    const conv = n => n * 100 / 65535;
    const c = logoColors[i] = {r: conv(r), g: conv(g), b: conv(b)};
    styleElement.textContent = (
`:root {
${logoColors.map((c, idx) => `
    --logo-color-${idx}: rgb(${c.r}%, ${c.g}%, ${c.b}%);`).join('\n')}
}
`);
}
Module.graphics.get_palette = (i, pR, pG, pB) => { 
    console.log(`get_palette ${i}}`);
    const logoColors = Module.graphics.logoColors;
    const styleElement = document.getElementById('logoColors').style;
        const {r, g, b} = logoColors[i] || {r: 0, g: 0, b: 0};
        const setpv = (p, v) => {
            setValue(p, v * 65535 / 100, 'i32');
        }
        setpv(pR, r);
        setpv(pG, g);
        setpv(pB, b);
};
Module.graphics.erase_screen = () => { console.log(`erase_screen ${[]}`) };
Module.graphics.draw_turtle = (heading) => {
    console.log(`draw_turtle ${heading}`);
    const turtle = document.getElementById('logoTurtle');
    turtle.setAttribute('transform', `rotate(${heading})`);
};
Module.graphics.get_label_size = () => {
    console.log('get_label_size');
    const pen_info = Module.graphics.pen_info;
    const theLetterM = document.getElementById('theLetterM');
    const { width } = theLetterM.getBoundingClientRect();
    return { width: Math.floor(width), height: pen_info.fntsz };
};
Module.graphics.adjust_label_height = (h) => {
    console.log(`adjust_label_height ${h}`);
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
    el.style.setProperty('stroke-linecap', 'round');
    el.style.setProperty('stroke-linejoin', 'round');
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
    return Module.graphics.lastclick?.button || 0;
}
Module.graphics.web_get_button = () => {
    const b = Module.graphics.lastclick?.button;
    if (b) {
        delete Module.graphics.lastclick.button;
    }
    return b || 0;
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