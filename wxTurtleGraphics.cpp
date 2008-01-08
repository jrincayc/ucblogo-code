#include <stdio.h>
#include "wxTurtleGraphics.h"
#include "TextEditor.h"
#include <wx/stdpaths.h>
#define WX_TURTLEGRAPHICS_CPP 1

using namespace std;


#define SCREEN_WIDTH		1
#define SCREEN_HEIGHT		2
#define	BACK_GROUND			3
#define	IN_SPLITSCREEN		4
#define	IN_GRAPHICS_MODE	5

// ----------------------------------------------------------------------------
// Custom Events
// ----------------------------------------------------------------------------

/* The line drawing event */
#ifdef MULTITHREAD
BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_MY_TURTLE_CUSTOM_COMMAND, 7777)
END_DECLARE_EVENT_TYPES()
DEFINE_EVENT_TYPE(wxEVT_MY_TURTLE_CUSTOM_COMMAND)

#define EVT_MY_TURTLE_CUSTOM_COMMAND(id, fn) \
DECLARE_EVENT_TABLE_ENTRY( \
						   wxEVT_MY_TURTLE_CUSTOM_COMMAND, id, -1, \
						   (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)&fn, \
						   (wxObject *) NULL \
						   ),
#endif

  /* The edit event */
BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_EDIT_CUSTOM_COMMAND, 7777)
END_DECLARE_EVENT_TYPES()
DEFINE_EVENT_TYPE(wxEVT_EDIT_CUSTOM_COMMAND)

#define EVT_EDIT_CUSTOM_COMMAND(id, fn) \
DECLARE_EVENT_TABLE_ENTRY( \
						   wxEVT_EDIT_CUSTOM_COMMAND, id, -2, \
						   (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)&fn, \
						   (wxObject *) NULL \
						   ),

 
  /* Used for multithread event handling, as well as
     for a dummy event otherwise */
BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_LOGO_CUSTOM_COMMAND, 7777)
END_DECLARE_EVENT_TYPES()
DEFINE_EVENT_TYPE(wxEVT_LOGO_CUSTOM_COMMAND)

#define EVT_LOGO_CUSTOM_COMMAND(id, fn) \
DECLARE_EVENT_TABLE_ENTRY( \
						   wxEVT_LOGO_CUSTOM_COMMAND, id, -1, \
						   (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)&fn, \
						   (wxObject *) NULL \
						   ),
// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#ifndef WIN32
char* LogoPlatformName="wxWidgets";
#endif

pen_info p;
int TurtleFrame::back_ground = 0;
int TurtleFrame::screen_height = 0; 
int TurtleFrame::screen_width = 0;
int TurtleFrame::in_graphics_mode = 0;
int TurtleFrame::in_splitscreen = 0;
pen_info TurtleFrame::xgr_pen = p;

int ignorePaint = 0;	// for Paint events when we've already handled it
int drawToWindow = 0;	// for redraw_graphics from gui thread
wxDC *windowDC = 0;

wxMemoryDC *m_memDC;
#define USE_MEMDC 0

int pictureleft = 0, pictureright = 0, picturetop = 0, picturebottom = 0;
// Keep track of max range for printing.


int TurtleCanvas::mousePosition_x;
int TurtleCanvas::mousePosition_y;

int TurtleCanvas::mouse_down_left;
int TurtleCanvas::mouse_down_middle;
int TurtleCanvas::mouse_down_right;

wxColour TurtleCanvas::colors[NUMCOLORS+2];

int R, G, B;

#ifdef MULTITHREAD
vector <struct line> lines;
#endif

// Global print data, to remember settings during the session
wxPrintData *g_printData = (wxPrintData*) NULL ;

// Global page setup data
wxPageSetupDialogData* g_pageSetupData = (wxPageSetupDialogData*) NULL;

// Used for printing
TurtleWindowPrintout *turtlePrintout;
int drawToPrinter=0;
wxDC *printerDC;
wxBitmap *tempBitmap;

#ifdef MULTITHREAD
wxCommandEvent turtleEvent = wxCommandEvent(wxEVT_MY_TURTLE_CUSTOM_COMMAND);
wxMutex * messageMut;
wxCondition * messageJoinCond;
wxThread * guiThread;
wxMutex tMut;
#define MAX_LINES_BUFFERED 100
wxCondition lineCond(tMut);
#endif
// have already called prepareToDraw
int prepared = 0;
TurtleFrame *turtleFrame;
int turtleIndex = 0;
int putInQueue = 0;

#ifdef MULTITHREAD
wxMutex editMut;
wxCondition editCond(editMut);
#endif
wxCommandEvent editEvent = wxCommandEvent(wxEVT_EDIT_CUSTOM_COMMAND);
char * file;

// the location of the turtle
int turtlePosition_x = 0;
int turtlePosition_y = 0;

#define LINEPAUSE 30

// ----------------------------------------------------------------------------
// Debug Functions
// ----------------------------------------------------------------------------

/* We'll use 16 named colors for now (see xgraphics.h).
   The ordering here corresponds to the zero-based argument
   to setpencolor that gives that color -- pink is 12,
   turquoise is 10 etc.
 */

/*

void PrintLines(){
	struct line l;
	unsigned int turtleIndex = 0;
	for (; turtleIndex<lines.size();turtleIndex++) {
		l = lines[turtleIndex];
		wxdprintf("Pen color = %s ", TurtleCanvas::colors[l.color+2]);
		//dc.SetPen( wxPen(TurtleCanvas::colors[l.color+2], 1, wxSOLID) );
		if(l.pm==PEN_REVERSE)
		  {wxdprintf("and reversed \n");}
		else if(l.pm==PEN_ERASE)
		  {wxdprintf("and erased \n");}
		else wxdprintf("and copy \n");
	}


	}*/

// ----------------------------------------------------------------------------
// TurtleCanvas Class
// ----------------------------------------------------------------------------
// this is used to make the logo thread freeze until the event
// is handled by the UI thread to avoid concurrency issues
int alreadyDone = 0;



/* The TurtleCanvas event table*/
BEGIN_EVENT_TABLE(TurtleCanvas, wxWindow)
EVT_PAINT  (TurtleCanvas::OnPaint)
EVT_SIZE (TurtleCanvas::OnSize)
EVT_MOTION (TurtleCanvas::OnMouseMove)
#ifdef MULTITHREAD
  EVT_MY_TURTLE_CUSTOM_COMMAND(-1, TurtleCanvas::drawLine)
#endif
EVT_EDIT_CUSTOM_COMMAND(-1, TurtleCanvas::editCall)
#ifdef MULTITHREAD
  EVT_LOGO_CUSTOM_COMMAND (-1, TurtleCanvas::logoHandle)
