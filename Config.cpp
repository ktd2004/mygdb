// vim: set fdm=marker ts=4 sw=4:

#pragma warning(disable: 4819)

#include <MyGDB.h>

Config::Config() 
{ 
	sXml = wxT("");

	//fileName = exePath + wxT("/mygdb.cfg");
#ifdef __MINGW32__
	fileName = wxFileName::GetHomeDir() + wxT("/mygdb.cfg");
#else
	fileName = wxFileName::GetHomeDir() + wxT("/.mygdb.cfg");
#endif

	if ( wxFileName::FileExists(fileName) )
	{
		wxString msg = wxT("Session already exist in the system.\ndo you want to load it?");
		int ret = wxMessageBox(msg, wxT("MyGDB"), 
				wxYES_NO|wxCANCEL|wxICON_QUESTION);
		if ( ret == wxYES ) 
		{
			Open();
		}
		else if ( ret == wxCANCEL ) 
		{
			exit(0);
		}
		else
		{
			New();
			Save();
		}
	}
	else
	{
		New();
		Save();
	}
}

Config::~Config() { }

void Config::New (void)
{
	xml = new wxXmlDocument();
	//xml->SetEncoding(wxT("UTF-8"));
	xml->SetVersion(wxT("1.0"));

	root = new wxXmlNode(wxXML_ELEMENT_NODE, wxT("MYGDB"));
	xml->SetRoot(root);

	// version
	SetProperty(wxT("VERSION"), wxT("value"), wxT("1.0"));

	// creation
	wxDateTime time = wxDateTime::Now();
	SetProperty(wxT("CREATION"), wxT("value"), 
			time.Format(wxT("%c"), wxDateTime::CET).c_str());
	SetProperty(wxT("NAME"), wxT("value"), wxT("MyGDB Configuration"));
	
	// invoke application
	SetProperty(wxT("INVOKE/EXECUTABLELOCATION"), wxT("value"), wxT(""));
	SetProperty(wxT("INVOKE/SOURCELOCATION"), wxT("value"), wxT(""));
	SetProperty(wxT("INVOKE/GDBLOCATION"), wxT("value"), wxT(""));
	SetProperty(wxT("INVOKE/ARGS"), wxT("value"), wxT(""));
	
	SetProperty(wxT("DEBUG/BREAKPOINT"), wxT("value"), wxT(""));

	// editor general
	SetProperty(wxT("EDITOR/GENERAL/USETABCHAR"), wxT("value"), wxT("true"));
	SetProperty(wxT("EDITOR/GENERAL/TABINDENTS"), wxT("value"), wxT("false"));
	SetProperty(wxT("EDITOR/GENERAL/TABSIZE"), wxT("value"), wxT("4"));
	SetProperty(wxT("EDITOR/GENERAL/AUTOINDENT"), wxT("value"), wxT("true"));
	SetProperty(wxT("EDITOR/GENERAL/SMARTINDENT"), wxT("value"), wxT("true"));
	SetProperty(wxT("EDITOR/GENERAL/INDENTGUIDES"), wxT("value"), wxT("true"));
	// 0: file name only, 1: relative file name, 2: full path
	SetProperty(wxT("EDITOR/GENERAL/TITLE"), wxT("value"), wxT("0"));
	SetProperty(wxT("EDITOR/GENERAL/WORDWRAP"), wxT("value"), wxT("true"));
	SetProperty(wxT("EDITOR/GENERAL/LINENUMBER"), wxT("value"), wxT("true"));
	SetProperty(wxT("EDITOR/GENERAL/LINECARET"), wxT("value"), wxT("true"));
	//SetProperty("EDITOR/GENERAL/SHOWHELP"), wxT("value"), wxT("false"));
	SetProperty(wxT("EDITOR/GENERAL/SHOWVALUE"), wxT("value"), wxT("true"));
	SetProperty(wxT("EDITOR/GENERAL/SHOWEDGE"), wxT("value"), wxT("false"));
	SetProperty(wxT("EDITOR/GENERAL/EDGECOLUMN"), wxT("value"), wxT("80"));
	SetProperty(wxT("EDITOR/GENERAL/RULER"), wxT("value"), wxT("false"));
	SetProperty(wxT("EDITOR/GENERAL/CODEEXPLORERUPDATETIME"), wxT("value"), wxT("2000"));
}

void Config::Save (void)
{
	xml->Save(fileName);
	sXml = Read(fileName);
}

bool Config::Open(void)
{
	xml = new wxXmlDocument();
	if ( !xml->Load(fileName) ) 
	{
		return false;
	}
	
	if (xml->GetRoot()->GetName() != wxT("MYGDB")) 
	{
		wxMessageDialog(NULL, 
			wxT("Unknown MyGDB configuration file"),
			wxT("MyGDB"), wxOK|wxICON_ERROR).ShowModal();
		return false;
	}

	root = xml->GetRoot();
	
	// -------------
	sXml = Read(fileName);

	return true;
}

