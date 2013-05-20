#include "stdafx.h"
#include "QtHeaderAnalyze.h"
#include <assert.h>

_dword_t read_dword(_pbyte_t pbuffer)
{
	if (pbuffer == NULL)
		return -1;

	_dword_t value = 0;
	for (int i = 0; i < sizeof(_dword_t); i++)
	{
		_dword_t temp = *(pbuffer+i);
		value += temp<<((sizeof(_dword_t)-i-1) * 8);
	}

	return value;
}

_uint64_t read_ulonglong(_pbyte_t pbuffer)
{
	if (pbuffer == NULL)
		return -1;

	_uint64_t value = 0;
	for (int i = 0; i < sizeof(_uint64_t); i++)
	{
		_uint64_t temp = *(pbuffer+i);
		value += temp<<((sizeof(_uint64_t)-i-1) * 8);
	}

	return value;
}

Qt_common_atom read_atom(_pbyte_t pbuffer)
{
	Qt_common_atom atom;

	atom.dwSize = read_dword(pbuffer);
	atom.dwFourcc = read_dword(pbuffer + sizeof(_dword_t));

	return atom;
}

void QtHeader::Fill(_pbyte_t in_pbuffer, _dword_t in_nbuffersize)
{
	_pbyte_t pbuffer = in_pbuffer;

	Qt_common_atom atom;

	do 
	{
		atom = read_atom(pbuffer);
		switch (atom.dwFourcc)
		{
		case ATOM_FTYP:
			ftyp.dwFourcc = atom.dwFourcc;
			ftyp.dwSize = atom.dwSize;
			ftyp.ullbufferoffset = pbuffer - in_pbuffer;
			ftyp.nbuffersize = atom.dwSize;
			pbuffer += atom.dwSize;
			break;
		case ATOM_MDAT:
			{			
				mdat.dwFourcc = atom.dwFourcc;
				mdat.dwSize = atom.dwSize;
				mdat.ullbufferoffset = pbuffer - in_pbuffer;
				mdat.nbuffersize = atom.dwSize;
				mdat.realoffset = pbuffer + sizeof(Qt_common_atom) - in_pbuffer;
				mdat.realsize = atom.dwSize - sizeof(Qt_common_atom);
				pbuffer += atom.dwSize;
				if (!moov.vTrakAtom.empty())
				{
					_uint32_t ui32MinOffset = (_uint32_t)-1;
					_uint32_t ui32MaxOffset = 0;
					for (_uint32_t i = 0; i < moov.vTrakAtom.size(); i++)
					{
						Qt_header_moov_trak_mdia_minf_stbl sStblAtom = moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom;
						_uint32_t ui32StartSampleOffset = sStblAtom.sStcoAtom.vSampleOffset[0];
						ui32MinOffset = min(ui32MinOffset, ui32StartSampleOffset);

						_uint32_t ui32SamplePerChunk = sStblAtom.sStscAtom.vStscAtomCell[sStblAtom.sStscAtom.vStscAtomCell.size() - 1].ui32SamplePerChunk;
						_uint32_t ui32EndSampleOffset = sStblAtom.sStcoAtom.vSampleOffset[sStblAtom.sStcoAtom.vSampleOffset.size()-1];
						_uint32_t ui32EndSampleSize = 0;
						if (sStblAtom.sStszAtom.ui32SampleSize == 0)
						{
							ui32EndSampleSize = ui32EndSampleOffset;
							
							if (ui32SamplePerChunk > 0)
							{
								while (ui32SamplePerChunk--)
								{
									_uint32_t ui32SampleSize = sStblAtom.sStszAtom.vSampleSize[sStblAtom.sStszAtom.vSampleSize.size()-ui32SamplePerChunk-1];
									ui32EndSampleSize += ui32SampleSize;
								}
							}
						}
						else
						{
							ui32EndSampleSize = ui32EndSampleOffset + sStblAtom.sStszAtom.ui32SampleSize*ui32SamplePerChunk;
						}
						ui32MaxOffset = max(ui32MaxOffset, ui32EndSampleSize);
					}

					if (mdat.realoffset != ui32MinOffset || mdat.realsize != ui32MaxOffset - ui32MinOffset)
					{
						assert(false);
						mdat.realoffset = 0;
						mdat.realsize = 0;
					}
				}
			}
			break;
		case ATOM_MOOV:
			moov.dwFourcc = atom.dwFourcc;
			moov.dwSize = atom.dwSize;
			if (pbuffer - in_pbuffer + moov.dwSize > in_nbuffersize)
			{
				printf("buffer size %d analyzed is smaller than we least need %d!\n", in_nbuffersize, pbuffer - in_pbuffer + moov.dwSize);
				return;
			}
			pbuffer += sizeof(Qt_common_atom);
			break;
		case ATOM_MOOV_MVHD:
			{
				moov.sMvhdAtom.dwFourcc = atom.dwFourcc;
				moov.sMvhdAtom.dwSize = atom.dwSize;
				moov.sMvhdAtom.ullbufferoffset = pbuffer - in_pbuffer;
				moov.sMvhdAtom.nbuffersize = atom.dwSize;

				pbuffer += sizeof(Qt_common_atom);

				moov.sMvhdAtom.ucVersion = *pbuffer;

				pbuffer += sizeof(_uint32_t);

				if (moov.sMvhdAtom.ucVersion == 1)
				{
					pbuffer += sizeof(_uint64_t);
					pbuffer += sizeof(_uint64_t);
				}
				else
				{
					pbuffer += sizeof(_uint32_t);
					pbuffer += sizeof(_uint32_t);
				}

				moov.sMvhdAtom.ui32TimeScale = read_dword(pbuffer);
				pbuffer += sizeof(_uint32_t);

				if (moov.sMvhdAtom.ucVersion == 1)
				{
					moov.sMvhdAtom.ui64Duration = read_ulonglong(pbuffer);
					pbuffer += sizeof(_uint64_t);
				}
				else
				{
					moov.sMvhdAtom.ui64Duration = read_dword(pbuffer);
					pbuffer += sizeof(_uint32_t);
				}

				pbuffer += 76;

				moov.sMvhdAtom.ui32TrakCountPlusOne = read_dword(pbuffer);
				pbuffer += sizeof(_uint32_t);
			}
			break;
		case ATOM_MOOV_TRAK:
			{
				_pbyte_t ptrackbuffer = pbuffer + sizeof(Qt_common_atom);
				pbuffer += atom.dwSize;

				Qt_header_moov_trak trak;
				
				do 
				{
					atom = read_atom(ptrackbuffer);
					trak.dwFourcc = atom.dwFourcc;
					trak.dwSize = atom.dwSize;
					switch (atom.dwFourcc)
					{
					case ATOM_MOOV_TRAK_TKHD:
						{
							trak.sTkhdAtom.dwFourcc = atom.dwFourcc;
							trak.sTkhdAtom.dwSize = atom.dwSize;
							trak.sTkhdAtom.ullbufferoffset = ptrackbuffer - in_pbuffer;
							trak.sTkhdAtom.nbuffersize = atom.dwSize;

							_pbyte_t ptkhdbuffer = ptrackbuffer;

							trak.sTkhdAtom.dwFourcc = atom.dwFourcc;
							trak.sTkhdAtom.dwSize = atom.dwSize;

							ptrackbuffer += sizeof(Qt_common_atom);
							trak.sTkhdAtom.ucVersion = *ptrackbuffer;

							ptrackbuffer += sizeof(_uint32_t);

							if (trak.sTkhdAtom.ucVersion == 1)
							{
								ptrackbuffer += sizeof(_uint64_t);
								ptrackbuffer += sizeof(_uint64_t);
							}
							else
							{
								ptrackbuffer += sizeof(_uint32_t);
								ptrackbuffer += sizeof(_uint32_t);
							}

							trak.sTkhdAtom.ui32TrakID = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							ptrackbuffer += sizeof(_uint32_t);

							if (trak.sTkhdAtom.ucVersion == 1)
							{
								trak.sTkhdAtom.ui64Duration = read_ulonglong(ptrackbuffer);
								ptrackbuffer += sizeof(_uint64_t);
							}
							else
							{
								trak.sTkhdAtom.ui64Duration = read_dword(ptrackbuffer);
								ptrackbuffer += sizeof(_uint32_t);
							}

							//Skip left
							ptrackbuffer = ptkhdbuffer + atom.dwSize;
						}
						break;
					case ATOM_MOOV_TRAK_MDIA:
						{
							trak.sMdiaAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.dwSize = atom.dwSize;
							ptrackbuffer += sizeof(Qt_common_atom);
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MDHD:
						{
							trak.sMdiaAtom.sMdhdAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMdhdAtom.dwSize = atom.dwSize;
							trak.sMdiaAtom.sMdhdAtom.ullbufferoffset = ptrackbuffer - in_pbuffer;
							trak.sMdiaAtom.sMdhdAtom.nbuffersize = atom.dwSize;

							ptrackbuffer += sizeof(Qt_common_atom);

							trak.sMdiaAtom.sMdhdAtom.ucVersion = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							if (trak.sMdiaAtom.sMdhdAtom.ucVersion == 1)
							{
								ptrackbuffer += sizeof(_uint64_t);
								ptrackbuffer += sizeof(_uint64_t);
							}
							else
							{
								ptrackbuffer += sizeof(_uint32_t);
								ptrackbuffer += sizeof(_uint32_t);
							}

							ptrackbuffer += sizeof(_uint32_t);

							if (trak.sMdiaAtom.sMdhdAtom.ucVersion == 1)
							{
								trak.sMdiaAtom.sMdhdAtom.ui64Duration = read_ulonglong(ptrackbuffer);
								ptrackbuffer += sizeof(_uint64_t);
							}
							else
							{
								trak.sMdiaAtom.sMdhdAtom.ui64Duration = read_dword(ptrackbuffer);
								ptrackbuffer += sizeof(_uint32_t);
							}

							ptrackbuffer += sizeof(_uint32_t);
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_HDLR:
						{
							trak.sMdiaAtom.sHdlrAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sHdlrAtom.dwSize = atom.dwSize;
							trak.sMdiaAtom.sHdlrAtom.ullbufferoffset = ptrackbuffer - in_pbuffer;
							trak.sMdiaAtom.sHdlrAtom.nbuffersize = atom.dwSize;

							ptrackbuffer += sizeof(Qt_common_atom);

							trak.sMdiaAtom.sHdlrAtom.ucVersion = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							ptrackbuffer += sizeof(_uint32_t);

							_uint32_t ui32MediaType = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							if (ui32MediaType == 'vide')
							{
								trak.sMdiaAtom.sHdlrAtom.eMediaType = keMoov_trak_video;
							}
							else if (ui32MediaType == 'soun')
							{
								trak.sMdiaAtom.sHdlrAtom.eMediaType = keMoov_trak_audio;
							}
							else
							{
								trak.sMdiaAtom.sHdlrAtom.eMediaType = keMoov_trak_other;
							}

							ptrackbuffer += 12;

							unsigned int uiReaded = 8 + 24;

							if (atom.dwSize > uiReaded)
							{
								ptrackbuffer += atom.dwSize - uiReaded;
							}
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF:
						{
							trak.sMdiaAtom.sMinfAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.dwSize = atom.dwSize;
							ptrackbuffer += sizeof(Qt_common_atom);
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF_VMHD:
					case ATOM_MOOV_TRAK_MDIA_MINF_SMHD:
					case ATOM_MOOV_TRAK_MDIA_MINF_NMHD:
					case ATOM_MOOV_TRAK_MDIA_MINF_HMHD:
						{
							trak.sMdiaAtom.sMinfAtom.sXmhdAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.sXmhdAtom.dwSize = atom.dwSize;
							trak.sMdiaAtom.sMinfAtom.sXmhdAtom.ullbufferoffset = ptrackbuffer - in_pbuffer;
							trak.sMdiaAtom.sMinfAtom.sXmhdAtom.nbuffersize = atom.dwSize;
							ptrackbuffer += atom.dwSize;
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF_DINF:
						{
							trak.sMdiaAtom.sMinfAtom.sDinfAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.sDinfAtom.dwSize = atom.dwSize;
							trak.sMdiaAtom.sMinfAtom.sDinfAtom.ullbufferoffset = ptrackbuffer - in_pbuffer;
							trak.sMdiaAtom.sMinfAtom.sDinfAtom.nbuffersize = atom.dwSize;
							ptrackbuffer += atom.dwSize;
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF_STBL:
						{
							trak.sMdiaAtom.sMinfAtom.sStblAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.dwSize = atom.dwSize;
							ptrackbuffer += sizeof(Qt_common_atom);
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF_STBL_STSD:
						{
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStsdAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStsdAtom.dwSize = atom.dwSize;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStsdAtom.ullbufferoffset = ptrackbuffer - in_pbuffer;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStsdAtom.nbuffersize = atom.dwSize;
							ptrackbuffer += atom.dwSize;
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF_STBL_STTS:
						{
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sSttsAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sSttsAtom.dwSize = atom.dwSize;
							ptrackbuffer += sizeof(Qt_common_atom);

							trak.sMdiaAtom.sMinfAtom.sStblAtom.sSttsAtom.ucVersion = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							_uint32_t ui32EntryCount = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							for (unsigned int uiIdx = 0; uiIdx < ui32EntryCount; uiIdx++)
							{
								Qt_header_moov_trak_mdia_minf_stbl_stts_cell sCell;
								sCell.ui32SampleCount = read_dword(ptrackbuffer);
								ptrackbuffer += sizeof(_uint32_t);

								trak.sMdiaAtom.sMinfAtom.sStblAtom.sSttsAtom.ui32TotalSampleCount += sCell.ui32SampleCount;

								sCell.ui32SampleDelta = read_dword(ptrackbuffer);
								ptrackbuffer += sizeof(_uint32_t);

								trak.sMdiaAtom.sMinfAtom.sStblAtom.sSttsAtom.vSttsAtomCell.push_back(sCell);
							}
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF_STBL_STSS:
						{
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStssAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStssAtom.dwSize = atom.dwSize;
							ptrackbuffer += sizeof(Qt_common_atom);

							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStssAtom.ucVersion = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							_uint32_t ui32EntryCount = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							for (_uint32_t uiIdx = 0; uiIdx < ui32EntryCount; uiIdx++)
							{
								_uint32_t ui32SampleNumber = read_dword(ptrackbuffer);
								ptrackbuffer += sizeof(_uint32_t);

								trak.sMdiaAtom.sMinfAtom.sStblAtom.sStssAtom.vSampleNumber.push_back(ui32SampleNumber);
							}

							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStssAtom.ui32SampleCount =
								trak.sMdiaAtom.sMinfAtom.sStblAtom.sSttsAtom.ui32TotalSampleCount;						
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF_STBL_STSC:
						{
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStscAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStscAtom.dwSize = atom.dwSize;
							ptrackbuffer += sizeof(Qt_common_atom);

							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStscAtom.ucVersion = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							_uint32_t ui32EntryCount = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							for (_uint32_t uiIdx = 0; uiIdx < ui32EntryCount; uiIdx++)
							{
								Qt_header_moov_trak_mdia_minf_stbl_stsc_cell sCell;
								sCell.ui32FirstChunk = read_dword(ptrackbuffer);
								ptrackbuffer += sizeof(_uint32_t);
								sCell.ui32SamplePerChunk = read_dword(ptrackbuffer);
								ptrackbuffer += sizeof(_uint32_t);
								sCell.ui32SampleDescriptionIndex = read_dword(ptrackbuffer);
								ptrackbuffer += sizeof(_uint32_t);

								trak.sMdiaAtom.sMinfAtom.sStblAtom.sStscAtom.vStscAtomCell.push_back(sCell);
							}
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF_STBL_STCO:
						{
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom.dwSize = atom.dwSize;
							ptrackbuffer += sizeof(Qt_common_atom);

							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom.ucVersion = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							_uint32_t ui32EntryCount = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							for (_uint32_t uiIdx = 0; uiIdx < ui32EntryCount; uiIdx++)
							{
								_uint32_t uiCell = read_dword(ptrackbuffer);
								ptrackbuffer += sizeof(_uint32_t);

								trak.sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom.vSampleOffset.push_back(uiCell);
							}

							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom.ui32LastSampleChunkSize = 
								trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.vSampleSize.size() ?
								trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.vSampleSize[trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.vSampleSize.size() - 1] :
								trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.ui32SampleSize;
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF_STBL_STSZ:
						{
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.dwSize = atom.dwSize;
							ptrackbuffer += sizeof(Qt_common_atom);

							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.ucVersion = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.ui32SampleSize = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.ui32SampleCount = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							if (trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.ui32SampleSize == 0)
							{
								for (_uint32_t uiIdx = 0; uiIdx < trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.ui32SampleCount; uiIdx++)
								{
									_uint32_t ui32Cell = read_dword(ptrackbuffer);
									ptrackbuffer += sizeof(_uint32_t);

									trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.vSampleSize.push_back(ui32Cell);
								}
							}
							else
							{
								//trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.vSampleSize.push_back(uiSampleSize);
							}
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF_STBL_CTTS:
						{
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sCttsAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sCttsAtom.dwSize = atom.dwSize;

							_pbyte_t pcttsbuffer = ptrackbuffer;
							ptrackbuffer += atom.dwSize;

							pcttsbuffer += sizeof(Qt_common_atom);

							trak.sMdiaAtom.sMinfAtom.sStblAtom.sCttsAtom.ucVersion = read_dword(pcttsbuffer);
							pcttsbuffer += sizeof(_uint32_t);

							_uint32_t ui32EntryCount = read_dword(pcttsbuffer);
							pcttsbuffer += sizeof(_uint32_t);

							unsigned int uiSampleCount = 0;
							for (unsigned int uiIdx = 0; uiIdx < ui32EntryCount; uiIdx++)
							{
								Qt_header_moov_trak_mdia_minf_stbl_ctts_cell sCell;
								sCell.ui32SampleCount = read_dword(pcttsbuffer);
								pcttsbuffer += sizeof(_uint32_t);
								sCell.ui32SampleDelta = read_dword(pcttsbuffer);
								pcttsbuffer += sizeof(_uint32_t);

								uiSampleCount += sCell.ui32SampleCount;
								
								_uint32_t ui32TotalSampleCount = trak.sMdiaAtom.sMinfAtom.sStblAtom.sSttsAtom.ui32TotalSampleCount;
								if (uiSampleCount < ui32TotalSampleCount)
								{
									trak.sMdiaAtom.sMinfAtom.sStblAtom.sCttsAtom.vCttsAtomCell.push_back(sCell);
									continue;
								}

								if (uiSampleCount - ui32TotalSampleCount > 0 &&
									sCell.ui32SampleCount > uiSampleCount - ui32TotalSampleCount)
								{
									sCell.ui32SampleCount -= uiSampleCount - ui32TotalSampleCount;
								}

								trak.sMdiaAtom.sMinfAtom.sStblAtom.sCttsAtom.vCttsAtomCell.push_back(sCell);

								break;
							}
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF_STBL_SDTP:
						{
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sSdtpAtom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sSdtpAtom.dwSize = atom.dwSize;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sSdtpAtom.ullbufferoffset = ptrackbuffer - in_pbuffer;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sSdtpAtom.nbuffersize = atom.dwSize;
							ptrackbuffer += atom.dwSize;
						}
						break;
					case ATOM_MOOV_TRAK_MDIA_MINF_STBL_CO64:
						{
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sCo64Atom.dwFourcc = atom.dwFourcc;
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sCo64Atom.dwSize = atom.dwSize;
							ptrackbuffer += sizeof(Qt_common_atom);

							trak.sMdiaAtom.sMinfAtom.sStblAtom.sCo64Atom.ucVersion = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							_uint32_t ui32EntryCount = read_dword(ptrackbuffer);
							ptrackbuffer += sizeof(_uint32_t);

							for (_uint32_t uiIdx = 0; uiIdx < ui32EntryCount; uiIdx++)
							{
								_uint64_t ui64Cell = read_ulonglong(ptrackbuffer);
								ptrackbuffer += sizeof(_uint64_t);

								trak.sMdiaAtom.sMinfAtom.sStblAtom.sCo64Atom.vSampleOffset.push_back(ui64Cell);
							}

							trak.sMdiaAtom.sMinfAtom.sStblAtom.sCo64Atom.ui32LastSampleChunkSize = 
								trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.vSampleSize.size() ?
								trak.sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom.vSampleOffset[trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.vSampleSize.size() - 1] :
							trak.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.ui32SampleSize;						}
						break;
					default:
						ptrackbuffer += atom.dwSize;
						break;
					}
				} while (ptrackbuffer < pbuffer);

				trak.eMediaType = trak.sMdiaAtom.sHdlrAtom.eMediaType;
				moov.vTrakAtom.push_back(trak);
			}
			break;
		default:
			if(atom.dwSize)
			{
				//skip atom
				pbuffer += atom.dwSize;
			}
			else
			{
				//invalid data
				pbuffer = in_pbuffer + in_nbuffersize;
			}

			break;
		}

	} while (pbuffer < in_pbuffer + in_nbuffersize);

	if (!moov.vTrakAtom.empty() && mdat.realoffset == 0 && mdat.realsize == 0)
	{
		_uint32_t ui32MinOffset = (_uint32_t)-1;
		_uint32_t ui32MaxOffset = 0;
		for (_uint32_t i = 0; i < moov.vTrakAtom.size(); i++)
		{
			Qt_header_moov_trak_mdia_minf_stbl sStblAtom = moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom;
			_uint32_t ui32StartSampleOffset = sStblAtom.sStcoAtom.vSampleOffset[0];
			ui32MinOffset = min(ui32MinOffset, ui32StartSampleOffset);

			_uint32_t ui32SamplePerChunk = sStblAtom.sStscAtom.vStscAtomCell[sStblAtom.sStscAtom.vStscAtomCell.size() - 1].ui32SamplePerChunk;
			_uint32_t ui32EndSampleOffset = sStblAtom.sStcoAtom.vSampleOffset[sStblAtom.sStcoAtom.vSampleOffset.size()-1];
			_uint32_t ui32EndSampleSize = 0;
			if (sStblAtom.sStszAtom.ui32SampleSize == 0)
			{
				ui32EndSampleSize = ui32EndSampleOffset;
				
				if (ui32SamplePerChunk > 0)
				{
					while (ui32SamplePerChunk--)
					{
						_uint32_t ui32SampleSize = sStblAtom.sStszAtom.vSampleSize[sStblAtom.sStszAtom.vSampleSize.size()-ui32SamplePerChunk-1];
						ui32EndSampleSize += ui32SampleSize;
					}
				}
			}
			else
			{
				ui32EndSampleSize = ui32EndSampleOffset + sStblAtom.sStszAtom.ui32SampleSize*ui32SamplePerChunk;
			}
			ui32MaxOffset = max(ui32MaxOffset, ui32EndSampleSize);
		}

		mdat.realoffset = ui32MinOffset;
		mdat.realsize = ui32MaxOffset - ui32MinOffset;
	}
}