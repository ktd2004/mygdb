// vim: set fdm=marker ts=4 sw=4:

#include "MyGDB.h"

BEGIN_EVENT_TABLE(Register, wxTreeListCtrl)
END_EVENT_TABLE()

Register::Register(wxWindow* parent,
	MyGdbFrame *myFrame,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style)
	: wxTreeListCtrl(parent, id, pos, size, style)
{
	this->myFrame = myFrame;

	AddColumn(wxT("Name"), 50, wxALIGN_LEFT);
	AddColumn(wxT("Value"), 50, wxALIGN_LEFT);
	AddColumn(wxT("Value"), 50, wxALIGN_LEFT);
	
	m_imgList = new wxImageList (16, 16, true);
	m_imgList->Add(GET_BITMAP("cpu"));
	SetImageList(m_imgList);
	
	rootItem = AddRoot (wxT("Register"));

	SetWindowStyle(wxTR_ROW_LINES | wxTR_FULL_ROW_HIGHLIGHT |
		wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT | wxTR_EXTENDED );

	m_menu=NULL;
}

Register::~Register () {
	delete m_imgList;
}

void Register::Clear()
{
	DeleteChildren(rootItem);
}

void Register::Build()
{
	SaveState();

	Clear();
	
	wxString error = wxString::Format(wxT("%c%cerror"), 26, 26);
	
	// ----------- info locals
	wxString msg = DEBUGGER->Eval(wxT("info registers"));

	// check error 
	if ( msg.Find(error) != wxNOT_FOUND ) return;

	wxString result = DEBUGGER->RemoveAnnotate(msg);
	Print(result, rootItem);
	
	LoadState();
}

void Register::Print(wxString result, wxTreeItemId item)
{
	// print 
	wxString spliter = wxEmptyString;
	if ( result.Find(wxT("\r\n")) != wxNOT_FOUND )
		spliter = wxT("\r\n");
	else
		spliter = wxT("\n");

	wxStringTokenizer tkz(result, spliter);
	while ( tkz.HasMoreTokens() ) 
	{
		wxString token = tkz.GetNextToken();
		token.Replace(wxT("  "), wxT("\t"));

		wxStringTokenizer tkz2(token, wxT("\t"));
		wxString name = tkz2.GetNextToken();
		wxString value1 = tkz2.GetNextToken();
		wxString value2 = tkz2.GetNextToken();

		wxTreeItemId child = AppendItem (item, name);
		SetItemImage(child, 0, 0);
		SetItemText (child, 1, value1);
		SetItemText (child, 2, value2);
	}
		

	AutoResize();
}

void Register::SaveTreeNodeState(wxTreeItemId id, wxString path)
{
	if(!id.IsOk()) return;

	wxString _path = path;
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild(id, cookie);

	while(child.IsOk())
	{
		if ( GetChildrenCount(child) > 0 && IsExpanded(child) )
		{
			path = path + wxT("/") + GetItemText(child);
			nodeExpandedArray.Add(path);

			SaveTreeNodeState(child, path);
			path = _path;
		}

		child = GetNextChild(id, cookie);
	}
}

void Register::SaveState ()
{
	wxTreeItemId rootItem = GetRootItem();
	if(rootItem.IsOk())
	{
		nodeExpandedArray.Clear();
		SaveTreeNodeState(rootItem, wxT(""));
	}
}

void Register::LoadState ()
{
	Freeze();

	wxTreeItemId rootItem = GetRootItem();
	if(rootItem.IsOk())
	{
		LoadTreeNodeState(rootItem, wxT(""));
	}

	Thaw();
}

void Register::LoadTreeNodeState(wxTreeItemId id, wxString path)
{
	if(!id.IsOk()) return;

	wxString _path = path;
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild(id, cookie);

	while(child.IsOk())
	{
		if ( GetChildrenCount(child) > 0 )
		{
			path = path + wxT("/") + GetItemText(child);

			for(unsigned int i=0; i<nodeExpandedArray.GetCount(); i++)
			{
				if ( nodeExpandedArray.Item(i) == path )
					Expand(child);
			}

			LoadTreeNodeState(child, path);
			path = _path;
		}

		child = GetNextChild(id, cookie);
	}
}

void Register::AutoResize()
{
	for(int col=0; col<GetColumnCount(); col++)
	{
		wxTreeItemIdValue cookie;
		wxTreeItemId child = GetFirstChild(rootItem, cookie);
		int maxWidth=0;
		while(child.IsOk())
		{
			wxString text = GetItemText(child, col);
			int width, height;
			GetTextExtent(text, &width, &height);
			if(col==0)width+=50;
			if ( width > maxWidth ) maxWidth = width;

			child = GetNextChild(rootItem, cookie);
		}

		SetColumnWidth(col, maxWidth);
	}
}
