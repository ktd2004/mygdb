// vim: set fdm=marker ts=4 sw=4:

#ifndef _MEMORY_H_
#define _MEMORY_H_

class GridCtrl;
class MyGdbFrame;

enum {
	ID_MEMORY_RANGE = wxID_HIGHEST+1,
};

// -----------------------
class Memory : public GridCtrl
{
public:
    Memory(wxWindow *parent,
		MyGdbFrame *myFrame,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxNO_BORDER | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE,
		const wxString& name = wxT("Memory")
	);

    ~Memory();

public:
	MyGdbFrame *myFrame;
	
	int m_column;
	unsigned int m_start;
	unsigned int m_count;

private:
	wxMenu *m_menu;

public:
	void Build();
	void Print(wxString msg);
	void AutoResizeRowLabels();
	void Clear();

private:
	void OnSize(wxSizeEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnMotion(wxMouseEvent& event);
	void OnRightDown(wxMouseEvent& event);

	void OnMemoryRange(wxCommandEvent& WXUNUSED(event));
	void OnMemoryColumn(wxCommandEvent& WXUNUSED(event));
	void OnCellValueChanged( wxGridEvent& ev );

	DECLARE_EVENT_TABLE()
};

// --------------------------------
class MemoryRange : public wxDialog
{
public:
	MemoryRange(wxWindow* parent,
		MyGdbFrame *myFrame,
		Memory *myParent,
		wxWindowID id = wxID_ANY,
		const wxString& title = _T(""),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE|wxSTAY_ON_TOP
	);

    ~MemoryRange();

public:
	MyGdbFrame *myFrame;
	Memory *myParent;
	
public:
	wxTextCtrl *m_startTextCtrl;
	wxTextCtrl *m_countTextCtrl;
	wxTextCtrl *m_columnTextCtrl;
	wxButton *m_okButton;
	wxButton *m_closeButton;
   
private:
	void OnOk(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);
    
	DECLARE_EVENT_TABLE()
};

#endif
