// vim: set fdm=marker ts=4 sw=4:

#ifndef _CODEEDITOR_H_
#define _CODEEDITOR_H_

class TclInterp;
class TclParser;
class CodeExplorer;
class StyledTextCtrl;
class Ruler;
class Tooltip;
class Completion;

class MyGdbFrame;


class CodeEditor : public wxAuiMDIChildFrame
{
public:
	CodeEditor(wxAuiMDIParentFrame* parent,
		MyGdbFrame *myFrame,
		wxWindowID id = wxID_ANY,
		const wxString& file = _T(""),
		const bool& main = false
	);

    ~CodeEditor();

public:
	void Open(wxString path);
	void UpdateTitle();
	void GotoLine(int line);
	wxString GetFullPath();
	wxString GetRelativePath();
	void GotoLineWithDebugMarker(int line);
	StyledTextCtrl *GetSTC();
	void ShowBreakpoint ();
	void ClearAllBreakpoint ();

private:
	wxBoxSizer *mainSizer;
	MyGdbFrame *myFrame;
	StyledTextCtrl* m_stc;

	wxString m_fullPath;
	wxString m_relativePath;

private:
	void OnIdle(wxIdleEvent& event);

	DECLARE_EVENT_TABLE()
};


#endif
