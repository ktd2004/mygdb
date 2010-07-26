#ifndef _ENCODING_H_
#define _ENCODING_H_

class Encoding
{
public:
	Encoding();
	~Encoding();

	bool Detect(const wxString& filename, bool ConvertToWxString);
	bool Detect(const wxByte* buffer, size_t size, bool ConvertToWxString);
	bool ConvertToWxStr(const wxByte* buffer, size_t size);

	bool UsesBOM();
	int GetBOMSizeInBytes();
	wxFontEncoding GetFontEncoding();
	wxString GetWxStr();

private:
	bool m_UseBOM;
	wxFontEncoding m_Encoding;
	int m_BOMSizeInBytes;
	wxString m_ConvStr;
};

#endif

