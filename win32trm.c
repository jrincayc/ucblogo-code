/* -*-C-*-
 * win32trm.c -- Module to provide Win32 API compliant graphics routines
 * to UCB Logo.  This module should contain Win32 versions of all necessary
 * drawing functions, as well as the WinMain entry point.
 *
 * Since this is a part of work being done FOR the University, I suppose
 * this part is (c) 1996 Regents of the University of California.
 * All rights reserved and all that jazz.
 *
 */
/*
 * By definition, this file is used for MS Windows versions of the
 * program, so no tests for #ifdef mac (for example) are needed.
 */

#include <windows.h>
#include <string.h>
#include <math.h>

#include "logo.h"
#include "globals.h"
#include "win32trm.h"

// #define WIN32_S

LRESULT CALLBACK ParentWindowFunc(HWND, UINT, WPARAM, LPARAM);

char szWinName[] = "UCBLogo";
char szGraphWinName[] = "UCBLogoGraph";
char szConWinName[] = "UCBLogoConsole";

pen_info turtlePen;

HBRUSH hbrush;

HBITMAP hbitG, hbitmG, hbitC, hbitmC;
RECT allRect;

HDC GraphDC, memGraphDC, ConDC, memConDC;
HWND hGraphWnd, hConWnd, hMainWnd;

/* color and palette related information */
static COLORREF palette[200];
FIXNUM back_ground, pen_color, old_pen_color;

int maxX, maxY, greeted = 0, oldWidth, oldHeight;
int pre_turtle_pen_mode;
long Xsofar, Ysofar;

BOOLEAN in_erase_mode, update_pos, seen_once = FALSE;

int in_graphics_mode, in_splitscreen;

static char **win32_lines; // for recording what should appear in the text win
static char input_lines[2][200];

char *read_line, buffered_char;
int cur_line = 0, cur_index = 0, read_index, hpos = 0, margin, cur_len;
int char_mode = 0, input_line = 0, input_index = 0;

int line_avail = FALSE, char_avail = FALSE;

#ifdef WIN32_DEBUG
void WinDebug(char *s) {
    ndprintf("%t", s);
}
#endif

int win32_screen_right(void) {
    RECT r;

    GetClientRect(hMainWnd, &r);
    return (int) r.right;
}

int win32_screen_bottom(void) {
    RECT r;

    GetClientRect(hMainWnd, &r);
    return (int) r.bottom;
}

void moveto(int x, int y) {
    /* no invalidation of the rectangle, etc needs to be done. */
    turtlePen.h = x;
    turtlePen.v = y;
    MoveCursor(x, y);
}

void lineto(int x, int y) {
    turtlePen.h = x;
    turtlePen.v = y;
    LineTo(memGraphDC, x, y);
    LineTo(GraphDC, x, y);
}

void win32_go_away(void) {
    int i;

    if (read_line != NULL)
	free(read_line);
    for (i = 0; i < NUM_LINES; i++)
	if (win32_lines[i] != NULL)
	    free(win32_lines[i]);
    if (win32_lines != NULL)
	free(win32_lines);
    DeleteObject(turtlePen.hpen);
    DeleteObject(hbrush);
    DeleteObject(hbitC);
    DeleteObject(hbitmC);
    DeleteObject(hbitG);
    DeleteObject(hbitmG);
    DeleteDC(memConDC);
    DeleteDC(memGraphDC);
    DeleteDC(GraphDC);
    DeleteDC(ConDC);
    exit(0);
}

void win32_update_text(void) {
    RECT r;

    GetClientRect(hConWnd, &r);
    BitBlt(ConDC, 0, 0, r.right, r.bottom, memConDC, 0, 0, SRCCOPY);
}

void win32_repaint_screen(void) {
    SetFocus(hMainWnd);
}

void win32_advance_line(void) {
    TEXTMETRIC tm;
    RECT r, inv, foo;
    int xpos, ypos;

    GetClientRect(hConWnd, &r);
    GetTextMetrics(memConDC, &tm);

    xpos = (tm.tmAveCharWidth * x_coord);
    ypos = ((tm.tmHeight + tm.tmExternalLeading) * y_coord);

    if ((ypos + 2 * (tm.tmHeight + tm.tmExternalLeading)) >= r.bottom) {
        ScrollDC(ConDC, 0, - (tm.tmHeight + tm.tmExternalLeading),
		       &r, &r, NULL, &inv);
        FillRect(ConDC, &inv, GetStockObject(WHITE_BRUSH));
        foo.left = 0;
        foo.right = Xsofar;
        foo.top = 0;
        foo.bottom = Ysofar;
        ScrollDC(memConDC, 0, - (tm.tmHeight + tm.tmExternalLeading),
		       &foo, &foo, NULL, &inv);
        FillRect(memConDC, &inv, GetStockObject(WHITE_BRUSH));
    }
}

