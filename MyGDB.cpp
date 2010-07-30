// vim: set ts=4 sw=4:

#include <MyGDB.h>
#include "xpm/mygdb.xpm"
//#include "about_bitmap.h"
#include "xpm/about.xpm"

wxString exePath; 	// MyGDB program path

bool m_started = false;

class MyGdbApp : public wxApp
{
public:
    bool OnInit();
};

DECLARE_APP(MyGdbApp)
IMPLEMENT_APP(MyGdbApp)

bool MyGdbApp::OnInit()
{
	MyGdbFrame *mainFrame = new MyGdbFrame(NULL,
		 wxID_ANY, wxT("MyGDB"), 
		 wxDefaultPosition, wxSize(1000, 700),
		 wxDEFAULT_FRAME_STYLE );
    SetTopWindow(mainFrame);
    mainFrame->Show();

    return true;
}

BEGIN_EVENT_TABLE(MyGdbFrame, wxAuiMDIParentFrame)
	EVT_UPDATE_UI(wxID_ANY, MyGdbFrame::OnUpdateUI)
	
    EVT_CLOSE(MyGdbFrame::OnCloseApp)
    EVT_MENU(wxID_EXIT, MyGdbFrame::OnExit)
	EVT_SIZE(MyGdbFrame::OnSize)

	// -----------
    EVT_MENU(ID_INVOKE_APP, MyGdbFrame::OnInvokeApp)
	
	// -----------
    EVT_MENU(ID_OPEN_FILE, MyGdbFrame::OnOpenFile)

	// -----------
    EVT_MENU(ID_VIEW_CONSOLE, MyGdbFrame::OnViewConsole)
    EVT_MENU(ID_VIEW_WATCH, MyGdbFrame::OnViewWatch)
    EVT_MENU(ID_VIEW_CALLSTACK, MyGdbFrame::OnViewCallStack)
    EVT_MENU(ID_VIEW_BREAKPOINT, MyGdbFrame::OnViewBreakpoint)
    EVT_MENU(ID_VIEW_REGISTER, MyGdbFrame::OnViewRegister)
    EVT_MENU(ID_VIEW_MEMORY, MyGdbFrame::OnViewMemory)

	// -----------
    EVT_MENU(ID_START, MyGdbFrame::OnStart)
    EVT_MENU(ID_ABORT, MyGdbFrame::OnAbort)
    EVT_MENU(ID_STOP, MyGdbFrame::OnStop)
    
	// run
	EVT_MENU(ID_RUN, MyGdbFrame::OnRun)
	// next
	EVT_MENU(ID_STEP_OVER, MyGdbFrame::OnStepOver)
	// si
	EVT_MENU(ID_STEP_INTO, MyGdbFrame::OnStepInto)
	// finish
	EVT_MENU(ID_STEP_OUT, MyGdbFrame::OnStepOut)
	// continue
	EVT_MENU(ID_GO, MyGdbFrame::OnGo)
	// continue
	EVT_MENU(ID_RUN_TO_CURSOR, MyGdbFrame::OnRunToCursor)
    
	// -----------
	EVT_MENU(wxID_ABOUT, MyGdbFrame::OnAbout)
END_EVENT_TABLE()

