#ifndef _QTHEADERANALYZE_
#define _QTHEADERANALYZE_

#include "QtStructDef.h"

// CCustomMergeDlg 消息处理程序
class QtHeader
{
public:

	QtHeader() {}

	void Fill(_pbyte_t in_pbuffer, _dword_t in_nbuffersize);

	_uint64_t GetDataOffset() {return mdat.realoffset;}
	_uint32_t GetDataSize() {return mdat.realsize;}

	Qt_header_ftyp ftyp;
	Qt_header_moov moov;
	Qt_header_mdat mdat;
};

#endif
