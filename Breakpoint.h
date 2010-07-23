// vim: set fdm=marker ts=4 sw=4:

#ifndef _BREAKPOINT_H_
#define _BREAKPOINT_H_

class MyGdbFrame;

enum {
	ID_DELETE_BREAKPOINT = wxID_HIGHEST+1,
	ID_ENABLE_BREAKPOINT,
	ID_DISABLE_BREAKPOINT,
};

class Breakpoint : public wxTreeListCtrl
{
public:
	Breakpoint(wxWindow* parent,
		MyGdbFrame *myFrame,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTR_DEFAULT_STYLE|wxTR_LINES_AT_ROOT|wxNO_BORDER

	);

    ~Breakpoint();

private:
	wxImageList *m_imgList;
	wxMenu *m_menu;
	wxTreeItemId rootItem;
	MyGdbFrame *myFrame;
	
	int numColNo;
	int whatColNo;
	int addressColNo;

	wxArrayString nodeExpandedArray;
	
public:
	void Build();
	void Clear();
	void Print(wxString msg, wxTreeItemId item);

	void LoadState ();
	void SaveState ();
	void SaveTreeNodeState(wxTreeItemId id, wxString path);
	void LoadTreeNodeState(wxTreeItemId id, wxString path);
	
	void AutoResize();
	
private:
	void OnLeftDClick(wxMouseEvent& event);
	void OnRightClick(wxTreeEvent& event);
	void OnDeleteBreakpoint(wxCommandEvent& WXUNUSED(event));
	void OnEnableBreakpoint(wxCommandEvent& WXUNUSED(event));
	void OnDisableBreakpoint(wxCommandEvent& WXUNUSED(event));

    DECLARE_EVENT_TABLE()
};

#endif
