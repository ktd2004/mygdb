// vim: set fdm=marker ts=4 sw=4:

#include <MyGDB.h>
#include "xpm/flag.xpm"

BEGIN_EVENT_TABLE(GridCtrl, wxGrid)
    //EVT_GRID_LABEL_LEFT_CLICK(GridCtrl::OnLabelClick)
	EVT_SCROLLWIN(GridCtrl::OnScrollEvent)
END_EVENT_TABLE()


GridCtrl::GridCtrl(wxWindow *parent,
	MyGdbFrame *myFrame,
	const wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
: wxGrid(parent, id, pos, size, style, name)
{
	this->myFrame = myFrame;
	wxAuiDockArt *dockArt = myFrame->GetDockArt();
	
	m_activeIndex = -1;
	m_indexCol = -1;
}

GridCtrl::~GridCtrl () 
{
}

wxString GridCtrl::GetLabel(int col)
{
	colLabelInfo item;
	for (unsigned int i=0; i<m_colLabelInfoItems.size(); i++)
	{
		item = m_colLabelInfoItems.at(i);
		if ( col >= item.from && col <= item.to )
			return item.label;
	}
	return wxT("");
}

void GridCtrl::AddLabelInfo(wxString label, int from, int to)
{
	colLabelInfo item;
	item.label = label;
	item.from = from;
	item.to = to;
	m_colLabelInfoItems.push_back(item);
}

wxArrayString GridCtrl::GetLabelInfo()
{
	wxArrayString labels;

	colLabelInfo item;
	for (unsigned int i=0; i<m_colLabelInfoItems.size(); i++)
	{
		item = m_colLabelInfoItems.at(i);
		labels.Add(item.label);
	}

	return labels;
}

void GridCtrl::GetLabelRange(
		wxString label, int *from, int *to)
{
	colLabelInfo item;
	for (unsigned int i=0; i<m_colLabelInfoItems.size(); i++)
	{
		item = m_colLabelInfoItems.at(i);
		if ( item.label == label )
		{
			*from = item.from;
			*to = item.to;
			return;
		}
	}

	*from = -1;
	*to = -1;

	return;
}

void GridCtrl::DrawLabel(wxDC& dc, wxString label, 
		int x, int y, int width, int height)
{
	dc.SetPen( wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW), 
				1, wxSOLID) );
	dc.DrawLine( x+width, y, x+width, y+height - 1 );
	dc.DrawLine( x, y, x+width, y );
	dc.DrawLine( x, y+height - 1,
				 x+width + 1, y+height - 1 );

	dc.SetPen( *wxWHITE_PEN );
	dc.DrawLine( x, y+1, x, height - 1 );
	dc.DrawLine( x, y+1, x+width, y+1 );

	dc.SetBackgroundMode( wxTRANSPARENT );
	dc.SetTextForeground( GetLabelTextColour() );
	dc.SetFont( GetLabelFont() );

	int hAlign, vAlign, orient;
	GetColLabelAlignment( &hAlign, &vAlign );
	orient = GetColLabelTextOrientation();

	wxRect rect;
	rect.SetX( x + 2 );
	rect.SetY( y+2 );
	rect.SetWidth( width - 4 );
	rect.SetHeight( height - 4 );
	DrawTextRectangle( dc, label, rect, hAlign, vAlign, orient );
}

void GridCtrl::ScrollToCol(int col)
{
	Scroll(GetColLeft(col)/GetScrollLineX(), 0);
}
		
void GridCtrl::SetActiveIndex(int index)
{
	m_activeIndex = index;
}

void GridCtrl::SetIndexCol(int col)
{
	m_indexCol = col;
}

/*
void GridCtrl::DrawRowLabel(wxDC& dc, int row)
{
    if (GetRowHeight(row)<=0 || m_rowLabelWidth<=0)
        return;

    wxRect rect;
    int rowTop=GetRowTop(row),
        rowBottom=GetRowBottom(row)-1;
    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW), 
				1, wxSOLID));
    dc.DrawLine(m_rowLabelWidth - 1, rowTop, m_rowLabelWidth - 1, rowBottom);
    dc.DrawLine(0, rowTop, 0, rowBottom);
    dc.DrawLine(0, rowBottom, m_rowLabelWidth, rowBottom);
    dc.SetPen(*wxWHITE_PEN);
    dc.DrawLine(1, rowTop, 1, rowBottom);
    dc.DrawLine(1, rowTop, m_rowLabelWidth - 1, rowTop);

	wxString label=GetCellValue( row, m_indexCol );

	if (atoi(label.c_str()) == m_activeIndex) {
		dc.DrawBitmap(wxBitmap(flag_xpm),0,GetRowTop(row),true);
	}
}
*/

