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
    x: 0,
    y: 0,
    h: 0,
    sz: 1,
    c: 0,
    fntsz: 12
};
Module.graphics.prepare_to_draw = () => { console.log("prepare_to_draw") };
Module.graphics.done_drawing = () => { console.log("done_drawing") };
Module.graphics.prepare_to_exit = (v) => { console.log(`prepare_to_exit(${v})`) };
Module.graphics.clear_screen = () => {
    console.log("clear_screen");
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
    console.log(`line_to ${[x, y]}`);
    const g = Module.graphics;
    const pen_info = g.pen_info;
    const ld = document.getElementById('logoDrawingElements')
    const line = document.createElementNS(SVG, 'line');
    line.setAttribute("stroke-width", pen_info.sz);
    line.setAttribute("pen-color", pen_info.c);
    line.setAttribute("x1", x);
    line.setAttribute("y1", y);
    line.setAttribute("x2", pen_info.x);
    line.setAttribute("y2", pen_info.y);
    ld.appendChild(line);
    pen_info.x = x;
    pen_info.y = y;
    const turtle = document.getElementById('logoTurtleTranslated');
    turtle.setAttribute('transform', `translate(${x - 6}, ${y - 9})`);
};
Module.graphics.move_to = (x, y) => {
    console.log(`move_to ${[x, y]}`)
    Module.graphics.pen_info.x = x;
    Module.graphics.pen_info.y = y;
    const turtle = document.getElementById('logoTurtleTranslated');
    turtle.setAttribute('transform', `translate(${x - 6}, ${y - 9})`);
};
Module.graphics.label = (s) => {
    console.log(`label ${[s]}`);
    const g = Module.graphics;
    const pen_info = g.pen_info;
    const ld = document.getElementById('logoDrawingElements')
    const text = document.createElementNS(SVG, 'text');
    text.textContent = UTF8ToString(s);
    text.setAttribute('x', pen_info.x);
    text.setAttribute('y', pen_info.y);
    ld.appendChild(text);
};
Module.graphics.set_pen_vis = (v) => { console.log(`set_pen_vis ${[v]}`) };
Module.graphics.set_pen_mode = (m) => { console.log(`set_pen_mode ${[m]}`) };
Module.graphics.set_pen_color = (c) => {
    console.log(`set_pen_color ${[c]}`);
    Module.graphics.pen_info.c = c;
};
Module.graphics.set_pen_width = (w) => {
    console.log(`set_pen_width ${[w]}`);
    Module.graphics.pen_info.sz = w;
};
Module.graphics.set_pen_height = (w) => {
    console.log(`set_pen_height ${[w]}`);
    Module.graphics.pen_info.sz = w;
};
Module.graphics.set_pen_x = () => { console.log(`set_pen_x ${[]}`) };
Module.graphics.set_pen_y = () => { console.log(`set_pen_y ${[]}`) };
Module.graphics.set_back_ground = (c) => { console.log(`set_back_ground ${[c]}`) };
Module.graphics.pen_reverse = () => { console.log(`pen_reverse ${[]}`) };
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
}