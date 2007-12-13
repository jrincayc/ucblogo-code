
/* 
	Much of the code for the terminal came from:
        taTelnet - A cross-platform telnet program.
        Copyright (c) 2000 Derry Bryson. 
  
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,Ä
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Contact Information:

       Technology Associates, Inc.
       Attn:  Derry Bryson
       959 W. 5th Street
       Reno, NV  89503
       USA

       derry@techass.com
*/

/* This file implements the logo frame, which is the main frame that
   contains the terminal, turtle graphics and the editor.
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
#include <wx/stdpaths.h>

#include <ctype.h>

extern unsigned char *cmdHistory[];
extern unsigned char **hist_inptr, **hist_outptr;
extern int readingInstruction;

#include <wx/print.h>
#include "LogoFrame.h"
#include "wxGlobals.h"
#include <wx/clipbrd.h>
#include <wx/html/htmprint.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include "wxTurtleGraphics.h"
#include "config.h"
#include "TextEditor.h"
#include "gterm.hpp"		/* must come after wxTurtleGraphics.h */
#include "wxTerminal.h"		/* must come after wxTurtleGraphics.h */
#ifdef __WXMAC__                                                        
#include <Carbon/Carbon.h>                                              
#endif                                                                  

using namespace std;

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------
// This is for the input for terminal-like behavior
unsigned char inputBuffer [8000];
// How far into the inputBuffer we are
int input_index = 0;

// if logo is in character mode
int logo_char_mode;
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

// used to calculate where the cursor should be
int cur_x = 0, cur_y = 0;
int first = 1;
int last_logo_x = 0, last_logo_y = 0;
int last_user_x = 0, last_user_y = 0;
bool cursor_moved = 0;
// the menu
wxMenuBar* menuBar;

extern "C" void wxTextScreen();

char *argv[2] = {"UCBLogo", 0};

// This is for stopping logo asynchronously
#ifdef SIG_TAKES_ARG
extern "C" RETSIGTYPE logo_stop(int);
extern "C" RETSIGTYPE logo_pause(int);
#else
extern "C" RETSIGTYPE logo_stop();
extern "C" RETSIGTYPE logo_pause();
#endif
extern void wxLogoWakeup();
int logo_stop_flag = 0;
int logo_pause_flag = 0;
int movedCursor = 0;

// this is a static reference to the main terminal
wxTerminal *wxTerminal::terminal;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

enum
{
	
    Menu_File = 200,
	Menu_File_Save,
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

  LogoFrame  * logoFrame = new LogoFrame
    ("Berkeley Logo",
     50, 50, 900, 500);

  logoFrame->Show(TRUE);
  SetTopWindow(logoFrame);
  return TRUE;	
}


// ----------------------------------------------------------------------------
// LogoFrame class
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (LogoFrame, wxFrame)
EVT_MENU(Menu_File_Save,				LogoFrame::OnSave)
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
END_EVENT_TABLE()

#include "ucblogo.xpm"

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
  wxTerminal::terminal = new wxTerminal (this, -1, wxPoint(-1, -1), 82, 25,  wxString(""));
  turtleGraphics = new TurtleCanvas( this );
  wxFont f(18, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
	   false, "Courier");
  editWindow = new TextEditor( this, -1, "", wxDefaultPosition, wxSize(100,60), wxTE_MULTILINE, f);
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
	
	SetAutoLayout(true);
	//topsizer->Fit(this);
	//topsizer->SetSizeHints(this);
	
    wxTerminal::terminal->SetFocus();
	SetUpMenu();
    wxSleep(1);
	
    init_Logo_Interpreter (1, argv );
}

void LogoFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(TRUE);
}

void LogoFrame::SetUpMenu(){
	int i;
	if(!menuBar)
		menuBar = new wxMenuBar( wxMB_DOCKABLE );
	else
		for(i=menuBar->GetMenuCount()-1;i>=0;i--)
			delete menuBar->Remove(i);

	
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append( Menu_File_Save, _T("Save Logo Session \tCtrl-S"));
	fileMenu->Append( Menu_File_Load, _T("Load Logo Session \tCtrl-O"));
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
#ifdef __WXMSW__
	editMenu->Append(Menu_Edit_Copy, _T("Copy \tCtrl-C"));
	editMenu->Append(Menu_Edit_Paste, _T("Paste \tCtrl-V"));

	logoMenu->Append(Menu_Logo_Pause, _T("Pause \tCtrl-P"));
	logoMenu->Append(Menu_Logo_Stop, _T("Stop \tCtrl-S"));	
#else
	editMenu->Append(Menu_Edit_Copy, _T("Copy \tCtrl-C"));
	editMenu->Append(Menu_Edit_Paste, _T("Paste \tCtrl-V"));

	logoMenu->Append(Menu_Logo_Pause, _T("Pause \tAlt-P"));
	logoMenu->Append(Menu_Logo_Stop, _T("Stop \tAlt-S"));
#endif
	menuBar->Append(logoMenu, _T("&Logo"));
	
	wxMenu *fontMenu = new wxMenu;
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

void doSave(char * name, int length);
void doLoad(char * name, int length);
void LogoFrame::OnSave(wxCommandEvent& WXUNUSED(event)){
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
//			    "Logo workspaces(*.lg)|All files(*.*)",
			    "*.*",
#ifdef __WXMAC__   /* needed for wxWidgets 2.6 */
			    wxSAVE|wxOVERWRITE_PROMPT|wxCHANGE_DIR);
#else
			    wxFD_SAVE|wxFD_OVERWRITE_PROMPT|wxFD_CHANGE_DIR);
#endif
	
	dialog.SetFilterIndex(1);
	
	if (dialog.ShowModal() == wxID_OK)
	{
	    doSave((char *)dialog.GetPath().c_str(),
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
//	 "Logo workspaces(*.lg)|All files(*.*)",
	 "*",
#ifdef __WXMAC__   /* needed for wxWidgets 2.6 */
	 wxOPEN|wxFILE_MUST_EXIST|wxCHANGE_DIR);
#else
	 wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR);
#endif
		
	if (dialog.ShowModal() == wxID_OK) {
	    doLoad((char *)dialog.GetPath().c_str(),
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
		htmlPrinter->SetFonts("Courier","Courier", fontsizes);
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
		htmlPrinter->SetFonts("Courier","Courier", fontsizes);
	}
	wxString *textString = wxTerminal::terminal->get_text();
	
	htmlPrinter->PreviewText(*textString,wxString(""));
	
}

