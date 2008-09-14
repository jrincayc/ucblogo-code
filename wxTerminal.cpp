/* This file implements the logo frame, which is the main frame that
   contains the terminal, turtle graphics and the editor.

CONTAINED IN #IF 0 BLOCKS:
-- if user types input, presses UP, and then DOWN, it remembers it...
   feature isn't very complete, though...
   to implement, probably need TWO copies of the history buffer

*/

#include <iostream>

#ifdef __GNUG__
    #pragma implementation "wxTerminal.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/timer.h>
extern std::string pathString;
#include <wx/stdpaths.h>
#include <ctype.h>

extern "C" int readingInstruction;

#include <wx/print.h>
#include "LogoFrame.h"
#include "wxGlobals.h"
#include <wx/clipbrd.h>
#include <wx/html/htmprint.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/dcbuffer.h>  //buffered_DC
#include "wxTurtleGraphics.h"
#include "TextEditor.h"
#ifndef __WXMSW__
    #include "config.h"
#endif
#include "wxTerminal.h"		/* must come after wxTurtleGraphics.h */
#include <wx/fontdlg.h>
#ifdef __WXMAC__                                                        
    #undef wxFontDialog
    #include "wx/mac/fontdlg.h"
    #include <Carbon/Carbon.h>                                              
#endif                                                                  

// in Visual Studio 6.0, min and max are not defined up this point
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

using namespace std;

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------
// This is for the input for terminal-like behavior
char input_buffer [MAXINBUFF+1];
// How far into the input_buffer we are
int input_index = 0;
// Where the cursor is in the input_buffer
int input_current_pos = 0;


//history storage
char *cmdHistory[HIST_MAX] = {0};
char **hist_inptr, **hist_outptr; 

#if 0
//temporary storage of currently typed input
//so that if we retrieve history and then come back, we can see it again
char latest_history_buffer[MAXINBUFF];
int latest_history_stored = 0;
#endif

// if logo is in character mode
int logo_char_mode;
extern "C" int reading_char_now;
// the terminal DC
wxFont old_font;
wxTextAttr old_style;
// when edit is called in logo
TextEditor * editWindow;
// the turtle graphics we are using
TurtleCanvas * turtleGraphics; 
// this contains the previous 3 window
wxBoxSizer *topsizer;
LogoFrame *logoFrame;
LogoEventManager *logoEventManager;

// used to calculate where the cursor should be
int cur_x = 0, cur_y = 0;
int first = 1;
int last_logo_x = 0, last_logo_y = 0;
int last_input_x = 0, last_input_y = 0;
// the menu
wxMenuBar* menuBar;

extern "C" void wxTextScreen();

char *fooargv[2] = {"UCBLogo", 0};

// This is for stopping logo asynchronously
#ifdef SIG_TAKES_ARG
extern "C" RETSIGTYPE logo_stop(int);
extern "C" RETSIGTYPE logo_pause(int);
#else
extern "C" RETSIGTYPE logo_stop();
extern "C" RETSIGTYPE logo_pause();
#endif
int logo_stop_flag = 0;
int logo_pause_flag = 0;

// this is a static reference to the main terminal
wxTerminal *wxTerminal::terminal;


// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

enum
{
	
    Menu_File = 200,
	Menu_File_Save,
	Menu_File_Save_As,
	Menu_File_Load,
    Menu_File_Page_Setup,
	Menu_File_Print_Text,
	Menu_File_Print_Text_Prev,
	Menu_File_Print_Turtle,
	Menu_File_Print_Turtle_Prev,
	Menu_File_Quit,
	
	Menu_Edit = 300,
    Menu_Edit_Copy,
	Menu_Edit_Paste,
	
	Menu_Logo = 400,
	Menu_Logo_Stop,
	Menu_Logo_Pause,
	
	Menu_Font = 500,
        Menu_Font_Choose,
	Menu_Font_Inc,
	Menu_Font_Dec,
	
	Menu_Help = 600,
	Menu_Help_Man,
	
	Edit_Menu_File = 700,
	Edit_Menu_File_Close_Accept,
	Edit_Menu_File_Close_Reject,
    Edit_Menu_File_Page_Setup,
    Edit_Menu_File_Print_Text,

	
	Edit_Menu_Edit = 800,
    Edit_Menu_Edit_Copy,
	Edit_Menu_Edit_Paste,
	Edit_Menu_Edit_Cut,
	Edit_Menu_Edit_Find,
	Edit_Menu_Edit_Find_Next
	
};

// ----------------------------------------------------------------------------
// LogoApplication class
// ----------------------------------------------------------------------------


bool LogoApplication::OnInit()
{

  logoFrame = new LogoFrame
    (_T("Berkeley Logo"));

  logoFrame->Show(TRUE);
  SetTopWindow(logoFrame);

#ifndef __WXMAC__
  m_mainLoop = new wxEventLoop();
#endif

  logoEventManager = new LogoEventManager(this);

  return TRUE;	
}

extern "C" int start (int, char **);


int LogoApplication::OnRun()
{
#ifndef __WXMAC__
  wxEventLoop::SetActive(m_mainLoop);
#endif
  //SetExitOnFrameDelete(true);

#ifndef __WXMAC__   /* needed for wxWidgets 2.6 */
  wxSetWorkingDirectory(wxStandardPaths::Get().GetDocumentsDir());
#endif

  // fix the working directory in mac
#ifdef __WXMAC__
  char path[1024];
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  assert( mainBundle );
  
  CFURLRef mainBundleURL = CFBundleCopyBundleURL( mainBundle);
  assert( mainBundleURL);
  
  CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
  assert( cfStringRef);
  
  CFStringGetCString( cfStringRef, path, 1024, kCFStringEncodingASCII);
  
  CFRelease( mainBundleURL);
  CFRelease( cfStringRef);
  
  //std::string pathString(path);
  pathString = path;
  pathString+="/Contents/Resources/";
  //	chdir(pathString.c_str());
  
#endif

  start(1, fooargv);
  return 0;
}

int LogoApplication::OnExit() {
  return 0;
}

// ----------------------------------------------------------------------------
// LogoEventManager class
// ----------------------------------------------------------------------------

LogoEventManager::LogoEventManager(LogoApplication *logoApp)
{
  m_logoApp = logoApp;
}

extern "C" void wx_refresh();
void LogoEventManager::ProcessEvents(int force_yield)
{
  static int inside_yield = 0;
  static int yield_delay = 500;  // carefully tuned fudge factor
  static int foo = yield_delay;

  foo--;

  if( m_logoApp->Pending() ) {
    while( m_logoApp->Pending() )
       m_logoApp->Dispatch();
  }
  else {
    if(force_yield || foo == 0) {
      if(!inside_yield) {
        inside_yield++;
        m_logoApp->Yield(TRUE);
        inside_yield--;
      }
    }
  }

  if(foo == 0) {
    wx_refresh();
    foo = yield_delay;
  }
}


// ----------------------------------------------------------------------------
// LogoFrame class
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (LogoFrame, wxFrame)
EVT_MENU(Menu_File_Save,				LogoFrame::OnSave)
EVT_MENU(Menu_File_Save_As,			LogoFrame::OnSaveAs)
EVT_MENU(Menu_File_Load,				LogoFrame::OnLoad)
EVT_MENU(Menu_File_Page_Setup,			TurtleCanvas::OnPageSetup)
EVT_MENU(Menu_File_Print_Text,			LogoFrame::OnPrintText)
EVT_MENU(Menu_File_Print_Text_Prev,		LogoFrame::OnPrintTextPrev)
EVT_MENU(Menu_File_Print_Turtle,	TurtleCanvas::PrintTurtleWindow)
EVT_MENU(Menu_File_Print_Turtle_Prev,   TurtleCanvas::TurtlePrintPreview)
EVT_MENU(Menu_File_Quit,			LogoFrame::OnQuit)
EVT_MENU(Menu_Edit_Copy,			LogoFrame::DoCopy)
EVT_MENU(Menu_Edit_Paste,			LogoFrame::DoPaste)
EVT_MENU(Menu_Logo_Pause,			LogoFrame::DoPause)
EVT_MENU(Menu_Logo_Stop,			LogoFrame::DoStop)
EVT_MENU(Menu_Font_Choose,                      LogoFrame::OnSelectFont)
EVT_MENU(Menu_Font_Inc,				LogoFrame::OnIncreaseFont)
EVT_MENU(Menu_Font_Dec,				LogoFrame::OnDecreaseFont)
EVT_MENU(Edit_Menu_File_Close_Accept,	LogoFrame::OnEditCloseAccept)
EVT_MENU(Edit_Menu_File_Close_Reject,	LogoFrame::OnEditCloseReject)
EVT_MENU(Edit_Menu_File_Page_Setup,		TurtleCanvas::OnPageSetup)
EVT_MENU(Edit_Menu_File_Print_Text,		LogoFrame::OnEditPrint)
EVT_MENU(Edit_Menu_Edit_Copy,			LogoFrame::OnEditCopy)
EVT_MENU(Edit_Menu_Edit_Cut,			LogoFrame::OnEditCut)
EVT_MENU(Edit_Menu_Edit_Paste,			LogoFrame::OnEditPaste)
EVT_MENU(Edit_Menu_Edit_Find,			LogoFrame::OnEditFind)
EVT_MENU(Edit_Menu_Edit_Find_Next,		LogoFrame::OnEditFindNext)
EVT_CLOSE(LogoFrame::OnCloseWindow)
END_EVENT_TABLE()

#include "ucblogo.xpm"

extern "C" void wxSetTextColor(int fg, int bg);

