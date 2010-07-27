// vim: set fdm=marker ts=4 sw=4:

#include "MyGDB.h"

BEGIN_EVENT_TABLE(Watch, wxTreeListCtrl)
END_EVENT_TABLE()

static wxRegEx reRepeatedElements(_T("repeats ([0-9]+) times>"));
	
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

// part of Code::Blocks
int Watch::FindCommaPos(const wxString& str)
{
    // comma is a special case because it separates the fields
    // but it can also appear in a function/template signature, where
    // we shouldn't treat it as a field separator

    // what we 'll do now, is decide if the comma is inside
    // a function signature.
    // we 'll do it by counting the opening and closing parenthesis/angled-brackets
    // *up to* the comma.
    // if they 're equal, it's a field separator.
    // if they 're not, it's in a function signature
    // ;)

    int len = str.Length();
    int i = 0;
    int parCount = 0;
    int braCount = 0;
    bool inQuotes = false;
    while (i < len)
    {
        wxChar ch = str.GetChar(i);
        switch (ch)
        {
            case _T('('):
                ++parCount; // increment on opening parenthesis
                break;

            case _T(')'):
                --parCount; // decrement on closing parenthesis
                break;

            case _T('<'):
                ++braCount; // increment on opening angle bracket
                break;

            case _T('>'):
                --braCount; // decrement on closing angle bracket
                break;

            case _T('"'):
                // fall through
            case _T('\''):
                inQuotes = !inQuotes; // toggle inQuotes flag
                break;

            default:
                break;
        }

        // if it's not inside quotes *and* we have parCount == 0, it's a field separator
        if (!inQuotes && parCount == 0 && braCount == 0 && ch == _T(','))
            return i;
        ++i;
    }
    return -1;
}

// part of Code::Blocks
int Watch::FindCharOutsideQuotes(const wxString& str, wxChar ch)
{
    int len = str.Length();
    int i = 0;
    bool inSingleQuotes = false;
    bool inDoubleQuotes = false;
    wxChar lastChar = _T('\0');
    while (i < len)
    {
        wxChar currChar = str.GetChar(i);

        // did we find the char outside of any quotes?
        if (!inSingleQuotes && !inDoubleQuotes && currChar == ch)
            return i;

        // double quotes (not escaped)
        if (currChar == _T('"') && lastChar != _T('\\'))
        {
            // if not in single quotes, toggle the flag
            if (!inSingleQuotes)
                inDoubleQuotes = !inDoubleQuotes;
        }
        // single quotes (not escaped)
        else if (currChar == _T('\'') && lastChar != _T('\\'))
        {
            // if not in double quotes, toggle the flag
            if (!inDoubleQuotes)
                inSingleQuotes = !inSingleQuotes;
        }
        // don't be fooled by double-escape
        else if (currChar == _T('\\') && lastChar == _T('\\'))
        {
            // this will be assigned to lastChar
            // so it's not an escape char
            currChar = _T('\0');
        }

        lastChar = currChar;
        ++i;
    }
    return -1;
}

void Watch::Build()
{
	SaveState();

	Clear();
	
	wxString info = wxEmptyString;

	wxString error = wxString::Format(wxT("%c%cerror"), 26, 26);
	
	// ----------- info locals
	wxString msg = DEBUGGER->Eval(wxT("info locals"));

	// check error 
	if ( msg.Find(error) != wxNOT_FOUND ) return;
	
	msg = DEBUGGER->RemoveAnnotate(msg);

	wxTreeItemId item = AppendItem (rootItem, wxT("Locals"));
	SetItemImage(item, 0, 2);
	PrintVariable(msg, item);
	
	// ----------- info args
	msg = DEBUGGER->Eval(wxT("info args"));

	// check error 
	if ( msg.Find(error) != wxNOT_FOUND ) return;
	
	msg = DEBUGGER->RemoveAnnotate(msg);

	item = AppendItem (rootItem, wxT("Arguments"));
	SetItemImage(item, 0, 2);
	PrintVariable(msg, item);
	
	LoadState();
}

#define PRETTY_PRINT

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

