#ifndef _QTSTRUCTDEF_
#define _QTSTRUCTDEF_

#include "Common.h"

#include <vector>
using namespace std;

struct Qt_common_atom
{
	Qt_common_atom()
	{
		dwFourcc = 0;
		dwSize = 0;
	}

	_dword_t	dwFourcc;
	_dword_t	dwSize;
};

struct Qt_version_atom : public Qt_common_atom
{
	Qt_version_atom() : Qt_common_atom()
	{
		dwFourcc = 0;
		dwSize = 0;
	}

	_uchar_t			ucVersion;
};

struct Qt_common_atom_info
{
	Qt_common_atom_info()
	{
		ullbufferoffset = 0;
		nbuffersize = 0;
	}

	_uint32_t	ullbufferoffset;
	_uint32_t	nbuffersize;
};

struct Qt_change_atom : public Qt_version_atom, public Qt_common_atom_info
{
	//部分改变内容，大小不变
};

struct Qt_copy_atom : public Qt_common_atom, public Qt_common_atom_info
{
	//不改变内容
};

struct Qt_addition_atom : public Qt_common_atom, public Qt_common_atom_info
{
	//追加内容
};

struct Qt_header_ftyp : public Qt_copy_atom
{
};

struct Qt_header_mdat : public Qt_addition_atom
{
	Qt_header_mdat() : Qt_addition_atom()
	{
		realoffset = 0;
		realsize = 0;
	}

	_uint32_t	realoffset;
	_uint32_t	realsize;
};

struct Qt_header_moov_mvhd : public Qt_change_atom
{
	Qt_header_moov_mvhd() : Qt_change_atom()
	{
		ui32TimeScale = 0;
		ui64Duration = 0;
		ui32TrakCountPlusOne = 0;
	}

	_uint32_t	ui32TimeScale;
	_uint64_t	ui64Duration;	
	_uint32_t	ui32TrakCountPlusOne;
};

struct Qt_header_moov_trak_tkhd : public Qt_change_atom
{
	Qt_header_moov_trak_tkhd() : Qt_change_atom()
	{
		ui32TrakID = 0;
		ui64Duration = 0;
	}

	_uint32_t	ui32TrakID;
	_uint64_t	ui64Duration;
};

struct Qt_header_moov_trak_edts_elst_cell
{
	Qt_header_moov_trak_edts_elst_cell()
	{
		ui64SegmentDuration = 0;
		ui64MediaTime = 0;
		ui32MediaRate = 0;
	}

	_uint64_t	ui64SegmentDuration;
	_uint64_t	ui64MediaTime;

	_uint32_t ui32MediaRate;
};

struct Qt_header_moov_trak_edts_elst : public Qt_version_atom
{
	Qt_header_moov_trak_edts_elst() : Qt_version_atom()
	{
		vElstAtomCell.clear();
	}

	std::vector<Qt_header_moov_trak_edts_elst_cell> vElstAtomCell;
};

struct Qt_header_moov_trak_edts : public Qt_common_atom
{
	Qt_header_moov_trak_edts_elst sElstAtom;
};

struct Qt_header_moov_trak_mdia_mdhd : public Qt_change_atom
{
	Qt_header_moov_trak_mdia_mdhd() : Qt_change_atom()
	{
		ui64Duration = 0;
	}

	_uint64_t	ui64Duration;
};

enum Qt_header_moov_trak_type
{
	keMoov_trak_Invalid = -1,
	keMoov_trak_video,
	keMoov_trak_audio,
	keMoov_trak_other
};

struct Qt_header_moov_trak_mdia_hdlr : public Qt_change_atom
{
	Qt_header_moov_trak_mdia_hdlr() : Qt_change_atom()
	{
		eMediaType = keMoov_trak_Invalid;
	}

	Qt_header_moov_trak_type eMediaType;
};

struct Qt_header_moov_trak_mdia_minf_xmhd : public Qt_copy_atom		//general structure vmhd, smhd
{
};

struct Qt_header_moov_trak_mdia_minf_dinf : public Qt_copy_atom
{
};

struct Qt_header_moov_trak_mdia_minf_stbl_stsd : public Qt_copy_atom
{
};

struct Qt_header_moov_trak_mdia_minf_stbl_stts_cell
{
	Qt_header_moov_trak_mdia_minf_stbl_stts_cell()
	{
		ui32SampleCount = 0;
		ui32SampleDelta = 0;
	}

	_uint32_t	ui32SampleCount;
	_uint32_t	ui32SampleDelta;
};

struct Qt_header_moov_trak_mdia_minf_stbl_stts : public Qt_version_atom
{
	Qt_header_moov_trak_mdia_minf_stbl_stts() : Qt_version_atom()
	{
		ui32TotalSampleCount = 0;
		vSttsAtomCell.clear();
	}