//this should compute the size based on the chosen font!
LogoFrame::LogoFrame (const wxChar *title,
 int xpos, int ypos,
 int width, int height)
  : wxFrame( (wxFrame *) NULL, -1, title,
	     wxPoint(xpos, ypos),
	     wxSize(width, height)) {
  // the topsizer allows different resizeable segments in the main frame (i.e. for 
  // turtle graphics and the terminal displaying simultaneously)
  SetMinSize(wxSize(100, 100));
  SetIcon(wxIcon(ucblogo));
  logoFrame = this;
  topsizer = new wxBoxSizer( wxVERTICAL );
  wxTerminal::terminal = new wxTerminal (this, -1, wxPoint(-1, -1),
			        TERM_COLS, TERM_ROWS,  _T(""));
  turtleGraphics = new TurtleCanvas( this );

  wxFont f(FONT_CFG(wx_font_face, wx_font_size));

  wxTerminal::terminal->SetFont(f);

  AdjustSize();

    wxSetTextColor(7+SPECIAL_COLORS, 0+SPECIAL_COLORS);
  editWindow = new TextEditor( this, -1, _T(""), wxDefaultPosition,
			      wxSize(100,60), wxTE_MULTILINE|wxTE_RICH, f);
  wxTerminal::terminal->isEditFile=0;
  
  topsizer->Add(
		editWindow,
		1,            // make vertically stretchable
		wxEXPAND |    // make horizontally stretchable
		wxALL,        //   and make border all around
		2 );  
 
  topsizer->Add(
		turtleGraphics,
		4,            // make vertically stretchable
		wxEXPAND |    // make horizontally stretchable
		wxALL,        //   and make border all around
		2 );
  topsizer->Add(
		wxTerminal::terminal,
		1,            // make vertically stretchable
		wxEXPAND |    // make horizontally stretchable
		wxALL,        //   and make border all around
		2 ); 
     

  topsizer->Show(wxTerminal::terminal, 1);
  topsizer->Show(turtleGraphics, 0);
  topsizer->Show(editWindow, 0);
  
  SetSizer( topsizer ); 
  
  //SetAutoLayout(true);
  //topsizer->Fit(this);
  //topsizer->SetSizeHints(this);  
  
  wxTerminal::terminal->SetFocus();
  SetUpMenu();
}

extern "C" int wx_leave_mainloop;
void LogoFrame::OnCloseWindow(wxCloseEvent& event)
{
  logo_stop_flag = 1;
  wx_leave_mainloop++;
  Destroy();
}

extern "C" int need_save;

void LogoFrame::OnQuit(wxCommandEvent& event)
{
    if (need_save) {
	wxMessageDialog dialog( NULL, _T("Save workspace before quitting?"),
			       _T("Quit Logo"), wxYES_DEFAULT|wxYES_NO|wxCANCEL);
	switch ( dialog.ShowModal() ) {
	    case wxID_YES:
		LogoFrame::OnSave(event);
	    /* falls through */
	    case wxID_NO:
		Close(TRUE);
	}
    } else Close(TRUE);
}



void LogoFrame::SetUpMenu(){
	int i;
	if(!menuBar)
		menuBar = new wxMenuBar( wxMB_DOCKABLE );
	else
		for(i=menuBar->GetMenuCount()-1;i>=0;i--)
			delete menuBar->Remove(i);

	
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append( Menu_File_Load, _T("Load Logo Session \tCtrl-O"));
	fileMenu->Append( Menu_File_Save, _T("Save Logo Session \tCtrl-S"));
	fileMenu->Append( Menu_File_Save_As, _T("Save As..."));
	fileMenu->AppendSeparator();
	fileMenu->Append( Menu_File_Page_Setup, _T("Page Setup"));
	fileMenu->Append( Menu_File_Print_Text, _T("Print Text Window"));
	fileMenu->Append( Menu_File_Print_Text_Prev, _T("Print Preview Text Window"));
	fileMenu->Append( Menu_File_Print_Turtle, _T("Print Turtle Graphics"));
	fileMenu->Append( Menu_File_Print_Turtle_Prev, _T("Turtle Graphics Print Preview"));
	fileMenu->AppendSeparator();
	fileMenu->Append(Menu_File_Quit, _T("Quit UCBLogo \tCtrl-Q"));
	
	
	wxMenu *editMenu = new wxMenu;
		
	menuBar->Append(fileMenu, _T("&File"));
	menuBar->Append(editMenu, _T("&Edit"));

	wxMenu *logoMenu = new wxMenu;
// #ifdef __WXMSW__
// 	editMenu->Append(Menu_Edit_Copy, _T("Copy \tCtrl-C"));
// 	editMenu->Append(Menu_Edit_Paste, _T("Paste \tCtrl-V"));
// 
// 	logoMenu->Append(Menu_Logo_Pause, _T("Pause \tCtrl-P"));
// 	logoMenu->Append(Menu_Logo_Stop, _T("Stop \tCtrl-S"));	
// #else
	editMenu->Append(Menu_Edit_Copy, _T("Copy \tCtrl-C"));
	editMenu->Append(Menu_Edit_Paste, _T("Paste \tCtrl-V"));

	logoMenu->Append(Menu_Logo_Pause, _T("Pause \tAlt-P"));
	logoMenu->Append(Menu_Logo_Stop, _T("Stop \tAlt-S"));
// #endif
	menuBar->Append(logoMenu, _T("&Logo"));
	
	wxMenu *fontMenu = new wxMenu;
	fontMenu->Append(Menu_Font_Choose, _T("Select Font..."));
	fontMenu->Append(Menu_Font_Inc, _T("Increase Font Size \tCtrl-+"));
	fontMenu->Append(Menu_Font_Dec, _T("Decrease Font Size \tCtrl--"));
	menuBar->Append(fontMenu, _T("&Font"));
	
	/*wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(Menu_Help_Man, _T("Browse Online Manual"));
	menuBar->Append(helpMenu, _T("&Help"));*/
	
	SetMenuBar(menuBar);
}
void LogoFrame::DoCopy(wxCommandEvent& WXUNUSED(event)){
	wxTerminal::terminal->DoCopy();
}

void LogoFrame::DoPaste(wxCommandEvent& WXUNUSED(event)){
	wxTerminal::terminal->DoPaste();
}

extern "C" void new_line(FILE *);
int firstloadsave = 1;
extern "C" void *save_name;

void doSave(char * name, int length);
void doLoad(char * name, int length);

extern "C" void *cons(void*, void*);
extern "C" void lsave(void*);

void LogoFrame::OnSave(wxCommandEvent& event) {
    if (save_name != NULL) {
	lsave(cons(save_name, NULL));
    } else {
	OnSaveAs(event);
    }
}

