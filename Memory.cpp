// vim: set fdm=marker ts=4 sw=4:

#pragma warning(disable: 4819)

#include <MyGDB.h>

BEGIN_EVENT_TABLE(Memory, GridCtrl)
    EVT_SIZE(Memory::OnSize)
    EVT_IDLE(Memory::OnIdle)
	EVT_MENU(ID_MEMORY_RANGE, Memory::OnMemoryRange)
    EVT_GRID_CELL_CHANGE( Memory::OnCellValueChanged )
END_EVENT_TABLE()

Memory::Memory(wxWindow *parent,
	MyGdbFrame *myFrame,
	const wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
: GridCtrl(parent, myFrame, id, pos, size, style, name)
{
	this->myFrame = myFrame;

	wxAuiDockArt *dockArt = myFrame->GetDockArt();
	
	m_column = 2;

	m_start = 0x0;
	m_count = 100;

    CreateGrid( 0, 0 );

	EnableEditing(true);
	DisableDragRowSize();
	//SetRowLabelSize(20);
	SetColLabelSize(20);

	SetColMinimalAcceptableWidth(0);
	SetRowMinimalAcceptableHeight(0);

	SetLabelFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	SetDefaultCellFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));

    //SetIndexCol(0);

	// ----------------
	wxFont font = GetDefaultCellFont();
	wxGridCellAttr *attr = new wxGridCellAttr;
	font.SetWeight(wxFONTWEIGHT_BOLD);
	attr->SetFont(font);
	attr->SetAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
	//SetColAttr(0, attr);
	
	wxWindow *win = GetTargetWindow(); 
	win->Connect(win->GetId(), wxEVT_MOTION, 
			wxMouseEventHandler(Memory::OnMotion),0, this);
	win->Connect(win->GetId(), wxEVT_RIGHT_DOWN, 
			wxMouseEventHandler(Memory::OnRightDown),0, this);

	m_menu=NULL;
}

Memory::~Memory () 
{
}

void Memory::Build()
{
	wxString error = wxString::Format(wxT("%c%cerror"), 26, 26);

	wxString msg = DEBUGGER->Eval(
		wxString::Format(wxT("p/x *0x%x@%d"), m_start, m_count));
	
	// check error 
	if ( msg.Find(error) != wxNOT_FOUND ) 
	{
		wxMessageDialog(NULL, DEBUGGER->RemoveAnnotate(msg), 
				wxT("MyGDB"), wxOK|wxICON_ERROR).ShowModal();
		return;
	}

	BeginBatch ();

	Print(msg);

    AutoSizeColumns(true);
	EndBatch ();
}

void Memory::Clear()
{
    DeleteCols(0, GetNumberCols());
    DeleteRows(0, GetNumberRows());
}

void Memory::Print(wxString msg)
{
	Clear();

    AppendCols(m_column);

	// based 32bit
	int rows = m_count/m_column;
	if ( rows == 0 ) rows=1;
    AppendRows(rows);

	int i;
	// m_column label
	for(i=0; i<m_column; i++)
	{
		SetColLabelAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
		int address = i*sizeof(long);
		wxString value = wxString::Format(wxT("0x%08x"), address);
		SetColLabelValue(i, value);
	}
	
	// row label
	for(i=0; i<rows; i++)
	{
		unsigned int address = m_start + (i*(sizeof(long)*m_column));
		wxString value = wxString::Format(wxT("0x%08x"), address);
		SetRowLabelValue(i, value);
	}
		
	// print memory value
	bool arrayBegin=false;

	wxString spliter = wxEmptyString;
	if ( msg.Find(wxT("\r\n")) != wxNOT_FOUND )
		spliter = wxT("\r\n");
	else
		spliter = wxT("\n");

	//DEBUGGER->Log(msg);

	wxStringTokenizer tkz(msg, spliter);

	int drawCol=0;
	int drawRow=0;

	while ( tkz.HasMoreTokens() ) 
	{
		wxString token = tkz.GetNextToken();
		
		wxString tag = wxString::Format(wxT("%c%carray-section-end"), 26, 26);
		if ( token.Find(tag) != wxNOT_FOUND )
		{
			break;
		}

		tag = wxString::Format(wxT("%c%celt"), 26, 26);
		if ( arrayBegin == true && token.StartsWith(tag) == false )
		{
			wxString value = token;

			value.Trim(true);
			value.Trim(false);

			wxString split = wxT(", ");
			long pos = value.Find(split);
			if ( pos != wxNOT_FOUND )
				value = value.SubString(pos+split.Length(), value.Length()); 

			SetCellValue(drawRow, drawCol, value);

			drawCol++;

			if ( (drawCol % m_column) == 0 )
			{
				drawRow++;
				drawCol=0;
			}
		}

		tag = wxString::Format(wxT("%c%carray-section-begin"), 26, 26);
		if ( token.StartsWith(tag) )
			arrayBegin = true;
		
	}

	for(i=0; i<rows; i++)
		AutoSizeColLabelSize(i);

	AutoResizeRowLabels();
}