void draw_string(char *str) {
    TEXTMETRIC tm;
    int x, y;

    GetTextMetrics(memGraphDC, &tm);
    x = turtlePen.h;
    y = turtlePen.v;
    TextOut(memGraphDC, x, y - tm.tmHeight, str, strlen(str));
    TextOut(GraphDC, x, y - tm.tmHeight, str, strlen(str));
    /* restore position */
    moveto(x, y);
}

char *eight_dot_three_helper(char *in) {
    static char out[20];
    char *filename, *extension;

    filename = strtok(in, ".");
    extension = strtok(NULL, ".");

    if (extension)
        sprintf(out, "%.8s.%.3s", filename, extension);
    else
        sprintf(out, "%.8s", filename);
    return out;
}

char *eight_dot_three(char *in) {
    static char out[100];
    char *last, *last_sep, *fear;
    int index;

    if (last_sep = strrchr(in, '\\')) {
        index = last_sep - in;
        index++;
        strncpy(out, in, index);
        out[index] = '\0';
        last = (char *)malloc((strlen(in) - index + 3) * sizeof(char));
        strncpy(last, &in[index], strlen(in) - index);
        last[strlen(in) - index] = '\0';
    } else if (last_sep = strrchr(in, ':')) {
        index = last_sep - in;
        index++;
        strncpy(out, in, index);
        out[index] = '\0';
        last = (char *)malloc((strlen(in) - index + 3) * sizeof(char));
        strncpy(last, &in[index], strlen(in) - index);
        last[strlen(in) - index] = '\0';
    }
    fear = eight_dot_three_helper(last);
    strcat(out, fear);
    free(last);
    return out;
}

void MoveCursor(int newx, int newy) {
    MoveToEx(memGraphDC, newx, newy, NULL);
    MoveToEx(GraphDC, newx, newy, NULL);
}

void win32_erase_screen(void) {
    RECT r;

    /* BLACK brush the whole window */
    GetClientRect(hGraphWnd, &r);
    PatBlt(memGraphDC, 0, 0, r.right, r.bottom, PATCOPY);
    PatBlt(GraphDC, 0, 0, r.right, r.bottom, PATCOPY);
    MoveCursor(screen_x_center, screen_y_center);
}

void win32_set_bg(FIXNUM c) {
    /*
     * set global variables that denote the background color, create a new
     * brush with the desired color, repaint the memory context, wait for
     * WM_PAINT, and redraw the old display (using the records...)
     */
    back_ground = c;
    DeleteObject(hbrush);
    hbrush = CreateSolidBrush(palette[back_ground]);
    SelectObject(memGraphDC, hbrush);
    SelectObject(GraphDC, hbrush);
    PatBlt(memGraphDC, 0, 0, maxX, maxY, PATCOPY);
    PatBlt(GraphDC, 0, 0, maxX, maxY, PATCOPY);
    SetBkColor(memGraphDC, palette[back_ground]);
    SetBkColor(GraphDC, palette[back_ground]);
    redraw_graphics();
}

void win32_clear_text(void) {
    // RECT r;
    /*
     * Draw over the entire client area, so that in split screen mode, for
     * example, there are no ghosts, when the user returns to textscreen mode.
     */

    // GetClientRect(hConWnd, &r);
    FillRect(ConDC, &allRect, GetStockObject(WHITE_BRUSH));
    BitBlt(memConDC, 0, 0, maxX, maxY, ConDC, 0, 0, SRCCOPY);
}