#endif
EVT_SET_FOCUS( TurtleCanvas::OnFocus)
EVT_KILL_FOCUS(TurtleCanvas::LoseFocus)
EVT_LEFT_DOWN(TurtleCanvas::OnLeftDown)
EVT_LEFT_UP(TurtleCanvas::OnLeftUp)
EVT_MIDDLE_DOWN(TurtleCanvas::OnMiddleDown)
EVT_MIDDLE_UP(TurtleCanvas::OnMiddleUp)
EVT_RIGHT_DOWN(TurtleCanvas::OnRightDown)
EVT_RIGHT_UP(TurtleCanvas::OnRightUp)
#ifdef MULTITHREAD
  EVT_TIMER(-1, TurtleCanvas::OnTimer)
#endif
EVT_ERASE_BACKGROUND(TurtleCanvas::OnEraseBackGround)
END_EVENT_TABLE()

/* The TurtleCanvas class is what the turtle is drawn on */
TurtleCanvas::TurtleCanvas(wxFrame *parent)
        : wxWindow(parent, -1, wxDefaultPosition, wxDefaultSize,
                           wxNO_FULL_REPAINT_ON_RESIZE)
{
  m_memDC=new wxMemoryDC();
  m_bitmap=0;
  m_owner = parent;
  m_show = Show_Lines;
  m_clip = FALSE;
#ifdef MULTITHREAD
  messageMut = new wxMutex();
  messageJoinCond = new wxCondition(*messageMut);
  guiThread = wxThread::This();
#endif
  tempBitmap=0;
  g_printData = new wxPrintData;

    g_pageSetupData = new wxPageSetupDialogData;
    // copy over initial paper size from print record
    (*g_pageSetupData) = *g_printData;
    // Set some initial page margins in mm. 
    g_pageSetupData->SetMarginTopLeft(wxPoint(15, 15));
    g_pageSetupData->SetMarginBottomRight(wxPoint(15, 15));
#ifndef __WXMAC__   /* needed for wxWidgets 2.6 */
    wxSetWorkingDirectory(wxStandardPaths::Get().GetDocumentsDir());
#endif

  oldWidth = -1;
  oldHeight = -1;
  mousePosition_x = 0;
  mousePosition_y = 0;
  mouse_down_left = 0;
  mouse_down_middle = 0;
  mouse_down_right = 0;

  // initialize the TurtleCanvas::colors
  int i;
    TurtleCanvas::colors[2] = wxColour(0, 0, 0);
    TurtleCanvas::colors[3] = wxColour(0, 0, 255);
    TurtleCanvas::colors[4] = wxColour(0, 255, 0);
    TurtleCanvas::colors[5] = wxColour(0, 255, 255);
    TurtleCanvas::colors[6] = wxColour(255, 0, 0);
    TurtleCanvas::colors[7] = wxColour(255, 0, 255);
    TurtleCanvas::colors[8] = wxColour(255, 255, 0);
    TurtleCanvas::colors[9] = wxColour(255, 255, 255);
    TurtleCanvas::colors[10] = wxColour(155, 96, 59);
    TurtleCanvas::colors[11] = wxColour(197, 136, 18);
    TurtleCanvas::colors[12] = wxColour(100, 162, 64);
    TurtleCanvas::colors[13] = wxColour(120, 187, 187);
    TurtleCanvas::colors[14] = wxColour(255, 149, 119);
    TurtleCanvas::colors[15] = wxColour(144, 113, 208);
    TurtleCanvas::colors[16] = wxColour(255, 163, 0);
    TurtleCanvas::colors[17] = wxColour(183, 183, 183);
  for(i=18;i<NUMCOLORS+2;i++){
    TurtleCanvas::colors[i] = TurtleCanvas::colors[(i-2)%NUMINITCOLORS+2];
  }
	
#ifdef MULTITHREAD  
  m_timer = new wxTimer(this);
  m_timer->Start(100);
#endif

		turtleFrame->xgr_pen.vis = 0;
	int screen_width, screen_height;
	parent->GetSize(&screen_width, &screen_height);
	setInfo(SCREEN_WIDTH, screen_width);
	setInfo(SCREEN_HEIGHT, screen_height);
		turtleFrame->xgr_pen.xpos = screen_width/2;
		turtleFrame->xgr_pen.ypos = screen_height/2;
	pictureleft = pictureright = screen_width/2;
	picturetop = picturebottom = screen_height/2;
		turtleFrame->xgr_pen.color = 7;
		turtleFrame->xgr_pen.pw = 1;
		turtleFrame->xgr_pen.pen_mode = PEN_DOWN;

}




void TurtleCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	wxdprintf("OnPaint starts\n");

  wxPaintDC dc(this);
  dc.DestroyClippingRegion(); //evan

  wxFont f(12, wxDEFAULT, wxNORMAL, wxNORMAL, false, "Courier");
  SetFont(f);

  int x, y;

    if (ignorePaint) {
	ignorePaint = 0;
	return;
    }

  GetSize(&x, &y);
#if USE_MEMDC
  if (oldWidth == x  && oldHeight == y && m_bitmap != NULL) {
    dc.DrawBitmap(*m_bitmap, 0,0);
    return;
  }
#endif
  
  oldWidth = x;
  oldHeight = y;

#if USE_MEMDC
  /*
   **  Create our bitmap for copying
   */
  if(m_bitmap)
  {
	  m_memDC->SelectObject(wxNullBitmap);
      delete m_bitmap;
  }
  m_bitmap = new wxBitmap(x, y);


    PrepareDC(*m_memDC);
    wxBrush myBrush(TurtleCanvas::colors[turtleFrame->back_ground+2],wxSOLID);
    m_memDC->SelectObject(*m_bitmap);
    m_memDC->SetBackgroundMode( wxSOLID );
    m_memDC->SetBackground( myBrush );
    m_memDC->Clear();
#endif

#ifdef MULTITHREAD
    tMut.Lock();
    lines.clear();
    tMut.Unlock();
#endif
//    pictureleft = pictureright = screen_width/2;
//    picturetop = picturebottom = screen_height/2;
#if 0
    in_mut.Lock();
    needToRefresh++;
    read_buff.Broadcast();
    in_mut.Unlock();
#else
    int unset_windowDC = 0;
    if(windowDC == 0) {
      windowDC = &dc;
      unset_windowDC++;
    }
    drawToWindow++;
    redraw_graphics();
    drawToWindow--;

    if(unset_windowDC) {
      windowDC = 0;
      unset_windowDC--;
    }
