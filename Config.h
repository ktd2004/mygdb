// vim: set fdm=marker ts=4 sw=4:

#ifndef _CONFIG_H_
#define _CONFIG_H_

class Config
{
public:
	wxXmlDocument* xml;
	wxXmlNode* root;

public:
	Config();
	~Config();
	
	wxString fileName;

	wxString sXml;
	wxString backup;

	bool Open(void);
	void Save (void);
	wxString Read(wxString fileName);
	bool Changed(void);
	void Backup(void);
	void Restore(void);

	wxXmlNode* SetNode(wxString nodePath);
	bool SetProperty(wxString nodePath, wxString property, wxString value);
	wxString GetProperty(wxString nodePath, wxString name);
	bool GetPropertyBool(wxString nodePath, wxString property);
	long GetPropertyInt(wxString nodePath, wxString property);
};

#endif
