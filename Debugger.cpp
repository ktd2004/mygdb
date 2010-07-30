// vim: set fdm=marker ts=4 sw=4:

#pragma warning(disable: 4819)

#include <MyGDB.h>

BEGIN_EVENT_TABLE(Debugger, wxProcess)
    EVT_END_PROCESS(wxID_ANY, Debugger::OnEndProcess)
    EVT_TIMER(wxID_ANY, Debugger::OnTimer)
END_EVENT_TABLE()

Debugger::Debugger(MyGdbFrame *myFrame)
	: wxProcess(myFrame)
{
	this->myFrame = myFrame;
}

Debugger::~Debugger () 
{
}

wxString Debugger::CreateInitFile ()
{
	wxFile* file = new wxFile();
	wxString path = wxFileName::CreateTempFileName(wxT("mygdb"));
	if ( file->Open(path, wxFile::write) ) 
	{
		wxString args = 
			CONFIG->GetProperty(wxT("INVOKE/ARGS"), wxT("value"));

		if ( args.Length() > 0 )
			file->Write(wxT("set args ") + args + wxT("\n"));

		file->Write(wxT("set annotate 2\n"));

		wxString breakpoints = 
			CONFIG->GetProperty(wxT("DEBUG/BREAKPOINT"), wxT("value"));

		wxStringTokenizer tkz(breakpoints, wxT(";"));
		while ( tkz.HasMoreTokens() ) 
		{
			wxString point = tkz.GetNextToken();

			wxString num, address, source, line;

			wxStringTokenizer tkz2(point, wxT("|"));

			if ( tkz2.CountTokens() == 4 )
			{
				num = tkz2.GetNextToken();
				address = tkz2.GetNextToken();
				source = tkz2.GetNextToken();
				line = tkz2.GetNextToken();

				file->Write(wxString::Format(wxT("break %s:%s\n"), 
						source.c_str(), line.c_str()));	
			}
			else
			{
				num = tkz2.GetNextToken();
				address = tkz2.GetNextToken();

				file->Write(wxString::Format(wxT("break *%s\n"), 
						address.c_str()));
			}
		}
	}
	else
	{
		file->Close();
		delete file;

		return wxEmptyString;
	}

	file->Close();
	delete file;

	return path;
}

bool Debugger::Start()
{
	wxString gdb = 
		CONFIG->GetProperty(wxT("INVOKE/GDBLOCATION"), wxT("value"));
	wxString executableFile = 
		CONFIG->GetProperty(wxT("INVOKE/EXECUTABLELOCATION"), wxT("value"));
	wxString sourceLocation = 
		CONFIG->GetProperty(wxT("INVOKE/SOURCELOCATION"), wxT("value"));

	if ( wxFileName::FileExists(executableFile) == false )
	{
		wxMessageDialog(NULL, wxT("Can not find 'Executable File'"), 
				wxT("MyGDB"), wxOK|wxICON_WARNING).ShowModal();
		return false;
	}
	
	if ( wxFileName::FileExists(gdb) == false )
	{
		wxMessageDialog(NULL, wxT("Can not find 'GDB Location'"), 
				wxT("MyGDB"), wxOK|wxICON_WARNING).ShowModal();
		return false;
	}
	
	if ( wxFileName::DirExists(sourceLocation) == false )
	{
		wxMessageDialog(NULL, wxT("Can not find 'Source Location'"), 
				wxT("MyGDB"), wxOK|wxICON_WARNING).ShowModal();
		return false;
	}
	
	wxString cmdPath = CreateInitFile ();

	wxString cmd = gdb + wxT(" ");
	cmd = cmd + wxT("--command=") + cmdPath + wxT(" ");
	// for the test
	// cmd = cmd + wxT("--args aaaaaaaaaa ");
	cmd = cmd + executableFile;

	m_process = new wxProcess(this);
	m_process->Redirect();

	m_pid = wxExecute(cmd, wxEXEC_ASYNC, m_process);

	CONSOLE->SetReadOnly(false);
    CONSOLE->EmptyUndoBuffer();
	CONSOLE->Initial();

	m_started = true;
	
	m_annotating=false;
	m_prompt=false;
	m_processing = false;
	
	// ------
	FILE_TOOLBAR->EnableTool(ID_INVOKE_APP, false);
	FILE_TOOLBAR->Refresh(false);
	DEBUG_TOOLBAR->EnableTool(ID_START, false);
	DEBUG_TOOLBAR->EnableTool(ID_ABORT, true);
	DEBUG_TOOLBAR->EnableTool(ID_STOP, true);
	DEBUG_TOOLBAR->Refresh(false);
	
	// ------
	MENU_BAR->Enable(ID_INVOKE_APP, false);
	MENU_BAR->Enable(ID_START, false);
	MENU_BAR->Enable(ID_ABORT, true);
	MENU_BAR->Enable(ID_STOP, true);

	m_timer = new wxTimer(this, wxID_ANY);
	m_timer->Start(100);
	
	CONSOLE->ConnectIdle();

	return true;
}

