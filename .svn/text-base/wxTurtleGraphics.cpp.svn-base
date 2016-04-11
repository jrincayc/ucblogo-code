#include <stdio.h>
#include "wxTurtleGraphics.h"
#include "TextEditor.h"
#include <wx/stdpaths.h>
#define WX_TURTLEGRAPHICS_CPP 1

using namespace std;


#define SCREEN_WIDTH		1
#define SCREEN_HEIGHT		2
#define	BACK_GROUND		3
#define	IN_SPLITSCREEN		4
#define	IN_GRAPHICS_MODE	5

// ----------------------------------------------------------------------------
// Custom Events
// ----------------------------------------------------------------------------

  /* The edit event */
#if 0
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
#endif
 
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

float fillScale;

pen_info p;
int TurtleFrame::back_ground = 0;
int TurtleFrame::screen_height = 0; 
int TurtleFrame::screen_width = 0;
int TurtleFrame::in_graphics_mode = 0;
int TurtleFrame::in_splitscreen = 0;
pen_info TurtleFrame::xgr_pen = p;

int drawToWindow = 0;	// for redraw_graphics from gui "thread"
wxDC *windowDC = 0;

wxMemoryDC *m_memDC;
#define USE_MEMDC 1

int pictureleft = 0, pictureright = 0, picturetop = 0, picturebottom = 0;
// Keep track of max range for printing.


int TurtleCanvas::mousePosition_x;
int TurtleCanvas::mousePosition_y;
int TurtleCanvas::clickPosition_x;
int TurtleCanvas::clickPosition_y;

int TurtleCanvas::mouse_down_left;
int TurtleCanvas::mouse_down_middle;
int TurtleCanvas::mouse_down_right;
int TurtleCanvas::mouse_down_last;

wxColour TurtleCanvas::colors[NUMCOLORS+SPECIAL_COLORS];

int R, G, B;


// Global print data, to remember settings during the session
wxPrintData *g_printData = (wxPrintData*) NULL ;

// Global page setup data
wxPageSetupDialogData* g_pageSetupData = (wxPageSetupDialogData*) NULL;

// Used for printing
TurtleWindowPrintout *turtlePrintout;
extern "C" int drawToPrinter;
wxDC *printerDC;
wxBitmap *tempBitmap;

// have already called prepareToDraw
int prepared = 0;
TurtleFrame *turtleFrame;
int turtleIndex = 0;
int putInQueue = 0;

#if 0
wxCommandEvent editEvent = wxCommandEvent(wxEVT_EDIT_CUSTOM_COMMAND);
#endif
char * file;

// the location of the turtle
extern "C" int turtlePosition_x;
extern "C" int turtlePosition_y;

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
		wxdprintf("Pen color = %s ",
			  TurtleCanvas::colors[l.color+SPECIAL_COLORS]);
//		dc.SetPen( wxPen(TurtleCanvas::colors[l.color+SPECIAL_COLORS],
				    1, wxSOLID) );
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
#if 0
EVT_EDIT_CUSTOM_COMMAND(-1, TurtleCanvas::editCall)
#endif
EVT_SET_FOCUS( TurtleCanvas::OnFocus)
EVT_KILL_FOCUS(TurtleCanvas::LoseFocus)
EVT_LEFT_DOWN(TurtleCanvas::OnLeftDown)
EVT_LEFT_UP(TurtleCanvas::OnLeftUp)
EVT_MIDDLE_DOWN(TurtleCanvas::OnMiddleDown)
EVT_MIDDLE_UP(TurtleCanvas::OnMiddleUp)
EVT_RIGHT_DOWN(TurtleCanvas::OnRightDown)
EVT_RIGHT_UP(TurtleCanvas::OnRightUp)
EVT_ERASE_BACKGROUND(TurtleCanvas::OnEraseBackGround)
EVT_CHAR(TurtleCanvas::OnChar)
END_EVENT_TABLE()