void LogoFrame::OnIncreaseFont(wxCommandEvent& WXUNUSED(event)){
	int expected;
	
	// get original size and number of characters per row and column
	int width, height, numCharX, numCharY, m_charWidth, m_charHeight;
	int m_lineHeight;
	wxClientDC dc(wxTerminal::terminal);
	dc.GetMultiLineTextExtent("M", &m_charWidth, &m_charHeight,
				  &m_lineHeight);	
	GetSize(&width, &height);
	numCharX = width/m_charWidth;
	numCharY = height/m_lineHeight;
	
	wxdprintf("m_charWidth: %d, m_charHeight: %d, width: %d, height: %d, numCharX: %d, numCharY: %d\n", m_charWidth, m_charHeight, width, height, numCharX, numCharY);
	
	wxFont font = wxTerminal::terminal->GetFont();
	expected = font.GetPointSize()+1;
	
	// see that we have the font we are trying to use
	while(font.GetPointSize() != expected && expected <= 24){
		expected++;
		font.SetPointSize(expected);
	}
	wxTerminal::terminal->SetFont(font);
	editWindow->SetFont(font);
	wxSizeEvent event;
	if(wxTerminal::terminal->IsShown())
		wxTerminal::terminal->OnSize(event);
	
	// resize the frame according to the new font size
	int new_m_charWidth, new_m_charHeight, new_m_lineHeight;
	wxClientDC newdc(wxTerminal::terminal);
	newdc.GetMultiLineTextExtent("M", &new_m_charWidth,
				     &new_m_charHeight, &new_m_lineHeight);
	if (new_m_charWidth != m_charWidth ||
		    new_m_lineHeight != m_lineHeight) {
	    SetSize(numCharX*new_m_charWidth, numCharY*new_m_lineHeight);
	}
	//GetSize(&width, &height); 
	//printf("new m_charWidth: %d, new m_charHeight: %d, new width: %d, new height: %d\n", new_m_charWidth, new_m_charHeight, width, height);
	Layout();
}

void LogoFrame::OnDecreaseFont(wxCommandEvent& WXUNUSED(event)){
	int expected;
	
	// get original size and number of characters per row and column
	int width, height, numCharX, numCharY, m_charWidth, m_charHeight;
	wxClientDC dc(wxTerminal::terminal);
	dc.GetTextExtent("M", &m_charWidth, &m_charHeight);	
	GetSize(&width, &height);
	numCharX = width/m_charWidth;
	numCharY = height/m_charHeight;
	//printf("m_charWidth: %d, m_charHeight: %d, width: %d, height: %d, numCharX: %d, numCharY: %d\n", m_charWidth, m_charHeight, width, height, numCharX, numCharY);
	
	wxFont font = wxTerminal::terminal->GetFont();
	expected = font.GetPointSize()-1;
	
	// see that we have the font we are trying to use
	while(font.GetPointSize() != expected && expected >= 6){
		expected--;
		font.SetPointSize(expected);
	}	
	wxTerminal::terminal->SetFont(font);
	editWindow->SetFont(font);
	wxSizeEvent event;
	wxTerminal::terminal->OnSize(event);
	
	// resize the frame according to the new font size
	int new_m_charWidth, new_m_charHeight;
	wxClientDC newdc(wxTerminal::terminal);
	newdc.GetTextExtent("M", &new_m_charWidth, &new_m_charHeight);
	if (new_m_charWidth != m_charWidth || new_m_charHeight != m_charHeight) {
		SetSize(numCharX*new_m_charWidth, numCharY*new_m_charHeight);	
		wxdprintf("resized window?");
	}
	//GetSize(&width, &height); 
	//printf("new m_charWidth: %d, new m_charHeight: %d, new width: %d, new height: %d\n", new_m_charWidth, new_m_charHeight, width, height);
	Layout();
	
}

void LogoFrame::DoStop(wxCommandEvent& WXUNUSED(event)){
  logo_stop_flag = 1;
  wxLogoWakeup();
}