void Debugger::Puts(wxString msg, int type)
{
	CONSOLE->Freeze();

	CONSOLE->CmdKeyExecute(wxSTC_CMD_DOCUMENTEND);

	for(unsigned int i=0; i<msg.Length(); i++)
	{
		// CRLF (Windows) - MinGW GDB
		if ( msg[i] == '\r' && msg[i+1] == '\n' && 
				msg[i+2] == 26 && msg[i+3] == 26 )
		{
			m_annotating = true; 
			i+=4;
		}
		// LF (Linux) - Cygwin, Linux GDB
		else if ( msg[i] == '\n' && 
				msg[i+1] == 26 && msg[i+2] == 26 )
		{
			m_annotating = true; 
			i+=3;
		}

		if ( m_annotating )
		{
			wxString annotate=wxEmptyString;

			while(1)
			{
				annotate = annotate + msg[i];
				if ( msg[i+1] == '\r' && msg[i+2] == '\n' ) 
				{
					i+=2;
					break;
				}

				if ( msg[i+1] == '\n' ) {
					i+=1;
					break;
				}
				i++;
			}

			//wxMessageBox(wxT("[") + annotate + "]");

			if ( annotate == wxT("pre-prompt") || 
				annotate == wxT("pre-commands") ||
				annotate == wxT("pre-overload-choice") || 
				annotate == wxT("pre-query") || 
				annotate == wxT("pre-prompt-for-continue") )
			{
				m_prompt = true;
			}

			if ( annotate == wxT("prompt") || 
				annotate == wxT("commands") ||
				annotate == wxT("overload-choice") || 
				annotate == wxT("query") || 
				annotate == wxT("prompt-for-continue") )
			{
				CONSOLE->SetStartPos(CONSOLE->GetLength());
				m_prompt = false;
			}

			m_annotating = false;
		}
		else
		{
			if ( m_prompt )
			{
				CONSOLE->Puts(msg[i], MYGDB_PROMPT);
			}
			else
			{
				CONSOLE->Puts(msg[i], type);
			}
		}
	}

	CONSOLE->LineEndDisplay();
	CONSOLE->Update();
	CONSOLE->Thaw();
}

bool Debugger::IsProcessing()
{
	return m_processing;
}

bool Debugger::Write (wxString cmd)
{
	if ( m_started == false ) 
		return false;

	if ( m_processing == true ) 
		return false;

	DEBUG_TOOLBAR->EnableTool(ID_RUN, false);
	DEBUG_TOOLBAR->EnableTool(ID_GO, false);
	DEBUG_TOOLBAR->EnableTool(ID_STEP_INTO, false);
	DEBUG_TOOLBAR->EnableTool(ID_STEP_OUT, false);
	DEBUG_TOOLBAR->EnableTool(ID_STEP_OVER, false);
	DEBUG_TOOLBAR->EnableTool(ID_RUN_TO_CURSOR, false);
	DEBUG_TOOLBAR->EnableTool(ID_TOGGLE_BREAKPOINT, false);
	DEBUG_TOOLBAR->Refresh(false);
	
	MENU_BAR->Enable(ID_RUN, false);
	MENU_BAR->Enable(ID_GO, false);
	MENU_BAR->Enable(ID_STEP_INTO, false);
	MENU_BAR->Enable(ID_STEP_OUT, false);
	MENU_BAR->Enable(ID_STEP_OVER, false);
	MENU_BAR->Enable(ID_RUN_TO_CURSOR, false);
	MENU_BAR->Enable(ID_TOGGLE_BREAKPOINT, false);
	
	m_processing = true;

	wxOutputStream *in = m_process->GetOutputStream();

	wxString s = cmd + _T("\n");
	in->Write((const char *)wxStringToChar(s), s.Length());
	
	m_timer->Start(100);
	CONSOLE->ConnectIdle();
	
	return true;
}

