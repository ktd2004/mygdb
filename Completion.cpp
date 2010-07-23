// vim: set fdm=marker ts=4 sw=4:

#include "MyGDB.h"

CompletionBase::CompletionBase( 
		wxWindow* parent, wxWindowID id, 
		const wxPoint& pos, const wxSize& size, long style ) 
: wxPanel( parent, id, pos, size, style )
{
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	m_listCtrl = new wxListCtrl( this, wxID_ANY, 
			wxDefaultPosition, wxDefaultSize,
			wxLC_NO_HEADER|wxLC_REPORT|wxLC_SINGLE_SEL|wxNO_BORDER);
	m_listCtrl->InsertColumn(0, wxT("Name"));
	mainSizer->Add( m_listCtrl, 1, wxEXPAND, 5 );

	m_imgList = new wxImageList(16, 16, true);
	m_imgList->Add(GET_BITMAP("green_led"));
	m_listCtrl->SetImageList(m_imgList, wxIMAGE_LIST_SMALL);

	m_listCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, 
			wxListEventHandler( CompletionBase::OnItemActivated ), NULL, this );
	m_listCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, 
			wxListEventHandler( CompletionBase::OnItemDeSelected ), NULL, this );
	m_listCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, 
			wxListEventHandler( CompletionBase::OnItemSelected ), NULL, this );
	m_listCtrl->Connect( wxEVT_COMMAND_LIST_KEY_DOWN, 
			wxListEventHandler( CompletionBase::OnKeyDown ), NULL, this );

	SetSizer( mainSizer );
	Layout();
}

CompletionBase::~CompletionBase()
{
	delete m_imgList;

	m_listCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, 
			wxListEventHandler( CompletionBase::OnItemActivated ), NULL, this );
	m_listCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, 
			wxListEventHandler( CompletionBase::OnItemDeSelected ), NULL, this );
	m_listCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, 
			wxListEventHandler( CompletionBase::OnItemSelected ), NULL, this );
	m_listCtrl->Disconnect( wxEVT_COMMAND_LIST_KEY_DOWN, 
			wxListEventHandler( CompletionBase::OnKeyDown ), NULL, this );
}

wxListCtrl *CompletionBase::GetListCtrl()
{
	return m_listCtrl;
}

// --------------------------------

Completion::Completion(
	wxStyledTextCtrl* parent, bool autoHide, bool autoInsertSingleChoice)
:
	CompletionBase(parent, wxID_ANY, wxDefaultPosition, wxSize(0, 0))
	, m_showFullDecl(false)
	, m_autoHide(autoHide)
	, m_insertSingleChoice(autoInsertSingleChoice)
	, m_owner(NULL)
{
	Hide();

	// 배경을 흰색으로 셋팅
	SetBackgroundColour (wxColour(255,255,255));

	m_editor = parent;
}

Completion::~Completion()
{
}

void Completion::Show(const wxString& word)
{
	m_selectedItem = FindMatch(word);

	if (m_selectedItem == wxNOT_FOUND)
		m_selectedItem = 0;

	Adjust();

	SelectItem(m_selectedItem);
	
	GetListCtrl()->SetFocus();
	m_editor->SetFocus();
}

void Completion::SetItems(const std::vector<compItemInfo> &items)
{
	compItemInfo comp;
	
	GetListCtrl()->DeleteAllItems();

	if (items.empty() == false)
	{
		for (unsigned int i=0; i<items.size(); i++)
		{
			comp = items.at(i);
			long item = GetListCtrl()->InsertItem (i, comp.name, comp.kind);
			GetListCtrl()->SetItemBackgroundColour(item, wxColour(255,255,255));
			GetListCtrl()->SetItemTextColour(item, 
					wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));

			wxRect rect;
			GetListCtrl()->GetItemRect(item, rect);
			m_lineHeight = rect.height;
		}
	}

	m_compItems = items;

	GetListCtrl()->SetColumnWidth(0, wxLIST_AUTOSIZE);
	GetListCtrl()->SetItemCount(items.size());
	
	m_width = GetListCtrl()->GetColumnWidth(0);
	m_width +=25;
}

int Completion::GetCurrLineHeight()
{
	int point = m_editor->GetCurrentPos();
	wxPoint pt = m_editor->PointFromPosition(point);

	// calculate the line height
	int curline = m_editor->LineFromPosition(point);
	int ll;
	int hh(0);
	if (curline > 0) {
		ll = curline - 1;
		int pp = m_editor->PositionFromLine(ll);
		wxPoint p = m_editor->PointFromPosition(pp);
		hh =  pt.y - p.y;
	} else {
		ll = curline + 1;
		int pp = m_editor->PositionFromLine(ll);
		wxPoint p = m_editor->PointFromPosition(pp);
		hh =  p.y - pt.y;
	}

	if (hh == 0) {
		hh = 12; // default height on most OSs
	}

	return hh;
}

