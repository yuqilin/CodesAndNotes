#pragma once
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include <sstream>

using namespace std;
enum sh_xml_encode_type
{
	sh_xml_encode_type_gb2312 = 0,
	sh_xml_encode_type_utf8
};

template<int v>
class Int2Type
{
	enum{value = v};
};

typedef rapidxml::xml_node<wchar_t>			sh_xml_node;
typedef rapidxml::xml_attribute<wchar_t>	sh_xml_attribute;

template<sh_xml_encode_type encode_type>
class sh_xml_document : public rapidxml::xml_document<wchar_t>
{
public:
	bool open(LPCTSTR szPath)
	{
		std::basic_ifstream<char, char_traits<char> > xml_file(szPath);
		if (!xml_file.is_open())
		{
			return false;
		}
		string buffer((istreambuf_iterator<char>(xml_file)), istreambuf_iterator<char>());
		//buffer.push_back(0);
		if(buffer.size() == 0)
			return false;
		//如果是gb2312先换成utf-8
		string::size_type pos = buffer.find("encoding=\"GB2312\"");
		if(pos != string::npos)
		{
			buffer.replace(pos,strlen("encoding=\"GB2312\""),"encoding=\"utf-8\"");
			wstring wbuffer = ANSIToUnicode(buffer.data(),buffer.size());
			buffer = W2Utf8(wbuffer.data(),wbuffer.size());
			std::basic_ofstream<char, char_traits<char> > xml_file(szPath);
			if (!xml_file.is_open())
			{
				return false;
			}
			xml_file.write(buffer.data(),buffer.size());
		}
		//去掉utf8标识
		char szHead[]="\xEF\xBB\xBF\x00";
		if(buffer.size() > 3 && memcmp(&buffer[0],szHead,3) == 0)
			buffer.erase(0,3);
		return parse(&buffer[0]);
	}
	

	void save(LPCTSTR szPath)
	{
		save(szPath,Int2Type<(int)encode_type>());
	}

	void save(LPCTSTR szPath,Int2Type<(int)sh_xml_encode_type_gb2312>)
	{
		save_gb2312(szPath);
	}

	void save(LPCTSTR szPath,Int2Type<(int)sh_xml_encode_type_utf8>)
	{
		save_utf8(szPath);
	}

	bool parse(LPCSTR szXml,Int2Type<(int)sh_xml_encode_type_gb2312>)
	{
		return inneropen(GBKToUnicode(szXml).c_str());
	}

	bool parse(LPCSTR szXml,Int2Type<(int)sh_xml_encode_type_utf8>)
	{
		wstring strContent;
		int len = MultiByteToWideChar(CP_UTF8,0,szXml,strlen(szXml),NULL,0);
		strContent.resize(len);
		MultiByteToWideChar(CP_UTF8,0,szXml,strlen(szXml),(wchar_t*)strContent.c_str(),len);
		return inneropen(strContent.c_str());
	}

	bool parse(LPCSTR szXml)
	{
		try
		{
			return parse(szXml,Int2Type<(int)encode_type>());
		}
		catch (...)
		{
			return false;
		}
	}

	sh_xml_node *allocate_node(rapidxml::node_type type, LPCTSTR szName = NULL, LPCTSTR szValue = NULL)
	{
		LPCTSTR szAllocName  = szName ? __super::allocate_string(szName): NULL;
		LPCTSTR szAllocValue = szValue ? __super::allocate_string(szValue): NULL;
		return __super::allocate_node(type,szAllocName,szAllocValue);
	}
	//
	sh_xml_attribute *allocate_attribute(LPCTSTR szName = NULL, LPCTSTR szValue = NULL)
	{
		LPCTSTR szAllocName  = szName ? __super::allocate_string(szName): NULL;
		LPCTSTR szAllocValue = szValue ? __super::allocate_string(szValue): NULL;
		return __super::allocate_attribute(szAllocName,szAllocValue);
	}
	//
	sh_xml_attribute *allocate_attribute(LPCTSTR szName, long value)
	{
		ATL::CString strValue;
		strValue.Format(_T("%d"),value);
		LPCTSTR szAllocName  = szName ? __super::allocate_string(szName): NULL;
		LPCTSTR szAllocValue = __super::allocate_string(strValue);
		return __super::allocate_attribute(szAllocName,szAllocValue);
	}
	//
private:
	std::wstring m_strXml;
private:
	void save_gb2312(LPCTSTR szPath)
	{
		try
		{
			std::wostringstream stream;
			rapidxml::print((std::basic_ostream<wchar_t>&)stream, *this, 0);
			wstring buffer = stream.str();
			//
			string strContent;
			int len = WideCharToMultiByte(936,0,&buffer[0],buffer.size(),NULL,0,NULL,NULL);
			strContent.resize(len);
			WideCharToMultiByte(936,0,&buffer[0],buffer.size(),(char*)strContent.c_str(),len,NULL,NULL);
			//
			save(szPath,strContent.c_str());
		}
		catch (...)
		{
		}
	}

	void save_utf8(LPCTSTR szPath)
	{
		try
		{
			std::wostringstream stream;
			rapidxml::print((std::basic_ostream<wchar_t>&)stream, *this, 0);
			wstring buffer = stream.str();
			//
			string strContent;
			int len = WideCharToMultiByte(CP_UTF8,0,&buffer[0],buffer.size(),NULL,0,NULL,NULL);
			strContent.resize(len);
			WideCharToMultiByte(CP_UTF8,0,&buffer[0],buffer.size(),(char*)strContent.c_str(),len,NULL,NULL);
			//
			save(szPath,strContent.c_str());
		}
		catch (...)
		{
		}
	}
	bool inneropen(LPCTSTR szXml)
	{
		try
		{
			m_strXml = szXml;
			rapidxml::xml_document<wchar_t>::parse<rapidxml::parse_full>((LPTSTR)m_strXml.c_str());
			return true;
		}
		catch (...)
		{
			return false;
		}
	}
	void save(LPCTSTR szPath,LPCSTR szXml)
	{
		try
		{
			std::basic_ofstream<char, char_traits<char> > xml_file(szPath);
			if (!xml_file.is_open())
			{
				return;
			}
			xml_file<<szXml;
			xml_file.close();
		}
		catch (...)
		{
		}
	}

};