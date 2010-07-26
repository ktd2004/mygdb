// vim: set fdm=marker ts=4 sw=4:

#pragma warning(disable: 4819)
#pragma warning(disable: 4100)

#include "MyGDB.h"
#include "xpm/page_white_cplusplus.xpm"
#include "xpm/page_white_c.xpm"
#include "xpm/page_white_h.xpm"
#include "xpm/page_white.xpm"

BEGIN_EVENT_TABLE(CodeEditor, wxAuiMDIChildFrame)
	EVT_IDLE(CodeEditor::OnIdle)
END_EVENT_TABLE()

CodeEditor::CodeEditor(wxAuiMDIParentFrame* parent,
	MyGdbFrame *myFrame,
	wxWindowID id,
	const wxString& file,
	const bool& main)
	: wxAuiMDIChildFrame(parent, id, file)
{
	wxFileName f(file);
	if ( f.GetExt().Lower() == wxT("cpp") )
		SetIcon(wxIcon(page_white_cplusplus_xpm));
	else if ( f.GetExt().Lower() == wxT("c") )
		SetIcon(wxIcon(page_white_c_xpm));
	else if ( f.GetExt().Lower() == wxT("h") )
		SetIcon(wxIcon(page_white_h_xpm));
	else
		SetIcon(wxIcon(page_white_xpm));

	this->myFrame = myFrame;
	
	m_fullPath = file;
	m_fullPath.Replace(wxT("\\"), wxT("/"), true);

	wxString sourceLocation = 
		CONFIG->GetProperty(wxT("INVOKE/SOURCELOCATION"), wxT("value"));
	wxFileName *path = new wxFileName(m_fullPath);
	path->MakeRelativeTo(sourceLocation);
	m_relativePath = path->GetFullPath();
	m_relativePath.Replace(wxT("\\"), wxT("/"), true);
	delete path;

	Freeze();
	
	mainSizer = new wxBoxSizer(wxVERTICAL);

    m_stc = new StyledTextCtrl(this, myFrame, wxID_ANY);
	mainSizer->Add(m_stc, 1, wxEXPAND|wxALL);
	
	UpdateTitle();

	SetSizer(mainSizer);
	
	Thaw();
}

CodeEditor::~CodeEditor ()
{
	delete m_stc;
}

wxString CodeEditor::GetFullPath()
{
	return m_fullPath;
}

wxString CodeEditor::GetRelativePath()
{
	return m_relativePath;
}

void CodeEditor::OnIdle(wxIdleEvent& event)
{
	Layout();
}

void CodeEditor::ShowBreakpoint ()
{
	m_stc->MarkerDeleteAll(BREAKPOINT_MARKER);

	wxString breakpoints = 
		CONFIG->GetProperty(wxT("DEBUG/BREAKPOINT"), wxT("value"));

	wxStringTokenizer tkz(breakpoints, wxT(";"));
	while ( tkz.HasMoreTokens() ) 
	{
		wxString point = tkz.GetNextToken();

		wxString _num, _address, _source, _line;

		wxStringTokenizer tkz2(point, wxT("|"));
		if ( tkz2.CountTokens() == 4 )
		{
			_num = tkz2.GetNextToken();
			_address = tkz2.GetNextToken();
			_source = tkz2.GetNextToken();
			_line = tkz2.GetNextToken();

			if ( _source == m_relativePath )
			{
				long line;
				_line.ToLong(&line, 10);
				line=line-1;
				m_stc->MarkerAdd(line, BREAKPOINT_MARKER);
			}
		}
	}
}

void CodeEditor::UpdateTitle()
{
	wxString title;
	
	if ( m_fullPath.Len() == 0 )
	{
		title = wxT("Noname");
	} 
	else 
	{
		int value = CONFIG->GetPropertyInt(wxT("EDITOR/GENERAL/TITLE"), wxT("value"));

		wxFileName* tmp = new wxFileName(m_fullPath);

		// file name only
		if ( value == 0 )
		{
			title = tmp->GetFullName();
		}
		// relative path
		else if ( value == 1 )
		{
			title = tmp->GetFullPath();
		}
		// absolute path
		else
		{
			tmp->MakeAbsolute(wxFileName::GetCwd());
			title = tmp->GetFullPath();
		}

		delete tmp;
	}

	SetTitle(title);
}

void CodeEditor::Open(wxString path)
{
	m_stc->SetReadOnly(false);
	Encoding *m_enc = new Encoding();
	if ( m_enc->Detect(path, true) == false )
	{
	}
	m_stc->AddText(m_enc->GetWxStr());
	delete m_enc;
	m_stc->SetReadOnly(true);
}

/*
void CodeEditor::Open(wxString path)
{
	m_stc->SetReadOnly(false);

	wxFile* file = new wxFile();

	if ( file->Open(path, wxFile::read) ) {
		unsigned long fileSize = file->Length();

		char *pBuf = (char *)malloc(fileSize+1);
		size_t nResult = file->Read( pBuf, fileSize );
		file->Close();

		delete file;

		if ( wxInvalidOffset == (int)nResult ) {
			wxMessageBox(wxString::Format(wxT("Error reading %s file."), 
						path.c_str()), wxT("MyTcl"), wxICON_ERROR);
			free(pBuf);
			m_stc->SetReadOnly(true);
			return;
		}

		pBuf[fileSize] = '\0';

		m_stc->AddText(wxString(pBuf, wxConvUTF8));
		free(pBuf);
	}
	else
	{
		delete file;
	}

	m_stc->SetReadOnly(true);
}
*/

void CodeEditor::GotoLine(int line)
{
	m_stc->SetSTCFocus(true);
	m_stc->GotoLine(line);
}

void CodeEditor::GotoLineWithDebugMarker(int line)
{
	GotoLine(line);
	m_stc->MarkerAdd(line, DEBUG_MARKER);
}

StyledTextCtrl *CodeEditor::GetSTC()
{
	return m_stc;
}
