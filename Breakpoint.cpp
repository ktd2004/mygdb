// vim: set fdm=marker ts=4 sw=4:

#include "MyGDB.h"

BEGIN_EVENT_TABLE(Breakpoint, wxTreeListCtrl)
	EVT_LEFT_DCLICK(Breakpoint::OnLeftDClick)
	EVT_TREE_ITEM_RIGHT_CLICK(-1, Breakpoint::OnRightClick)
	EVT_MENU(ID_DELETE_BREAKPOINT, Breakpoint::OnDeleteBreakpoint)
	EVT_MENU(ID_ENABLE_BREAKPOINT, Breakpoint::OnEnableBreakpoint)
	EVT_MENU(ID_DISABLE_BREAKPOINT, Breakpoint::OnDisableBreakpoint)
END_EVENT_TABLE()

Breakpoint::Breakpoint(wxWindow* parent,
	MyGdbFrame *myFrame,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style)
	: wxTreeListCtrl(parent, id, pos, size, style)
{
	this->myFrame = myFrame;
			
	AddColumn(wxT("Num"), 50, wxALIGN_LEFT);

	m_imgList = new wxImageList (16, 16, true);
	m_imgList->Add(GET_BITMAP("breakpoint"));
	SetImageList(m_imgList);
	
	rootItem = AddRoot (wxT("Breakpoint"));
	
	SetWindowStyle(wxTR_ROW_LINES | wxTR_FULL_ROW_HIGHLIGHT |
		wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT | wxTR_EXTENDED );

	numColNo=0;
	whatColNo=0;
	addressColNo=0;

	m_menu=NULL;
}

Breakpoint::~Breakpoint () {
	delete m_imgList;
}

void Breakpoint::Clear()
{
	DeleteChildren(rootItem);
}

void Breakpoint::Build()
{
	SaveState();
	
	Clear();

	wxString error = wxString::Format(wxT("%c%cerror"), 26, 26);
	
	// ----------- breakpoints
	wxString msg = DEBUGGER->Eval(wxT("info breakpoints"));

	// check error 
	if ( msg.Find(error) != wxNOT_FOUND ) return;

	Print(msg, rootItem);
	
	// ---------- display breakpoint
	std::vector<CodeEditor*> editors;
	myFrame->GetAllEditors(editors);
	
	for (size_t i = 0; i < editors.size(); i++) 
		editors[i]->ShowBreakpoint();
	
	LoadState();
}

void Breakpoint::Print(wxString msg, wxTreeItemId item)
{
	wxString parse = DEBUGGER->Parse(msg);

	//DEBUGGER->Log(parse);

	wxString value = wxEmptyString;
	wxTreeItemId child;

	wxString breakpoints = wxEmptyString;

	bool startHeader = false;
	bool drawHeader = false;
	bool startTable = false;
	int col=0;
	int totalCol=0;
	wxStringTokenizer tkz(parse, wxString::Format(wxT("%c%c"), 26, 26));
	wxString at = wxEmptyString;
	while ( tkz.HasMoreTokens() ) 
	{
		wxString token = tkz.GetNextToken();

		wxString tag = wxT("breakpoints-headers");
		if ( token.StartsWith(tag) )
		{
			startHeader = true;
		}
		
		tag = wxT("breakpoints-table");
		if ( token.StartsWith(tag) )
		{
			drawHeader = true;	// start header
			startHeader = false;
		}

		tag = wxT("field 0");
		if ( drawHeader && token.StartsWith(tag) )
			startTable = true;
		
		// header name
		tag = wxT("field ");
		if ( startHeader == true && drawHeader == false && 
				token.StartsWith(tag) )
		{
			value = token.SubString(tag.Length(), token.Length()); 
			value = value.SubString(value.Find(wxT(" ")), value.Length()); 
			if ( totalCol > 0 )
			{
				if (GetColumnCount() <= totalCol )
				{
					value.Replace(wxT("\r"), wxT(""));
					value.Replace(wxT("\n"), wxT(""));
					value.Replace(wxT("\t"), wxT(""));
					value.Trim(true);
					value.Trim(false);

					if(value == wxT("Num"))
					{
						numColNo = totalCol;
					}
					else if(value == wxT("What"))
					{
						whatColNo = totalCol;
					}
					else if(value == wxT("Address"))
					{
						addressColNo = totalCol;
					}

					AddColumn(value, 50, wxALIGN_LEFT);
				}
			}
			totalCol++;
		}
		
		// table
		if ( startTable == true && drawHeader == true && 
				token.StartsWith(tag) )
		{
			value = token.SubString(tag.Length(), token.Length()); 
			value = value.SubString(value.Find(wxT(" ")), value.Length()); 

			wxString spliter = wxEmptyString;
			if ( value.Find(wxT("\r\n")) != wxNOT_FOUND )
				value = value.SubString(0, value.Find(wxT("\r\n")));
			else if ( value.Find(wxT("\n")) != wxNOT_FOUND )
				value = value.SubString(0, value.Find(wxT("\n")));

			value.Replace(wxT("\r"), wxT(""));
			value.Replace(wxT("\n"), wxT(""));
			value.Replace(wxT("\t"), wxT(""));
			value.Trim(true);
			value.Trim(false);

			// breakpoint number
			if ( numColNo == col )
				at = at + value;

			// address
			if ( addressColNo == col )
			{
				at = at + wxT("|");
				at = at + value;
			}

			// source & line
			if ( whatColNo == col )
			{
				if ( value.Length() > 0 )
				{
					wxString tag = wxT(" at ");
					long pos = value.Find(tag);
					if ( pos != wxNOT_FOUND )
					{
						at = at + wxT("|");

						wxString where = value.SubString(
								pos+tag.Length(), value.Length()); 
						// find ":" from end
						long pos2 = where.Find(':', true);
						where.SetChar(pos2, '|');

						at = at + where;
					}
				}

				breakpoints = breakpoints + at + wxT(";");
				at = wxEmptyString;
			}

			if ( col == 0 )
			{
				child = AppendItem (rootItem, value);
				SetItemImage(child, 0, 0);
			}
			else
			{
				SetItemText (child, col, value);
			}

			col++;
			
			if ( col == totalCol )
			{
				col = 0;
				startTable = false;
			}
		}
	}

	CONFIG->SetProperty(wxT("DEBUG/BREAKPOINT"), wxT("value"), breakpoints);
	CONFIG->Save();

	AutoResize();
}

