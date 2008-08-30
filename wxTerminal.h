#ifndef INCLUDE_WXTERM
#define INCLUDE_WXTERM

#ifdef __GNUG__
#pragma interface
#endif

#include <wx/html/htmprint.h>
#include <wx/print.h>

#define CURSOR_BLINK_DEFAULT_TIMEOUT	300
#define CURSOR_BLINK_MAX_TIMEOUT	2000
#define wxEVT_COMMAND_TERM_RESIZE        wxEVT_USER_FIRST + 1000
#define wxEVT_COMMAND_TERM_NEXT          wxEVT_USER_FIRST + 1001

#define EVT_TERM_RESIZE(id, fn) { wxEVT_COMMAND_TERM_RESIZE, id, -1, (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) &fn, (wxObject *)NULL },


class TurtleWindowPrintout: public wxPrintout
{
 public:
  TurtleWindowPrintout(wxChar *title = _T("Turtle Graphics")):wxPrintout(title) {}
  bool OnPrintPage(int page);
  bool HasPage(int page);
  bool OnBeginDocument(int startPage, int endPage);
  void TurtlePrintPreview();
  void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
};


// Structures for maintaining characters and lines
#define WXTERM_CB_SIZE 8000
#define WXTERM_LB_SIZE 1000

struct wxterm_char_buffer {
  char cbuf[WXTERM_CB_SIZE];       //characters
  char mbuf[WXTERM_CB_SIZE];       //mode flags
  wxterm_char_buffer *next; //next part of buffer
} ;

struct wxterm_charpos {
  wxterm_char_buffer *buf;   //which char buffer
  int offset;                //offset into buffer
  int line_length;           //length of line
} ;

#define inc_charpos(cp)   cp.offset++; \
                          if(cp.offset == WXTERM_CB_SIZE) {  \
                            if(!cp.buf->next) {  \
                              cp.buf->next = (wxterm_char_buffer *) malloc(sizeof(wxterm_char_buffer)); \
                              memset(cp.buf->next, '\0', sizeof(wxterm_char_buffer));  \
                            }  \
			    cp.buf = cp.buf->next; \
                            cp.offset = 0; \
                          }

// adjust offset >= WXTERM_CB_SIZE issues
#define adjust_charpos(cp) while(cp.offset >= WXTERM_CB_SIZE) { \
                             if(!cp.buf->next) {  \
                               cp.buf->next = (wxterm_char_buffer *) malloc(sizeof(wxterm_char_buffer)); \
                               memset(cp.buf->next, '\0', sizeof(wxterm_char_buffer));  \
                             }  \
			     cp.buf = cp.buf->next; \
                             cp.offset -= WXTERM_CB_SIZE; \
                          }
 

//lineposition consists of just a buffer and an offset.
#define inc_linepos(lp)   lp.offset++; \
                          if(lp.offset == WXTERM_LB_SIZE) { \
                              if(!lp.buf->next) { \
                                lp.buf->next = (wxterm_line_buffer *) malloc(sizeof(wxterm_line_buffer)); \
                                memset(lp.buf->next, 0, sizeof(wxterm_line_buffer)); \
                              } \
                              lp.buf = lp.buf->next; \
                              lp.offset = 0; \
                          }

//adjust offset >= WXTERM_LB_SIZE issue
#define adjust_linepos(lp) while(lp.offset >= WXTERM_LB_SIZE) { \
                               if(!lp.buf->next) { \
                                 lp.buf->next = (wxterm_line_buffer *) malloc(sizeof(wxterm_line_buffer)); \
                                 memset(lp.buf->next, 0, sizeof(wxterm_line_buffer)); \
                               } \
                               lp.buf = lp.buf->next; \
                               lp.offset -= WXTERM_LB_SIZE; \
                           }

                               
struct wxterm_line_buffer {
  wxterm_charpos lbuf[WXTERM_LB_SIZE];  //lines
  wxterm_line_buffer *next;          //next part of buffer
};

struct wxterm_linepos {
  wxterm_line_buffer *buf;  //which line buffer
  int offset;               //offset into line buffer
};


#define line_of(lpos) lpos.buf->lbuf[lpos.offset]
#define char_of(cpos) cpos.buf->cbuf[cpos.offset]
#define mode_of(cpos) cpos.buf->mbuf[cpos.offset]

// enter/exit standout mode character
#define TOGGLE_STANDOUT 17

  class wxTerminal : public wxScrolledWindow
{

 public:
  int m_charWidth;
  int m_charHeight;

  int  m_width;
  int m_height;
  int  m_curFG,
    m_curBG;
 private:
  int  m_selx1,
    m_sely1,
    m_selx2,
    m_sely2,
    m_seloldx1,
    m_seloldy1,
    m_seloldx2,
    m_seloldy2,
    m_curX,
    m_curY,
    m_curFlags;

  //used in enableScrolling
  int 
    m_vscroll_pos;

  //number of input lines ready
  int m_inputLines;
  bool m_inputReady; //whether ENTER was hit.


  bool
    m_selecting;
//    m_marking;

