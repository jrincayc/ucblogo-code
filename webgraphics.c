#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <emscripten.h>

#include "logo.h"
#include "globals.h"
#include "webgraphics.h"

int bg;

pen_info xgr_pen;

char *LogoPlatformName = "Web Assembly/SVG";

void nop(int a, int b, int c, int d)
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

EM_JS(void, set_pen_vis, (int v), {
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

EM_JS(void, web_set_back_ground, (int c), {
   Module.graphics.set_back_ground(c);
});

void set_back_ground(int c)
{
   bg = c;
   web_set_back_ground(c);
}

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

EM_JS(void, label, (char *s), {
   Module.graphics.label(s);
});

EM_JS(void, tone, (int pitch, int duration), {
   Module.graphics.tone(pitch, duration);
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

EM_JS(void, set_palette, (int i, unsigned int R, unsigned int G, unsigned int B), {
   Module.graphics.set_palette(i, R, G, B);
});

EM_JS(void, get_palette, (int i, uint32_t *pR, uint32_t *pG, uint32_t *pB), {
   Module.graphics.get_palette(i, pR, pG, pB);
});

EM_JS(void, erase_screen, (), {
   Module.graphics.erase_screen();
});

EM_JS(int, web_get_mouse_x, (), {
   return Module.graphics.web_get_mouse_x();
});

EM_JS(int, web_get_mouse_y, (), {
   return Module.graphics.web_get_mouse_y();
});

EM_JS(int, web_get_click_x, (), {
   return Module.graphics.web_get_click_x();
});

EM_JS(int, web_get_click_y, (), {
   return Module.graphics.web_get_click_y();
});

EM_JS(int, web_get_button, (), {
   return Module.graphics.web_get_button();
});

EM_JS(int, web_get_buttonp, (), {
   return Module.graphics.web_get_buttonp();
});

EM_JS(void, web_draw_turtle, (int heading), {
   Module.graphics.draw_turtle(heading);
});

EM_JS(void, adjust_label_height, (int label_height), {
   Module.graphics.adjust_label_height(label_height);
});

NODE *lsetlabelheight(NODE *arg)
{
   NODE *val = integer_arg(arg);
   int label_height = getint(val);
   adjust_label_height(label_height);
   return (UNBOUND);
}

EM_JS(void, get_label_size, (int *w, int *h), {
   const {width, height} = Module.graphics.get_label_size();
   setValue(h, height, 'i32');
   setValue(w, width, 'i32');
});

NODE *llabelsize(NODE *arg)
{
   int w, h;
   get_label_size(&w, &h);
   return cons(make_intnode(w / x_scale),
               cons(make_intnode(h / y_scale), NIL));
}

EM_JS(void, web_hide_turtle, (), {
   Module.graphics.hide_turtle();
});

EM_JS(void, web_show_turtle, (), {
   Module.graphics.show_turtle();
});

EM_JS(void, web_filled_begin, (int color), {
   Module.graphics.filled_begin(color);
});

EM_JS(void, web_filled_end, (), {
   Module.graphics.filled_end();
});

EM_JS(void, web_filled_add_point, (int x, int y), {
   Module.graphics.filled_add_point(x, y);
});

void doFilled(int fillcolor, int count, mypoint_t *points) {
   web_filled_begin(fillcolor);
   for (size_t n = 0; n < count; n++){
      const mypoint_t pnt = points[n];
      web_filled_add_point(pnt.x, pnt.y);
   }
   web_filled_end();
}

EM_JS(void, web_graphics_init, (), {
   Module.graphics.graphics_init();
});

void graphics_init()
{
   web_graphics_init();
   set_pen_color(7);
   set_pen_vis(0);
}

EM_JS(int, web_keyp, (), {
   //FIXME
   return 0;
});