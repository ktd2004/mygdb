// vim: set fdm=marker ts=4 sw=4:

//
// IME: 
// http://doloopwhile.blogspot.com/2009_02_08_archive.html
// http://miau.s9.xrea.com/blog/index.php?itemid=876
// http://ultrauseless.spaces.live.com/blog/cns!63D20CF80D9772C7!270.entry
// http://oosugi.net/read/pc12.2ch.net/tech/1214657360/-100
//
#pragma warning(disable: 4819)
#pragma warning(disable: 4100)

#include <MyGDB.h>

#include "xpm/arrow_right_green.xpm"
#include "xpm/breakpoint.xpm"
#include "xpm/collapsed.xpm"
#include "xpm/expanded.xpm"

BEGIN_EVENT_TABLE(StyledTextCtrl, wxStyledTextCtrl)
    EVT_STC_MARGINCLICK(wxID_ANY, StyledTextCtrl::OnMarginClicked)
	EVT_STC_UPDATEUI (wxID_ANY, StyledTextCtrl::OnUpdateUI)
	EVT_STC_DWELLSTART (wxID_ANY, StyledTextCtrl::OnShowValue)
	EVT_STC_DWELLEND (wxID_ANY, StyledTextCtrl::OnHideValue)
	EVT_LEAVE_WINDOW(StyledTextCtrl::OnLeave)
	EVT_ENTER_WINDOW(StyledTextCtrl::OnEnter)
END_EVENT_TABLE()

StyledTextCtrl::StyledTextCtrl(CodeEditor* parent,
	MyGdbFrame *myFrame,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style)
	: wxStyledTextCtrl(parent, id, pos, size, style)
{
	this->myParent = parent;
	this->myFrame = myFrame;

	m_focusOut = false;
	m_closing = false;

	SetLexer(wxSTC_LEX_CPP);
	SetMouseDwellTime(250);
	UsePopUp(false);
	SetCodePage(wxSTC_CP_UTF8);
	SetUseAntiAliasing(false);

#if 1
	// Setting default newline
#ifdef __MINGW32__
	SetEOLMode(wxSTC_EOL_CRLF);
#else
	// Linux
	SetEOLMode(wxSTC_EOL_LF);
#endif
#endif
	
	SetMarginType(NUMBER_MARGIN_ID, wxSTC_MARGIN_NUMBER);
	SetMarginType(SYMBOLS_MARGIN_ID, wxSTC_MARGIN_SYMBOL);

	// margin for bookmarks, breakpoints
	SetMarginMask(NUMBER_MARGIN_ID, ~(256 | 512 | 128 | wxSTC_MASK_FOLDERS));

	// Fold margin - allow only folder symbols to display
	SetMarginMask(FOLD_MARGIN_ID, wxSTC_MASK_FOLDERS);

	// allow everything except for the folding symbols
	SetMarginMask(SYMBOLS_MARGIN_ID, ~(wxSTC_MASK_FOLDERS));

	// Mark fold margin & symbols margins as sensetive
	SetMarginSensitive(NUMBER_MARGIN_ID, true);
	SetMarginSensitive(FOLD_MARGIN_ID, true);
	SetMarginSensitive(SYMBOLS_MARGIN_ID, true);
	
	// debugger current line marker
	wxImage img(arrow_right_green_xpm);
	wxBitmap bmp(img);
	MarkerDefineBitmap(DEBUG_MARKER, bmp);
	MarkerSetBackground(DEBUG_MARKER, wxT("LIME GREEN"));
	//MarkerSetForeground(DEBUG_MARKER, wxT("BLACK"));
	
	// debugger breakpoint marker
	wxImage img2(stop_xpm);
	wxBitmap bmp2(img2);
	MarkerDefineBitmap(BREAKPOINT_MARKER, bmp2);
	MarkerSetBackground(BREAKPOINT_MARKER, wxT("RED"));

	CallTipUseStyle(40); //enable custom calltip styling 
	wxColour color = wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK);
	wxString back=wxEmptyString;
    back += wxDecToHex(color.Red());
    back += wxDecToHex(color.Green());
    back += wxDecToHex(color.Blue());
	StyleSetSpec(wxSTC_STYLE_CALLTIP, 
			wxT("fore:#000000,back:#") + back); 
	wxFont font = myFrame->GetDockArt()->GetFont(wxAUI_DOCKART_CAPTION_FONT);
	StyleSetFont(wxSTC_STYLE_CALLTIP, font); 
	
	// fold margin
	SetProperty(wxT("fold"), wxT("1"));
	SetProperty(wxT("fold.comment"), wxT("1"));
	SetProperty(wxT("fold.compact"), wxT("1"));
	SetProperty(wxT("fold.preprocessor"), wxT("1"));
	SetProperty(wxT("fold.at.else"), wxT("1"));
	SetProperty(wxT("tcl.default.language"), wxT("1"));

	// show folds
	SetFoldFlags(16);
	SetMarginType(2, wxSTC_MARGIN_SYMBOL);
	SetMarginWidth(2, 16);
	SetMarginMask(2, wxSTC_MASK_FOLDERS);
	SetMarginSensitive(2, 1);
	
	CmdKeyAssign('+', wxSTC_SCMOD_CTRL, wxSTC_CMD_ZOOMIN);
	CmdKeyAssign('-', wxSTC_SCMOD_CTRL, wxSTC_CMD_ZOOMOUT);
	
	MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_VLINE);
	MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_LCORNER);
	MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNER);
	
	// -
	wxImage img3(expanded_xpm);
	wxBitmap bmp3(img3);
	MarkerDefineBitmap(wxSTC_MARKNUM_FOLDEROPEN, bmp3);
	MarkerDefineBitmap(wxSTC_MARKNUM_FOLDEROPENMID, bmp3);
	MarkerSetBackground(BREAKPOINT_MARKER, wxT("RED"));
	
	// +
	wxImage img4(collapsed_xpm);
	wxBitmap bmp4(img4);
	MarkerDefineBitmap(wxSTC_MARKNUM_FOLDER, bmp4);
	MarkerDefineBitmap(wxSTC_MARKNUM_FOLDEREND, bmp4);
	MarkerSetBackground(BREAKPOINT_MARKER, wxT("RED"));

	SetStyles();
	SetProperties();

	SetReadOnly(true);
}

