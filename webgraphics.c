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

char *LogoPlatformName = "WebAssembly";

int mouse_x;
int mouse_y;
int click_x;
int click_y;
int lastbutton;
int button;

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

// EM_JS(int, web_get_mouse_x, (), {
//    return Module.graphics.web_get_mouse_x();
// });

// EM_JS(int, web_get_mouse_y, (), {
//    return Module.graphics.web_get_mouse_y();
// });

// EM_JS(int, web_get_click_x, (), {
//    return Module.graphics.web_get_click_x();
// });

// EM_JS(int, web_get_click_y, (), {
//    return Module.graphics.web_get_click_y();
// });

// EM_JS(int, web_get_button, (), {
//    return Module.graphics.web_get_button();
// });

// EM_JS(int, web_get_buttonp, (), {
//    return Module.graphics.web_get_buttonp();
// });

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

// EM_JS(void, web_graphics_init, (), {
//    if (typeof document !== 'undefined') {
//       Module.graphics.graphics_init();
//    }
// });

// void graphics_init()
// {
//    web_graphics_init();
//    set_pen_color(7);
//    set_pen_vis(0);
// }

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

extern int LOGO_GIL;

extern int yield_enabled;
extern int in_eval_save;
extern NODE* proc_name_arg(NODE*);

bool animating = false;
EMSCRIPTEN_KEEPALIVE int animationFrameID = 0;
double animationFrameTime = 0;

NODE* get_action(NODE* action_node) {
 	NODE* line = valnode__caseobj(action_node);
	if (line == UNBOUND || line == NIL) return NIL;
   return line;
}

NODE* onmousedown(uievent* evt) {
   click_x = mouse_x = evt->mouseevent.x;
   click_y = mouse_y = evt->mouseevent.y;
   lastbutton = evt->mouseevent.buttonnum;
   button = 1;
   return get_action(Buttonact);
}

NODE* onmouseup(uievent* evt) {
   mouse_x = evt->mouseevent.x;
   mouse_y = evt->mouseevent.y;
   button = 0;
   return NIL;
}

NODE* onmousemove(uievent* evt) {
   mouse_x = evt->mouseevent.x;
   mouse_y = evt->mouseevent.y;
   return NIL;
}

NODE* onkeydown(uievent* evt) {
   readchar_lookahead_buf = evt->keyboardevent.key;
   return get_action(Keyact);
   return NIL;
}

extern int stop_quietly_flag;

void onpauselogo() {
   charmode_off();
   to_pending = 0;
   lpause(NIL);
}

void onstoplogo() {
   charmode_off();
   to_pending = 0;
   if (!stop_quietly_flag)
      err_logo(STOP_ERROR,NIL);
   stop_quietly_flag = 0;
   unblock_input();
}

NODE* onanimate(uievent* evt) {
   double time = evt->animationevent.time;
   // FIXME suppress zero frame
   double timedelta = time - animationFrameTime;
   setvalnode__caseobj(Animationtime, make_floatnode(time));
   setvalnode__caseobj(Animationdelta, make_floatnode(timedelta));
   animationFrameTime = time;
   yield_enabled = 0;
   return get_action(Animationact);
}

NODE* dispatch_uievent(uievent* evt) {
   NODE* r = NIL;
	if (!evt) {
      return NIL;
	}
   switch (evt->type) {
      case animate:
         r = onanimate(evt);
         break;
      case pauselogo:
         onpauselogo();
         break;
      case stoplogo:
         onstoplogo();   
         break;
      case mousemove: 
         r = onmousemove(evt);
         break;
      case mousedown: 
         r = onmousedown(evt);
         break;
      case mouseup: 
         r = onmouseup(evt);
         break;
      case keydown:
         r = onkeydown(evt);
         break;
      default:
         break;
   }
   // fix_turtle_shownness();
   return r;
}

EM_ASYNC_JS(bool, getDeviceOrientationPermission, (), {
   //  if (window.DeviceOrientationEvent.requestPermission) {
      const dlg = document.getElementById('orientationPermissionDialog');
      const ok = dlg.querySelector('button.ok');
      const cancel = dlg.querySelector('button.cancel');
      dlg.showModal();
      try
      {
         const canceled = new Promise((res, rej) => {
            cancel.addEventListener('mousedown', () => {
               dlg.close();
               res(false);
            });
         });
         const oked = new Promise((res,rej)=>ok.addEventListener('mousedown', async ()=>{
            const r = await window.DeviceOrientationEvent.requestPermission();
            if (r === 'granted') {
               window.addEventListener("deviceorientation", Module.ondeviceorientation);
               await new Promise((res, rej)=>
                  window.addEventListener("deviceorientation", res, {once:true}));
               res(true);
            }
            else {
               console.error('DeviceOrientationEvent transient activation', r);
               res(false);
            }
         }, {once: true}));
         return await Promise.any([oked, canceled]);
      }
      catch (error) {
         console.error(error.stack);
         return false;
      }
      finally {
         dlg.close();
      }
});

NODE* ldeviceorientation(NODE* argl) {
   double a, b, g;
   static bool initialized;

   if (!initialized) { 
      initialized = getDeviceOrientationPermission();
      if (!initialized) {
         err_logo(CANT_OPEN_ERROR, make_static_strnode("window.DeviceOrientationEvent"));
         return (UNBOUND);
      }
   }

   EM_ASM({
      const o = Module.graphics.deviceOrientation;
      setValue($0, o.alpha, 'double');
      setValue($1, o.beta, 'double');
      setValue($2, o.gamma, 'double');
   }, &a, &b, &g);

   return(
      cons(make_floatnode(a),
		cons(make_floatnode(b), 
		cons(make_floatnode(g), 
   NIL))));
}