bool Debugger::FindPrompt(wxString msg)
{
	wxString prompt1 = wxString::Format(wxT("%c%cprompt"), 26, 26);
	wxString prompt2 = wxString::Format(wxT("%c%ccommands"), 26, 26);
	wxString prompt3 = wxString::Format(wxT("%c%coverload-choice"), 26, 26);
	wxString prompt4 = wxString::Format(wxT("%c%cquery"), 26, 26);
	wxString prompt5 = wxString::Format(wxT("%c%cprompt-for-continue"), 26, 26);
	if ( msg.Find(prompt1) != wxNOT_FOUND ||
		msg.Find(prompt2) != wxNOT_FOUND ||
		msg.Find(prompt3) != wxNOT_FOUND ||
		msg.Find(prompt4) != wxNOT_FOUND ||
		msg.Find(prompt5) != wxNOT_FOUND )
		return true;
	else
		return false;
}

char* Debugger::wxStringToChar(wxString input)
{
#if (wxUSE_UNICODE)
        size_t size = input.size() + 1;
		//No need to multiply by 4, converting to 1 byte char only.
        char *buffer = new char[size];
		//Good Practice, Can use buffer[0] = '&#65533;' also.
        memset(buffer, 0, size); 
        wxEncodingConverter wxec;
        wxec.Init(wxFONTENCODING_ISO8859_1, wxFONTENCODING_ISO8859_1, wxCONVERT_SUBSTITUTE);
        wxec.Convert(input.mb_str(), buffer);
		//To free this buffer memory is user responsibility.
        return buffer; 
#else
        return (char *)(input.c_str());
#endif
}

wxString Debugger::Eval(wxString cmd)
{
	wxString annotate = wxEmptyString;

	wxOutputStream *in = m_process->GetOutputStream();

	wxString s = cmd + _T("\n");
	in->Write((const char *)wxStringToChar(s), s.Length());

	while(1)
	{
		// stderr
		wxInputStream *err = m_process->GetErrorStream();
		while(err->CanRead())
		{
			char buf[9072];
			buf[err->Read(buf, sizeof(buf)-1).LastRead()] = _T('\0');
			annotate.Append(wxString(buf, wxConvUTF8)); 
		}

		if ( FindPrompt(annotate) == true ) break;

		// stdout
		wxInputStream *out = m_process->GetInputStream();
		while(out->CanRead())
		{
			char buf[9072];
			buf[out->Read(buf, sizeof(buf)-1).LastRead()] = _T('\0');
			annotate.Append(wxString(buf, wxConvUTF8)); 
		}
	}

	return annotate;
}

