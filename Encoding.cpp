#include <MyGDB.h>

Encoding::Encoding() 
{
	m_UseBOM=false;
	m_Encoding=wxLocale::GetSystemEncoding();
	m_BOMSizeInBytes=-1;
	m_ConvStr = wxT("");
}

Encoding::~Encoding() 
{
}

bool Encoding::UsesBOM()
{
    return m_UseBOM;
}

int Encoding::GetBOMSizeInBytes()
{
    return m_BOMSizeInBytes;
}

wxFontEncoding Encoding::GetFontEncoding()
{
    return m_Encoding;
}

wxString Encoding::GetWxStr()
{
    return m_ConvStr;
}

bool Encoding::Detect(const wxString& filename, 
		bool ConvertToWxString)
{
    wxFile file(filename);
    if (!file.IsOpened())
        return false;

    size_t size = file.Length();
    if (size == 0)
    {
        file.Close();
        return false;
    }

    wxByte* buffer = (wxByte*) malloc(sizeof(wxByte) * (size + 4));
    if (!buffer)
    {
        file.Close();
        return false;
    }
    buffer[size + 0] = 0;
    buffer[size + 1] = 0;
    buffer[size + 2] = 0;
    buffer[size + 3] = 0;

    size_t readBytes = file.Read((void*)buffer, size);
    bool result = false;
    if (readBytes > 0)
    {
        result = Detect(buffer, size, ConvertToWxString);
    }

    file.Close();
    free(buffer);
    return result;
}

bool Encoding::Detect(
		const wxByte* buffer, size_t size, bool ConvertToWxString)
{
	if (!buffer)
		return false;

	if (size >= 4)
	{
		// BOM is max 4 bytes
		char buff[4] = {'\0'};
		memcpy(buff, buffer, 4);

		if (memcmp(buff, "\xEF\xBB\xBF", 3) == 0)
		{
			m_UseBOM = true;
			m_BOMSizeInBytes = 3;
			m_Encoding = wxFONTENCODING_UTF8;
		}
		else if (memcmp(buff, "\x00\x00\xFE\xFF", 4) == 0)
		{
			m_UseBOM = true;
			m_BOMSizeInBytes = 4;
			m_Encoding = wxFONTENCODING_UTF32BE;
		}
		else if (memcmp(buff, "\x00\x00\xFF\xFE", 4) == 0)
		{
		// 00 00 FF FE  UCS-4, unusual octet order BOM (2143)
		// X-ISO-10646-UCS-4-2143 can not (yet) be handled by wxWidgets
			m_Encoding = (wxFontEncoding)-1;
		}
		else if (memcmp(buff, "\xFF\xFE\x00\x00", 4) == 0)
		{
			m_UseBOM = true;
			m_BOMSizeInBytes = 4;
			m_Encoding = wxFONTENCODING_UTF32LE;
		}
		else if (memcmp(buff, "\xFE\xFF\x00\x00", 4) == 0)
		{
		// FE FF 00 00  UCS-4, unusual octet order BOM (3412)
		// X-ISO-10646-UCS-4-3412 can not (yet) be handled by wxWidgets
			m_Encoding = (wxFontEncoding)-1;
		}
		else if (memcmp(buff, "\xFE\xFF", 2) == 0)
		{
			m_UseBOM = true;
			m_BOMSizeInBytes = 2;
			m_Encoding = wxFONTENCODING_UTF16BE;
		}
		else if (memcmp(buff, "\xFF\xFE", 2) == 0)
		{
			m_UseBOM = true;
			m_BOMSizeInBytes = 2;
			m_Encoding = wxFONTENCODING_UTF16LE;
		}
	}

    if (ConvertToWxString)
    {
        ConvertToWxStr(buffer, size);
    }

    return true;
} // end of DetectEncoding