LRESULT CALLBACK GraphWindowFunc(HWND hwnd, UINT message, WPARAM wParam,
				 LPARAM lParam) {
    PAINTSTRUCT ps;

    switch (message) {
        case WM_CREATE:
            /*
             * Now, create a copy of the entire screen in memory to act as a virtual
             * window to record turtle motions so they can be repainted in a
             * WM_PAINT message.
             */
            GraphDC = GetDC(hwnd);
            memGraphDC = CreateCompatibleDC(GraphDC);
            maxX = GetSystemMetrics(SM_CXSCREEN);
            maxY = GetSystemMetrics(SM_CYSCREEN);
            hbitmG = CreateCompatibleBitmap(GraphDC, maxX, maxY);
            SelectObject(memGraphDC, hbitmG);
            hbitG = CreateCompatibleBitmap(GraphDC, maxX, maxY);
            SelectObject(GraphDC, hbitG);
            /*
             * first-time initialization of the brush for the background requires
             * setting the back_ground, and selecting the right color.  while
             * there may be stock objects that contain the right color for the
             * initial pen/brush, subsequent changes might try to delete these
             * "stock" pens/brushes, and that would be "Bad".
             */
            back_ground = 0;
            hbrush = CreateSolidBrush(palette[back_ground]);
            SetBkColor(memGraphDC, palette[back_ground]);
            SetBkColor(GraphDC, palette[back_ground]);
            SelectObject(memGraphDC, hbrush);
            SelectObject(GraphDC, hbrush);

            /*
             * first-time initialization of the pen structure requires setting up
             * all of the fields in the data first, then creating a new pen object
             */
            turtlePen.h = 0; /* ?? */
            turtlePen.v = 0; /* ?? */
            turtlePen.vis = 0;
            turtlePen.mode = WIN_PEN_DOWN;
            turtlePen.width = 1; /* default */
            turtlePen.color = pen_color = old_pen_color = 7;
            /* turtlePen.pattern = ... ? */
            turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width,
				       palette[turtlePen.color]);
            SetTextColor(memGraphDC, palette[turtlePen.color]);
            SetTextColor(GraphDC, palette[turtlePen.color]);
            SelectObject(memGraphDC, turtlePen.hpen);
            SelectObject(GraphDC, turtlePen.hpen);
            PatBlt(memGraphDC, 0, 0, maxX, maxY, PATCOPY);
            PatBlt(GraphDC, 0, 0, maxX, maxY, PATCOPY);
            oldWidth = win32_screen_right();
            oldHeight = win32_screen_bottom();
            break;
        case WM_USER:
            InvalidateRect(hGraphWnd, NULL, 1);
        case WM_PAINT:
            BeginPaint(hGraphWnd, &ps);
            BitBlt(ps.hdc, 0, 0, maxX, maxY, memGraphDC, 0, 0, SRCCOPY);
            EndPaint(hGraphWnd, &ps);
            break;
        case WM_DESTROY:  /* end program */
	    PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

void win32_text_cursor(void) {
    print_char(stdout, '_');
    update_coords('\b');
}

LRESULT CALLBACK ConsoleWindowFunc(HWND hwnd, UINT message, WPARAM wParam,
				   LPARAM lParam) {
    HFONT hfont;
    PAINTSTRUCT ps;
    RECT rect;
    TEXTMETRIC tm;

    switch (message) {
        case WM_CREATE:
            ConDC = GetDC(hwnd);
            memConDC = CreateCompatibleDC(ConDC);
            allRect.left = allRect.top = 0;
            allRect.right = maxX = GetSystemMetrics(SM_CXSCREEN);
            allRect.bottom = maxY = GetSystemMetrics(SM_CYSCREEN);
            hbitmC = CreateCompatibleBitmap(ConDC, maxX, maxY);
            SelectObject(memConDC, hbitmC);
            hbitC = CreateCompatibleBitmap(ConDC, maxX, maxY);
            SelectObject(ConDC, hbitC);
	    hfont = GetStockObject(OEM_FIXED_FONT);
            SelectObject(memConDC, hfont);
            SelectObject(ConDC, hfont);
            hbrush = GetStockObject(WHITE_BRUSH);
            SelectObject(memConDC, hbrush);
            SelectObject(ConDC, hbrush);
            FillRect(memConDC, &allRect, hbrush);
            GetClientRect(hConWnd, &rect);
            GetTextMetrics(ConDC, &tm);
            break;
        case WM_CHAR:
            if (char_mode) {
	        buffered_char = (char)wParam;
	        char_avail++;
	        return 0;
            }
	    print_space(stdout);	    // flush cursor
	    update_coords('\b');
            if ((char) wParam == '\b') {
	        if (cur_index > 0) {
		    update_coords('\b');
		    print_space(stdout);
		    update_coords('\b');
		    win32_text_cursor();
		    cur_index--;
		} else {
		    MessageBeep(0);
		    win32_text_cursor();
		}
            } else if ((char) wParam == '\r') {    // line ready, let's go!
	        print_char(stdout, '\n');

	        win32_lines[cur_line][cur_index++] = '\n'; // reader code expects \n
	        cur_len = cur_index;
	        read_line = (char *)malloc((strlen(win32_lines[cur_line]) + 1) *
				                sizeof(char));
	        strncpy(read_line, win32_lines[cur_line], cur_index + 1);
	        line_avail = 1;
	        read_index = 0;
	        if (++cur_line >= NUM_LINES)
	            cur_line %= NUM_LINES;  // wrap around
	        cur_index = 0;
	        return 0;
            } else if ((char)wParam == 17 || (char)wParam == 23) {
	        print_char(stdout, '\n');
	        read_line = (char *)malloc(sizeof(char));
	        *read_line = (char)wParam;
	        line_avail = 1;
	        read_index = 0;
	        cur_index = 0;
	        return 0;
            } else {
	        win32_lines[cur_line][cur_index++] = (char) wParam;
	        print_char(stdout, (char) wParam);
		win32_text_cursor();
            }
            break;
       case WM_USER:
            InvalidateRect(hConWnd, NULL, 1);
       case WM_PAINT:
            BeginPaint(hConWnd, &ps);
            GetClientRect(ps.hdc, &rect);
            BitBlt(ps.hdc, 0, 0, rect.right, rect.bottom, memConDC, 0, 0, SRCCOPY);
            EndPaint(hConWnd, &ps);
            break;
        case WM_DESTROY:  /* end program */
	    PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs,
		   int nWinMode) {
    NODE *exec_list = NIL;
    WNDCLASSEX wGraphCL, wConCL, wParentCL;
    RECT r;
    TEXTMETRIC tm;
    int i;
    char *argv[20];
    int argc;

    /* Set up the parameters that define the Graphics window's class */
    wParentCL.hInstance = hThisInst;
    wParentCL.lpszClassName = szWinName;
    wParentCL.lpfnWndProc = ParentWindowFunc;
    wParentCL.style = 0;
    wParentCL.cbSize = sizeof(WNDCLASSEX);
    wParentCL.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wParentCL.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wParentCL.hCursor = LoadCursor(NULL, IDC_ARROW);
    wParentCL.lpszMenuName = NULL;
    wParentCL.cbClsExtra = 0;
    wParentCL.cbWndExtra = 0;
    wParentCL.hbrBackground = (HBRUSH) GetStockObject(LTGRAY_BRUSH);

    /* Set up the parameters that define the "Console" window's class */
    wConCL.hInstance = hThisInst;
    wConCL.lpszClassName = szConWinName;
    wConCL.lpfnWndProc = ConsoleWindowFunc;
    wConCL.style = 0;
    wConCL.cbSize = sizeof(WNDCLASSEX);
    wConCL.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wConCL.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wConCL.hCursor = LoadCursor(NULL, IDC_ARROW);
    wConCL.lpszMenuName = NULL;
    wConCL.cbClsExtra = 0;
    wConCL.cbWndExtra = 0;
    wConCL.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);

    /* Set up the parameters that define the Graphics window's class */
    wGraphCL.hInstance = hThisInst;
    wGraphCL.lpszClassName = szGraphWinName;
    wGraphCL.lpfnWndProc = GraphWindowFunc;
    wGraphCL.style = 0;
    wGraphCL.cbSize = sizeof(WNDCLASSEX);
    wGraphCL.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wGraphCL.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wGraphCL.hCursor = LoadCursor(NULL, IDC_ARROW);
    wGraphCL.lpszMenuName = NULL;
    wGraphCL.cbClsExtra = 0;
    wGraphCL.cbWndExtra = 0;
    wGraphCL.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);

    /* Initially, the program is in its "Text only" mode */
    in_graphics_mode = in_splitscreen = 0;

    /* Register the Parent (main) window */
    if (!RegisterClassEx(&wParentCL))
        return 0;
    /* Register the Graphics (child) window */
    if (!RegisterClassEx(&wGraphCL))
        return 0;
    /* Register the Console (child) window */
    if (!RegisterClassEx(&wConCL))
        return 0;

    win32_init_palette();

    hMainWnd = CreateWindow(szWinName, "UCB Logo", WS_OVERLAPPEDWINDOW,
			    CW_USEDEFAULT, CW_USEDEFAULT,
			    CW_USEDEFAULT, CW_USEDEFAULT,
			    HWND_DESKTOP,
			    NULL, hThisInst, NULL);

    win32_lines = (char **) malloc(NUM_LINES * sizeof(char *));
    if (win32_lines == NULL) {
        // print some message and puke
    }
    for (i = 0; i < NUM_LINES; i++)
        win32_lines[i] = (char *) malloc(CHARS_PER_LINE * sizeof(char));
    so_arr[0] = '\1'; so_arr[1] = '\0';
    se_arr[0] = '\2'; se_arr[1] = '\0';

    ShowWindow(hMainWnd, nWinMode);
    UpdateWindow(hMainWnd);

    /*
     * at this point, all of the windows have been created, so it's safe to
     * check on their size
     */

    GetTextMetrics(memConDC, &tm);
    GetClientRect(hConWnd, &r);

    x_coord = y_coord = 0;

    x_max = r.right;
    x_max /= tm.tmAveCharWidth;
    x_max--;  // this eliminates splitting a character over the right border

    y_max = r.bottom;
    y_max /= (tm.tmHeight + tm.tmExternalLeading);
    y_max--;
    Xsofar = r.right;
    Ysofar = r.bottom;

    argc = 1;
    argv[0] = strtok(lpszArgs, " \t\r\n");
    while (argv[argc] = strtok(NULL, " \t\r\n"))
        argc++;

    (void) main(argc, argv);
    win32_go_away();
    return 0;
}

