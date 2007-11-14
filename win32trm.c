/* -*-C-*-
 * win32trm.c -- Module to provide Win32 API compliant graphics routines
 *
 *	Copyright (C) 1996 by the Regents of the University of California
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *  
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *  
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <windows.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "logo.h"
#include "globals.h"
#include "win32trm.h"
#include "ucbwlogo/resource.h"

// #define WIN32_S

LRESULT CALLBACK ParentWindowFunc(HWND, UINT, WPARAM, LPARAM);

char szWinName[] = "UCBLogo";
char szGraphWinName[] = "UCBLogoGraph";
char szConWinName[] = "UCBLogoConsole";

char *LogoPlatformName="Windows";

pen_info turtlePen;

HBRUSH hbrush, textbrush;

HBITMAP hbitG, hbitmG, hbitC, hbitmC;
RECT allRect;

HDC GraphDC, memGraphDC, ConDC, memConDC;
HWND hGraphWnd, hConWnd, hMainWnd;

/* color and palette related information */
static COLORREF palette[266];
FIXNUM back_ground, pen_color, old_pen_color;

int maxX, maxY, greeted = 0;
int pre_turtle_pen_mode;
long Xsofar, Ysofar;
int w_button = 0;
FIXNUM mouse_x = 0, mouse_y = 0;

BOOLEAN in_erase_mode, update_pos, seen_once = FALSE;

int in_graphics_mode, in_splitscreen;

static char **win32_lines; // for recording what should appear in the text win
static char input_lines[2][200];

char *read_line, buffered_char;
int cur_line = 0, cur_index = 0, read_index, hpos = 0, margin, cur_len;
int char_mode = 0, input_line = 0, input_index = 0;

int line_avail = FALSE, char_avail = FALSE;
char *winPasteText = NULL, *winPastePtr = NULL;
char *myScreen = NULL;
int maxXChar, maxYChar;

/* Default font behavior set here: OEM_FIXED_FONT, ANSI_FIXED_FONT, or default */
BOOLEAN oemfont = FALSE;
BOOLEAN ansifont = TRUE;

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
	/* Good old Windows does us the favor of leaving out
	   the endpoint, so we have to beat it up */
	LineTo(memGraphDC, x, y+1);
	LineTo(GraphDC, x, y+1);
	MoveCursor(x, y);
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
    DeleteDC(memConDC);
    DeleteDC(memGraphDC);
    DeleteDC(GraphDC);
    DeleteDC(ConDC);
    DeleteObject(turtlePen.hpen);
    DeleteObject(hbrush);
    DeleteObject(textbrush);
    DeleteObject(hbitC);
    DeleteObject(hbitmC);
    DeleteObject(hbitG);
    DeleteObject(hbitmG);
    exit(0);
}

void win32_update_text(void) {
    RECT r;

    GetClientRect(hConWnd, &r);
    BitBlt(ConDC, 0, 0, r.right, r.bottom, memConDC, 0, 0, SRCCOPY);
}

void win32_repaint_screen(void) {
    PostMessage(hMainWnd, WM_SETFOCUS, 0, 0);
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
	memmove(myScreen, myScreen+maxXChar, maxXChar*(maxYChar-1));
        ScrollDC(ConDC, 0, - (tm.tmHeight + tm.tmExternalLeading),
		       &r, &r, NULL, &inv);
        FillRect(ConDC, &inv, textbrush);
        foo.left = 0;
        foo.right = Xsofar;
        foo.top = 0;
        foo.bottom = Ysofar;
        ScrollDC(memConDC, 0, - (tm.tmHeight + tm.tmExternalLeading),
		       &foo, &foo, NULL, &inv);
        FillRect(memConDC, &inv, textbrush);
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
    for (fear = last; *fear != '\0'; fear++)
        if (*fear == '?') *fear = 'Q';
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
    PatBlt(memGraphDC, 0, 0, maxX, maxY, PATCOPY);
    PatBlt(GraphDC, 0, 0, maxX, maxY, PATCOPY);
}

