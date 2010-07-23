// vim: set fdm=marker ts=4 sw=4:

#include <MyGDB.h>

#include <wx/image.h>
#include <wx/dc.h>

#include "xpm/closetab.xpm"
#include "xpm/closetab-active.xpm"
#include "xpm/closetab-pressed.xpm"

MyAuiDockArt::MyAuiDockArt()
{
	wxImage img(closetab_xpm);
	m_bmp_close = wxBitmap(img);

	wxImage img2(closetabactive_xpm);
	m_bmp_close_hover = wxBitmap(img2);
	
	wxImage img3(closetabpressed_xpm);
	m_bmp_close_pressed = wxBitmap(img3);
}

MyAuiDockArt::~MyAuiDockArt()
{
}

void MyAuiDockArt::DrawPaneButton(wxDC& dc, wxWindow* window, 
		int button, int button_state, const wxRect& rect, wxAuiPaneInfo& pane)
{
	switch(button){
	case wxAUI_BUTTON_CLOSE: {
		wxBitmap bmp;
		switch(button_state){
		case wxAUI_BUTTON_STATE_PRESSED:
			bmp = m_bmp_close_pressed;
			break;
		case wxAUI_BUTTON_STATE_HIDDEN:
			break;
		case wxAUI_BUTTON_STATE_HOVER:
			bmp = m_bmp_close_hover;
			break;
		case wxAUI_BUTTON_STATE_NORMAL:
			bmp = m_bmp_close;
			break;
		default:
			bmp = m_bmp_close;
			break;
		}
		dc.DrawBitmap(bmp, rect.x, rect.y+1, true);
		break;
	}
	default:
		wxAuiDefaultDockArt::DrawPaneButton(dc, 
				window, button, button_state, rect, pane);
		break;
	}
}