#endif
	wxdprintf("OnPaint ends\n");
}

void TurtleCanvas::OnSize(wxSizeEvent& event) {
	wxdprintf("OnSize starts\n");

	wxClientDC dc(this);
	int x, y;

	dc.DestroyClippingRegion(); //evan
	
	GetSize(&x, &y);
#if USE_MEMDC
	if (oldWidth == x  && oldHeight == y && m_bitmap != NULL) {
		dc.DrawBitmap(*m_bitmap, 0,0);
		return;
	}
#endif
	
	oldWidth = x;
	oldHeight = y;
	/*
	 *  Create our bitmap for copying
	 */
#if USE_MEMDC
	if(m_bitmap)
	{
	  m_memDC->SelectObject(wxNullBitmap);
	  delete m_bitmap;
	}
	m_bitmap = new wxBitmap(x, y);
	
	
    PrepareDC(*m_memDC);
	wxBrush myBrush(TurtleCanvas::colors[turtleFrame->back_ground+2],wxSOLID);
	m_memDC->SelectObject(*m_bitmap);
    m_memDC->SetBackgroundMode( wxSOLID );
    m_memDC->SetBackground( myBrush );
    m_memDC->Clear();
#endif
//    pictureleft = pictureright = getInfo(SCREEN_WIDTH)/2;
//    picturetop = picturebottom = getInfo(SCREEN_HEIGHT)/2;

#ifdef MULTITHREAD
    tMut.Lock();
    lines.clear();
    tMut.Unlock();
#endif
    int screen_width, screen_height;
    logoFrame->GetSize(&screen_width, &screen_height);
    setInfo(SCREEN_WIDTH, screen_width);
    setInfo(SCREEN_HEIGHT, screen_height);
#if 0
    in_mut.Lock();
    needToRefresh++;
    read_buff.Broadcast();
    in_mut.Unlock();
#else
    int unset_windowDC = 0;
    if(windowDC == 0) {   //OnSize may be triggered multiple times...
      windowDC = &dc;
      unset_windowDC++;
    }
    
    drawToWindow++;
    redraw_graphics();
    drawToWindow--;
#endif
    if(unset_windowDC) {
      windowDC = 0;
      unset_windowDC--;
    }
	
    wxdprintf("OnSize ends\n");
}


void TurtleCanvas::OnFocus (wxFocusEvent & event){
	wxTerminal::terminal->SetFocus();
}

void TurtleCanvas::LoseFocus (wxFocusEvent & event){
  
}

void TurtleCanvas::SetOwner(wxFrame * owner) {
	m_owner = owner;
}

wxFrame *  TurtleCanvas::GetOwner() {
	return m_owner;
}

wxDC *  TurtleCanvas::GetDC() {
	return dc;
}

void TurtleCanvas::OnEraseBackGround(wxEraseEvent& event) {
	wxdprintf("Executing OnEraseBackGround\n");
}


#ifdef MULTITHREAD
void TurtleCanvas::OnTimer (wxTimerEvent& event) {
    wxCommandEvent  e;
    if (!drawToPrinter && !drawToWindow) drawLine(e);
}
#endif

void TurtleCanvas::drawOneLine(struct line *l, wxDC *dc) {
    wxPen myPen;
    wxColour xorColor;

    if (l->pm==PEN_ERASE) {
	myPen = wxPen(TurtleCanvas::colors[turtleFrame->back_ground+2],
			l->pw, wxSOLID);

    } else if (l->pm==PEN_REVERSE) {
	unsigned int pr, pg, pb, br, bg, bb;
	get_palette(l->color, &pr, &pg, &pb);
	get_palette(turtleFrame->back_ground, &br, &bg, &bb);
	xorColor=wxColour((pr^br)/256, (pg^bg)/256, (pb^bb)/256);
	myPen = wxPen(xorColor, l->pw, wxSOLID);

    } else if(drawToPrinter && turtleFrame->back_ground==0 && l->color==7){
	myPen = wxPen( wxT("black"), l->pw, wxSOLID);
    } else {
	myPen = wxPen(TurtleCanvas::colors[l->color+2], l->pw, wxSOLID);
    }
    dc->SetPen(myPen);

#if USE_MEMDC
	if (!drawToPrinter)
	    m_memDC->SetPen(wxPen(TurtleCanvas::colors[l->color+2],
				 l->pw, wxSOLID) );		  
#endif
    if(l->pm==PEN_REVERSE){
	dc->SetLogicalFunction(wxXOR);
#if USE_MEMDC
	if (!drawToPrinter)
	    m_memDC->SetLogicalFunction(wxXOR);
#endif
    } else {  
	dc->SetLogicalFunction(wxCOPY);
#if USE_MEMDC
	if (!drawToPrinter)
	    m_memDC->SetLogicalFunction(wxCOPY);
#endif
    }
    if(l->vis) {
      dc->DrawLine(l->x1,l->y1,l->x2,l->y2);
	if (!drawToPrinter && !drawToWindow) {
#if USE_MEMDC
	    m_memDC->DrawLine(l->x1,l->y1,l->x2,l->y2);
#endif
	    if (l->x2 < pictureleft) pictureleft = l->x2;
	    if (l->x2 > pictureright) pictureright = l->x2;
	    if (l->y2 < picturetop) picturetop = l->y2;
	    if (l->y2 > picturebottom) picturebottom = l->y2;
	}
    }
    turtlePosition_x = l->x2;
    turtlePosition_y = l->y2;
    dc->SetPen(wxNullPen);
}

extern int turtle_shown;
extern "C" void draw_turtle();

#ifdef MULTITHREAD
void TurtleCanvas::drawLine(wxCommandEvent & event) {
    wxDC *dc;
    int broadcast = 0;
    tMut.Lock();
    putInQueue = 0;

    if (lines.size() > MAX_LINES_BUFFERED)
      broadcast = 1;

    if(drawToPrinter)
	dc=printerDC;
    else {
      //need to set drawToWindow!
      //this event cannot be happening at the same time as
      // a redraw_graphics, so we should be okay!
      
      //just in case, set windowDC
      if(drawToWindow) {
	//this is an error... what shall I do? debug for now
	fprintf(stderr, " **** error!!!! ****");
	
      }
      dc = new wxClientDC(this);
      windowDC = dc;
      drawToWindow++;    
    }
    turtleIndex = 0;
    for (; turtleIndex<lines.size();turtleIndex++) {
	drawOneLine(&lines[turtleIndex], dc);
    }

    lines.clear();
    if (broadcast) {
      	lineCond.Broadcast();
    }
        tMut.Unlock();
  
    if(!drawToPrinter){
      drawToWindow--;
      windowDC = 0;
      delete dc;
    }
}
#endif

