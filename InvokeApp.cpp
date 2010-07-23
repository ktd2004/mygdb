// vim: set fdm=marker ts=4 sw=4:

#pragma warning(disable: 4819)

#include <MyGDB.h>

BEGIN_EVENT_TABLE(InvokeApp, wxDialog)
END_EVENT_TABLE()

InvokeApp::InvokeApp(wxWindow* parent,
	MyGdbFrame *myFrame,
	wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style)
	: wxDialog(parent, id, title, pos, size, style)
{
	this->myFrame = myFrame;

	wxAuiDockArt *dockArt = myFrame->GetDockArt();

	wxBoxSizer *m_mainSizer= new wxBoxSizer(wxVERTICAL);
	
	// -------- application
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *label = new wxStaticText(this, wxID_ANY, 
			wxT("Executable File: "), 
			wxPoint(-1,-1), wxSize(120,-1), wxALIGN_LEFT);
#ifdef __MINGW32__
    label->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif

	wxFont font = label->GetFont();
	font.SetWeight(wxFONTWEIGHT_BOLD);
	label->SetFont(font);

	sizer->Add( label, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
	m_executableLocation = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
			wxDefaultPosition, wxDefaultSize, wxTE_RICH2);
	sizer->Add( m_executableLocation, 1, wxALL, 0);
#ifdef __MINGW32__
    m_executableLocation->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	wxString value = CONFIG->GetProperty(wxT("INVOKE/EXECUTABLELOCATION"), wxT("value"));
	if ( value.Length() > 0 )
	{
		m_executableLocation->AppendText(value);
	}
	else
	{
		m_executableLocation->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY));
		m_executableLocation->AppendText(wxT("<Enter_executable_file>"));
	}
	wxButton *m_button = new wxButton(this, 
			ID_SELECT_EXECUTABLE, wxT("Browse..."));
	Connect(ID_SELECT_EXECUTABLE, wxEVT_COMMAND_BUTTON_CLICKED, 
			(wxObjectEventFunction) (wxEventFunction) 
			&InvokeApp::OnSelectExecutable);
#ifdef __MINGW32__
    m_button->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	sizer->Add( m_button, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 3);
	m_mainSizer->Add( sizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 6);
	
	m_executableLocation->Connect(wxEVT_SET_FOCUS, 
			(wxObjectEventFunction) (wxEventFunction) 
			&InvokeApp::OnExecutableEnter, 0, this);
	m_executableLocation->Connect(wxEVT_KILL_FOCUS, 
			(wxObjectEventFunction) (wxEventFunction) 
			&InvokeApp::OnExecutableLeave, 0, this);
	
	// --------- arguments
	sizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(this, wxID_ANY, 
			wxT("Arguments: "),
			wxPoint(-1,-1), wxSize(120,-1), wxALIGN_LEFT);
#ifdef __MINGW32__
    label->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	label->SetFont(font);

	sizer->Add( label, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
	m_args = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
			wxDefaultPosition, wxDefaultSize, wxTE_RICH2);
#ifdef __MINGW32__
    m_args->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	value = CONFIG->GetProperty(wxT("INVOKE/ARGS"), wxT("value"));
	if ( value.Length() > 0 )
	{
		m_args->AppendText(value);
	}
	else
	{
		m_args->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY));
		m_args->AppendText(wxT("<Enter_arguments - optional>"));
	}
	sizer->Add( m_args, 1, wxEXPAND|wxALL, 0);
	m_mainSizer->Add( sizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 6);
	
	m_args->Connect(wxEVT_SET_FOCUS, 
			(wxObjectEventFunction) (wxEventFunction) 
			&InvokeApp::OnArgumentsEnter, 0, this);
	m_args->Connect(wxEVT_KILL_FOCUS, 
			(wxObjectEventFunction) (wxEventFunction) 
			&InvokeApp::OnArgumentsLeave, 0, this);
	
	// -------- source
	sizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(this, wxID_ANY, 
			wxT("Source Location: "), 
			wxPoint(-1,-1), wxSize(120,-1), wxALIGN_LEFT);
#ifdef __MINGW32__
    label->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	label->SetFont(font);

	sizer->Add( label, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
	m_sourceLocation = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
			wxDefaultPosition, wxDefaultSize, wxTE_RICH2);
	sizer->Add( m_sourceLocation, 1, wxALL, 0);
#ifdef __MINGW32__
    m_sourceLocation->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	value = CONFIG->GetProperty(wxT("INVOKE/SOURCELOCATION"), wxT("value"));
	if ( value.Length() > 0 )
	{
		m_sourceLocation->AppendText(value);
	}
	else
	{
		m_sourceLocation->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY));
		m_sourceLocation->AppendText(wxT("<Enter_location>"));
	}
	m_button = new wxButton(this, 
			ID_SELECT_SOURCE_LOCATION, wxT("Browse..."));
	Connect(ID_SELECT_SOURCE_LOCATION, wxEVT_COMMAND_BUTTON_CLICKED, 
			(wxObjectEventFunction) (wxEventFunction) 
			&InvokeApp::OnSelectSourceLocation);
