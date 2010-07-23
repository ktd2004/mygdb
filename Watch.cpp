// vim: set fdm=marker ts=4 sw=4:

#include "MyGDB.h"

BEGIN_EVENT_TABLE(Watch, wxTreeListCtrl)
END_EVENT_TABLE()

Watch::Watch(wxWindow* parent,
	MyGdbFrame *myFrame,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style)
	: wxTreeListCtrl(parent, id, pos, size, style)
{
	this->myFrame = myFrame;

	AddColumn(_T("Name"), 140, wxALIGN_LEFT);
	AddColumn(_T("Value"), 150, wxALIGN_LEFT);
	
	m_imgList = new wxImageList (16, 16, true);
	m_imgList->Add(GET_BITMAP("open_node"));
	m_imgList->Add(GET_BITMAP("close_node"));
	m_imgList->Add(GET_BITMAP("viewmag"));
	SetImageList(m_imgList);
	
	rootItem = AddRoot (wxT("Watch"));
	SetItemImage(rootItem, 0);

	SetWindowStyle(wxTR_ROW_LINES | wxTR_FULL_ROW_HIGHLIGHT |
		wxTR_LINES_AT_ROOT | wxTR_HAS_BUTTONS | 
		wxTR_HIDE_ROOT | wxTR_EXTENDED );

	m_menu=NULL;
}

Watch::~Watch () {
	delete m_imgList;
}

void Watch::Clear()
{
	DeleteChildren(rootItem);
}

void Watch::Build()
{
	SaveState();

	Clear();
	
	wxString error = wxString::Format(wxT("%c%cerror"), 26, 26);
	
	// ----------- info locals
	wxString msg = DEBUGGER->Eval(wxT("info locals"));

	// check error 
	if ( msg.Find(error) != wxNOT_FOUND ) return;
	
	wxString result = DEBUGGER->RemoveAnnotate(msg);

	wxTreeItemId item = AppendItem (rootItem, wxT("Locals"));
	SetItemImage(item, 0, 2);
	PrintVariable(result, item);
	
	// ----------- info args
	msg = DEBUGGER->Eval(wxT("info args"));

	// check error 
	if ( msg.Find(error) != wxNOT_FOUND ) return;
	
	result = DEBUGGER->RemoveAnnotate(msg);

	item = AppendItem (rootItem, wxT("Arguments"));
	SetItemImage(item, 0, 2);
	PrintVariable(result, item);
	
	LoadState();
}

void Watch::PrintVariable(wxString result, wxTreeItemId item)
{
	// print varaibles
	wxString spliter = wxEmptyString;
	if ( result.Find(wxT("\r\n")) != wxNOT_FOUND )
		spliter = wxT("\r\n");
	else
		spliter = wxT("\n");

	wxStringTokenizer tkz(result, spliter);
	while ( tkz.HasMoreTokens() ) 
	{
		wxString token = tkz.GetNextToken();

		int pos = token.Find('=');
		wxString name = token.SubString(0, pos-1);

		wxString value = wxEmptyString;
		if ( pos != wxNOT_FOUND )
			value = token.SubString(pos+1, token.Length()-1);

		wxTreeItemId child = AppendItem (item, name);
		SetItemText (child, 1, value);
	}
}

void Watch::SaveTreeNodeState(wxTreeItemId id, wxString path)
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

void Watch::SaveState ()
{
	wxTreeItemId rootItem = GetRootItem();
	if(rootItem.IsOk())
	{
		nodeExpandedArray.Clear();
		SaveTreeNodeState(rootItem, wxT(""));
	}
}

void Watch::LoadState ()
{
	Freeze();

	wxTreeItemId rootItem = GetRootItem();
	if(rootItem.IsOk())
	{
		LoadTreeNodeState(rootItem, wxT(""));
	}

	Thaw();
}

void Watch::LoadTreeNodeState(wxTreeItemId id, wxString path)
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