void LogoFrame::DoPause(wxCommandEvent& WXUNUSED(event)){
  logo_pause_flag = 1;
	wxLogoWakeup();
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
	fileMenu->Append( Edit_Menu_File_Close_Accept, _T("Close and Accept Changes \tCtrl-Q"));
	fileMenu->Append( Edit_Menu_File_Close_Reject, _T("Close and Revert Changes \tCtrl-R"));
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
  EVT_PAINT(wxTerminal::OnPaint)
  EVT_CHAR(wxTerminal::OnChar)
  EVT_LEFT_DOWN(wxTerminal::OnLeftDown)
  EVT_LEFT_UP(wxTerminal::OnLeftUp)
  EVT_MOTION(wxTerminal::OnMouseMove)
  EVT_TIMER(-1, wxTerminal::OnTimer)
  EVT_MY_CUSTOM_COMMAND(-1, wxTerminal::printText)
  EVT_SIZE(wxTerminal::OnSize)
  EVT_KILL_FOCUS(wxTerminal::LoseFocus)
#if 0
  EVT_KEY_DOWN(wxTerminal::OnKeyDown)
#endif
END_EVENT_TABLE()

wxCommandEvent * haveInputEvent = new wxCommandEvent(wxEVT_MY_CUSTOM_COMMAND);

 wxTerminal::wxTerminal(wxWindow* parent, wxWindowID id,
               const wxPoint& pos,
               int width, int height,
               const wxString& name) :
//  wxScrolledWindow(parent, id, pos, wxSize(-1, -1), wxWANTS_CHARS|wxVSCROLL, name) ,
    wxScrolledWindow(parent, id, pos, wxSize(-1, -1), wxWANTS_CHARS, name) ,
  GTerm(width, height)
{
  // start us out not in char mode
  logo_char_mode = 0;
  // For printing the text
  htmlPrinter = 0;
  set_mode_flag(DESTRUCTBS);
   m_bitmap = 0;
  wxTerminal::terminal = this;
  int
    i;

  m_init = 1;

  m_selecting = FALSE;
  m_selx1 = m_sely1 = m_selx2 = m_sely2 = 0;
  m_seloldx1 = m_seloldy1= m_seloldx2 = m_seloldy2 = 0;
  m_marking = FALSE;
  m_curX = -1;
  m_curY = -1;
  m_curBlinkRate=0;
  m_timer.SetOwner(this);
  if(m_curBlinkRate)
    m_timer.Start(m_curBlinkRate);

  m_boldStyle = FONT;

  GetDefVTColors(m_vt_colors);
  GetDefPCColors(m_pc_colors);

  m_colors = m_vt_colors;

  SetBackgroundColour(m_colors[0]);
  SetMinSize(wxSize(50, 50));

  for(i = 0; i < 16; i++)
    m_vt_colorPens[i] = wxPen(m_vt_colors[i], 1, wxSOLID);

  for(i = 0; i < 16; i++)
    m_pc_colorPens[i] = wxPen(m_pc_colors[i], 1, wxSOLID);

  m_colorPens = m_vt_colorPens;

  m_width = width;
  m_height = height;

  m_printerFN = 0;
  m_printerName = 0;

  wxFont f(18, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
	   false, "Courier");
  SetFont(f);

  wxClientDC
    dc(this);
  dc.GetTextExtent("M", &m_charWidth, &m_charHeight);
  m_charWidth--;
  
  int x, y;
  GetSize(&x, &y);
//  SetScrollbars(m_charWidth, m_charHeight, 0, 30);
  
  parent->SetSize(-1,-1, m_charWidth * width, m_charHeight * height + 1);

  ResizeTerminal(width, height);

}

wxTerminal::~wxTerminal()
{
  if(m_bitmap)
  {
    m_memDC.SelectObject(wxNullBitmap);
    delete m_bitmap;
  }
}

void wxTerminal::deferUpdate(int flag) {
    if (flag)
	set_mode_flag(DEFERUPDATE);
    else
	clear_mode_flag(DEFERUPDATE);
}

void
wxTerminal::SetBoldStyle(wxTerminal::BOLDSTYLE boldStyle)
{
  /*wxColour
    colors[16];*/

  if(boldStyle == DEFAULT)
    boldStyle = COLOR;

  m_boldStyle = boldStyle;
  Refresh();
}

bool
wxTerminal::SetFont(const wxFont& font)
{
  m_init = 1;
  
  wxWindow::SetFont(font);
  m_normalFont = font;
  m_underlinedFont = font;
  m_underlinedFont.SetUnderlined(TRUE);
  m_boldFont = GetFont();
  m_boldFont.SetWeight(wxBOLD);
  m_boldUnderlinedFont = m_boldFont;
  m_boldUnderlinedFont.SetUnderlined(TRUE);
  m_init = 0;
  wxClientDC
	  dc(this);
  dc.GetTextExtent("M", &m_charWidth, &m_charHeight);
  m_charWidth--;
  ResizeTerminal(m_width, m_height);
  Refresh();
  
  return TRUE;
}

void
wxTerminal::GetDefVTColors(wxColour colors[16], wxTerminal::BOLDSTYLE boldStyle)
{
  if(boldStyle == DEFAULT)
    boldStyle = m_boldStyle;

  if(boldStyle != COLOR)
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
  else
  {
    colors[0] = wxColour(0, 0, 0);                             // black
    colors[1] = wxColour(170, 0, 0);                           // red
    colors[2] = wxColour(0, 170, 0);                           // green
    colors[3] = wxColour(170, 0, 170);                         // yellow
    colors[4] = wxColour(0, 0, 170);                           // blue
    colors[5] = wxColour(170, 170, 0);                         // magenta
    colors[6] = wxColour(0, 170, 170);                         // cyan
    colors[7] = wxColour(192, 192, 192);                       // white
//    colors[7] = wxColour(170, 170, 170);                       // white
#if 0
    colors[8] = wxColour(85, 85, 85);                          // bold black
    colors[9] = wxColour(255, 85, 85);                         // bold red
    colors[10] = wxColour(85, 255, 85);                        // bold green
    colors[11] = wxColour(255, 85, 255);                       // bold yellow
    colors[12] = wxColour(85, 85, 255);                        // bold blue
    colors[13] = wxColour(255, 255, 85);                       // bold magenta
    colors[14] = wxColour(85, 255, 255);                       // bold cyan
    colors[15] = wxColour(255, 255, 255);                      // bold white
#else
    colors[8] = wxColour(85, 85, 85);                          // bold black
    colors[9] = wxColour(255, 0, 0);                         // bold red
    colors[10] = wxColour(0, 255, 0);                        // bold green
    colors[11] = wxColour(255, 0, 255);                       // bold yellow
    colors[12] = wxColour(0, 0, 255);                        // bold blue
    colors[13] = wxColour(255, 255, 0);                       // bold magenta
    colors[14] = wxColour(0, 255, 255);                       // bold cyan
    colors[15] = wxColour(255, 255, 255);                      // bold white
#endif    
  }
}

void
wxTerminal::GetVTColors(wxColour colors[16])
{
  int
    i;

  for(i = 0; i < 16; i++)
    colors[i] = m_vt_colors[i];
}

void
wxTerminal::SetVTColors(wxColour colors[16])
{
  int
    i;

  m_init = 1;
  for(i = 0; i < 16; i++)
    m_vt_colors[i] = colors[i];

  if(!(GetMode() & PC))
    SetBackgroundColour(m_vt_colors[0]);

  for(i = 0; i < 16; i++)
    m_vt_colorPens[i] = wxPen(m_vt_colors[i], 1, wxSOLID);
  m_init = 0;

  Refresh();
}

void
wxTerminal::GetDefPCColors(wxColour colors[16])
{
#if 0
  /*
  **  These colors need tweaking.  I'm sure they are not correct.
  */
  colors[0] = wxColour(0, 0, 0);                             // black
  colors[1] = wxColour(0, 0, 128);                           // blue
  colors[2] = wxColour(0, 128, 0);                           // green
  colors[3] = wxColour(0, 128, 128);                         // cyan
  colors[4] = wxColour(128, 0, 0);                           // red
  colors[5] = wxColour(128, 0, 128);                         // magenta
  colors[6] = wxColour(128, 128, 0);                         // brown
  colors[7] = wxColour(128, 128, 128);                       // white
  colors[8] = wxColour(64, 64, 64);                          // gray
  colors[9] = wxColour(0, 0, 255);                           // lt blue
  colors[10] = wxColour(0, 255, 0);                          // lt green
  colors[11] = wxColour(0, 255, 255);                        // lt cyan
  colors[12] = wxColour(255, 0, 0);                          // lt red
  colors[13] = wxColour(255, 0, 255);                        // lt magenta
  colors[14] = wxColour(255, 255, 0);                        // yellow
  colors[15] = wxColour(255, 255, 255);                      // white
#else
  /*
  **  These are much better
  */
  colors[0] = wxColour(0, 0, 0);                             // black
  colors[1] = wxColour(0, 0, 170);                           // blue
  colors[2] = wxColour(0, 170, 0);                           // green
  colors[3] = wxColour(0, 170, 170);                         // cyan
  colors[4] = wxColour(170, 0, 0);                           // red
  colors[5] = wxColour(170, 0, 170);                         // magenta
  colors[6] = wxColour(170, 170, 0);                         // brown
  colors[7] = wxColour(170, 170, 170);                       // white
#if 0
  colors[8] = wxColour(85, 85, 85);                          // gray
  colors[9] = wxColour(85, 85, 255);                         // lt blue
  colors[10] = wxColour(85, 255, 85);                        // lt green
  colors[11] = wxColour(85, 255, 255);                       // lt cyan
  colors[12] = wxColour(255, 85, 85);                        // lt red
  colors[13] = wxColour(255, 85, 255);                       // lt magenta
  colors[14] = wxColour(255, 255, 85);                       // yellow
  colors[15] = wxColour(255, 255, 255);                      // white
#else
  colors[8] = wxColour(50, 50, 50);                          // gray
  colors[9] = wxColour(0, 0, 255);                         // lt blue
  colors[10] = wxColour(0, 255, 0);                        // lt green
  colors[11] = wxColour(0, 255, 255);                       // lt cyan
  colors[12] = wxColour(255, 0, 0);                        // lt red
  colors[13] = wxColour(255, 0, 255);                       // lt magenta
  colors[14] = wxColour(255, 255, 0);                       // yellow
  colors[15] = wxColour(255, 255, 255);                      // white
#endif  
#endif
}

void
wxTerminal::GetPCColors(wxColour colors[16])
{
  int
    i;

  for(i = 0; i < 16; i++)
    colors[i] = m_pc_colors[i];
}

void
wxTerminal::SetPCColors(wxColour colors[16])
{
  int
    i;

  m_init = 1;
  for(i = 0; i < 16; i++)
    m_pc_colors[i] = colors[i];

  if(GetMode() & PC)
    SetBackgroundColour(m_pc_colors[0]);

  for(i = 0; i < 16; i++)
    m_pc_colorPens[i] = wxPen(m_pc_colors[i], 1, wxSOLID);
  m_init = 0;

  Refresh();
}

void
wxTerminal::SetCursorBlinkRate(int rate)
{
  
  if(rate < 0 || rate > CURSOR_BLINK_MAX_TIMEOUT)
    return;

  m_init = 1;
  if(rate != m_curBlinkRate)
  {
    m_curBlinkRate = rate;
    if(!m_curBlinkRate)
      m_timer.Stop();
    else
      m_timer.Start(m_curBlinkRate);
  }
  m_init = 0;
}


void  wxTerminal::printText (wxCommandEvent& event){
  

 GTerm::set_mode_flag(BOLD);
  
  out_mut.Lock();

  if (event.GetClientData() != NULL) {
    int * temp = (int *)event.GetClientData();
    this->setCursor(temp[0], temp[1]);
    free (temp);
    movedCursor = 1;
    buff_full_cond.Broadcast();
    out_mut.Unlock();
    return;
  }

  if (out_buff_index_public == 0) {
    alreadyAlerted = 0;
    buff_full_cond.Broadcast();
    out_mut.Unlock();
    GTerm::clear_mode_flag(BOLD);
    return;
  }

  alreadyAlerted = 0;
  wxClientDC
    dc(this);
  if(input_index != 0){
    // set the cursor in the proper place 
    setCursor(last_logo_x, last_logo_y);
    scroll_region(last_logo_y, last_logo_y + 1, -1);
    cursor_moved = 1;
    
  }
  if (out_buff_index_public != 0) {
	  
	  //printf("wxTerminal:printText starts, out_buff_index_public is: %d\n", out_buff_index_public);
	  
    int num_returns = 0;
     int i;
     for (i = 0; i < out_buff_index_public; i++) {
       if (out_buff_public[i] == '\n')
     	 num_returns++;
     }
     int thingy = (num_returns+ cursor_y) - height;
	 
	 	  //printf("Terminal Height: %d, Screen height: %d, \"thingy\": %d\n", height, turtleGraphics->getInfo(SCREEN_HEIGHT), thingy);
		  //printf("x_max: %d, y_max: %d, x_coord: %d. y_coord: %d, m_width: %d, m_height: %d\n", x_max, y_max, x_coord, y_coord, m_width, m_height);
		  //printf("Height(): %d, Width(): %d\n", Height(), Width());
		  
     if (thingy < 0) {
       i = 0;
     }
     else {
       if (num_returns  > height) {
	 thingy = num_returns - height;
	 scroll_region(scroll_top, scroll_bot, height-1);
	 setCursor(cursor_x,0);
       }
       else if (num_returns + cursor_y > height) {
	 thingy = num_returns - (num_returns + cursor_y - height) -1; 
	 scroll_region(scroll_top, scroll_bot, num_returns + cursor_y - height - 1);
	 setCursor(cursor_x,height- (num_returns + cursor_y - height));
       }
       
       for (i = 0; i < out_buff_index_public; i++) {
	 if (out_buff_public[i] == '\n')
	   thingy--;
	 if (thingy == 0)
	   break;
       }
     }

     ClearSelection();
     PassInputToGterm(out_buff_index_public - i, (unsigned char *)out_buff_public + i);
     out_buff_index_public = 0;
  } 

  buff_full_cond.Broadcast();
  out_mut.Unlock();

  last_logo_x = cursor_x;
  last_logo_y = cursor_y;
  if(cursor_moved){
    int x_changed, y_changed;
    x_changed = input_index % x_max;
    y_changed = input_index / x_max;
    setCursor(last_logo_x, last_logo_y);
    // Because scrolling trouble occurs 
    GTerm::clear_mode_flag(BOLD);
    ClearSelection();
    PassInputToGterm(input_index, (unsigned char *)inputBuffer);
    cursor_moved = 0;
  }
  GTerm::clear_mode_flag(BOLD);
}


/* 
	PassInputToInterp() takes all characters in the input buffer and hands
	them off to the logo interpreter
 */
void wxTerminal::PassInputToInterp() {
  int i;  
  in_mut.Lock();
  if(logo_char_mode){
    buff[buff_index++] = inputBuffer[--input_index];
    input_index = 0;
    read_buff.Broadcast();
  }
  else {
    buff[buff_index++] = '\n';
    for (i = input_index -1;i >= 0; i --) {
      buff[buff_index++] = inputBuffer[i];
    }
    input_index = 0;
    read_buff.Broadcast();
    
    // sent to logo, so the text is locked
    last_logo_x = cursor_x;
    last_logo_y = cursor_y;
  }
  in_mut.Unlock();
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
			wxTextDataObject data;
			wxTheClipboard->GetData( data );
			wxString s = data.GetText();

			unsigned int i; 
			char chars[2];
			int len;
			char prev = ' ';
			for (i = 0; i < s.Length(); i++){
				len = 1;
				chars[0] = s.GetChar(i);
				if (prev == ' ' && chars[0] == ' ')
				  continue;
				prev = chars[0];
				inputBuffer[input_index++] = s.GetChar(i);
				if(chars[0] == 10){
					chars[0] = 10;
					chars[1] = 13;
					len = 2;
				}
				ClearSelection();
				PassInputToGterm(len, (unsigned char *)chars);
			}
			
		}  
		wxTheClipboard->Close();
	}
	
}

