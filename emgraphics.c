/* Turtle-graphics backend for the WebAssembly/browser proof-of-concept.
 * Implements emgraphics.h's em_* hooks by calling into JavaScript (via
 * Emscripten's EM_JS) to draw on a <canvas> the surrounding HTML page is
 * expected to expose as the global `window.logoCanvasCtx` (a 2D context).
 * Palette matches TurtleCanvas::colors in wxTurtleGraphics.cpp exactly, so
 * SETPENCOLOR/etc. behave the same as the desktop build for the 16
 * standard colors.
 */

#include <emscripten.h>

int pw = 1, ph = 1, pc = 0, pm = 0, pv = 0, px = 0, py = 0, bg = 7;
char *LogoPlatformName = "WebAssembly";

void nop(void) {
}

EM_JS(void, em_prepare_to_draw, (void), {
});

EM_JS(void, em_done_drawing, (void), {
});

EM_JS(void, em_clear_screen, (void), {
    if (typeof window.logoCanvasCtx === 'undefined') return;
    var ctx = window.logoCanvasCtx;
    if (window.__logoTurtle) window.__logoTurtle.saved = null;   /* whole canvas is repainted below */
    ctx.fillStyle = window.logoBackgroundColor || 'white';
    ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);
    ctx.strokeStyle = 'black';
    ctx.lineWidth = 1;
    ctx.beginPath();
});

/* --- Turtle ---------------------------------------------------------------
 * UCBLogo draws and erases the turtle with an XOR pen, which a <canvas> doesn't
 * have. Instead graphics.c's draw_turtle_helper() brackets its outline between
 * em_turtle_begin()/em_turtle_end(); we capture those points here (they never
 * go into the drawing), and paint them on top after saving the pixels below.
 * Any real drawing first "lifts" the turtle by restoring those pixels. */
static int capturing = 0;

EM_JS(void, em_turtle_js, (int op, double x, double y), {
    var ctx = window.logoCanvasCtx;
    if (typeof ctx === 'undefined') return;
    var T = window.__logoTurtle || (window.__logoTurtle = { saved: null, pts: [], poly: null, vis: false });
    var lift = function () {
        if (T.saved) { ctx.putImageData(T.saved.d, T.saved.x, T.saved.y); T.saved = null; }
    };
    var paint = function () {
        lift();
        if (!T.vis || !T.poly || T.poly.length < 3) return;
        var x0 = 1e9, y0 = 1e9, x1 = -1e9, y1 = -1e9, i;
        for (i = 0; i < T.poly.length; i++) {
            x0 = Math.min(x0, T.poly[i][0]); x1 = Math.max(x1, T.poly[i][0]);
            y0 = Math.min(y0, T.poly[i][1]); y1 = Math.max(y1, T.poly[i][1]);
        }
        x0 = Math.max(0, Math.floor(x0) - 3); y0 = Math.max(0, Math.floor(y0) - 3);
        x1 = Math.min(ctx.canvas.width, Math.ceil(x1) + 3); y1 = Math.min(ctx.canvas.height, Math.ceil(y1) + 3);
        if (x1 <= x0 || y1 <= y0) return;
        T.saved = { x: x0, y: y0, d: ctx.getImageData(x0, y0, x1 - x0, y1 - y0) };
        var path = new Path2D();
        path.moveTo(T.poly[0][0], T.poly[0][1]);
        for (i = 1; i < T.poly.length; i++) path.lineTo(T.poly[i][0], T.poly[i][1]);
        path.closePath();
        ctx.save();
        var bg = (window.logoBackgroundColor || 'white').toString();
        ctx.strokeStyle = (bg === 'rgb(0,0,0)' || bg === 'black') ? 'white' : 'black';
        ctx.lineWidth = 1;
        ctx.stroke(path);
        ctx.restore();
    };
    switch (op) {
    case 0: lift(); break;                                   /* lift the turtle before drawing */
    case 1: T.pts = []; break;                               /* begin capture */
    case 2: T.pts.push([x, y]); break;                       /* outline point */
    case 3: T.poly = T.pts; T.vis = true; paint(); break;    /* end capture: show */
    case 4: T.vis = false; lift(); break;                    /* hide */
    }
});