void Debugger::Flush()
{
	static bool working = false;

	if ( m_started == false ) return;
	if ( working == true ) return;

	working = true;

	if ( m_process )
	{
		CONSOLE->SetReadOnly(false);
		CONSOLE->EmptyUndoBuffer();

		// ---------------- stderr
		wxInputStream *err = m_process->GetErrorStream();
		wxString msg = wxEmptyString;
		while(err->CanRead())
		{
			char buf[9072];
			buf[err->Read(buf, sizeof(buf)-1).LastRead()] = _T('\0');
			msg = msg + wxString(buf, wxConvUTF8);
		}

		if ( msg.Length() > 0 )
		{
			wxMessageDialog(NULL, RemoveAnnotate(msg),
					wxT("MyGDB"), wxOK|wxICON_ERROR).ShowModal();
		}

		// ---------------- stdout
		wxString buffer = wxEmptyString;

		bool prompt=false;

		wxInputStream *out = m_process->GetInputStream();
		while(out->CanRead())
		{
			char buf[9072];
			buf[out->Read(buf, sizeof(buf)-1).LastRead()] = _T('\0');
			buffer.Append(wxString(buf, wxConvUTF8)); 
		}

		if ( buffer.Length() > 0 )
		{
			Puts(buffer, MYGDB_STDOUT);

			// ----------------
			if ( FindPrompt(buffer) == true )
			{
				prompt = true;
				m_processing = false;
			}

			if (prompt)
			{
				m_timer->Stop();
				CONSOLE->DisconnectIdle();
			}

			// ----------------------
			CONSOLE->Update();

			if ( prompt )
			{
				CONSOLE->SetReadOnly(false);
				CONSOLE->EmptyUndoBuffer();

				DEBUG_TOOLBAR->EnableTool(ID_RUN, true);
				DEBUG_TOOLBAR->EnableTool(ID_GO, true);
				DEBUG_TOOLBAR->EnableTool(ID_STEP_INTO, true);
				DEBUG_TOOLBAR->EnableTool(ID_STEP_OUT, true);
				DEBUG_TOOLBAR->EnableTool(ID_STEP_OVER, true);
				DEBUG_TOOLBAR->EnableTool(ID_RUN_TO_CURSOR, true);
				DEBUG_TOOLBAR->EnableTool(ID_TOGGLE_BREAKPOINT, true);
				DEBUG_TOOLBAR->Refresh(false);
				
				MENU_BAR->Enable(ID_RUN, true);
				MENU_BAR->Enable(ID_GO, true);
				MENU_BAR->Enable(ID_STEP_INTO, true);
				MENU_BAR->Enable(ID_STEP_OUT, true);
				MENU_BAR->Enable(ID_STEP_OVER, true);
				MENU_BAR->Enable(ID_RUN_TO_CURSOR, true);
				MENU_BAR->Enable(ID_TOGGLE_BREAKPOINT, true);

				bool changed = false;	

				wxString source = wxEmptyString;
				wxString address = wxEmptyString;
				long line = -1;

				wxString tag = wxString::Format(wxT("%c%csource"), 26, 26);
				if ( buffer.Find(tag) != wxNOT_FOUND )
				{
					if ( GetWhereBySource (buffer, &source, &line, &address) == true )
					{
						UpdatePointer(source, line);
						changed = true;
					}
				}

				tag = wxString::Format(wxT("%c%cframe-begin"), 26, 26);
				if ( changed == false && buffer.Find(tag) != wxNOT_FOUND )
				{
					if ( GetWhereByFrame (buffer, &address) == true )
					{
						changed = true;
					}
				}

				if ( changed == true )
				{
					myFrame->Freeze();

					if ( M_MGR.GetPane(wxT("watch")).IsShown() == true )
						WATCH->Build();
					if ( M_MGR.GetPane(wxT("register")).IsShown() == true )
						REGISTER->Build();

					if ( M_MGR.GetPane(wxT("callstack")).IsShown() == true )
					{
						CALLSTACK->Build();
						if ( address.Length() > 0 )
						{
							CALLSTACK->UpdatePointer(address);
						}
					}
					
					if ( M_MGR.GetPane(wxT("memory")).IsShown() == true )
						MEMORY->Build();

					myFrame->Thaw();
				}
					
				if ( M_MGR.GetPane(wxT("breakpoint")).IsShown() == true )
				{
					myFrame->Freeze();
					BREAKPOINT->Build();
					myFrame->Thaw();
				}
			}
		}
		else
		{
			CONSOLE->SetReadOnly(true);
		}

	}

	working = false;
}

void Debugger::Log(wxString msg)
{
	FILE *fp = fopen("debugger.log", "w");
	fprintf(fp, "%s", (char *)msg.c_str());
	fclose(fp);
}

