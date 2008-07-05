#include <iostream>
#include <wx/wx.h>
#include "LogoFrame.h"
#include <stdio.h>
#include <wx/thread.h>
#include <wx/app.h>
#include "stdio.h"
#include "wxTurtleGraphics.h"
#include "wxGlobals.h"
#ifdef __WXMAC__
#include <CoreServices/CoreServices.h>
#endif
#include <string>
#include "wxTerminal.h"		/* must come after wxTurtleGraphics.h */
std::string pathString;
#include <wx/stdpaths.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

int wx_Debugging = 0;

// start the application
IMPLEMENT_APP(LogoApplication)


// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------


// externs from the logo interpreter 
extern "C" int start (int , char **);
extern "C" void redraw_graphics(); 

// need to redraw turtle graphics
int needToRefresh = 0;
// need to load
int load_flag = 0;
// need to save
int save_flag = 0;


// IO buffer handling
char nameBuffer [NAME_BUFFER_SIZE];
int nameBufferSize = 0;


char output_buffer[MAXOUTBUFF];
int output_index = 0;

char buff[BUFF_LEN];
int buff_push_index = 0;
int buff_pop_index = 0;



// ----------------------------------------------------------------------------
// misc functions
// ----------------------------------------------------------------------------

extern "C" void wxLogoExit(int code) {
}

#define LINEPAUSE 25

// flush the output
extern "C" void flushFile(FILE * stream) {
  static int numLines = 0;
  char * temp;

  if (numLines >= LINEPAUSE) {
    wxMilliSleep(1);
    numLines = 0;
  }
  else 
    numLines++;

  wxTerminal::terminal->Flush();
}

// have the interpreter go to sleep

extern "C" void wxLogoSleep(unsigned int milli) {
  //may not work on mac according to wxWidgets doc
  wxDateTime stop_waiting = wxDateTime::UNow() + wxTimeSpan(0,0,0,milli);
  flushFile(stdout);
  extern void wx_refresh();
  wx_refresh();
  if(milli <= 100) {
    wxMilliSleep(milli);
    return;
  }
  while(wxDateTime::UNow().IsEarlierThan(stop_waiting)) {
    if(check_wx_stop(1)) {  //force yielding
      break;
    }
    wxMilliSleep(1);
  }
}

/* Called by the logo thread to display a character onto the terminal screen */
extern "C" void printToScreen(char c, FILE * stream) 
{ 
  if (stream != stdout) {
    putc(c, stream);
    return;
  }
  if(TurtleFrame::in_graphics_mode && !TurtleFrame::in_splitscreen)
    // we are in fullscreen mode
    wxSplitScreen();
  
  if (output_index >= (MAXOUTBUFF - 1)) {   
    flushFile(stdout);   
  }
    
  if (c == '\n') {
    output_buffer[output_index++] = 10;
    output_buffer[output_index++] = 13;
    flushFile(stdout);
  }
  else {
     output_buffer[output_index++] = c;
  }
}



extern "C" char getFromWX_2(FILE * f) ;

extern "C" char getFromWX() 
{ 
  return getFromWX_2(stdin);
}

extern "C" char getFromWX_2(FILE * f) 
{
  int putReturn = 0;
 if (f != stdin) {
    return getc(f);
  }
	
   //  while (buff_index == 0 && !putReturn) {
   while(buff_empty && !putReturn) {
    if(needToRefresh){
      redraw_graphics();
      wxdprintf("wxMain after calling redraw graphics\n");
      needToRefresh = 0;
      turtleGraphics->Refresh();
      wxdprintf("after wxMain calling refresh()");	  
    }
    if (load_flag) {
      load_flag = 0;
      int i;

      buff_push('\n');
       
      for (i = 0; i < nameBufferSize; i++) {

	buff_push(nameBuffer[nameBufferSize - i - 1]);
      }

      buff_push('l'); 
      buff_push('o'); 
      buff_push('a'); 
      buff_push('d'); 
      buff_push(' '); 
      buff_push('"');
    }
    if (save_flag) {
      save_flag = 0;
      int i;

      buff_push('\n');
      for (i = 0; i < nameBufferSize; i++) {

	buff_push(nameBuffer[nameBufferSize - i - 1]);
      }

      buff_push('s'); 
      buff_push('a'); 
      buff_push('v');
      buff_push('e'); 
      buff_push(' '); 
      buff_push('"');      
    }
    // Do this while the lock is released just in case the longjump occurs
    if (check_wx_stop(1)) {   // force yield (1)
      putReturn = 1;
    }
    flushFile(stdout);
    //if (buff_index == 0 && !putReturn)
    if (buff_empty && !putReturn)
      wxMilliSleep(1); // don't wait too long now...
  }
  char c;
  if (putReturn)
    c = '\n';
  else
    buff_pop(c);
  return c;
}


extern "C" int wxKeyp() {
  int ret = 0;
  if (!buff_empty)
    ret = 1;
  return ret;
}

extern "C" int wxUnget_c(int c, FILE * f) {
  if (f != stdin)
    return ungetc(c, f);
  else {
    buff_push((char)c);
    return c;
  }
}

extern "C" char* wx_fgets(char* s, int n, FILE* stream) {
  if (stream != stdin) {
    return fgets(s, n, stream);
  }
  
  //turn on logo char mode (evan)
  //setCharMode(1);
  extern void charmode_on();
  charmode_on();

  char c;
  char * orig = s;
  n --;
  c = ' ';
  while (c != '\n' && n != 0) {
    c = getFromWX_2(stream);
    s[0] = c;
    s++;
    n--;
  }
  return orig;
}


void doLoad(char * name, int length) {
  int i = 0;
  while (i < length && i < NAME_BUFFER_SIZE) {
    nameBuffer[i] = name[i];
    i++;
  }
  nameBufferSize = (length >= NAME_BUFFER_SIZE? NAME_BUFFER_SIZE : length);

  load_flag = 1;  
}

void doSave(char * name, int length) {
  int i = 0;
  while (i < length && i < NAME_BUFFER_SIZE) {
    nameBuffer[i] = name[i];
    i++;
  }
  nameBufferSize = (length >= NAME_BUFFER_SIZE? NAME_BUFFER_SIZE : length);
  
  save_flag = 1;
  }

extern "C" const char* wxMacGetLibloc(){
#ifdef __WXMAC__
	static std::string libloc;
	libloc=pathString;
	libloc+="logolib";
	return libloc.c_str();
#endif
	return 0;
	
}

extern "C" const char* wxMacGetCslsloc(){
#ifdef __WXMAC__
	static std::string cslsloc;
	cslsloc=pathString;
	cslsloc+="csls";
	return cslsloc.c_str();	
#endif
	return 0;
}

extern "C" const char* wxMacGetHelploc(){
#ifdef __WXMAC__
	static std::string helploc;
	helploc=pathString;
	helploc+="helpfiles";
	return helploc.c_str();
#endif
	return 0;
}
