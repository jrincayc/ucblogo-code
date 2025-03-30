#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <emscripten.h>

#include "logo.h"
#include "globals.h"
#include "webgraphics.h"

int pw, ph, pc, pm, pv, px, py, bg;

char *LogoPlatformName = "Web Assembly/SVG";

void nop(int, int, int, int)
{
}

EM_JS(void, web_prepare_to_draw, (), {
   Module.graphics.prepare_to_draw();
});

EM_JS(void, web_done_drawing, (), {
   Module.graphics.done_drawing();
});

EM_JS(void, prepare_to_exit, (BOOLEAN v), {
   Module.graphics.prepare_to_exit(v);
});

EM_JS(void, web_clear_screen, (), {
   Module.graphics.clear_screen();
});

EM_JS(void, line_to, (FIXNUM x, FIXNUM y), {
   Module.graphics.line_to(x, y);
});

EM_JS(void, move_to, (FIXNUM x, FIXNUM y), {
   Module.graphics.move_to(x, y);
});

EM_JS(void, draw_string, (char *s), {
   Module.graphics.draw_str(s);
});

EM_JS(void, set_pen_vis, (BOOLEAN v), {
   Module.graphics.set_pen_vis(v);
});

EM_JS(void, set_pen_mode, (int m), {
   Module.graphics.set_pen_mode(m);
});

EM_JS(void, set_pen_color, (int c), {
   Module.graphics.set_pen_color(c);
});

EM_JS(void, set_pen_width, (int w), {
   Module.graphics.set_pen_width(w);
});

EM_JS(void, set_pen_height, (int h), {
   Module.graphics.set_pen_height(h);
});

EM_JS(void, set_pen_x, (int x), {
   Module.graphics.set_pen_x();
});

EM_JS(void, set_pen_y, (int y), {
   Module.graphics.set_pen_y();
});

EM_JS(void, set_back_ground, (int c), {
   Module.graphics.set_back_ground(c);
});

EM_JS(void, web_pen_reverse, (), {
   Module.graphics.pen_reverse();
});

EM_JS(void, web_pen_erase, (), {
   Module.graphics.pen_erase();
});

EM_JS(void, web_pen_down, (), {
   Module.graphics.pen_down();
});

EM_JS(void, web_full_screen, (), {
   Module.graphics.full_screen();
});

EM_JS(void, web_split_screen, (), {
   Module.graphics.split_screen();
});

EM_JS(void, web_text_screen, (), {
   Module.graphics.text_screen();
});

EM_JS(void, save_pen, (pen_info * p), {
   // will this give us a pointer or an object?
   Module.graphics.save_pen(p);
});

EM_JS(void, restore_pen, (pen_info * p), {
   Module.graphics.restore_pen(p);
});

EM_JS(void, plain_xor_pen, (), {
   Module.graphics.plain_xor_pen();
});

EM_JS(void, label, (char *s), {
   Module.graphics.label(s);
});

EM_JS(void, tone, (int pitch, int duration), {
   Module.graphics.tone(pitch, duration);
});

EM_JS(void, set_pen_pattern, (char *pat), {
   Module.graphics.set_pen_pattern(pat);
});

EM_JS(char *, get_pen_pattern, (char *buf), {
   Module.graphics.get_pen_pattern(pat);
   // FIXME how to return string buffer?
});

EM_JS(void, set_list_pen_pattern, (char *pat), {
   Module.graphics.set_list_pen_pattern(pat);
});

EM_JS(void, web_prepare_to_draw_turtle, (), {
   Module.graphics.prepare_to_draw_turtle();
});

EM_JS(void, web_done_drawing_turtle, (), {
   Module.graphics.web_done_drawing_turtle();
});

EM_JS(void, logofill, (), {
   Module.graphics.logofill();
});

// FIXME
EM_JS(void, set_palette, (int i, char *c1, char *c2, char *c3), {
   Module.graphics.set_palette(i, c1, c2, c3);
});

// FIXME
EM_JS(void, get_palette, (int *i, char **c1, char **c2, char **c3), {
   Module.graphics.get_pallette();
});

EM_JS(void, erase_screen, (), {
   Module.graphics.erase_screen();
});
