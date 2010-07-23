// vim: set fdm=marker ts=4 sw=4:

#ifndef _STYLEDTEXTCTRL_H_
#define _STYLEDTEXTCTRL_H_

class TclInterp;
class TclParser;
class CodeExplorer;
class StyledTextCtrl;
class Ruler;
class Tooltip;
class Completion;

class MyGdbFrame;

#include <Completion.h>

class StyledTextCtrl : public wxStyledTextCtrl
{
public:
	StyledTextCtrl(CodeEditor* parent,
		MyGdbFrame *myFrame,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxNO_BORDER
	);

    ~StyledTextCtrl();

private:
	MyGdbFrame *myFrame;
	CodeEditor *myParent;

	Completion *m_completion;
    wxTimer *m_timer;

	bool m_focusOut;
	bool m_commenting;
	bool m_autoIndent;
	bool m_smartIndent;
    bool m_modified;
	bool m_closing;
	
	wxMenu* menu;
	
public:
	void SetClosing(bool close);

	void UpdateTitle(void);
	wxString GetTitle(void);
	void SetTitle(wxString title);

	void SetProperties(void);
	void SetStyles();

	wxString NormalizeString (wxString string);
	void SetCommenting(bool comment);

	void DoSetCaretAt(long pos);
	void SetCaretAt(long pos);
	void ToggleCurrentFold();
	void FoldAll();

	wxString GetLineIndentString(int line);
	wxChar GetLastNonWhitespaceChar(int position);
	wxString GetWordAtCaret();
	
	int FindBlockStart(int position, wxChar blockStart, wxChar blockEnd);
	void HighlightBraces(void);
	void GotoMatchingBrace(void);
	wxString GetTextUnderCursor(void);
	wxString GetTextLeftUnderCursor(void);

	bool WordComplete(void);
	void SubCommandComplete(void);
	wxArrayString Split(wxString text);

	int GetCurrLineHeight();
	long GetStartLine(int line);

	bool ToggleBreakpoint(int line);
	bool MarkBreakpoint(int line);
	bool UnmarkBreakpoint(int line);

	// ----
	void UpdateToolBar(void);
	void UpdateMenu(void);
	void UpdateStatusBar(void);

	// Ruler
	long CalOffset();
	long CalColumn();
	void UpdateRuler();
	
private:
	void OnIdle(wxIdleEvent& event);
	void OnKeyPressed(wxKeyEvent& event);
	void OnKeyReleased(wxKeyEvent& event);
	void OnMarginClicked(wxStyledTextEvent& event);
	void OnUpdateUI(wxStyledTextEvent& event);
	void OnModified(wxStyledTextEvent& event);
	void OnChanged(wxStyledTextEvent& event);
	void OnTimer(wxTimerEvent& WXUNUSED(event));
	void OnRightDown(wxMouseEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	void OnMotion(wxMouseEvent& event);
	void OnLeave(wxMouseEvent& event);
	void OnEnter(wxMouseEvent& event);

	void OnShowValue(wxStyledTextEvent& event);
	void OnHideValue(wxStyledTextEvent& event);

    DECLARE_EVENT_TABLE()
};

#endif