void win32_set_bg(FIXNUM c) {
    /*
     * set global variables that denote the background color, create a new
     * brush with the desired color, repaint the memory context, wait for
     * WM_PAINT, and redraw the old display (using the records...)
     */
    back_ground = c;
    hbrush = CreateSolidBrush(palette[back_ground+2]);
    SelectObject(memGraphDC, hbrush);
    DeleteObject(SelectObject(GraphDC, hbrush));
    PatBlt(memGraphDC, 0, 0, maxX, maxY, PATCOPY);
    PatBlt(GraphDC, 0, 0, maxX, maxY, PATCOPY);
    SetBkColor(memGraphDC, palette[back_ground+2]);
    SetBkColor(GraphDC, palette[back_ground+2]);
    redraw_graphics();
}

void win32_clear_text(void) {
    // RECT r;
    /*
     * Draw over the entire client area, so that in split screen mode, for
     * example, there are no ghosts, when the user returns to textscreen mode.
     */

    // GetClientRect(hConWnd, &r);
    memset(myScreen, ' ',maxXChar*maxYChar);
    FillRect(ConDC, &allRect, textbrush);
    FillRect(memConDC, &allRect, textbrush);
}

LRESULT CALLBACK GraphWindowFunc(HWND hwnd, UINT message, WPARAM wParam,
				 LPARAM lParam) {
    HFONT hfont;
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
	    if (oemfont) {
		hfont = GetStockObject(OEM_FIXED_FONT);
		SelectObject(memGraphDC, hfont);
		SelectObject(GraphDC, hfont);
	    }
	    if (ansifont) {
		hfont = GetStockObject(ANSI_FIXED_FONT);
		SelectObject(memGraphDC, hfont);
		SelectObject(GraphDC, hfont);
	    }
            /*
             * first-time initialization of the brush for the background requires
             * setting the back_ground, and selecting the right color.  while
             * there may be stock objects that contain the right color for the
             * initial pen/brush, subsequent changes might try to delete these
             * "stock" pens/brushes, and that would be "Bad".
             */
            back_ground = 0;
            hbrush = CreateSolidBrush(palette[back_ground+2]);
            SetBkColor(memGraphDC, palette[back_ground+2]);
            SetBkColor(GraphDC, palette[back_ground+2]);
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
				       palette[2+turtlePen.color]);
            SetTextColor(memGraphDC, palette[2+turtlePen.color]);
            SetTextColor(GraphDC, palette[2+turtlePen.color]);
            SelectObject(memGraphDC, turtlePen.hpen);
            SelectObject(GraphDC, turtlePen.hpen);
            PatBlt(memGraphDC, 0, 0, maxX, maxY, PATCOPY);
            PatBlt(GraphDC, 0, 0, maxX, maxY, PATCOPY);
            break;
	case WM_LBUTTONDOWN:
	    w_button = 1;
	    goto abutton;
	case WM_RBUTTONDOWN:
	    w_button = 2;
	    goto abutton;
	case WM_MBUTTONDOWN:
	    w_button = 3;
abutton:
	    SetCapture(hwnd);
	case WM_MOUSEMOVE:
	    mouse_x = (LOWORD(lParam))-(screen_width/2);
	    mouse_y = (screen_height/2)-(HIWORD(lParam));
	    break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	    w_button = 0;
	    ReleaseCapture();
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

void DrawBoxOutline(POINT ptBeg, POINT ptEnd) {
    SetROP2(ConDC, R2_NOT);
    SelectObject(ConDC, GetStockObject(NULL_BRUSH));
    Rectangle(ConDC, ptBeg.x, ptBeg.y, ptEnd.x, ptEnd.y);
}