MyGdbFrame::MyGdbFrame(wxWindow* parent,
	 wxWindowID id,
	 const wxString& title,
	 const wxPoint& pos,
	 const wxSize& size,
	 long style)
	: wxAuiMDIParentFrame(parent, id, title, pos, size, style)
{
    m_mgr.SetManagedWindow(this);

	wxInitAllImageHandlers();

#ifdef __MINGW32__
	char _drv[MAX_PATH], _path[MAX_PATH];
	char fname[MAX_PATH], ext[MAX_PATH];
	WCHAR path[MAX_PATH];

	GetModuleFileName(NULL, path, 1024);
	char temp[MAX_PATH];
	sprintf(temp, "%ls\n", path);	// wide char to char
	_splitpath(temp, _drv, _path, fname, ext);
	wxFileName exe( wxString(_drv, wxConvUTF8) + 
			wxString(_path, wxConvUTF8));
	exePath = exe.GetPath();
#else
	char _procname[FILENAME_MAX];
	int len = readlink("/proc/self/exe", _procname, FILENAME_MAX - 1);
	_procname[len] = '\0';
	wxString procname = wxString(_procname, wxConvUTF8);
	wxFileName exe(procname);
	exePath = exe.GetPath();
#endif

	exePath.Replace(wxT("\\"), wxT("/"), true);
    
	// ----------------------
	m_menuBar = new wxMenuBar();
    SetMenuBar(m_menuBar);
	
	wxMenu* menu = new wxMenu;
	wxMenuItem *item;
	
	item = new wxMenuItem(menu, ID_INVOKE_APP, 
		wxString("&Invoke Application\tCtrl+I", wxConvUTF8));
	item->SetBitmap(GET_BITMAP("application"));
	menu->Append(item);
	menu->AppendSeparator();
	item = new wxMenuItem(menu, wxID_EXIT, wxT("E&xit\tCtrl+X"));
	item->SetBitmap(GET_BITMAP("exit"));
	menu->Append(item);
	m_menuBar->Append(menu, wxT("&Invoke"));
	
	// ------------------------
	menu = new wxMenu;
	item = new wxMenuItem(menu, ID_OPEN_FILE, wxT("&Open Source\tCtrl+O"));
	item->SetBitmap(GET_BITMAP("file_open"));
	menu->Append(item);
	m_menuBar->Append(menu, wxT("&File"));
	
	// ----------------------
	menu = new wxMenu;
	
	item = new wxMenuItem(menu, ID_START, 
			wxT("Start\tF5"));
	item->SetBitmap(GET_BITMAP("start"));
	menu->Append(item);
	item = new wxMenuItem(menu, ID_ABORT, 
			wxT("Abort\tShift+F5"));
	item->SetBitmap(GET_BITMAP("abort"));
	menu->Append(item);
	// ------------
	menu->AppendSeparator();
	item = new wxMenuItem(menu, ID_RUN, 
			wxT("Run\tF6"));
	item->SetBitmap(GET_BITMAP("run"));
	menu->Append(item);
	menu->AppendSeparator();
	// ------------
	// Executes the next line of code but does not 
	// step into any function calls
	item = new wxMenuItem(menu, ID_STEP_OVER, 
			wxT("Step Over\tF10"));
	item->SetBitmap(GET_BITMAP("step_over"));
	menu->Append(item);
	// Executes code one statement at a time, 
	// tracing execution into function calls
	item = new wxMenuItem(menu, ID_STEP_INTO, 
			wxT("Step Into\tF11"));
	item->SetBitmap(GET_BITMAP("step_into"));
	menu->Append(item);
	// Executes the remaining lines of a function in 
	// which the current execution point lines
	item = new wxMenuItem(menu, ID_STEP_OUT, 
			wxT("Step Out\tShift+F11"));
	item->SetBitmap(GET_BITMAP("step_out"));
	menu->Append(item);
	item = new wxMenuItem(menu, ID_RUN_TO_CURSOR, 
			wxT("Run To Cursor\tF7"));
	item->SetBitmap(GET_BITMAP("run_to_cursor"));
	menu->Append(item);
	item = new wxMenuItem(menu, ID_GO, 
			wxT("Continue\tF8"));
	item->SetBitmap(GET_BITMAP("go"));
	menu->Append(item);
	// ------------
	menu->AppendSeparator();
	// ------------
	item = new wxMenuItem(menu, ID_STOP, 
			wxT("Stop\tCtrl+F9"));
	item->SetBitmap(GET_BITMAP("stop"));
	menu->Append(item);
	m_menuBar->Append(menu, wxT("&Debug"));

	// ------------------------
	menu = new wxMenu;
	SetWindowMenu(menu);
	
	item = new wxMenuItem(menu, ID_VIEW_CONSOLE, wxT("Console Window"));
	menu->Append(item);
	menu->AppendSeparator();
	item = new wxMenuItem(menu, ID_VIEW_WATCH, wxT("Watch Window"));
	menu->Append(item);
	item = new wxMenuItem(menu, ID_VIEW_CALLSTACK, wxT("Backtrace Window"));
	menu->Append(item);
	item = new wxMenuItem(menu, ID_VIEW_BREAKPOINT, wxT("Breakpoint Window"));
	menu->Append(item);
	item = new wxMenuItem(menu, ID_VIEW_REGISTER, wxT("Register Window"));
	menu->Append(item);
	item = new wxMenuItem(menu, ID_VIEW_MEMORY, wxT("Memory Window"));
	menu->Append(item);
	
	// ------------------------
	menu = new wxMenu;
	item = new wxMenuItem(menu, wxID_ABOUT, wxT("About\tF12"));
	menu->Append(item);
	m_menuBar->Append(menu, wxT("&Help"));
	
	// ----------------------	
	m_config = new Config();

#ifdef __MINGW32__
	SetFont(GetDockArt()->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	
	unsigned int flags = m_mgr.GetFlags();
	flags &= ~wxAUI_MGR_TRANSPARENT_HINT;
	flags &= ~wxAUI_MGR_RECTANGLE_HINT;
	flags &= ~wxAUI_MGR_ALLOW_ACTIVE_PANE;
	m_mgr.SetFlags(flags);
	m_mgr.SetFlags(m_mgr.GetFlags() ^ wxAUI_MGR_RECTANGLE_HINT);
	m_mgr.SetFlags(m_mgr.GetFlags() ^ wxAUI_MGR_ALLOW_ACTIVE_PANE);
	m_mgr.Update();

	// define dock frame style
	m_mgr.SetArtProvider(new MyAuiDockArt());

    //set frame icon
    SetIcon(wxIcon(mygdb_xpm));

	// status bar
	// ---------------------
	m_statusBar = new wxStatusBar(this, -1);
	m_statusBar->SetFieldsCount(3);
	int width[3] = { -1, 150, 150 };
	m_statusBar->SetStatusWidths(3, width);
	
	SetStatusBar(m_statusBar);

	// ----------------------- toolbar
	// -- file
    m_fileToolBar = new wxAuiToolBar(this, 
		wxID_ANY, wxDefaultPosition, wxDefaultSize, 
		wxAUI_TB_DEFAULT_STYLE );
    m_fileToolBar->SetToolBitmapSize(wxSize(16,16));

    m_fileToolBar->AddTool(ID_INVOKE_APP, 
			wxEmptyString, GET_BITMAP(wxT("application")), wxT("Invoke Application"));
    m_fileToolBar->AddTool(ID_OPEN_FILE, 
			wxEmptyString, GET_BITMAP(wxT("file_open")), wxT("Open Source"));
    
	m_fileToolBar->Realize();

	// -- debug
    m_debugToolBar = new wxAuiToolBar(this, 
		wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE );
    m_debugToolBar->SetToolBitmapSize(wxSize(16,16));

    m_debugToolBar->AddTool(ID_START, 
			wxEmptyString, GET_BITMAP(wxT("start")), 
			wxT("Debug (F5)"));
    m_debugToolBar->AddSeparator();
    m_debugToolBar->AddTool(ID_ABORT, 
			wxEmptyString, GET_BITMAP(wxT("abort")), 
			wxT("Abort (Shift+F5)"));

    m_debugToolBar->AddSeparator();
	
	m_debugToolBar->AddTool(ID_RUN, wxEmptyString, 
			GET_BITMAP(wxT("run")), wxT("Run (F6)"));

    m_debugToolBar->AddSeparator();

	m_debugToolBar->AddTool(ID_STEP_OVER, wxEmptyString, 
			GET_BITMAP(wxT("step_over")), wxT("Step Over (F10)"));
    m_debugToolBar->AddTool(ID_STEP_INTO, wxEmptyString, 
			GET_BITMAP(wxT("step_into")), wxT("Step Into (F11)"));
	m_debugToolBar->AddTool(ID_STEP_OUT, wxEmptyString, 
			GET_BITMAP(wxT("step_out")), wxT("Step Out (Shift+F11)"));
	m_debugToolBar->AddTool(ID_RUN_TO_CURSOR, wxEmptyString, 
			GET_BITMAP(wxT("run_to_cursor")), wxT("Run To Cursor (F7)"));
    m_debugToolBar->AddTool(ID_GO, wxEmptyString, 
			GET_BITMAP(wxT("go")), wxT("Continue (F8)"));
    
	m_debugToolBar->AddSeparator();
	
	m_debugToolBar->AddTool(ID_STOP, wxEmptyString, 
			GET_BITMAP(wxT("stop")), wxT("Stop (Ctrl+F9)"));

    m_debugToolBar->Realize();

	// -------------
    m_mgr.AddPane(m_fileToolBar, wxAuiPaneInfo().
		  Name(wxT("file toolbar")).
		  Caption(wxT("File")).
		  ToolbarPane().Top().Row(0).Position(0));
	
    m_mgr.AddPane(m_debugToolBar, wxAuiPaneInfo().
		  Name(wxT("debug toolbar")).
		  Caption(wxT("File")).
		  ToolbarPane().Top().Row(0).Position(1));

	// ----------- wxStyledTextCtrl style
	m_stcStyle = new STCStyle();
	m_stcStyle->SetKeyWords(wxT("auto const double float int short struct unsigned break continue else for long signed switch void case default enum goto register sizeof typedef volatile char do extern if return static union while asm dynamic_cast namespace reinterpret_cast try bool explicit new static_cast typeid catch false operator template typename class friend private this using const_cast inline public throw virtual delete mutable protected true wchar_t and bitand compl not_eq or_eq xor_eq and_eq bitor not or xor cin endl INT_MIN iomanip main npos std cout include INT_MAX iostream MAX_RAND NULL string"));
			
	wxString stylePath = exePath + wxT("/style");
	m_stcStyle->Scan(stylePath);
	
	// ----------- console
	m_console = new Console(this, this, wxID_ANY,
			wxDefaultPosition, wxSize(600,150));

    m_mgr.AddPane(m_console, wxAuiPaneInfo().
		Name(wxT("console")).
		Caption(wxT("Console")).
		Bottom().CloseButton(true).MaximizeButton(false));
    
	// ----------- watch variable
    m_watch = new Watch(
		this, this, wxID_ANY|wxNO_BORDER,
		wxDefaultPosition, wxSize(200,200));
    m_watch->SetFont(GetDockArt()->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	
    m_mgr.AddPane(m_watch, wxAuiPaneInfo().
		Name(wxT("watch")).
		Caption(wxT("Watch")).
		Left().Layer(1).CloseButton(true).MaximizeButton(false));
	//m_mgr.GetPane(wxT("watch")).Show(false);
	
	// ----------- register
    m_register = new Register(
		this, this, wxID_ANY|wxNO_BORDER,
		wxDefaultPosition, wxSize(200,200));
    m_register->SetFont(GetDockArt()->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	
    m_mgr.AddPane(m_register, wxAuiPaneInfo().
		Name(wxT("register")).
		Caption(wxT("Register")).
		Left().Layer(1).CloseButton(true).MaximizeButton(false));
	
	// ----------- memory
    m_memory = new Memory(
		this, this, wxID_ANY|wxNO_BORDER,
		wxDefaultPosition, wxSize(200,200));
    m_memory->SetFont(GetDockArt()->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	
    m_mgr.AddPane(m_memory, wxAuiPaneInfo().
		Name(wxT("memory")).
		Caption(wxT("Memory")).
		Left().Layer(1).CloseButton(true).MaximizeButton(false));
	m_mgr.GetPane(wxT("memory")).Show(false);
	
	// ----------- call stack
    m_callStack = new CallStack(
		this, this, wxID_ANY|wxNO_BORDER,
		wxDefaultPosition, wxSize(200,120));
    m_callStack->SetFont(GetDockArt()->GetFont(wxAUI_DOCKART_CAPTION_FONT));
	
    m_mgr.AddPane(m_callStack, wxAuiPaneInfo().
		Name(wxT("callstack")).
		Caption(wxT("Backtrace")).
		Top().Layer(0).CloseButton(true).MaximizeButton(false));
	
	// ----------- breakpoint
    m_breakpoint = new Breakpoint(
		this, this, wxID_ANY|wxNO_BORDER,
		wxDefaultPosition, wxSize(200,120));
    m_breakpoint->SetFont(GetDockArt()->GetFont(wxAUI_DOCKART_CAPTION_FONT));
    
	m_mgr.AddPane(m_breakpoint, wxAuiPaneInfo().
		Name(wxT("breakpoint")).
		Caption(wxT("Breakpoint")).
		Top().Layer(0).CloseButton(true).MaximizeButton(false));
	
	// --------------------------
	m_debugToolBar->EnableTool(ID_START, true);
	m_debugToolBar->EnableTool(ID_ABORT, false);

	m_debugToolBar->EnableTool(ID_RUN, false);
	m_debugToolBar->EnableTool(ID_GO, false);
	m_debugToolBar->EnableTool(ID_STEP_INTO, false);
	m_debugToolBar->EnableTool(ID_STEP_OUT, false);
	m_debugToolBar->EnableTool(ID_STEP_OVER, false);
	m_debugToolBar->EnableTool(ID_RUN_TO_CURSOR, false);
	
	m_debugToolBar->EnableTool(ID_STOP, false);
	
	// ----------
	m_menuBar->Enable(ID_START, true);
	m_menuBar->Enable(ID_ABORT, false);

	m_menuBar->Enable(ID_RUN, false);
	m_menuBar->Enable(ID_GO, false);
	m_menuBar->Enable(ID_STEP_INTO, false);
	m_menuBar->Enable(ID_STEP_OUT, false);
	m_menuBar->Enable(ID_STEP_OVER, false);
	m_menuBar->Enable(ID_RUN_TO_CURSOR, false);
	
	m_menuBar->Enable(ID_STOP, false);
	
	// ------
#ifdef __MINGW32__
	wxAuiMDIClientWindow* nb = GetClientWindow();
	nb->SetFont(GetDockArt()->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	
	wxString m_perspectives = m_config->GetProperty(wxT("GEOMETRY"), wxT("value"));
	if ( m_perspectives.Length() > 0 )
	{
		m_mgr.LoadPerspective(m_perspectives);
	}
	
	m_mgr.Update();
}

MyGdbFrame::~MyGdbFrame()
{
	m_config->SetProperty(wxT("GEOMETRY"), wxT("value"), m_mgr.SavePerspective());
	m_config->Save();

    m_mgr.UnInit();
}

void MyGdbFrame::OnSize(wxSizeEvent& event)
{
}

void MyGdbFrame::OnUpdateUI(wxUpdateUIEvent& WXUNUSED(event))
{
}

wxAuiDockArt* MyGdbFrame::GetDockArt()
{
    return m_mgr.GetArtProvider();
}

void MyGdbFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
	m_config->SetProperty(wxT("GEOMETRY"), wxT("value"), m_mgr.SavePerspective());
	m_config->Save();

	m_mgr.UnInit();
	exit(0);
}

void MyGdbFrame::OnCloseApp(wxCloseEvent& WXUNUSED(event))
{
	m_config->SetProperty(wxT("GEOMETRY"), wxT("value"), m_mgr.SavePerspective());
	m_config->Save();

	m_mgr.UnInit();
	exit(0);
}

void MyGdbFrame::OnStart(wxCommandEvent& WXUNUSED(event))
{
	wxString value = 
		m_config->GetProperty(wxT("INVOKE/GDBLOCATION"), wxT("value"));
	if ( value.Length() <= 0 )
	{
		wxMessageDialog(this, 
			wxT("No 'GDB Location' specified.\nInvoke -> Invoke Application"), 
			wxT("MyGDB"), wxOK|wxICON_WARNING).ShowModal();
		return;
	}

	value = m_config->GetProperty(wxT("INVOKE/EXECUTABLELOCATION"), wxT("value"));
	if ( value.Length() <= 0 )
	{
		wxMessageDialog(this, 
			wxT("No 'Executable File' specified.\nInvoke -> Invoke Application"), 
			wxT("MyGDB"), wxOK|wxICON_WARNING).ShowModal();
		return;
	}
	
	value = m_config->GetProperty(wxT("INVOKE/SOURCELOCATION"), wxT("value"));
	if ( value.Length() <= 0 )
	{
		wxMessageDialog(this, 
			wxT("No 'Source Location' specified.\nInvoke -> Invoke Application"), 
			wxT("MyGDB"), wxOK|wxICON_WARNING).ShowModal();
		return;
	}

	m_debugger = new Debugger(this);

	if ( m_debugger->Start() == false )
		m_debugger->Abort();
}

void MyGdbFrame::OnAbort(wxCommandEvent& WXUNUSED(event))
{
	if ( m_started )
		m_debugger->Abort();
}

void MyGdbFrame::OnStop(wxCommandEvent& WXUNUSED(event))
{
	if ( m_started )
		m_debugger->Stop();
}

void MyGdbFrame::OnRun(wxCommandEvent& WXUNUSED(event))
{
	if ( m_started )
	{
		m_console->Puts(wxT("\n"));
		m_debugger->Write(wxT("run"));
	}
}

void MyGdbFrame::OnStepOver(wxCommandEvent& WXUNUSED(event))
{
	if ( m_started )
	{
		m_console->Puts(wxT("\n"));
		m_debugger->Write(wxT("next"));
	}
}

void MyGdbFrame::OnStepInto(wxCommandEvent& WXUNUSED(event))
{
	if ( m_started )
	{
		m_console->Puts(wxT("\n"));
		m_debugger->Write(wxT("si"));
	}
}

void MyGdbFrame::OnStepOut(wxCommandEvent& WXUNUSED(event))
{
	if ( m_started )
	{
		m_console->Puts(wxT("\n"));
		m_debugger->Write(wxT("finish"));
	}
}

void MyGdbFrame::OnGo(wxCommandEvent& WXUNUSED(event))
{
	if ( m_started )
	{
		m_console->Puts(wxT("\n"));
		m_debugger->Write(wxT("continue"));
	}
}

void MyGdbFrame::AddBreakpoint(wxString path, int addLine)
{
	if ( m_started )
	{
		m_console->Puts(wxT("\n"));
		m_debugger->Write(wxString::Format(wxT("break \"%s\":%d"), 
					path.c_str(), addLine));
	}
}

void MyGdbFrame::RemoveBreakpoint(wxString path, int removeLine)
{
	if ( m_started )
	{
		wxString breakpoints = 
			m_config->GetProperty(wxT("DEBUG/BREAKPOINT"), wxT("value"));

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
   	
				long line;
				_line.ToLong(&line, 10);
				long num;
				_num.ToLong(&num, 10);

				if ( path == _source && removeLine == line)
				{
					wxString cmd = wxString::Format(wxT("delete %d"), num);
					m_debugger->Eval(cmd);
				}
			}
		}

		m_breakpoint->Build();
	}
}


void MyGdbFrame::OnRunToCursor(wxCommandEvent& WXUNUSED(event))
{
	if ( m_started )
	{
		CodeEditor *editor = (CodeEditor *)GetActiveChild();
		if ( editor ) 
		{
			wxString path = editor->GetFullPath();
			int line = editor->GetSTC()->GetCurrentLine();

			m_console->Puts(wxT("\n"));
			m_debugger->Write( wxString::Format(wxT("advance %s:%d"), 
				path.c_str(), line+1));
		}
	}
}

CodeEditor *MyGdbFrame::OpenSource(wxString path)
{
	path.Replace(wxT("\\"), wxT("/"), true);

	std::vector<CodeEditor*> editors;
	GetAllEditors(editors);
	
	CodeEditor *editor;

	bool exist = false;
	for (size_t i = 0; i < editors.size(); i++) 
	{
		wxString _path = editors[i]->GetFullPath();
		if ( _path == path )
		{
			editor = editors[i];
			exist=true;
			break;
		}
	}
	
	if ( exist == false )
	{
		editor = new CodeEditor(this, this, wxID_ANY, path);
		editor->Open(path);
		editor->Layout();
		editor->ShowBreakpoint();
	}

	editor->Activate();
	editor->SetFocus();


	return editor;
}

void MyGdbFrame::OnOpenFile(wxCommandEvent& WXUNUSED(event))
{
	wxString path = m_config->GetProperty(
			wxT("INVOKE/SOURCELOCATION"), wxT("value"));

    wxFileDialog dlg(this,
		wxT("Open Source"), path,
		wxEmptyString,
		wxT("C/C++ Files (*.c;*.cpp)|*.c;*.cpp|Header Files (*.h)|*.h|All Files (*)|*")
	);
    dlg.CentreOnParent();

    if (dlg.ShowModal() == wxID_OK)
	{
		OpenSource(dlg.GetPath());
	}
}

void MyGdbFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	About *dlg = new About(this, this);
	//dlg->SetAppName(wxTheApp->GetAppName());
	dlg->SetAppName(wxT("MyGDB"));
	dlg->SetVersion(wxT("dev-20100730"));

	wxString copy = wxT("MyGDB is distributed under the GNU GPL-v3\n\n");
	copy = copy + wxT("inhak.min@gmail.com\n");
	copy = copy + wxT("http://kldp.net/projects/mygdb");
	dlg->SetCopyright(copy);

	dlg->SetCustomBuildInfo(wxString::Format(wxT("%s. %s"),
		About::GetBuildInfo(About::wxBUILDINFO_LONG).GetData(),
		wxT("\nPowered by GDB and wxWidgets")));
	dlg->SetHeaderBitmap(wxBitmap(about_xpm));
	//dlg->SetHeaderBitmap(wxGetBitmapFromMemory(
	//			about_bitmap, sizeof(about_bitmap)));
	dlg->ApplyInfo();
	dlg->ShowModal();
	dlg->Destroy();
}

void MyGdbFrame::OnInvokeApp(wxCommandEvent& WXUNUSED(event))
{
    InvokeApp dlg(this, this, wxID_ANY, wxT("Invoke Application"));
	wxSize size = dlg.GetBestSize();
	dlg.SetSize(wxSize(400, size.GetHeight()));
    dlg.CentreOnParent();

    if ( dlg.ShowModal() == wxID_OK ) 
	{
		wxString executableFile = dlg.GetExecutableFile();
		wxString sourceLocation = dlg.GetSourceLocation();
		wxString gdbLocation = dlg.GetGDBLocation();
		wxString args = dlg.GetArguments();

		m_config->SetProperty(wxT("INVOKE/EXECUTABLELOCATION"), 
				wxT("value"), executableFile);
		m_config->SetProperty(wxT("INVOKE/SOURCELOCATION"), 
				wxT("value"), sourceLocation);
		m_config->SetProperty(wxT("INVOKE/GDBLOCATION"), 
				wxT("value"), gdbLocation);
		m_config->SetProperty(wxT("INVOKE/ARGS"), 
				wxT("value"), args);
		m_config->Save();
	}
}

void MyGdbFrame::GetAllEditors(std::vector<CodeEditor*> &editors)
{
	editors.clear();
	wxAuiNotebook* nb = GetNotebook();
	for (size_t i = 0; i < nb->GetPageCount(); i++) {
		CodeEditor *editor = dynamic_cast<CodeEditor*>(nb->GetPage(i));
		if (editor) {
			editors.push_back(editor);
		}
	}
}

void MyGdbFrame::OnViewConsole(wxCommandEvent& WXUNUSED(event))
{
	m_mgr.GetPane(wxT("console")).Show(true);
	m_mgr.Update();
}

void MyGdbFrame::OnViewWatch(wxCommandEvent& WXUNUSED(event))
{
	m_mgr.GetPane(wxT("watch")).Show(true);
	m_mgr.Update();
}

void MyGdbFrame::OnViewCallStack(wxCommandEvent& WXUNUSED(event))
{
	m_mgr.GetPane(wxT("callstack")).Show(true);
	m_mgr.Update();
}

void MyGdbFrame::OnViewBreakpoint(wxCommandEvent& WXUNUSED(event))
{
	m_mgr.GetPane(wxT("breakpoint")).Show(true);
	m_mgr.Update();
}

void MyGdbFrame::OnViewRegister(wxCommandEvent& WXUNUSED(event))
{
	m_mgr.GetPane(wxT("register")).Show(true);
	m_mgr.Update();
}

void MyGdbFrame::OnViewMemory(wxCommandEvent& WXUNUSED(event))
{
	m_mgr.GetPane(wxT("memory")).Show(true);
	m_mgr.Update();
}