void em_turtle_begin(void) { capturing = 1; em_turtle_js(1, 0, 0); }
void em_turtle_end(void)   { capturing = 0; em_turtle_js(3, 0, 0); }
void em_turtle_hide(void)  { em_turtle_js(4, 0, 0); }

EM_JS(void, em_move_to_js, (double x, double y), {
    if (typeof window.logoCanvasCtx === 'undefined') return;
    window.logoCanvasCtx.moveTo(x, y);
});

void em_move_to(double x, double y) {
    if (capturing) { em_turtle_js(2, x, y); return; }
    em_turtle_js(0, 0, 0);
    em_move_to_js(x, y);
}

EM_JS(void, em_line_to_js, (double x, double y), {
    if (typeof window.logoCanvasCtx === 'undefined') return;
    var ctx = window.logoCanvasCtx;
    ctx.lineTo(x, y);
    ctx.stroke();
    ctx.beginPath();
    ctx.moveTo(x, y);
});

void em_line_to(double x, double y) {
    if (capturing) { em_turtle_js(2, x, y); return; }
    em_turtle_js(0, 0, 0);
    em_line_to_js(x, y);
}

EM_JS(void, em_set_pen_vis, (int v), {
});

EM_JS(void, em_set_pen_color, (int c), {
    if (typeof window.logoCanvasCtx === 'undefined') return;
    var palette = [
        'rgb(0,0,0)', 'rgb(0,0,255)', 'rgb(0,255,0)', 'rgb(0,255,255)',
        'rgb(255,0,0)', 'rgb(255,0,255)', 'rgb(255,255,0)', 'rgb(255,255,255)',
        'rgb(155,96,59)', 'rgb(197,136,18)', 'rgb(100,162,64)', 'rgb(120,187,187)',
        'rgb(255,149,119)', 'rgb(144,113,208)', 'rgb(255,163,0)', 'rgb(183,183,183)'
    ];
    if (palette[c]) window.logoCanvasCtx.strokeStyle = palette[c];
});

EM_JS(void, em_set_pen_width, (int w), {
    if (typeof window.logoCanvasCtx === 'undefined') return;
    window.logoCanvasCtx.lineWidth = w > 0 ? w : 1;
});

EM_JS(void, em_set_back_ground, (int c), {
    var palette = [
        'rgb(0,0,0)', 'rgb(0,0,255)', 'rgb(0,255,0)', 'rgb(0,255,255)',
        'rgb(255,0,0)', 'rgb(255,0,255)', 'rgb(255,255,0)', 'rgb(255,255,255)',
        'rgb(155,96,59)', 'rgb(197,136,18)', 'rgb(100,162,64)', 'rgb(120,187,187)',
        'rgb(255,149,119)', 'rgb(144,113,208)', 'rgb(255,163,0)', 'rgb(183,183,183)'
    ];
    if (!palette[c] || typeof window.logoCanvasCtx === 'undefined') return;
    window.logoBackgroundColor = palette[c];
    var ctx = window.logoCanvasCtx;
    if (window.__logoTurtle) window.__logoTurtle.saved = null;
    ctx.fillStyle = palette[c];
    ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);
});

/* Shows a blocking (from C's perspective) edit dialog for the procedure
 * text already staged in tmp_filename by ledit() (wrksp.c), mirroring the
 * Android backend (androidjni.c's em_edit_file() + MainActivity's
 * showEditDialogBlocking()). EM_ASYNC_JS lets this "return" only after the
 * user closes the page's own HTML modal (window.__logoShowEditDialog, see
 * wasm-poc/index.html), while C-side execution appears to block the whole
 * time -- requires -sASYNCIFY=1 at link time. Leaving the temp file
 * untouched on cancel makes ledit()'s unconditional reload afterwards a
 * no-op, same cancel behaviour as the Android backend. */
EM_ASYNC_JS(void, em_edit_file, (char *tmp_filename), {
    var path = UTF8ToString(tmp_filename);
    var initial = '';
    try {
        initial = FS.readFile(path, { encoding: 'utf8' });
    } catch (e) {
        initial = '';
    }
    if (typeof window.__logoShowEditDialog !== 'function') return;
    var result = await window.__logoShowEditDialog(initial);
    if (result !== null) {
        FS.writeFile(path, result);
    }
});