LRESULT CALLBACK ConsoleWindowFunc(HWND hwnd, UINT message, WPARAM wParam,
				   LPARAM lParam) {
    HFONT hfont;
    PAINTSTRUCT ps;
    RECT rect;
    TEXTMETRIC tm;
    HGLOBAL hClipMemory, hCopyText;
    PSTR pClipMemory, copyText;
    static int fBlocking=FALSE, haveBlock=FALSE;
    static POINT ptBeg, ptEnd;
    static int chBegX, chBegY, chEndX, chEndY;
    char *copyPtr;
    int i,j;

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
	    if (oemfont) {
		hfont = GetStockObject(OEM_FIXED_FONT);
		SelectObject(memConDC, hfont);
		SelectObject(ConDC, hfont);
	    }
	    if (ansifont) {
		hfont = GetStockObject(ANSI_FIXED_FONT);
		SelectObject(memConDC, hfont);
		SelectObject(ConDC, hfont);
	    }
            textbrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
            SelectObject(memConDC, textbrush);
            SelectObject(ConDC, textbrush);
            FillRect(memConDC, &allRect, textbrush);
            GetClientRect(hConWnd, &rect);
            GetTextMetrics(ConDC, &tm);
	    maxXChar = maxX / tm.tmAveCharWidth;
	    maxYChar = maxY / (tm.tmHeight + tm.tmExternalLeading);
	    myScreen = (char *)malloc(maxXChar * maxYChar);
	    memset(myScreen, ' ',maxXChar*maxYChar);
            ibm_plain_mode();
            break;
	case WM_LBUTTONDOWN:
	    if (haveBlock) DrawBoxOutline(ptBeg, ptEnd);
	    GetTextMetrics(memConDC, &tm);
	    ptEnd.x = LOWORD(lParam);
	    ptEnd.y = HIWORD(lParam);
	    ptEnd.x = (ptEnd.x/tm.tmAveCharWidth) * tm.tmAveCharWidth;
	    ptEnd.y = (ptEnd.y/(tm.tmHeight + tm.tmExternalLeading))
			* (tm.tmHeight + tm.tmExternalLeading);
	    ptBeg.x = ptEnd.x;
	    ptBeg.y = ptEnd.y;
	    DrawBoxOutline(ptBeg, ptEnd);
	    SetCapture(hwnd);
	    fBlocking = TRUE;
	    haveBlock = FALSE;
	    return 0;
	case WM_MOUSEMOVE:
	    GetTextMetrics(memConDC, &tm);
	    if (fBlocking) {
		DrawBoxOutline(ptBeg, ptEnd);
		ptEnd.x = LOWORD(lParam);
		ptEnd.y = HIWORD(lParam);
		ptEnd.x = (ptEnd.x/tm.tmAveCharWidth) * tm.tmAveCharWidth;
		ptEnd.y = (ptEnd.y/(tm.tmHeight + tm.tmExternalLeading))
			    * (tm.tmHeight + tm.tmExternalLeading);
		DrawBoxOutline(ptBeg, ptEnd);
	    }
	    return 0;
	case WM_LBUTTONUP:
	    if (fBlocking) {
		ReleaseCapture();
		fBlocking = FALSE;
		haveBlock=TRUE;
		GetClientRect(hConWnd, &rect);
		if (ptEnd.x >= 0 && ptEnd.y >= 0 && ptEnd.x < rect.right &&
			ptEnd.y < rect.bottom) {
		    GetTextMetrics(memConDC, &tm);
		    chBegX = ptBeg.x/tm.tmAveCharWidth;
		    chBegY = ptBeg.y/(tm.tmHeight + tm.tmExternalLeading);
		    chEndX = ptEnd.x/tm.tmAveCharWidth;
		    chEndY = ptEnd.y/(tm.tmHeight + tm.tmExternalLeading);
		} else {
		    DrawBoxOutline(ptBeg, ptEnd);
		    haveBlock = FALSE;
		}
	    }
	    return 0;
        case WM_CHAR:
	    if (haveBlock) DrawBoxOutline(ptBeg, ptEnd);
            if (char_mode) {
	        buffered_char = (char)wParam;
	        char_avail++;
		haveBlock = FALSE;
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
		    haveBlock = FALSE;
		    win32_lines[cur_line][cur_index++] = (char) wParam;
		    print_char(stdout, (char) wParam);
		    win32_text_cursor();
		}
            } else if ((char) wParam == '\r') {    // line ready, let's go!
	        print_char(stdout, '\n');
		haveBlock = FALSE;
	        win32_lines[cur_line][cur_index++] = '\n'; // reader code expects \n
		win32_lines[cur_line][cur_index] = '\0';
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
		haveBlock = FALSE;
	        print_char(stdout, '\n');
	        read_line = (char *)malloc(sizeof(char));
	        *read_line = (char)wParam;
	        line_avail = 1;
	        read_index = 0;
	        cur_index = 0;
	        return 0;
	    } else if ((char)wParam == 3 && haveBlock) { /* ^C for copy */
		int temp;
		haveBlock = FALSE;
		if (chEndX < chBegX) {
		    temp = chEndX;
		    chEndX = chBegX;
		    chBegX = temp;
		}
		if (chEndY < chBegY) {
		    temp = chEndY;
		    chEndY = chBegY;
		    chBegY = temp;
		}
		hCopyText = GlobalAlloc(GHND,
					(chEndY-chBegY)*(2+chEndX-chBegX)+1);
		copyText = GlobalLock(hCopyText);
		copyPtr = copyText;
		for (i=chBegY; i<chEndY; i++) {
		    for (j=chBegX; j<chEndX; j++)
			*copyPtr++ = myScreen[i*maxXChar + j];
		    *copyPtr++ = '\r';
		    *copyPtr++ = '\n';
		}
		*(copyPtr-2) = '\0';	/* no newline at end */
		GlobalUnlock(hCopyText);
		if (OpenClipboard(hwnd)) {
		    EmptyClipboard();
		    SetClipboardData(CF_TEXT,hCopyText);
		    CloseClipboard();
		}
		return 0;
	    } else if ((char)wParam == 22) {	/* ^V for paste */
		haveBlock = FALSE;
		if (OpenClipboard(hwnd)) {
		    hClipMemory = GetClipboardData(CF_TEXT);
		    if (hClipMemory != NULL) {
			winPasteText = (char *)malloc(GlobalSize(hClipMemory));
			pClipMemory = GlobalLock(hClipMemory);
			strcpy(winPasteText, pClipMemory);
			winPastePtr = winPasteText;
		    }
		    GlobalUnlock(hClipMemory);
		    CloseClipboard();
		    winDoPaste();
		}
            } else {
		haveBlock = FALSE;
	        win32_lines[cur_line][cur_index++] = (char) wParam;
	        print_char(stdout, (char) wParam);
		win32_text_cursor();
            }
            break;
       case WM_USER:
            InvalidateRect(hConWnd, NULL, 1);
       case WM_PAINT:
	    haveBlock = FALSE;
            BeginPaint(hConWnd, &ps);
            GetClientRect(hConWnd /* ps.hdc */ , &rect);
            BitBlt(ps.hdc, 0, 0, rect.right, rect.bottom, memConDC, 0, 0,
		   SRCCOPY);
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
    char *argv[20], *cp;
    int argc;
    char *fontenv;

    fontenv = getenv("LOGOFONT");
    if (fontenv != NULL && !strcmp(fontenv,"oem")) {
	oemfont = TRUE;
	ansifont = FALSE;
    }
    if (fontenv != NULL && !strcmp(fontenv,"ansi")) {
	ansifont = TRUE;
	oemfont = FALSE;
    }
    if (fontenv != NULL && !strcmp(fontenv,"default")) {
	ansifont = FALSE;
	oemfont = FALSE;
    }

    /* Set up the parameters that define the Graphics window's class */
    wParentCL.hInstance = hThisInst;
    wParentCL.lpszClassName = szWinName;
    wParentCL.lpfnWndProc = ParentWindowFunc;
    wParentCL.style = 0;
    wParentCL.cbSize = sizeof(WNDCLASSEX);
    wParentCL.hIcon = LoadIcon(hThisInst, (char *)IDI_ICON1);
    wParentCL.hIconSm = LoadIcon(hThisInst, (char *)IDI_ICON2);
    wParentCL.hCursor = LoadCursor(NULL, IDC_ARROW);
    wParentCL.lpszMenuName = NULL;
    wParentCL.cbClsExtra = 0;
    wParentCL.cbWndExtra = 0;
    wParentCL.hbrBackground = (void *)(COLOR_WINDOWFRAME+1);

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
    wConCL.hbrBackground = (void *)(COLOR_WINDOW+1);

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
			    700, 440,
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

    argv[0] = "ucblogo";
    argc = 1;
    if (lpszArgs != NULL) {
	cp = lpszArgs;
	while (*cp != '\0' && *cp != '\n' && *cp != '\r') {
	    if (*cp == '"') {
		argv[argc++] = ++cp;
		while (*cp != '"' && *cp != '\0') cp++;
		if (*cp == '"') *cp++ = '\0';
	    } else {
		argv[argc++] = cp;
		while (*cp != '\0' && *cp != ' ' && *cp != '\t'
		       && *cp != '\n' && *cp != '\r') cp++;
		if (*cp != '\0') *cp++ = '\0';
	    }
	    while (*cp == ' ' || *cp == '\t') cp++;
	}
    }

