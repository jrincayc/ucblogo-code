#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <emscripten.h>
#include <emscripten/html5.h>

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
   if (typeof document !== 'undefined') {
      Module.graphics.graphics_init();
   }
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

// EM_JS(int, definejs, ((const char* src))) {
//    if (!Module.jswords) {
//       Module.jswords = [];
//    }
//    const idx = Module.jswords.length;
//    Module.jswords.push(eval(`UTF8ToString(src)`);
//    return idx;
// }

extern int yield_enabled;
extern int in_eval_save;

bool _animationframe(double time, void *userdata)
{
   // EM_ASM({console.log('in _animationframe')});
   NODE* expresn = (NODE*) userdata;
   static NODE ndTime;
   static NODE wordcons;
   static NODE argcons;
   static int initialized;
   if (!initialized) {
      ndTime.node_type = FLOATT;
      wordcons.node_type = CONS | NT_TREE;
      argcons.node_type = CONS;
      setcdr(&wordcons, &argcons);
      setcar(&argcons, &ndTime);
      setcdr(&argcons, NIL);
      setcdr(&ndTime, NIL);
      initialized = 1;
   }

   if (inside_gc || in_eval_save) {
      // EM_ASM({console.log('_animationframe inside_gc || in_eval_save')});
      if (int_during_gc == 0)
         int_during_gc = 3;
      return true;
   }
   // put together the word and the argument into a... sentence?
   if (expresn != UNBOUND && expresn != NIL)
   {
      setcar(&wordcons, expresn);
      setfloat(&ndTime, time);
      if (inside_evaluator)
      {
         // EM_ASM({console.log('_animationframe inside_eval')});
         eval_animationframe = &wordcons;
      }
      else
      {
         if (NOT_THROWING) {
            yield_enabled = 0;
            // EM_ASM({
               // console.log('_animationframe calling eval_driver')
            // });
            eval_driver(&wordcons);
            // fix_turtle_shownness();
            yield_enabled = 1;
         }
      }
   }
   else {
      // EM_ASM({console.log('_animationframe line unbound or nil')});
   }
   return true;
}

extern NODE* find_instance(NODE*);
extern NODE* proc_name_arg(NODE*);

NODE *lanimationframe(NODE * word)
{
   EM_ASM({console.log('in lanimationframe')});
   // calls requestAnimationFrame with a Logo callback proc
   // does a minor GC first as a side effect
   // word: name of callback procedure

   // TODO check that word is a proc that takes 1 arg

   // FIXME NODE* word falls off the stack and is invisible to the GC after lanimationframe returns.
   // Lookup the symbol for the proc named word and pass that or the proc
   // itself; these will not be freed unless the proc is erased before it
   // is even called by requestAnimationFrame which would be an error anyway.
   
   NODE* name = proc_name_arg(word);
   if (!NOT_THROWING) {
      return UNBOUND;
   }
	name=intern(name);
	check_library(name);
	NODE* val = procnode__caseobj(name);
	if (val == UNDEFINED) {
	    err_logo(DK_HOW_UNREC,name);
	    return UNBOUND;
	} else if (is_prim(val)) {
	    err_logo(IS_PRIM,name);
	    return UNBOUND;
	}
   if (
      getint(minargs__procnode(val)) != 1
      || getint(dfltargs__procnode(val)) != 1
      || getint(maxargs__procnode(val)) != 1
   ) {
      err_logo(TOO_MUCH, name);
      return UNBOUND;
   }

   int r = emscripten_request_animation_frame(&_animationframe, name);
   // EM_ASM({console.log('request_animation_frame', $0)}, r);
   return UNBOUND;
}