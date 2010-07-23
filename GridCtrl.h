// vim: set fdm=marker ts=4 sw=4:

#ifndef _GRIDCTRL_H_
#define _GRIDCTRL_H_

typedef struct ColLabelInfo {
	wxString label;
	int from;
	int to;
} colLabelInfo;

// -------------------
class GridCtrl: public wxGrid
{
public:
    GridCtrl(wxWindow *parent,
		MyGdbFrame *myFrame,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxNO_BORDER | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE,
		const wxString& name = wxT("GridCtrl")
	);

    ~GridCtrl();

public:
	MyGdbFrame *myFrame;
	std::vector<colLabelInfo> m_colLabelInfoItems;
	int m_activeIndex;
	int m_indexCol;

public:
	void Sort(int col);
	void ScrollToCol(int col);
	wxFont GetBold(void);
	wxGridCellAttr *GetAttrBold();
	int GetRowByIndex(int index);
	wxString GetLabel(int col);
	void AddLabelInfo(wxString label, int from, int to);
	wxArrayString GetLabelInfo();
	void GetLabelRange(wxString label, int *from, int *to);
	void DrawLabel(wxDC& dc, wxString label, int x, int y, int width, int height);
	void SetActiveIndex(int index);
	void SetIndexCol(int col);

	virtual void DrawColLabel(wxDC& dc, int col); 
    //virtual void DrawRowLabel(wxDC& dc, int row);

private:
	void OnLabelClick(wxGridEvent& event);
	void OnScrollEvent( wxScrollWinEvent &event );

private:
    DECLARE_EVENT_TABLE()
};

#endif