wxString Debugger::Parse(wxString msg)
{
	bool annotating = false;
	bool prompt=false;

	wxString result = wxEmptyString;

	for(unsigned int i=0; i<msg.Length(); i++)
	{
		// CRLF (Windows) - MinGW GDB
		if ( msg[i] == '\r' && msg[i+1] == '\n' && 
				msg[i+2] == 26 && msg[i+3] == 26 )
		{
			annotating = true; 
			i+=4;
		}
		// LF (Linux) - Cygwin, Linux GDB
		else if ( msg[i] == '\n' && 
				msg[i+1] == 26 && msg[i+2] == 26 )
		{
			annotating = true; 
			i+=3;
		}

		if ( annotating )
		{
			wxString annotate=wxEmptyString;

			while(1)
			{
				annotate = annotate + msg[i];
				if ( msg[i+1] == '\r' && msg[i+2] == '\n' ) 
				{
					i+=2;
					break;
				}

				if ( msg[i+1] == '\n' ) {
					i+=1;
					break;
				}
				i++;
			}

			result = result + wxT(" ");
			result = result + wxString::Format(wxT("%c%c"), 26, 26);
			result = result + annotate;
			result = result + wxT(" ");

			if ( annotate == wxT("pre-prompt") || 
				annotate == wxT("prompt") ||
				annotate == wxT("pre-commands") || 
				annotate == wxT("commands") ||
				annotate == wxT("pre-overload-choice") || 
				annotate == wxT("overload-choice") ||
				annotate == wxT("pre-query") || 
				annotate == wxT("query") ||
				annotate == wxT("pre-prompt-for-continue") || 
				annotate == wxT("prompt-for-continue") )
			{
				prompt=true;
			}
			else 
			{
				prompt=false;
			}
			
			annotating = false;
		}
		else
		{
			if ( prompt == false )
				result = result + msg[i];
		}
	}

	return result;
}

wxString Debugger::RemoveAnnotate(wxString msg)
{
	bool annotating = false;
	bool add = false;

	wxString result = wxEmptyString;
	
	// annotate 메세지가 없다면 원래의 메세지를 리턴
	wxString annotate1 = wxString::Format(wxT("\r\n%c%c"), 26, 26);
	if ( msg.Find(annotate1) == wxNOT_FOUND )
	{
		wxString annotate2 = wxString::Format(wxT("\n%c%c"), 26, 26);
		if ( msg.Find(annotate2) == wxNOT_FOUND )
			return msg;
	}

	for(unsigned int i=0; i<msg.Length(); i++)
	{
		// CRLF (Windows) - MinGW GDB
		if ( msg[i] == '\r' && msg[i+1] == '\n' && 
				msg[i+2] == 26 && msg[i+3] == 26 )
		{
			annotating = true; 
			i+=4;
		}
		// LF (Linux) - Cygwin, Linux GDB
		else if ( msg[i] == '\n' && 
				msg[i+1] == 26 && msg[i+2] == 26 )
		{
			annotating = true; 
			i+=3;
		}

		if ( annotating )
		{
			wxString annotate=wxEmptyString;

			while(1)
			{
				annotate = annotate + msg[i];
				if ( msg[i+1] == '\r' && msg[i+2] == '\n' ) 
				{
					i+=2;
					break;
				}

				if ( msg[i+1] == '\n' ) {
					i+=1;
					break;
				}
				i++;
			}

			annotating = false;

			if ( annotate == wxT("pre-prompt") || annotate == wxT("prompt") ||
				annotate == wxT("pre-commands") || annotate == wxT("commands") ||
				annotate == wxT("pre-overload-choice") || 
				annotate == wxT("overload-choice") ||
				annotate == wxT("pre-query") || annotate == wxT("query") ||
				annotate == wxT("pre-prompt-for-continue") || 
				annotate == wxT("prompt-for-continue") )
			{
				add = false;
			}
			else
			{
				add = true;
			}
		}
		else
		{
			if (add) 
				result = result + msg[i];
		}
	}

	return result;
}

bool Debugger::GetWhereByFrame (wxString msg, wxString *address)
{
	// check error 
	wxString error = wxString::Format(wxT("%c%cerror"), 26, 26);
	if ( msg.Find(error) != wxNOT_FOUND ) 
		return false;
			
	// -------------
	wxString parse = Parse(msg);
	//wxMessageBox(parse);

	wxStringTokenizer tkz(parse, wxString::Format(wxT("%c%c"), 26, 26));
	while ( tkz.HasMoreTokens() ) 
	{
		wxString token = tkz.GetNextToken();

		wxString tag = wxT("frame-address ");
		if ( token.StartsWith(tag) )
		{
			wxString frameAddress;
			frameAddress = token.SubString(tag.Length(), token.Length()); 
			frameAddress.Trim(true);
			frameAddress.Trim(false);

			*address = frameAddress;
			break;
		}
	}

	return true;
}

