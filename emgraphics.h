
/* Turtle-graphics backend for the WebAssembly/browser proof-of-concept,
 * mirroring nographics.h's macro surface but routing drawing calls into
 * JavaScript (an HTML5 canvas 2D context) instead of doing nothing. See
 * emgraphics.c for the EM_JS implementations.
 */

#define GR_SIZE 60000

#define prepare_to_draw em_prepare_to_draw()
#define done_drawing em_done_drawing()

/* The turtle is not painted into the picture (no XOR pen here): the backend
 * captures its outline between these two calls and draws it on top. */
#define prepare_to_draw_turtle em_turtle_begin()
#define done_drawing_turtle em_turtle_end()

/* Matches the actual <canvas id="turtleCanvas" width="440" height="440">
 * element in the surrounding HTML page (see wasm-poc/index.html and its
 * copies in the Kolibri/XBlock deployments) 1:1 -- em_move_to()/em_line_to()
 * pass coordinates straight through to ctx.moveTo()/lineTo() with no
 * JS-side scaling, so this constant IS the real on-screen pixel range, not
 * an abstract logical-coordinate choice. Previously 200 (turtle range
 * -100..100), which wasted more than half the canvas's actual resolution
 * and made ordinary scripts (e.g. "herhaal 5 [vt 150 re 144]", a plain
 * 5-pointed star) exceed the -100..100 range and wrap/reappear on the
 * opposite edge well before finishing. Real desktop UCBLogo backends
 * (win32trm.h, wxGraphics.h, xgraphics.h) don't hardcode a virtual size
 * at all -- they query the actual window's pixel dimensions at runtime;
 * 440 here is the WASM-equivalent fixed counterpart to that, since all
 * three deployments currently share one fixed-size canvas element. */
#define screen_left 0
#define screen_right 440
#define screen_top 0
#define screen_bottom 440

#define screen_height (1 + screen_bottom - screen_top)
#define screen_width (1 + screen_right - screen_left)

#define screen_x_center (screen_left + (screen_width)/2)
#define screen_y_center (screen_top + (screen_height)/2)

#define turtle_left_max ((screen_left) - (screen_x_center))
#define turtle_right_max ((screen_right) - (screen_x_center))
#define turtle_top_max ((screen_y_center) - (screen_top))
#define turtle_bottom_max ((screen_y_center) - (screen_bottom))

#define screen_x_coord ((screen_x_center) + turtle_x)
#define screen_y_coord ((screen_y_center) - turtle_y)

#define turtle_height 18
#define turtle_half_bottom 6.0
#define turtle_side 19.0

#define clear_screen em_clear_screen()

#define line_to(x,y) em_line_to(x,y)
#define move_to(x,y) em_move_to(x,y)
#define draw_string(s) nop()
#define set_pen_vis(v) em_set_pen_vis(v)
#define set_pen_mode(m) nop()
#define set_pen_color(c) em_set_pen_color(c)
#define set_pen_width(w) em_set_pen_width(w)
#define set_pen_height(h) nop()
#define set_pen_x(x) nop()
#define set_pen_y(y) nop()
#define set_back_ground(c) em_set_back_ground(c)

typedef struct { int dummy; } pen_info;

#define p_info_x(p) p.dummy
#define p_info_y(p) p.dummy

#define pen_width pw
#define pen_height ph
#define pen_color pc
#define pen_mode pm
#define pen_vis pv
#define pen_x px
#define pen_y py
#define get_node_pen_pattern make_intnode(0)
#define back_ground bg

#define pen_reverse nop()
#define pen_erase nop()
#define pen_down nop()

#define button FALSE
#define mouse_x 0
#define mouse_y 0

#define full_screen nop()
#define split_screen nop()
#define text_screen nop()

#define save_pen(p) nop()
#define restore_pen(p) nop()
#define plain_xor_pen() nop()
#define label(s) nop()
#define tone(p,d) nop()
#define get_pen_pattern(p) nop()
#define set_pen_pattern(p) nop()
#define set_list_pen_pattern(p) nop()

extern int pw, ph, pc, pm, pv, px, py, bg;
extern void nop();

extern void em_turtle_begin(void);
extern void em_turtle_end(void);
extern void em_turtle_hide(void);
extern void em_prepare_to_draw(void);
extern void em_done_drawing(void);
extern void em_clear_screen(void);
extern void em_line_to(double x, double y);
extern void em_move_to(double x, double y);
extern void em_set_pen_vis(int v);
extern void em_set_pen_color(int c);
extern void em_set_pen_width(int w);
extern void em_set_back_ground(int c);

/* Used by ledit() in wrksp.c in place of fork()+execlp() of an external
 * editor: hands the temp file (already containing the current procedure
 * definition) to the host app, which shows its own blocking edit UI and
 * rewrites the file with the result before returning. The Android backend
 * (androidjni.c) implements this with a real blocking dialog; the WASM/
 * browser backend (emgraphics.c) uses EM_ASYNC_JS + Asyncify to show an
 * HTML modal (see wasm-poc/index.html's window.__logoShowEditDialog) while
 * appearing to block synchronously from C's point of view. */
extern void em_edit_file(char *tmp_filename);

#define logofill() nop()
#define set_palette(i, r, g, b) nop()
#define get_palette(i, r, g, b) nop()
#define erase_screen() nop()
