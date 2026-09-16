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
    ctx.fillStyle = window.logoBackgroundColor || 'white';
    ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);
    ctx.strokeStyle = 'black';
    ctx.lineWidth = 1;
    ctx.beginPath();
});

EM_JS(void, em_move_to, (double x, double y), {
    if (typeof window.logoCanvasCtx === 'undefined') return;
    window.logoCanvasCtx.moveTo(x, y);
});

EM_JS(void, em_line_to, (double x, double y), {
    if (typeof window.logoCanvasCtx === 'undefined') return;
    var ctx = window.logoCanvasCtx;
    ctx.lineTo(x, y);
    ctx.stroke();
    ctx.beginPath();
    ctx.moveTo(x, y);
});

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