	_uint32_t ui32TotalSampleCount;
	vector<Qt_header_moov_trak_mdia_minf_stbl_stts_cell> vSttsAtomCell;
};

struct Qt_header_moov_trak_mdia_minf_stbl_stss : public Qt_version_atom //video only
{
	Qt_header_moov_trak_mdia_minf_stbl_stss() : Qt_version_atom()
	{
		vSampleNumber.clear();
		ui32SampleCount = 0;
	}

	vector<_uint32_t> vSampleNumber;
	_uint32_t	ui32SampleCount;
};

struct Qt_header_moov_trak_mdia_minf_stbl_stsc_cell
{
	Qt_header_moov_trak_mdia_minf_stbl_stsc_cell()
	{
		ui32FirstChunk = 0;
		ui32SamplePerChunk = 0;
		ui32SampleDescriptionIndex = 0;
	}

	_uint32_t	ui32FirstChunk;
	_uint32_t	ui32SamplePerChunk;
	_uint32_t	ui32SampleDescriptionIndex;
};

struct Qt_header_moov_trak_mdia_minf_stbl_stsc : public Qt_version_atom
{
	Qt_header_moov_trak_mdia_minf_stbl_stsc() : Qt_version_atom()
	{
		vStscAtomCell.clear();
		uiEntryCountStco = 0;
	}

	vector<Qt_header_moov_trak_mdia_minf_stbl_stsc_cell> vStscAtomCell;
	_uint32_t	uiEntryCountStco;
};

struct Qt_header_moov_trak_mdia_minf_stbl_stsz : public Qt_version_atom
{
	Qt_header_moov_trak_mdia_minf_stbl_stsz() : Qt_version_atom()
	{
		ui32SampleSize = 0;
		ui32SampleCount = 0;
		vSampleSize.clear();
	}

	_uint32_t	ui32SampleSize;
	_uint32_t	ui32SampleCount;
	vector<_uint32_t> vSampleSize;
};

struct Qt_header_moov_trak_mdia_minf_stbl_offset : public Qt_version_atom
{
	Qt_header_moov_trak_mdia_minf_stbl_offset() : Qt_version_atom()
	{
		ui32LastSampleChunkSize = 0;
		ui64DeltaOffset = 0;
	}

	_uint32_t	ui32LastSampleChunkSize;
	_uint64_t	ui64DeltaOffset;
};

struct Qt_header_moov_trak_mdia_minf_stbl_stco : public Qt_header_moov_trak_mdia_minf_stbl_offset	//2^32
{
	Qt_header_moov_trak_mdia_minf_stbl_stco() : Qt_header_moov_trak_mdia_minf_stbl_offset()
	{
		vSampleOffset.clear();
	}

	vector<_uint32_t> vSampleOffset;
};

struct Qt_header_moov_trak_mdia_minf_stbl_co64	: public Qt_header_moov_trak_mdia_minf_stbl_offset	//2^64
{
	Qt_header_moov_trak_mdia_minf_stbl_co64() : Qt_header_moov_trak_mdia_minf_stbl_offset()
	{
		vSampleOffset.clear();
	}

	vector<_uint64_t> vSampleOffset;
};

struct Qt_header_moov_trak_mdia_minf_stbl_ctts_cell
{
	Qt_header_moov_trak_mdia_minf_stbl_ctts_cell()
	{
		ui32SampleCount = 0;
		ui32SampleDelta = 0;
	}

	_uint32_t	ui32SampleCount;
	_uint32_t	ui32SampleDelta;
};

struct Qt_header_moov_trak_mdia_minf_stbl_ctts : public Qt_version_atom
{
	Qt_header_moov_trak_mdia_minf_stbl_ctts() : Qt_version_atom()
	{
		vCttsAtomCell.clear();
	}

	vector<Qt_header_moov_trak_mdia_minf_stbl_ctts_cell> vCttsAtomCell;
};

struct Qt_header_moov_trak_mdia_minf_stbl_sdtp : public Qt_addition_atom
{
};

struct Qt_header_moov_trak_mdia_minf_stbl : public Qt_common_atom
{
	Qt_header_moov_trak_mdia_minf_stbl_stsd		sStsdAtom;
	Qt_header_moov_trak_mdia_minf_stbl_stts		sSttsAtom;
	Qt_header_moov_trak_mdia_minf_stbl_stss		sStssAtom;
	Qt_header_moov_trak_mdia_minf_stbl_stsc		sStscAtom;
	Qt_header_moov_trak_mdia_minf_stbl_stsz		sStszAtom;
	Qt_header_moov_trak_mdia_minf_stbl_stco		sStcoAtom;
	Qt_header_moov_trak_mdia_minf_stbl_co64	sCo64Atom;
	Qt_header_moov_trak_mdia_minf_stbl_ctts		sCttsAtom;
	Qt_header_moov_trak_mdia_minf_stbl_sdtp		sSdtpAtom;
};

