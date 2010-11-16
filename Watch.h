// vim: set fdm=marker ts=4 sw=4:
/* 
 * $Log$
 */

#ifndef _WATCH_H_
#define _WATCH_H_

class MyGdbFrame;

enum {
	ID_SET_MAIN_FILE = wxID_HIGHEST+1,
	ID_ADD_SOURCE_FILE,
	ID_ADD_ADDITIONAL_FILE,
	ID_REMOVE_SOURCE_FILE,
	ID_EDIT_SOURCE_FILE,
	ID_REMOVE_ADDITIONAL_FILE,
	ID_REMOVE_ALL_ADDITIONAL_FILE,
	ID_OPEN_FOLDER,
	ID_FILE_PROPERTIES,
};

class Watch : public wxTreeListCtrl
{
public:
	Watch(wxWindow* parent,
		MyGdbFrame *myFrame,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTR_DEFAULT_STYLE|wxTR_LINES_AT_ROOT|wxNO_BORDER

	);

    ~Watch();

private:
	wxImageList *m_imgList;
	wxMenu *m_menu;
	wxTreeItemId rootItem;
	MyGdbFrame *myFrame;

	long m_parsePos;

	wxArrayString nodeExpandedArray;
	
public:
	void Build();
	void Clear();
	void PrintVariable(wxString result, wxTreeItemId item);
	int FindCommaPos(const wxString& str);
	int FindCharOutsideQuotes(const wxString& str, wxChar ch);
	void ParseEntry (wxTreeItemId parent, wxString &text);
	wxString GetValue(wxString text);
	wxString GetVariable(wxString text);

	void LoadState ();
	void SaveState ();
	void SaveTreeNodeState(wxTreeItemId id, wxString path);
	void LoadTreeNodeState(wxTreeItemId id, wxString path);
	
private:

    DECLARE_EVENT_TABLE()
};

#endif