void wxTerminal::LoseFocus (wxFocusEvent & event){
}


int wxTerminal::currentPosition () {
  // Gives the length of the character up to the cursor

  int ret = 0;
  if (cursor_y - last_logo_y >= 2) {
    ret = (cursor_y - last_logo_y - 1) * width + (width - last_logo_x) + cursor_x;
  }
  else if (cursor_y - last_logo_y == 1) {
    ret = (width - last_logo_x) + cursor_x;
  }
  else 
    ret = cursor_x - last_logo_x;
  return ret;
}

/*
 OnChar is called each time the user types a character
 in the main terminal window
 */


void
wxTerminal::OnChar(wxKeyEvent& event)
{
  ClearSelection();
  static int xval;
  static int yval;
  int
      keyCode = 0,
      len;
    unsigned char
      buf[10];
  keyCode = (int)event.GetKeyCode();
  if(logo_char_mode){
    if (keyCode == WXK_RETURN) {
      keyCode = '\n';
    }
    else if (keyCode == WXK_BACK) {
      keyCode = 8;
    }
    else if (keyCode >= WXK_START) {
	/* ignore it */
    } else {
	if (event.ControlDown()) keyCode -= 0140;
	if (event.AltDown()) keyCode += 0200;
	inputBuffer[input_index++] = keyCode;
	PassInputToInterp();
    }
  }
  else if (keyCode == WXK_RETURN) {
    buf[0] = 10;
    buf[1] = 13;
    len = 2;
    
    PassInputToGterm(len, buf);

    PassInputToInterp();
  }
  else if (keyCode == WXK_BACK) {
    if (input_index == 0)
      return;
    
    int currentPos = currentPosition(); 
    
    if (currentPos == 0)
      return;
    
    if ( currentPos < input_index) { // we are in the middle of input
      int i;
      if (cursor_x == 0) { // need to go back to the prev line
	setCursor(width, cursor_y - 1);
      }
      for (i = currentPos; i < input_index; i++) {
	inputBuffer[i-1] = inputBuffer[i]; 
      }
      input_index--;
      currentPos--;
      inputBuffer[input_index] = ' ';
      cur_x = cursor_x, cur_y = cursor_y;
      setCursor(cur_x - 1, cur_y);
      set_mode_flag(CURSORINVISIBLE);
      PassInputToGterm(input_index - currentPos + 1, // 1 for the space 
		       (unsigned char *)inputBuffer + currentPos);
      clear_mode_flag(CURSORINVISIBLE);
      setCursor(cur_x - 1, cur_y);
    }
    else {
      input_index--;
      buf[0] = keyCode;
      len = 1;
      
      if (cursor_x == 0) { // need to go back to the prev line
	setCursor(width, cursor_y - 1);
      }
      PassInputToGterm(len, buf);
    }
  }
  else if (keyCode == WXK_UP) { // up
    xval = last_logo_x;
    yval = last_logo_y;
    if (readingInstruction) {
      int i;
      setCursor(xval, yval); 
      if (input_index != 0) { // we have to swipe what is already there
	for (i = 0; i < input_index; i++) {
	  inputBuffer[i] = ' ';
	}
	PassInputToGterm(input_index, (unsigned char *)inputBuffer);
      }
      setCursor(xval, yval); 
      // Now get a history entry
	if (--hist_outptr < cmdHistory) {
	    hist_outptr = &cmdHistory[HIST_MAX-1];
	}
	if (*hist_outptr == 0) {
	    wxBell();
	    hist_outptr++;
	} else {
	    PassInputToGterm(strlen((const char *)*hist_outptr), *hist_outptr);
	    for (i = 0; (*hist_outptr)[i]; i++)
		inputBuffer[i] = (*hist_outptr)[i];
      input_index = i;
    }
  }}
  else if  (keyCode == WXK_DOWN) { // down
    xval = last_logo_x;
    yval = last_logo_y;
    if (readingInstruction) {
      int i;
      setCursor(xval, yval); 
      if (input_index != 0) { // we have to swipe what is already there
	for (i = 0; i < input_index; i++) {
	  inputBuffer[i] = ' ';
	}
	PassInputToGterm(input_index, (unsigned char *)inputBuffer);
      }
      setCursor(xval, yval); 
	if (*hist_outptr != 0) {
	    hist_outptr++;
	}
	if (hist_outptr >= &cmdHistory[HIST_MAX]) {
	    hist_outptr = cmdHistory;
	}
	if (*hist_outptr == 0) {
	    wxBell();
	} else {
	    PassInputToGterm(strlen((const char *)*hist_outptr),
			     *hist_outptr);
	    for (i = 0; (*hist_outptr)[i]; i++)
		inputBuffer[i] = (*hist_outptr)[i];
      input_index = i;
    }
  }}
  else if  (keyCode == WXK_LEFT) { // left
    if (cursor_x - 1 < 0)
      setCursor(x_max, max(cursor_y,last_logo_y) - 1);
    int xval;
    if (last_logo_y == cursor_y && cursor_x - 1 < last_logo_x)
      xval = last_logo_x;
    else
      xval = cursor_x - 1;
    setCursor( xval, max(cursor_y,last_logo_y)); 
  }
  else if  (keyCode == WXK_RIGHT) { // right
    int currentPos =  currentPosition();
    if (currentPos >= input_index)
      return;
    if (max(last_logo_x, cursor_x + 1) >= x_max)
       setCursor(0,  max(cursor_y,last_logo_y) + 1);
    else 
      setCursor(max(last_logo_x, cursor_x + 1), max(cursor_y,last_logo_y)); 
  }
  else if (keyCode >= WXK_START) {
	/* ignore it */
  } 
  else {
    buf[0] = keyCode;
    len = 1;
    int doInsert = 0;
    int currentPos = currentPosition();
    if (currentPos < input_index ) { // we are in the middle of input
      doInsert = 1;
      int i;
      for (i = input_index; i >= currentPos + 1; i--) {
	inputBuffer[i] = inputBuffer[i - 1]; 
      }
      inputBuffer[currentPos] = keyCode;
      input_index++;
    }
    else
      inputBuffer[input_index++] = keyCode;
    if (doInsert) {
      // I can't get insert to work quite right ???
      // So I guess this will do
      // Use the global ones so it will be decremented if this scrolls
      cur_x = cursor_x; cur_y = cursor_y;
      set_mode_flag(CURSORINVISIBLE);
      PassInputToGterm(input_index - currentPos,
		       (unsigned char *)(inputBuffer + currentPos));
      clear_mode_flag(CURSORINVISIBLE);
      if (cur_x == width)
	setCursor(1, cur_y + 1);
      else
	setCursor(cur_x+1, cur_y);
    }
    else {
      PassInputToGterm(len, buf);
    }
    
  }

}

