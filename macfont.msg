$ diff wxMac-2.6.4/src/mac/carbon/fontdlg.cpp wxMac-2.8.8/src/mac/carbon/fontdlg.cpp

2c2
< // Name:        fontdlg.cpp
---
> // Name:        src/mac/carbon/fontdlg.cpp
7c7
< // RCS-ID:      $Id: fontdlg.cpp,v 1.23 2005/05/10 06:28:21 SC Exp $
---
> // RCS-ID:      $Id: fontdlg.cpp 42208 2006-10-21 16:39:42Z VZ $
20,23d19
< #if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
< #pragma implementation "fontdlg.h"
< #endif
< 
26a23,24
> #if wxUSE_FONTDLG
> 
32c30,40
<     #include "wx/cmndata.h"
---
>     #include "wx/intl.h"
>     #include "wx/wxchar.h"
>     #include "wx/dcclient.h"
>     #include "wx/frame.h"
>     #include "wx/textctrl.h"
>     #include "wx/listbox.h"
>     #include "wx/checkbox.h"
>     #include "wx/choice.h"
>     #include "wx/sizer.h"
>     #include "wx/stattext.h"
>     #include "wx/button.h"
36c44,219
<     
---
> 
> #if wxMAC_USE_EXPERIMENTAL_FONTDIALOG
> 
> IMPLEMENT_DYNAMIC_CLASS(wxFontDialog, wxDialog)
> 
> #include "wx/mac/private.h"
> 
> // ---------------------------------------------------------------------------
> // wxFontDialog
> // ---------------------------------------------------------------------------
> 
> static const EventTypeSpec eventList[] =
> {
>     { kEventClassFont, kEventFontSelection } ,
> } ;
> 
> 
> pascal OSStatus wxMacCarbonFontPanelHandler(EventHandlerCallRef nextHandler, EventRef event, void *userData)
> {
>     OSStatus result = eventNotHandledErr ;
>     wxFontDialog *fontdialog = (wxFontDialog*) userData ;
>     wxFontData& fontdata= fontdialog->GetFontData() ;
> 
>     wxMacCarbonEvent cEvent( event );
>     switch(cEvent.GetKind())
>     {
>         case kEventFontSelection :
>         {
>             ATSUFontID fontId = 0 ;
>             if ( cEvent.GetParameter<ATSUFontID>(kEventParamATSUFontID, &fontId) == noErr )
>             {
>                 FMFontStyle fontStyle = cEvent.GetParameter<FMFontStyle>(kEventParamFMFontStyle);
>                 FMFontSize fontSize = cEvent.GetParameter<FMFontSize>(kEventParamFMFontSize);
> 
>                 CFStringRef cfName = NULL;
> #if 1
>                 FMFontFamily fontFamily = cEvent.GetParameter<FMFontFamily>(kEventParamFMFontFamily);
>                 ATSFontFamilyRef atsfontfamilyref = FMGetATSFontFamilyRefFromFontFamily( fontFamily ) ;
>                 OSStatus err = ATSFontFamilyGetName( atsfontfamilyref , kATSOptionFlagsDefault , &cfName ) ;
>                 wxASSERT_MSG( err == noErr , wxT("ATSFontFamilyGetName failed") );
> #else
>                 // we don't use the ATSU naming anymore
>                 ByteCount actualLength = 0;
>                 char *c = NULL;
>                 OSStatus err = ATSUFindFontName(fontId , kFontFamilyName, kFontUnicodePlatform, kFontNoScriptCode,
>                     kFontNoLanguageCode , 0 , NULL , &actualLength , NULL );
>                 if ( err == noErr)
>                 {
>                     actualLength += 1 ;
>                     char *c = (char*)malloc( actualLength );
>                     err = ATSUFindFontName(fontId, kFontFamilyName, kFontUnicodePlatform, kFontNoScriptCode,
>                         kFontNoLanguageCode, actualLength, c , NULL, NULL);
>                     cfName = CFStringCreateWithCharacters(NULL, (UniChar*) c, (actualLength-1) >> 1);
>                 }
>                 else
>                 {
>                     err = ATSUFindFontName(fontId , kFontFamilyName, kFontNoPlatformCode, kFontNoScriptCode,
>                     kFontNoLanguageCode , 0 , NULL , &actualLength , NULL );
>                     if ( err == noErr )
>                     {
>                         actualLength += 1 ;
>                         c = (char*)malloc(actualLength);
>                         err = ATSUFindFontName(fontId, kFontFamilyName, kFontNoPlatformCode, kFontNoScriptCode,
>                             kFontNoLanguageCode, actualLength, c , NULL, NULL);
>                         c[actualLength-1] = 0;
>                         cfName = CFStringCreateWithCString(NULL, c, kCFStringEncodingMacRoman );
>                     }
>                 }
>                 if ( c!=NULL )
>                     free(c);
> #endif
>                  
>                 if ( cfName!=NULL )
>                 {
>                     fontdata.m_chosenFont.SetFaceName(wxMacCFStringHolder(cfName).AsString(wxLocale::GetSystemEncoding()));
>                     fontdata.m_chosenFont.SetPointSize(fontSize);
>                     fontdata.m_chosenFont.SetStyle(fontStyle & italic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);
>                     fontdata.m_chosenFont.SetUnderlined((fontStyle & underline)!=0);
>                     fontdata.m_chosenFont.SetWeight(fontStyle & bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
>                 }
>             }
> 
>             RGBColor fontColor ;
>             if ( cEvent.GetParameter<RGBColor>(kEventParamFontColor, &fontColor) == noErr )
>                 fontdata.m_fontColour.FromRGBColor((WXCOLORREF*) &fontColor);
>             else
>             {
>                 CFDictionaryRef dict ;
>                 if ( cEvent.GetParameter<CFDictionaryRef>(kEventParamDictionary, &dict) == noErr )
>                 {
>                     CFDictionaryRef attributesDict ;
>                     if ( CFDictionaryGetValueIfPresent(dict, kFontPanelAttributesKey, (const void **)&attributesDict) )
>                     {
>                         CFDataRef tagsData;
>                         CFDataRef sizesData;
>                         CFDataRef valuesData;
>                         if ( CFDictionaryGetValueIfPresent(attributesDict, kFontPanelAttributeTagsKey, (const void **)&tagsData) &&
>                              CFDictionaryGetValueIfPresent(attributesDict, kFontPanelAttributeSizesKey, (const void **)&sizesData) &&
>                              CFDictionaryGetValueIfPresent(attributesDict, kFontPanelAttributeValuesKey, (const void **)&valuesData) )
>                         {
>                             ItemCount count = CFDataGetLength(tagsData)/sizeof(ATSUAttributeTag);
>                             ATSUAttributeTag *tagPtr = (ATSUAttributeTag *)CFDataGetBytePtr(tagsData);
>                             ByteCount *sizePtr = (ByteCount *)CFDataGetBytePtr(sizesData);
>                             UInt32 *bytePtr = (UInt32*)CFDataGetBytePtr(valuesData);
>                             ATSUAttributeValuePtr valuesPtr = bytePtr ;
>                             for ( ItemCount i = 0 ; i < count ; ++i)
>                             {
>                                 if ( tagPtr[i] == kATSUColorTag && sizePtr[i] == sizeof(RGBColor))
>                                 {
>                                     fontdata.m_fontColour.FromRGBColor((WXCOLORREF*) valuesPtr);
>                                     break ;
>                                 }
>                                 bytePtr = (UInt32*)( (UInt8*)bytePtr + sizePtr[i]);
>                             }
>                         }
>                     }
>                 }
>             }
>         }
>             break ;
>     } ;
> 
>     return result ;
> }
> 
> DEFINE_ONE_SHOT_HANDLER_GETTER( wxMacCarbonFontPanelHandler )
> 
> wxFontDialog::wxFontDialog()
> {
> }
> 
> wxFontDialog::wxFontDialog(wxWindow *parent, const wxFontData&  data)
> {
>     Create(parent, data);
> }
> 
> wxFontDialog::~wxFontDialog()
> {
> }
> 
> bool wxFontDialog::Create(wxWindow *parent, const wxFontData& data)
> {
>     m_fontData = data;
>     return true ;
> }
> 
> int wxFontDialog::ShowModal()
> {
>     OSStatus err ;
>     wxFont font = *wxNORMAL_FONT ;
>     if ( m_fontData.m_initialFont.Ok() )
>     {
>         font = m_fontData.m_initialFont ;
>     }
> 
>     ATSUStyle style = (ATSUStyle)font.MacGetATSUStyle();
>     err = SetFontInfoForSelection (kFontSelectionATSUIType,1, &style , NULL);
>     // just clicking on ENTER will not send us any font setting event, therefore we have to make sure
>     // that field is already correct
>     m_fontData.m_chosenFont = font ;
> 
>     EventHandlerRef handler ;
> 
>     err = InstallApplicationEventHandler( GetwxMacCarbonFontPanelHandlerUPP(), GetEventTypeCount(eventList), eventList, this , &handler );
> 
>     if ( !FPIsFontPanelVisible() )
>         FPShowHideFontPanel();
> 
>     int retval = RunMixedFontDialog(this);
> 
>     ::RemoveEventHandler(handler);
> 
>     return retval ;
> }
> 
> #else
44,46c227,229
< IMPLEMENT_DYNAMIC_CLASS(wxFontDialog, wxDialog)
< 
< #include "wx/mac/private.h"
---
> #include "wx/fontenum.h"
> #include "wx/colordlg.h"
> #include "wx/spinctrl.h"
52c235,384
< wxFontDialog::wxFontDialog()
---
> static const wxChar *FontFamilyIntToString(int family);
> static int FontFamilyStringToInt(const wxChar *family);
> 
> 
> //-----------------------------------------------------------------------------
> // helper class - wxFontPreviewCtrl
> //-----------------------------------------------------------------------------
> 
> class WXDLLEXPORT wxFontPreviewCtrl : public wxWindow
> {
> public:
>     wxFontPreviewCtrl(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& sz = wxDefaultSize, long style = 0) :
>         wxWindow(parent, id, pos, sz, style)
>     {
>         SetBackgroundColour(*wxWHITE);
>     }
> 
> private:
>     void OnPaint(wxPaintEvent& event);
>     DECLARE_EVENT_TABLE()
> };
> 
> BEGIN_EVENT_TABLE(wxFontPreviewCtrl, wxWindow)
>     EVT_PAINT(wxFontPreviewCtrl::OnPaint)
> END_EVENT_TABLE()
> 
> void wxFontPreviewCtrl::OnPaint(wxPaintEvent& WXUNUSED(event))
> {
>     wxPaintDC dc(this);
> 
>     wxSize size = GetSize();
>     wxFont font = GetFont();
> 
>     if ( font.Ok() )
>     {
>         dc.SetFont(font);
>         // Calculate vertical centre
>         long w = 0, h = 0;
>         dc.GetTextExtent( wxT("X"), &w, &h);
>         dc.SetTextForeground(GetForegroundColour());
>         dc.SetClippingRegion(2, 2, size.x-4, size.y-4);
>         dc.DrawText(_("ABCDEFGabcdefg12345"),
>                      10, size.y/2 - h/2);
>         dc.DestroyClippingRegion();
>     }
> }
> 
> /*
>  * A control for displaying a small preview of a colour or bitmap
>  */
> 
> class wxFontColourSwatchCtrl: public wxControl
> {
>     DECLARE_CLASS(wxFontColourSwatchCtrl)
> public:
>     wxFontColourSwatchCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
>     virtual ~wxFontColourSwatchCtrl();
> 
>     void OnPaint(wxPaintEvent& event);
>     void OnMouseEvent(wxMouseEvent& event);
> 
>     void SetColour(const wxColour& colour) { m_colour = colour; SetBackgroundColour(m_colour); }
> 
>     wxColour& GetColour() { return m_colour; }
> 
>     virtual wxSize DoGetBestSize() const { return GetSize(); }
> 
> protected:
>     wxColour    m_colour;
> 
> DECLARE_EVENT_TABLE()
> };
> 
> /*
>  * A control for displaying a small preview of a colour or bitmap
>  */
> 
> BEGIN_EVENT_TABLE(wxFontColourSwatchCtrl, wxControl)
>     EVT_MOUSE_EVENTS(wxFontColourSwatchCtrl::OnMouseEvent)
> END_EVENT_TABLE()
> 
> IMPLEMENT_CLASS(wxFontColourSwatchCtrl, wxControl)
> 
> wxFontColourSwatchCtrl::wxFontColourSwatchCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style):
>     wxControl(parent, id, pos, size, style)
> {
>     SetColour(* wxWHITE);
>     SetBackgroundStyle(wxBG_STYLE_COLOUR);
> }
> 
> wxFontColourSwatchCtrl::~wxFontColourSwatchCtrl()
> {
> }
> 
> void wxFontColourSwatchCtrl::OnMouseEvent(wxMouseEvent& event)
> {
>     if (event.LeftDown())
>     {
>         wxWindow* parent = GetParent();
>         while (parent != NULL && !parent->IsKindOf(CLASSINFO(wxDialog)) && !parent->IsKindOf(CLASSINFO(wxFrame)))
>             parent = parent->GetParent();
> 
>         wxColourData data;
>         data.SetChooseFull(true);
>         data.SetColour(m_colour);
>         wxColourDialog *dialog = new wxColourDialog(parent, &data);
>         // Crashes on wxMac (no m_peer)
> #ifndef __WXMAC__
>         dialog->SetTitle(_("Background colour"));
> #endif
>         if (dialog->ShowModal() == wxID_OK)
>         {
>             wxColourData retData = dialog->GetColourData();
>             m_colour = retData.GetColour();
>             SetBackgroundColour(m_colour);
>         }
>         dialog->Destroy();
>         Refresh();
> 
>         wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
>         GetEventHandler()->ProcessEvent(event);
>     }
> }
> 
> /*!
>  * wxFontDialog type definition
>  */
> 
> IMPLEMENT_DYNAMIC_CLASS( wxFontDialog, wxDialog )
> 
> /*!
>  * wxFontDialog event table definition
>  */
> 
> BEGIN_EVENT_TABLE( wxFontDialog, wxDialog )
>     EVT_LISTBOX( wxID_FONTDIALOG_FACENAME, wxFontDialog::OnFontdialogFacenameSelected )
>     EVT_SPINCTRL( wxID_FONTDIALOG_FONTSIZE, wxFontDialog::OnFontdialogFontsizeUpdated )
>     EVT_TEXT( wxID_FONTDIALOG_FONTSIZE, wxFontDialog::OnFontdialogFontsizeTextUpdated )
>     EVT_CHECKBOX( wxID_FONTDIALOG_BOLD, wxFontDialog::OnFontdialogBoldClick )
>     EVT_CHECKBOX( wxID_FONTDIALOG_ITALIC, wxFontDialog::OnFontdialogItalicClick )
>     EVT_CHECKBOX( wxID_FONTDIALOG_UNDERLINED, wxFontDialog::OnFontdialogUnderlinedClick )
>     EVT_BUTTON( wxID_OK, wxFontDialog::OnOkClick )
>     EVT_BUTTON(wxID_FONTDIALOG_COLOUR, wxFontDialog::OnColourChanged)
> END_EVENT_TABLE()
> 
> /*!
>  * wxFontDialog constructors
>  */
> 
> wxFontDialog::wxFontDialog( )
57c389
< wxFontDialog::wxFontDialog(wxWindow *parent, const wxFontData&  data)
---
> wxFontDialog::wxFontDialog(wxWindow* parent, const wxFontData& fontData)
59c391,393
<     Create(parent, data);
---
>     m_dialogParent = NULL;
> 
>     Create(parent, fontData);
67c401,405
< void wxFontDialog::SetData(wxFontData& fontdata)
---
> /*!
>  * wxFontDialog creator
>  */
> 
> bool wxFontDialog::Create(wxWindow* parent, const wxFontData& fontData)
69c407,430
<     m_fontData = fontdata;
---
>     m_fontData = fontData;
>     m_suppressUpdates = false;
> 
>     wxString caption = _("Font");
> 
>     m_facenameCtrl = NULL;
>     m_sizeCtrl = NULL;
>     m_boldCtrl = NULL;
>     m_italicCtrl = NULL;
>     m_underlinedCtrl = NULL;
>     m_colourCtrl = NULL;
>     m_previewCtrl = NULL;
> 
>     InitializeFont();
> 
>     SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
>     wxDialog::Create( parent, wxID_ANY, caption, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
> 
>     CreateControls();
>     GetSizer()->Fit(this);
>     GetSizer()->SetSizeHints(this);
>     Centre();
> 
>     return true;
72c433,437
< bool wxFontDialog::Create(wxWindow *parent, const wxFontData& data)
---
> /*!
>  * Control creation for wxFontDialog
>  */
> 
> void wxFontDialog::CreateControls()
74c439
<     m_dialogParent = parent;
---
>     wxFontDialog* itemDialog1 = this;
76c441,565
<     m_fontData = data;
---
>     wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
>     itemDialog1->SetSizer(itemBoxSizer2);
> 
>     wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
>     itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 5);
> 
>     wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(6, 2, 10, 0);
>     itemFlexGridSizer4->AddGrowableRow(4);
>     itemFlexGridSizer4->AddGrowableCol(1);
>     itemBoxSizer3->Add(itemFlexGridSizer4, 1, wxGROW|wxALL, 5);
> 
>     wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Font:"), wxDefaultPosition, wxDefaultSize, 0 );
>     itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxALL|wxADJUST_MINSIZE, 5);
> 
>     wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
>     itemFlexGridSizer4->Add(itemBoxSizer6, 0, wxGROW|wxGROW, 5);
> 
>     wxString* m_facenameCtrlStrings = NULL;
>     m_facenameCtrl = new wxListBox( itemDialog1, wxID_FONTDIALOG_FACENAME, wxDefaultPosition, wxSize(320, 100), 0, m_facenameCtrlStrings, wxLB_SINGLE );
>     itemBoxSizer6->Add(m_facenameCtrl, 0, wxGROW|wxALL, 5);
> 
>     wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Size:"), wxDefaultPosition, wxDefaultSize, 0 );
>     itemFlexGridSizer4->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);
> 
>     m_sizeCtrl = new wxSpinCtrl( itemDialog1, wxID_FONTDIALOG_FONTSIZE, _T("12"), wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS, 1, 300, 12 );
>     m_sizeCtrl->SetHelpText(_("The font size in points."));
>     if (ShowToolTips())
>         m_sizeCtrl->SetToolTip(_("The font size in points."));
>     itemFlexGridSizer4->Add(m_sizeCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
> 
>     wxStaticText* itemStaticText10 = new wxStaticText( itemDialog1, wxID_STATIC, _("Style:"), wxDefaultPosition, wxDefaultSize, 0 );
>     itemFlexGridSizer4->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);
> 
>     wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
>     itemFlexGridSizer4->Add(itemBoxSizer11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
> 
>     m_boldCtrl = new wxCheckBox( itemDialog1, wxID_FONTDIALOG_BOLD, _("Bold"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
>     m_boldCtrl->SetValue(false);
>     m_boldCtrl->SetHelpText(_("Check to make the font bold."));
>     if (ShowToolTips())
>         m_boldCtrl->SetToolTip(_("Check to make the font bold."));
>     itemBoxSizer11->Add(m_boldCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
> 
>     m_italicCtrl = new wxCheckBox( itemDialog1, wxID_FONTDIALOG_ITALIC, _("Italic"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
>     m_italicCtrl->SetValue(false);
>     m_italicCtrl->SetHelpText(_("Check to make the font italic."));
>     if (ShowToolTips())
>         m_italicCtrl->SetToolTip(_("Check to make the font italic."));
>     itemBoxSizer11->Add(m_italicCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
> 
>     if (m_fontData.GetEnableEffects())
>     {
>         m_underlinedCtrl = new wxCheckBox( itemDialog1, wxID_FONTDIALOG_UNDERLINED, _("Underlined"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
>         m_underlinedCtrl->SetValue(false);
>         m_underlinedCtrl->SetHelpText(_("Check to make the font underlined."));
>         if (ShowToolTips())
>             m_underlinedCtrl->SetToolTip(_("Check to make the font underlined."));
>         itemBoxSizer11->Add(m_underlinedCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
>     }
> 
>     if (m_fontData.GetEnableEffects())
>     {
>         wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_STATIC, _("Colour:"), wxDefaultPosition, wxDefaultSize, 0 );
>         itemFlexGridSizer4->Add(itemStaticText15, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxALL|wxADJUST_MINSIZE, 5);
> 
>         m_colourCtrl = new wxFontColourSwatchCtrl( itemDialog1, wxID_FONTDIALOG_COLOUR, wxDefaultPosition, wxSize(-1, 30), wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
>         m_colourCtrl->SetHelpText(_("Click to change the font colour."));
>         if (ShowToolTips())
>             m_colourCtrl->SetToolTip(_("Click to change the font colour."));
>         itemFlexGridSizer4->Add(m_colourCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
>     }
> 
>     wxStaticText* itemStaticText17 = new wxStaticText( itemDialog1, wxID_STATIC, _("Preview:"), wxDefaultPosition, wxDefaultSize, 0 );
>     itemFlexGridSizer4->Add(itemStaticText17, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxALL|wxADJUST_MINSIZE, 5);
> 
>     m_previewCtrl = new wxFontPreviewCtrl( itemDialog1, wxID_FONTDIALOG_PREVIEW, wxDefaultPosition, wxSize(-1, 70), wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
>     m_previewCtrl->SetHelpText(_("Shows a preview of the font."));
>     if (ShowToolTips())
>         m_previewCtrl->SetToolTip(_("Shows a preview of the font."));
>     itemFlexGridSizer4->Add(m_previewCtrl, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
> 
>     wxBoxSizer* itemBoxSizer19 = new wxBoxSizer(wxHORIZONTAL);
>     itemBoxSizer3->Add(itemBoxSizer19, 0, wxALIGN_RIGHT|wxALL, 5);
> 
>     wxButton* itemButton20 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
>     itemButton20->SetHelpText(_("Click to cancel changes to the font."));
>     if (ShowToolTips())
>         itemButton20->SetToolTip(_("Click to cancel changes to the font."));
>     itemBoxSizer19->Add(itemButton20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
> 
>     wxButton* itemButton21 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
>     itemButton21->SetDefault();
>     itemButton21->SetHelpText(_("Click to confirm changes to the font."));
>     if (ShowToolTips())
>         itemButton21->SetToolTip(_("Click to confirm changes to the font."));
>     itemBoxSizer19->Add(itemButton21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
> 
>     wxFontEnumerator enumerator;
>     enumerator.EnumerateFacenames();
>     wxArrayString facenames = enumerator.GetFacenames();
>     if (facenames)
>     {
>         facenames.Add(_("<Any>"));
>         facenames.Add(_("<Any Roman>"));
>         facenames.Add(_("<Any Decorative>"));
>         facenames.Add(_("<Any Modern>"));
>         facenames.Add(_("<Any Script>"));
>         facenames.Add(_("<Any Swiss>"));
>         facenames.Add(_("<Any Teletype>"));
>         facenames.Sort();
>         m_facenameCtrl->Append(facenames);
>     }
> 
>     InitializeControls();
>     m_previewCtrl->SetFont(m_dialogFont);
>     if (m_fontData.GetColour().Ok())
>     {
>         m_previewCtrl->SetForegroundColour(m_fontData.GetColour());
>     }
>     m_previewCtrl->Refresh();
> }
> 
> /*!
>  * wxEVT_COMMAND_SPINCTRL_UPDATED event handler for wxID_FONTDIALOG_FONTSIZE
>  */
78,80c567,728
<     // TODO: you may need to do dialog creation here, unless it's
<     // done in ShowModal.
<     return TRUE;
---
> void wxFontDialog::OnFontdialogFontsizeUpdated( wxSpinEvent& WXUNUSED(event) )
> {
>     ChangeFont();
> }
> 
> /*!
>  * wxEVT_COMMAND_TEXT_UPDATED event handler for wxID_FONTDIALOG_FONTSIZE
>  */
> 
> void wxFontDialog::OnFontdialogFontsizeTextUpdated( wxCommandEvent& WXUNUSED(event) )
> {
>     ChangeFont();
> }
> 
> /*!
>  * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for wxID_FONTDIALOG_BOLD
>  */
> 
> void wxFontDialog::OnFontdialogBoldClick( wxCommandEvent& WXUNUSED(event) )
> {
>     ChangeFont();
> }
> 
> /*!
>  * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for wxID_FONTDIALOG_ITALIC
>  */
> 
> void wxFontDialog::OnFontdialogItalicClick( wxCommandEvent& WXUNUSED(event) )
> {
>     ChangeFont();
> }
> 
> /*!
>  * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for wxID_FONTDIALOG_UNDERLINED
>  */
> 
> void wxFontDialog::OnFontdialogUnderlinedClick( wxCommandEvent& WXUNUSED(event) )
> {
>     ChangeFont();
> }
> 
> /*!
>  * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
>  */
> 
> void wxFontDialog::OnOkClick( wxCommandEvent& event )
> {
>     event.Skip();
> }
> 
> 
> /*!
>  * wxEVT_COMMAND_LISTBOX_SELECTED event handler for wxID_FONTDIALOG_FACENAME
>  */
> 
> void wxFontDialog::OnFontdialogFacenameSelected( wxCommandEvent& WXUNUSED(event) )
> {
>     ChangeFont();
> }
> 
> void wxFontDialog::OnColourChanged(wxCommandEvent & WXUNUSED(event))
> {
>     m_fontData.SetColour(m_colourCtrl->GetColour());
>     m_previewCtrl->SetForegroundColour(m_colourCtrl->GetColour());
>     m_previewCtrl->Refresh();
> }
> 
> /*!
>  * Should we show tooltips?
>  */
> 
> bool wxFontDialog::ShowToolTips()
> {
>     return true;
> }
> 
> void wxFontDialog::InitializeFont()
> {
>     int fontFamily = wxSWISS;
>     int fontWeight = wxNORMAL;
>     int fontStyle = wxNORMAL;
>     int fontSize = 12;
>     bool fontUnderline = false;
>     wxString fontName;
> 
>     if (m_fontData.m_initialFont.Ok())
>     {
>         fontFamily = m_fontData.m_initialFont.GetFamily();
>         fontWeight = m_fontData.m_initialFont.GetWeight();
>         fontStyle = m_fontData.m_initialFont.GetStyle();
>         fontSize = m_fontData.m_initialFont.GetPointSize();
>         fontUnderline = m_fontData.m_initialFont.GetUnderlined();
>         fontName = m_fontData.m_initialFont.GetFaceName();
>     }
> 
>     m_dialogFont = wxFont(fontSize, fontFamily, fontStyle,
>                           fontWeight, fontUnderline, fontName);
> 
>     if (m_previewCtrl)
>         m_previewCtrl->SetFont(m_dialogFont);
> 
>     m_fontData.SetChosenFont(m_dialogFont);
> }
> 
> /// Set controls according to current font
> void wxFontDialog::InitializeControls()
> {
>     m_suppressUpdates = true;
> 
>     if (m_underlinedCtrl)
>         m_underlinedCtrl->SetValue(m_dialogFont.GetUnderlined());
> 
>     m_boldCtrl->SetValue(m_dialogFont.GetWeight() == wxBOLD);
>     m_italicCtrl->SetValue(m_dialogFont.GetStyle() == wxITALIC);
>     m_sizeCtrl->SetValue(m_dialogFont.GetPointSize());
> 
>     wxString facename = m_dialogFont.GetFaceName();
>     if (facename.empty() || m_facenameCtrl->FindString(facename) == wxNOT_FOUND)
>     {
>         facename = FontFamilyIntToString(m_dialogFont.GetFamily());
>     }
>     m_facenameCtrl->SetStringSelection(facename);
> 
>     if (m_colourCtrl && m_fontData.GetColour().Ok())
>     {
>         m_colourCtrl->SetColour(m_fontData.GetColour());
>         m_colourCtrl->Refresh();
>     }
> 
>     m_suppressUpdates = false;
> }
> 
> /// Respond to font change
> void wxFontDialog::ChangeFont()
> {
>     if (m_suppressUpdates)
>         return;
> 
>     bool underlined = m_underlinedCtrl ? m_underlinedCtrl->GetValue() : false;
>     bool italic = m_italicCtrl->GetValue();
>     bool bold = m_boldCtrl->GetValue();
>     int size = m_sizeCtrl->GetValue();
>     wxString facename = m_facenameCtrl->GetStringSelection();
> 
>     int family = FontFamilyStringToInt(facename);
>     if (family == -1)
>         family = wxDEFAULT;
>     else
>         facename = wxEmptyString;
> 
>     m_dialogFont = wxFont(size, family, italic ? wxITALIC : wxNORMAL, bold ? wxBOLD : wxNORMAL,
>         underlined, facename);
> 
>     m_fontData.SetChosenFont(m_dialogFont);
> 
>     m_previewCtrl->SetFont(m_dialogFont);
>     m_previewCtrl->Refresh();
> }
> 
> void wxFontDialog::SetData(const wxFontData& fontdata)
> {
>     m_fontData = fontdata;
90,91c738
<     // TODO: show (maybe create) the dialog
<     return wxID_CANCEL;
---
>     return wxDialog::ShowModal();
94c741,789
< #endif // 10.2+
---
> void wxFontDialog::OnPanelClose()
> {
> }
> 
> const wxChar *FontFamilyIntToString(int family)
> {
>     switch (family)
>     {
>         case wxROMAN:
>             return _("<Any Roman>");
>         case wxDECORATIVE:
>             return _("<Any Decorative>");
>         case wxMODERN:
>             return _("<Any Modern>");
>         case wxSCRIPT:
>             return _("<Any Script>");
>         case wxTELETYPE:
>             return _("<Any Teletype>");
>         case wxSWISS:
>         default:
>             return _("<Any Swiss>");
>     }
> }
> 
> int FontFamilyStringToInt(const wxChar *family)
> {
>     if (!family)
>         return wxSWISS;
> 
>     if (wxStrcmp(family, _("<Any Roman>")) == 0)
>         return wxROMAN;
>     else if (wxStrcmp(family, _("<Any Decorative>")) == 0)
>         return wxDECORATIVE;
>     else if (wxStrcmp(family, _("<Any Modern>")) == 0)
>         return wxMODERN;
>     else if (wxStrcmp(family, _("<Any Script>")) == 0)
>         return wxSCRIPT;
>     else if (wxStrcmp(family, _("<Any Teletype>")) == 0)
>         return wxTELETYPE;
>     else if (wxStrcmp(family, _("<Any Swiss>")) == 0)
>         return wxSWISS;
>     else return -1;
> }
> 
> #endif // !USE_NATIVE_FONT_DIALOG_FOR_MACOSX
> 
> #endif // wxMAC_USE_EXPERIMENTAL_FONTDIALOG
> 
> #endif // wxUSE_FONTDLG


$ diff wxMac-2.6.4/include/wx/mac/carbon/fontdlg.h wxMac-2.8.8/include/wx/mac/carbon/fontdlg.h

2,3c2,3
< // Name:        fontdlg.h
< // Purpose:     wxFontDialog class using fonts window services (10.2+). 
---
> // Name:        wx/mac/carbon/fontdlg.h
> // Purpose:     wxFontDialog class using fonts window services (10.2+).
7c7
< // RCS-ID:      $Id: fontdlg.h,v 1.5 2004/09/26 14:13:20 RN Exp $
---
> // RCS-ID:      $Id: fontdlg.h 43384 2006-11-12 23:34:38Z VZ $
15,18d14
< #if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
< #pragma interface "fontdlg.h"
< #endif
< 
25c21,84
<  
---
> 
> #ifndef wxMAC_USE_EXPERIMENTAL_FONTDIALOG
> #if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_2
> #define wxMAC_USE_EXPERIMENTAL_FONTDIALOG 0
> #else
> #define wxMAC_USE_EXPERIMENTAL_FONTDIALOG 1
> #endif
> #endif
> 
> #if wxMAC_USE_EXPERIMENTAL_FONTDIALOG
> 
> class WXDLLEXPORT wxFontDialog : public wxDialog
> {
> public:
>     wxFontDialog();
>     wxFontDialog(wxWindow *parent, const wxFontData& data);
>     virtual ~wxFontDialog();
> 
>     bool Create(wxWindow *parent, const wxFontData& data);
> 
>     int ShowModal();
>     wxFontData& GetFontData() { return m_fontData; }
> 
> protected:
>     wxFontData m_fontData;
> 
>     DECLARE_DYNAMIC_CLASS_NO_COPY(wxFontDialog)
> };
> 
> extern "C" int RunMixedFontDialog(wxFontDialog* dialog) ;
> 
> #else // wxMAC_USE_EXPERIMENTAL_FONTDIALOG
> 
> #if !USE_NATIVE_FONT_DIALOG_FOR_MACOSX
> 
> /*!
>  * Forward declarations
>  */
> 
> class wxFontColourSwatchCtrl;
> class wxFontPreviewCtrl;
> class WXDLLEXPORT wxSpinCtrl;
> class WXDLLEXPORT wxSpinEvent;
> class WXDLLEXPORT wxListBox;
> class WXDLLEXPORT wxChoice;
> class WXDLLEXPORT wxButton;
> class WXDLLEXPORT wxStaticText;
> class WXDLLEXPORT wxCheckBox;
> 
> /*!
>  * Control identifiers
>  */
> 
> #define wxID_FONTDIALOG_FACENAME 20001
> #define wxID_FONTDIALOG_FONTSIZE 20002
> #define wxID_FONTDIALOG_BOLD 20003
> #define wxID_FONTDIALOG_ITALIC 20004
> #define wxID_FONTDIALOG_UNDERLINED 20005
> #define wxID_FONTDIALOG_COLOUR 20006
> #define wxID_FONTDIALOG_PREVIEW 20007
> 
> #endif
>     // !USE_NATIVE_FONT_DIALOG_FOR_MACOSX
> 
29a89,92
> #if !USE_NATIVE_FONT_DIALOG_FOR_MACOSX
> DECLARE_EVENT_TABLE()
> #endif
> 
33c96
<     ~wxFontDialog();
---
>     virtual ~wxFontDialog();
41c104,159
<     void SetData(wxFontData& data);
---
>     void SetData(const wxFontData& data);
> 
> #if !USE_NATIVE_FONT_DIALOG_FOR_MACOSX
> 
>     /// Creates the controls and sizers
>     void CreateControls();
> 
>     /// Initialize font
>     void InitializeFont();
> 
>     /// Set controls according to current font
>     void InitializeControls();
> 
>     /// Respond to font change
>     void ChangeFont();
> 
>     /// Respond to colour change
>     void OnColourChanged(wxCommandEvent& event);
> 
>     /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for wxID_FONTDIALOG_FACENAME
>     void OnFontdialogFacenameSelected( wxCommandEvent& event );
> 
>     /// wxEVT_COMMAND_SPINCTRL_UPDATED event handler for wxID_FONTDIALOG_FONTSIZE
>     void OnFontdialogFontsizeUpdated( wxSpinEvent& event );
> 
>     /// wxEVT_COMMAND_TEXT_UPDATED event handler for wxID_FONTDIALOG_FONTSIZE
>     void OnFontdialogFontsizeTextUpdated( wxCommandEvent& event );
> 
>     /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for wxID_FONTDIALOG_BOLD
>     void OnFontdialogBoldClick( wxCommandEvent& event );
> 
>     /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for wxID_FONTDIALOG_ITALIC
>     void OnFontdialogItalicClick( wxCommandEvent& event );
> 
>     /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for wxID_FONTDIALOG_UNDERLINED
>     void OnFontdialogUnderlinedClick( wxCommandEvent& event );
> 
>     /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
>     void OnOkClick( wxCommandEvent& event );
> 
>     /// Should we show tooltips?
>     static bool ShowToolTips();
> 
>     wxListBox* m_facenameCtrl;
>     wxSpinCtrl* m_sizeCtrl;
>     wxCheckBox* m_boldCtrl;
>     wxCheckBox* m_italicCtrl;
>     wxCheckBox* m_underlinedCtrl;
>     wxFontColourSwatchCtrl* m_colourCtrl;
>     wxFontPreviewCtrl* m_previewCtrl;
> 
>     wxFont      m_dialogFont;
>     bool        m_suppressUpdates;
> 
> #endif
>     // !USE_NATIVE_FONT_DIALOG_FOR_MACOSX
46c164
<     void*		m_pEventHandlerRef;
---
>     void*       m_pEventHandlerRef;
50d167
<     // _WX_FONTDLG_H_
51a169,170
> #endif
>     // _WX_FONTDLG_H_


$ diff wxMac-2.6.4/src/mac/carbon/fontdlgosx.mm wxMac-2.8.8/src/mac/carbon/fontdlgosx.mm

2c2
< // Name:        fontdlg.cpp
---
> // Name:        src/mac/carbon/fontdlgosx.cpp
7c7
< // RCS-ID:      $Id: fontdlgosx.mm,v 1.6 2005/07/28 22:08:10 VZ Exp $
---
> // RCS-ID:      $Id: fontdlgosx.mm 39337 2006-05-25 21:08:11Z ABX $
11a12,13
> #include "wx/wxprec.h"
> 
20d21
< #include "wx/cmndata.h"
21a23,29
> 
> #ifndef WX_PRECOMP
>     #include "wx/intl.h"
>     #include "wx/log.h"
>     #include "wx/cmndata.h"
> #endif
> 
23,24c31
< #include "wx/log.h"
<     
---
> 
29c36,191
< //Mac OSX 10.2+ only
---
> 
> #include "wx/cocoa/autorelease.h"
> #include "wx/cocoa/string.h"
> 
> #if wxMAC_USE_EXPERIMENTAL_FONTDIALOG
> 
> #import <Foundation/Foundation.h>
> #import <AppKit/AppKit.h>
> 
> #include "wx/mac/uma.h"
> 
> @interface wxMacFontPanelAccView : NSView
> {
>     BOOL m_okPressed ;
>     BOOL m_shouldClose ;
>     NSButton* m_cancelButton ;
>     NSButton* m_okButton ;
> }
> 
> - (IBAction)cancelPressed:(id)sender;
> - (IBAction)okPressed:(id)sender;
> - (void)resetFlags;
> - (BOOL)closedWithOk;
> - (BOOL)shouldCloseCarbon;
> - (NSButton*)okButton;
> @end
> 
> @implementation wxMacFontPanelAccView : NSView
> - (id)initWithFrame:(NSRect)rectBox
> {
>     [super initWithFrame:rectBox];
> 
>     wxMacCFStringHolder cfOkString( wxT("OK"), wxLocale::GetSystemEncoding() );
>     wxMacCFStringHolder cfCancelString( wxT("Cancel"), wxLocale::GetSystemEncoding() );
> 
>     NSRect rectCancel = NSMakeRect( 10.0 , 10.0 , 82  , 24 );
>     NSRect rectOK = NSMakeRect( 100.0 , 10.0 , 82  , 24 );
> 
>     NSButton* cancelButton = [[NSButton alloc] initWithFrame:rectCancel];
>     [cancelButton setTitle:(NSString*)cfCancelString.Detach()];
>     [cancelButton setBezelStyle:NSRoundedBezelStyle];
>     [cancelButton setButtonType:NSMomentaryPushInButton];
>     [cancelButton setAction:@selector(cancelPressed:)];
>     [cancelButton setTarget:self];
>     m_cancelButton = cancelButton ;
> 
>     NSButton* okButton = [[NSButton alloc] initWithFrame:rectOK];
>     [okButton setTitle:(NSString*)cfOkString.Detach()];
>     [okButton setBezelStyle:NSRoundedBezelStyle];
>     [okButton setButtonType:NSMomentaryPushInButton];
>     [okButton setAction:@selector(okPressed:)];
>     [okButton setTarget:self];
>     // doesn't help either, the button is not highlighted after a color dialog has been used
>     // [okButton setKeyEquivalent:@"\r"];
>     m_okButton = okButton ;
> 
> 
>     [self addSubview:cancelButton];
>     [self addSubview:okButton];
> 
>     [self resetFlags];
>     return self;
> }
> 
> - (void)resetFlags
> {
>     m_okPressed = NO ;
>     m_shouldClose = NO ;
> }
> 
> - (IBAction)cancelPressed:(id)sender
> {
>     m_shouldClose = YES ;
>     [NSApp stopModal];
> }
> 
> - (IBAction)okPressed:(id)sender
> {
>     m_okPressed = YES ;
>     m_shouldClose = YES ;
>     [NSApp stopModal];
> }
> 
> -(BOOL)closedWithOk
> {
>     return m_okPressed ;
> }
> 
> -(BOOL)shouldCloseCarbon
> {
>     return m_shouldClose ;
> }
> 
> -(NSButton*)okButton
> {
>     return m_okButton ;
> }
> @end
> 
> 
> extern "C" int RunMixedFontDialog(wxFontDialog* dialog) ;
> 
> int RunMixedFontDialog(wxFontDialog* dialog)
> {
>     int retval = wxID_CANCEL ;
> 
>     bool cocoaLoaded = NSApplicationLoad();
>     wxASSERT_MSG(cocoaLoaded,wxT("Couldn't load Cocoa in Carbon Environment")) ;
> 
>     wxAutoNSAutoreleasePool pool;
> 
>     // setting up the ok/cancel buttons
>     NSFontPanel* fontPanel = [NSFontPanel sharedFontPanel] ;
> 
>     // adjust modality for carbon environment
>     WindowRef carbonWindowRef = (WindowRef)[fontPanel windowRef] ;
>     SetWindowModality(carbonWindowRef, kWindowModalityAppModal , 0) ;
>     SetWindowGroup(carbonWindowRef , GetWindowGroupOfClass(kMovableModalWindowClass));
> 
>     [fontPanel setFloatingPanel:NO] ;
>     [[fontPanel standardWindowButton:NSWindowCloseButton] setEnabled:NO] ;
> 
>     wxMacFontPanelAccView* accessoryView = (wxMacFontPanelAccView*) [fontPanel accessoryView] ;
>     if ( accessoryView == nil)
>     {
>         NSRect rectBox = NSMakeRect( 0 , 0 , 192 , 40 );
>         accessoryView = [[wxMacFontPanelAccView alloc] initWithFrame:rectBox];
>         [fontPanel setAccessoryView:accessoryView];
> 
>         [fontPanel setDefaultButtonCell:[[accessoryView okButton] cell]] ;
>     }
> 
>     [accessoryView resetFlags];
> 
>     NSModalSession session = [NSApp beginModalSessionForWindow:fontPanel];
> 
>     [NSApp runModalSession:session];
> 
>     [NSApp endModalSession:session];
> 
>     // if we don't reenable it, FPShowHideFontPanel does not work
>     [[fontPanel standardWindowButton:NSWindowCloseButton] setEnabled:YES] ;
>     if( FPIsFontPanelVisible())
>         FPShowHideFontPanel() ;
> 
>     if ( [accessoryView closedWithOk])
>     {
>         retval = wxID_OK ;
>     }
> 
> 
>     return retval ;
> }
> 
> #else
> 
35,36d196
< #include "wx/cocoa/autorelease.h"
< #include "wx/cocoa/string.h"
72c232
<     
---
> 
118c278
<     
---
> 
170,172c330,332
<     //	This is the key call - this initializes
<     //	events and window stuff for cocoa for carbon
<     //	applications.
---
>     //  This is the key call - this initializes
>     //  events and window stuff for cocoa for carbon
>     //  applications.
174,177c334,337
<     //	This is also the only call here that is 
<     //	10.2+ specific (the rest is OSX only),
<     //	which, ironically, the carbon font
<     //	panel requires.
---
>     //  This is also the only call here that is
>     //  10.2+ specific (the rest is OSX only),
>     //  which, ironically, the carbon font
>     //  panel requires.
187,188c347,348
<     
<     //if the font is valid set the default (selected) font of the 
---
> 
>     //if the font is valid set the default (selected) font of the
193c353
<     
---
> 
196c356
<     
---
> 
200c360
<         NSFont* theDefaultFont = 
---
>         NSFont* theDefaultFont =
208c368
<             
---
> 
226,227c386,387
<         [[NSColorPanel sharedColorPanel] setColor:[NSColor blackColor]];   
<         
---
>         [[NSColorPanel sharedColorPanel] setColor:[NSColor blackColor]];
> 
230c390
<     
---
> 
235,236c395,396
< {    
<     //Start the pool.  Required for carbon interaction 
---
> {
>     //Start the pool.  Required for carbon interaction
245c405
<     NSFontPanel* theFontPanel 	= [NSFontPanel sharedFontPanel];
---
>     NSFontPanel* theFontPanel = [NSFontPanel sharedFontPanel];
255c415
<       
---
> 
257c417
<     //	Begin the modal loop for the font and color panels
---
>     //  Begin the modal loop for the font and color panels
259c419
<     //	The idea is that we first make the font panel modal,
---
>     //  The idea is that we first make the font panel modal,
264,265c424,425
<     //	So we set up delegates for both the color and font panels,
<     //  and the if the font panel opens the color panel, we 
---
>     //  So we set up delegates for both the color and font panels,
>     //  and the if the font panel opens the color panel, we
273c433
<         //	Start the font panel modal loop
---
>         //  Start the font panel modal loop
276c436
<         for (;;) 
---
>         for (;;)
279c439
<             
---
> 
286c446
<         
---
> 
291c451
<             //	Start the color panel modal loop
---
>             //  Start the color panel modal loop
294c454
<             for (;;) 
---
>             for (;;)
297c457
<                 
---
> 
308c468
<     
---
> 
312c472
<     
---
> 
314,315c474,475
<     NSFont* theFont = [theFontPanel panelConvertFont:[NSFont userFontOfSize:0]];      
<     
---
>     NSFont* theFont = [theFontPanel panelConvertFont:[NSFont userFontOfSize:0]];
> 
320c480
<     
---
> 
324c484
<        
---
> 
329,330c489,490
<                                     theFontWeight >= 9 ? wxBOLD : wxNORMAL);    
<                                         
---
>                                     theFontWeight >= 9 ? wxBOLD : wxNORMAL);
> 
333c493
<                                     
---
> 
340,341c500,501
<     wxPrintf(wxT("---Font Panel---\n--NS--\nSize:%f\nWeight:%i\nTraits:%i\n--WX--\nFaceName:%s\nPointSize:%i\nStyle:%i\nWeight:%i\nColor:%i,%i,%i\n---END Font Panel---\n"), 
<     
---
>     wxPrintf(wxT("---Font Panel---\n--NS--\nSize:%f\nWeight:%i\nTraits:%i\n--WX--\nFaceName:%s\nPointSize:%i\nStyle:%i\nWeight:%i\nColor:%i,%i,%i\n---END Font Panel---\n"),
> 
345c505
<                 
---
> 
350,351c510,511
<                     m_fontData.m_fontColour.Red(), 
<                     m_fontData.m_fontColour.Green(), 
---
>                     m_fontData.m_fontColour.Red(),
>                     m_fontData.m_fontColour.Green(),
369a530,531
> 
> #endif


$ diff wxMac-2.6.4/include/wx/mac/carbon/private.h wxMac-2.8.8/include/wx/mac/carbon/private.h

# snipped everything but wxMacGetEventParamType

203,211c217,229
< template<> inline EventParamType wxMacGetEventParamType<Boolean>() { return typeBoolean ; }
---
> template<> inline EventParamType wxMacGetEventParamType<Boolean>() { return typeBoolean; }
> template<> inline EventParamType wxMacGetEventParamType<SInt16>() { return typeSInt16; }
> template<> inline EventParamType wxMacGetEventParamType<SInt32>() { return typeSInt32; }
> template<> inline EventParamType wxMacGetEventParamType<UInt32>() { return typeUInt32; }
> template<> inline EventParamType wxMacGetEventParamType<RGBColor>() { return typeRGBColor; }
219,220c237,241
< template<> inline EventParamType wxMacGetEventParamType<Collection>() { return typeCollection ; }
< template<> inline EventParamType wxMacGetEventParamType<CGContextRef>() { return typeCGContextRef ; }
---
> #if TARGET_API_MAC_OSX && ( MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_2 )
> template<> inline EventParamType wxMacGetEventParamType<CFDictionaryRef>() { return typeCFDictionaryRef; }
> #endif
> template<> inline EventParamType wxMacGetEventParamType<Collection>() { return typeCollection; }
> template<> inline EventParamType wxMacGetEventParamType<CGContextRef>() { return typeCGContextRef; }