#ifdef PRETTY_PRINT
		ParseEntry(item, token);
#else
		wxString name = GetVariable(token);
		wxString value = GetValue(token);

		if ( name.Length() == 0 )
		{
			AppendItem (item, token);
		}
		else
		{
			wxTreeItemId child = AppendItem (item, name);
			SetItemText (child, 1, value);
		}
#endif
	}
}

wxString Watch::GetVariable(wxString text)
{
	wxString tag = wxT(" = ");
	int pos = text.Find(tag);
	if ( pos == wxNOT_FOUND )
		return wxEmptyString;

	return text.SubString(0, pos-1);
}

wxString Watch::GetValue(wxString text)
{
	wxString tag = wxT(" = ");
	int pos = text.Find(tag);
	if ( pos == wxNOT_FOUND )
		return wxEmptyString;

	return text.SubString(pos+tag.Length(), text.Length()-1);
}

// part of Code::Blocks and some modified by ihmin
void Watch::ParseEntry (wxTreeItemId parent, wxString &text)
{
    if (text.IsEmpty())
        return;

    while (1)
    {
        // trim the string from left and right
        text.Trim(true);
        text.Trim(false);

        // find position of '{', '}' and ',' ***outside*** of any quotes.
        // decide which is nearer to the start
        int braceOpenPos = FindCharOutsideQuotes(text, _T('{'));
        if (braceOpenPos == -1)    braceOpenPos = 0xfffffe;
        int braceClosePos = FindCharOutsideQuotes(text, _T('}'));
        if (braceClosePos == -1) braceClosePos = 0xfffffe;
        int commaPos = FindCommaPos(text);
        if (commaPos == -1) commaPos = 0xfffffe;
        int pos = std::min(commaPos, std::min(braceOpenPos, braceClosePos));

        if (pos == 0xfffffe || GetValue(text).GetChar(0) == _T('"'))
        {
            // no comma, opening or closing brace 
            if (text.Right(3).Matches(_T(" = ")))
                text.Truncate(text.Length() - 3);
            if (!text.IsEmpty())
            {
                wxString name = GetVariable(text);
                wxString value = GetValue(text);

				if ( name.Length() == 0 )
					AppendItem(parent, text);
				else
				{
					wxTreeItemId item = AppendItem(parent, name);
					SetItemText(item, 1, value);
				}
                text.Clear();
            }
            break;
        }
		else
		{
			// found '{' and '}'
            if ( braceOpenPos != 0xfffffe && braceClosePos != 0xfffffe)
            {
                wxString tmp = text.Left(braceClosePos + 1);
                if (text.Freq(_T('{')) == 1 && text.Freq(_T('}')) == 1)
                {
					int commas = 8;
                    if (tmp.Freq(_T(',')) < commas)
                    {
						wxString name = GetVariable(tmp);
						wxString value = GetValue(tmp);

						if ( name.Length() == 0 )
							AppendItem(parent, tmp);
						else
						{
							wxTreeItemId item = AppendItem(parent, name);
							SetItemText(item, 1, value);
						}

						//AppendItem(parent, tmp);
                        text.Remove(0, braceClosePos + 1);
                        continue;
					}
				}
			}

            wxString tmp = text.Left(pos);
            wxTreeItemId newchild;

            if (tmp.Right(3).Matches(_T(" = ")))
				tmp.Truncate(tmp.Length() - 3); // remove " = " if last in string

            if (!tmp.IsEmpty())
            {
                //newchild = AppendItem(parent, tmp);
				wxString name = GetVariable(tmp);
				wxString value = GetValue(tmp);

				if ( name.Length() == 0 )
					newchild = AppendItem(parent, tmp);
				else
				{
					newchild = AppendItem(parent, name);
					SetItemText(newchild, 1, value);
				}
			}
            text.Remove(0, pos + 1);

            if (pos == braceOpenPos)
            {
                if (!newchild.IsOk())
                    newchild = parent;

                ParseEntry(newchild, text); // proceed one level deeper
			}
			else if (pos == braceClosePos)
				break; // return one level up
		}
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