/*
    if (lpszArgs != NULL && (argv[1] = strtok(lpszArgs, " \t\r\n")) != NULL) {
	argc = 2;
	while (argv[argc] = strtok(NULL, " \t\r\n"))
	    argc++;
    }
 */

    (void) main(argc, argv);
    win32_go_away();
    return 0;
}

void winDoPaste(void) {
    char ch = *winPastePtr++;
    while (ch != '\0') {
	if (ch == '\r') {
	    print_char(stdout, '\n');
	    win32_lines[cur_line][cur_index++] = '\n';
	    cur_len = cur_index;
	    read_line = (char *)malloc((strlen(win32_lines[cur_line]) + 1) *
					    sizeof(char));
	    strncpy(read_line, win32_lines[cur_line], cur_index + 1);
	    line_avail = 1;
	    read_index = 0;
	    if (++cur_line >= NUM_LINES)
		cur_line %= NUM_LINES;  // wrap around
	    cur_index = 0;
	    return;
	}
	if (ch != 3 && ch != 22 && ch != '\n') {
	    win32_lines[cur_line][cur_index++] = ch;
	    print_char(stdout, ch);
	}
	ch = *winPastePtr++;
    }
    free(winPasteText);
    winPasteText = NULL;
}

NODE *win32_get_node_pen_pattern(void) {
    return cons(make_intnode(-1), NIL);
}