NODE *win32_get_node_pen_pattern(void) {
    return cons(make_intnode(-1), NIL);
}

NODE *win32_get_node_pen_mode(void) {
    if (in_erase_mode)
        return make_static_strnode("erase");
    if (GetROP2(memGraphDC) == R2_XORPEN)
        return make_static_strnode("reverse");
    return make_static_strnode("paint");
}

void logofill(void) {
    COLORREF col;

    DeleteObject(hbrush);
    hbrush = CreateSolidBrush(palette[pen_color]);
    SelectObject(memGraphDC, hbrush);
    SelectObject(GraphDC, hbrush);
    col = GetPixel(memGraphDC, g_round(screen_x_coord), g_round(screen_y_coord));
    (void)ExtFloodFill(memGraphDC, g_round(screen_x_coord),
		         g_round(screen_y_coord), col, FLOODFILLSURFACE);
    (void)ExtFloodFill(GraphDC, g_round(screen_x_coord),
		         g_round(screen_y_coord), col, FLOODFILLSURFACE);
    DeleteObject(hbrush);
    hbrush = CreateSolidBrush(palette[back_ground]);
    SelectObject(memGraphDC, hbrush);
    SelectObject(GraphDC, hbrush);
}

void get_pen_pattern(void) {
}

void set_pen_pattern(void) {
}

