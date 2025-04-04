const SVG = 'http://www.w3.org/2000/svg';

Module.preRun = () => {
    const PREFIX = "/share/ucblogo";
    ENV.LOGOLIB = PREFIX + "/logolib";
    ENV.LOGOHELP = PREFIX + "/helpfiles";
    ENV.CSLS = PREFIX + "/csls";
};
Module.env = {};
Module.graphics = {};
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
};
Module.graphics.prepare_to_draw = () => { console.log("prepare_to_draw") };
Module.graphics.done_drawing = () => { console.log("done_drawing") };
Module.graphics.prepare_to_exit = (v) => { console.log(`prepare_to_exit(${v})`) };
Module.graphics.clear_screen = () => {
    console.log("clear_screen");
    if (typeof document === 'undefined') return;
    const dr = document.getElementById('logoDrawingContainer');
    const g = document.getElementById('logoDrawingElements');
    dr.removeChild(g);
    const newg = document.createElementNS(SVG, 'g')
    newg.id = 'logoDrawingElements';
    dr.appendChild(newg);
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
        const ld = document.getElementById('logoDrawingElements')
        const el = document.createElementNS(SVG, 'line');
        el.setAttribute("stroke-width", pen_info.sz);
        el.style.setProperty("mix-blend-mode", pen_info.mode);
        el.style.setProperty('stroke', `var(--logo-color-${pen_info.c})`);
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
Module.graphics.pen_erase = () => { console.log(`pen_erase ${[]}`) };
Module.graphics.pen_down = () => { console.log(`pen_down ${[]}`) };
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
Module.graphics.set_palette = (i, c1, c2, c3) => { console.log(`set_palette ${[i, c1, c2, c3]}`) };
Module.graphics.get_pallette = () => { console.log(`get_pallette ${[]}`) };
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