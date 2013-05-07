#ifndef _QTSEQUENCEMERGE_
#define _QTSEQUENCEMERGE_

#include "QtHeaderAnalyze.h"

struct SectionInfo
{
	_uint32_t ui32SectionInfoBufferSize;
	_pbyte_t pSectionInfoBuffer;
};

struct SectionDataInfo
{
	_uint32_t ui32DataOffset;
	_uint32_t ui32DataSize;

	_uint32_t ui32AbsoluteDataOffset;
};

struct ClipOfRangeInfo
{
	_uint32_t index;
	_uint32_t StartOffset;
	_uint32_t EndOffset;
};

struct SequenceInfo
{
	_uint32_t nSectionCount;
	SectionInfo* pSectioninfo;
};

struct TrakStcoInfo
{
	_pbyte_t pbufferptr;
	_uint32_t ui32cellcount;
};

class QtSequence
{
public:
	QtSequence(SequenceInfo* pinfo);
	~QtSequence() 
	{ 
		if(pdestinationbuffer != _null_t) 
			delete [] pdestinationbuffer;

		pdestinationbuffer = _null_t;

		if(pSectionHeader != _null_t) 
			delete [] pSectionHeader;

		pSectionHeader = _null_t;

		ui32destinationbuffersize = 0;
		ui32destinationfilesize = 0;

		ui32SectionCount = 0;

		vDataInfo.clear();
		vClipInfo.clear();
	}

	void getHeaderBuffer(_pbyte_t* out_ppBuffer, _uint32_t* out_pBufferSize)
	{
		if (out_ppBuffer)
		{
			*out_ppBuffer = pdestinationbuffer;
		}

		if (out_pBufferSize)
		{
			*out_pBufferSize = ui32destinationbuffersize;
		}
	}

	void getFileSize(_uint32_t* out_pFileSize)
	{
		if (out_pFileSize)
			*out_pFileSize = ui32destinationfilesize;
	}

	void TranslateSectionRange(_uint32_t in_startpos, _uint32_t in_endpos, _uint32_t* out_pstartclip, _uint32_t* out_pendclip)
	{
		if (in_endpos <= in_startpos)
			return;

		vClipInfo.clear();

		_uint32_t startclip = (_uint32_t)-1;
		_uint32_t endclip = 0;

		for (_uint32_t i = 0; i < vDataInfo.size(); i++)
		{
			ClipOfRangeInfo info; 
			info.index = i;
			info.StartOffset = 0;
			info.EndOffset = 0;

			if(in_endpos < vDataInfo[i].ui32AbsoluteDataOffset)
			{
			}
			else if(in_startpos < vDataInfo[i].ui32AbsoluteDataOffset && 
				in_endpos > vDataInfo[i].ui32AbsoluteDataOffset &&
				in_endpos <= vDataInfo[i].ui32AbsoluteDataOffset + vDataInfo[i].ui32DataSize)
			{
				endclip = max(i, endclip);

				info.StartOffset = vDataInfo[i].ui32DataOffset;
				info.EndOffset = in_endpos - vDataInfo[i].ui32AbsoluteDataOffset + vDataInfo[i].ui32DataOffset;
			}
			else if(in_startpos >= vDataInfo[i].ui32AbsoluteDataOffset &&
				in_startpos < vDataInfo[i].ui32AbsoluteDataOffset + vDataInfo[i].ui32DataSize && 
				in_endpos > vDataInfo[i].ui32AbsoluteDataOffset &&
				in_endpos <= vDataInfo[i].ui32AbsoluteDataOffset + vDataInfo[i].ui32DataSize)
			{
				startclip = min(i, startclip);
				endclip = max(i, endclip);

				info.StartOffset = in_startpos- vDataInfo[i].ui32AbsoluteDataOffset + vDataInfo[i].ui32DataOffset;
				info.EndOffset = in_endpos - vDataInfo[i].ui32AbsoluteDataOffset + vDataInfo[i].ui32DataOffset;
			}
			else if(in_startpos >= vDataInfo[i].ui32AbsoluteDataOffset &&
				in_startpos < vDataInfo[i].ui32AbsoluteDataOffset + vDataInfo[i].ui32DataSize &&
				in_endpos > vDataInfo[i].ui32AbsoluteDataOffset + vDataInfo[i].ui32DataSize)
			{
				startclip = min(i, startclip);

				info.StartOffset = in_startpos- vDataInfo[i].ui32AbsoluteDataOffset + vDataInfo[i].ui32DataOffset;
				info.EndOffset =  vDataInfo[i].ui32DataOffset + vDataInfo[i].ui32DataSize;
			}
			
			vClipInfo.push_back(info);
		}

		if(out_pstartclip)
			*out_pstartclip = startclip;

		if(out_pendclip)
			*out_pendclip = endclip;
	}

	void getSectionInfoByIndex(_uint32_t in_index, ClipOfRangeInfo* out_pInfo)
	{
		if (out_pInfo)
			*out_pInfo = vClipInfo[in_index];
	}

	//test only
	void getSectionDataInfoByIndex(_uint32_t in_index, SectionDataInfo* out_pInfo)
	{
		if (in_index >= vDataInfo.size())
			return;

		if (out_pInfo)
		{
			*out_pInfo = vDataInfo[in_index];
		}
	}

private:
	_pbyte_t pdestinationbuffer;
	_uint32_t ui32destinationbuffersize;
	_uint32_t ui32destinationfilesize;

	QtHeader* pSectionHeader;
	_uint32_t ui32SectionCount;

	vector<SectionDataInfo> vDataInfo;
	vector<ClipOfRangeInfo> vClipInfo;
};

#endif