
#include <wx/wx.h>

#include <wx/colordlg.h>
#include <wx/image.h>
#include <wx/artprov.h>

#include <vector>
#include "LogoFrame.h"
#include "logo.h"
#include "wxTerminal.h"
#include <wx/colordlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include "wxGlobals.h"

// ----------------------------------------------------------------------------
// structs
// ----------------------------------------------------------------------------

struct line {
	short int x1;
	short int y1;
	short int x2;
	short int y2;
	short int pw;
	unsigned char color;
	unsigned char pm;
	int vis;
};

typedef struct {
  int color;
  int xpos;
  int ypos;
  int vis;
  int pw;
  int ph;
  int pen_mode;
  } pen_info;




// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------


// what do we show on screen (there are too many shapes to put them all on
// screen simultaneously)
enum ScreenToShow
{
    Show_Default,
    Show_Text,
    Show_Lines,
    Show_Brushes,
    Show_Polygons,
    Show_Mask,
    Show_Ops,
    Show_Regions,
    Show_Circles
};

enum messageEnum {
	PREPARE = 0,
	SPLITSCREEN,
	FULLSCREEN,
	TEXTSCREEN,
	DRAWLINE,
	EDITCALL,
	CLEARSCREEN,
	SAVEPEN,
	RESTOREPEN,
	KILLAPPLICATION,
	SETPENCOLOR,
	ADDLINE,
	SETPENWIDTH,
	GETPALETTE,
	FLOODFILL,
	TOPRINTER,
	GETMOUSECOORDS,
	GETMOUSEDOWN,
	SETINFO,
	GETINFO,
	DRAWLABEL,
	CATCHUP,
	PRINTPICT,
	PRINTPREVIEWPICT,
	PRINTTEXT,
	PRINTPREVIEWTEXT
};

#define PEN_REVERSE              0
#define PEN_ERASE                1
#define PEN_DOWN                 2

#define NUMCOLORS 512
#define NUMINITCOLORS 16


// ----------------------------------------------------------------------------
// Classes
// ----------------------------------------------------------------------------

class TurtleCanvas;

// This frame will contain the TurtleCanvas
class TurtleFrame : public wxFrame
{
public:
    TurtleFrame( wxFrame * parent, const wxString& title, const wxPoint& pos, const wxSize& size);
    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);	
    void OnShow(wxCommandEvent &event);
	
	static int 
		back_ground,
		screen_height, 
		screen_width,
	    in_graphics_mode,
	    in_splitscreen;
	static pen_info xgr_pen;
    int         m_backgroundMode;
    int         m_textureBackground;
    int         m_mapMode;
    double      m_xUserScale;
    double      m_yUserScale;
    int         m_xLogicalOrigin;
    int         m_yLogicalOrigin;
    bool        m_xAxisReversed,
		m_yAxisReversed;
    wxColour    m_colourForeground,    // these are _text_ colours
		m_colourBackground;
    wxBrush     m_backgroundBrush;
    TurtleCanvas   *m_canvas;
	
    TurtleCanvas * GetCanvas ();
private:

		// any class wishing to process wxWindows events must use this macro
		DECLARE_EVENT_TABLE()
};

struct mypoint {
    int x,y;
};

// define a scrollable canvas for drawing onto
class TurtleCanvas: public wxWindow
{
public:
    TurtleCanvas( wxFrame *parent );
    virtual ~TurtleCanvas();
    wxFrame *m_owner;
    wxPaintDC * dc;

	static void FinishedEvent();
	static void WaitForEvent();
    void OnPaint(wxPaintEvent &event);
    void OnSize(wxSizeEvent &event);
    void OnDraw(wxDC &dc);

    bool SetFont(const wxFont &f);

    void Show(ScreenToShow show) { m_show = show; Refresh(); }
	void internalPrepare();
	void TurtlePrintPreview(wxCommandEvent& event);
	void PrintTurtleWindow(wxCommandEvent& event);
    void SetOwner (wxFrame * frame);
	void SetDC (wxPaintDC * dc);
	void drawLine(wxCommandEvent & e);
    void editCall();
    void logoHandle ( wxCommandEvent & e);
    void exitApplication();
    void OnFocus (wxFocusEvent & event);
    void LoseFocus (wxFocusEvent & event);
    void OnLeftDown(wxMouseEvent& event) ;
    void OnLeftUp(wxMouseEvent& event);
     void OnMiddleDown(wxMouseEvent& event) ;
    void OnMiddleUp(wxMouseEvent& event);
     void OnRightDown(wxMouseEvent& event) ;
    void OnRightUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event) ;
    void OnChar(wxKeyEvent& event);
    static void setInfo(int type, int val);
    static int getInfo(int type);
    void OnTimer (wxTimerEvent& event);
	void OnEraseBackGround(wxEraseEvent& event) ;
    void OnPageSetup(wxCommandEvent& event);
    static void drawOneLine(struct line *, wxDC *);
    static void realFloodFill(int, wxDC *);
    static void realdoFilled(int fillcolor, int count,
				    struct mypoint *points, wxDC *dc);

    static void realDrawLabel(char *, wxDC *);
    static void realClearScreen(wxDC *);
	void PaintBackground(wxDC& dc) ;

    wxFrame *  GetOwner();
	wxDC *  GetDC();

	static int mousePosition_x;
	static int mousePosition_y;
	static int clickPosition_x;
	static int clickPosition_y;

	static int mouse_down_left;
	static int mouse_down_middle;
	static int mouse_down_right;
	static int mouse_down_last;

	static wxColour colors[NUMCOLORS+SPECIAL_COLORS];

private:

	wxBitmap
		*m_bitmap;

	int resized;
	ScreenToShow m_show;
    wxBitmap     m_smile_bmp;
    wxIcon       m_std_icon;
    bool         m_clip;

    wxTimer * m_timer;


    DECLARE_EVENT_TABLE()
};


// ----------------------------------------------------------------------------
// Accessed by interpreter 
// ----------------------------------------------------------------------------
extern "C" void nop();
extern "C" void set_palette(int, unsigned int, unsigned int, unsigned int);
extern "C" void get_palette(int, unsigned int*, unsigned int*, unsigned int*);
extern "C" void save_pen(pen_info *p);
extern "C" void restore_pen(pen_info *p);
extern "C" void set_pen_patter();
extern "C" void logofill();
extern "C" void wx_clear();
extern "C" NODE lclearscreen(NODE *);
extern "C" void wxDrawLine(int x1, int y1, int x2, int y2, int vis);
extern "C" void wxSplitScreen();
extern "C" void wxFullScreen();
extern "C" void wxTextScreen();
extern "C" void wxPrepare();
extern "C" int  wxEditFile(char * f);

extern "C" int wxGetMouseX();
extern "C" int wxGetMouseY();
extern "C" int wxGetButton();

extern "C" void redraw_graphics();

extern "C" void wxSetInfo(int type, int val);
extern "C" int wxGetInfo(int type);