bool Debugger::GetWhereBySource (wxString msg, 
		wxString *source, long *line, wxString *address)
{
	// check error 
	wxString error = wxString::Format(wxT("%c%cerror"), 26, 26);
	if ( msg.Find(error) != wxNOT_FOUND ) 
		return false;


	// -------------
	wxString parse = Parse(msg);
	wxStringTokenizer tkz(parse, wxString::Format(wxT("%c%c"), 26, 26));
	while ( tkz.HasMoreTokens() ) 
	{
		wxString token = tkz.GetNextToken();
		wxString tag = wxT("source ");
		if ( token.StartsWith(tag) )
		{
			wxString info = token.SubString(tag.Length(), token.Length()); 

			wxStringTokenizer tkz2(info, wxT(":"));
			wxString sourcePath;

#ifdef __MINGW32__
			wxString sourceDrive = tkz2.GetNextToken();
			wxString sourceFile = tkz2.GetNextToken();
			sourcePath = sourceDrive + wxT(":") + sourceFile;
#else
			sourcePath = tkz2.GetNextToken();
#endif
			*source = sourcePath;
			
			wxString sourceLine;
			sourceLine = tkz2.GetNextToken();
			sourceLine.ToLong(line);
			
			tkz2.GetNextToken();
			tkz2.GetNextToken();
			
			*address = tkz2.GetNextToken();

			break;
		}
	}

	return true;
}

void Debugger::UpdatePointer(wxString source, long line)
{
	//if ( sourceFile.Length() <= 0 || sourceLine.Length() <= 0 ) return;
	// ---------
	wxString sourceLocation = 
		CONFIG->GetProperty(wxT("INVOKE/SOURCELOCATION"), wxT("value"));
	if ( sourceLocation.Length() <= 0 ) return;

	wxFileName path(source);
	path.MakeAbsolute(sourceLocation);

	CodeEditor *editor = myFrame->OpenSource(path.GetFullPath());

	// -------- clear breakpoint
	editor->GetSTC()->MarkerDeleteAll(DEBUG_MARKER);
	
	// -------- clear debug pointer in all editor
	std::vector<CodeEditor*> editors;
	myFrame->GetAllEditors(editors);

	for (size_t i = 0; i < editors.size(); i++) 
		editors[i]->GetSTC()->MarkerDeleteAll(DEBUG_MARKER);

	// -------- goto line
	//long line;
	//sourceLine.ToLong(&line);
	editor->GotoLineWithDebugMarker(line-1);

	// -------- console refocus
	CONSOLE->SetFocus();
}

void Debugger::OnEndProcess(wxProcessEvent& event)
{
	Flush();

	m_started = false;

	m_timer->Stop();
	CONSOLE->DisconnectIdle();
	
	CONSOLE->SetReadOnly(false);
    CONSOLE->EmptyUndoBuffer();
	CONSOLE->CmdKeyExecute(wxSTC_CMD_DOCUMENTEND);

	// build successful
	if ( event.GetExitCode() == 0 ) 
	{
		CONSOLE->Puts(wxString::Format(
					wxT("Debugger exited with exit code %d.\n"), 
			event.GetExitCode()), MYGDB_STDOUT);
	}
	else
	{
		CONSOLE->Puts(wxString::Format(
					wxT("\nDebugger aborted with exit code %d.\n"), 
			event.GetExitCode()), MYGDB_STDERR);
	}
	
	// --------- clear marker
	std::vector<CodeEditor*> editors;
	myFrame->GetAllEditors(editors);

	for (size_t i = 0; i < editors.size(); i++) 
	{
		editors[i]->GetSTC()->MarkerDeleteAll(BREAKPOINT_MARKER);
		editors[i]->GetSTC()->MarkerDeleteAll(DEBUG_MARKER);
	}

	// ---------------
	CONSOLE->LineEndDisplay();
	CONSOLE->SetReadOnly(true);

	// ------
	FILE_TOOLBAR->EnableTool(ID_INVOKE_APP, true);
	FILE_TOOLBAR->Refresh(false);

	DEBUG_TOOLBAR->EnableTool(ID_START, true);
	DEBUG_TOOLBAR->EnableTool(ID_ABORT, false);

	DEBUG_TOOLBAR->EnableTool(ID_RUN, false);
	DEBUG_TOOLBAR->EnableTool(ID_GO, false);
	DEBUG_TOOLBAR->EnableTool(ID_STEP_INTO, false);
	DEBUG_TOOLBAR->EnableTool(ID_STEP_OUT, false);
	DEBUG_TOOLBAR->EnableTool(ID_STEP_OVER, false);
	DEBUG_TOOLBAR->EnableTool(ID_RUN_TO_CURSOR, false);

	DEBUG_TOOLBAR->EnableTool(ID_STOP, false);
	DEBUG_TOOLBAR->Refresh(false);
	
	// ------
	MENU_BAR->Enable(ID_INVOKE_APP, true);

	MENU_BAR->Enable(ID_START, true);
	MENU_BAR->Enable(ID_ABORT, false);

	MENU_BAR->Enable(ID_RUN, false);
	MENU_BAR->Enable(ID_GO, false);
	MENU_BAR->Enable(ID_STEP_INTO, false);
	MENU_BAR->Enable(ID_STEP_OUT, false);
	MENU_BAR->Enable(ID_STEP_OVER, false);
	MENU_BAR->Enable(ID_RUN_TO_CURSOR, false);

	MENU_BAR->Enable(ID_STOP, false);

	WATCH->Clear();
	REGISTER->Clear();
	CALLSTACK->Clear();
	BREAKPOINT->Clear();
	MEMORY->Clear();

	delete m_timer;
	delete m_process;

	//delete this;
}

