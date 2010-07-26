// vim: set fdm=marker ts=4 sw=4:

#define _WIN32_IE 0x0600

#include <Base.h>

#ifndef _MYGDB_H_
#define _MYGDB_H_

class wxString;

//#include <Icon.h>

#include <Console.h>
#include <Debugger.h>
#include <MyAuiDockArt.h>
#include <notebookstyles.h>
#include <Completion.h>
#include <STCStyle.h>
#include <Config.h>
#include <InvokeApp.h>
#include <StyledTextCtrl.h>
#include <CodeEditor.h>
#include <Watch.h>
#include <Register.h>
#include <CallStack.h>
#include <Breakpoint.h>
#include <GridCtrl.h>
#include <Memory.h>
#include <About.h>
#include <Encoding.h>

//
// MyGdb 메인 프레임
//
class MyGdbFrame : public wxAuiMDIParentFrame
{

public:
    MyGdbFrame(wxWindow* parent,
            wxWindowID id,
            const wxString& title,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_FRAME_STYLE);

    ~MyGdbFrame();

public:
	wxAuiManager m_mgr;
	wxStatusBar* m_statusBar;
    
	wxAuiToolBar* m_fileToolBar;
	wxAuiToolBar* m_debugToolBar;
	
	STCStyle* m_stcStyle;
	
	Console* m_console;
	Debugger* m_debugger;
	
	Watch* m_watch;
	Register* m_register;
	CallStack* m_callStack;
	Breakpoint* m_breakpoint;
	Memory* m_memory;

	Config *m_config;
	
	wxMenuBar *m_menuBar;
    
public:
	wxAuiDockArt* GetDockArt();
	void GetAllEditors(std::vector<CodeEditor*> &editors);
	void ClearAllDebugPointer();
	CodeEditor *OpenSource(wxString path);

	void AddBreakpoint(wxString path, int addLine);
	void RemoveBreakpoint(wxString path, int removeLine);
	
private:
	void OnSize(wxSizeEvent& event);
    void OnUpdateUI(wxUpdateUIEvent& evt);
	void OnExit(wxCommandEvent& WXUNUSED(event));
	void OnCloseApp(wxCloseEvent& WXUNUSED(event));

	void OnOpenFile(wxCommandEvent& WXUNUSED(event));
	void OnInvokeApp(wxCommandEvent& WXUNUSED(event));

	void OnStart(wxCommandEvent& WXUNUSED(event));
	void OnAbort(wxCommandEvent& WXUNUSED(event));
	void OnStop(wxCommandEvent& WXUNUSED(event));

	void OnAbout(wxCommandEvent& WXUNUSED(event));

	// run
	void OnRun(wxCommandEvent& WXUNUSED(event));
	// next
	void OnStepOver(wxCommandEvent& WXUNUSED(event));
	// si
	void OnStepInto(wxCommandEvent& WXUNUSED(event));
	// finish
	void OnStepOut(wxCommandEvent& WXUNUSED(event));
	// continue
	void OnGo(wxCommandEvent& WXUNUSED(event));
	// advance
	void OnRunToCursor(wxCommandEvent& WXUNUSED(event));

	void OnViewConsole(wxCommandEvent& WXUNUSED(event));
	void OnViewWatch(wxCommandEvent& WXUNUSED(event));
	void OnViewCallStack(wxCommandEvent& WXUNUSED(event));
	void OnViewBreakpoint(wxCommandEvent& WXUNUSED(event));
	void OnViewRegister(wxCommandEvent& WXUNUSED(event));
	void OnViewMemory(wxCommandEvent& WXUNUSED(event));

private:
    wxArrayString m_perspectives;
    long m_notebook_style;

    DECLARE_EVENT_TABLE()
};

#endif