void TurtleCanvas::editCall(wxCommandEvent &e){  // So long as this is handled by any gui thread, it should be thread safe
  editWindow->Clear();
  topsizer->Show(wxTerminal::terminal, 0);
  topsizer->Show(turtleGraphics, 0);
  topsizer->Show(editWindow, 1);
  logoFrame->SetUpEditMenu();
  topsizer->Layout();
  editWindow->SetFocus();
  FILE * filestream;
  filestream = fopen(file, "r");
  if (filestream == NULL) {
		filestream = fopen(file, "w");
  }
  fclose(filestream);
  editWindow->Load(file);
}


void
TurtleCanvas::OnLeftDown(wxMouseEvent& event) {
  mouse_down_left = 1;
  event.Skip(); //allow native handler to set focus
}
void TurtleCanvas::OnLeftUp(wxMouseEvent& event) {
  mouse_down_left = 0;
  event.Skip(); //allow native handler to set focus
}
void TurtleCanvas::OnMiddleDown(wxMouseEvent& event) {
  mouse_down_middle = 3;
  event.Skip(); //allow native handler to set focus
}
void TurtleCanvas::OnMiddleUp(wxMouseEvent& event) {
  mouse_down_middle = 0;
  event.Skip(); //allow native handler to set focus
}
void TurtleCanvas::OnRightDown(wxMouseEvent& event) {
  mouse_down_right = 2;
  event.Skip(); //allow native handler to set focus
}
void TurtleCanvas::OnRightUp(wxMouseEvent& event) {
  mouse_down_right = 0;
  event.Skip(); //allow native handler to set focus
}

void TurtleCanvas::OnMouseMove(wxMouseEvent& event){
  mousePosition_x = event.m_x;
  mousePosition_y = event.m_y;
  event.Skip(); //allow native handler to set focus
}

#ifdef MULTITHREAD
void TurtleCanvas::FinishedEvent(){
	// wake the sleeping logo thread
  	messageMut->Lock();
  	alreadyDone = 1;
  	messageJoinCond->Broadcast();
  	messageMut->Unlock();
}


void TurtleCanvas::WaitForEvent(){	
  	messageMut->Lock();
  	while (!alreadyDone)
  		messageJoinCond->Wait();
  	messageMut->Unlock();	
}
#endif

extern "C" pen_info* getPen();

void TurtleCanvas::realClearScreen(wxDC *dc) {
    wxBrush myBrush(TurtleCanvas::colors[turtleFrame->back_ground+2],wxSOLID);
    if(drawToPrinter && turtleFrame->back_ground==0){
	    myBrush.SetColour("white");
    }
    dc->SetBackgroundMode( wxSOLID );
    dc->SetBackground( myBrush );
    dc->Clear();
    if (!drawToPrinter && !drawToWindow) {
#if USE_MEMDC
	m_memDC->SetBackgroundMode( wxSOLID );
	m_memDC->SetBackground( myBrush );
	m_memDC->Clear();
#endif
	pictureleft = pictureright = getInfo(SCREEN_WIDTH)/2;
	picturetop = picturebottom = getInfo(SCREEN_HEIGHT)/2;
    }
}

void TurtleCanvas::realFloodFill(int color, wxDC *dc) {
    wxColour c;
//fprintf(stderr, "realFloodFill: (x,y): (%d, %d)\n", turtlePosition_x, turtlePosition_y);
    dc->GetPixel(turtlePosition_x, turtlePosition_y, &c);
    wxBrush brush(TurtleCanvas::colors[color+2]);
#if USE_MEMDC
    if (!drawToPrinter) {
	m_memDC->SetBrush(brush);
	m_memDC->FloodFill(turtlePosition_x, turtlePosition_y , c);
    }
#endif
    dc->SetBrush(brush);
    dc->FloodFill(turtlePosition_x, turtlePosition_y , c);
//    dc->FloodFill(dc->LogicalToDeviceX(turtlePosition_x), dc->LogicalToDeviceY(turtlePosition_y) , c);
}

void TurtleCanvas::realDrawLabel(char *data, wxDC *dc) {
    wxString s(data);
    wxCoord wid, ht;
	
    dc->GetTextExtent(s, &wid, &ht);
    dc->SetBackgroundMode(wxTRANSPARENT);
    if (turtleFrame->back_ground == 0 && drawToPrinter) {
	dc->SetTextBackground(TurtleCanvas::colors[9]);
	if (turtleFrame->xgr_pen.color == 7)
	    dc->SetTextForeground(TurtleCanvas::colors[2]);
	else
	    dc->SetTextForeground(TurtleCanvas::colors[turtleFrame->xgr_pen.color+2]);
    } else {
	dc->SetTextBackground(TurtleCanvas::colors[turtleFrame->back_ground+2]);
	dc->SetTextForeground(TurtleCanvas::colors[turtleFrame->xgr_pen.color+2]);
    }
    dc->DrawText(s, getPen()->xpos, getPen()->ypos-ht);
    if (!drawToPrinter) {
#if USE_MEMDC
	m_memDC->SetBackgroundMode(wxSOLID);
	m_memDC->SetTextBackground(TurtleCanvas::colors[turtleFrame->back_ground+2]);
	m_memDC->SetTextForeground(TurtleCanvas::colors[turtleFrame->xgr_pen.color+2]);
	m_memDC->DrawText(s, getPen()->xpos, getPen()->ypos-ht);
#endif
	if (getPen()->xpos < pictureleft) pictureleft = getPen()->xpos;
	if (getPen()->xpos+wid > pictureright)
	    pictureright = getPen()->xpos+wid;
	if (getPen()->ypos-ht < picturetop) picturetop = getPen()->ypos-ht;
	if (getPen()->ypos > picturebottom) picturebottom = getPen()->ypos;
    }
}

/* For synchronization purposes, this make sure all these actions happen
   in the GUI thread but are posted as events from logo*/
