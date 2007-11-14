#ifndef BASIC_H
#define BASIC_H
#include <wx/wx.h>

#define NAME_BUFFER_SIZE 300
#define MAXOUTBUFF_ 4 * (1024)

// This is the Top level Application Class
class LogoApplication : public wxApp
{
  public:
	virtual bool OnInit();
};


// This is the Top level frame for logo
class LogoFrame : public wxFrame
{
  public:
	LogoFrame( const wxChar *title,
                int xpos, int ypos,
                int width, int height);
	
	void SetUpEditMenu();
	void SetUpMenu();
	
private:
	void OnQuit(wxCommandEvent& WXUNUSED(event));
	void OnSave(wxCommandEvent& WXUNUSED(event));
	void OnLoad(wxCommandEvent& WXUNUSED(event));
	void OnTurtlePrintPreview(wxCommandEvent& WXUNUSED(event));
	void DoStop(wxCommandEvent& WXUNUSED(event));
	void DoPause(wxCommandEvent& WXUNUSED(event));
	void DoPaste(wxCommandEvent&);
	void DoCopy(wxCommandEvent&);
	void OnPrintText(wxCommandEvent&);
	void OnPrintTextPrev(wxCommandEvent&);
	void OnPrintTurtle(wxCommandEvent&);
	void OnIncreaseFont(wxCommandEvent&);
	void OnDecreaseFont(wxCommandEvent&);
	
	void OnEditCloseAccept(wxCommandEvent& WXUNUSED(event));
	void OnEditCloseReject(wxCommandEvent& WXUNUSED(event));
	void OnEditPrint(wxCommandEvent& WXUNUSED(event));
	void OnEditCopy(wxCommandEvent& WXUNUSED(event));
	void OnEditCut(wxCommandEvent& WXUNUSED(event));
	void OnEditFind(wxCommandEvent& WXUNUSED(event));
	void OnEditFindNext(wxCommandEvent& WXUNUSED(event));
	void OnEditPaste(wxCommandEvent& WXUNUSED(event));
	void OnEditSave(wxCommandEvent& WXUNUSED(event));
	void OnEditLoad(wxCommandEvent& WXUNUSED(event));
	
	DECLARE_EVENT_TABLE()
};


#endif