void get_palette(int slot, unsigned int *r, unsigned int *g, unsigned int *b) {
    *r = (palette[slot % 200] & 0x00ff0000) >> 16;
    *g = (palette[slot % 200] & 0x0000ff00) >> 8;
    *b = (palette[slot % 200] & 0x000000ff);
}

void set_palette(int slot, unsigned int r, unsigned int g, unsigned int b) {
    if (slot > 199)  // 200 rgb values
        return;
    palette[slot] = RGB(r, g, b);
}

void save_pen(pen_info *p) {
    POINT pt;

    p->vis = pen_vis;
    p->color = pen_color;
    GetCurrentPositionEx(memGraphDC, &pt);
    p->h = (int) pt.x;
    p->v = (int) pt.y;
    p->width = turtlePen.width;
    p->mode = turtlePen.mode;
}

void restore_pen(pen_info *p) {
    pen_vis = p->vis;
    MoveCursor(p->h, p->v);
    turtlePen.mode = p->mode;
    win32_set_pen_mode(turtlePen.mode);
    turtlePen.width = p->width;
    if (p->color != pen_color) {
        pen_color = p->color;
        DeleteObject(turtlePen.hpen);
        turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width, palette[pen_color]);
        SelectObject(memGraphDC, turtlePen.hpen);
        SelectObject(GraphDC, turtlePen.hpen);
    }
}

void set_list_pen_pattern(void) {
}

void label(char *str) {
    draw_string(str);
}

void plain_xor_pen(void) {
    SetROP2(memGraphDC, R2_XORPEN);
    SetROP2(GraphDC, R2_XORPEN);
}

BOOLEAN check_ibm_stop(void) {
    MSG msg;
    SHORT s;
    static int count;

    if ((s = GetAsyncKeyState(VK_CONTROL)) < 0) {
        if ((s = GetAsyncKeyState(65 + ('q' - 'a')) < 0)) {
            err_logo(STOP_ERROR,NIL);
            return (1);
        }
        if ((s = GetAsyncKeyState(65 + ('w' - 'a')) < 0)) {
            // control w
            to_pending = 0;
            lpause(NIL);
        }
    }
    count++;
    count %= 300;  // empirical value

    if (!count) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
		;
            else {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
            }
        }
    }
    return FALSE;
}

void win32_con_full_screen(void) {
    RECT r;
    PAINTSTRUCT ps;
    int deltaX, deltaY;

    if (in_graphics_mode && !in_splitscreen)
        return;
    in_graphics_mode = 1;
    in_splitscreen = 0;

    GetClientRect(hMainWnd, &r);
    /* position coordinates are relative to the parent's frame. */
    MoveWindow(hGraphWnd, 0, 0, r.right, r.bottom, TRUE);
    ShowWindow(hConWnd, SW_HIDE);
    ShowWindow(hGraphWnd, SW_SHOWNORMAL);
    BeginPaint(hGraphWnd, &ps);
    BitBlt(ps.hdc, 0, 0, maxX, maxY, memGraphDC, 0, 0, SRCCOPY);
    EndPaint(hGraphWnd, &ps);
    deltaX = (win32_screen_right()-oldWidth)/2;
    deltaY = (win32_screen_bottom()-oldHeight)/2;
    oldWidth = win32_screen_right();
    oldHeight = win32_screen_bottom();
    if (deltaX != 0 || deltaY != 0) {
        resize_record(deltaX, deltaY);
        redraw_graphics();
    }
}