#ifdef MULTITHREAD
void TurtleCanvas::logoHandle ( wxCommandEvent & e) {
    wxDC *dc;

    if(drawToPrinter)
	dc=printerDC;
    else{
	dc = new wxClientDC(turtleGraphics);
    }	
    pen_info *p;
    wxPen pen;
    switch (e.GetInt()) {
	case SPLITSCREEN:
	    turtleFrame->in_graphics_mode = 1;
	    turtleFrame->in_splitscreen = 1;
	    topsizer->Show(wxTerminal::terminal, 1);
	    topsizer->Show(turtleGraphics, 1);
	    topsizer->Show(editWindow, 0);   ////
	    topsizer->Layout();
	    wxTerminal::terminal->SetFocus();
//	    needToRefresh++;
	    wxTerminal::terminal->deferUpdate(0);
	    FinishedEvent();
	    break;
	case FULLSCREEN:
	    turtleFrame->in_graphics_mode = 1;
	    turtleFrame->in_splitscreen = 0;
	    topsizer->Show(wxTerminal::terminal, 0);
	    topsizer->Show(turtleGraphics, 1);
	    topsizer->Show(editWindow, 0);
	    topsizer->Layout();
	    wxTerminal::terminal->deferUpdate(1);
	    FinishedEvent();
	    break;
	case TEXTSCREEN:
	    turtleFrame->in_graphics_mode = 0;
	    turtleFrame->in_splitscreen = 0;
	    topsizer->Show(wxTerminal::terminal, 1);
	    topsizer->Show(turtleGraphics, 0);
	    topsizer->Show(editWindow, 0);
	    topsizer->Layout();
	    wxTerminal::terminal->SetFocus();
	    wxTerminal::terminal->deferUpdate(0);
	    FinishedEvent();
	    break;
	case CLEARSCREEN:
	    realClearScreen(dc);
	    FinishedEvent();
	    break;
	case SAVEPEN:
	    p = (pen_info *)e.GetClientData();
	    memcpy(((char *)(p)),((char *)(&turtleFrame->xgr_pen)),sizeof(pen_info));
	    FinishedEvent();
	    break;
	case RESTOREPEN:
	    p = (pen_info *)e.GetClientData();
	    memcpy(((char *)(&turtleFrame->xgr_pen)),((char *)(p)),sizeof(pen_info));
	    FinishedEvent();
	    break;
	case SETPENWIDTH:
	    int width;
	    width = (int) e.GetClientData();
	    turtleFrame->xgr_pen.pw = width;
	    FinishedEvent();
	    break;
	case GETPALETTE:
	{
	    int col;
	    col = (int) e.GetClientData();
	    wxColour colour(TurtleCanvas::colors[col+2]);
	    R = colour.Red()*256;
	    G = colour.Green()*256;
	    B = colour.Blue()*256;
	    FinishedEvent();
	}
	    break;
	case FLOODFILL:
	{
	    int * data;
	    data = (int* ) e.GetClientData();
	    realFloodFill(data[0], dc);
	    FinishedEvent();
	}
	    break;
	case GETMOUSECOORDS:
	{
	    int * data;
	    data = (int* ) e.GetClientData();
	    data[0] = mousePosition_x;
	    data[1] = mousePosition_y;
	    FinishedEvent();
	}
	    break;
	case GETMOUSEDOWN:
	{
	    int * data;
	    data = (int* ) e.GetClientData();
	    data[0] = mouse_down_left + mouse_down_middle + mouse_down_right;
	    FinishedEvent();
	}
	    break;
	case SETINFO:
	  {
	    int * data;
	    data = (int* ) e.GetClientData();
	    int type = data[0];
	    int val = data[1];
	    setInfo(type, val);
	    FinishedEvent();
	  }
	  break;
	case GETINFO:
	  {
	    int * data;
	    data = (int* ) e.GetClientData();
	    int type = data[0];
	    data [1] = getInfo(type);
	    FinishedEvent();
	  }
	  break;
	case DRAWLABEL:
	  {
	    realDrawLabel((char *)e.GetClientData(), dc);
	    FinishedEvent();
	  }
	  break;
	case KILLAPPLICATION:
	  exit(0);
	  break;
        case CATCHUP:
	    drawLine(e);
	    FinishedEvent();
	    break;
	case PRINTPICT:
	    TurtleCanvas::PrintTurtleWindow(e);
	    FinishedEvent();
	    break;
	case PRINTPREVIEWPICT:
	    TurtleCanvas::TurtlePrintPreview(e);
	    FinishedEvent();
	    break;
	case PRINTTEXT:
	    logoFrame->OnPrintText(e);
	    FinishedEvent();
	    break;
	case PRINTPREVIEWTEXT:
	    logoFrame->OnPrintTextPrev(e);
	    FinishedEvent();
	    break;
	}
	if(!drawToPrinter)
		delete dc;
}
#endif

/* A setter function for various turtle graphics properties */
void TurtleCanvas::setInfo(int type, int val){
	switch (type){
	case SCREEN_WIDTH:
		turtleFrame->screen_width = val;
		break;
	case SCREEN_HEIGHT:
		turtleFrame->screen_height = val;
		break;
	case BACK_GROUND:
		turtleFrame->back_ground = val;
		break;
	case IN_SPLITSCREEN:
		turtleFrame->in_splitscreen = val;
		break;
	case IN_GRAPHICS_MODE:
		turtleFrame->in_graphics_mode = val;
		break;
	}

}

/* A getter function for various turtle graphics properties */
int TurtleCanvas::getInfo(int type){
	switch (type){
	case SCREEN_WIDTH:
		return turtleFrame->screen_width;
		break;
	case SCREEN_HEIGHT:
		return turtleFrame->screen_height;
		break;
	case BACK_GROUND:
		return turtleFrame->back_ground;
		break;
	case IN_SPLITSCREEN:
		return turtleFrame->in_splitscreen;
		break;
	case IN_GRAPHICS_MODE:
		return turtleFrame->in_graphics_mode;
		break;
	}
	return -1;

}


void TurtleCanvas::OnPageSetup(wxCommandEvent& WXUNUSED(event))
{
    (*g_pageSetupData) = *g_printData;

    wxPageSetupDialog pageSetupDialog(this, g_pageSetupData);
    pageSetupDialog.ShowModal();

    (*g_printData) = pageSetupDialog.GetPageSetupDialogData().GetPrintData();
    (*g_pageSetupData) = pageSetupDialog.GetPageSetupDialogData();
}


void TurtleCanvas::PrintTurtleWindow(wxCommandEvent& WXUNUSED(event)) {
    wxPrintDialogData printDialogData(* g_printData);

    wxPrinter printer(& printDialogData);
    TurtleWindowPrintout printout(_T("Turtle Graphics"));
    if (!printer.Print(turtleFrame, &printout, true /*prompt*/))
    {
	if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
	    wxMessageBox(_T("There was a problem printing.\nPerhaps your current printer is not set correctly?"), _T("Printing"), wxOK);
	else
	    wxMessageBox(_T("Printing Canceled"), _T("Printing"), wxOK);
    } else {
	(*g_printData) = printer.GetPrintDialogData().GetPrintData();
    }
}


