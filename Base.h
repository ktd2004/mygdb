// vim: set fdm=marker ts=4 sw=4:

#ifndef _BASE_H_
#define _BASE_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include <wx/generic/aboutdlgg.h>
#include <wx/aboutdlg.h>
#include <wx/url.h>         //for wxURL/wxURI
#include <wx/cmdline.h>
#include <wx/dnd.h>
#include <wx/process.h>
#include <wx/txtstrm.h>
#include <wx/list.h>
#include <wx/tokenzr.h>
#include <wx/app.h>
#include <wx/grid.h>
#include <wx/laywin.h>
#include <wx/treectrl.h>
#include <wx/spinctrl.h>
#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/combobox.h>
#include <wx/colordlg.h>
#include <wx/wxhtml.h>
#include <wx/imaglist.h>
#include <wx/dataobj.h>
#include <wx/dcclient.h>
#include <wx/bmpbuttn.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/tooltip.h>
#include <wx/statusbr.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/timer.h>
#include <wx/statline.h>
#include <wx/listctrl.h>
#include <wx/dir.h>
#include <wx/dirctrl.h>
#include <wx/file.h>
#include <wx/mimetype.h>
#include <wx/regex.h>
#include <wx/taskbar.h>
#include <wx/xml/xml.h>
#include <wx/aui/aui.h>
#include <wx/stc/stc.h>  
#include <wx/ffile.h>
#include <wx/bitmap.h> 
#include <wx/mstream.h> 
#include <wx/stream.h> 
#include <wx/sstream.h>
#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/fileconf.h> 
#include <wx/fontenum.h>
#include <wx/socket.h>
#include <wx/hash.h>
#include <wx/clipbrd.h>
#include <wx/thread.h>
#include <wx/image.h>
#include <wx/dcbuffer.h>
#include <wx/colordlg.h>
#include <wx/popupwin.h>
#include <wx/strconv.h>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>

// stl
#include <vector>
#include <map>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

// treelistctrl
#include "treelistctrl.h"

using namespace std;

#define	MYGDB_STDOUT	0
#define	MYGDB_STDERR	1
#define	MYGDB_PROMPT	2
#define	MYGDB_STDIN		3

// --------------------------------------------
extern wxString exePath;
extern bool m_started;
class CodeEditor;

// --------------------------------------------
enum {
	ID_INVOKE_APP = wxID_HIGHEST+1,

	ID_OPEN_FILE,

	ID_ABORT,
	ID_START,
	ID_RUN,
	ID_GO,
	ID_STEP_INTO,
	ID_STEP_OUT,
	ID_STEP_OVER,
	ID_RUN_TO_CURSOR,
	ID_TOGGLE_BREAKPOINT,
	ID_CLEAR_ALL_BREAKPOINTS,
	ID_STOP,

	ID_VIEW_CONSOLE,
	ID_VIEW_WATCH,
	ID_VIEW_CALLSTACK,
	ID_VIEW_BREAKPOINT,
	ID_VIEW_REGISTER,
	ID_VIEW_MEMORY,
};

// --------------------------------------------
#define MENU_BAR					myFrame->m_menuBar
#define STATUS_BAR					myFrame->m_statusBar
#define PROJECT						myFrame->m_project
#define CONFIG						myFrame->m_config
#define BREAKPOINT					myFrame->m_breakpoint
#define FILE_TOOLBAR				myFrame->m_fileToolBar
#define DEBUG_TOOLBAR				myFrame->m_debugToolBar
#define CONSOLE						myFrame->m_console
#define M_MGR						myFrame->m_mgr
#define STC_STYLE					myFrame->m_stcStyle
#define WATCH						myFrame->m_watch
#define REGISTER					myFrame->m_register
#define CALLSTACK					myFrame->m_callStack
#define DEBUGGER					myFrame->m_debugger
#define MEMORY						myFrame->m_memory

#define COMPLETION					myParent->m_completion

#define BOOKMARK_MARKER     0x7
#define BREAKPOINT_MARKER   0x8
#define DEBUG_MARKER        0x9

#define NUMBER_MARGIN_ID 	0
#define SEP_MARGIN_ID 		1
#define SYMBOLS_MARGIN_ID 	2
#define FOLD_MARGIN_ID 		3

#define BOX_HEIGHT 150
#define BOX_WIDTH  300

#define GET_BITMAP(name)	\
	wxBitmap(exePath + wxT("/icons/") + \
			wxString(name, wxConvUTF8) + \
			wxT(".png"), wxBITMAP_TYPE_PNG)
#define GET_PNG_PATH(name)	\
	exePath + wxT("/icons/") + \
			wxString(name, wxConvUTF8) + wxT(".png")

extern const char *breakpoint_xpm[];
extern const char *stop_xpm[];
extern const char *arrow_right_green_xpm[];

#endif
