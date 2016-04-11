#include <iostream>
#include "wx/wx.h"
//#include "gterm.hpp"
#include "LogoFrame.h"
#include "wxGlobals.h"
#include <wx/clipbrd.h>
#include <wx/html/htmprint.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/msgdlg.h>
#include <wx/colour.h>
#include "TextEditor.h"
#include "wxTurtleGraphics.h"
#include "wxTerminal.h"		/* must come after wxTurtleGraphics.h */


// ----------------------------------------------------------------------------
// TextEditor
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (TextEditor, wxTextCtrl)
EVT_CHAR(TextEditor::OnChar)  
EVT_TEXT(wxID_ANY, TextEditor::SetThisFont)
EVT_FIND(wxID_ANY, TextEditor::OnFindDialog)
EVT_CLOSE(TextEditor::OnCloseEvent)
END_EVENT_TABLE()


// globals for signalling to logo 
int editor_active = 0; 

/* The constructor */
TextEditor::TextEditor(wxWindow* const f, int a, wxString s, const wxPoint& p , const wxSize& sz, int b, wxFont font ) :
wxTextCtrl(f, a, s, p, sz, b)
{
	findDlg=0;
	findData=0;
	prevFind=0;
	this->font = font;
	SetFont(font);
}

void TextEditor::SetFont(wxFont font){
	this->font = font;
#ifdef __WXMAC__                                                        
	SetDefaultStyle(wxTextAttr(wxNullColour,wxNullColour, font));
	if(this->IsShown()){
		SetStyle(0,GetLastPosition(), wxTextAttr(wxNullColour,wxNullColour,font));
#else
	SetDefaultStyle(wxTextAttr(TurtleCanvas::colors[wxTerminal::terminal->m_curFG],
				   TurtleCanvas::colors[wxTerminal::terminal->m_curBG], font));
	if(this->IsShown()){
		SetStyle(0,GetLastPosition(),
			 wxTextAttr(TurtleCanvas::colors[wxTerminal::terminal->m_curFG],
				    TurtleCanvas::colors[wxTerminal::terminal->m_curBG],font));
    SetBackgroundColour(
		TurtleCanvas::colors[wxTerminal::terminal->m_curBG]);
#endif
		Refresh();
		Update();
	}
}

void TextEditor::SetThisFont(wxCommandEvent& event) {
#ifdef __WXMAC__                                                        
	SetDefaultStyle(wxTextAttr(wxNullColour,wxNullColour, this->font));
	if(this->IsShown()){
		SetStyle(GetLastPosition()-1, GetLastPosition(), wxTextAttr(wxNullColour,wxNullColour,this->font));
#else
	SetDefaultStyle(wxTextAttr(TurtleCanvas::colors[wxTerminal::terminal->m_curFG],
				   TurtleCanvas::colors[wxTerminal::terminal->m_curBG], this->font));
	if(this->IsShown()){
		SetStyle(GetLastPosition()-1, GetLastPosition(),
			 wxTextAttr(TurtleCanvas::colors[wxTerminal::terminal->m_curFG],
				    TurtleCanvas::colors[wxTerminal::terminal->m_curBG],this->font));
	    SetBackgroundColour(
			TurtleCanvas::colors[wxTerminal::terminal->m_curBG]);
#endif
		Refresh();
		Update();
		Refresh();
		Update();
	}
}

/* Loads the given filename into the text editor*/
bool TextEditor::Load(const wxString& filename){
	file = filename;
	LoadFile(file);
	SetFont(font);
	return true;
}

void TextEditor::OnFindNext(){
	wxFindDialogEvent event;
	event.SetFindString(*prevFind);
	OnFindDialog(event);
	
}

void TextEditor::OnFind(){
	if(findDlg)
		return;
	findData = new wxFindReplaceData();
	findDlg = new wxFindReplaceDialog(this, findData, wxString("Find...", wxConvUTF8), wxFR_NOWHOLEWORD | 
									  wxFR_NOMATCHCASE | wxFR_NOUPDOWN );
	findDlg->Show(TRUE);
	
	
}

/* This is called when someone pressed the Find button inside
	the finder dialog */
void TextEditor::OnFindDialog(wxFindDialogEvent& event)
{
	long int start, end, dummy, loc;
	end = GetLastPosition();
	wxString textstring(GetRange(0, GetLastPosition()));
	GetSelection(&dummy,&start);
	wxString findString(event.GetFindString());
	if(prevFind){
		delete prevFind;
	}
	prevFind = new wxString(findString);
	loc = Find(findString, start);
	
	// if we didn't find anything, try again from the beginning
	if (loc == -1 && start != 0){
		start = 0;
		loc = Find(findString, start);
	}
	if (loc == -1){
		wxMessageDialog dlg(this, wxString("Not Found", wxConvUTF8),
							wxString("Find String Not Found", wxConvUTF8),
							wxOK | wxICON_INFORMATION);
		dlg.ShowModal();
		dlg.Destroy();
		if (findDlg){
			if (loc == -1)
				findDlg->SetFocus();
			return;
		}
	}
	
	else{
		if(findDlg){
			delete findDlg->GetData();	
			findDlg->Destroy();
			findDlg=0;
		}
		ShowPosition(loc);
	}
}

/* This finds the next instance of a search string ESEARCH
within the entire text of the editor starting from
START
*/
int TextEditor::Find (const wxString & sSearch, int start){
    wxString allText = GetValue();
	wxString sText = allText.Mid(start);
	// the replace is for windows compatibility
#ifdef __WXMSW__
	sText.Replace(wxT( "\n" ), wxT( " \n" ) ) ;
#endif
    int iStart =sText.Find (sSearch);
    if(iStart == -1)
		return -1;
	if(iStart==0){
		if(sText.Mid(0,sSearch.length()) != sSearch)
			return -1;
	}
	int iEnd = iStart + sSearch.Length();
    SetSelection (iStart+start, iEnd+start);
    return iStart+start ;
}

void TextEditor::DoCut(){
	this->Cut();
}

void TextEditor::DoCopy(){
	this->Copy();
}

void TextEditor::DoPaste(){
	this->Paste();
}


void TextEditor::OnCloseEvent(wxCloseEvent& event){
	OnCloseReject();
}

extern "C" int getTermInfo(int);
extern "C" void setTermInfo(int,int);

/* Closes the text editor and saves changes */
void TextEditor::OnCloseAccept(){
	doSave=1;
	if(getTermInfo(EDIT_STATE) == NO_INFO){
		wxMessageDialog dialog( NULL, _T("Load changes into Logo?"),
								_T("Load Changes"), wxNO_DEFAULT|wxYES_NO|wxCANCEL|wxICON_INFORMATION);
		switch ( dialog.ShowModal() )
		{
			case wxID_YES:
				setTermInfo(EDIT_STATE,DO_LOAD);
				break;
				
			case wxID_NO:
				setTermInfo(EDIT_STATE,NO_LOAD);
				break;
				
			case wxID_CANCEL:
				return;
				
			default:
				wxLogError(wxT("Unexpected wxMessageDialog return code!"));
		}
	}
	OnSave();
	Close();
}

/* Closes the text editor but does not save changes */
void TextEditor::OnCloseReject(){
	doSave=0;
	Close();
}



/* Does the actual closing of the editor by hiding it and
   bringing back the terminal */
void TextEditor::Close(){
	if(findDlg){
		delete findDlg->GetData();
		findDlg->Destroy();
		findDlg = 0; 
	}
	topsizer->Show(wxTerminal::terminal, 1);
	topsizer->Show((wxWindow *)turtleGraphics, turtleGraphics->getInfo(IN_GRAPHICS_MODE));
	topsizer->Show(editWindow, 0);
	topsizer->Layout();
	wxTerminal::terminal->SetFocus();
	logoFrame->SetUpMenu();
	editor_active = 0;
}

void TextEditor::OnSave(){
	SaveFile(file);
}

/* Prints out the text in this editor */
void TextEditor::DoPrint(){
	
	if(!wxTerminal::terminal->htmlPrinter)
		wxTerminal::terminal->htmlPrinter = new wxHtmlEasyPrinting();
	int fontsizes[] = { 6, 8, 12, 14, 16, 20, 24 };
	wxTerminal::terminal->htmlPrinter->SetFonts(_T("Courier"),_T("Courier"), fontsizes);
	wxString textString;
	textString.Clear();
	
	long i;
	for(i=0;i<GetNumberOfLines();i++){
		textString.Append(GetLineText(i));
		textString.Append(_T("<BR>"));
	}
	
	wxTerminal::terminal->htmlPrinter->PrintText(textString);
	 
	
}