void TurtleCanvas::TurtlePrintPreview(wxCommandEvent& WXUNUSED(event)) {
    // Pass two printout objects: for preview, and possible printing.
    wxPrintDialogData printDialogData(* g_printData);
    wxPrintPreview *preview = new wxPrintPreview(new TurtleWindowPrintout, new TurtleWindowPrintout, & printDialogData);
    if (!preview->Ok())
    {
        delete preview;
        wxMessageBox(_T("There was a problem previewing.\nPerhaps your current printer is not set correctly?"), _T("Previewing"), wxOK);
        return;
    }
    wxPreviewFrame *frame = new wxPreviewFrame(preview, this, _T("Turtle Graphics Preview"), wxPoint(100, 100), wxSize(600, 650));
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show();
}

// ----------------------------------------------------------------------------
// TurtleFrame Class
// ----------------------------------------------------------------------------



BEGIN_EVENT_TABLE(TurtleFrame, wxFrame)
END_EVENT_TABLE()

// frame constructor
TurtleFrame::TurtleFrame(wxFrame * parent, const wxString& title, const wxPoint& pos, const wxSize& size)
  : wxFrame(/*(wxFrame *)NULL*/ parent, -1, title, pos, size,
                 wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
   m_canvas = new TurtleCanvas( this );
}

void TurtleFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(TRUE);
}

TurtleCanvas * TurtleFrame::GetCanvas() {
  return m_canvas;
}


// ----------------------------------------------------------------------------
// Functions called from the interpreter thread
// ----------------------------------------------------------------------------

extern "C" void nop() {
}

extern "C" void set_palette(int color, unsigned int r, unsigned int g, unsigned int b){
	TurtleCanvas::colors[color+2] = wxColour(r/256,g/256,b/256);
}

extern "C" void get_palette(int color, unsigned int *r, unsigned int *g, unsigned int *b){
#ifdef MULTITHREAD
    if (drawToWindow) {
	wxColour colour(TurtleCanvas::colors[color+2]);
	*r = colour.Red()*256;
	*g = colour.Green()*256;
	*b = colour.Blue()*256;
    } else {
	wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
	event.SetInt(GETPALETTE);
	event.SetClientData((void *)color);
	alreadyDone = 0;
	turtleGraphics->AddPendingEvent(event);
	TurtleCanvas::WaitForEvent();
	*r = R;
	*g = G;
	*b = B;
    }
#else
    wxColour colour(TurtleCanvas::colors[color+2]);
    *r = colour.Red()*256;
    *g = colour.Green()*256;
    *b = colour.Blue()*256;
#endif
}

extern "C" void save_pen(pen_info *p) {
#ifdef MULTITHREAD
    if (drawToPrinter || drawToWindow)
	memcpy(((char *)(p)),((char *)(&turtleFrame->xgr_pen)),
	       sizeof(pen_info));
    else {
	wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
	event.SetInt(SAVEPEN);
	event.SetClientData(p);
	alreadyDone = 0;
	turtleGraphics->AddPendingEvent(event);
	TurtleCanvas::WaitForEvent();      
    }
#else
    memcpy(((char *)(p)),((char *)(&turtleFrame->xgr_pen)),
	   sizeof(pen_info));
#endif
}

extern "C" void restore_pen(pen_info *p) {
#ifdef MULTITHREAD
    if (drawToPrinter || drawToWindow)
	memcpy(((char *)(&turtleFrame->xgr_pen)),((char *)(p)),
	       sizeof(pen_info));
    else {
	wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
	event.SetInt(RESTOREPEN);
	event.SetClientData(p);
	alreadyDone = 0;
	turtleGraphics->AddPendingEvent(event);	
	TurtleCanvas::WaitForEvent();
    }
#else
    memcpy(((char *)(&turtleFrame->xgr_pen)),((char *)(p)),
	   sizeof(pen_info));
#endif
}

extern "C" void set_pen_patter(){
  nop();
}

extern "C" void logofill() {
#ifdef MULTITHREAD
    int data[2];
    //extern int turtle_shown;
    //extern void draw_turtle();
#endif

    if (drawToPrinter)
	TurtleCanvas::realFloodFill(turtleFrame->xgr_pen.color, printerDC);
    else if (drawToWindow)
	TurtleCanvas::realFloodFill(turtleFrame->xgr_pen.color, windowDC);
    else {
#ifdef MULTITHREAD
	if (1 || turtle_shown) { /* wait for turtle to disappear */
	     wxCommandEvent e(wxEVT_LOGO_CUSTOM_COMMAND);
	     e.SetInt(CATCHUP);
	     e.SetClientData((void *)data);
	     alreadyDone = 0;
	     turtleGraphics->AddPendingEvent(e);	     
	     TurtleCanvas::WaitForEvent();
	}

	data[0] = turtleFrame->xgr_pen.color;
	data[1] = turtleFrame->xgr_pen.pw;
	wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
	event.SetInt(FLOODFILL);
	event.SetClientData((void *)data);
	alreadyDone = 0;
	turtleGraphics->AddPendingEvent(event);
	TurtleCanvas::WaitForEvent();
#else
	wxDC *dc = new wxClientDC(turtleGraphics);
	TurtleCanvas::realFloodFill(turtleFrame->xgr_pen.color, dc);
	delete dc;
#endif
    }

}

/* Clear the turtle graphics screen, and put in splitscreen if we are
   currently in full text mode */
extern "C" void wx_clear() {
    if (drawToPrinter)
	TurtleCanvas::realClearScreen(printerDC);
    else if (drawToWindow)
	TurtleCanvas::realClearScreen(windowDC);
    else {
      // finish drawing lines first
#ifdef MULTITHREAD
      wxCommandEvent e(wxEVT_LOGO_CUSTOM_COMMAND);
      e.SetInt(CATCHUP);
      alreadyDone = 0;      
      turtleGraphics->AddPendingEvent(e);
      TurtleCanvas::WaitForEvent();
      
      wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
      event.SetInt(CLEARSCREEN);
      alreadyDone = 0;
      turtleGraphics->AddPendingEvent(event);
      TurtleCanvas::WaitForEvent();
#else
      wxDC *dc = new wxClientDC(turtleGraphics);
      TurtleCanvas::realClearScreen(dc);
      delete dc;
#endif

      if(!TurtleFrame::in_graphics_mode)
	wxSplitScreen();
    }
    return;
}

extern char record_buffer[];

extern "C" void wxPrepare(){
    if (drawToPrinter || drawToWindow) {
      return;
    }
    
    if(!turtleFrame->in_graphics_mode) {
	wxSplitScreen();
    }
    if(!prepared){
	record_buffer[sizeof(int)] = 0;
        wx_clear();
	prepared = 1;
    }
    return;
}