void LogoFrame::OnSaveAs(wxCommandEvent& WXUNUSED(event)) {
	wxFileDialog dialog(this,
			    _T("Save Logo Workspace"),
			    (firstloadsave ?
#ifdef __WXMAC__   /* needed for wxWidgets 2.6 */
			      *wxEmptyString :
#else
			      wxStandardPaths::Get().GetDocumentsDir() :
#endif
			      *wxEmptyString),
			    wxEmptyString,
			    _T("Logo workspaces(*.lg)|*.lg|All files(*)|*"),
//			    "*",
#ifdef __WXMAC__   /* needed for wxWidgets 2.6 */
			    wxSAVE|wxOVERWRITE_PROMPT|wxCHANGE_DIR
#else
			    wxFD_SAVE|wxFD_OVERWRITE_PROMPT|wxFD_CHANGE_DIR
#endif
			    );
	
//	dialog.SetFilterIndex(1);
	if (dialog.ShowModal() == wxID_OK)
	{
#ifdef __WXMAC__
	    doSave((char *)dialog.GetPath().c_str(),
#else
	    doSave((char *)dialog.GetPath().char_str(wxConvUTF8),
#endif
		   dialog.GetPath().length());
	    new_line(stdout);
	}
    firstloadsave = 0;
}

void LogoFrame::OnLoad(wxCommandEvent& WXUNUSED(event)){
	wxFileDialog dialog
	(
	 this,
	 _T("Load Logo Workspace"),
	 (firstloadsave ?
#ifdef __WXMAC__   /* needed for wxWidgets 2.6 */
	    *wxEmptyString :
#else
	    wxStandardPaths::Get().GetDocumentsDir() :
#endif
			  *wxEmptyString),
	 wxEmptyString,
	 _T("Logo workspaces(*.lg)|*.lg|All files(*)|*"),
//	 "*",
#ifdef __WXMAC__   /* needed for wxWidgets 2.6 */
	 wxOPEN|wxFILE_MUST_EXIST|wxCHANGE_DIR
#else
	 wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR
#endif
	 );
		
	if (dialog.ShowModal() == wxID_OK) {
#ifdef __WXMAC__
	    doLoad((char *)dialog.GetPath().c_str(),
#else
	    doLoad((char *)dialog.GetPath().char_str(wxConvUTF8),
#endif
		   dialog.GetPath().length());
	    new_line(stdout);
	}
    firstloadsave = 0;
}


void LogoFrame::OnPrintText(wxCommandEvent& WXUNUSED(event)){
	wxHtmlEasyPrinting *htmlPrinter=wxTerminal::terminal->htmlPrinter;
	if(!htmlPrinter){
		htmlPrinter = new wxHtmlEasyPrinting();
		int fontsizes[] = { 6, 8, 12, 14, 16, 20, 24 };
		htmlPrinter->SetFonts(_T("Courier"),_T("Courier"), fontsizes);
	}
	wxString *textString = wxTerminal::terminal->get_text();
	
	
	htmlPrinter->PrintText(*textString);	
	delete textString;
}

void LogoFrame::OnPrintTextPrev(wxCommandEvent& WXUNUSED(event)){
	wxHtmlEasyPrinting *htmlPrinter=wxTerminal::terminal->htmlPrinter;
	if(!htmlPrinter){
		htmlPrinter = new wxHtmlEasyPrinting();
		int fontsizes[] = { 6, 8, 12, 14, 16, 20, 24 };
		htmlPrinter->SetFonts(_T("Courier"),_T("Courier"), fontsizes);
	}
	wxString *textString = wxTerminal::terminal->get_text();
	
	htmlPrinter->PreviewText(*textString,_T(""));
	
}

extern "C" void wxSetFont(char *fm, int sz);

void LogoFrame::OnSelectFont(wxCommandEvent& WXUNUSED(event)) {
    wxFontData data;
    data.SetInitialFont(wxTerminal::terminal->GetFont());

    wxFontDialog dialog(this, data);
    if ( dialog.ShowModal() == wxID_OK )
    {
        wxFontData retData = dialog.GetFontData();
        wxFont font = retData.GetChosenFont();

#ifdef __WXMAC__
	wxSetFont((char *)font.GetFaceName().c_str(), 
		  font.GetPointSize());
#else
	wxSetFont((char *)font.GetFaceName().char_str(wxConvUTF8), 
		  font.GetPointSize());	
#endif
    }    
}

void LogoFrame::OnIncreaseFont(wxCommandEvent& WXUNUSED(event)){
	int expected;
	
	wxFont font = wxTerminal::terminal->GetFont();
	expected = font.GetPointSize()+1;
	
	// see that we have the font we are trying to use
	// since for some fonts, the next size is +2
	while(font.GetPointSize() != expected && expected <= 24){
		expected++;
		font.SetPointSize(expected);
	}

	wxSetFont(wx_font_face, expected);
	
}

void LogoFrame::OnDecreaseFont(wxCommandEvent& WXUNUSED(event)){
	int expected;

	wxFont font = wxTerminal::terminal->GetFont();
	expected = font.GetPointSize()-1;
	
	// see that we have the font we are trying to use
	while(font.GetPointSize() != expected && expected >= 6){
		expected--;
		font.SetPointSize(expected);
	}	

	wxSetFont(wx_font_face, expected);

}

extern "C" void *Unbound;

extern "C" void *IncreaseFont(void *) {
    wxCommandEvent dummy;
    logoFrame->OnIncreaseFont(dummy);
    return Unbound;
}

extern "C" void *DecreaseFont(void *) {
    wxCommandEvent dummy;
    logoFrame->OnDecreaseFont(dummy);
    return Unbound;
}
 
void LogoFrame::AdjustSize() {
  int char_width, char_height;	
  wxTerminal::terminal->GetCharSize(&char_width, &char_height);
  wxSize sz(char_width * TERM_COLS, char_height * TERM_ROWS + 5 );
  SetClientSize(sz);
  //Layout();
}

void LogoFrame::DoStop(wxCommandEvent& WXUNUSED(event)){
  logo_stop_flag = 1;
}


void LogoFrame::DoPause(wxCommandEvent& WXUNUSED(event)){
  logo_pause_flag = 1;
}

void LogoFrame::OnEditCloseAccept(wxCommandEvent& WXUNUSED(event)){
	editWindow->OnCloseAccept();
}
void LogoFrame::OnEditCloseReject(wxCommandEvent& WXUNUSED(event)){
	editWindow->OnCloseReject();
}
void LogoFrame::OnEditPrint(wxCommandEvent& WXUNUSED(event)){
	editWindow->DoPrint();
}
void LogoFrame::OnEditCopy(wxCommandEvent& WXUNUSED(event)){
	editWindow->DoCopy();
}
void LogoFrame::OnEditCut(wxCommandEvent& WXUNUSED(event)){
	editWindow->DoCut();
}
void LogoFrame::OnEditPaste(wxCommandEvent& WXUNUSED(event)){
	editWindow->Paste();
}
void LogoFrame::OnEditFind(wxCommandEvent& WXUNUSED(event)){
	editWindow->OnFind();
}
void LogoFrame::OnEditFindNext(wxCommandEvent& WXUNUSED(event)){
	editWindow->OnFindNext();
}
void LogoFrame::OnEditSave(wxCommandEvent& WXUNUSED(event)){
	editWindow->OnSave();
}

void LogoFrame::SetUpEditMenu(){
	int i;
	for(i=menuBar->GetMenuCount()-1;i>=0;i--)
		delete menuBar->Remove(i);
	
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append( Edit_Menu_File_Close_Accept, _T("Close and Accept Changes \tAlt-A"));
	fileMenu->Append( Edit_Menu_File_Close_Reject, _T("Close and Revert Changes \tAlt-R"));
	fileMenu->AppendSeparator();
	fileMenu->Append( Edit_Menu_File_Page_Setup, _T("Page Setup"));
	fileMenu->Append( Edit_Menu_File_Print_Text, _T("Print... \tCtrl-P"));
	
	wxMenu *editMenu = new wxMenu;
	
	menuBar->Append(fileMenu, _T("&File"));
	menuBar->Append(editMenu, _T("&Edit"));
	
	
	editMenu->Append(Edit_Menu_Edit_Cut, _T("Cut \tCtrl-X"));
	editMenu->Append(Edit_Menu_Edit_Copy, _T("Copy \tCtrl-C"));
	editMenu->Append(Edit_Menu_Edit_Paste, _T("Paste \tCtrl-V"));
	editMenu->AppendSeparator();
	editMenu->Append(Edit_Menu_Edit_Find, _T("Find... \tCtrl-F"));
	editMenu->Append(Edit_Menu_Edit_Find_Next, _T("Find Next \tCtrl-G"));
	
	wxMenu *fontMenu = new wxMenu;
	fontMenu->Append(Menu_Font_Choose, _T("Select Font..."));
	fontMenu->Append(Menu_Font_Inc, _T("Increase Font Size \tCtrl-+"));
	fontMenu->Append(Menu_Font_Dec, _T("Decrease Font Size \tCtrl--"));
	menuBar->Append(fontMenu, _T("&Font"));
	
	
	logoFrame->SetMenuBar(menuBar);
}

     

// ----------------------------------------------------------------------------
// wxTerminal
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()
  DECLARE_EVENT_TYPE(wxEVT_MY_CUSTOM_COMMAND, 7777)
  END_DECLARE_EVENT_TYPES()
  DEFINE_EVENT_TYPE(wxEVT_MY_CUSTOM_COMMAND)

#define EVT_MY_CUSTOM_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_MY_CUSTOM_COMMAND, id, -1, \
        (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)&fn, \
        (wxObject *) NULL \
    ),


BEGIN_DECLARE_EVENT_TYPES()
  DECLARE_EVENT_TYPE(wxEVT_TERM_CUSTOM_COMMAND, 7777)
  END_DECLARE_EVENT_TYPES()
  DEFINE_EVENT_TYPE(wxEVT_TERM_CUSTOM_COMMAND)

#define wxEVT_TERM_CUSTOM_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_TERM_CUSTOM_COMMAND, id, -1, \
        (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)&fn, \
        (wxObject *) NULL \
    ),


BEGIN_EVENT_TABLE(wxTerminal, wxWindow)
EVT_ERASE_BACKGROUND(wxTerminal::OnEraseBackground)
EVT_PAINT(wxTerminal::OnPaint)
EVT_CHAR(wxTerminal::OnChar)
EVT_LEFT_DOWN(wxTerminal::OnLeftDown)
EVT_LEFT_UP(wxTerminal::OnLeftUp)
EVT_MOTION(wxTerminal::OnMouseMove)
//EVT_MY_CUSTOM_COMMAND(-1, wxTerminal::Flush)
EVT_SIZE(wxTerminal::OnSize)
EVT_KILL_FOCUS(wxTerminal::LoseFocus)
END_EVENT_TABLE()

wxCommandEvent * haveInputEvent = new wxCommandEvent(wxEVT_MY_CUSTOM_COMMAND);

extern "C" void color_init(void);

 wxTerminal::wxTerminal(wxWindow* parent, wxWindowID id,
               const wxPoint& pos,
               int width, int height,
               const wxString& name) :
  wxScrolledWindow(parent, id, pos, wxSize(-1, -1), wxWANTS_CHARS|wxVSCROLL, name)
//    wxScrolledWindow(parent, id, pos, wxSize(-1, -1), wxWANTS_CHARS, name)
{
  //allocating char structures.
  term_chars = (wxterm_char_buffer *) malloc(sizeof(wxterm_char_buffer));
  memset(term_chars, '\0', sizeof(wxterm_char_buffer));
  term_lines = (wxterm_line_buffer *) malloc(sizeof(wxterm_line_buffer));
  memset(term_lines, 0, sizeof(wxterm_line_buffer));
  term_chars->next = 0;
  term_lines->next = 0;

  //initializations
  curr_line_pos.buf = term_lines;
  curr_line_pos.offset = 0;
  line_of(curr_line_pos).buf = term_chars;
  line_of(curr_line_pos).offset = 0;

  curr_char_pos.buf = term_chars;
  curr_char_pos.offset = 0;
  //curr_char_pos line_length not used!

  //initializing history
  hist_inptr = hist_outptr = cmdHistory; 

  m_width = width;
  m_height = height;


  m_currMode = 0;
  y_max = 0;
  x_max = m_width - 1;


  // start us out not in char mode
  logo_char_mode = 0;
  // For printing the text
  htmlPrinter = 0;
  //  set_mode_flag(DESTRUCTBS);
  wxTerminal::terminal = this;
  int
    i;

  m_selecting = FALSE;
  m_selx1 = m_sely1 = m_selx2 = m_sely2 = 0;
  m_seloldx1 = m_seloldy1= m_seloldx2 = m_seloldy2 = 0;
  //m_marking = FALSE;
  m_curX = -1;
  m_curY = -1;

  //  m_boldStyle = FONT;

  GetColors(m_colors);

    color_init();
  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
  SetBackgroundColour(TurtleCanvas::colors[m_curBG]);
  SetMinSize(wxSize(50, 50));

  for(i = 0; i < 16; i++)
    m_colorPens[i] = wxPen(m_colors[i], 1, wxSOLID);

  m_printerFN = 0;
  m_printerName = 0;

  wxClientDC
    dc(this);
  
  GetCharSize(&m_charWidth, &m_charHeight);
  //dc.GetTextExtent("M", &m_charWidth, &m_charHeight);
  //  m_charWidth--;

  m_inputReady = FALSE;
  m_inputLines = 0;
  
  //  int x, y;
  //GetSize(&x, &y);  
  
  //  parent->SetSize(-1,-1, m_charWidth * width, m_charHeight * height + 1);
  //  SetScrollbars(m_charWidth, m_charHeight, 0, 30);
  //SetScrollRate(0, m_charHeight);
  //SetVirtualSize(m_charWidth * width, 2 * m_charHeight);  //1 row (nothing displayed yet)


  //  ResizeTerminal(width, height);

}

wxTerminal::~wxTerminal()
{
  //clean up the buffers
  wxTerminal::terminal = 0;
}

void wxTerminal::deferUpdate(int flag) {
    if (flag)
	set_mode_flag(DEFERUPDATE);
    else
	clear_mode_flag(DEFERUPDATE);
}

void wxTerminal::set_mode_flag(int flag) {
  m_currMode |= flag;
}

void wxTerminal::clear_mode_flag(int flag) {
  m_currMode &= ~flag;
}