  wxColour
//    m_vt_colors[16],
//    m_pc_colors[16],
    m_colors[16];

  wxPen
//    m_vt_colorPens[16],
 //   m_pc_colorPens[16],
    m_colorPens[16];

  wxFont
    m_normalFont,
    m_underlinedFont,
    m_boldFont,
    m_boldUnderlinedFont;

  wxMemoryDC
    m_memDC;

  FILE
    *m_printerFN;

  char
    *m_printerName;


// character buffer
  wxterm_char_buffer *term_chars;
  wxterm_line_buffer *term_lines;
  wxterm_charpos curr_char_pos;
  wxterm_linepos curr_line_pos;
public:
  int cursor_x, cursor_y;

public:
	// mode flags
	enum MODES
        {
          BOLD=0x1, 
          BLINK=0x2, 
          UNDERLINE=0x4, 
          INVERSE=0x8,
	  DEFERUPDATE=0x10,
//          DESTRUCTBS=0x800,
          CURSORINVISIBLE=0x20,
          //SELECTED=0x40	// flag to indicate a char is selected
        };

	char m_currMode;

#if 0
  enum BOLDSTYLE
  {
    DEFAULT = -1,
    COLOR = 0,
    OVERSTRIKE = 1,
    FONT = 2
  };
#endif

private:
//  BOLDSTYLE m_boldStyle;

  typedef struct
  {
    wxKeyCode
      keyCode;

    int
      VTKeyCode;
  } TermKeyMap;


public:
  wxTerminal(wxWindow* parent, wxWindowID id,
         const wxPoint& pos = wxDefaultPosition,
         int width = 81, int height = 25,
         const wxString& name = _T("wxTerminal"));

  virtual ~wxTerminal();
  // For printing the text
  wxHtmlEasyPrinting *htmlPrinter;
  int 
	  x_coord,
	  y_coord,
	  x_max,
	  y_max,
	  isEditFile;
  static wxTerminal *terminal; 
  void GetCharSize(int *cw, int *ch);
  bool SetFont(const wxFont& font);
  void GetColors(wxColor colors[16]/*, wxTerminal::BOLDSTYLE boldStyle = wxTerminal::DEFAULT*/);
  void deferUpdate(int);
  void set_mode_flag(int flag);
  void clear_mode_flag(int flag);

  void ClearSelection();
  bool HasSelection();
  wxString GetChars(int x1,int y1,int x2, int y2);
  wxString GetSelection();
  void SelectAll();
  wxterm_linepos GetLinePosition(int y);
  wxterm_charpos GetCharPosition(int x, int y);
  void DoCopy();
  void DoPaste();
  void ProcessInput();
  void Flush ();
  void OnSize(wxSizeEvent& event);
  void terminalEvent (wxCommandEvent & event);
  void PassInputToInterp();
  void setCursor (int x, int y, bool fromLogo = FALSE);

  //scrolling
  void EnableScrolling(bool want_scrolling);
    
  virtual void DrawText(wxDC& dc, int fg_color, int bg_color, int flags,
                        int x, int y, int len, char *string);

//  virtual void MoveChars(int sx, int sy, int dx, int dy, int w, int h);
//  virtual void ClearChars(int bg_color, int x, int y, int w, int h);
//  virtual void ModeChange(int state);
  virtual void Bell();
  void RenumberLines(int new_width);
  virtual void ResizeTerminal(int w, int h);
//  virtual void RequestSizeChange(int w, int h);

  void DebugOutputBuffer();
  void InsertChar(char c);
  void NextLine();
  virtual void PassInputToTerminal(int len, char *data);

  wxString *get_text();

  void ClearScreen();

  virtual void SelectPrinter(char *PrinterName);
  virtual void PrintChars(int len, char *data);


  //TurtleCanvas passes char to here.
  void OnChar(wxKeyEvent& event);

  void handle_backspace();
  void handle_home();
  void handle_end();
  void handle_clear_to_end();
  void handle_history_prev();
  void handle_history_next();
  void handle_left();
  void handle_right();



private:
  int MapKeyCode(int keyCode);
  void InvertArea(wxDC &dc, int tx1, int tx2, int w, int h, bool scrolled_coord = FALSE);
  void MarkSelection(wxDC &dc, bool scrolled_coord = FALSE);

  int CheckPlatformKeys(wxKeyEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnEraseBackground(wxEraseEvent& event);
  void OnPaint(wxPaintEvent& event);
  void OnDraw(wxDC& dc);
  void GetClickCoords(wxMouseEvent& event, int *x, int *y);
  void OnLeftDown(wxMouseEvent& event);
  void OnLeftUp(wxMouseEvent& event);
  void OnMouseMove(wxMouseEvent& event);
  void LoseFocus (wxFocusEvent & event);
  
  DECLARE_EVENT_TABLE()
};

 enum terminalEvents {
    CLEARTEXT = 0,
    SETCURSOR
  };

void init_Logo_Interpreter (int argc, char ** argv);
#endif /* INCLUDE_WXTERM */