void Completion::Adjust(int point)
{
	int height;

	wxPoint show = m_editor->PointFromPosition(point);

	wxSize size = m_editor->GetClientSize();
	int availableDownHeight = size.y - show.y;
	int availableUpHeight = size.y - availableDownHeight;

	// 보더 사이즈
	wxSize border = GetWindowBorderSize();

	// 아래쪽으로 영역이 더 크면...
	if ( availableDownHeight >= availableUpHeight )
	{
		show.y += GetCurrLineHeight();

		int availableHeight = availableDownHeight-GetCurrLineHeight();
		height = availableHeight;

		// height이 자동 완성 윈도우보다 작으면
		// 자동 완성 윈도우의 height을 변경한다.
		int count = GetListCtrl()->GetItemCount();
		if ( count * m_lineHeight < height )
			height = count * m_lineHeight;
		
		// 자동완성 윈도우의 두께를 위해 여분을 추가
		height+=(border.GetHeight()*2); // 위,아래

		if ( height > availableHeight )
			height = availableHeight;
	}
	// 위쪽이 더 크면...
	else 
	{
		height = availableUpHeight;

		// height이 자동 완성 윈도우보다 작으면
		// 자동 완성 윈도우의 height을 변경한다.
		int count = GetListCtrl()->GetItemCount();
		if ( count * m_lineHeight < height )
			height = count * m_lineHeight;

		// 자동완성 윈도우의 두께를 위해 여분을 추가
		height+=(border.GetHeight()*2); // 위,아래

		if ( height > availableUpHeight )
			height = availableUpHeight;

		show.y -= height;
	}

	if (size.x - show.x < m_width)
	{
		if (size.x > m_width)
			show.x = size.x - m_width;
		else
			show.x = 0;
	}

	Move(show);

	SetSize(m_width, height);
	GetSizer()->Layout();
	wxWindow::Show();
}

void Completion::Adjust()
{
	int point = m_editor->GetCurrentPos();
	Adjust(point);
}

void Completion::SelectItem(long item)
{
	GetListCtrl()->SetItemState(item, wxLIST_STATE_SELECTED, 
			wxLIST_STATE_SELECTED);
	GetListCtrl()->EnsureVisible(item);
}

void Completion::SelectWord(const wxString& word)
{
	long item = FindMatch(word);
	if (item != wxNOT_FOUND)
	{
		// first unselect the current item
		if (m_selectedItem != wxNOT_FOUND && m_selectedItem != item)
		{
			//GetListCtrl()->SelecItem(m_selectedItem, false);
		}

		m_selectedItem = item;
		SelectItem(m_selectedItem);
	}
	else
	{
		if (GetAutoHide())
		{
			Hide();
		}
	}
}

int Completion::FindMatch(const wxString& word)
{
	// first try to match case sensetive
	for(size_t i=0; i<m_compItems.size(); i++) {
		compItemInfo item = m_compItems.at(i);
		
		wxString s1(word);
		wxString s2(item.name);
		if(s2.StartsWith(s1)) {
			return static_cast<int>(i);
		}
	}
	
	// if we are here, it means we failed, try case insensitive
	for(size_t i=0; i<m_compItems.size(); i++) {
		compItemInfo item = m_compItems.at(i);
		
		wxString s1(word);
		wxString s2(item.name);
		s1.MakeLower(); s2.MakeLower();
		if(s2.StartsWith(s1)) {
			return static_cast<int>(i);
		}
	}
	
	return wxNOT_FOUND;
}

void Completion::InsertSelection()
{
	if (m_selectedItem == wxNOT_FOUND)
	{
		return;
	}

	// get the selected word
	//wxString word = GetColumnText(GetListCtrl(), m_selectedItem, 0);
	wxString word = GetListCtrl()->GetItemText(m_selectedItem);
	DoInsertSelection(word);
}

void Completion::ReplaceStartPosition(long pos)
{
	m_startPos = pos;
}

void Completion::DoInsertSelection(const wxString& word, bool triggerTip)
{
	if (m_owner)
	{

		/*
		// simply send an event and dismiss the dialog
		wxCommandEvent e(wxEVT_CCBOX_SELECTION_MADE);
		e.SetClientData( (void*)&word );
		m_owner->ProcessEvent(e);
		*/

	}
	else
	{
		//int insertPos = m_editor->WordStartPosition(m_editor->GetCurrentPos(), true);

		m_editor->SetSelection(m_startPos, 
				m_editor->GetCurrentPos());
		m_editor->ReplaceSelection(word);
	}
}

void Completion::Previous()
{
	if (m_selectedItem != wxNOT_FOUND)
	{
		if (m_selectedItem - 1 >= 0)
		{
			m_selectedItem--;

			// select previous item
			SelectItem(m_selectedItem);
		}
	}
}

void Completion::Next()
{
	if (m_selectedItem != wxNOT_FOUND)
	{
		if (m_selectedItem + 1 < GetListCtrl()->GetItemCount())
		{
			m_selectedItem++;
			// select next item
			SelectItem(m_selectedItem);
		}
	}
}

void Completion::PreviousPage()
{
	if (m_selectedItem != wxNOT_FOUND)
	{
		if (m_selectedItem - 10 >= 0)
		{
			m_selectedItem -= 10;

			// select previous item
			SelectItem(m_selectedItem);
		}
		else
		{
			m_selectedItem = 0;

			// select previous item
			SelectItem(m_selectedItem);
		}
	}
}

void Completion::NextPage()
{
	if (m_selectedItem != wxNOT_FOUND)
	{
		if (m_selectedItem + 1 < GetListCtrl()->GetItemCount())
		{
			m_selectedItem += 10;
			// select next item
			SelectItem(m_selectedItem);
		}
		else
		{
			m_selectedItem = GetListCtrl()->GetItemCount() - 1;
			// select next item
			SelectItem(m_selectedItem);
		}
	}
}