#ifdef __MINGW32__
    m_button->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	sizer->Add( m_button, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 3);
	m_mainSizer->Add( sizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 6);
	
	m_sourceLocation->Connect(wxEVT_SET_FOCUS, 
			(wxObjectEventFunction) (wxEventFunction) 
			&InvokeApp::OnSourceLocationEnter, 0, this);
	m_sourceLocation->Connect(wxEVT_KILL_FOCUS, 
			(wxObjectEventFunction) (wxEventFunction) 
			&InvokeApp::OnSourceLocationLeave, 0, this);
	
	// -------- debugger
	sizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(this, wxID_ANY, 
			wxT("GDB Location: "), 
			wxPoint(-1,-1), wxSize(120,-1), wxALIGN_LEFT);
#ifdef __MINGW32__
    label->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	label->SetFont(font);

	sizer->Add( label, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
	m_gdbLocation = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
			wxDefaultPosition, wxDefaultSize, wxTE_RICH2);
	sizer->Add( m_gdbLocation, 1, wxALL, 0);
#ifdef __MINGW32__
    m_gdbLocation->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	value = CONFIG->GetProperty(wxT("INVOKE/GDBLOCATION"), wxT("value"));
	if ( value.Length() > 0 )
	{
		m_gdbLocation->AppendText(value);
	}
	else
	{
		m_gdbLocation->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY));
		m_gdbLocation->AppendText(wxT("<Enter_location>"));
	}
	m_button = new wxButton(this, 
			ID_SELECT_GDB_LOCATION, wxT("Browse..."));
	Connect(ID_SELECT_GDB_LOCATION, wxEVT_COMMAND_BUTTON_CLICKED, 
			(wxObjectEventFunction) (wxEventFunction) 
			&InvokeApp::OnSelectGDBLocation);
#ifdef __MINGW32__
    m_button->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	sizer->Add( m_button, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 3);
	m_mainSizer->Add( sizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 6);
	
	m_gdbLocation->Connect(wxEVT_SET_FOCUS, 
			(wxObjectEventFunction) (wxEventFunction) 
			&InvokeApp::OnGDBLocationEnter, 0, this);
	m_gdbLocation->Connect(wxEVT_KILL_FOCUS, 
			(wxObjectEventFunction) (wxEventFunction) 
			&InvokeApp::OnGDBLocationLeave, 0, this);
	
	// --------------------
	wxStaticLine* line = new wxStaticLine(this, wxID_ANY);
	m_mainSizer->Add(line, 0, wxEXPAND|wxALL, 6);

	// --------------------
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxButton *m_okButton = new wxButton(this, wxID_OK, wxT("Ok"));
	m_okButton->SetDefault();
#ifdef __MINGW32__
    m_okButton->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, 
			(wxObjectEventFunction) (wxEventFunction) &InvokeApp::OnOk);
	buttonSizer->Add( m_okButton, 0, wxALIGN_RIGHT, 0);
	
	wxButton *m_cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"));
#ifdef __MINGW32__
    m_cancelButton->SetFont(dockArt->GetFont(wxAUI_DOCKART_CAPTION_FONT));
#endif
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, 
			(wxObjectEventFunction) (wxEventFunction) &InvokeApp::OnCancel);
	buttonSizer->Add( m_cancelButton, 0, wxALIGN_RIGHT, 0);
	m_mainSizer->Add( buttonSizer, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT, 3);
	
	m_executableLocation->SetFocus();

	SetSizer( m_mainSizer );
}

InvokeApp::~InvokeApp () 
{
}

void InvokeApp::OnArgumentsEnter(wxChildFocusEvent& event)
{
	if ( m_args->GetValue() == wxT("<Enter_arguments - optional>") )
	{
		m_args->SetValue(wxEmptyString);
	}
	event.Skip();
}

void InvokeApp::OnArgumentsLeave(wxChildFocusEvent& event)
{
	if ( m_args->GetValue() == wxT("") )
	{
		m_args->Clear();
		m_args->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY));
		m_args->AppendText(wxT("<Enter_arguments - optional>"));
	}
	event.Skip();
}

void InvokeApp::OnExecutableEnter(wxChildFocusEvent& event)
{
	if ( m_executableLocation->GetValue() == wxT("<Enter_executable_file>") )
	{
		m_executableLocation->SetValue(wxEmptyString);
	}
	event.Skip();
}