void Memory::OnSize(wxSizeEvent& event)
{
	event.Skip();
}

void Memory::OnIdle(wxIdleEvent& event)
{
	event.Skip();
}

void Memory::AutoResizeRowLabels()
{
	int maxWidth = 0;
	for(int i=0; i<GetNumberRows(); i++)
	{
		wxString value = GetRowLabelValue(i);
		int width, height;
		GetTextExtent(value, &width, &height);
		width+=4; // border width
		if ( width > maxWidth ) maxWidth = width;
	}

	SetRowLabelSize(maxWidth);
	SetDefaultColSize(maxWidth);
}

void Memory::OnMotion(wxMouseEvent& event)
{
	wxPoint point = CalcUnscrolledPosition(event.GetPosition());

	int row = YToRow(point.y);
	int col = XToCol(point.x);
	
	if ( row == -1 || col == -1 ) return;
	
	unsigned int here = m_start + (row*(sizeof(long)*m_column)) + (col*sizeof(long));

	wxString value = GetCellValue(row, col);
	if ( value.Length() > 0 )
	{
		wxString tooltip = wxString::Format(wxT("0x%08x\n"), here);
		tooltip = tooltip + wxT("---------------\n");
		tooltip = tooltip + wxString::Format(wxT("Hex: %s\n"), value.c_str());
		long v;
		value.ToLong(&v, 16);		
		tooltip = tooltip + wxString::Format(wxT("Dec: %d"), (int)v);
		GetGridWindow()->SetToolTip(tooltip);
	}

	event.Skip();
}

void Memory::OnRightDown(wxMouseEvent& event)
{
	if ( m_started == false ) return;

	if ( m_menu != NULL ) delete m_menu;

	m_menu = new wxMenu;
	wxMenuItem *item;
	
	item = new wxMenuItem(m_menu, ID_MEMORY_RANGE, 
			wxT("Memory Range"));
	m_menu->Append(item);

	PopupMenu(m_menu);
}

void Memory::OnMemoryRange(wxCommandEvent& WXUNUSED(event))
{
	MemoryRange dlg(this, myFrame, this, wxID_ANY, wxT("Memory Range"));
	wxSize size = dlg.GetBestSize();
	dlg.SetSize(wxSize(250, size.GetHeight()));
    dlg.CentreOnParent();
	dlg.ShowModal();
}

void Memory::OnCellValueChanged( wxGridEvent& ev )
{
    int row = ev.GetRow(),
        col = ev.GetCol();
	
	long here = m_start + (row*(sizeof(long)*m_column)) + (col*sizeof(long));

    wxLogMessage(wxT("Value changed for cell at row %d, col %d: now \"%s\""),
                 row, col, GetCellValue(row, col).c_str());

    ev.Skip();
}

// ---------------------------------------------
BEGIN_EVENT_TABLE(MemoryRange, wxDialog)
END_EVENT_TABLE()

