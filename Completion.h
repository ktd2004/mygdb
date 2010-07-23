// vim: set fdm=marker ts=4 sw=4:

#ifndef _COMPLETION_H_
#define _COMPLETION_H_

// kind
#define	COMP_CMD		0
#define	COMP_OPT		1

typedef struct CompItemInfo {
	int kind;		
	wxString name;
} compItemInfo;

struct CompItemInfoSortByName
{
	bool operator()(const CompItemInfo& p1, const CompItemInfo& p2)
	{ 
		return strcmp((char*)p2.name.c_str(), (char*)p1.name.c_str()) > 0; 
	}
};

class wxStyledTextCtrl;

// ---------------------------
class CompletionBase : public wxPanel 
{
protected:
	// Virtual event handlers, overide them in your derived class
	virtual void OnItemActivated( wxListEvent& event ){ event.Skip(); }
	virtual void OnItemDeSelected( wxListEvent& event ){ event.Skip(); }
	virtual void OnItemSelected( wxListEvent& event ){ event.Skip(); }
	virtual void OnKeyDown( wxListEvent& event ){ event.Skip(); }

public:
	CompletionBase( wxWindow* parent, wxWindowID id = wxID_ANY, 
			const wxPoint& pos = wxDefaultPosition, 
			const wxSize& size = wxSize( 400,300 ), 
			long style = wxTAB_TRAVERSAL|wxBORDER_RAISED );
	~CompletionBase();
	wxListCtrl *GetListCtrl();

private:
	wxBoxSizer* mainSizer;
	wxListCtrl *m_listCtrl;
	wxImageList *m_imgList;
};

// -----------------------
class Completion: public CompletionBase
{
private:
	int m_width;
	int m_lineHeight;
	int m_startPos;

	int m_selectedItem;
	bool m_showFullDecl;
	bool m_autoHide;
	bool m_insertSingleChoice;
	wxEvtHandler *m_owner;
	std::vector<compItemInfo> m_compItems;
	wxStyledTextCtrl *m_editor;
	
public:
	Completion(wxStyledTextCtrl* parent, 
			bool autoHide = true, 
			bool autoInsertSingleChoice = true);
	~Completion();

	void ReplaceStartPosition(long pos);
	void Show(const wxString& word);
	int GetCurrLineHeight();
	void Adjust();
	void Adjust(int point);
	void SetList();
	void SetItems(const std::vector<compItemInfo> &items);
	void SelectWord(const wxString& word);
	int FindMatch(const wxString& word);
	void SelectItem(long item);
	void InsertSelection();
	void DoInsertSelection(const wxString &word, bool triggerTip = true);
	void Previous();
	void PreviousPage();
	void Next();
	void NextPage();

	void SetAutoHide(const bool& autoHide) {
		this->m_autoHide = autoHide;
	}

	const bool& GetAutoHide() const {
		return m_autoHide;
	}
};

#endif