struct Qt_header_moov_trak_mdia_minf : public Qt_common_atom
{
	Qt_header_moov_trak_mdia_minf_xmhd	sXmhdAtom; 
	Qt_header_moov_trak_mdia_minf_dinf		sDinfAtom;
	Qt_header_moov_trak_mdia_minf_stbl		sStblAtom;
};

struct Qt_header_moov_trak_mdia : public Qt_common_atom
{
	Qt_header_moov_trak_mdia_mdhd		sMdhdAtom;
	Qt_header_moov_trak_mdia_hdlr		sHdlrAtom;
	Qt_header_moov_trak_mdia_minf		sMinfAtom;
};

struct Qt_header_moov_trak : public Qt_common_atom
{
	Qt_header_moov_trak_tkhd sTkhdAtom;
	Qt_header_moov_trak_edts sEdtsAtom;
	Qt_header_moov_trak_mdia sMdiaAtom;
	Qt_header_moov_trak_type eMediaType;
};

struct Qt_header_moov : public Qt_common_atom
{
	Qt_header_moov() : Qt_common_atom()
	{
		vTrakAtom.clear();
	}

	Qt_header_moov_mvhd sMvhdAtom;
	vector<Qt_header_moov_trak> vTrakAtom;
};

#define ATOM_SKIP																0x736b6970	//"skip" 
#define ATOM_FREE															0x66726565	//"free" 
#define ATOM_WIDE															0x77696465	//"wide" 
#define ATOM_APPL															0x6170706c  //"appl"

#define ATOM_FTYP															0x66747970	//"ftyp"
#define ATOM_MOOV															0x6D6F6F76	//"moov"
#define ATOM_MDAT															0x6D646174	//"mdat"

#define  ATOM_MOOV_MVHD											0x6D766864  //"mvhd"
#define  ATOM_MOOV_TRAK											0x7472616B  //"trak"
#define  ATOM_MOOV_TRAK_TKHD								0x746B6864	//"tkhd"
#define  ATOM_MOOV_TRAK_EDTS								0x65647473	//"edts"
#define  ATOM_MOOV_TRAK_EDTS_ELST					0x656C7374 //"elst"
#define  ATOM_MOOV_TRAK_TREF								0x74726566	//"tref"
#define  ATOM_MOOV_TRAK_MDIA								0x6D646961	//"mdia"
#define  ATOM_MOOV_TRAK_MDIA_MDHD				0x6D646864	//"mdhd"
#define  ATOM_MOOV_TRAK_MDIA_HDLR					0x68646C72	//"hdlr"
#define  ATOM_MOOV_TRAK_MDIA_MINF					0x6D696E66	//"minf"
#define ATOM_MOOV_TRAK_MDIA_MINF_VMHD	0x766D6864	//"vmhd"
#define ATOM_MOOV_TRAK_MDIA_MINF_SMHD    0x736D6864	//"smhd"
#define ATOM_MOOV_TRAK_MDIA_MINF_HMHD    0x686D6864	//"hmhd"
#define ATOM_MOOV_TRAK_MDIA_MINF_NMHD	0x6E6D6864	//"nmhd"
#define ATOM_MOOV_TRAK_MDIA_MINF_DINF		0x64696E66	//"dinf"
#define ATOM_MOOV_TRAK_MDIA_MINF_STBL		0x7374626C	//"stbl"

#define ATOM_MOOV_TRAK_MDIA_MINF_STBL_STSD		0x73747364	//"stsd"
#define ATOM_MOOV_TRAK_MDIA_MINF_STBL_STTS		0x73747473	//"stts"
#define ATOM_MOOV_TRAK_MDIA_MINF_STBL_STSS		0x73747373	//"stss"
#define ATOM_MOOV_TRAK_MDIA_MINF_STBL_STSC		0x73747363	//"stsc"
#define ATOM_MOOV_TRAK_MDIA_MINF_STBL_STCO		0x7374636F	//"stco"
#define ATOM_MOOV_TRAK_MDIA_MINF_STBL_CO64		0x636F3634	//"co64"
#define ATOM_MOOV_TRAK_MDIA_MINF_STBL_SDTP		0x73647470	//"sdtp"
#define ATOM_MOOV_TRAK_MDIA_MINF_STBL_CTTS		0x63747473	//"ctts"
#define ATOM_MOOV_TRAK_MDIA_MINF_STBL_STSH		0x73747368	//"stsh"
#define ATOM_MOOV_TRAK_MDIA_MINF_STBL_STSZ		0x7374737A //"stsz"

#endif