wxString Config::Read(wxString fileName)
{
	wxString buffer = wxT("");

	wxFile* file = new wxFile();
	if ( file->Open(fileName, wxFile::read) ) {
		unsigned long fileSize = file->Length();

		char *pBuf = (char *)malloc(fileSize+1);
		size_t nResult = file->Read( pBuf, fileSize );

		if ( wxInvalidOffset == (int)nResult ) {
			wxMessageBox(wxString::Format(wxT("Error reading %s file."), 
						fileName.c_str()), wxT("MyGDB"), wxICON_ERROR);
			return buffer;
		}

		pBuf[fileSize] = 0;
		buffer = wxString::Format(wxT("%s"), pBuf);
		file->Close();
		free(pBuf);
	}
	delete file;

	return buffer;
}

bool Config::Changed(void)
{
	wxString buffer;

	wxStringOutputStream stream;
	xml->Save(stream);
	buffer = stream.GetString();
	stream.Close();

	if ( sXml == buffer )
	{
		return false;
	}
	else
	{
		return true;
	}
}

void Config::Backup(void)
{
	wxStringOutputStream stream;
	xml->Save(stream);
	backup = stream.GetString();
	stream.Close();
}

void Config::Restore(void)
{
	wxStringInputStream stream(backup);
	delete xml;
	xml = new wxXmlDocument();
	xml->Load(stream);
	root = xml->GetRoot();
}

wxXmlNode* Config::SetNode(wxString nodePath)
{
	wxXmlNode* pNode = root;	// parent node

	wxStringTokenizer tkz(nodePath, wxT("/"));
	while ( tkz.HasMoreTokens() ) 
	{
		wxString token = tkz.GetNextToken();
		
		wxXmlNode* node = pNode->GetChildren();
	
		int exist = false;
		while (node) 
		{
			if ( node->GetName() == token ) 
			{
				pNode = node;
				exist = true;
				break;
			}
			node = node->GetNext();
		}

		if ( exist == false ) 
		{
			wxXmlNode *childNode = new wxXmlNode (wxXML_ELEMENT_NODE, token);
			pNode->AddChild(childNode);
			pNode = childNode;
		}
	}

	return pNode;
}

bool Config::SetProperty(wxString nodePath, wxString property, wxString value)
{
	wxXmlNode *node = SetNode(nodePath);
	wxXmlProperty* prop = node->GetProperties();
	
	bool exist=false;	
	while (prop) 
	{
		if ( prop->GetName() == property ) 
		{
			prop->SetValue(value);
			exist=true;
		}
		prop = prop->GetNext();
	}

	if (exist==false)
	{
		node->AddProperty(property, value);
	}

	return true;
}

bool Config::GetPropertyBool(wxString nodePath, wxString property)
{
	wxString value = GetProperty(nodePath, property);
	if ( value == wxT("true") ) 
		return true;
	else 
		return false;
}

long Config::GetPropertyInt(wxString nodePath, wxString property)
{
	long intValue; 
	wxString value = GetProperty(nodePath, property);
	value.ToLong(&intValue, 10);
	return intValue;
}

wxString Config::GetProperty(wxString nodePath, wxString property)
{
	wxXmlNode *node = SetNode(nodePath);
	wxXmlProperty* prop = node->GetProperties();
		
	while (prop) 
	{
		if ( prop->GetName() == property ) 
		{
			return prop->GetValue();
		}
		prop = prop->GetNext();
	}
		
	if ( nodePath == wxT("EDITOR/GENERAL/USETABCHAR") ) return wxT("true");
	if ( nodePath == wxT("EDITOR/GENERAL/TABINDENTS")) return wxT("false");
	if ( nodePath == wxT("EDITOR/GENERAL/TABSIZE") ) return wxT("4");
	if ( nodePath == wxT("EDITOR/GENERAL/AUTOINDENT") ) return wxT("true");
	if ( nodePath == wxT("EDITOR/GENERAL/SMARTINDENT") ) return wxT("true");
	if ( nodePath == wxT("EDITOR/GENERAL/INDENTGUIDES") ) return wxT("true");
	// 0: file name only, 1: relative file name, 2: full path
	if ( nodePath == wxT("EDITOR/GENERAL/TITLE") ) return wxT("0");
	if ( nodePath == wxT("EDITOR/GENERAL/WORDWRAP") ) return wxT("true");
	if ( nodePath == wxT("EDITOR/GENERAL/LINENUMBER") ) return wxT("true");
	if ( nodePath == wxT("EDITOR/GENERAL/LINECARET") ) return wxT("true");
	//if ( nodePath == wxT("EDITOR/GENERAL/SHOWHELP") ) return wxT("false");
	if ( nodePath == wxT("EDITOR/GENERAL/SHOWVALUE") ) return wxT("true");
	if ( nodePath == wxT("EDITOR/GENERAL/SHOWEDGE") ) return wxT("true");
	if ( nodePath == wxT("EDITOR/GENERAL/EDGECOLUMN") ) return wxT("80");
	if ( nodePath == wxT("EDITOR/GENERAL/RULER") ) return wxT("false");
	if ( nodePath == wxT("EDITOR/GENERAL/CODEEXPLORERUPDATETIME" )) return wxT("2000");
	if ( nodePath == wxT("EDITOR/STYLE") ) return wxT("Default");

	return wxT("");
}
