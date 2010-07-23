// vim: set fdm=marker ts=4 sw=4:

#ifndef _INVOKEAPP_H_
#define _INVOKEAPP_H_

class wxFileName;
class MyGdbFrame;

class InvokeApp : public wxDialog
{
	enum {
		ID_SELECT_EXECUTABLE = wxID_HIGHEST+1,
		ID_SELECT_SOURCE_LOCATION,
		ID_SELECT_GDB_LOCATION,
	};

public:
	InvokeApp(wxWindow* parent,
		MyGdbFrame *myFrame,
		wxWindowID id = wxID_ANY,
		const wxString& title = _T("Invoke Application"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE
	);

    ~InvokeApp();
	
private:
	MyGdbFrame *myFrame;

	wxTextCtrl *m_executableLocation;
	wxTextCtrl *m_args;
	wxTextCtrl *m_sourceLocation;
	wxTextCtrl *m_gdbLocation;

public:
	wxString GetSourceLocation();
	wxString GetExecutableFile();
	wxString GetArguments();
	wxString GetGDBLocation();

private:
	void OnSelectExecutable(wxCommandEvent& WXUNUSED(event));
	void OnSelectSourceLocation(wxCommandEvent& WXUNUSED(event));
	void OnSelectGDBLocation(wxCommandEvent& WXUNUSED(event));

	void OnExecutableEnter(wxChildFocusEvent& event);
	void OnExecutableLeave(wxChildFocusEvent& event);
	void OnArgumentsEnter(wxChildFocusEvent& event);
	void OnArgumentsLeave(wxChildFocusEvent& event);
	void OnSourceLocationEnter(wxChildFocusEvent& event);
	void OnSourceLocationLeave(wxChildFocusEvent& event);
	void OnGDBLocationEnter(wxChildFocusEvent& event);
	void OnGDBLocationLeave(wxChildFocusEvent& event);

	void OnOk(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};

#endif