/* Have turtle graphics draw the given line */
extern "C" void wxDrawLine(int x1, int y1, int x2, int y2, int vis){

    static int numLines = 0;
#ifdef MULTITHREAD
    tMut.Lock();
#endif
    if (!drawToPrinter && !drawToWindow) {
#ifdef MULTITHREAD
      while (lines.size() > MAX_LINES_BUFFERED) {
	  lineCond.Wait();
      }
#endif
	if (numLines == LINEPAUSE) {
#ifdef MULTITHREAD
	  lineCond.WaitTimeout(1);
#else
	  wxMilliSleep(1);
#endif
	  numLines = 0;
	}
	else 
	  numLines++;

    }


    
    wxDC *dc;

    struct line l;
    l.x1 = x1;
    l.y1 = y1;
    l.x2 = x2;
    l.y2 = y2;
    l.vis = vis;
    l.color = turtleFrame->xgr_pen.color;
    l.pm = turtleFrame->xgr_pen.pen_mode;
    l.pw = turtleFrame->xgr_pen.pw;
    if (drawToPrinter)
	TurtleCanvas::drawOneLine(&l, printerDC);
    else if (drawToWindow)
	TurtleCanvas::drawOneLine(&l, windowDC);
    else {
#ifdef MULTITHREAD
	lines.push_back(l);
	  if (!putInQueue) {
	  putInQueue = 1;
	  }
#else
	dc = new wxClientDC(turtleGraphics);
	TurtleCanvas::drawOneLine(&l, dc);
	delete dc;
#endif
      // NOTE: DO NOT FORGET
      // YOU CAN NOT BE HOLDING A LOCK THAT THE OTHER THREAD IS WAITING ON
      // AND DO A POST EVENT!!!! IT IS DEADLOCK
      // BEWARE!!!!!!!!!!!!!!!!!!!!!!!!!!!
    }
#ifdef MULTITHREAD
        tMut.Unlock();
#endif
    return;
}

/* Set the pen width.  Notice this only takes one number, because wx only
   allows us to set the width and not the pen height */
extern "C" void wxSetPenWidth(int width){
#ifdef MULTITHREAD
    if (drawToPrinter || drawToWindow)
	turtleFrame->xgr_pen.pw = width;
    else {
	wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
	event.SetInt(SETPENWIDTH);
	event.SetClientData((void *)width);
	alreadyDone = 0;
	turtleGraphics->AddPendingEvent(event);
	TurtleCanvas::WaitForEvent();
    }
#else
    turtleFrame->xgr_pen.pw = width;
#endif
}

extern enum s_md {SCREEN_TEXT, SCREEN_SPLIT, SCREEN_FULL} screen_mode;

/* Put the logoframe into splitscreen mode*/
extern "C" void wxSplitScreen(){
#ifdef MULTITHREAD
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
  event.SetInt(SPLITSCREEN);
  alreadyDone = 0;
  turtleGraphics->AddPendingEvent(event);  
  TurtleCanvas::WaitForEvent();
#else
    turtleFrame->in_graphics_mode = 1;
    turtleFrame->in_splitscreen = 1;
    topsizer->Show(wxTerminal::terminal, 1);
    topsizer->Show(turtleGraphics, 1);
    topsizer->Show(editWindow, 0);   
    topsizer->Layout();
    wxTerminal::terminal->SetFocus();
    wxTerminal::terminal->deferUpdate(0);
#endif
    screen_mode = SCREEN_SPLIT;
}

/* Put the logoframe into full screen mode */
extern "C" void wxFullScreen(){
#ifdef MULTITHREAD
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
  event.SetInt(FULLSCREEN);
  alreadyDone = 0;
  turtleGraphics->AddPendingEvent(event);
  TurtleCanvas::WaitForEvent();
#else
    turtleFrame->in_graphics_mode = 1;
    turtleFrame->in_splitscreen = 0;
    topsizer->Show(wxTerminal::terminal, 0);
    topsizer->Show(turtleGraphics, 1);
    topsizer->Show(editWindow, 0);
    topsizer->Layout();
    wxTerminal::terminal->deferUpdate(1);
#endif
    screen_mode = SCREEN_FULL;
}

/* Put the logoframe into text screen mode*/
extern "C" void wxTextScreen(){
#ifdef MULTITHREAD
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
  event.SetInt(TEXTSCREEN);
  alreadyDone = 0;
  turtleGraphics->AddPendingEvent(event);  
  TurtleCanvas::WaitForEvent();
#else
  turtleFrame->in_graphics_mode = 0;
  turtleFrame->in_splitscreen = 0;
  topsizer->Show(wxTerminal::terminal, 1);
  topsizer->Show(turtleGraphics, 0);
  topsizer->Show(editWindow, 0);
  topsizer->Layout();
  wxTerminal::terminal->SetFocus();
  wxTerminal::terminal->deferUpdate(0);
#endif
  screen_mode = SCREEN_TEXT;
}

extern "C" void wxlPrintPict(){
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
#ifdef MULTITHREAD
  event.SetInt(PRINTPICT);
  alreadyDone = 0;
  turtleGraphics->AddPendingEvent(event);
  TurtleCanvas::WaitForEvent();
#else
  turtleGraphics->PrintTurtleWindow(event);
#endif
}

extern "C" void wxlPrintPreviewPict(){
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
#ifdef MULTITHREAD
  event.SetInt(PRINTPREVIEWPICT);
  alreadyDone = 0;
  turtleGraphics->AddPendingEvent(event);
  TurtleCanvas::WaitForEvent();
#else
  turtleGraphics->TurtlePrintPreview(event);
#endif
}

extern "C" void wxlPrintText(){
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
#ifdef MULTITHREAD
  event.SetInt(PRINTTEXT);
  alreadyDone = 0;
  turtleGraphics->AddPendingEvent(event);
  TurtleCanvas::WaitForEvent();
#else
  logoFrame->OnPrintText(event);
#endif
}

extern "C" void wxlPrintPreviewText(){
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
#ifdef MULTITHREAD
  event.SetInt(PRINTPREVIEWTEXT);
  alreadyDone = 0;
  turtleGraphics->AddPendingEvent(event);
  TurtleCanvas::WaitForEvent();
#else
  logoFrame->OnPrintTextPrev(event);
#endif
}

