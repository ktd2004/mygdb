// vim: set fdm=marker ts=4 sw=4:

#ifndef _CALLSTACK_H_
#define _CALLSTACK_H_

class MyGdbFrame;

class CallStack : public wxTreeListCtrl
{
public:
	CallStack(wxWindow* parent,
		MyGdbFrame *myFrame,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTR_DEFAULT_STYLE|wxTR_LINES_AT_ROOT|wxNO_BORDER

	);

    ~CallStack();

private:
	wxImageList *m_imgList;
	wxMenu *m_menu;
	wxTreeItemId rootItem;
	MyGdbFrame *myFrame;

	wxArrayString nodeExpandedArray;
	
public:
	void Build();
	void Clear();
	void Print(wxString msg, wxTreeItemId item);
	void UpdatePointer(wxString address);

	void LoadState ();
	void SaveState ();
	void SaveTreeNodeState(wxTreeItemId id, wxString path);
	void LoadTreeNodeState(wxTreeItemId id, wxString path);
	
	void AutoResize();
	
private:
	void OnLeftDClick(wxMouseEvent& event);

    DECLARE_EVENT_TABLE()
};

#endif