void wxTerminal::setCursor (int x, int y) {
  wxClientDC dc (this);
  in_mut.Lock();
  GTerm::move_cursor(x, y);
  GTerm::Update();
  in_mut.Unlock();
}

void wxTerminal::OnSize(wxSizeEvent& event) {
	
	//printf("wxTerminal.OnSize starts\n");
	
  int x, y;
  GetSize(&x, &y);
  
  //int tmpX, tmpY;
  //logoFrame->GetSize(&tmpX, &tmpY);
  //printf("frame height: %d, frame width: %d\n", tmpY, tmpX);
  //printf("wat did getSize return? x: %d,, y: %d\n", x, y);
  //printf("character width: %d, character height: %d\n", m_charWidth, m_charHeight);
  //wxClientDC dc(this);
  //dc.GetSize(&tmpX, &tmpY);
	  //printf("dc size? x: %d, y: %d\n", tmpX, tmpY);
	  
  if (x_max == x / m_charWidth && y_max == y / m_charHeight)
    return;
  x_max = x / m_charWidth;
  y_max = y / m_charHeight;
  
  //printf("what is new x_max, y_max? x_max: %d, y_max:%d\n", x_max, y_max);
  
  if (x_max < 1) 
    x_max = 1;
  if (y_max < 1) 
    y_max = 1;
  ResizeTerminal(x_max, y_max);
}