void win32_prepare_draw(void) {
    if (in_graphics_mode) return;
    win32_con_split_screen();
}

long upscroll_text(int limit) {
    TEXTMETRIC tm;
    RECT r, inv;
    int y_new, offset, old, rbot;

    GetClientRect(hMainWnd, &r);
    GetTextMetrics(memConDC, &tm);
    y_new = r.bottom;
    y_new /= (tm.tmHeight + tm.tmExternalLeading);
    rbot = y_new * (tm.tmHeight + tm.tmExternalLeading);
    y_new--;
    if (limit > 0 && limit < y_new) {
        y_new = limit;
        rbot = r.bottom = (limit+1) * (tm.tmHeight + tm.tmExternalLeading);
    }
    offset = y_coord - y_new;
    if (offset > 0) {
        old = (y_coord + 1) * (tm.tmHeight + tm.tmExternalLeading);
        BitBlt(ConDC, 0, 0, maxX, maxY, memConDC, 0, old-rbot, SRCCOPY);
        inv.left = 0;
        inv.right = allRect.right;
        inv.top = rbot - 1;
        inv.bottom = allRect.bottom;
        BitBlt(memConDC, 0, 0, maxX, maxY, ConDC, 0, 0, SRCCOPY);
        FillRect(memConDC, &inv, GetStockObject(WHITE_BRUSH));
        y_coord = y_new;
    }
    return r.bottom;
}

void reshow_text(void) {
    TEXTMETRIC tm;
    RECT r, foo;

    ShowWindow(hConWnd, SW_SHOW);
    GetClientRect(hConWnd, &r);
    GetTextMetrics(memConDC, &tm);
    FillRect(ConDC, &r, GetStockObject(WHITE_BRUSH));
    if (r.right > Xsofar) {
        foo.left = Xsofar;
        foo.right = r.right;
        foo.top = 0;
        foo.bottom = allRect.bottom;
        FillRect(memConDC, &foo, GetStockObject(WHITE_BRUSH));
        Xsofar = r.right;
    }
    if (r.bottom > Ysofar) {
        foo.left = 0;
        foo.right = r.right;
        foo.top = Ysofar;
        foo.bottom = r.bottom;
        FillRect(memConDC, &foo, GetStockObject(WHITE_BRUSH));
        Ysofar = r.bottom;
    }
    BitBlt(ConDC, 0, 0, r.right, r.bottom, memConDC, 0, 0, SRCCOPY);
    ValidateRect(hConWnd, &r);

    x_max = r.right;
    x_max /= tm.tmAveCharWidth;
    x_max--;

    y_max = r.bottom;
    y_max /= (tm.tmHeight + tm.tmExternalLeading);
    y_max--;
}

void win32_con_split_screen(void) {
    RECT r;
    PAINTSTRUCT ps;
    long vert;
    int deltaX, deltaY;

    if (in_graphics_mode && in_splitscreen)
        return;
    in_graphics_mode = in_splitscreen = 1;

    vert = upscroll_text(3);
    GetClientRect(hMainWnd, &r);
    MoveWindow(hConWnd, 0, r.bottom - vert - 6, r.right, vert + 6, FALSE);
    MoveWindow(hGraphWnd, 0, 0, r.right, r.bottom - vert - 8, TRUE);
    ShowWindow(hGraphWnd, SW_SHOW);
    BeginPaint(hGraphWnd, &ps);
    BitBlt(ps.hdc, 0, 0, maxX, maxY, memGraphDC, 0, 0, SRCCOPY);
    EndPaint(hGraphWnd, &ps);
    reshow_text();
    deltaX = (win32_screen_right()-oldWidth)/2;
    deltaY = (win32_screen_bottom()-oldHeight)/2;
    oldWidth = win32_screen_right();
    oldHeight = win32_screen_bottom();
    if (deltaX != 0 || deltaY != 0) {
        resize_record(deltaX, deltaY);
        redraw_graphics();
    }
    if (!seen_once) {
        seen_once = TRUE;
        lclearscreen(NIL);
    }
}

void win32_con_text_screen(void) {
    RECT r;

    if (!in_graphics_mode)
        return;
    in_graphics_mode = in_splitscreen = 0;

    (void)upscroll_text(0);
    GetClientRect(hMainWnd, &r);
    MoveWindow(hConWnd, 0, 0, r.right, r.bottom, FALSE);
    ShowWindow(hGraphWnd, SW_HIDE);
    reshow_text();
}

