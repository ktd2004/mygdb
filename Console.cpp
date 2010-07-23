// vim: set fdm=marker ts=4 sw=4:

#include "MyGDB.h"

// ----------------------
BEGIN_EVENT_TABLE(Console, wxStyledTextCtrl)
	EVT_STC_CHARADDED (wxID_ANY, Console::OnCharAdded)
    EVT_KEY_DOWN(Console::OnKeyPressed)
    EVT_KEY_UP(Console::OnKeyReleased)
	EVT_LEFT_UP(Console::OnLeftUp)
	EVT_MIDDLE_DOWN(Console::OnMiddleDown)
END_EVENT_TABLE()

Console::Console(
	wxWindow *parent,
	MyGdbFrame *myFrame,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style)
	: wxStyledTextCtrl(parent, id, pos, size, style)
{
	this->myFrame = myFrame;
	
	m_idle = false;

	m_startPos = -1;
	m_cmdIndex = -1;

	UsePopUp(false);
	SetUseAntiAliasing(true);
	SetEOLMode(wxSTC_EOL_LF);
	SetCodePage(wxSTC_CP_UTF8);
	SetWrapMode(wxSTC_WRAP_WORD);
	SetReadOnly(true);

	SetSavePoint();
	EmptyUndoBuffer();
	
	StyleClearAll();

	SetStyle();

	m_completion = new Completion(this);
	m_completion->SetAutoHide(true);
}

Console::~Console () 
{
}

void Console::OnIdle(wxIdleEvent & evt)
{
#if 0
	static int count=0;
	FILE *fp = fopen("idle.log", "a");
	fprintf(fp, "idle.. %d\n", count++);
	fclose(fp);
#endif

	STATUS_BAR->SetStatusText( wxT("Waiting for Response..."), 1);

	DEBUGGER->Flush();
}

void Console::Initial()
{
	m_startPos = -1;
	m_cmdIndex = -1;
	m_cmdList.Clear();

	ClearAll();
}

void Console::DisconnectIdle()
{
	if ( m_idle == true )
	{
		Disconnect(wxID_ANY, wxEVT_IDLE, 
				wxIdleEventHandler(Console::OnIdle));
		m_idle = false;
		STATUS_BAR->SetStatusText( wxT(""), 1);
	}
}

void Console::ConnectIdle ()
{
	if ( m_idle == false )
	{
		Connect(wxID_ANY, wxEVT_IDLE, 
				wxIdleEventHandler(Console::OnIdle));
		m_idle = true;
	}
}

void Console::SetStyle()
{
	wxString styleName = CONFIG->GetProperty(wxT("EDITOR/STYLE"), wxT("value"));
	wxXmlDocument *xml = STC_STYLE->OpenStyle(styleName);
	STC_STYLE->ApplyConsoleStyle(this);
	STC_STYLE->CloseStyle(xml);
}

// 왼쪽 마우스로 블럭 선택시 자동 복사됨
void Console::OnLeftUp(wxMouseEvent& event)
{
	if ( GetSelectedText().Length() > 0 ) 
		Copy();

	event.Skip();
}

// 가운데 마우스 버튼 눌릴시 paste
void Console::OnMiddleDown(wxMouseEvent& event)
{
	if ( CanPaste() )
		Paste();

	event.Skip();
}

void Console::Puts(wxString str)
{
	Puts(str, MYGDB_STDOUT);
}

void Console::Puts(wxString str, int type)
{
	unsigned long pos1, pos2;

	pos1 = GetLength();
	AddText(str);
	pos2 = GetLength();

	if ( type == MYGDB_STDERR ) 
	{
		StartStyling(pos1, 0xff);
		SetStyling(pos2-pos1, 4);

	} 
	else if ( type == MYGDB_STDOUT ) 
	{
		StartStyling(pos1, 0xff);
		SetStyling(pos2-pos1, 5);

	} 
	else if ( type == MYGDB_PROMPT ) 
	{
		StartStyling(pos1, 0xff);
		SetStyling(pos2-pos1, 6);
	}
	
	//GotoPos(GetLength());
}