#if 0
void
wxTerminal::OnKeyDown(wxKeyEvent& event)
{
  if(!(GetMode() & PC) && event.AltDown())
    event.Skip();
  else if(event.AltDown())
  {
  }
  else
    event.Skip();
}
#endif

wxMemoryDC * currentMemDC = NULL;
wxBitmap * currentBitmap = NULL;
int oldWidth = -1;
int oldHeight = -1;

void
wxTerminal::OnPaint(wxPaintEvent& event)
{
	wxPaintDC
    dc(this);
  ExposeArea(0, 0, x_max, y_max);

}

void
wxTerminal::OnLeftDown(wxMouseEvent& event)
{
  m_selecting = TRUE;
  int tmpx = m_selx2, tmpy = m_sely2;
  m_selx2 = m_selx1;
  m_sely2 = m_sely1;
  m_seloldx2 = tmpx;
  m_seloldy2 = tmpy;
  MarkSelection();

	m_seloldx1 = m_selx1;	
	m_seloldx2 = m_selx2;
	m_seloldy1 = m_sely1;
	m_seloldy2 = m_sely2;

  m_selx1 = m_selx2 = event.GetX() / m_charWidth;
  m_sely1 = m_sely2 = event.GetY() / m_charHeight;
  m_selecting = TRUE;

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
}

void
wxTerminal::OnMouseMove(wxMouseEvent& event)
{
  
  if(m_selecting)
  { 
    int tmpx, tmpy;
    tmpx = event.GetX() / m_charWidth;
    if(tmpx >= Width())
      tmpx = Width() - 1;
    tmpy = event.GetY() / m_charHeight;
    if(tmpy >= Height())
      tmpy = Height() - 1;
    
    if ((m_sely2 > m_sely1 || (m_sely2 == m_sely1 && m_selx2 > m_selx1)) &&
	((tmpy < m_sely1) || (tmpy == m_sely1 && tmpx < m_selx1))) {
      int x = m_selx2, y = m_sely2;
      m_selx2 = m_selx1;
      m_sely2 = m_sely1;
      m_seloldx2 = x;
      m_seloldy2 = y;
      MarkSelection();
    }
    m_seloldx1 = m_selx1;	
    m_seloldx2 = m_selx2;
    m_seloldy1 = m_sely1;
	m_seloldy2 = m_sely2;
	
	m_selx2 = tmpx;
	m_sely2 = tmpy;
    
      MarkSelection();
  }
}

void
wxTerminal::ClearSelection()
{
  if (m_sely2 != m_sely1 || m_selx2 != m_selx1) {
    m_sely2 = m_sely1;
    m_selx2 = m_selx1;
    MarkSelection();
  }
}

