#include <wx/fdrepdlg.h>



class TextEditor : public wxTextCtrl{
	
public:
	TextEditor(wxWindow* const, int, wxString, const wxPoint&, const wxSize&, int, wxFont font);
	void OnSave();
	void OnFind();
	void OnFindNext();
	void DoPaste();
	void DoCopy();
	void DoCut();
	void DoPrint();
	void OnCloseAccept();
	void OnCloseReject();
	bool Load(const wxString& filename);
	void SetFont(wxFont font);
	void SetThisFont(wxCommandEvent&);
	
	int stateFlag;
	int doSave;
	
private:    
	wxFindReplaceDialog *findDlg;
	wxFindReplaceData *findData;
	wxString *prevFind;
	wxFont font;
	
	void OnCloseEvent(wxCloseEvent& event);
	void OnFindDialog(wxFindDialogEvent& event);
	void Close();
	int Find (const wxString & sSearch, int start);
	wxString file;
	
	
	DECLARE_EVENT_TABLE()
		
};
