// vim: set fdm=marker ts=4 sw=4:

#ifndef _STCSTYLE_H_
#define _STCSTYLE_H_

class STCStyle
{
public:
	STCStyle();
	~STCStyle();

private:
	wxArrayString m_styleNames;
	wxString m_keyWords;
	
	wxXmlNode *root;
	
public:
	bool Scan(wxString baseDir);

	wxArrayString GetStyleNames();
	wxString GetStyleFile(wxString styleName);

	bool ApplyEditorStyle(wxStyledTextCtrl *stc);
	bool ApplyConsoleStyle(wxStyledTextCtrl *stc);

	void GetInfo(wxString nodePath, wxString *fg, wxString *bg, 
		wxString *face, long *size, bool *bold, bool *italic, bool *underline);
	wxXmlDocument *OpenStyle(wxString styleName);
	void CloseStyle(wxXmlDocument *xml);

	wxXmlNode* SetNode(wxString nodePath);
	bool SetProperty(wxString nodePath, wxString property, wxString value);
	bool GetPropertyBool(wxString nodePath, wxString property);
	long GetPropertyInt(wxString nodePath, wxString property);
	wxString GetProperty(wxString nodePath, wxString property);

	void SetKeyWords(wxString words);
};

#endif
