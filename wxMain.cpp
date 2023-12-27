#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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

extern "C" NODE *cons(NODE *, NODE *);
extern "C" NODE *make_static_strnode(char *);
extern "C" NODE *lload(NODE *);
extern "C" NODE *lsave(NODE *);
extern int logo_stop_flag;
extern "C" int stop_quietly_flag;

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------


// externs from the logo interpreter 
extern "C" int start (int , char **);
extern "C" void redraw_graphics(); 
extern "C" char *strnzcpy(char *, char *, int);

// need to redraw turtle graphics
int needToRefresh = 0;


// IO buffer handling
char nameBuffer [NAME_BUFFER_SIZE];

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
extern "C" void *eval_buttonact;

extern "C" void wxLogoSleep(unsigned int milli) {
  //may not work on mac according to wxWidgets doc
  wxDateTime stop_waiting = wxDateTime::UNow() + wxTimeSpan(0,0,0,milli);
  flushFile(stdout);
  extern void wx_refresh();

  wx_refresh();
  while(wxDateTime::UNow().IsEarlierThan(stop_waiting)) {
    if(check_wx_stop(1, 0) || eval_buttonact) {  //force yielding
      break;
    }
    wx_refresh();
    wxMilliSleep(10);
  }
}

/* Called by the logo thread to display a character onto the terminal screen */
extern "C" void printToScreen(char c, FILE * stream) 
{ 
  if (stream != stdout) {
    putc(c, stream);
    return;
  }

    if (c == 0x7) {	/* ^G */
	wxBell();
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


extern "C" int wxBuffContainsLine() {
  if (buff_push_index == buff_pop_index) {
    return false;
  } else if (buff_pop_index < buff_push_index) {
    for (int i = buff_pop_index; i <= buff_push_index; i++) {
      if (buff[i] == '\n') {
        return true;
      }
    }
  } else {
    for (int i = 0; i <= buff_push_index; i++) {
      if (buff[i] == '\n') {
        return true;
      }
    }
    for (int i = buff_pop_index; i < BUFF_LEN; i++) {
      if (buff[i] == '\n') {
        return true;
      }
    }
  }

  return false;
}


extern "C" int getFromWX_2(FILE * f);

extern "C" int getFromWX()
{ 
  return getFromWX_2(stdin);
}

extern "C" int getFromWX_2(FILE * f)
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
    // Do this while the lock is released just in case the longjump occurs
    if (check_wx_stop(1, 1)) {   // force yield (1)
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

extern "C" void getExecutableDir(char * path, int maxlen) {
  wxString executable = wxStandardPaths::Get().GetExecutablePath();
  wxString executable_dir =  wxFileName(executable).GetPath();
  strncpy(path, (const char*)executable_dir.mb_str(wxConvUTF8), maxlen);
  path[maxlen - 1] = '\0';
}


void doLoad(char * name) {
    strnzcpy(nameBuffer, name, NAME_BUFFER_SIZE - 1);
    (void)lload(cons(make_static_strnode(nameBuffer),NIL));
    stop_quietly_flag = 1;
    logo_stop_flag = 1;
}

void doSave(char * name) {
    strnzcpy(nameBuffer, name, NAME_BUFFER_SIZE - 1);
    (void)lsave(cons(make_static_strnode(nameBuffer),NIL));
    stop_quietly_flag = 1;
    logo_stop_flag = 1;
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

extern "C" long wxLaunchExternalEditor(char *editor, char *filename) {
  return wxExecute(wxString::Format("%s %s", editor, filename), wxEXEC_SYNC);
}