bool
wxTerminal::SetFont(const wxFont& font)
{
  wxWindow::SetFont(font);

  m_normalFont = font;
  m_underlinedFont = font;
  m_underlinedFont.SetUnderlined(TRUE);
  m_boldFont = font;
  m_boldFont.SetWeight(wxFONTWEIGHT_BOLD);
  m_boldUnderlinedFont = m_boldFont;
  m_boldUnderlinedFont.SetUnderlined(TRUE);
  GetCharSize(&m_charWidth, &m_charHeight);
  //  wxClientDC
  //	  dc(this);
  
  //  dc.GetTextExtent("M", &m_charWidth, &m_charHeight);
  //  m_charWidth--;
  ResizeTerminal(m_width, m_height);
  if(!(m_currMode & DEFERUPDATE)) {
    Refresh();
  } 
  return TRUE;
}

void 
wxTerminal::GetCharSize(int *cw, int *ch) {
  wxClientDC
    dc(this);
  
  //int dummy;
  //dc.GetTextExtent("M", cw, &dummy);
  //dc.GetTextExtent("(", &dummy, ch);

  int descent, extlead; 
  dc.GetTextExtent("M", cw, ch, &descent, &extlead);
  //for the tails of g's and y's, if needed.
#ifdef __WXMSW__
    *ch += descent + extlead + 1;
#endif
}

void
wxTerminal::GetColors(wxColour colors[16] /*, wxTerminal::BOLDSTYLE boldStyle*/)
{
    colors[0] = wxColour(0, 0, 0);                             // black
    colors[1] = wxColour(255, 0, 0);                           // red
    colors[2] = wxColour(0, 255, 0);                           // green
    colors[3] = wxColour(255, 0, 255);                         // yellow
    colors[4] = wxColour(0, 0, 255);                           // blue
    colors[5] = wxColour(255, 255, 0);                         // magenta
    colors[6] = wxColour(0, 255, 255);                         // cyan
    colors[7] = wxColour(255, 255, 255);                       // white
    colors[8] = wxColour(0, 0, 0);                             // black
    colors[9] = wxColour(255, 0, 0);                           // red
    colors[10] = wxColour(0, 255, 0);                          // green
    colors[11] = wxColour(255, 0, 255);                        // yellow
    colors[12] = wxColour(0, 0, 255);                          // blue
    colors[13] = wxColour(255, 255, 0);                        // magenta
    colors[14] = wxColour(0, 255, 255);                        // cyan
    colors[15] = wxColour(255, 255, 255);                      // white
}



/*
 * ProcessInput()
 *
 * passes input to logo, one line at a time
 * and prints to terminal as well
 *
 * assumes cursor is set at last logo position already 
 */
void wxTerminal::ProcessInput() {
  //pass input up to newline.
  int i;
  for(i = 0; i < input_index; i++) {
    if(input_buffer[i] == '\n') break;
  }
  PassInputToTerminal(i+1,input_buffer); //include '\n'
  last_logo_x = cursor_x;
  last_logo_y = cursor_y;
  
  PassInputToInterp();
  m_inputLines--;
  if(!m_inputLines) m_inputReady = FALSE;  
}


/*
 * Flush()
 * 
 * Handles output from logo
 */
void  wxTerminal::Flush (){
  set_mode_flag(BOLD);

  if(output_index == 0) {
    clear_mode_flag(BOLD);
    return;
  }


  bool cursor_moved = FALSE;

  if(input_index != 0){
    // set the cursor in the proper place 
    setCursor(last_logo_x, last_logo_y);
    //    scroll_region(last_logo_y, last_logo_y + 1, -1);
    cursor_moved = TRUE;
    
  }    //calculate new cursor location
  if (output_index != 0) {
    ClearSelection(); //eventually, only clear if overlap with updated region?  
    PassInputToTerminal(output_index, output_buffer);
    output_index = 0;
  }

  //save current cursor position to last_logo
  last_logo_x = cursor_x;
  last_logo_y = cursor_y;

  clear_mode_flag(BOLD);
  if(cursor_moved/* && readingInstruction*/){   
    cursor_moved = FALSE;

    if(m_inputReady && readingInstruction) {
      ProcessInput();
    }
    else {
      //pass the input up to the current input location to terminal
      //e.g. cpos is 6, then pass chars 0 to 5 to terminal (6 chars)
      PassInputToTerminal(input_current_pos, input_buffer);
      int new_cursor_x = cursor_x, new_cursor_y = cursor_y;
      //pass the rest of input to terminal
      //e.g. inputindex is 20, then pass chars 6 to 19 to terminal (14 chars)
      PassInputToTerminal(input_index-input_current_pos, input_buffer+input_current_pos);
      // set last_input coords
      last_input_x = cursor_x;
      last_input_y = cursor_y;
      // and set cursor back to proper location
      setCursor(new_cursor_x, new_cursor_y);
    }
  }

  if(!(m_currMode & DEFERUPDATE)) {
    Refresh();
  }

}


/* 
	PassInputToInterp() 
        takes all characters in the input buffer 
	up to the FIRST '\n' and hands
	them off to the logo interpreter
        if logo_char_mode, then just send the character
 ** does not edit cursor locations!
 */
void wxTerminal::PassInputToInterp() {
  int i;  
  if(logo_char_mode){
    //buff[buff_index++] = input_buffer[--input_index];
    buff_push(input_buffer[--input_index]);
    
    input_index = 0;
    input_current_pos = 0;
  }
  else {
    for (i = 0; i < input_index; i++) {
      buff_push(input_buffer[i]);	
      if(input_buffer[i] == '\n') {
	break;
      }
    }
    int saw_newline = i;

    //so create a string of size i+1 and copy the contents over
    char *histline = (char *)malloc(1+i);
    for(i = 0; i < saw_newline; i++) {
      histline[i] = input_buffer[i];
    }
    histline[saw_newline] = 0;
    //put it in the history
    *hist_inptr++ = histline;
    if (hist_inptr >= &cmdHistory[HIST_MAX]) {  //wraparound
      hist_inptr = cmdHistory;
    }
    if (*hist_inptr) { 
      free(*hist_inptr);
      *hist_inptr = 0;
    }
    hist_outptr = hist_inptr;

    for(i = saw_newline + 1; i < input_index; i++) {
      input_buffer[i - saw_newline - 1] = input_buffer[i];
    }
    // a to b, length is b - a + 1
    input_index = input_index - saw_newline - 1;
    input_current_pos = input_index;
  }
}


void wxTerminal::DoCopy(){
	
	if (wxTheClipboard->Open())
	{
		// This data objects are held by the clipboard, 
		// so do not delete them in the app.
		wxTheClipboard->SetData( new wxTextDataObject(GetSelection()) );
		wxTheClipboard->Close();
	}
	
}


void wxTerminal::DoPaste(){
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported( wxDF_TEXT ))
		{
		  wxClientDC dc(this);

		  wxTextDataObject data;
		  wxTheClipboard->GetData( data );
		  wxString s = data.GetText();
		  
		  int i; 
		  //char chars[2];
		  char c;
		  int num_newlines = 0;
		  int len;
		  char prev = ' ';
		  for (i = 0; i < s.Length() && input_index < MAXINBUFF; i++){
		    len = 1;
		    c = s.GetChar(i);
		    if (c == '\n') {
		      num_newlines++;
		    }
		    if (prev == ' ' && c == ' ') {
		      continue;
		    }
		    prev = c;
		    if(input_index < MAXINBUFF) {
		      input_buffer[input_index++] = c;
		    }
		    else {
		      break;
		    }
		    ClearSelection();
		    PassInputToTerminal(len, &c);
		  }
		  m_inputLines = num_newlines;
		  input_current_pos = input_index;
		}  
		wxTheClipboard->Close();
	}
	
}


void wxTerminal::LoseFocus (wxFocusEvent & event) {
}


extern "C" int keyact_set(void);
extern "C" void do_keyact(int);

/*
 OnChar
 is called each time the user types a character
 in the main terminal window
 */


void
wxTerminal::OnChar(wxKeyEvent& event)
{
  ClearSelection();
  int
      keyCode = 0,
      len;
  char buf[10];


  keyCode = (int)event.GetKeyCode();
  if (!readingInstruction && !reading_char_now && keyact_set()) {
    if (keyCode == WXK_RETURN) {
      keyCode = '\n';
    }
    else if (keyCode == WXK_BACK) {
      keyCode = 8;
    }
    else if (keyCode >= WXK_START) {
	/* ignore it */
      return;  //not sure about this (evan)
    } 
    else {
      
    }
    if (event.ControlDown()) keyCode &= 0237;
    if (event.AltDown()) keyCode += 0200;

    do_keyact(keyCode);
  }
  else if(logo_char_mode){
    if (keyCode == WXK_RETURN) {
      keyCode = '\n';
    }
    else if (keyCode == WXK_BACK) {
      keyCode = 8;
    }
    else if (keyCode >= WXK_START) {
	/* ignore it */
      return;  //not sure about this (evan)
    } 
    else {
      
    }
    if (event.ControlDown()) keyCode &= 0237;
    if (event.AltDown()) keyCode += 0200;
    if(input_index < MAXINBUFF) {
      input_buffer[input_index++] = keyCode;
      input_current_pos++;
      PassInputToInterp();
    }
  }
  else if (keyCode == WXK_RETURN) {
    //the following condition should never happen.
    //input_buffer is size MAXINBUFF+1, to leave room for \n char
    //this is the only code segment that's allowed to add a \n
    //in that last slot.
    if(input_index > MAXINBUFF) fprintf(stderr, "onChar: WXK_RETURN, moved past end of input buffer. Should never happen!\n");  

    
    if(input_current_pos < input_index) {
      setCursor(last_input_x, last_input_y);
    }

    //    buf[0] = 10;
    //buf[1] = 13;
    //len = 2;
    
    input_buffer[input_index++] = '\n';
    input_current_pos = input_index;

    char newline = '\n';
    PassInputToTerminal(1,&newline);

    m_inputReady = TRUE;
    m_inputLines++;

    if(readingInstruction) {
      setCursor(last_logo_x, last_logo_y);
      ProcessInput(); 
    }
    else {
      last_input_x = cursor_x;
      last_input_y = cursor_y;
    }
  }
  else if (keyCode == WXK_BACK) {
    handle_backspace();
  }
  else if (keyCode == WXK_UP) { // up
    handle_history_prev();
  }
  else if  (keyCode == WXK_DOWN) { // down
    handle_history_next();
  }
  else if  (keyCode == WXK_LEFT) { // left    
    handle_left();
  }
  else if  (keyCode == WXK_RIGHT) { // right
    handle_right();
  }
  else if (keyCode == WXK_TAB) { // tab
    //do nothing for now. could be tab completion later.
  }
  else if  (keyCode == WXK_DELETE) { // DEL key
    if(input_current_pos < input_index) {
      handle_right();
      handle_backspace();
    }
  }
  else if  (keyCode == WXK_HOME) {  //HOME key
    handle_home();
  }
  else if  (keyCode == WXK_END) {  //END key
    handle_end();
  }
  else if (keyCode >= WXK_START) {
    /* ignore it */
  } 
  else {
    if(input_index >= MAXINBUFF) return;	

    buf[0] = keyCode;
    len = 1;
    int doInsert = 0;
    if (input_current_pos < input_index ) { // we are in the middle of input
      doInsert = 1;
      int i;
      for (i = input_index; i >= input_current_pos + 1; i--) {
	input_buffer[i] = input_buffer[i - 1]; 
      }
      input_buffer[input_current_pos] = keyCode;
      input_index++;
      input_current_pos++;
    }
    else {
      input_buffer[input_index++] = keyCode;
      input_current_pos++;
    }

    if (doInsert) {
      cur_x = cursor_x; cur_y = cursor_y;


      //remember, input_current_pos - 1 has last character typed
      PassInputToTerminal(input_index - (input_current_pos - 1),
			  (input_buffer + (input_current_pos - 1)));
	
      //now the cursor is where the last input position is
      last_input_x = cursor_x;
      last_input_y = cursor_y;
	
      //set cursor back to cursorPos
      if (cur_x == x_max)
	setCursor(0, cur_y + 1);
      else
	setCursor(cur_x+1, cur_y);
    } 
    else {
      PassInputToTerminal(len, buf);
      //now the cursor is where the last input position is
      last_input_x = cursor_x;
      last_input_y = cursor_y;	
    }   
  }

  if(!(m_currMode & DEFERUPDATE)) {
    Refresh();
  }

}