NODE *maximize(NODE *args) {
	int big=torf_arg(args);
	ShowWindow(hMainWnd, (big ? SW_MAXIMIZE : SW_RESTORE));
    UpdateWindow(hMainWnd);
	return UNBOUND;
}

void logofill(void) {
    COLORREF col;

    hbrush = CreateSolidBrush(palette[2+pen_color]);
    SelectObject(memGraphDC, hbrush);
    DeleteObject(SelectObject(GraphDC, hbrush));
    col = GetPixel(memGraphDC, g_round(screen_x_coord), g_round(screen_y_coord));
    (void)ExtFloodFill(memGraphDC, g_round(screen_x_coord),
		         g_round(screen_y_coord), col, FLOODFILLSURFACE);
    (void)ExtFloodFill(GraphDC, g_round(screen_x_coord),
		         g_round(screen_y_coord), col, FLOODFILLSURFACE);
    hbrush = CreateSolidBrush(palette[2+back_ground]);
    SelectObject(memGraphDC, hbrush);
    DeleteObject(SelectObject(GraphDC, hbrush));
}

void get_pen_pattern(void) {
}

void set_pen_pattern(void) {
}

void get_palette(int slot, unsigned int *r, unsigned int *g, unsigned int *b) {
    if (slot > 263 || (slot >= 0 && slot < 8) || slot < -2)  // 256 rgb values
        return;
    slot+=2;
    *b = ((palette[slot % 264] & 0x00ff0000) >> 16) * 256;
    *g = ((palette[slot % 264] & 0x0000ff00) >> 8) * 256;
    *r = (palette[slot % 264] & 0x000000ff) * 256;
}

void set_palette(int slot, unsigned int r, unsigned int g, unsigned int b) {
    if (slot > 263 || (slot >= 0 && slot < 8) || slot < -2)  // 256 rgb values
        return;
    slot+=2;
    palette[slot] = RGB(r/256, g/256, b/256);
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
    pen_color = p->color;
    turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width,
					 (in_erase_mode ? palette[2+back_ground]
							: palette[2+pen_color]));
    SelectObject(memGraphDC, turtlePen.hpen);
    DeleteObject(SelectObject(GraphDC, turtlePen.hpen));
}

void set_list_pen_pattern(void) {
}

void label(char *str) {
    draw_string(str);
}

