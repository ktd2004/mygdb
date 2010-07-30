// vim: set fdm=marker ts=4 sw=4:

#include "MyGDB.h"

BEGIN_EVENT_TABLE(CallStack, wxTreeListCtrl)
	EVT_LEFT_DCLICK(CallStack::OnLeftDClick)
END_EVENT_TABLE()

CallStack::CallStack(wxWindow* parent,
	MyGdbFrame *myFrame,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style)
	: wxTreeListCtrl(parent, id, pos, size, style)
{
	this->myFrame = myFrame;

	AddColumn(_T("Level"), 10, wxALIGN_LEFT);
	AddColumn(_T("Address"), 50, wxALIGN_LEFT);
	AddColumn(_T("Function"), 50, wxALIGN_LEFT);
	AddColumn(_T("Source"), 50, wxALIGN_LEFT);
	AddColumn(_T("Line"), 50, wxALIGN_LEFT);
	
	m_imgList = new wxImageList (16, 16, true);
	m_imgList->Add(GET_BITMAP("stack"));
	m_imgList->Add(GET_BITMAP("pointer"));
	SetImageList(m_imgList);
	
	rootItem = AddRoot (wxT("CallStack"));

	SetWindowStyle(wxTR_ROW_LINES | wxTR_FULL_ROW_HIGHLIGHT |
		wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT | wxTR_EXTENDED );

	m_menu=NULL;
}

CallStack::~CallStack () {
	delete m_imgList;
}

void CallStack::Clear()
{
	DeleteChildren(rootItem);
}

void CallStack::Build()
{
	SaveState();

	Clear();
	
	wxString error = wxString::Format(wxT("%c%cerror"), 26, 26);
	
	// ----------- bt
	wxString msg = DEBUGGER->Eval(wxT("bt"));
	
	// check error 
	if ( msg.Find(error) != wxNOT_FOUND ) return;

	Print(msg, rootItem);
	
	LoadState();
}

void CallStack::Print(wxString msg, wxTreeItemId item)
{
	wxString parse = DEBUGGER->Parse(msg);

	wxString value = wxEmptyString;
	wxTreeItemId child;
		
	wxStringTokenizer tkz(parse, wxString::Format(wxT("%c%c"), 26, 26));
	while ( tkz.HasMoreTokens() ) 
	{
		wxString token = tkz.GetNextToken();
		
		wxString tag = wxT("frame-begin ");
		if ( token.StartsWith(tag) )
		{
			value = token.SubString(tag.Length(), token.Length()); 
			wxStringTokenizer tkz2(value, wxT(" "));

			value = tkz2.GetNextToken();	// no
			child = AppendItem (item, value);
			SetItemImage(child, 0, 0);

			value = tkz2.GetNextToken();
			SetItemText (child, 1, value);	// address
		}

		tag = wxT("frame-function-name ");
		if ( token.StartsWith(tag) )
		{
			value = token.SubString(tag.Length(), token.Length()); 
			value.Trim(true);
			value.Trim(false);
			SetItemText (child, 2, value);	// function
		}

		tag = wxT("frame-source-file ");
		if ( token.StartsWith(tag) )
		{
			value = token.SubString(tag.Length(), token.Length()); 
			value.Trim(true);
			value.Trim(false);
			SetItemText (child, 3, value);	// source
		}
		
		tag = wxT("frame-source-line ");
		if ( token.StartsWith(tag) )
		{
			value = token.SubString(tag.Length(), token.Length()); 
			value.Trim(true);
			value.Trim(false);
			SetItemText (child, 4, value);	// line
		}
	}

	AutoResize();
}

void CallStack::UpdatePointer(wxString address)
{
	wxTreeItemId id = rootItem;
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild(id, cookie);

	while(child.IsOk())
	{
		wxString text = GetItemText(child, 1);
		text.Trim(true);
		text.Trim(false);

		address.Trim(true);
		address.Trim(false);

		long addr1, addr2;
		text.ToLong(&addr1, 16);
		address.ToLong(&addr2, 16);

		if (addr1 == addr2)
		{
			SetItemImage(child, 0, 1);
			ScrollTo(child);
			break;
		}

		child = GetNextChild(id, cookie);
	}
}

void CallStack::SaveTreeNodeState(wxTreeItemId id, wxString path)
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

void CallStack::SaveState ()
{
	wxTreeItemId rootItem = GetRootItem();
	if(rootItem.IsOk())
	{
		nodeExpandedArray.Clear();
		SaveTreeNodeState(rootItem, wxT(""));
	}
}

void CallStack::LoadState ()
{
	Freeze();

	wxTreeItemId rootItem = GetRootItem();
	if(rootItem.IsOk())
	{
		LoadTreeNodeState(rootItem, wxT(""));
	}

	Thaw();
}

void CallStack::LoadTreeNodeState(wxTreeItemId id, wxString path)
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

void CallStack::AutoResize()
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

void CallStack::OnLeftDClick(wxMouseEvent& event)
{
	wxTreeItemId item = GetSelection();

	wxString sourceFile = GetItemText(item, 3);
	wxString sourceLine = GetItemText(item, 4);

	if ( sourceFile.Length() <= 0 || 
			sourceLine.Length() <= 0 ) return;
	
	// ---------
	wxString sourceLocation = 
		CONFIG->GetProperty(wxT("INVOKE/SOURCELOCATION"), wxT("value"));
	if ( sourceLocation.Length() <= 0 ) return;

	wxFileName path(sourceFile);
	path.MakeAbsolute(sourceLocation);
	
	CodeEditor *editor = myFrame->OpenSource(path.GetFullPath());

	// goto line
	long line;
	sourceLine.ToLong(&line);
	editor->GotoLine(line-1);
}
