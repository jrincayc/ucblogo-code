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
#define BUFF_LEN 1024  //should be a power of two.

#define buff_empty (buff_push_index == buff_pop_index)

#define buff_push(c) buff[buff_push_index++] = c; \
                     buff_push_index = buff_push_index % BUFF_LEN; \
                     if(buff_push_index == buff_pop_index) { \
                       fprintf(stderr, "buff push warning: pushindex = popindex\n. discarding last push"); \
                       buff_push_index = (buff_push_index - 1) % BUFF_LEN; \
		     }

#define buff_pop(c)  c = buff[buff_pop_index++]; \
                     buff_pop_index = buff_pop_index % BUFF_LEN
extern char buff[];
extern int buff_push_index;
extern int buff_pop_index;
extern int alreadyAlerted;
extern int MAXOUTBUFF;
extern char out_buff[];
extern int out_buff_index;

/* wxTerminal */
class TurtleCanvas;
class TextEditor;
extern wxCommandEvent * haveInputEvent;
extern LogoFrame *logoFrame;
extern LogoEventManager *logoEventManager;
extern wxMenuBar* menuBar;
extern wxBoxSizer *topsizer;
extern TextEditor *editWindow;
extern TurtleCanvas * turtleGraphics; 

/* wxTurtleGraphics */
// the size of the turtle graphics window
extern "C" int getInfo(int);
extern "C" void setInfo(int, int);

/* wxTextEdit */
extern char * file; // using this is safe because the other thread is sleeping
					// and it stack is also asleep
extern "C" int check_wx_stop(int force_yield);
extern "C" int internal_check();

/* Synchronization for Multithreaded version */
#ifdef MULTITHREAD
/* wxMain, wxTerminal */
extern wxCondition read_buff;
extern wxMutex buff_full_m;
extern wxCondition buff_full_cond;
extern wxMutex out_mut;
extern wxMutex in_mut;
extern wxCondition read_buff;
extern int needToRefresh;
/* wxTextEdit */
extern wxCondition editCond;
extern wxMutex editMut; 
#endif


#define wxdprintf if (wx_Debugging) printf


extern int wx_Debugging;	/* in wxMain */