void InvokeApp::OnExecutableLeave(wxChildFocusEvent& event)
{
	if ( m_executableLocation->GetValue() == wxT("") )
	{
		m_executableLocation->Clear();
		m_executableLocation->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY));
		m_executableLocation->AppendText(wxT("<Enter_executable_file>"));
	}
	event.Skip();
}

void InvokeApp::OnSelectExecutable(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog dialog(this,
		wxT("Select a executable file."),
		wxEmptyString, exePath,
		wxT("Executable Files (*)|*"),
		wxFD_OPEN);
	dialog.SetFilterIndex(0);

	if (dialog.ShowModal() == wxID_OK) 
	{
		//wxFileName* file = new wxFileName(dialog.GetPath());
		m_executableLocation->SetValue(dialog.GetPath());
	}
}

void InvokeApp::OnSelectSourceLocation(wxCommandEvent& WXUNUSED(event))
{
    int style = wxDD_DEFAULT_STYLE & ~wxDD_DIR_MUST_EXIST;
    wxDirDialog dialog(this, 
		wxT("Select a source location."), 
		wxEmptyString, style);

    if (dialog.ShowModal() == wxID_OK)
    {
		m_sourceLocation->SetValue(dialog.GetPath());
    }
}

void InvokeApp::OnSelectGDBLocation(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog dialog(this,
		wxT("Select a GDB program."),
		wxEmptyString, exePath,
		wxT("GDB Files (*)|*"),
		wxFD_OPEN);
	dialog.SetFilterIndex(0);

	if (dialog.ShowModal() == wxID_OK) 
	{
		m_gdbLocation->SetValue(dialog.GetPath());
	}
}

void InvokeApp::OnSourceLocationEnter(wxChildFocusEvent& event)
{
	if ( m_sourceLocation->GetValue() == wxT("<Enter_location>") )
	{
		m_sourceLocation->SetValue(wxEmptyString);
	}
	event.Skip();
}

void InvokeApp::OnSourceLocationLeave(wxChildFocusEvent& event)
{
	if ( m_sourceLocation->GetValue() == wxT("") )
	{
		m_sourceLocation->Clear();
		m_sourceLocation->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY));
		m_sourceLocation->AppendText(wxT("<Enter_location>"));
	}
	event.Skip();
}

void InvokeApp::OnGDBLocationEnter(wxChildFocusEvent& event)
{
	if ( m_gdbLocation->GetValue() == wxT("<Enter_location>") )
	{
		m_gdbLocation->SetValue(wxEmptyString);
	}
	event.Skip();
}

void InvokeApp::OnGDBLocationLeave(wxChildFocusEvent& event)
{
	if ( m_gdbLocation->GetValue() == wxT("") )
	{
		m_gdbLocation->Clear();
		m_gdbLocation->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY));
		m_gdbLocation->AppendText(wxT("<Enter_location>"));
	}
	event.Skip();
}

void InvokeApp::OnOk(wxCommandEvent& WXUNUSED(event))
{
	wxString app = GetExecutableFile();
	wxString args = GetArguments();
	wxString srcDir = GetSourceLocation();

	if ( app.Length() == 0 || app == wxT("<Enter_executable_file>") ) {
		wxMessageDialog(this, 
			wxT("Please select a application by using Browse button or enter in the Location field."), 
			wxT("MyGDB"), wxOK|wxICON_WARNING).ShowModal();
		return;
	}
	
	if ( srcDir.Length() == 0 || srcDir == wxT("<Enter_location>") ) {
		wxMessageDialog(this, 
			wxT("Please select a source location by using Browse button or enter in the Location field."), 
			wxT("MyGDB"), wxOK|wxICON_WARNING).ShowModal();
		return;
	}

	EndDialog(wxID_OK);
}

void InvokeApp::OnCancel(wxCommandEvent& WXUNUSED(event))
{
	EndDialog(wxID_CANCEL);
}

// --------- getter
wxString InvokeApp::GetSourceLocation()
{
	wxString dir = m_sourceLocation->GetValue();
	if ( dir ==  wxT("<Enter_location>") )
		return wxEmptyString;

	return dir;
}

wxString InvokeApp::GetGDBLocation()
{
	wxString gdb = m_gdbLocation->GetValue();
	if ( gdb ==  wxT("<Enter_location>") )
		return wxEmptyString;

	return gdb;
}

wxString InvokeApp::GetExecutableFile()
{
	wxString app = m_executableLocation->GetValue();
	if ( app ==  wxT("<Enter_executable_file>") )
		return wxEmptyString;

	return app;
}

wxString InvokeApp::GetArguments()
{
	wxString args = m_args->GetValue();
	if ( args ==  wxT("<Enter_arguments - optional>") )
		return wxEmptyString;

	return args;
}