void
wxTerminal::MarkSelection() {
  static int prev = 0;
  wxClientDC dc(this);
  m_marking = TRUE;
  
  int pixx2 = m_selx2 * m_charWidth, pixy2 = m_sely2 * m_charHeight,
    pixoldx2 = m_seloldx2 * m_charWidth, pixoldy2 = m_seloldy2 * m_charHeight;
  if ((m_sely2 > m_sely1 || (m_sely2 == m_sely1 && m_selx2 > m_selx1)) || 
     ((m_sely2 == m_sely1 && m_selx2 == m_selx1) && !prev)) {
    prev = 0;
    if(m_seloldy2 == m_sely2)
      {
	if(m_selx2 > m_seloldx2)
	  dc.Blit( pixoldx2, pixoldy2, pixx2 - pixoldx2, m_charHeight, &dc, pixoldx2, pixoldy2, wxINVERT);
	else
	  dc.Blit( pixx2, pixy2,pixoldx2 - pixx2, m_charHeight, &dc, pixx2, pixy2, wxINVERT);
      }
    else if(m_seloldy2 < m_sely2)
      {
	dc.Blit( pixoldx2, pixoldy2, (Width() * m_charWidth) - pixoldx2, m_charHeight, &dc, pixoldx2, pixoldy2, wxINVERT);

	dc.Blit( 0,  (m_seloldy2 + 1) * m_charHeight,
		       (Width() * m_charWidth), pixy2 - ((m_seloldy2 + 1) * m_charHeight),
		       &dc,0, (m_seloldy2 + 1) * m_charHeight, wxINVERT);
	dc.Blit( 0, pixy2, pixx2, m_charHeight, &dc, 0, pixy2, wxINVERT);
	
      }
    else
      {
	dc.Blit( 0, pixoldy2, pixoldx2, m_charHeight, &dc, 0, pixoldy2, wxINVERT);

	dc.Blit( 0,  (m_sely2 + 1) * m_charHeight,
		       (Width() * m_charWidth), pixoldy2 - ((m_sely2 + 1) * m_charHeight),
		       &dc,0, (m_sely2 + 1) * m_charHeight, wxINVERT);

	dc.Blit( pixx2 /*+ m_charWidth*/, pixy2, 
		       (Width() * m_charWidth)-(pixx2  /*+ m_charWidth*/),
		       m_charHeight, &dc, pixx2 /*+ m_charWidth*/, pixy2, wxINVERT);
      }
  } else {
    prev = 1;
    if(m_seloldy2 == m_sely2)
      {
	if(m_selx2 <= m_seloldx2)
	  dc.Blit( pixx2, pixy2, pixoldx2 - pixx2 , m_charHeight, &dc, pixx2, pixy2, wxINVERT);
	else
	  dc.Blit( pixoldx2, pixoldy2,pixx2 - pixoldx2, m_charHeight, &dc, pixoldx2, pixoldy2, wxINVERT);
      }
    else if(m_seloldy2 > m_sely2)
      {
	dc.Blit( pixx2, pixy2, (Width() * m_charWidth) - pixx2, m_charHeight, &dc, pixx2, pixy2, wxINVERT);

	dc.Blit( 0,  (m_sely2 + 1) * m_charHeight,
		       (Width() * m_charWidth), pixoldy2 - ((m_sely2 + 1) * m_charHeight),
		       &dc,0, (m_sely2 + 1) * m_charHeight, wxINVERT);

	dc.Blit( 0, pixoldy2, pixoldx2, m_charHeight, &dc, 0, pixoldy2, wxINVERT);
      }
    else
      {
	dc.Blit( 0, pixy2, pixx2, m_charHeight, &dc, 0, pixy2, wxINVERT);

	dc.Blit( 0,  (m_seloldy2 + 1) * m_charHeight,
		       (Width() * m_charWidth), pixy2 - ((m_seloldy2 + 1) * m_charHeight),
		       &dc,0, (m_seloldy2 + 1) * m_charHeight, wxINVERT);
	
	dc.Blit( pixoldx2, pixoldy2, 
		       (Width() * m_charWidth)-(pixoldx2  ) ,
		       m_charHeight, &dc, pixoldx2, pixoldy2, wxINVERT);
	
    }
    
  }

  dc.SetLogicalFunction(wxCOPY);
  wxWindow::Update();

  m_marking = FALSE;
}

bool
wxTerminal::HasSelection()
{
  return(m_selx1 != m_selx2 || m_sely1 != m_sely2);
}

wxString
wxTerminal::GetSelection()
{
  int
    x1,
    y1,
    x2,
    y2;

  wxString
    sel;

  if(m_sely1 <= m_sely2)
  {
    x1 = m_selx1;
    y1 = m_sely1;
    x2 = m_selx2;
    y2 = m_sely2;
  }
  else
  {
    x1 = m_selx2;
    y1 = m_sely2;
    x2 = m_selx1;
    y2 = m_sely1;
  }

  int numSpace=0;
  while(x1 != x2 || y1 != y2)
  {
	  if(GetChar(x1, y1)){
		  if(GetChar(x1,y1)==32)   // watch for trailing spaces
			  numSpace++;
		  else{
			  int i;
			  for(i=0;i<numSpace;i++)
				  sel.Append(32);
			  numSpace=0;
			  sel.Append(GetChar(x1, y1));
		  }

	  }

    x1++;
    if(x1 == Width())
    {
      sel.Append('\n');
	  numSpace=0;
      x1 = 0;
      y1++;
    }
  }
  if(GetChar(x1, y1))
    sel.Append(GetChar(x1, y1));

  return sel;
}

void
wxTerminal::SelectAll()
{
  m_selx1 = 0;
  m_sely1 = 0;
  m_selx2 = Width() - 1;
  m_sely2 = Height() - 1;
  MarkSelection();
}

/*
**  GTerm hooks
*/
void
wxTerminal::DrawText(int fg_color, int bg_color, int flags,
                 int x, int y, int len, unsigned char *string)
{
	int
    t;
    wxClientDC dc(this);

  if(flags & BOLD && m_boldStyle == COLOR)
    fg_color = (fg_color % 8) + 8;

  if(flags & SELECTED)
  {
    fg_color = 0;
    bg_color = 15;
  }

  if(flags & INVERSE)
  {
    t = fg_color;
    fg_color = bg_color;
    bg_color = t;
  }

  wxString
    str(string, len);

  if(m_boldStyle != FONT)
  {
    if(flags & UNDERLINE)
      dc.SetFont(m_underlinedFont);
    else
      dc.SetFont(m_normalFont);
  }
  else
  {
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
  }

  int coord_x, coord_y;
  dc.SetBackgroundMode(wxSOLID);
  dc.SetTextBackground(m_colors[bg_color]);
  dc.SetTextForeground(m_colors[fg_color]);
  coord_y = y * m_charHeight; 
  coord_x = x * (m_charWidth);
	  
  for(unsigned int i = 0; i < str.Length(); i++, coord_x+=m_charWidth){
	  dc.DrawText(str.Mid(i, 1), coord_x, coord_y);
	  if(flags & BOLD && m_boldStyle == OVERSTRIKE)
	  dc.DrawText(str, x + 1, y);
  }
}