void show_if_not_shown(void) {
    if (!in_graphics_mode) {
        ShowWindow(hGraphWnd, SW_NORMAL);
        in_graphics_mode = in_splitscreen = 1;

        lclearscreen(NIL);
        return ;
    }
}
    
void win32_turtle_prep(void) {
    if (in_erase_mode) {
        /* current pen color != "real" pen color */
        DeleteObject(turtlePen.hpen);
        turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width, palette[pen_color]);
    }
    update_pos = FALSE;
    SelectObject(memGraphDC, turtlePen.hpen);
    SelectObject(GraphDC, turtlePen.hpen);
    pre_turtle_pen_mode = SetROP2(memGraphDC, R2_XORPEN);
    (void)SetROP2(GraphDC, R2_XORPEN);
}

void win32_turtle_end(void) {
    if (in_erase_mode) {
        /*
         * current pen color should now be set to background color to resume
         * "erase mode"
         */
        DeleteObject(turtlePen.hpen);
        turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width, palette[back_ground]);
    }
    update_pos = TRUE;
    SelectObject(memGraphDC, turtlePen.hpen);
    SelectObject(GraphDC, turtlePen.hpen);
    SetROP2(memGraphDC, pre_turtle_pen_mode);
    SetROP2(GraphDC, pre_turtle_pen_mode);
}

void win32_init_palette(void) {
    palette[0] = RGB(0, 0, 0); /* black */
    palette[1] = RGB(0, 0, 255); /* blue */
    palette[2] = RGB(0, 255, 0); /* green */
    palette[3] = RGB(0, 255, 255); /* cyan */
    palette[4] = RGB(255, 0, 0); /* red */
    palette[5] = RGB(255, 0, 255); /* magenta */
    palette[6] = RGB(255, 255, 0); /* yellow */
    palette[7] = RGB(255, 255, 255); /* white */
    palette[8] = RGB(155, 96, 59);    /* brown */
    palette[9] = RGB(197, 136, 18); /* tan */
    palette[10] = RGB(100, 162, 64); /* forest */
    palette[11] = RGB(120, 187, 187); /* aqua */
    palette[12] = RGB(255, 149, 119); /* salmon */
    palette[13] = RGB(144, 113, 208); /* purple */
    palette[14] = RGB(255, 163, 0); /* orange */
    palette[15] = RGB(183, 183, 183); /* gray */
    /* rest are user defined */
}

void win32_set_pen_color(int c) {
    draw_turtle();
    turtlePen.color = pen_color = c;
    DeleteObject(turtlePen.hpen);
    turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width,
			         palette[turtlePen.color]);
    SelectObject(memGraphDC, turtlePen.hpen);
    SelectObject(GraphDC, turtlePen.hpen);
    SetTextColor(memGraphDC, palette[pen_color]);
    SetTextColor(GraphDC, palette[pen_color]);
    draw_turtle();
}

int win32_set_pen_width(int w) {
    int old;

    old = turtlePen.width;
    turtlePen.width = w;
    DeleteObject(turtlePen.hpen);
    turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width,
			         palette[turtlePen.color]);
    SelectObject(memGraphDC, turtlePen.hpen);
    SelectObject(GraphDC, turtlePen.hpen);
    return old;
}

NODE *win32_lsetcursor(NODE *args) {
    NODE *arg;
    TEXTMETRIC tm;
    int xpos, ypos;

    GetTextMetrics(memConDC, &tm);

    arg = pos_int_vector_arg(args);
    if (NOT_THROWING) {
        x_coord = x_margin + getint(car(arg));
        y_coord = y_margin + getint(cadr(arg));
        while ((x_coord >= x_max || y_coord >= y_max) && NOT_THROWING) {
            setcar(args, err_logo(BAD_DATA, arg));
            if (NOT_THROWING) {
		arg = pos_int_vector_arg(args);
		x_coord = x_margin + getint(car(arg));
		y_coord = y_margin + getint(cadr(arg));
	    }
	}
    }

    xpos = tm.tmAveCharWidth * x_coord;
    ypos = (tm.tmHeight + tm.tmExternalLeading) * y_coord;

    if (NOT_THROWING) {
        MoveToEx(memConDC, xpos, ypos, NULL);
        MoveToEx(ConDC, xpos, ypos, NULL);
    }
    return(UNBOUND);
}

void win32_pen_erase(void) {
    win32_set_pen_mode(WIN_PEN_ERASE);
}

void win32_pen_down(void) {
    win32_set_pen_mode(WIN_PEN_DOWN);
}

void win32_pen_reverse(void) {
    win32_set_pen_mode(WIN_PEN_REVERSE);
}