void Debugger::OnTimer(wxTimerEvent& WXUNUSED(event))
{
	wxWakeUpIdle();
}

void Debugger::OnTerminate(int pid, int status)
{
	Flush();

	CONSOLE->SetReadOnly(false);
	CONSOLE->CmdKeyExecute(wxSTC_CMD_DOCUMENTEND);
	CONSOLE->Puts(wxString::Format(
		wxT("Debugger terminated with exit code %d.\n"), 
		status), MYGDB_STDERR);
	CONSOLE->LineEndDisplay();
    CONSOLE->EmptyUndoBuffer();
	CONSOLE->SetReadOnly(true);
}

#ifdef __MINGW32__
int kill(pid_t pid, int sig)
{
	if (pid > 0 && sig == SIGTERM) {
		HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);

		if (TerminateProcess(h, -1)) {
			CloseHandle(h);
			return 0;
		}

		CloseHandle(h);
		errno = GetLastError();
		return -1;
	}

	errno = EINVAL;
	return -1;
}
#endif

// send SIGTERM
void Debugger::Abort(void)
{
	kill(m_pid, SIGTERM);
}

// send SIGINT
void Debugger::Stop()
{
#ifdef __MINGW32__
	wxString gdb = CONFIG->GetProperty(wxT("INVOKE/GDBLOCATION"), wxT("value"));
	gdb.Replace(wxT("\\"), wxT("/"), true);

	// Get the first child window. Use it.
	HWND hWnd = ::GetWindow( GetDesktopWindow(), GW_CHILD|GW_HWNDFIRST );

	while( hWnd )
	{
		wxChar tmp[1024];
		GetWindowText(hWnd, tmp, 1024);

		wxString title = tmp;
		title.Replace(wxT("\\"), wxT("/"), true);

		if ( title == gdb )
		{
			// focus to gdb
			SetForegroundWindow(hWnd);

			keybd_event(VK_CONTROL, 0, 0, 0);
			// Send the C key (43 is 'C')
			keybd_event (0x43, 0, 0, 0 ); 
			keybd_event (0x43, 0, KEYEVENTF_KEYUP, 0);
			// 'Left Control Up
			keybd_event (VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
		}

		hWnd = ::GetWindow( hWnd, GW_HWNDNEXT );
	}

	// restore focus
	SetForegroundWindow((HWND)myFrame->GetHWND());

#else
	kill(m_pid, SIGINT);
#endif

	CONSOLE->CmdKeyExecute(wxSTC_CMD_DOCUMENTEND);
	CONSOLE->Puts(wxT("\n"));

	CONSOLE->ConnectIdle();
	m_timer->Start(100);
}
