// vim: set fdm=marker ts=4 sw=4:

#ifndef _REGISTER_H_
#define _REGISTER_H_

class MyGdbFrame;

enum {
	ID_CHANGE_REGISTER_VALUE = wxID_HIGHEST+1,
};

class Register : public wxTreeListCtrl
{
public:
	Register(wxWindow* parent,
		MyGdbFrame *myFrame,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTR_DEFAULT_STYLE|wxTR_LINES_AT_ROOT|wxNO_BORDER

	);

    ~Register();

private:
	wxImageList *m_imgList;
	wxMenu *m_menu;
	wxTreeItemId rootItem;
	MyGdbFrame *myFrame;

	wxArrayString nodeExpandedArray;
	
public:
	void Build();
	void Clear();
	void Print(wxString result, wxTreeItemId item);

	void LoadState ();
	void SaveState ();
	void SaveTreeNodeState(wxTreeItemId id, wxString path);
	void LoadTreeNodeState(wxTreeItemId id, wxString path);

	void AutoResize();
	
private:

    DECLARE_EVENT_TABLE()
};

#endif