/* The TurtleCanvas class is what the turtle is drawn on */
TurtleCanvas::TurtleCanvas(wxFrame *parent)
        : wxWindow(parent, -1, wxDefaultPosition, wxDefaultSize,
                           wxNO_FULL_REPAINT_ON_RESIZE)
{
  m_owner = parent;
  m_show = Show_Lines;
  m_clip = FALSE;
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

  mousePosition_x = 0;
  mousePosition_y = 0;
  clickPosition_x = 0;
  clickPosition_y = 0;
  mouse_down_left = 0;
  mouse_down_middle = 0;
  mouse_down_right = 0;
  mouse_down_last = 0;

  // initialize the TurtleCanvas::colors
  int i;
    TurtleCanvas::colors[SPECIAL_COLORS] = wxColour(0, 0, 0);
    TurtleCanvas::colors[SPECIAL_COLORS+1] = wxColour(0, 0, 255);
    TurtleCanvas::colors[SPECIAL_COLORS+2] = wxColour(0, 255, 0);
    TurtleCanvas::colors[SPECIAL_COLORS+3] = wxColour(0, 255, 255);
    TurtleCanvas::colors[SPECIAL_COLORS+4] = wxColour(255, 0, 0);
    TurtleCanvas::colors[SPECIAL_COLORS+5] = wxColour(255, 0, 255);
    TurtleCanvas::colors[SPECIAL_COLORS+6] = wxColour(255, 255, 0);
    TurtleCanvas::colors[SPECIAL_COLORS+7] = wxColour(255, 255, 255);
    TurtleCanvas::colors[SPECIAL_COLORS+8] = wxColour(155, 96, 59);
    TurtleCanvas::colors[SPECIAL_COLORS+9] = wxColour(197, 136, 18);
    TurtleCanvas::colors[SPECIAL_COLORS+10] = wxColour(100, 162, 64);
    TurtleCanvas::colors[SPECIAL_COLORS+11] = wxColour(120, 187, 187);
    TurtleCanvas::colors[SPECIAL_COLORS+12] = wxColour(255, 149, 119);
    TurtleCanvas::colors[SPECIAL_COLORS+13] = wxColour(144, 113, 208);
    TurtleCanvas::colors[SPECIAL_COLORS+14] = wxColour(255, 163, 0);
    TurtleCanvas::colors[SPECIAL_COLORS+15] = wxColour(183, 183, 183);
  for(i=SPECIAL_COLORS+16;i<NUMCOLORS+SPECIAL_COLORS;i++){
    TurtleCanvas::colors[i] =
        TurtleCanvas::colors[(i-SPECIAL_COLORS)%NUMINITCOLORS+SPECIAL_COLORS];
  }
	
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


#if USE_MEMDC

  m_memDC=new wxMemoryDC();
  
  //make a bitmap the maximum size of the screen (the monitor, not the drawing area)
  int m_w,m_h; //monitor screen width and height
  wxDisplaySize(&m_w,&m_h);
  m_bitmap = new wxBitmap(m_w, m_h);

  PrepareDC(*m_memDC);
  wxBrush myBrush(TurtleCanvas::colors[turtleFrame->back_ground
				       +SPECIAL_COLORS],wxSOLID);
  m_memDC->SelectObject(*m_bitmap);
  m_memDC->SetBackgroundMode( wxSOLID );
  m_memDC->SetBackground( myBrush );
  m_memDC->Clear();

  //compute origin based on  screen size vs monitor size
  //(BIGWIDTH - SMALLWIDTH)/2

  m_memDC->SetDeviceOrigin( (m_w - screen_width)/2, 
			    (m_h - screen_height)/2 );

  wxFont f(FONT_CFG(wx_font_face, wx_font_size));
  m_memDC->SetFont(f);

  
#endif

  
}

TurtleCanvas::~TurtleCanvas() {
  turtleGraphics = 0;
}


void TurtleCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
  wxdprintf("OnPaint starts\n");

  wxPaintDC dc(this);
  OnDraw(dc);
  wxdprintf("OnPaint ends\n");
}