void wxTerminal::handle_backspace() {
  if (input_index == 0)
    return;
    

  if (input_current_pos == 0)
    return;
    
  bool removing_newline = FALSE;
  if(input_buffer[input_current_pos-1] == '\n') {
    removing_newline = TRUE;
  }
    
  for (int i = input_current_pos; i < input_index; i++) {
    input_buffer[i-1] = input_buffer[i]; 
  }
  input_index--;
  input_current_pos--;

  cur_x = cursor_x - 1, cur_y = cursor_y;
  if(cur_x < 0) {
    wxterm_linepos cpos = GetLinePosition(cursor_y - 1);
    // x_max if wrapped line,  line_length otherwise.
    cur_x = min(x_max, line_of(cpos).line_length);
    cur_y = cursor_y - 1;
    setCursor(cur_x, cur_y);
  }
  else {
    setCursor(cur_x, cur_y);
  }
    
  PassInputToTerminal(input_index - input_current_pos,
		      input_buffer + input_current_pos);
    
  //cursor_x , cursor_y now at input's last location
  last_input_x = cursor_x;
  last_input_y = cursor_y;
    
    
  if(removing_newline) {
    //add a second newline, to erase contents of the last
    //input line.
    //this causes a very specific "feature"....
    //if last input line contains other noninput chars
    //then doing this will erase them too
    //this situation can happen when you use setcursor and hit Backspace...
    //it's a very specific situation that should not clash with 
    //intended behavior...
    char nl = '\n';
    PassInputToTerminal(1, &nl);
    PassInputToTerminal(1, &nl);  //pass two newlines

    m_inputLines--; //merged two lines
  }
  else {
    char spc = ' ';
    PassInputToTerminal(1, &spc);
  }
    
  //set cursor back to backspace'd location
  setCursor(cur_x,cur_y); 
}

void wxTerminal::handle_home() {
  setCursor(last_logo_x, last_logo_y);
  input_current_pos = 0;
}

void wxTerminal::handle_end() {
  setCursor(last_input_x, last_input_y);
  input_current_pos = input_index;
}


void wxTerminal::handle_clear_to_end() {
#if 0
  //old code that put spaces everyhwere...
  int xval;
  int yval;

  xval = cursor_x;
  yval = cursor_y;

  int i;

  for (i = input_current_pos; i < input_index; i++) {
      if(input_buffer[i] == '\n') {
	input_buffer[i] = '\n';
      }
      else {	    
	input_buffer[i] = ' ';
      }
  }
  PassInputToTerminal(input_index-input_current_pos, &input_buffer[i]);
  
  setCursor(xval, yval); 
#else
  //make the current position the end of the buffer 
  //update line structure accordingly

  char_of(curr_char_pos) = '\0';
  line_of(curr_line_pos).line_length = cursor_x;
  y_max = cursor_y;
#endif
}



// warning: the history buffer WRAPS around!
// so to detect for the beginning/end, check for NULL string pointer

void wxTerminal::handle_history_prev() {
  
  // Now get a history entry
  if (--hist_outptr < cmdHistory) {
    hist_outptr = &cmdHistory[HIST_MAX-1];
  }

  if (*hist_outptr == 0) {
    wxBell();
    hist_outptr++;  //put it back to where it was
    if (hist_outptr >= &cmdHistory[HIST_MAX]) {  //wraparound
      hist_outptr = cmdHistory;      
    }
    return;
  }

#if 0
  //if we're not currently storing anything 
  //in latest_history_buffer:
  //store current input buffer in it.
  //we are about to change the input
  if(!latest_history_stored) {
    strcpy(latest_history_buffer, input_buffer);
    latest_history_buffer[input_index] = 0;
    latest_history_stored++;
  }
#endif

  handle_home();  //go to the start of input

  int hist_len = strlen((const char *) *hist_outptr);
  strcpy(input_buffer, (char *)*hist_outptr);
  PassInputToTerminal(hist_len, input_buffer);

  m_inputLines = 0;

  input_current_pos = hist_len;

  handle_clear_to_end();  //clear to the old end of input_buffer

  input_index = hist_len;

  //cursor_x , cursor_y now at input's last location
  last_input_x = cursor_x;
  last_input_y = cursor_y;	
}

void wxTerminal::handle_history_next() {
  if (*hist_outptr != 0) { 
      hist_outptr++;
  }

  if (hist_outptr >= &cmdHistory[HIST_MAX]) {  //wraparound
    hist_outptr = cmdHistory;
  }


  if(*hist_outptr == 0) {
#if 0
    //if latest_history_buffer is not empty, put that here.
    if(latest_history_stored) {	
      handle_home();  //go to front
      int latest_len = strlen(latest_history_buffer);
      PassInputToTerminal(latest_len, latest_history_buffer);
      strcpy(input_buffer, latest_history_buffer);
      input_current_pos = latest_len;

      handle_clear_to_end();  //clear to old end of input

      input_index = latest_len;

      m_inputLines = 0;

      latest_history_stored--;
      
      //cursor_x , cursor_y now at input's last location
      last_input_x = cursor_x;
      last_input_y = cursor_y;	
    }
    else {
      wxBell();    
    }
#else
    wxBell();
    handle_home();
    handle_clear_to_end();
    input_current_pos = 0;
    input_index = 0;
    m_inputLines = 0;
      
    //cursor_x , cursor_y now at input's last location
    last_input_x = cursor_x;
    last_input_y = cursor_y;	

#endif
    return;
  }

    
  handle_home();  //jump to beginning

  int hist_len = strlen((const char *) *hist_outptr);
  strcpy(input_buffer, (char *)*hist_outptr);
  PassInputToTerminal(hist_len, input_buffer);

  m_inputLines = 0;

  input_current_pos = hist_len;

  handle_clear_to_end(); //clear to old end of input_buffer

  input_index = hist_len;
  
  //cursor_x , cursor_y now at input's last location
  last_input_x = cursor_x;
  last_input_y = cursor_y;	
}

void wxTerminal::handle_left() {
  if(input_current_pos > 0) {
    if (cursor_x - 1 < 0) {	  
      //if previous char is a newline, then cursor goes to line_length
      //otherwise, it's a wrapped line, and should go to the end
      // just use min...
      wxterm_linepos cpos = GetLinePosition(cursor_y - 1);
      setCursor(min(x_max, line_of(cpos).line_length), cursor_y - 1);   
    }   
    else {
      setCursor(cursor_x - 1, cursor_y);
    }
    input_current_pos--;
  }
}

void wxTerminal::handle_right() {
  if(input_current_pos < input_index) {
    if (input_buffer[input_current_pos] == '\n' ||
	cursor_x + 1 > x_max) {
      setCursor(0, cursor_y + 1);
    }
    else {
      setCursor(cursor_x + 1, cursor_y);	  
    }
    input_current_pos++;
  }
}


void wxTerminal::setCursor (int x, int y, bool fromLogo) {


  int vis_x,vis_y;
  GetViewStart(&vis_x,&vis_y);


  if(fromLogo) {
    //need to change to unscrolled coordinates

    if(x < 0 || x > m_width ||
       y < 0 || y > m_height) {
      return;
    }
    //GetViewStart(&cursor_x,&cursor_y);

    cursor_x = x;
    cursor_y = vis_y + y;

    
  }
  else {
    cursor_x = x;
    cursor_y = y;
  }


  int want_x, want_y;
  want_x = cursor_x;
  want_y = cursor_y;

  if(cursor_y < 0) {
    fprintf(stderr, "cursor_y < 0 in setcursor. BAD \n");
  }
  cursor_y = min(cursor_y,y_max);
  curr_line_pos = GetLinePosition(cursor_y);
  curr_char_pos = line_of(curr_line_pos);
  if(cursor_y < want_y ||  
     cursor_x > curr_char_pos.line_length) {
    cursor_x = curr_char_pos.line_length;
  }
  curr_char_pos.offset = curr_char_pos.offset + cursor_x;
  adjust_charpos(curr_char_pos);

  if(fromLogo && 
     (cursor_x != want_x ||
      cursor_y != want_y)) {
    //add spaces until we get to desired location

    if(cursor_x > x_max) {
      cursor_x = 0;
      cursor_y++;
      inc_linepos(curr_line_pos);
    }

    char space = ' ';  
    char newline = '\n';

    while(cursor_y != want_y) {
      PassInputToTerminal(1,&newline);
    }
    while(cursor_x != want_x) {
      PassInputToTerminal(1,&space);
    }
  }

  if(!(m_currMode & DEFERUPDATE)) {
    if(cursor_y >= vis_y &&
       cursor_y <= vis_y + m_height - 1) {
    }
    else {

      Scroll(-1, cursor_y);
      
      //      Refresh();
    }
  }
}


