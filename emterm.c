/* Minimal stand-in for term.c, for the WebAssembly/browser proof-of-concept
 * build. The core reader/parser uses plain C stdio (getc(stdin)), which
 * Emscripten already emulates -- this file only needs to satisfy the
 * handful of terminal-control primitives term.c normally provides, none of
 * which have a meaningful equivalent in a browser tab (no raw/cbreak mode,
 * no ANSI cursor positioning).
 */

#include "logo.h"
#include "globals.h"

/* Normally defined in term.c, which this file replaces. x_coord/y_coord
 * track a virtual text-cursor position for CURSOR/SETCURSOR -- not real
 * screen coordinates here, but the interpreter core reads/writes them
 * regardless of whether a terminal exists. interactive selects prompt-
 * printing behaviour in the reader. */
int x_coord = 0, y_coord = 0, x_max = 80, y_max = 24;
int interactive = 0, tty_charmode = 0;

void term_init(void) {
}

void charmode_on(void) {
}

void charmode_off(void) {
}

NODE *lcleartext(NODE *args) {
    return(UNBOUND);
}

NODE *lcursor(NODE *args) {
    return(cons(make_intnode(0), cons(make_intnode(0), NIL)));
}

NODE *lsetcursor(NODE *args) {
    return(UNBOUND);
}

NODE *lsetmargins(NODE *args) {
    return(UNBOUND);
}

NODE *lstandout(NODE *args) {
    return(car(args));
}