void TurtleCanvas::OnSize(wxSizeEvent& event) {

  wxdprintf("OnSize starts\n");

  int m_w,m_h;
  int screen_width, screen_height;

  wxDisplaySize(&m_w,&m_h);

  logoFrame->GetSize(&screen_width, &screen_height);
  setInfo(SCREEN_WIDTH, screen_width);
  setInfo(SCREEN_HEIGHT, screen_height);

#if USE_MEMDC  
  m_memDC->SetDeviceOrigin( (m_w - screen_width)/2, 
			    (m_h - screen_height)/2 );
  
#endif

  wxClientDC dc(this);

  OnDraw(dc);

  wxdprintf("OnSize ends\n");
}


void TurtleCanvas::OnDraw(wxDC &dc) {
  int x, y;

  //dc.DestroyClippingRegion();
  
  GetSize(&x, &y);
  
#if USE_MEMDC
  dc.Blit(0,0,x,y,m_memDC,0,0);
  return;
#endif
	
  int unset_windowDC = 0;
  if(windowDC == 0) {   //OnSize may be triggered multiple times...
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


void TurtleCanvas::drawOneLine(struct line *l, wxDC *dc) {
    wxPen myPen;
    wxColour xorColor;

    if (l->pm==PEN_ERASE) {
	myPen = wxPen(TurtleCanvas::colors[turtleFrame->back_ground+
					    SPECIAL_COLORS],
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
	myPen = wxPen(TurtleCanvas::colors[l->color+SPECIAL_COLORS],
			l->pw, wxSOLID);
    }
    dc->SetPen(myPen);

    if(l->pm==PEN_REVERSE){
	dc->SetLogicalFunction(wxXOR);
    } else {  
	dc->SetLogicalFunction(wxCOPY);
    }
    if(l->vis) {
      dc->DrawLine(l->x1,l->y1,l->x2,l->y2);
      dc->DrawPoint(l->x2,l->y2);	/* draw endpoint */
	if (!drawToPrinter && !drawToWindow) {
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

extern "C" int turtle_shown;
extern "C" void draw_turtle();
extern int editor_active;  //from TextEditor.cpp

void TurtleCanvas::editCall(){ 
  editor_active = 1;  
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
  editWindow->Load(wxString(file,wxConvUTF8));
  //need to busy wait and handle events...
  while(editor_active) {
    if(check_wx_stop(1))
      break; 
    wxMilliSleep(10);
  }
}

extern "C" void mouse_down(int);

void
TurtleCanvas::OnLeftDown(wxMouseEvent& event) {
  mouse_down_left = 1;
  mouse_down_last = 1;
  clickPosition_x = mousePosition_x;
  clickPosition_y = mousePosition_y;
    mouse_down(0);
  event.Skip(); //allow native handler to set focus
}
void TurtleCanvas::OnLeftUp(wxMouseEvent& event) {
  mouse_down_left = 0;
  event.Skip(); //allow native handler to set focus
}
void TurtleCanvas::OnMiddleDown(wxMouseEvent& event) {
  mouse_down_middle = 3;
  mouse_down_last = 3;
  clickPosition_x = mousePosition_x;
  clickPosition_y = mousePosition_y;
    mouse_down(0);
  event.Skip(); //allow native handler to set focus
}
void TurtleCanvas::OnMiddleUp(wxMouseEvent& event) {
  mouse_down_middle = 0;
  event.Skip(); //allow native handler to set focus
}
void TurtleCanvas::OnRightDown(wxMouseEvent& event) {
  mouse_down_right = 2;
  mouse_down_last = 2;
  clickPosition_x = mousePosition_x;
  clickPosition_y = mousePosition_y;
    mouse_down(0);
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


extern "C" pen_info* getPen();

void TurtleCanvas::realClearScreen(wxDC *dc) {
    wxBrush myBrush(TurtleCanvas::colors[turtleFrame->back_ground+
					    SPECIAL_COLORS],wxSOLID);
    if(drawToPrinter && turtleFrame->back_ground==0){
	    myBrush.SetColour(_T("white"));
    }
    dc->SetBackgroundMode( wxSOLID );
    dc->SetBackground( myBrush );
    dc->Clear();
    if (!drawToPrinter && !drawToWindow) {
	pictureleft = pictureright = getInfo(SCREEN_WIDTH)/2;
	picturetop = picturebottom = getInfo(SCREEN_HEIGHT)/2;
    }
}

void TurtleCanvas::realFloodFill(int color, wxDC *dc) {
    wxColour c;
//fprintf(stderr, "realFloodFill: (x,y): (%d, %d)\n", turtlePosition_x, turtlePosition_y);
    dc->GetPixel(turtlePosition_x, turtlePosition_y, &c);
    wxBrush brush(TurtleCanvas::colors[color+SPECIAL_COLORS]);

    dc->SetBrush(brush);
    if (drawToPrinter) {
      dc->SetUserScale(1.0, 1.0);
      dc->FloodFill((long)(turtlePosition_x*fillScale),
		    (long)(turtlePosition_y*fillScale) , c);
      dc->SetUserScale(fillScale, fillScale);
    } else {
      dc->FloodFill(turtlePosition_x, turtlePosition_y , c);
    }


//    dc->FloodFill(dc->LogicalToDeviceX(turtlePosition_x), dc->LogicalToDeviceY(turtlePosition_y) , c);

}

void TurtleCanvas::realdoFilled(int fillcolor, int count,
				    struct mypoint *points, wxDC *dc) {
    wxPen myPen;
    wxPoint *wxpoints = (wxPoint*)malloc(count*sizeof(wxPoint));
    wxPoint *wxptr;
    struct mypoint *ptr;

    for (wxptr = wxpoints, ptr = points; ptr < &points[count];
	    wxptr++, ptr++) {
	 wxptr->x = ptr->x;
	 wxptr->y = ptr->y;
    }

    if(drawToPrinter && turtleFrame->back_ground==0 &&
		turtleFrame->xgr_pen.color==7){
	myPen = wxPen( wxT("black"), turtleFrame->xgr_pen.pw, wxSOLID);
    } else {
	myPen = wxPen(colors[turtleFrame->xgr_pen.color+SPECIAL_COLORS],
			turtleFrame->xgr_pen.pw, wxSOLID);
    }
    dc->SetPen(myPen);
    wxBrush brush(TurtleCanvas::colors[fillcolor+SPECIAL_COLORS], wxSOLID);

    dc->SetBrush(brush);
    dc->DrawPolygon(count, wxpoints);
    free(wxpoints);

}

extern "C" FLONUM y_scale;

extern "C" void wx_get_label_size(int *w, int *h) {
    /* returns size in pixels; converted to turtle steps in wxterm.c */
  int descent, extlead;
  m_memDC->GetTextExtent("M", w, h, &descent, &extlead);  
}

extern "C" void wx_adjust_label_height() {
    //take the font name, change the size based on scrunch
    int px_height = y_scale * label_height;
    int descent, extlead; 
    int cw,ch;

    wxFont label_font = m_memDC->GetFont();
    //now, initial guess for pt size is height / 1.5
    int font_size = px_height * 3 / 2;
    label_font.SetPointSize(font_size);

    m_memDC->SetFont(label_font);	
    m_memDC->GetTextExtent("M", &cw, &ch, &descent, &extlead);

    //now... first figure out whether we undershot or overshot...
    //this determines which direction to change the size

    int tmp_height = ch;
    wxFont tmp_font = label_font;
    if(tmp_height < px_height) {
        //increase the point size until we get two consecutive font sizes
        //where one is below px_height and one is above px_height
        int expected;
        while(tmp_height < px_height) {
            expected = tmp_font.GetPointSize() + 1;
            while(tmp_font.GetPointSize() != expected && expected <= 100){
	  	expected++;
		tmp_font.SetPointSize(expected);
	    }
	    if (expected == 100) break;
	    m_memDC->SetFont(tmp_font);	
	    m_memDC->GetTextExtent("M", &cw, &tmp_height, &descent, &extlead);

	    if(tmp_height >= px_height) break;

	    label_font.SetPointSize(expected);
	    ch = tmp_height;
	}       	
    }
    else {
        //same as above, but decrease instead
        int expected;
        while(tmp_height > px_height) {
            expected = tmp_font.GetPointSize() - 1;
            while(tmp_font.GetPointSize() != expected && expected >= 2){
	  	expected--;
		tmp_font.SetPointSize(expected);
	    }
	    if (expected == 2) break;
	    m_memDC->SetFont(tmp_font);	
	    m_memDC->GetTextExtent("M", &cw, &tmp_height, &descent, &extlead);

	    if(tmp_height <= px_height) break;

	    label_font.SetPointSize(expected);
	    ch = tmp_height;
	}
    }
    //now we have two fonts and two heights, we pick the closest one!
    int curr_diff = ch - px_height;
    int tmp_diff = tmp_height - px_height;
    if(curr_diff < 0) curr_diff = -curr_diff;
    if(tmp_diff < 0) tmp_diff = -tmp_diff;

//    fprintf(stderr, "ph: %d, cpt: %d, ch: %d, tpt: %d, th: %d \n", px_height, label_font.GetPointSize(), ch, tmp_font.GetPointSize(), tmp_height);
    if(curr_diff < tmp_diff) {
        m_memDC->SetFont(label_font);
    }
    else if(curr_diff > tmp_diff) {
        m_memDC->SetFont(tmp_font);
    }
    else {
        // if difference are the same, pick the smaller one
        if(ch < tmp_height) {
	    m_memDC->SetFont(label_font);
        }	
	else {
  	    m_memDC->SetFont(tmp_font);
	}
    }
}

bool TurtleCanvas::SetFont(const wxFont &f) {
    m_memDC->SetFont(f);
    wx_adjust_label_height();
    
    return TRUE;
}

void TurtleCanvas::realDrawLabel(char *data, wxDC *dc) {
    wxString s(data, wxConvUTF8);
    wxCoord wid, ht;
	
    dc->GetTextExtent(s, &wid, &ht);
    dc->SetBackgroundMode(wxTRANSPARENT);
    if (turtleFrame->back_ground == 0 && drawToPrinter) {
	dc->SetTextBackground(TurtleCanvas::colors[SPECIAL_COLORS+7]);
	if (turtleFrame->xgr_pen.color == 7)
	    dc->SetTextForeground(TurtleCanvas::colors[SPECIAL_COLORS+0]);
	else
	    dc->SetTextForeground(TurtleCanvas::colors
				  [turtleFrame->xgr_pen.color+SPECIAL_COLORS]);
    } else {
	dc->SetTextBackground(TurtleCanvas::colors[turtleFrame->back_ground+
						    SPECIAL_COLORS]);
	dc->SetTextForeground(TurtleCanvas::colors[turtleFrame->xgr_pen.color+
						    SPECIAL_COLORS]);
    }
    dc->DrawText(s, getPen()->xpos, getPen()->ypos-ht);
    if (!drawToPrinter) {
	if (getPen()->xpos < pictureleft) pictureleft = getPen()->xpos;
	if (getPen()->xpos+wid > pictureright)
	    pictureright = getPen()->xpos+wid;
	if (getPen()->ypos-ht < picturetop) picturetop = getPen()->ypos-ht;
	if (getPen()->ypos > picturebottom) picturebottom = getPen()->ypos;
    }
}


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
    if (!printer.Print(turtleFrame, &printout, true /*prompt*/)) {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
	        wxMessageBox(_T("There was a problem printing.\nPerhaps your current printer is not set correctly?"), _T("Printing"), wxOK);
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
    preview->SetZoom(100);
    wxPreviewFrame *frame = new wxPreviewFrame(preview, wxTerminal::terminal->terminal, _T("Turtle Graphics Preview"), wxPoint(100, 100), wxSize(600, 650));
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
	TurtleCanvas::colors[color+SPECIAL_COLORS] =
				wxColour(r/256,g/256,b/256);
}

extern "C" void get_palette(int color, unsigned int *r, unsigned int *g, unsigned int *b){
    wxColour colour(TurtleCanvas::colors[color+SPECIAL_COLORS]);
    *r = colour.Red()*256;
    *g = colour.Green()*256;
    *b = colour.Blue()*256;
}

extern "C" void save_pen(pen_info *p) {
  memcpy(((char *)(p)),((char *)(&turtleFrame->xgr_pen)),
	   sizeof(pen_info));
}

extern "C" void restore_pen(pen_info *p) {
    memcpy(((char *)(&turtleFrame->xgr_pen)),((char *)(p)),
	   sizeof(pen_info));
}

extern "C" void set_pen_patter(){
  nop();
}

extern "C" void logofill() {
    if (drawToPrinter)
	TurtleCanvas::realFloodFill(turtleFrame->xgr_pen.color, printerDC);
    else if (drawToWindow)
	TurtleCanvas::realFloodFill(turtleFrame->xgr_pen.color, windowDC);
    else {
#if USE_MEMDC
      TurtleCanvas::realFloodFill(turtleFrame->xgr_pen.color, m_memDC);
#else
	wxDC *dc = new wxClientDC(turtleGraphics);
	TurtleCanvas::realFloodFill(turtleFrame->xgr_pen.color, dc);
	delete dc;
#endif
    }

}


extern "C" void wx_refresh() {
#if USE_MEMDC
  if(turtleGraphics) {
    turtleGraphics->Refresh();
    turtleGraphics->Update();
  }
#endif
}

/* Clear the turtle graphics screen, and put in splitscreen if we are
   currently in full text mode */
extern "C" void wx_clear() {
    if (drawToPrinter)
	TurtleCanvas::realClearScreen(printerDC);
    else if (drawToWindow)
	TurtleCanvas::realClearScreen(windowDC);
    else {
#if USE_MEMDC
      TurtleCanvas::realClearScreen(m_memDC);
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

extern "C" char record_buffer[];

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
    if (!drawToPrinter && !drawToWindow) {
	if (numLines == LINEPAUSE) {
	  wxMilliSleep(1);
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
#if USE_MEMDC
        TurtleCanvas::drawOneLine(&l, m_memDC);
#else
	dc = new wxClientDC(turtleGraphics);
	TurtleCanvas::drawOneLine(&l, dc);
	delete dc;
#endif
    }
    return;
}

extern "C" void doFilled(int fillcolor, int count, struct mypoint *points) {
    if (drawToPrinter)
	TurtleCanvas::realdoFilled(fillcolor, count, points, printerDC);
    else if (drawToWindow)
	TurtleCanvas::realdoFilled(fillcolor, count, points, windowDC);
    else {
#if USE_MEMDC
      TurtleCanvas::realdoFilled(fillcolor, count, points, m_memDC);
#else
	wxDC *dc = new wxClientDC(turtleGraphics);
	TurtleCanvas::realdoFilled(fillcolor, count, points, dc);
	delete dc;
#endif
    }
}

/* Set the pen width.  Notice this only takes one number, because wx only
   allows us to set the width and not the pen height */
extern "C" void wxSetPenWidth(int width){
    turtleFrame->xgr_pen.pw = width;
}

extern "C" enum s_md {SCREEN_TEXT, SCREEN_SPLIT, SCREEN_FULL} screen_mode;

//the event calling this never seems to trigger...
void TurtleCanvas::OnChar(wxKeyEvent& event) {
  wxTerminal::terminal->OnChar(event);
}

/* Put the logoframe into splitscreen mode*/
extern "C" void wxSplitScreen(){
    turtleFrame->in_graphics_mode = 1;
    turtleFrame->in_splitscreen = 1;
    topsizer->Show(wxTerminal::terminal, 1);
    topsizer->Show(turtleGraphics, 1);
    topsizer->Show(editWindow, 0);   
    topsizer->Layout();
    wxTerminal::terminal->SetFocus();
    wxTerminal::terminal->deferUpdate(0);
    screen_mode = SCREEN_SPLIT;
}

/* Put the logoframe into full screen mode */
extern "C" void wxFullScreen(){
    turtleFrame->in_graphics_mode = 1;
    turtleFrame->in_splitscreen = 0;
    topsizer->Show(wxTerminal::terminal, 0);
    topsizer->Show(turtleGraphics, 1);
    topsizer->Show(editWindow, 0);
    topsizer->Layout();
    wxTerminal::terminal->SetFocus();
    wxTerminal::terminal->deferUpdate(1);
    screen_mode = SCREEN_FULL;
}

/* Put the logoframe into text screen mode*/
extern "C" void wxTextScreen(){
  turtleFrame->in_graphics_mode = 0;
  turtleFrame->in_splitscreen = 0;
  topsizer->Show(wxTerminal::terminal, 1);
  topsizer->Show(turtleGraphics, 0);
  topsizer->Show(editWindow, 0);
  topsizer->Layout();
  wxTerminal::terminal->SetFocus();
  wxTerminal::terminal->deferUpdate(0);
  screen_mode = SCREEN_TEXT;
}

extern "C" void wxlPrintPict(){
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
  turtleGraphics->PrintTurtleWindow(event);
}

extern "C" void wxlPrintPreviewPict(){
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
  turtleGraphics->TurtlePrintPreview(event);
}

extern "C" void wxlPrintText(){
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
  logoFrame->OnPrintText(event);
}

extern "C" void wxlPrintPreviewText(){
  wxCommandEvent event(wxEVT_LOGO_CUSTOM_COMMAND);
  logoFrame->OnPrintTextPrev(event);
}

void getMousePosition (int * x, int * y) {
  *x = TurtleCanvas::mousePosition_x - wxGetInfo(SCREEN_WIDTH)/2;
  *y = wxGetInfo(SCREEN_HEIGHT)/2 - TurtleCanvas::mousePosition_y;
}

void getClickPosition (int * x, int * y) {
  *x = TurtleCanvas::clickPosition_x - wxGetInfo(SCREEN_WIDTH)/2;
  *y = wxGetInfo(SCREEN_HEIGHT)/2 - TurtleCanvas::clickPosition_y;
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

extern "C" int wxGetClickX() {
  int x, y;
  getClickPosition(&x,&y);
  return x;
}
extern "C" int wxGetClickY() {
  int x, y;
  getClickPosition(&x,&y);
  return y;
}

extern "C" int wxGetButton () {
    return TurtleCanvas::mouse_down_left + TurtleCanvas::mouse_down_middle
	    + TurtleCanvas::mouse_down_right;
}
extern "C" int wxGetLastButton () {
    return TurtleCanvas::mouse_down_last;
}

/* Show the text editor and have it load the given file */
extern "C" int wxEditFile(char * f){
  file = f;
  alreadyDone = 0;
  //turtleGraphics->ProcessEvent(editEvent);
  turtleGraphics->editCall();
 
  return editWindow->doSave;
}

extern "C" pen_info* getPen(){
	return &turtleFrame->xgr_pen;	
}

extern "C" void wxSetInfo(int type, int val) {
    TurtleCanvas::setInfo(type, val);
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
#if USE_MEMDC
      TurtleCanvas::realDrawLabel(string, m_memDC);
#else
	wxDC *dc = new wxClientDC(turtleGraphics);
	TurtleCanvas::realDrawLabel(string, dc);
	delete dc;	
#endif
    }
}


void TurtleCanvas::exitApplication()
{
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

#if !defined(__WXMAC__) && !defined(__WXMSW__)
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
    
#if defined(__WXMAC__) || defined(__WXMSW__)
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
    fillScale = actualScale;
#endif

    drawToPrinter = 1;
    turtle_shown = 0;
    redraw_graphics();
    turtle_shown = oldshown;
    drawToPrinter = 0;

#if !defined(__WXMAC__) && !defined(__WXMSW__)
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
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool TurtleWindowPrintout::HasPage(int pageNum)
{
    return (pageNum == 1);
}
