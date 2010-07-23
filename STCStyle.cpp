// vim: set fdm=marker ts=4 sw=4:

#pragma warning(disable: 4819)

#include <MyGDB.h>

STCStyle::STCStyle() 
{ 
	m_keyWords = wxT("");
}

STCStyle::~STCStyle() 
{ 
}

bool STCStyle::Scan (wxString baseDir)
{
#ifdef __MINGW32__
	wxString SLASH = wxT("\\");
#else
	wxString SLASH = wxT("/");
#endif
	
	m_styleNames.Clear();
 
    if (baseDir[baseDir.length()-1] != SLASH) {
		baseDir += SLASH;        
    }
 
    if(!wxDir::Exists(baseDir)) {
		return false;
    }
       
    wxDir dir(baseDir);
    wxString fileName;
    bool bla = dir.GetFirst(&fileName);
   
    if (bla) {
        do {
			if ( fileName.Lower().Matches(wxT("*.mygdb")) ) 
			{
				wxXmlDocument *xml = new wxXmlDocument();
				if ( !xml->Load(baseDir + fileName) ) 
				{
					delete xml;
					return false;
				}
				root = xml->GetRoot();
				wxString value = GetProperty(wxT("NAME"), wxT("value"));
				m_styleNames.Add(value);
				m_styleNames.Add(baseDir + fileName);
				delete xml;
			}
        }
        while (dir.GetNext(&fileName) );
    }

    return true;
}

wxArrayString STCStyle::GetStyleNames()
{
	wxArrayString names;

	for(unsigned int i=0; i < m_styleNames.Count(); i+=2)
	{
		names.Add(m_styleNames[i]);
	}

	return names;
}

wxString STCStyle::GetStyleFile(wxString styleName)
{
	for(unsigned int i=0; i < m_styleNames.Count(); i+=2)
	{
		if ( m_styleNames[i] == styleName )
			return m_styleNames[i+1];
	}

	return wxT("");
}

void STCStyle::GetInfo(wxString nodePath, wxString *fg, wxString *bg,
		wxString *face, long *size, bool *bold, bool *italic, bool *underline)
{
	*fg = GetProperty(nodePath, wxT("fg"));
	*bg = GetProperty(nodePath, wxT("bg"));
	*face = GetProperty(nodePath, wxT("face"));
	*size = GetPropertyInt(nodePath, wxT("size"));
	*bold = GetPropertyBool(nodePath, wxT("bold"));
	*italic = GetPropertyBool(nodePath, wxT("italic"));
	*underline = GetPropertyBool(nodePath, wxT("underline"));
}

wxXmlDocument *STCStyle::OpenStyle(wxString styleName)
{
	wxString fileName = GetStyleFile(styleName);

	wxXmlDocument *xml = new wxXmlDocument();
	if ( !xml->Load(fileName) ) 
	{
		return false;
	}

	root = xml->GetRoot();

	return xml;
}

void STCStyle::CloseStyle(wxXmlDocument *xml)
{
	delete xml;
}