void
wxTerminal::DoDrawCursor(int fg_color, int bg_color, int flags,
                   int x, int y, unsigned char c)
{
  int
    t;
    wxClientDC dc(this);

  if(flags & BOLD && m_boldStyle == COLOR)
    fg_color = (fg_color % 8) + 8;

  if(flags & INVERSE)
  {
    t = fg_color;
    fg_color = bg_color;
    bg_color = t;
  }

#ifndef __WXMSW__
  /*  c = xCharMap[c];*/
#endif

  wxString
    str((char)c);

  if(m_boldStyle != FONT)
  {
    if(flags & UNDERLINE)
      dc.SetFont(m_underlinedFont);
    else
      dc.SetFont(m_normalFont);
  }
  else
  {
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
  }
  int old_y = y;
  x = x * m_charWidth;
  y = y * m_charHeight;
  dc.SetBackgroundMode(wxSOLID);
  dc.SetTextBackground(m_colors[fg_color]);
  dc.SetTextForeground(m_colors[bg_color]);
  dc.DrawText(str, x, y);
  if (old_y == height - 1){
    dc.SetBackgroundMode(wxSOLID);
    dc.SetTextBackground(m_colors[bg_color]);
    dc.SetTextForeground(m_colors[fg_color]);
    dc.DrawText(' ', x, y + m_charHeight - 1);
  }
  if(flags & BOLD && m_boldStyle == OVERSTRIKE)
    dc.DrawText(str, x + 1, y);
}

void
wxTerminal::DrawCursor(int fg_color, int bg_color, int flags,
                   int x, int y, unsigned char c)
{
  m_curX = x;
  m_curY = y;
  m_curFG = fg_color;
  m_curBG = bg_color,
  m_curFlags = flags;
  m_curChar = c;

  if(m_timer.IsRunning())
    m_timer.Stop();
  DoDrawCursor(fg_color, bg_color, flags, x, y, c);
  if(m_curBlinkRate)
  {
    m_timer.Start(m_curBlinkRate);
    m_curState = 1;
  }
}

void
wxTerminal::OnTimer(wxTimerEvent& WXUNUSED(event))
{
 
  wxClientDC dc(this);

  if(m_init)
    return;
    
  if(m_curX == -1 || m_curY == -1)
    return;
    
  if(GetMode() & CURSORINVISIBLE)
    return;

  if(m_curBlinkRate)
  {
    m_curState++;
    if(m_curState & 1 && m_curX != -1 && m_curY != -1)
      DoDrawCursor(m_curFG, m_curBG, m_curFlags, m_curX, m_curY, m_curChar);
    else
      DoDrawCursor(m_curBG, m_curFG, m_curFlags, m_curX, m_curY, m_curChar);
  }
}

void
wxTerminal::MoveChars(int sx, int sy, int dx, int dy, int w, int h)
{
    wxClientDC dc(this);

  if(!m_marking)
    ClearSelection();

  sx = sx * m_charWidth;
  sy = sy * m_charHeight;
  dx = dx * m_charWidth;
  dy = dy * m_charHeight;
  w = w * m_charWidth;
  h = h * m_charHeight;

  dc.Blit(dx, dy, w, h, &dc, sx, sy);

}

void
wxTerminal::ClearChars(int bg_color, int x, int y, int w, int h)
{
    wxClientDC dc(this);
  x = x * m_charWidth;
  y = y * m_charHeight;
  w = w * (m_charWidth + 1);	// to get rid of the extra bit of cursor
  h = h * m_charHeight;

  dc.SetPen(m_colorPens[bg_color]);
  dc.SetBrush(wxBrush(m_colors[bg_color], wxSOLID));
  dc.DrawRectangle(x, y, w /* + 1*/, h /* + 1*/);
}

void
wxTerminal::ModeChange(int state)
{
  GTerm::ModeChange(state);
}

void
wxTerminal::Bell()
{
  wxBell();
}

void wxTerminal::RefreshTerminal(){
	ResizeTerminal(m_width, m_height);
	Refresh();
}

void
wxTerminal::ResizeTerminal(int width, int height)
{
  int
    w,
    h;

  /*
  **  Determine window size from current font
  */
  wxClientDC
    dc(this);

  if(m_boldStyle != FONT)
    dc.SetFont(m_normalFont);
  else
    dc.SetFont(m_boldFont);
	
  dc.GetTextExtent("M", &m_charWidth, &m_charHeight);
  m_charWidth--;
  w = width * m_charWidth;
  h = height * m_charHeight;

  /*
  **  Create our bitmap for copying
  */
  if(m_bitmap)
  {
    m_memDC.SelectObject(wxNullBitmap);
      delete m_bitmap;
  }

  /*
  **  Set terminal size
  */
  GTerm::ResizeTerminal(width, height);
  m_width = width;
  m_height = height;

  /*
  **  Send event
  */
  if(!m_init)
  {
    wxCommandEvent e(wxEVT_COMMAND_TERM_RESIZE, GetId());
    e.SetEventObject(this);
    GetParent()->GetEventHandler()->ProcessEvent(e);
  }
}

void
wxTerminal::RequestSizeChange(int w, int h)
{
  ResizeTerminal(w, h);
}


void
wxTerminal::PassInputToGterm(int len, unsigned char *data)
{
   wxClientDC
    dc(this);

  GTerm::PassInputToGterm(len, data);
  last_user_x = cursor_x;
  last_user_y = cursor_y;

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
wxTerminal::PrintChars(int len, unsigned char *data)
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
}

extern "C" void wxClearText() {
	out_mut.Lock();
	wxTerminal::terminal->Reset();
	out_buff_index_public = 0;
	out_buff_index_private = 0;
	out_mut.Unlock();
}

extern "C" void flushFile(FILE * stream, int);

extern "C" void wxSetCursor(int x, int y){
	int * data = (int *)malloc(2 * sizeof(int));
	data[0] = x;
	data[1] = y;
	wxCommandEvent event(wxEVT_MY_CUSTOM_COMMAND);
	event.SetClientData((void *)data);
	flushFile(stdout, 0);
	out_mut.Lock();
	movedCursor = 0;
	out_mut.Unlock();
	wxPostEvent(wxTerminal::terminal,event);
	out_mut.Lock();
	if (!movedCursor)
		buff_full_cond.Wait();
	movedCursor = 0;
	out_mut.Unlock();
}



extern "C" int check_wx_stop() {
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
		return wxTerminal::terminal->x_coord;
		break;
	case Y_COORD:
		return wxTerminal::terminal->y_coord;
		break;
	case X_MAX:
		return wxTerminal::terminal->x_max;
		break;
	case Y_MAX:
		return wxTerminal::terminal->y_max;
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
			wxTerminal::terminal->x_coord=val;
			break;
		case Y_COORD:
			wxTerminal::terminal->y_coord=val;
			break;
		case X_MAX:
			wxTerminal::terminal->x_max=val;
			break;
		case Y_MAX:
			wxTerminal::terminal->y_max=val;
			break;
		case EDIT_STATE:
			editWindow->stateFlag=val;
			break;
	}

}