StyledTextCtrl::~StyledTextCtrl () 
{
}

void StyledTextCtrl::SetStyles()
{
	// editor
	wxString styleName = CONFIG->GetProperty(wxT("EDITOR/STYLE"), wxT("value"));
	wxXmlDocument *xml = STC_STYLE->OpenStyle(styleName);
	STC_STYLE->ApplyEditorStyle(this);
	STC_STYLE->CloseStyle(xml);
}

void StyledTextCtrl::SetProperties(void)
{
	bool bValue = CONFIG->GetPropertyBool(wxT("EDITOR/GENERAL/USETABCHAR"), wxT("value"));
	if ( bValue )
		SetUseTabs(true);
	else
		SetUseTabs(false);

	long iValue = CONFIG->GetPropertyInt(wxT("EDITOR/GENERAL/TABSIZE"), wxT("value"));
	SetTabWidth(iValue);
	
	bValue = CONFIG->GetPropertyBool(wxT("EDITOR/GENERAL/TABINDENTS"), wxT("value"));
	if ( bValue )
		SetTabIndents(true);
	else
		SetTabIndents(false);

	bValue = CONFIG->GetPropertyBool(wxT("EDITOR/GENERAL/SHOWEDGE"), wxT("value"));
	if ( bValue )
		SetEdgeMode(wxSTC_EDGE_LINE);
	else
		SetEdgeMode(wxSTC_EDGE_NONE);
	
	iValue = CONFIG->GetPropertyInt(wxT("EDITOR/GENERAL/EDGECOLUMN"), wxT("value"));
	SetEdgeColumn(iValue);

	SetBackSpaceUnIndents (true);
	
	bValue = CONFIG->GetPropertyBool(wxT("EDITOR/GENERAL/INDENTGUIDES"), wxT("value"));
	if ( bValue )
		SetIndentationGuides(true);
	else
		SetIndentationGuides(false);
	
	bValue = CONFIG->GetPropertyBool(wxT("EDITOR/GENERAL/LINECARET"), wxT("value"));
	if ( bValue )
		SetCaretLineVisible(true);
	else
		SetCaretLineVisible(false);

	bValue = CONFIG->GetPropertyBool(wxT("EDITOR/GENERAL/WORDWRAP"), wxT("value"));
	if ( bValue )
		SetWrapMode(wxSTC_WRAP_WORD);
	else
		SetWrapMode(wxSTC_WRAP_NONE);
	
	bValue = CONFIG->GetPropertyBool(wxT("EDITOR/GENERAL/LINENUMBER"), wxT("value"));
	if ( bValue )
	{
		wxString temp;
		temp = temp.Format(wxT("%d0"), GetLineCount());
		int width = TextWidth(wxSTC_STYLE_LINENUMBER, temp);
		SetMarginWidth(NUMBER_MARGIN_ID, width);
	}
	else
	{
		SetMarginWidth(NUMBER_MARGIN_ID, 0);
	}

	SetMarginWidth(SYMBOLS_MARGIN_ID, 16);

	myParent->UpdateTitle();
}