bool Encoding::ConvertToWxStr(const wxByte* buffer, size_t size)
{
    if (!buffer || size == 0)
    {
		wxMessageDialog(NULL, 
			wxT("Encoding conversion has failed. (buffer is empty)"),
			wxT("MyGDB"), wxOK|wxICON_ERROR).ShowModal();
        return false;
    }

    if (m_BOMSizeInBytes > 0)
    {
        for (int i = 0; i < m_BOMSizeInBytes; ++i)
            *buffer++;
    }

    size_t outlen = 0;

    wxWCharBuffer wideBuff;

    // if possible use the special conversion-routines, 
	// they are much faster than wxCSCov (at least on linux)
    if ( m_Encoding == wxFONTENCODING_UTF7 )
    {
        wxMBConvUTF7 conv;
        wideBuff = conv.cMB2WC((char*)buffer, 
				size + 4 - m_BOMSizeInBytes, &outlen);
    }
    else if ( m_Encoding == wxFONTENCODING_UTF8 )
    {
        wxMBConvUTF8 conv;
        wideBuff = conv.cMB2WC((char*)buffer, 
				size + 4 - m_BOMSizeInBytes, &outlen);
    }
    else if ( m_Encoding == wxFONTENCODING_UTF16BE )
    {
        wxMBConvUTF16BE conv;
        wideBuff = conv.cMB2WC((char*)buffer, 
				size + 4 - m_BOMSizeInBytes, &outlen);
    }
    else if ( m_Encoding == wxFONTENCODING_UTF16LE )
    {
        wxMBConvUTF16LE conv;
        wideBuff = conv.cMB2WC((char*)buffer, 
				size + 4 - m_BOMSizeInBytes, &outlen);
    }
    else if ( m_Encoding == wxFONTENCODING_UTF32BE )
    {
        wxMBConvUTF32BE conv;
        wideBuff = conv.cMB2WC((char*)buffer, 
				size + 4 - m_BOMSizeInBytes, &outlen);
    }
    else if ( m_Encoding == wxFONTENCODING_UTF32LE )
    {
        wxMBConvUTF32LE conv;
        wideBuff = conv.cMB2WC((char*)buffer, 
				size + 4 - m_BOMSizeInBytes, &outlen);
    }
    else
    {
        // try wxEncodingConverter first, even it it only works for
        // wxFONTENCODING_ISO8859_1..15, wxFONTENCODING_CP1250..1257 and wxFONTENCODING_KOI8
        // but it's much, much faster than wxCSConv (at least on linux)
        wxEncodingConverter conv;
        wchar_t* tmp = new wchar_t[size + 4 - m_BOMSizeInBytes];
        if(   conv.Init(m_Encoding, wxFONTENCODING_UNICODE)
		   && conv.Convert((char*)buffer, tmp) )
        {
            wideBuff = tmp;
            outlen = size + 4 - m_BOMSizeInBytes; // should be correct, because Convert has returned true
        }
        else
        {
            // try wxCSConv, if nothing else works
            wxCSConv conv(m_Encoding);
            wideBuff = conv.cMB2WC((char*)buffer, size + 4 - m_BOMSizeInBytes, &outlen);
        }
        delete [] tmp;
    }

    m_ConvStr = wxString(wideBuff);

    if (outlen == 0)
    {
#ifdef __MINGW32__
		// Trying system locale as fallback...
		m_Encoding = wxLocale::GetSystemEncoding();
#else
		// Trying ISO-8859-1 as fallback...
		m_Encoding = wxFONTENCODING_ISO8859_1;
#endif

		wxCSConv conv_system(m_Encoding);
		wideBuff = conv_system.cMB2WC((char*)buffer, size + 4 - m_BOMSizeInBytes, &outlen);
		m_ConvStr = wxString(wideBuff);
		
		if (outlen == 0)
		{
			wxMessageDialog(NULL, 
				wxT("Encoding conversion using system locale fallback has failed!"),
				wxT("MyGDB"), wxOK|wxICON_ERROR).ShowModal();
			return false;
		}
    }

    return true;
}
