// vim: set fdm=marker ts=4 sw=4:

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

class MyGdbFrame;
class Completion;
class Console;

// ----------------
class Console: public wxStyledTextCtrl
{
public:
	Console(wxWindow *parent,
		MyGdbFrame *myFrame,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxNO_BORDER
	);

    ~Console();

public:
	MyGdbFrame *myFrame;
	Completion *m_completion;

	wxStringList m_cmdList;

private:
	long m_startPos;	// start position
	int m_cmdIndex;
	bool m_idle;

public:
	void DisconnectIdle();
	void ConnectIdle();
	void Puts(wxString str);
	void Puts(wxString str, int type);
	void SetStartPos(long pos);
	void HideCompletionBox();
	wxString GetWordAtCaret();
	void SetStyle();
	void Initial();

private:
	void OnIdle(wxIdleEvent & evt);
	void OnKeyPressed(wxKeyEvent& event);
	void OnKeyReleased(wxKeyEvent& event);
	void OnLeftUp(wxMouseEvent& event);
	void OnMiddleDown(wxMouseEvent& event);
	void OnRightDown(wxMouseEvent& event);
	void OnCharAdded(wxStyledTextEvent& event);

    DECLARE_EVENT_TABLE()
};

#endif