MemoryRange::MemoryRange(wxWindow* parent,
	MyGdbFrame *myFrame,
	Memory *myParent,
	wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style)
	: wxDialog(parent, id, title, pos, size, style)
{
	this->myFrame = myFrame;
	this->myParent = myParent;
	
	wxAuiDockArt *dockArt = myFrame->GetDockArt();
	
	wxBoxSizer *mainSizer= new wxBoxSizer(wxHORIZONTAL);
	
	wxBoxSizer *leftSizer= new wxBoxSizer(wxVERTICAL);
	mainSizer->Add( leftSizer, 1, wxALL, 5);

	// ----------------
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText *label = new wxStaticText(this, wxID_ANY, wxT("Start Address:"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	label->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	sizer->Add( label, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);

	wxString value = wxString::Format(wxT("0x%x"), myParent->m_start);

	m_startTextCtrl = new wxTextCtrl(this, wxID_ANY, 
		value, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_startTextCtrl->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	sizer->Add( m_startTextCtrl, 1, wxEXPAND|wxTOP|wxRIGHT, 3);
	leftSizer->Add( sizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 3);
	
	// ----------------
	sizer = new wxBoxSizer(wxVERTICAL);

	label = new wxStaticText(this, wxID_ANY, wxT("Count:"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	label->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	sizer->Add( label, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);

	value = wxString::Format(wxT("%d"), myParent->m_count);

	m_countTextCtrl = new wxTextCtrl(this, wxID_ANY, 
		value, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_countTextCtrl->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	sizer->Add( m_countTextCtrl, 1, wxEXPAND|wxTOP|wxRIGHT, 3);
	leftSizer->Add( sizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 3);
	
	// ----------------
	sizer = new wxBoxSizer(wxVERTICAL);

	label = new wxStaticText(this, wxID_ANY, wxT("Show Column:"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	label->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	sizer->Add( label, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);

	value = wxString::Format(wxT("%d"), myParent->m_column);

	m_columnTextCtrl = new wxTextCtrl(this, wxID_ANY, 
		value, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_columnTextCtrl->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	sizer->Add( m_columnTextCtrl, 1, wxEXPAND|wxTOP|wxRIGHT, 3);
	leftSizer->Add( sizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 3);
	
	// ----------------
	wxBoxSizer *rightSizer= new wxBoxSizer(wxVERTICAL);
	mainSizer->Add( rightSizer, 0, wxTOP|wxRIGHT, 3);

	m_okButton = new wxButton(this, wxID_OK, wxT("Ok"));
	m_okButton->SetDefault();
    m_okButton->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, 
			(wxObjectEventFunction) (wxEventFunction) 
			&MemoryRange::OnOk);
	rightSizer->Add( m_okButton, 0, wxALIGN_RIGHT, 0);
	
	m_closeButton = new wxButton(this, wxID_CANCEL, wxT("Close"));
    m_closeButton->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, 
			(wxObjectEventFunction) (wxEventFunction) 
			&MemoryRange::OnClose);
	rightSizer->Add(m_closeButton, 0, wxALIGN_RIGHT|wxLEFT|wxTOP, 3);
	
	m_startTextCtrl->SetFocus();

	// --------------------
	SetSizer( mainSizer );
}

MemoryRange::~MemoryRange () 
{
}

void MemoryRange::OnOk(wxCommandEvent& WXUNUSED(event))
{
	wxString value = m_startTextCtrl->GetValue();
	long start;
	if ( value.ToLong(&start, 16) == false ) return;
	myParent->m_start = start;
	
	value = m_countTextCtrl->GetValue();
	long count;
	if ( value.ToLong(&count, 10) == false ) return;
	myParent->m_count = count;
	
	value = m_columnTextCtrl->GetValue();
	long column;
	if ( value.ToLong(&column, 10) == false ) return;
	myParent->m_column = column;

	myParent->Build();
	
	EndDialog(wxID_OK);
}

void MemoryRange::OnClose(wxCommandEvent& WXUNUSED(event))
{
	EndDialog(wxID_CLOSE);
}