bool STCStyle::ApplyEditorStyle(wxStyledTextCtrl *stc)
{
	wxString fg, bg, face;
	bool bold, italic, underline;
	long size;

	GetInfo(wxT("EDITOR/SELECTION"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	if ( fg.Len() > 0 ) stc->SetSelForeground(1, fg);
	if ( bg.Len() > 0 ) stc->SetSelBackground(1, bg);

	GetInfo(wxT("EDITOR/CARETLINE"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	if ( fg.Len() > 0 ) stc->SetCaretForeground(fg);	// cursor color
	if ( bg.Len() > 0 ) stc->SetCaretLineBackground(bg);
	
	GetInfo(wxT("EDITOR/EDGE"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	if ( fg.Len() > 0 ) stc->SetEdgeColour(fg);

	GetInfo(wxT("EDITOR/FOLDMARGIN"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	if ( fg.Len() > 0 )
	{
		stc->SetFoldMarginColour(true, wxColour(fg));
		stc->SetFoldMarginHiColour(true, wxColour(fg));
	}

	stc->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, wxColour(fg));
	stc->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, wxColour(bg));
	stc->MarkerSetForeground(wxSTC_MARKNUM_FOLDER, wxColour(fg));
	stc->MarkerSetBackground(wxSTC_MARKNUM_FOLDER, wxColour(bg));
	stc->MarkerSetForeground(wxSTC_MARKNUM_FOLDERSUB, wxColour(fg));
	stc->MarkerSetBackground(wxSTC_MARKNUM_FOLDERSUB, wxColour(bg));
	stc->MarkerSetForeground(wxSTC_MARKNUM_FOLDERTAIL, wxColour(fg));
	stc->MarkerSetBackground(wxSTC_MARKNUM_FOLDERTAIL, wxColour(bg));
	stc->MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, wxColour(fg));
	stc->MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, wxColour(bg));
	stc->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, wxColour(fg));
	stc->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, wxColour(bg));
	stc->MarkerSetForeground(wxSTC_MARKNUM_FOLDERMIDTAIL, wxColour(fg));
	stc->MarkerSetBackground(wxSTC_MARKNUM_FOLDERMIDTAIL, wxColour(bg));

	GetInfo(wxT("EDITOR/DEFAULT"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_STYLE_DEFAULT, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetSpec(wxSTC_C_DEFAULT, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetSize(wxSTC_C_DEFAULT, size);

	GetInfo(wxT("EDITOR/IDENTIFIER"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_C_IDENTIFIER, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_C_IDENTIFIER, bold);
	stc->StyleSetItalic(wxSTC_C_IDENTIFIER, italic);
	stc->StyleSetUnderline(wxSTC_C_IDENTIFIER, underline);
	stc->StyleSetSize(wxSTC_C_IDENTIFIER, size);
	
	GetInfo(wxT("EDITOR/COMMENT"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_C_COMMENT, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_C_COMMENT, bold);
	stc->StyleSetItalic(wxSTC_C_COMMENT, italic);
	stc->StyleSetUnderline(wxSTC_C_COMMENT, underline);
	stc->StyleSetSize(wxSTC_C_COMMENT, size);
	
	stc->StyleSetSpec(wxSTC_C_COMMENTDOC, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_C_COMMENTDOC, bold);
	stc->StyleSetItalic(wxSTC_C_COMMENTDOC, italic);
	stc->StyleSetUnderline(wxSTC_C_COMMENTDOC, underline);
	stc->StyleSetSize(wxSTC_C_COMMENTDOC, size);
	
	stc->StyleSetSpec(wxSTC_C_COMMENTLINE, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_C_COMMENTLINE, bold);
	stc->StyleSetItalic(wxSTC_C_COMMENTLINE, italic);
	stc->StyleSetUnderline(wxSTC_C_COMMENTLINE, underline);
	stc->StyleSetSize(wxSTC_C_COMMENTLINE, size);
	
	// ------- new
	GetInfo(wxT("EDITOR/STRING"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_C_STRING, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_C_STRING, bold);
	stc->StyleSetItalic(wxSTC_C_STRING, italic);
	stc->StyleSetUnderline(wxSTC_C_STRING, underline);
	stc->StyleSetSize(wxSTC_C_STRING, size);
	
	stc->StyleSetSpec(wxSTC_C_STRINGEOL, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_C_STRINGEOL, bold);
	stc->StyleSetItalic(wxSTC_C_STRINGEOL, italic);
	stc->StyleSetUnderline(wxSTC_C_STRINGEOL, underline);
	stc->StyleSetSize(wxSTC_C_STRINGEOL, size);
	
	// ------- new
	GetInfo(wxT("EDITOR/CHARACTER"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_C_CHARACTER, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_C_CHARACTER, bold);
	stc->StyleSetItalic(wxSTC_C_CHARACTER, italic);
	stc->StyleSetUnderline(wxSTC_C_CHARACTER, underline);
	stc->StyleSetSize(wxSTC_C_CHARACTER, size);

	GetInfo(wxT("EDITOR/NUMBER"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_C_NUMBER, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_C_NUMBER, bold);
	stc->StyleSetItalic(wxSTC_C_NUMBER, italic);
	stc->StyleSetUnderline(wxSTC_C_NUMBER, underline);
	stc->StyleSetSize(wxSTC_C_NUMBER, size);

	GetInfo(wxT("EDITOR/OPERATOR"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_C_OPERATOR, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_C_OPERATOR, bold);
	stc->StyleSetItalic(wxSTC_C_OPERATOR, italic);
	stc->StyleSetUnderline(wxSTC_C_OPERATOR, underline);
	stc->StyleSetSize(wxSTC_C_OPERATOR, size);
	
	// ---------- new
	GetInfo(wxT("EDITOR/PREPROCESSOR"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_C_PREPROCESSOR, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_C_PREPROCESSOR, bold);
	stc->StyleSetItalic(wxSTC_C_PREPROCESSOR, italic);
	stc->StyleSetUnderline(wxSTC_C_PREPROCESSOR, underline);
	stc->StyleSetSize(wxSTC_C_PREPROCESSOR, size);

	GetInfo(wxT("EDITOR/WORD"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_C_WORD, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_C_WORD, bold);
	stc->StyleSetItalic(wxSTC_C_WORD, italic);
	stc->StyleSetUnderline(wxSTC_C_WORD, underline);
	stc->StyleSetSize(wxSTC_C_WORD, size);
	
	GetInfo(wxT("EDITOR/WORD"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_C_WORD2, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_C_WORD2, bold);
	stc->StyleSetItalic(wxSTC_C_WORD2, italic);
	stc->StyleSetUnderline(wxSTC_C_WORD2, underline);
	stc->StyleSetSize(wxSTC_C_WORD2, size);

	GetInfo(wxT("EDITOR/LINENUMBERMARGIN"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_STYLE_LINENUMBER, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(wxSTC_STYLE_LINENUMBER, bold);
	stc->StyleSetItalic(wxSTC_STYLE_LINENUMBER, italic);
	stc->StyleSetUnderline(wxSTC_STYLE_LINENUMBER, underline);
	stc->StyleSetSize(wxSTC_STYLE_LINENUMBER, size);
	
	stc->SetWordChars(wxT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"));
	stc->SetKeyWords(0, m_keyWords);
	
	return true;
}

bool STCStyle::ApplyConsoleStyle(wxStyledTextCtrl *stc)
{
	wxString fg, bg, face;
	bool bold, italic, underline;
	long size;

	GetInfo(wxT("CONSOLE/SELECTION"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	if ( fg.Len() > 0 ) stc->SetSelForeground(1, fg);
	if ( bg.Len() > 0 ) stc->SetSelBackground(1, bg);

	GetInfo(wxT("CONSOLE/CARETLINE"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	if ( fg.Len() > 0 ) stc->SetCaretForeground(fg);	// cursor color
	if ( bg.Len() > 0 ) stc->SetCaretLineBackground(bg);
	
	GetInfo(wxT("CONSOLE/EDGE"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	if ( fg.Len() > 0 ) stc->SetEdgeColour(fg);
	
	GetInfo(wxT("CONSOLE/DEFAULT"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_STYLE_DEFAULT, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetSpec(wxSTC_C_DEFAULT, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetSize(wxSTC_C_DEFAULT, size);
	
	GetInfo(wxT("CONSOLE/INPUT"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(wxSTC_C_DEFAULT, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetSize(wxSTC_C_DEFAULT, size);
	
	GetInfo(wxT("CONSOLE/ERROR"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(4, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetSpec(4, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(4, bold);
	stc->StyleSetItalic(4, italic);
	stc->StyleSetUnderline(4, underline);
	stc->StyleSetSize(4, size);
	
	GetInfo(wxT("CONSOLE/NORMAL"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(5, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetSpec(5, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(5, bold);
	stc->StyleSetItalic(5, italic);
	stc->StyleSetUnderline(5, underline);
	stc->StyleSetSize(5, size);

	GetInfo(wxT("CONSOLE/PROMPT"), 
			&fg, &bg, &face, &size, &bold, &italic, &underline);
	stc->StyleSetSpec(6, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetSpec(6, 
		wxString::Format(wxT("back:%s,fore:%s,face:%s"), 
			bg.c_str(), fg.c_str(), face.c_str()));
	stc->StyleSetBold(6, bold);
	stc->StyleSetItalic(6, italic);
	stc->StyleSetUnderline(6, underline);
	stc->StyleSetSize(6, size);

	return true;
}

void STCStyle::SetKeyWords(wxString words)
{
	m_keyWords = words;
}

wxXmlNode* STCStyle::SetNode(wxString nodePath)
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

bool STCStyle::SetProperty(
		wxString nodePath, wxString property, wxString value)
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

bool STCStyle::GetPropertyBool(wxString nodePath, wxString property)
{
	wxString value = GetProperty(nodePath, property);
	if ( value == wxT("true") ) 
		return true;
	else 
		return false;
}

long STCStyle::GetPropertyInt(wxString nodePath, wxString property)
{
	long v;
	wxString value = GetProperty(nodePath, property);
	value.ToLong(&v);
	return v;
}

wxString STCStyle::GetProperty(wxString nodePath, wxString property)
{
	wxXmlNode *node = SetNode(nodePath);
	wxXmlProperty* prop = node->GetProperties();
		
	while (prop) 
	{
		if ( prop->GetName() == property ) 
			return prop->GetValue();
		prop = prop->GetNext();
	}

	// return default value
	if ( property == wxT("bold") ) return wxT("false");
	if ( property == wxT("italic") ) return wxT("false");
	if ( property == wxT("underline") ) return wxT("false");
	if ( property == wxT("size") ) return wxT("-1");

	return wxEmptyString;
}