void Breakpoint::SaveTreeNodeState(wxTreeItemId id, wxString path)
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

void Breakpoint::SaveState ()
{
	wxTreeItemId rootItem = GetRootItem();
	if(rootItem.IsOk())
	{
		nodeExpandedArray.Clear();
		SaveTreeNodeState(rootItem, wxT(""));
	}
}

void Breakpoint::LoadState ()
{
	Freeze();

	wxTreeItemId rootItem = GetRootItem();
	if(rootItem.IsOk())
	{
		LoadTreeNodeState(rootItem, wxT(""));
	}

	Thaw();
}

void Breakpoint::LoadTreeNodeState(wxTreeItemId id, wxString path)
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

void Breakpoint::AutoResize()
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
			else width+=20;
			if ( width > maxWidth ) maxWidth = width;

			child = GetNextChild(rootItem, cookie);
		}

		SetColumnWidth(col, maxWidth);
	}
}

void Breakpoint::OnLeftDClick(wxMouseEvent& event)
{
	wxTreeItemId item = GetSelection();
	wxString what = GetItemText(item, whatColNo);
				
	if ( what.Length() > 0 )
	{
		wxString tag = wxT(" at ");
		long pos = what.Find(tag);
		if ( pos != wxNOT_FOUND )
		{
			wxString value = what.SubString(
					pos+tag.Length(), what.Length()); 
			long pos2 = value.Find(':');
			wxString sourceFile = value.SubString(0, pos2-1);
			sourceFile.Trim(true);
			sourceFile.Trim(false);
			wxString sourceLine = value.SubString(pos2+1, value.Length()-1);
			sourceLine.Trim(true);
			sourceLine.Trim(false);

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
	}
}

void Breakpoint::OnRightClick(wxTreeEvent& event)
{
	SelectItem(event.GetItem());

	if ( m_started == false ) return;

	wxString num = GetItemText(event.GetItem(), 0);
	
	if ( m_menu != NULL ) delete m_menu;

	m_menu = new wxMenu;
	wxMenuItem *item;

	item = new wxMenuItem(m_menu, ID_DELETE_BREAKPOINT, wxT("Delete"));
	m_menu->Append(item);
	m_menu->AppendSeparator();
	item = new wxMenuItem(m_menu, ID_ENABLE_BREAKPOINT, wxT("Enable"));
	m_menu->Append(item);
	item = new wxMenuItem(m_menu, ID_DISABLE_BREAKPOINT, wxT("Disable"));
	m_menu->Append(item);

	PopupMenu(m_menu);
}

void Breakpoint::OnDeleteBreakpoint(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId item = GetSelection();

	int num = atoi((char*)GetItemText(item, 0).c_str());

	wxString cmd = wxString::Format(wxT("delete %d"), num);
	DEBUGGER->Eval(cmd);

	Build();
}

void Breakpoint::OnEnableBreakpoint(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId item = GetSelection();

	int num = atoi((char*)GetItemText(item, 0).c_str());

	wxString cmd = wxString::Format(wxT("enable %d"), num);
	DEBUGGER->Eval(cmd);

	Build();
}

void Breakpoint::OnDisableBreakpoint(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId item = GetSelection();

	int num = atoi((char*)GetItemText(item, 0).c_str());

	wxString cmd = wxString::Format(wxT("disable %d"), num);
	DEBUGGER->Eval(cmd);

	Build();
}