void plain_xor_pen(void) {
    win32_set_pen_mode(WIN_PEN_REVERSE);
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
    redraw_graphics();
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
        FillRect(memConDC, &inv, textbrush);
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
    FillRect(ConDC, &r, textbrush);
    if (r.right > Xsofar) {
        foo.left = Xsofar;
        foo.right = r.right;
        foo.top = 0;
        foo.bottom = allRect.bottom;
        FillRect(memConDC, &foo, textbrush);
        Xsofar = r.right;
    }
    if (r.bottom > Ysofar) {
        foo.left = 0;
        foo.right = r.right;
        foo.top = Ysofar;
        foo.bottom = r.bottom;
        FillRect(memConDC, &foo, textbrush);
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

    if (in_graphics_mode && in_splitscreen)
        return;
    in_graphics_mode = in_splitscreen = 1;

    ShowWindow(hConWnd, SW_SHOW);
    vert = upscroll_text(3);
    GetClientRect(hMainWnd, &r);
    MoveWindow(hConWnd, 0, r.bottom - vert - 6, r.right, vert + 6, FALSE);
    MoveWindow(hGraphWnd, 0, 0, r.right, r.bottom - vert - 8, TRUE);
    ShowWindow(hGraphWnd, SW_HIDE);
    BeginPaint(hGraphWnd, &ps);
    BitBlt(ps.hdc, 0, 0, maxX, maxY, memGraphDC, 0, 0, SRCCOPY);
    EndPaint(hGraphWnd, &ps);
    reshow_text();
    redraw_graphics();
    ShowWindow(hGraphWnd, SW_SHOW);
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
    
void win32_turtle_prep(void) {
    if (in_erase_mode) {
        /* current pen color != "real" pen color */
        turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width, palette[2+pen_color]);
	SelectObject(memGraphDC, turtlePen.hpen);
	DeleteObject(SelectObject(GraphDC, turtlePen.hpen));
    } else {
	SelectObject(memGraphDC, turtlePen.hpen);
	SelectObject(GraphDC, turtlePen.hpen);
    }
    update_pos = FALSE;
    pre_turtle_pen_mode = SetROP2(memGraphDC, R2_XORPEN);
    (void)SetROP2(GraphDC, R2_XORPEN);
}

void win32_turtle_end(void) {
    if (in_erase_mode) {
        /*
         * current pen color should now be set to background color to resume
         * "erase mode"
         */
        turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width, palette[2+back_ground]);
	SelectObject(memGraphDC, turtlePen.hpen);
	DeleteObject(SelectObject(GraphDC, turtlePen.hpen));
    } else {
	SelectObject(memGraphDC, turtlePen.hpen);
	SelectObject(GraphDC, turtlePen.hpen);
    }
    update_pos = TRUE;
    SetROP2(memGraphDC, pre_turtle_pen_mode);
    SetROP2(GraphDC, pre_turtle_pen_mode);
}

void win32_init_palette(void) {
    palette[2] = RGB(0, 0, 0); /* black */
    palette[3] = RGB(0, 0, 255); /* blue */
    palette[4] = RGB(0, 255, 0); /* green */
    palette[5] = RGB(0, 255, 255); /* cyan */
    palette[6] = RGB(255, 0, 0); /* red */
    palette[7] = RGB(255, 0, 255); /* magenta */
    palette[8] = RGB(255, 255, 0); /* yellow */
    palette[9] = RGB(255, 255, 255); /* white */
    palette[10] = RGB(155, 96, 59);    /* brown */
    palette[11] = RGB(197, 136, 18); /* tan */
    palette[12] = RGB(100, 162, 64); /* forest */
    palette[13] = RGB(120, 187, 187); /* aqua */
    palette[14] = RGB(255, 149, 119); /* salmon */
    palette[15] = RGB(144, 113, 208); /* purple */
    palette[16] = RGB(255, 163, 0); /* orange */
    palette[17] = RGB(183, 183, 183); /* gray */
    /* rest are user defined */
}

void win32_set_pen_color(int c) {
    draw_turtle();
    turtlePen.color = pen_color = c;
    if (!in_erase_mode) {
	turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width,
				   palette[2+turtlePen.color]);
	SelectObject(memGraphDC, turtlePen.hpen);
	DeleteObject(SelectObject(GraphDC, turtlePen.hpen));
    }
    SetTextColor(memGraphDC, palette[2+pen_color]);
    SetTextColor(GraphDC, palette[2+pen_color]);
    draw_turtle();
}