void wxTerminal::OnSize(wxSizeEvent& event) {      
	
  int x, y;
  GetSize(&x, &y);
  
  //leave one char on the right for scroll bar width
  if (m_width == (x / m_charWidth) - 1 && m_height == y / m_charHeight)
    return;
  x = (x / m_charWidth) - 1;
  y = y / m_charHeight;
  
  if (x < 1) 
    x = 1;
  if (y < 1) 
    y = 1;
  ResizeTerminal(x,y);
  Scroll(-1, cursor_y);
  Refresh();
}

wxMemoryDC * currentMemDC = NULL;
wxBitmap * currentBitmap = NULL;
int oldWidth = -1;
int oldHeight = -1;

void wxTerminal::OnEraseBackground(wxEraseEvent &WXUNUSED(event)) 
{
  //don't erase background.. for double buffering!
}

extern "C" void wxSetTextColor(int fg, int bg) {
    wxTerminal::terminal->m_curFG = fg;
    wxTerminal::terminal->m_curBG = bg;
    wxTerminal::terminal->SetBackgroundColour(
		TurtleCanvas::colors[wxTerminal::terminal->m_curBG]);
}

void wxTerminal::OnPaint(wxPaintEvent &WXUNUSED(event)) 
{
#ifndef __WXMAC__   /* needed for wxWidgets 2.6 */
  wxAutoBufferedPaintDC dc(this);
#else
  wxBufferedPaintDC dc(this);
#endif

  DoPrepareDC(dc);
  dc.SetBackground(TurtleCanvas::colors[m_curBG]);
  dc.Clear();
  OnDraw(dc);
}

void wxTerminal::OnDraw(wxDC& dc)
{  
  //DebugOutputBuffer();
  
  wxRect rectUpdate = GetUpdateRegion().GetBox();
  CalcUnscrolledPosition(rectUpdate.x, rectUpdate.y,
			 &rectUpdate.x, &rectUpdate.y);
  int lineFrom = rectUpdate.y / m_charHeight;
  int lineTo = rectUpdate.GetBottom() / m_charHeight;

  if ( lineTo > y_max)
    lineTo = y_max;
  
  //find the position!
  wxterm_linepos tlpos;
  tlpos.buf = term_lines;
  wxterm_charpos tline;
  tlpos.offset = lineFrom;
  adjust_linepos(tlpos);

  for ( int line = lineFrom; line <= lineTo; line++ )
  {
    tline = line_of(tlpos);
    for ( int col = 0; col < tline.line_length; col++ ) {      
      DrawText(dc, m_curFG, m_curBG, mode_of(tline), col, line, 1, &char_of(tline));
      inc_charpos(tline);
    }

    inc_linepos(tlpos);
  }

  //draw cursor if visible
  if(lineFrom <= cursor_y  && cursor_y <= lineTo &&
     !(m_currMode & CURSORINVISIBLE)) {
    int c_x = cursor_x;
    int c_y = cursor_y;
    InvertArea(dc, c_x*m_charWidth, c_y*m_charHeight, m_charWidth, m_charHeight);
  }

  MarkSelection(dc,FALSE);
  
}

// gets the click coordinate (unscrolled) in terms of characters
void wxTerminal::GetClickCoords(wxMouseEvent& event, int *click_x, int *click_y) {
  // pixel coordinates
  *click_x = event.GetX();
  *click_y = event.GetY();
  CalcUnscrolledPosition(*click_x, *click_y, click_x, click_y);

  // convert to character coordinates
  *click_x = *click_x / m_charWidth;
  *click_y = *click_y / m_charHeight;
  if(*click_x < 0) {
    *click_x = 0;
  }
  else if(*click_x > x_max) {
    *click_x = x_max;
  }

  if(*click_y < 0) { 
    *click_y = 0; 
  }
  else if(*click_y > y_max) {
    *click_y = y_max;
  }
}

void
wxTerminal::OnLeftDown(wxMouseEvent& event)
{
  m_selecting = TRUE;
  int click_x, click_y;
  GetClickCoords(event, &click_x, &click_y);

  m_selx1 = m_selx2 = click_x;
  m_sely1 = m_sely2 = click_y;
  //Refresh();

  event.Skip();  //let native handler reset focus
}

void
wxTerminal::OnLeftUp(wxMouseEvent& event)
{
  
  if ((m_sely2 != m_sely1 || m_selx2 != m_selx1))
    {
      if (wxTheClipboard->Open() ) {
	// This data objects are held by the clipboard, 
	// so do not delete them in the app.
	wxTheClipboard->SetData( new wxTextDataObject(GetSelection()) );
	wxTheClipboard->Close();
      }
    }
  m_selecting = FALSE;

  if (HasCapture()) {
    // uncapture mouse
    ReleaseMouse();
  }
}

void
wxTerminal::OnMouseMove(wxMouseEvent& event)
{
  if(m_selecting)
  {

    int click_x, click_y;
    GetClickCoords(event, &click_x, &click_y);
    m_selx2 = click_x;
    m_sely2 = click_y;
          
    if(!HasCapture()) {
      CaptureMouse();
    }      
  }

  if(!(m_currMode & DEFERUPDATE)) {
    Refresh();
  }

}

void
wxTerminal::ClearSelection()
{
  if (m_sely2 != m_sely1 || m_selx2 != m_selx1) {

    m_sely2 = m_sely1;
    m_selx2 = m_selx1;   

  }

  if(!(m_currMode & DEFERUPDATE)) {
    Refresh();
  }

}

void 
wxTerminal::InvertArea(wxDC &dc, int t_x, int t_y, int w, int h, bool scrolled_coord) {

  if(scrolled_coord) {
    CalcScrolledPosition(t_x,t_y,&t_x,&t_y);
    //calculate if out of bounds
    //    if(t_x < 0 || t_x > m_width * m_charWidth ||
    //   t_y < 0 || t_y > m_height * m_charHeight) {
    //  return;
    //}
  }
  dc.Blit( t_x, t_y, w, h, &dc, t_x, t_y, wxINVERT);
}


void
wxTerminal::MarkSelection(wxDC &dc, bool scrolled_coord) {

  int 
    pic_x1, pic_y1,
    pic_x2, pic_y2;

  if(m_sely1 > m_sely2 ||
     (m_sely1 == m_sely2 && m_selx1 > m_selx2)) {
    pic_x1 = m_selx2;
    pic_y1 = m_sely2;
    pic_x2 = m_selx1;
    pic_y2 = m_sely1;
  }
  else {
    pic_x1 = m_selx1;
    pic_y1 = m_sely1;
    pic_x2 = m_selx2;
    pic_y2 = m_sely2;
  }

  if(pic_y1 == pic_y2) {
    InvertArea(dc, 
	       pic_x1 * m_charWidth, pic_y1 * m_charHeight, 
	       (pic_x2 - pic_x1)*m_charWidth, m_charHeight,
	       scrolled_coord);
  }
  else if(pic_y1 < pic_y2) {
    InvertArea(dc, 
	       pic_x1 * m_charWidth, pic_y1 * m_charHeight, 
	       (x_max - pic_x1) * m_charWidth, m_charHeight,
	       scrolled_coord);
    InvertArea(dc, 
	       0, (pic_y1 + 1)*m_charHeight,
	       x_max * m_charWidth, (pic_y2 - pic_y1 - 1)*m_charHeight,
	       scrolled_coord);
    InvertArea(dc, 
	       0, pic_y2*m_charHeight, 
	       pic_x2*m_charWidth, m_charHeight,
	       scrolled_coord);
  }
}

bool
wxTerminal::HasSelection()
{
  return(m_selx1 != m_selx2 || m_sely1 != m_sely2);
}


/*
 * Gets characters from x1,y1 up to , but not including x2,y2
 */
wxString
wxTerminal::GetChars(int x1, int y1, int x2, int y2) 
{
  int tx,ty;
  if(y1 > y2 || 
     (y1 == y2 && x1 > x2)) {
    tx = x1;
    ty = y1;
    x1 = x2;
    y1 = y2;
    x2 = tx;
    y2 = ty;
  }

  //this case from dragging the mouse position off screen.
  if(x1 < 0) x1 = 0;
  if(x2 < 0) x2 = 0;
  if(y1 < 0) y1 = 0;
  if(y2 < 0) y2 = 0;

  wxString ret;

  if(0 < y1 && y1 > y_max) {
    return ret;
  }

  wxterm_charpos a = GetCharPosition(0,y1);

  a.offset = a.offset + min(x1, max(0,a.line_length - 1)); 
  adjust_charpos(a);

  wxterm_charpos b = GetCharPosition(0,min(y2, y_max));
  
  b.offset = b.offset + min(x2, b.line_length); 
  adjust_charpos(b);

  while(a.buf != b.buf) {
    //  size 10, offset 5,  copy 10-5=5 chars... yup
    ret.Append((wxChar*)a.buf->cbuf+a.offset, WXTERM_CB_SIZE-a.offset);
    if(a.buf->next) {
      a.offset = 0;
      a.buf = a.buf->next;
    }
    else {
      //bad
      fprintf(stderr, "BAD (getchars)\n");
    }
  }
  ret.Append((wxChar*)a.buf->cbuf+a.offset, b.offset - a.offset);
  return ret;
}

wxString
wxTerminal::GetSelection()
{
  return GetChars(m_selx1,m_sely1,m_selx2,m_sely2);
}

void
wxTerminal::SelectAll()
{
  m_selx1 = 0;
  m_sely1 = 0;
  m_selx2 = x_max;
  m_sely2 = y_max;

  //Refresh();
}

wxterm_linepos wxTerminal::GetLinePosition(int y) 
{
  wxterm_linepos lpos;
  lpos.buf = term_lines;
  lpos.offset = y;
  adjust_linepos(lpos);
  return lpos;
}

wxterm_charpos wxTerminal::GetCharPosition(int x, int y)
{ 
  wxterm_charpos ret = line_of(GetLinePosition(y));

  if(x > ret.line_length) {    
    fprintf(stderr, "invalid longer than linelength: %d\n", ret.line_length);
  }

  
  ret.offset = ret.offset + x;
  adjust_charpos(ret);
  return ret;  
}

