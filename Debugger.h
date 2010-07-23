// vim: set fdm=marker ts=4 sw=4:

#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

class MyGdbFrame;

class Debugger : public wxProcess
{
public:
	Debugger(MyGdbFrame *myFrame);
    
	~Debugger();
    virtual void OnTerminate(int pid, int status);

public:
	MyGdbFrame *myFrame;
	wxProcess *m_process;
    wxTimer *m_timer;
	
	bool m_annotating;
	bool m_prompt;
	bool m_processing;
	int m_pid;
	
public:
	char* wxStringToChar(wxString input);

	wxString Parse(wxString msg);
	wxString RemoveAnnotate(wxString msg);
	void UpdateWhere (wxString annotate);

	void Log(wxString msg);
	bool FindPrompt(wxString msg);

	wxString Eval(wxString cmd);

	void Puts(wxString str, int type);
	bool Write (wxString cmd);

	bool IsProcessing();
	bool IsSilent();

	wxString CreateInitFile ();
	bool Start ();
	void Flush();
	void Stop();
	void Abort();

private:
	void OnEndProcess(wxProcessEvent& event);
	void OnTimer(wxTimerEvent& WXUNUSED(event));
	void OnIdle(wxIdleEvent& WXUNUSED(event));
    
	DECLARE_EVENT_TABLE()
};

#endif