int win32_set_pen_width(int w) {
    int old;

    old = turtlePen.width;
    turtlePen.width = w;
    turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width,
			         (in_erase_mode ? palette[2+back_ground]
						: palette[2+turtlePen.color]));
    SelectObject(memGraphDC, turtlePen.hpen);
    DeleteObject(SelectObject(GraphDC, turtlePen.hpen));
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

    if (newmode == WIN_PEN_ERASE)
	newpc = back_ground;
    else
	newpc = pen_color;
    turtlePen.hpen = CreatePen(PS_SOLID, turtlePen.width, palette[2+newpc]);
    SelectObject(memGraphDC, turtlePen.hpen);
    DeleteObject(SelectObject(GraphDC, turtlePen.hpen));
}

LRESULT CALLBACK ParentWindowFunc(HWND hwnd, UINT message, WPARAM wParam,
				 LPARAM lParam) {
    RECT r;

    switch (message) {
        case WM_CHAR:
		SendMessage(hConWnd, WM_CHAR, wParam, lParam);
            break;
        case WM_SIZE:
	case WM_EXITSIZEMOVE:
	    if (wParam != SIZE_MINIMIZED) {
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
	    }
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
	    GetClientRect(hwnd, &r);
	    ValidateRect(hwnd, &r);
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

void CharOut(int c) {
    TEXTMETRIC tm;
    RECT r;
    int xpos, ypos;
    char nog[3];

    sprintf(nog, "%c", c);

    if (x_coord < maxXChar && y_coord < maxYChar)
	myScreen[(y_coord * maxXChar) + x_coord] = c;

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

int win32_putc(int c, FILE *strm) {
    if (strm == stdout || strm == stderr) {
        if (c == '\n')
            win32_advance_line();
	else if (c == '\t') /* do nothing */ ;
	else if (c == '\007') tone(400,30);
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
    SetTextColor(memConDC, GetSysColor(COLOR_WINDOW));
    SetTextColor(ConDC, GetSysColor(COLOR_WINDOW));
    SetBkColor(memConDC, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor(ConDC, GetSysColor(COLOR_WINDOWTEXT));
}

void ibm_plain_mode(void) {
    SetTextColor(memConDC, GetSysColor(COLOR_WINDOWTEXT));
    SetTextColor(ConDC, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor(memConDC, GetSysColor(COLOR_WINDOW));
    SetBkColor(ConDC, GetSysColor(COLOR_WINDOW));
}

NODE *set_text_color(NODE *args) {
    int fore, back;

    fore = getint(pos_int_arg(args));
    if (NOT_THROWING) {
        back = getint(pos_int_arg(cdr(args)));
        if (NOT_THROWING) {
           SetTextColor(memConDC, palette[2+fore]);
           SetTextColor(ConDC, palette[2+fore]);
           SetBkColor(memConDC, palette[2+back]);
           SetBkColor(ConDC, palette[2+back]);
        }
    }
    return UNBOUND;
}

/* Thanks to George Mills for the tone code! */
/*   was:    MessageBeep(0xffffffff);	*/

void tone(FIXNUM pitch, FIXNUM duration) {
    OSVERSIONINFO VersionInformation;
    unsigned char count_lo;
    unsigned char count_hi;
    FIXNUM count;
    clock_t NumTicksToWait;

    memset(&VersionInformation, 0, sizeof(OSVERSIONINFO));
    VersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (pitch < 37) pitch = 37;
    GetVersionEx(&VersionInformation);
    if (VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT)
	Beep(pitch, duration);
    else {
	count = 1193180L / pitch;
	count_lo = LOBYTE(count);
	count_hi = HIBYTE(count);

	_asm {
	    mov al, 0xB6
	    out 0x43, al
	    mov al, count_lo
	    out 0x42, al
	    mov al, count_hi
	    out 0x42, al
	    xor al, al
	    in al, 0x61
	    or al, 0x03
	    out 0x61, al
	}

	NumTicksToWait = duration + clock();
	while (NumTicksToWait > clock());

	_asm {
	    xor al, al
	    in al, 0x61
	    xor al, 0x03
	    out 0x61, al
	}
    }
}