void getMousePosition (int * x, int * y) {
#ifdef MULTITHREAD
  int data[2];
  data[0] = 0;
  data[1] = 0;
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
  event.SetInt(GETMOUSECOORDS);
  event.SetClientData((void *)data);
  alreadyDone = 0;
  turtleGraphics->AddPendingEvent(event);
  TurtleCanvas::WaitForEvent();
  
  *x = data[0] - wxGetInfo(SCREEN_WIDTH)/2;
  *y = wxGetInfo(SCREEN_HEIGHT)/2 - data[1];
#else
  *x = TurtleCanvas::mousePosition_x - wxGetInfo(SCREEN_WIDTH)/2;
  *y = wxGetInfo(SCREEN_HEIGHT)/2 - TurtleCanvas::mousePosition_y;
#endif
}

extern "C" int wxGetMouseX() {
  int x, y;
  getMousePosition(&x,&y);
  return x;
}
extern "C" int wxGetMouseY() {
  int x, y;
  getMousePosition(&x,&y);
  return y;
}
extern "C" int wxGetButton () {
#ifdef MULTITHREAD
  int data[1];
  data[0] = 0;
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
  event.SetInt(GETMOUSEDOWN);
  event.SetClientData((void *)data);
  alreadyDone = 0;
  turtleGraphics->AddPendingEvent(event);
  TurtleCanvas::WaitForEvent();
  return data[0];
#else
    return TurtleCanvas::mouse_down_left + TurtleCanvas::mouse_down_middle
	    + TurtleCanvas::mouse_down_right;
#endif
}

/* Show the text editor and have it load the given file */
extern "C" int wxEditFile(char * f){
  file = f;
  alreadyDone = 0;
#ifdef MULTITHREAD
    turtleGraphics->AddPendingEvent(editEvent);
#else
  turtleGraphics->ProcessEvent(editEvent);
#endif
#ifdef MULTITHREAD
   editMut.Lock();
   editCond.Wait();
   editMut.Unlock();
#endif
  return editWindow->doSave;
}

extern "C" pen_info* getPen(){
	return &turtleFrame->xgr_pen;	
}

extern "C" void wxSetInfo(int type, int val) {
#ifdef MULTITHREAD
    if (drawToPrinter || drawToWindow)
	TurtleCanvas::setInfo(type, val);
    else {
	int data[2];
	data[0] = type;
	data[1] = val;
	wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
	event.SetInt(SETINFO);
	event.SetClientData((void *)data);
	alreadyDone = 0;
	turtleGraphics->AddPendingEvent(event);	
	TurtleCanvas::WaitForEvent();
    }
#else
    TurtleCanvas::setInfo(type, val);
#endif
}

extern "C" int wxGetInfo(int type) {
    return TurtleCanvas::getInfo(type);
}

extern "C" void wxLabel(char * string) {
    if (drawToPrinter) 
	TurtleCanvas::realDrawLabel(string, printerDC);
    else if (drawToWindow) 
	TurtleCanvas::realDrawLabel(string, windowDC);
    else {
#ifdef MULTITHREAD
	wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
	event.SetInt(DRAWLABEL);
	event.SetClientData((void *)string);
	alreadyDone = 0;
	turtleGraphics->AddPendingEvent(event);	
	TurtleCanvas::WaitForEvent();
#else
	wxDC *dc = new wxClientDC(turtleGraphics);
	TurtleCanvas::realDrawLabel(string, dc);
	delete dc;	
#endif
    }
}


void TurtleCanvas::exitApplication()
{
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
  event.SetInt(KILLAPPLICATION);
  alreadyDone = 0;
#ifdef MULTITHREAD
  turtleGraphics->AddPendingEvent(event);
#else
  turtleGraphics->ProcessEvent(event);
#endif
}




// ----------------------------------------------------------------------------
// TurtleWindowPrintout
// ----------------------------------------------------------------------------

extern int turtle_shown;

bool TurtleWindowPrintout::OnPrintPage(int page)
{
    int w,h; 
    wxDC *dc=GetDC();
    int oldshown = turtle_shown;

    if (!dc) return false;

#ifndef __WXMAC__
  //using a memdc for printer here
    /*
     *  Create our bitmap for copying
     */
  
    dc->GetSize(&w, &h);
 
    wxMemoryDC *printMemDC = new wxMemoryDC();
    wxBitmap *printBitmap = new wxBitmap(w,h);
    printMemDC->SelectObject(*printBitmap);

    printerDC = printMemDC;
#else
    printerDC = dc;
#endif

#if 0
//#ifndef __WXMAC__   /* needed for wxWidgets 2.6 */
    int maxX = pictureright - pictureleft;
    int maxY = picturebottom - picturetop;

    FitThisSizeToPageMargins(wxSize(maxX, maxY), *g_pageSetupData);
    wxRect fitRect = GetLogicalPageMarginsRect(*g_pageSetupData);

    wxCoord xoff = (fitRect.width - maxX) / 2 - pictureleft;
    //wxCoord xoff = (fitRect.width - maxX) / 2;
    wxCoord yoff = (fitRect.height - maxY) / 2 - picturetop;
    //wxCoord yoff = (fitRect.height - maxY) / 2;
    OffsetLogicalOrigin(xoff, yoff);
#else
    float maxX = pictureright - pictureleft;
    float maxY = picturebottom - picturetop;

    // Let's have at least 50 device units margin
    float marginX = 50;
    float marginY = 50;

    // Get the size of the DC in pixels
    
#ifdef __WXMAC__
     dc->GetSize(&w, &h);   //this is now done above with memDC
#endif

    // Calculate a suitable scaling factor
    float scaleX=(float)((w-2*marginX)/maxX);
    float scaleY=(float)((h-2*marginY)/maxY);

    // Use x or y scaling factor, whichever fits on the DC
    float actualScale = wxMin(scaleX,scaleY);

    // Calculate the position on the DC for centring the graphic
    float posX = (float)((w - actualScale*maxX)/2)-pictureleft*actualScale;
    float posY = (float)((h - actualScale*maxY)/2)-picturetop*actualScale;

    // Set the scale and origin
    printerDC->SetUserScale(actualScale, actualScale);
    printerDC->SetDeviceOrigin( (long)posX, (long)posY );
#endif

    drawToPrinter = 1;
    turtle_shown = 0;
    redraw_graphics();
    turtle_shown = oldshown;
    drawToPrinter = 0;

#ifndef __WXMAC__
    //now print the bitmap to the dc
    //actualPrinterDC->Blit(0,0,w,h,printMemDC,0,0); //appears not to work
    dc->DrawBitmap(*printBitmap, 0, 0);

    //delete bitmap and memorydc
    delete printBitmap;
    delete printMemDC;
#endif

    return true;
}

bool TurtleWindowPrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument(startPage, endPage))
        return false;
	
    return true;
}

void TurtleWindowPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 0;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool TurtleWindowPrintout::HasPage(int pageNum)
{
    return (pageNum == 1);
}