void win32_set_pen_mode(int newmode) {
    int rop2_mode, newpc;

    turtlePen.mode = newmode;

    if (newmode == WIN_PEN_ERASE)
        in_erase_mode = TRUE;
    else
        in_erase_mode = FALSE;

    if (newmode == WIN_PEN_REVERSE)
        rop2_mode = R2_XORPEN;
    else
        rop2_mode = R2_COPYPEN;

    SetROP2(memGraphDC, rop2_mode);
    SetROP2(GraphDC, rop2_mode);

    if (newmode != WIN_PEN_REVERSE) {
        if (newmode == WIN_PEN_ERASE)
            newpc = back_ground;
        else
            newpc = pen_color;
        DeleteObject(turtlePen.hpen);
        turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width, palette[newpc]);
        SelectObject(memGraphDC, turtlePen.hpen);
        SelectObject(GraphDC, turtlePen.hpen);
    }
}

LRESULT CALLBACK ParentWindowFunc(HWND hwnd, UINT message, WPARAM wParam,
				 LPARAM lParam) {
    switch (message) {
        case WM_CHAR:
		SendMessage(hConWnd, WM_CHAR, wParam, lParam);
            break;
        case WM_SIZE:
            if (!in_graphics_mode) {   // Text screen mode
                in_graphics_mode = 1;
                win32_con_text_screen();
            } else if (in_splitscreen) {
                in_splitscreen = 0;
                win32_con_split_screen();
            } else {
                in_graphics_mode = 0;
                win32_con_full_screen();
            }
	        break;
        case WM_MOVE:
        case WM_SETFOCUS:
        case WM_EXITMENULOOP:
            if (!in_graphics_mode || in_splitscreen)
                SendMessage(hConWnd, WM_USER, wParam, lParam);
            if (in_graphics_mode)
                SendMessage(hGraphWnd, WM_USER, wParam, lParam);
            win32_update_text();
            break;
        case WM_PAINT:
            /*
             * The parent window really has nothing to draw.  Therefore, all that
             * the parent window's paint function does is dispatch the appropriate
             * message to the child window.
             */
            if (!in_graphics_mode || in_splitscreen)
		SendMessage(hConWnd, WM_PAINT, wParam, lParam);
            if (in_graphics_mode)
		SendMessage(hGraphWnd, WM_PAINT, wParam, lParam);
            break;
        case WM_CREATE:
            hConWnd = CreateWindow(szConWinName, NULL,
				   WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER,
				   0, 0, 0, 0,
				   hwnd, (HMENU) (2 << 8),
				   (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE),
				   NULL);
            hGraphWnd = CreateWindow(szGraphWinName, NULL,
				     WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER,
				     0, 0, 0, 0,
				     hwnd, (HMENU) (4 << 8),
				     (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE),
				     NULL);

            break;
        case WM_DESTROY:  /* end program */
	    win32_go_away();
            break;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

void CharOut(char c) {
    TEXTMETRIC tm;
    RECT r;
    int xpos, ypos;
    char nog[3];

    sprintf(nog, "%c", c);

    GetTextMetrics(memConDC, &tm);
    xpos = tm.tmAveCharWidth * x_coord;
    ypos = (tm.tmHeight + tm.tmExternalLeading) * y_coord;

    r.left = xpos;
    r.right = (xpos + tm.tmAveCharWidth);

    r.top = ypos;
    r.bottom = (ypos + tm.tmHeight + tm.tmExternalLeading);

    TextOut(memConDC, xpos, ypos, nog, 1);
    TextOut(ConDC, xpos, ypos, nog, 1);
}

int win32_putc(char c, FILE *strm) {
    if (strm == stdout || strm == stderr) {
        if (c == '\n')
            win32_advance_line();
	else if (c == '\t') /* do nothing */ ;
        else {
            if (x_coord == x_max)
	    new_line(strm);
            CharOut(c);
        }
        return 0;
    }
    return putc(c, strm);
}

void win32_charmode_on(void) {
    char_mode = 1;
}

void win32_charmode_off(void) {
    char_mode = 0;
}

void ibm_bold_mode(void) {
    SetTextColor(memConDC, RGB(255, 255, 255));
    SetTextColor(ConDC, RGB(255, 255, 255));
    SetBkColor(memConDC, RGB(0, 0, 0));
    SetBkColor(ConDC, RGB(0, 0, 0));
}

void ibm_plain_mode(void) {
    SetTextColor(memConDC, RGB(0, 0, 0));
    SetTextColor(ConDC, RGB(0, 0, 0));
    SetBkColor(memConDC, RGB(255, 255, 255));
    SetBkColor(ConDC, RGB(255, 255, 255));
}