void
wxTerminal::DrawText(wxDC& dc, int fg_color, int bg_color, int flags,
                 int x, int y, int len, char *string)
{
  wxString
    str((const char *)string, wxConvUTF8, len);

    if(flags & BOLD)
    {
      if(flags & UNDERLINE)
        dc.SetFont(m_boldUnderlinedFont);
      else
        dc.SetFont(m_boldFont);
    }
    else
    {
      if(flags & UNDERLINE)
        dc.SetFont(m_underlinedFont);
      else
        dc.SetFont(m_normalFont);
    }

  int coord_x, coord_y;
  dc.SetBackgroundMode(wxSOLID);
  dc.SetTextBackground(TurtleCanvas::colors[bg_color]);
  dc.SetTextForeground(TurtleCanvas::colors[fg_color]);
  coord_y = y * m_charHeight; 
  coord_x = x * (m_charWidth);	

  for(unsigned int i = 0; i < str.Length(); i++, coord_x+=m_charWidth){
    //clear the pixels first
    //dc.Blit(coord_x, coord_y, m_charWidth, m_charHeight, &dc, coord_x, coord_y, wxCLEAR);
    dc.DrawText(str.Mid(i, 1), coord_x, coord_y);
    //	  if(flags & BOLD && m_boldStyle == OVERSTRIKE)
    //	  dc.DrawText(str, x + 1, y);
    if(flags & INVERSE) {
      InvertArea(dc, coord_x, coord_y, m_charWidth, m_charHeight);
      //	    dc.Blit( coord_x, coord_y, m_charWidth, m_charHeight, &dc, coord_x, coord_y, wxINVERT);
    }
  }
}

void
wxTerminal::Bell()
{
  wxBell();
}

void 
wxTerminal::RenumberLines(int new_width) 
{
  //m_width is the OLD WIDTH at this point of the code.
  
  wxterm_linepos l_pos;
  l_pos.buf = term_lines;
  l_pos.offset = 0;

  wxterm_charpos c_pos;
  c_pos.buf = term_chars;
  c_pos.line_length = 0;
  c_pos.offset = 0; 
  wxterm_charpos last_logo_pos = GetCharPosition(last_logo_x,last_logo_y);

  //IMPORTANT: 
  //  * line_number stores the current line we're looking at,
  //  * c_pos.line_length is how far into the current line we are
  //  * c_pos.offset is the offset into the BUFFER

  int line_number = 0;
  
  //run until see '\0'
  int next_line = 0; // flag to say "mark next line".
  
  while(char_of(c_pos) != '\0') {
    if(c_pos.buf == curr_char_pos.buf && c_pos.offset == curr_char_pos.offset) {
      cursor_x = c_pos.line_length;
      cursor_y = line_number;
      curr_line_pos = l_pos;
    }
    if(c_pos.buf == last_logo_pos.buf && c_pos.offset == last_logo_pos.offset) {
      last_logo_x = c_pos.line_length;
      last_logo_y = line_number;      
    }
    if(char_of(c_pos) == '\n') {
      next_line = 1;
    }
    else {
      c_pos.line_length++;
      if(c_pos.line_length == new_width) {
	next_line = 1;
      }
    }

    inc_charpos(c_pos);

    if(next_line) {
      line_of(l_pos).line_length = c_pos.line_length;
      inc_linepos(l_pos);

      line_of(l_pos).buf = c_pos.buf;
      line_of(l_pos).offset = c_pos.offset;
      
      c_pos.line_length = 0;
      next_line = 0;
      line_number++;
    }



  }
  if(c_pos.buf == curr_char_pos.buf && c_pos.offset == curr_char_pos.offset) {
    cursor_x = c_pos.line_length;
    cursor_y = line_number;
    curr_line_pos = l_pos;
  }
  if(c_pos.buf == last_logo_pos.buf && c_pos.offset == last_logo_pos.offset) {
    last_logo_x = c_pos.line_length;
    last_logo_y = line_number;      
  }

  line_of(l_pos).line_length = c_pos.line_length;
  y_max = line_number;

  //sanity check on variables
  //fprintf(stderr, "cursor %d %d\n", cursor_x, cursor_y);
  //fprintf(stderr, "lastlogopos xy %d %d\n", last_logo_x, last_logo_y);
}

void
wxTerminal::ResizeTerminal(int width, int height)
{

  /*
  **  Set terminal size
  */
  if(width != m_width) {
    //need to renumber the lines
    RenumberLines(width);
    m_width = width;
    x_max = m_width - 1;
  }
  m_height = height;


  //reset virtual size
  SetScrollRate(0, m_charHeight);
  //number of lines is y_max + 1
  SetVirtualSize(m_width*m_charWidth,(y_max+1)*m_charHeight);
}


void wxTerminal::DebugOutputBuffer() {
  //debugging
  wxterm_linepos lpos;
  lpos.buf = term_lines;
  lpos.offset = 0;
  wxterm_charpos pos_1 = line_of(lpos);
  
    fprintf(stderr, "WXTERMINAL STATS: \n  width: %d, height: %d, \n cw: %d, ch: %d \n x_max: %d, y_max: %d \n cursor_x: %d, cursor_y: %d \n last_logo_x : %d, last_logo_y: %d \ncurr_charpos buf %d offset %d  \ncurr_line buf %d offset %d\n", m_width, m_height, m_charWidth, m_charHeight, x_max, y_max,cursor_x, cursor_y, last_logo_x, last_logo_y,(int)curr_char_pos.buf, curr_char_pos.offset, (int)curr_line_pos.buf, curr_line_pos.offset);
    fprintf(stderr, "WXTERMINAL CHARACTER BUFFER\n###############\n");
  while(char_of(pos_1) != '\0') {
    if(char_of(pos_1) == '\n') {
      fprintf(stderr, "\\n\n");
    }
    else {
      fprintf(stderr,"%c", char_of(pos_1));
      
    }
    inc_charpos(pos_1);
  }
  fprintf(stderr, "|");
    fprintf(stderr, "\n#############\n");
    fprintf(stderr, "WXTERMINAL LINE BUFFER\n##############\n");
  for(int i = 0; i <= y_max; i++) {
    fprintf(stderr, "LINE %d: buf: %d, offset: %d, len: %d\n", i,(int)line_of(lpos).buf, line_of(lpos).offset, line_of(lpos).line_length);
    inc_linepos(lpos);
  }
    fprintf(stderr, "\n#############\n\n");
}

void wxTerminal::InsertChar(char c)
{
  
  //insert a character at current location

  //if there is a newline at the current location and we're not inserting one
  // scenario:
  // Buffer: abcde\nfgh
  //         *    X *
  // X = current position
  // * = line locations
  // 
  // After insert k:
  //         abcdek\nfgh
  //         *     X *

  if(char_of(curr_char_pos) == '\n' &&
     c != '\n') {
    // check case if we're about to insert the last character of the line
    if(line_of(curr_line_pos).line_length < m_width - 1) {       

      //have to add characters to current line, 

      //bump characters up.      
      wxterm_linepos lpos = curr_line_pos;
      wxterm_charpos pos_1 = curr_char_pos;
      wxterm_charpos pos_2 = pos_1;

      inc_charpos(pos_2);


      //  Method of relocating characters:
      //   need two variables, S = save, G = grab
      // 
      //   save 1, 
      //   S1  G      
      //  12345678
      //
      //   grab 2, place 1, save 2
      //  
      //  S2 G2
      //  _1345678
      //
      //  grab 3, place 2, save 3...
      //
      //  S3 G3
      //  _1245678
      // 
      //  etc...
      // ends when next position to grab is '\0'
      // (allocate buffer when necesary)
      // grab 8, place 7, save 8
      // place 8 in that last position

      char save_char, save_mode; 
      char grab_char, grab_mode;       
      save_char = char_of(pos_1);
      save_mode = mode_of(pos_1);

      while(char_of(pos_2) != '\0') {

	grab_char = char_of(pos_2);
	grab_mode = mode_of(pos_2);
	char_of(pos_2) = save_char;
	mode_of(pos_2) = save_mode;
	save_char = grab_char;
	save_mode = grab_mode;

	inc_charpos(pos_1);
	inc_charpos(pos_2);	
      }
      //insert "save" into pos2
      char_of(pos_2) = save_char;
      mode_of(pos_2) = save_mode;
      
      //now bump all the lines up.
      //look at the current line number (cursor_y)
      // go until  y_max, and adjust position of all lines +1
      inc_linepos(lpos);
      for(int lnum = cursor_y+1; lnum <= y_max; lnum++) {
	inc_charpos(line_of(lpos));
	inc_linepos(lpos);      
      }
    }
  }
  char_of(curr_char_pos) = c;
  mode_of(curr_char_pos) = m_currMode;
  inc_charpos(curr_char_pos);
}

void wxTerminal::NextLine() {
  //points next line position to current char position, and increments line offset.

  inc_linepos(curr_line_pos);

  line_of(curr_line_pos).buf = curr_char_pos.buf;
  line_of(curr_line_pos).offset = curr_char_pos.offset;
  //line_of(curr_line_pos).line_length = 0;


  cursor_y++;
  if(cursor_y > y_max) {
    y_max = cursor_y;
    int x,y;
    GetVirtualSize(&x,&y);

    //y_max + 1 = number of lines    
    SetVirtualSize(max(x,m_width*m_charWidth),max(y,(y_max+1)*m_charHeight)); 
  }
  cursor_x = 0;
}

