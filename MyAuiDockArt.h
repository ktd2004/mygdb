// vim: set fdm=marker ts=4 sw=4:

#ifndef __MYAUIDOCKART__
#define __MYAUIDOCKART__

#include <wx/aui/framemanager.h>
#include <wx/aui/dockart.h>

class MyAuiDockArt : public wxAuiDefaultDockArt
{
	wxBitmap m_bmp_close;
	wxBitmap m_bmp_close_hover;
	wxBitmap m_bmp_close_pressed;

public:
	MyAuiDockArt();
	virtual ~MyAuiDockArt();

	void DrawPaneButton(wxDC& dc, wxWindow *window, int button, int button_state, const wxRect& rect, wxAuiPaneInfo& pane);
};
#endif // __clauidockart__
