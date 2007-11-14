/*
    Much of this code from: taTelnet - A cross-platform telnet program.
							Copyright (c) 2000 Derry Bryson.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
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

#ifndef INCLUDE_WXTERM
#define INCLUDE_WXTERM

#ifdef __GNUG__
#pragma interface
#endif

#include <wx/html/htmprint.h>
#include "gterm.hpp"
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

  class wxTerminal : public wxScrolledWindow , public GTerm
{

  wxCoord
	m_charWidth;
  int
    m_charHeight,
    m_init,
    m_width,
    m_height,
    m_selx1,
    m_sely1,
    m_selx2,
    m_sely2,
	m_seloldx1,
    m_seloldy1,
    m_seloldx2,
    m_seloldy2,
    m_curX,
    m_curY,
    m_curFG,
    m_curBG,
    m_curFlags,
    m_curState,
    m_curBlinkRate;

  unsigned char
    m_curChar;

  bool
    m_selecting,
    m_marking;

  wxColour
    m_vt_colors[16],
    m_pc_colors[16],
    *m_colors;

  wxPen
    m_vt_colorPens[16],
    m_pc_colorPens[16],
    *m_colorPens;

  wxFont
    m_normalFont,
    m_underlinedFont,
    m_boldFont,
    m_boldUnderlinedFont;

  wxMemoryDC
    m_memDC;

  wxBitmap
    *m_bitmap;

  FILE
    *m_printerFN;

  char
    *m_printerName;

  wxTimer
    m_timer;

public:
  enum BOLDSTYLE
  {
    DEFAULT = -1,
    COLOR = 0,
    OVERSTRIKE = 1,
    FONT = 2
  };

private:
  BOLDSTYLE
    m_boldStyle;

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
         const wxString& name = "wxTerminal");

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
  bool SetFont(const wxFont& font);
  void GetDefVTColors(wxColor colors[16], wxTerminal::BOLDSTYLE boldStyle = wxTerminal::DEFAULT);
  void GetVTColors(wxColour colors[16]);
  void SetVTColors(wxColour colors[16]);
  void GetDefPCColors(wxColour colors[16]);
  void GetPCColors(wxColour colors[16]);
  void SetPCColors(wxColour colors[16]);
  int GetCursorBlinkRate() { return m_curBlinkRate; }
  void SetCursorBlinkRate(int rate);
  void RefreshTerminal();
  void SetBoldStyle(wxTerminal::BOLDSTYLE boldStyle);
  wxTerminal::BOLDSTYLE GetBoldStyle(void) { return m_boldStyle; }

  void ClearSelection();
  bool HasSelection();
  wxString GetSelection();
  void SelectAll();
  void DoCopy();
  void DoPaste();
  void printText (wxCommandEvent& event);
  void OnSize(wxSizeEvent& event);
  void terminalEvent (wxCommandEvent & event);
  void PassInputToInterp();
  void setCursor (int x, int y);
  int currentPosition ();
    
  /*
  **  GTerm stuff
  */
  virtual void DrawText(int fg_color, int bg_color, int flags,
                        int x, int y, int len, unsigned char *string);
  virtual void DrawCursor(int fg_color, int bg_color, int flags,
                          int x, int y, unsigned char c);

  virtual void MoveChars(int sx, int sy, int dx, int dy, int w, int h);
  virtual void ClearChars(int bg_color, int x, int y, int w, int h);
  virtual void ModeChange(int state);
  virtual void Bell();
  virtual void ResizeTerminal(int w, int h);
  virtual void RequestSizeChange(int w, int h);

  virtual void PassInputToGterm(int len, unsigned char *data);

  virtual void SelectPrinter(char *PrinterName);
  virtual void PrintChars(int len, unsigned char *data);

private:
  int MapKeyCode(int keyCode);
  void MarkSelection();
  void DoDrawCursor(int fg_color, int bg_color, int flags,
                    int x, int y, unsigned char c);

  int CheckPlatformKeys(wxKeyEvent& event);
  void OnChar(wxKeyEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnPaint(wxPaintEvent& event);
  void OnLeftDown(wxMouseEvent& event);
  void OnLeftUp(wxMouseEvent& event);
  void OnMouseMove(wxMouseEvent& event);
  void OnTimer(wxTimerEvent& event);
  void LoseFocus (wxFocusEvent & event);
  
  DECLARE_EVENT_TABLE()
};

 enum terminalEvents {
    CLEARTEXT = 0,
    SETCURSOR
  };

void init_Logo_Interpreter (int argc, char ** argv);
#endif /* INCLUDE_WXTERM */