void StyledTextCtrl::OnMarginClicked(wxStyledTextEvent& event)
{
    switch (event.GetMargin())
    {
		case 0: // line number margin
        case 1: // bookmarks and breakpoints margin
        {
			wxString sourceFullPath = myParent->GetFullPath();
			wxString sourceRelativePath = myParent->GetRelativePath();

			if ( m_started )
			{
				int line = LineFromPosition(event.GetPosition());
				int mask = MarkerGet(line);
				if (mask & 256) 
				{
					MarkerDelete(line, BREAKPOINT_MARKER);
					myFrame->RemoveBreakpoint(sourceRelativePath, line+1);
				} 
				else 
				{
					MarkerAdd(line, BREAKPOINT_MARKER);
					myFrame->AddBreakpoint(sourceRelativePath, line+1);
				}
			}
            break;
        }
        case 2: // folding margin
        {
            int line = LineFromPosition(event.GetPosition());
            ToggleFold(line);
            break;
        }
    }
}

void StyledTextCtrl::OnUpdateUI(wxStyledTextEvent& event)
{
	UpdateStatusBar();
}

void StyledTextCtrl::UpdateStatusBar(void)
{
	int pos = GetCurrentPos();

	//HighlightBraces(); // brace highlighting

	bool value = CONFIG->GetPropertyBool(wxT("EDITOR/GENERAL/LINENUMBER"), wxT("value"));
	if ( value )
	{
		wxString temp;
		temp = temp.Format(wxT("%d0"), GetLineCount());
		int width = TextWidth(wxSTC_STYLE_LINENUMBER, temp);
		SetMarginWidth(NUMBER_MARGIN_ID, width);
	}
	else
	{
		SetMarginWidth(NUMBER_MARGIN_ID, 0);
	}

	STATUS_BAR->SetStatusText( wxString::Format(wxT("L:%d, C:%d, P:%d"), 
		LineFromPosition(pos)+1, GetColumn(pos)+1, pos), 2);
}

void StyledTextCtrl::OnShowValue(wxStyledTextEvent& event)
{
	if ( m_focusOut ) return;

	int pos = event.GetPosition();

	if ( m_started )
	{
		if (CallTipActive()) return;

		int start = WordStartPosition(pos, true);
		int end = WordEndPosition(pos, true);
		
		wxString token = GetTextRange(start, end);
		if ( token.Length() <= 0 ) return;

		wxString annotate = 
			DEBUGGER->Eval(wxString::Format(wxT("output %s"), token.c_str()));
	
		wxString error = wxString::Format(wxT("%c%cerror"), 26, 26);
		if ( annotate.Find(error) == wxNOT_FOUND ) 
			StyleSetSpec(wxSTC_STYLE_CALLTIP, wxT("fore:#000000,back:#FFFFFF")); 
		else
			StyleSetSpec(wxSTC_STYLE_CALLTIP, wxT("fore:#FF0000,back:#FFFFFF")); 

		wxString value = DEBUGGER->RemoveAnnotate(annotate);
		if ( value[value.Length()-1] == '\n' )
			value[value.Length()-1] = '\0';

		if (CallTipActive())
			CallTipCancel();

		CallTipShow(pos, value);
	}
}

void StyledTextCtrl::OnHideValue(wxStyledTextEvent& event)
{
	if ( m_started )
	{
		if (CallTipActive())
			CallTipCancel();
	}
}

void StyledTextCtrl::OnLeave(wxMouseEvent& event)
{
	m_focusOut = true;
}

void StyledTextCtrl::OnEnter(wxMouseEvent& event)
{
	m_focusOut = false;
}