void Console::OnKeyReleased(wxKeyEvent& WXUNUSED(event))
{
}

void Console::HideCompletionBox()
{
	if (m_completion->IsShown()) {
		m_completion->Hide();
	}
}

wxString Console::GetWordAtCaret()
{
	// Get the partial word that we have
	long pos = GetCurrentPos();
	long start = WordStartPosition(pos, true);
	long end   = WordEndPosition(pos, true);
	return GetTextRange(start, end);
}

void Console::OnCharAdded(wxStyledTextEvent& event)
{
    int pos = GetCurrentPos();
    int key = event.GetKey();

	if (m_completion->IsShown()) {
		int start = WordStartPosition(pos, true);
		wxString word = GetTextRange(start, pos);

		if ( word.IsEmpty() ) {
			HideCompletionBox();
		} else {
			m_completion->SelectWord(word);
		}
	}
}

void Console::SetStartPos(long pos)
{
	m_startPos = pos;
}

void Console::OnKeyPressed(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	int pos = GetCurrentPos();

	if (m_completion->IsShown()) 
	{
		switch (key)
		{
			case WXK_NUMPAD_ENTER:
			case WXK_RETURN:
			case WXK_TAB:
				m_completion->InsertSelection();
				HideCompletionBox();
				return;

			case WXK_ESCAPE:
			case WXK_LEFT:
			case WXK_RIGHT:
			case WXK_HOME:
			case WXK_END:
			case WXK_DELETE:
			case WXK_NUMPAD_DELETE:
				HideCompletionBox();
				return;
			case WXK_UP:
				m_completion->Previous();
				return;
			case WXK_DOWN:
				m_completion->Next();
				return;
			case WXK_PAGEUP:
				m_completion->PreviousPage();
				return;
			case WXK_PAGEDOWN:
				m_completion->NextPage();
				return;
			case WXK_BACK: 
			{
				if (event.ControlDown()) 
				{
					HideCompletionBox();
				} 
				else 
				{
					wxString word = GetWordAtCaret();
					if (word.IsEmpty()) 
					{
						HideCompletionBox();
					} 
					else 
					{
						word.RemoveLast();
						m_completion->SelectWord(word);
					}
				}
				break;
			}
			default:
				break;
		}
	}
	
	if (pos < m_startPos )
		CmdKeyExecute(wxSTC_CMD_DOCUMENTEND);

	// BackSpace, Left
	if ( key == 8 || key == 314 || key == 376 ) 
	{
		if ( pos <= m_startPos ) 
		{
			GotoPos(m_startPos);
			return;
		}
	}

	// Shift+Home
	if ( (key == 313 || key == 375) && event.ShiftDown() ) 
	{
		SetSelection(pos, m_startPos);
		return;
	}

	// Home
	if ( key == 313 || key == 375) 
	{
		GotoPos(m_startPos);
		return;
	}

	// Ctrl+A	: Home
	if ( key == 65 && event.ControlDown() ) 
	{
		GotoPos(m_startPos);
		return;
	}

	// Ctrl+E	: LineEnd
	if ( key == 69 && event.ControlDown() ) 
	{
		CmdKeyExecute(wxSTC_CMD_LINEEND);
		return;
	}

	// Ctrl+U
	if ( key == 85 && event.ControlDown() ) 
	{
		SetTargetStart(m_startPos);
		SetTargetEnd(GetTextLength());
		ReplaceTarget(wxT(""));
		GotoPos(m_startPos);
		return;
	}

	// Tab
	// Ctrl+Space or Ctrl+Enter
	if ( key == 9 || ((key == 32 && event.ControlDown()) || 
				(key == 13 && event.ControlDown())) )
	{
		long int endPos = GetCurrentPos();
		wxString cword = GetTextRange(m_startPos, endPos);

		if ( cword.length() > 0 ) 
		{
			wxString annotate = DEBUGGER->Eval(wxT("complete ") + cword);
			wxString complete = DEBUGGER->RemoveAnnotate(annotate);

			std::vector<compItemInfo> items;
			compItemInfo item;

			wxString spliter = wxEmptyString;
			if ( complete.Find(wxT("\r\n")) != wxNOT_FOUND )
				spliter = wxT("\r\n");
			else
				spliter = wxT("\n");

			wxStringTokenizer tkz(complete, spliter);
			while ( tkz.HasMoreTokens() ) 
			{
				wxString token = tkz.GetNextToken();

				compItemInfo item;
				item.kind = COMP_CMD;
				item.name = token;
				items.push_back(item);
			}

			if (items.size() > 0)
			{
				std::sort(items.begin(), items.end(), CompItemInfoSortByName());

				m_completion->ReplaceStartPosition(m_startPos);
				m_completion->SetItems(items);
				m_completion->Show(cword);
				m_completion->Adjust();
			} 
			else 
			{
				wxBell();
			}
		}

		return;
	}

	// Enter, Return
	if ( key == 13 || key == 370 ) 
	{
		if ( m_started == false ) return;
		if ( DEBUGGER->IsProcessing() ) return;

		GotoPos(GetTextLength());

		CmdKeyExecute(wxSTC_CMD_LINEEND);

		long int endPos = GetCurrentPos();
		wxString cmd = GetTextRange(m_startPos, endPos);

		if ( cmd.length() > 0 ) 
		{
			bool exist = false;
			wxStringList::Node *node = m_cmdList.GetFirst();
			while (node) {
				if ( cmd.Cmp(node->GetData()) == 0 ) {
					exist = true;
					break;
				}
				node = node->GetNext();
			}
			if ( exist == false ) {
				m_cmdList.Add(cmd);
			} else {
				m_cmdList.Delete(cmd);
				m_cmdList.Add(cmd);
			}
		} else {
			if ( m_cmdList.GetCount() <= 0 ) return;

			wxStringList::Node *node = m_cmdList.GetLast();
			cmd = node->GetData();
		}

		m_cmdIndex = -1;
		CmdKeyExecute(wxSTC_CMD_NEWLINE);

		DEBUGGER->Write(cmd);
		return;
	}

	// Up
	if ( key == 315 || key == 377 ) 
	{
		if ( m_cmdList.GetCount() > 0 ) 
		{
			m_cmdIndex = m_cmdIndex - 1;

			if ( m_cmdIndex < 0 ) 
			{
				m_cmdIndex = m_cmdList.GetCount()-1;
			}

			int index = 0;
			wxStringList::Node *node = m_cmdList.GetFirst();
			while (node) 
			{
				if ( index == m_cmdIndex ) break;
				node = node->GetNext();
				index+=1;
			}
			wxString cmd = node->GetData();

			Freeze();
			SetTargetStart(m_startPos);
			SetTargetEnd(GetTextLength());
			ReplaceTarget(wxT(""));
			GotoPos(m_startPos);
			AddText(cmd);
			Thaw();
		}
		return;
	}

	// Down
	if ( key == 317 || key == 379 ) 
	{
		if ( m_cmdList.GetCount() > 0 ) 
		{
			m_cmdIndex = m_cmdIndex + 1;

			if ( m_cmdIndex >= (int)m_cmdList.GetCount() ) 
			{
				m_cmdIndex = 0;
			}

			int index = 0;
			wxStringList::Node *node = m_cmdList.GetFirst();
			while (node) 
			{
				if ( index == m_cmdIndex ) break;
				node = node->GetNext();
				index+=1;
			}

			wxString cmd = node->GetData();

			Freeze();
			SetTargetStart(m_startPos);
			SetTargetEnd(GetTextLength());
			ReplaceTarget(wxT(""));
			GotoPos(m_startPos);
			AddText(cmd);
			Thaw();
		}
		return;
	}

	event.Skip(true);
}