void GridCtrl::DrawColLabel(wxDC& dc, int col)
{
	int h = m_colLabelHeight;
	int w = GetColWidth(col) + GetColWidth(col+1);

    if ( GetColWidth(col) <= 0 || m_colLabelHeight <= 0 )
        return;

	bool drawLargeLabel=false;
	int colLeft;
	int colRight;

	colLabelInfo item;
	for (unsigned int i=0; i<m_colLabelInfoItems.size(); i++)
	{
		item = m_colLabelInfoItems.at(i);
		if ( item.from <= col && item.to >= col )
		{
			drawLargeLabel=true;
			int x =GetColLeft(item.from);
			int y =0;
			int width = GetColRight(item.to)-x-1;
			int height= m_colLabelHeight/2;
			DrawLabel(dc, item.label,  x, y, width, height);
		}
	}

	if ( drawLargeLabel == false )
	{
		int x =GetColLeft(col);
		int y =0;
		int width = GetColRight(col)-x-1;
		int height= m_colLabelHeight;
		wxString label=GetColLabelValue( col );
		DrawLabel(dc, label, x, y, width, height);
	}
	else
	{
		int x =GetColLeft(col);
		int y =m_colLabelHeight/2-1;
		int width = GetColRight(col)-x-1;
		int height= m_colLabelHeight/2+1;
		wxString label=GetColLabelValue( col );
		DrawLabel(dc, label, x, y, width, height);
	}
}

void GridCtrl::OnScrollEvent( wxScrollWinEvent &event )
{
	//if (event.GetOrientation() == wxVERTICAL) return;
	wxScrollBar *sb = (wxScrollBar*)event.GetEventObject();
	int pos   = event.GetPosition();

	event.Skip();
}

// ------------------------------------------------------------------------
struct colCompare
{
	bool operator()(wxString s1, wxString s2) const
	{
		if ( s1.IsNumber() || s1.Matches(wxT("*.*")) )
		{
			double a = atof((char*)s1.c_str());
			double b = atof((char*)s2.c_str());

			bool m_sortAscendingOrder = true;

			if ( m_sortAscendingOrder )
			{
				if ( a < b ) return 1;
				return 0;
			} 
			else 
			{
				if ( a < b ) return 0;
				return 1;
			}
		}

		return strcmp((char*)s1.c_str(), (char*)s2.c_str()) < 0;
	}
};

void GridCtrl::OnLabelClick(wxGridEvent& event)
{
	if ( event.GetCol() == -1 ) return;
	Sort(event.GetCol());
}

void GridCtrl::Sort(int col)
{
	typedef std::list<wxString> sortList;
	typedef boost::shared_ptr<sortList> ptrSortList;

	// sort as string
	std::multimap<wxString, ptrSortList, colCompare> sort_col;
	for (int i=0; i<GetNumberRows(); i++)
	{
		ptrSortList a_row(new sortList);
		wxString v = GetCellValue(i, col).c_str();
		sort_col.insert(std::pair<wxString, ptrSortList>(v, a_row));
		for (int j=0; j<GetNumberCols(); j++)
		{
			wxString v = GetCellValue(i, j).c_str();
			a_row->push_back(v);
		}
	}

	int current_row = 0;
	while(sort_col.size())
	{
		ptrSortList row_data = sort_col.begin()->second;
		sort_col.erase(sort_col.begin());
		for(int i=0; i<GetNumberCols(); i++)
		{
			wxString v;
			v = row_data->front();
			SetCellValue(current_row, i, v);
			row_data->pop_front();
		}
		current_row++;
	}

	GetGridColLabelWindow()->Refresh(); 
	GetGridRowLabelWindow()->Refresh(); 
}

wxFont GridCtrl::GetBold(void)
{
	wxAuiDockArt *dockArt = myFrame->GetDockArt();
	wxFont font = dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	return font;
}

int GridCtrl::GetRowByIndex(int index)
{
	for (int row=0; row<GetNumberRows(); row++)
	{
		wxString s = GetCellValue(row, 0);
		if ( index == atoi((char*)s.c_str()) )
			return row;
	}
}