void
wxTerminal::PassInputToTerminal(int len, char *data)
{
  int i;
  int numspaces, j;
  wxterm_linepos lpos;
  wxterm_charpos pos_1, pos_2;
  int new_line_length;

  int vis_x,vis_y;
  GetViewStart(&vis_x,&vis_y);
  static int old_vis_y = 0;

  char spc = ' ';


  for(i = 0; i < len; i++) {
    switch(data[i]) {
    case TOGGLE_STANDOUT:  // char 17
      //enter/exit standout mode , ignore character
      m_currMode ^= INVERSE;
      break;
    case '\t':
      // formula is: (8 - (cursorpos%8)) spaces
      numspaces = (8 - (cursor_x % 8));
      if(numspaces == 0) {
	numspaces = 8;
      }
      for(j = 0; j < numspaces; j++) {
	InsertChar(spc);


	cursor_x++;
	if(cursor_x > line_of(curr_line_pos).line_length) {
	  line_of(curr_line_pos).line_length = cursor_x;
	}
	if(cursor_x > x_max) {
	  //tab should stop inserting spaces.	   
	  NextLine();
	  break; //out of the for loop
	}
      }
      break;
    case '\r':
    case '\n':
      new_line_length = cursor_x;
      InsertChar('\n');

      
      if(i + 1 < len && 
	 ((data[i] == '\r' && data[i+1] == '\n') ||  //LF+CR
	  (data[i] == '\n' && data[i+1] == '\r')))  { //CR+LF
	i++;
      }

      // this case triggers when you do
      // ? setcursor [0 0]
      // and then press <ENTER>
      // if this is commented out, it will still work, but the characters
      // will remain in the buffer!

      if(new_line_length < line_of(curr_line_pos).line_length &&
	 cursor_y < y_max) {
	//shift all the characters down.
	//(remove characters between inserted newline and end of line)
	
	lpos = curr_line_pos;
	inc_linepos(lpos);
	pos_1 = curr_char_pos;
	pos_2 = line_of(lpos);
	while(char_of(pos_1) != '\0') {
	  if(char_of(pos_2) != '\0' &&
	     pos_2.buf == line_of(lpos).buf &&
	     pos_2.offset == line_of(lpos).offset) {
	    line_of(lpos).buf = pos_1.buf;
	    line_of(lpos).offset = pos_1.offset;
	    inc_linepos(lpos);
	  }
	  char_of(pos_1) = char_of(pos_2);
	  mode_of(pos_1) = mode_of(pos_2);
	  inc_charpos(pos_1);
	  inc_charpos(pos_2);
	}
      }

      line_of(curr_line_pos).line_length = new_line_length;
      
      NextLine();
      
      break;
    default:
      InsertChar(data[i]);


      cursor_x++;
      if(cursor_x > line_of(curr_line_pos).line_length) {
	line_of(curr_line_pos).line_length = cursor_x;
      }
      if(cursor_x > x_max) {
	NextLine();
      }      
      break;
    }   
  }


  if(!(m_currMode & DEFERUPDATE)) {
    //scroll if cursor current cursor not visible or 
    // if we're not reading an instruction (logo output)
    // old_vis_y keeps track of whether the screen has changed lately
    
    if((!readingInstruction &&
	1 ||                     //don't use old_vis_y??
	vis_y != old_vis_y) || 
       cursor_y < vis_y  ||
       cursor_y > vis_y + m_height - 1) {

      Scroll(-1, cursor_y);
      old_vis_y = vis_y;
      

      //Refresh();
    }  
    
  }
}

wxString * wxTerminal::get_text() 
{
  //  int i;
  wxString *outputString = new wxString();
  outputString->Clear();
  outputString->Append(_T("<HTML>\n"));
  outputString->Append(_T("<BODY>\n"));
  outputString->Append(_T("<FONT SIZE=2>\n"));
  wxString txt = GetChars(0,0,x_max,y_max);
  txt.Replace(_T("\n"),_T("<BR>\n"));
  outputString->Append(txt);
  /*
  wxterm_linepos tlpos = term_lines;
  for(i=0;i<ymax;i++){
    outputString->append(textString->Mid(linenumbers[i]*MAXWIDTH),MAXWIDTH);
    outputString->append(_T("<BR>"));		
    }*/
  outputString->Append(_T("<\\FONT>"));
  outputString->Append(_T("<\\BODY>"));
  outputString->Append(_T("<\\HTML>"));
  //  delete textString;
  return outputString;
}


void
wxTerminal::SelectPrinter(char *PrinterName)
{
  if(m_printerFN)
  {
    if(m_printerName[0] == '#')
      fclose(m_printerFN);
    else
#if defined(__WXMSW__)
      fclose(m_printerFN);
#else
      pclose(m_printerFN);
#endif

    m_printerFN = 0;
  }

  if(m_printerName)
  {
    free(m_printerName);
    m_printerName = 0;
  }

  if(strlen(PrinterName))
  {
    m_printerName = strdup(PrinterName);
  }
}

void
wxTerminal::PrintChars(int len, char *data)
{
  char
    pname[100];

  if(!m_printerFN)
  {
    if(!m_printerName)
      return;

    if(m_printerName[0] == '#')
    {
#if defined(__WXMSW__)
      sprintf(pname, "lpt%d", m_printerName[1] - '0' + 1);
#else
      sprintf(pname, "/dev/lp%d", m_printerName[1] - '0');
#endif
      m_printerFN = fopen(pname, "wb");
    }
    else
    {
#if defined(__WXMSW__)
      m_printerFN = fopen(m_printerName, "wb");
#else
      sprintf(pname, "lpr -P%s", m_printerName);
      m_printerFN = popen(pname, "w");
#endif
    }
  }
  
  if(m_printerFN)
  {
    fwrite(data, len, 1, m_printerFN);
  }
}



// ----------------------------------------------------------------------------
// Functions called from the interpreter thread
// ----------------------------------------------------------------------------

extern "C" void setCharMode(int mode){
	logo_char_mode = mode;
	
	//if turning charmode off, flush the
	//buffer (not the input buffer, logo's buffer)

	if(!logo_char_mode) {
          char tmp;
	  while(!buff_empty) {
	    buff_pop(tmp);
	  }
        }
}

extern "C" void wxClearText() {
  wxTerminal::terminal->ClearScreen();
  //  output_index = 0;
}

void wxTerminal::ClearScreen() {
  if(y_max > 0) {
    int x,y;
    GetVirtualSize(&x,&y);
    SetVirtualSize(max(x,m_width*m_charWidth),max(y,(y_max+1+m_height)*m_charHeight));
    Scroll(-1,y_max+1);
    int vx,vy;
    GetViewStart(&vx,&vy);
    //pretend setcursor from logo so that it can insert spaces if needed
    wxClientDC dc(this);
    setCursor(0,y_max + 1 - vy, TRUE); 
    if(!(m_currMode & DEFERUPDATE)) {
      Refresh();
    }
  }
}

//currently does nothing.
void wxTerminal::EnableScrolling(bool want_scrolling) {
  //wxScrolledWindow::EnableScrolling(FALSE,want_scrolling); //doesn't work
  return;
}

extern "C" void flushFile(FILE * stream);

extern "C" void wxSetCursor(int x, int y){
  //just call wxTerminal::setCursor with a special flag.
  flushFile(stdout);
  wxTerminal::terminal->EnableScrolling(FALSE);
  wxClientDC dc(wxTerminal::terminal);
  wxTerminal::terminal->setCursor(x,y,TRUE);
}

extern "C" void wxSetFont(char *fm, int sz) {

    int adjust_label = 0;
    if(fm != wx_font_face) {
        strcpy(wx_font_face, fm);
        adjust_label++;
    }

    wx_font_size = sz;

    wxFont f(FONT_CFG(wx_font_face, wx_font_size));

    wxTerminal::terminal->SetFont(f);
    editWindow->SetFont(f);

    //TurtleCanvas memDC, set size depending on scrunch!
    if(adjust_label) {
        turtleGraphics->SetFont(f);
    }

    logoFrame->AdjustSize();
}

extern "C" void wx_enable_scrolling() {
  wxTerminal::terminal->EnableScrolling(TRUE);
}

extern enum s_md {SCREEN_TEXT, SCREEN_SPLIT, SCREEN_FULL} screen_mode;


extern "C" int check_wx_stop(int force_yield) {
  logoEventManager->ProcessEvents(force_yield); 

  //give focus to terminal if text window shown
  //and Frame has focus
  wxWindow * focus_win = wxWindow::FindFocus();
  if(wxTerminal::terminal &&
     //screen_mode != SCREEN_FULL &&  // screen_text or screen_split mode
     !(focus_win == (wxWindow *)wxTerminal::terminal) &&
     (focus_win == (wxWindow *)logoFrame)) {
    wxTerminal::terminal->SetFocus();
  }

  if (logo_stop_flag) {
    logo_stop_flag = 0;
#ifdef SIG_TAKES_ARG
    logo_stop(0);
#else
    logo_stop();
#endif
    return 1;
  }
  if (logo_pause_flag) {
    logo_pause_flag = 0;
#ifdef SIG_TAKES_ARG
    logo_pause(0);
#else
    logo_pause();
#endif
    return 0;
  }
  return 0;
}

extern "C" int internal_check(){
 if (logo_stop_flag) {
    logo_stop_flag = 0;
#ifdef SIG_TAKES_ARG
    logo_stop(0);
#else
    logo_stop();
#endif
    return 1;
  }
  if (logo_pause_flag) {
    logo_pause_flag = 0;
#ifdef SIG_TAKES_ARG
    logo_pause(0);
#else
    logo_pause();
#endif
    return 1;
  }
  return 0;
}

extern "C" int getTermInfo(int type){
	switch (type){
	case X_COORD:
		return wxTerminal::terminal->cursor_x;
		break;
	case Y_COORD:
	  int vx,vy;
	  wxTerminal::terminal->GetViewStart(&vx,&vy);
		return wxTerminal::terminal->cursor_y - vy;
		break;
	case X_MAX:
	  //return wxTerminal::terminal->x_max;
	  return wxTerminal::terminal->m_width;
		break;
	case Y_MAX:
	  //return wxTerminal::terminal->y_max;
	  return wxTerminal::terminal->m_height;
		break;
	case EDIT_STATE:
		return editWindow->stateFlag;
		break;
	default:
		return -1;
	}
	
	return -1;
}

extern "C" void setTermInfo(int type, int val){
	switch (type){
		case X_COORD:
		  //wxTerminal::terminal->x_coord=val;
		  return;
			break;
		case Y_COORD:
		  //wxTerminal::terminal->y_coord=val;
		  return;
			break;
		case X_MAX:
		  return;
		  //wxTerminal::terminal->x_max=val;
			break;
		case Y_MAX:
		  return;
		  //wxTerminal::terminal->y_max=val;
			break;
		case EDIT_STATE:
			editWindow->stateFlag=val;
			break;
	}
}


extern "C" void doClose() {
  extern int wx_leave_mainloop;

  if(!wx_leave_mainloop) {
    logoFrame->Close(TRUE);
  }
}


