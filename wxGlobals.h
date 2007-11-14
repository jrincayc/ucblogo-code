/*
 *      wxGlobals.h       wx logo global references module           
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
 *
 */

#define SCREEN_WIDTH		1
#define SCREEN_HEIGHT		2
#define	BACK_GROUND			3
#define	IN_SPLITSCREEN		4
#define	IN_GRAPHICS_MODE	5
#define	X_COORD				6
#define	Y_COORD				7
#define X_MAX				8
#define Y_MAX				9
#define EDIT_STATE   	    10

// used for the text editor flag
#define NO_INFO 0
#define DO_LOAD	1
#define NO_LOAD 2

/* wxMain */
extern int out_buff_index_public;
extern int out_buff_index_private;
extern char * out_buff_public;
extern char * out_buff_private;
extern wxCondition read_buff;
extern char buff[];
extern int buff_index;
extern int alreadyAlerted;
extern wxMutex buff_full_m;
extern wxCondition buff_full_cond;
extern int MAXOUTBUFF;
extern char out_buff[];
extern int out_buff_index;
extern wxMutex out_mut;
extern wxMutex in_mut;

extern int needToRefresh;
extern wxCondition read_buff;

/* wxTerminal */
class TurtleCanvas;
class TextEditor;
extern wxCommandEvent * haveInputEvent;
extern LogoFrame *logoFrame;
extern wxMenuBar* menuBar;
extern wxBoxSizer *topsizer;
extern TextEditor *editWindow;
extern TurtleCanvas * turtleGraphics; 

/* wxTurtleGraphics */
// the size of the turtle graphics window
extern "C" int getInfo(int);
extern "C" void setInfo(int, int);

/* wxTextEdit */
extern wxCondition editCond;
extern wxMutex editMut; 
extern char * file; // using this is safe because the other thread is sleeping
					// and it stack is also asleep
extern "C" int check_wx_stop();
extern "C" int internal_check();

#define wxdprintf if (wx_Debugging) printf


extern int wx_Debugging;	/* in wxMain */
