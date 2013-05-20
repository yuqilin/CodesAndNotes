#include "stdafx.h"
#include "QtSequenceMerge.h"
#include <assert.h>

 void write_dword(_pbyte_t pbuffer, _uint32_t dword)
{
	if (pbuffer == NULL)
		return;

	for (int i = 0; i < sizeof(_uint32_t); i++)
	{
		*(pbuffer + i) = (_byte_t)(dword>>((sizeof(_uint32_t)-i-1) * 8));
	}
}

void write_ulonglong(_pbyte_t pbuffer, _uint64_t ulonglong)
{
	if (pbuffer == NULL)
		return;

	for (int i = 0; i < sizeof(_uint64_t); i++)
	{
		*(pbuffer + i) = (_byte_t)(ulonglong>>((sizeof(_uint64_t)-i-1) * 8));
	}
}

void QtSequence::Fill(SequenceInfo* pinfo)
{
	if (pinfo == _null_t || pinfo->nSectionCount == 0)
		return;

	ui64TotalDuration = 0;
	ui32TimeScale = 0;

	ui32SectionCount = pinfo->nSectionCount;
	vDataInfo.clear();
	vClipInfo.clear();

	if (pinfo->nSectionCount == 1)
	{
		ui32destinationbuffersize = pinfo->pSectioninfo[0].ui32SectionInfoBufferSize;
		pdestinationbuffer = new _byte_t[ui32destinationbuffersize];
		if (pdestinationbuffer == NULL)
			return;

		memcpy(pdestinationbuffer, pinfo->pSectioninfo[0].pSectionInfoBuffer, ui32destinationbuffersize);

		QtHeader sHeader;
		sHeader.Fill(pdestinationbuffer, ui32destinationbuffersize);
		
		SectionDataInfo sInfo;
		sInfo.ui32DataOffset = sHeader.mdat.realoffset;
		sInfo.ui32DataSize = sHeader.mdat.realsize;
		sInfo.ui32AbsoluteDataOffset = sHeader.mdat.realoffset;

		vDataInfo.push_back(sInfo);

		ui32destinationfilesize = sInfo.ui32DataOffset + sInfo.ui32DataSize;
		ui32TimeScale = sHeader.moov.sMvhdAtom.ui32TimeScale;
		ui64TotalDuration = sHeader.moov.sMvhdAtom.ui64Duration;
	}
	else
	{
		vector<TrakStcoInfo> vTrakStcoInfo;

		vTrakStcoInfo.clear();
		_uint32_t ui32EstimateBufferSize = 0;

		pdestinationbuffer = _null_t;
		ui32destinationbuffersize = 0;

		for (_uint32_t i = 0; i < pinfo->nSectionCount; i++)
		{
			ui32EstimateBufferSize += pinfo->pSectioninfo[i].ui32SectionInfoBufferSize;
		}

		pdestinationbuffer = new _byte_t[ui32EstimateBufferSize];
		if (pdestinationbuffer == _null_t)
			return;

		pSectionHeader = _null_t;

		pSectionHeader = new QtHeader[pinfo->nSectionCount];
		if (pSectionHeader == _null_t)
			return;

		for (_uint32_t i = 0; i < pinfo->nSectionCount; i++)
		{
			pSectionHeader[i].Fill(pinfo->pSectioninfo[i].pSectionInfoBuffer, pinfo->pSectioninfo[i].ui32SectionInfoBufferSize);

			SectionDataInfo sInfo;
			sInfo.ui32DataOffset = pSectionHeader[i].mdat.realoffset;
			sInfo.ui32DataSize = pSectionHeader[i].mdat.realsize;

			vDataInfo.push_back(sInfo);

			//printf("Analyze Section %d: Track 0, first offset = %d\n", i, pSectionHeader[i].moov.vTrakAtom[0].sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom.vSampleOffset[0]);
			//printf("Analyze Section %d: Track 1, first offset = %d\n", i, pSectionHeader[i].moov.vTrakAtom[1].sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom.vSampleOffset[0]);
		}

		_pbyte_t pwritedestinationbuffer = pdestinationbuffer;

		//ftyp
		{
			_memory_copy(pwritedestinationbuffer, 
				pinfo->pSectioninfo[0].pSectionInfoBuffer + pSectionHeader[0].ftyp.ullbufferoffset, 
				pSectionHeader[0].ftyp.dwSize);

			pwritedestinationbuffer += pSectionHeader[0].ftyp.dwSize;
			ui32destinationbuffersize += pSectionHeader[0].ftyp.dwSize;
		}

		//moov
		{
			_pbyte_t preservedatomheaderbuffer_moov = pwritedestinationbuffer;
			_uint32_t ui32reservedatomheaderbuffersize_moov = 0;

			//skip reserved moov atom
			pwritedestinationbuffer += sizeof(Qt_common_atom);
			ui32destinationbuffersize += sizeof(Qt_common_atom);
			ui32reservedatomheaderbuffersize_moov += sizeof(Qt_common_atom);

			//mvhd
			{
				_memory_copy(pwritedestinationbuffer, 
					pinfo->pSectioninfo[0].pSectionInfoBuffer + pSectionHeader[0].moov.sMvhdAtom.ullbufferoffset, 
					pSectionHeader[0].moov.sMvhdAtom.nbuffersize);

				//fix mvhd
				ui32TimeScale = pSectionHeader[0].moov.sMvhdAtom.ui32TimeScale;
				for (_uint32_t i = 0; i < pinfo->nSectionCount; i++)
				{
					ui64TotalDuration += pSectionHeader[i].moov.sMvhdAtom.ui64Duration;
				}

				_uint32_t ui32Offset = sizeof(Qt_common_atom);
				_uchar_t ucVersion = *(pwritedestinationbuffer + ui32Offset);
				ui32Offset += sizeof(_uint32_t);

				if (ucVersion == 1)
				{
					ui32Offset += sizeof(_uint64_t);
					ui32Offset += sizeof(_uint64_t);
				}
				else
				{
					ui32Offset += sizeof(_uint32_t);
					ui32Offset += sizeof(_uint32_t);
				}

				ui32Offset += sizeof(_uint32_t);

				if (ucVersion == 1)
				{
					write_ulonglong(pwritedestinationbuffer + ui32Offset, ui64TotalDuration);
					ui32Offset += sizeof(_uint64_t);
				}
				else
				{
					write_dword(pwritedestinationbuffer + ui32Offset, (_uint32_t)ui64TotalDuration);
					ui32Offset += sizeof(_uint32_t);
				}

				pwritedestinationbuffer += pSectionHeader[0].moov.sMvhdAtom.nbuffersize;
				ui32destinationbuffersize += pSectionHeader[0].moov.sMvhdAtom.nbuffersize;
				ui32reservedatomheaderbuffersize_moov += pSectionHeader[0].moov.sMvhdAtom.nbuffersize;
			}

			//trak list
			for (_uint32_t i = 0; i < pSectionHeader[0].moov.vTrakAtom.size(); i++)
			{
				Qt_header_moov_trak sTrakAtom = pSectionHeader[0].moov.vTrakAtom[i];

				_pbyte_t preservedatomheaderbuffer_trak = pwritedestinationbuffer;
				_uint32_t ui32reservedatomheaderbuffersize_trak = 0;

				//skip reserved trak atom
				pwritedestinationbuffer += sizeof(Qt_common_atom);
				ui32destinationbuffersize += sizeof(Qt_common_atom);
				ui32reservedatomheaderbuffersize_trak += sizeof(Qt_common_atom);

				//tkhd
				{
					_memory_copy(pwritedestinationbuffer, 
						pinfo->pSectioninfo[0].pSectionInfoBuffer + sTrakAtom.sTkhdAtom.ullbufferoffset, 
						sTrakAtom.sTkhdAtom.nbuffersize);

					_uint64_t ui64Duration = 0;
					for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
					{
						ui64Duration += pSectionHeader[j].moov.vTrakAtom[i].sTkhdAtom.ui64Duration;
					}

					_uint32_t ui32Offset = sizeof(Qt_common_atom);

					_uchar_t ucVersion = pwritedestinationbuffer[ui32Offset];
					ui32Offset += sizeof(_uint32_t);

					if (ucVersion == 1)
					{
						ui32Offset += sizeof(_uint64_t);
						ui32Offset += sizeof(_uint64_t);
					}
					else
					{
						ui32Offset += sizeof(_uint32_t);
						ui32Offset += sizeof(_uint32_t);
					}

					ui32Offset += sizeof(_uint32_t);
					ui32Offset += sizeof(_uint32_t);

					if (ucVersion == 1)
					{
						write_ulonglong(pwritedestinationbuffer + ui32Offset, ui64Duration);
						ui32Offset += sizeof(_uint64_t);
					}
					else
					{
						write_dword(pwritedestinationbuffer + ui32Offset, (_uint32_t)ui64Duration);
						ui32Offset += sizeof(_uint32_t);
					}

					pwritedestinationbuffer += sTrakAtom.sTkhdAtom.nbuffersize;
					ui32destinationbuffersize += sTrakAtom.sTkhdAtom.nbuffersize;
					ui32reservedatomheaderbuffersize_trak += sTrakAtom.sTkhdAtom.nbuffersize;
				}

				//mdia
				{
					_pbyte_t preservedatomheaderbuffer_mdia = pwritedestinationbuffer;
					_uint32_t ui32reservedatomheaderbuffersize_mdia = 0;

					//skip reserved mdia atom
					pwritedestinationbuffer += sizeof(Qt_common_atom);
					ui32destinationbuffersize += sizeof(Qt_common_atom);
					ui32reservedatomheaderbuffersize_mdia += sizeof(Qt_common_atom);

					//mdhd
					{
						_memory_copy(pwritedestinationbuffer, 
							pinfo->pSectioninfo[0].pSectionInfoBuffer + sTrakAtom.sMdiaAtom.sMdhdAtom.ullbufferoffset, 
							sTrakAtom.sMdiaAtom.sMdhdAtom.nbuffersize);

						//fix mdhd
						_uint64_t ui64Duration = 0;
						for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
						{
							ui64Duration += pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMdhdAtom.ui64Duration;
						}

						_uint32_t ui32Offset = sizeof(Qt_common_atom);

						_uchar_t ucVersion = pwritedestinationbuffer[ui32Offset];
						ui32Offset += sizeof(_uint32_t);

						if (ucVersion == 1)
						{
							ui32Offset += sizeof(_uint64_t);
							ui32Offset += sizeof(_uint64_t);
						}
						else
						{
							ui32Offset += sizeof(_uint32_t);
							ui32Offset += sizeof(_uint32_t);
						}

						ui32Offset += sizeof(_uint32_t);

						if (ucVersion == 1)
						{
							write_ulonglong(pwritedestinationbuffer + ui32Offset, ui64Duration);
							ui32Offset += sizeof(_uint64_t);
						}
						else
						{
							write_dword(pwritedestinationbuffer + ui32Offset, (_uint32_t)ui64Duration);
							ui32Offset += sizeof(_uint32_t);
						}

						pwritedestinationbuffer += sTrakAtom.sMdiaAtom.sMdhdAtom.nbuffersize;
						ui32destinationbuffersize += sTrakAtom.sMdiaAtom.sMdhdAtom.nbuffersize;
						ui32reservedatomheaderbuffersize_mdia += sTrakAtom.sMdiaAtom.sMdhdAtom.nbuffersize;
					}

					//hdlr
					{
						_memory_copy(pwritedestinationbuffer, 
							pinfo->pSectioninfo[0].pSectionInfoBuffer + sTrakAtom.sMdiaAtom.sHdlrAtom.ullbufferoffset, 
							sTrakAtom.sMdiaAtom.sHdlrAtom.nbuffersize);

						//no need fix hdlr, all the same

						pwritedestinationbuffer += sTrakAtom.sMdiaAtom.sHdlrAtom.nbuffersize;
						ui32destinationbuffersize += sTrakAtom.sMdiaAtom.sHdlrAtom.nbuffersize;
						ui32reservedatomheaderbuffersize_mdia += sTrakAtom.sMdiaAtom.sHdlrAtom.nbuffersize;
					}

					//minf
					{
						_pbyte_t preservedatomheaderbuffer_minf = pwritedestinationbuffer;
						_uint32_t ui32reservedatomheaderbuffersize_minf = 0;

						//skip reserved minf atom
						pwritedestinationbuffer += sizeof(Qt_common_atom);
						ui32destinationbuffersize += sizeof(Qt_common_atom);
						ui32reservedatomheaderbuffersize_minf += sizeof(Qt_common_atom);

						//vmhd
						{
							_memory_copy(pwritedestinationbuffer, 
								pinfo->pSectioninfo[0].pSectionInfoBuffer + sTrakAtom.sMdiaAtom.sMinfAtom.sXmhdAtom.ullbufferoffset, 
								sTrakAtom.sMdiaAtom.sMinfAtom.sXmhdAtom.nbuffersize);

							//no need fix vmhd, all the same

							pwritedestinationbuffer += sTrakAtom.sMdiaAtom.sMinfAtom.sXmhdAtom.nbuffersize;
							ui32destinationbuffersize += sTrakAtom.sMdiaAtom.sMinfAtom.sXmhdAtom.nbuffersize;
							ui32reservedatomheaderbuffersize_minf += sTrakAtom.sMdiaAtom.sMinfAtom.sXmhdAtom.nbuffersize;
						}

						//dinf
						if (sTrakAtom.sMdiaAtom.sMinfAtom.sDinfAtom.nbuffersize > 0)
						{
							_memory_copy(pwritedestinationbuffer, 
								pinfo->pSectioninfo[0].pSectionInfoBuffer + sTrakAtom.sMdiaAtom.sMinfAtom.sDinfAtom.ullbufferoffset, 
								sTrakAtom.sMdiaAtom.sMinfAtom.sDinfAtom.nbuffersize);

							//no need fix dinf, all the same

							pwritedestinationbuffer += sTrakAtom.sMdiaAtom.sMinfAtom.sDinfAtom.nbuffersize;
							ui32destinationbuffersize += sTrakAtom.sMdiaAtom.sMinfAtom.sDinfAtom.nbuffersize;
							ui32reservedatomheaderbuffersize_minf += sTrakAtom.sMdiaAtom.sMinfAtom.sDinfAtom.nbuffersize;
						}

						//stbl
						{
							_pbyte_t preservedatomheaderbuffer_stbl = pwritedestinationbuffer;
							_uint32_t ui32reservedatomheaderbuffersize_stbl = 0;

							//skip reserved stbl atom
							pwritedestinationbuffer += sizeof(Qt_common_atom);
							ui32destinationbuffersize += sizeof(Qt_common_atom);
							ui32reservedatomheaderbuffersize_stbl += sizeof(Qt_common_atom);

							//stsd
							if (sTrakAtom.sMdiaAtom.sMinfAtom.sStblAtom.sStsdAtom.nbuffersize > 0)
							{
								_memory_copy(pwritedestinationbuffer, 
									pinfo->pSectioninfo[0].pSectionInfoBuffer + sTrakAtom.sMdiaAtom.sMinfAtom.sStblAtom.sStsdAtom.ullbufferoffset, 
									sTrakAtom.sMdiaAtom.sMinfAtom.sStblAtom.sStsdAtom.nbuffersize);

								//no need fix stsd, all the same

								pwritedestinationbuffer += sTrakAtom.sMdiaAtom.sMinfAtom.sStblAtom.sStsdAtom.nbuffersize;
								ui32destinationbuffersize += sTrakAtom.sMdiaAtom.sMinfAtom.sStblAtom.sStsdAtom.nbuffersize;
								ui32reservedatomheaderbuffersize_stbl += sTrakAtom.sMdiaAtom.sMinfAtom.sStblAtom.sStsdAtom.nbuffersize;
							}

							//stts
							if (!sTrakAtom.sMdiaAtom.sMinfAtom.sStblAtom.sSttsAtom.vSttsAtomCell.empty())
							{
								_pbyte_t preservedatomheaderbuffer = pwritedestinationbuffer;
								_uint32_t ui32atombuffersize = 0;

								//skip reserved stts atom
								pwritedestinationbuffer += sizeof(Qt_common_atom);
								ui32destinationbuffersize += sizeof(Qt_common_atom);
								ui32atombuffersize += sizeof(Qt_common_atom);

								vector<Qt_header_moov_trak_mdia_minf_stbl_stts_cell> vSttsAtomCell;
								for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
								{
									Qt_header_moov_trak_mdia_minf_stbl_stts sSttsAtom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sSttsAtom;

									for (_uint32_t k = 0; k < sSttsAtom.vSttsAtomCell.size(); k++)
									{
										if (!vSttsAtomCell.empty() &&
											vSttsAtomCell[vSttsAtomCell.size() - 1].ui32SampleDelta == sSttsAtom.vSttsAtomCell[0].ui32SampleDelta)
										{
											vSttsAtomCell[vSttsAtomCell.size() - 1].ui32SampleCount += sSttsAtom.vSttsAtomCell[0].ui32SampleCount;
										}
										else
										{
											vSttsAtomCell.push_back(sSttsAtom.vSttsAtomCell[k]);
										}
									}
								}

								write_dword(pwritedestinationbuffer, (_uint32_t)pSectionHeader[0].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sSttsAtom.ucVersion);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								_uint32_t ui32EntryCount = (_uint32_t)vSttsAtomCell.size();
								write_dword(pwritedestinationbuffer, ui32EntryCount);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								for (_uint32_t uiIdx = 0; uiIdx < vSttsAtomCell.size(); uiIdx++)
								{
									write_dword(pwritedestinationbuffer, vSttsAtomCell[uiIdx].ui32SampleCount);
									pwritedestinationbuffer += sizeof(_uint32_t);
									ui32destinationbuffersize += sizeof(_uint32_t);
									ui32atombuffersize += sizeof(_uint32_t);

									write_dword(pwritedestinationbuffer, vSttsAtomCell[uiIdx].ui32SampleDelta);
									pwritedestinationbuffer += sizeof(_uint32_t);
									ui32destinationbuffersize += sizeof(_uint32_t);
									ui32atombuffersize += sizeof(_uint32_t);
								}

								//reset reserved stts atom
								write_dword(preservedatomheaderbuffer, ui32atombuffersize);
								write_dword(preservedatomheaderbuffer + sizeof(_uint32_t), ATOM_MOOV_TRAK_MDIA_MINF_STBL_STTS);
								ui32reservedatomheaderbuffersize_stbl += ui32atombuffersize;
							}

							//ctts
							if (!sTrakAtom.sMdiaAtom.sMinfAtom.sStblAtom.sCttsAtom.vCttsAtomCell.empty())
							{
								_pbyte_t preservedatomheaderbuffer = pwritedestinationbuffer;
								_uint32_t ui32atombuffersize = 0;

								//skip reserved stss atom
								pwritedestinationbuffer += sizeof(Qt_common_atom);
								ui32destinationbuffersize += sizeof(Qt_common_atom);
								ui32atombuffersize += sizeof(Qt_common_atom);

								write_dword(pwritedestinationbuffer, (_uint32_t)pSectionHeader[0].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sCttsAtom.ucVersion);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								_uint32_t ui32EntryCount = 0;
								for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
								{
									Qt_header_moov_trak_mdia_minf_stbl_ctts sCttsAtom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sCttsAtom;
									ui32EntryCount += sCttsAtom.vCttsAtomCell.size();
								}

								write_dword(pwritedestinationbuffer, ui32EntryCount);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
								{
									Qt_header_moov_trak_mdia_minf_stbl_ctts sCttsAtom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sCttsAtom;
									for (_uint32_t k = 0; k < sCttsAtom.vCttsAtomCell.size(); k++)
									{
										write_dword(pwritedestinationbuffer, sCttsAtom.vCttsAtomCell[k].ui32SampleCount);
										pwritedestinationbuffer += sizeof(_uint32_t);
										ui32destinationbuffersize += sizeof(_uint32_t);
										ui32atombuffersize += sizeof(_uint32_t);

										write_dword(pwritedestinationbuffer, sCttsAtom.vCttsAtomCell[k].ui32SampleDelta);
										pwritedestinationbuffer += sizeof(_uint32_t);
										ui32destinationbuffersize += sizeof(_uint32_t);
										ui32atombuffersize += sizeof(_uint32_t);
									}
								}

								//reset reserved ctts atom
								write_dword(preservedatomheaderbuffer, ui32atombuffersize);
								write_dword(preservedatomheaderbuffer + sizeof(_uint32_t), ATOM_MOOV_TRAK_MDIA_MINF_STBL_CTTS);
								ui32reservedatomheaderbuffersize_stbl += ui32atombuffersize;
							}

							//stss
							if (!sTrakAtom.sMdiaAtom.sMinfAtom.sStblAtom.sStssAtom.vSampleNumber.empty())
							{
								_pbyte_t preservedatomheaderbuffer = pwritedestinationbuffer;
								_uint32_t ui32atombuffersize = 0;

								//skip reserved stss atom
								pwritedestinationbuffer += sizeof(Qt_common_atom);
								ui32destinationbuffersize += sizeof(Qt_common_atom);
								ui32atombuffersize += sizeof(Qt_common_atom);

								write_dword(pwritedestinationbuffer, (_uint32_t)pSectionHeader[0].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStssAtom.ucVersion);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								_uint32_t ui32EntryCount = 0;
								for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
								{
									Qt_header_moov_trak_mdia_minf_stbl_stss sStssAtom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStssAtom;
									ui32EntryCount += sStssAtom.vSampleNumber.size();
								}

								write_dword(pwritedestinationbuffer, ui32EntryCount);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								_uint32_t ui32LastSampleNumber = 0;
								for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
								{
									Qt_header_moov_trak_mdia_minf_stbl_stss sStssAtom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStssAtom;

									for (_uint32_t k = 0; k < sStssAtom.vSampleNumber.size(); k++)
									{
										write_dword(pwritedestinationbuffer, sStssAtom.vSampleNumber[k] + ui32LastSampleNumber);
										pwritedestinationbuffer += sizeof(_uint32_t);
										ui32destinationbuffersize += sizeof(_uint32_t);
										ui32atombuffersize += sizeof(_uint32_t);
									}

									Qt_header_moov_trak_mdia_minf_stbl_stts sSttsAtom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sSttsAtom;
									ui32LastSampleNumber += sSttsAtom.ui32TotalSampleCount;
								}

								//reset reserved stss atom
								write_dword(preservedatomheaderbuffer, ui32atombuffersize);
								write_dword(preservedatomheaderbuffer + sizeof(_uint32_t), ATOM_MOOV_TRAK_MDIA_MINF_STBL_STSS);
								ui32reservedatomheaderbuffersize_stbl += ui32atombuffersize;
							}

							//stsc
							if (!sTrakAtom.sMdiaAtom.sMinfAtom.sStblAtom.sStscAtom.vStscAtomCell.empty())
							{
								_pbyte_t preservedatomheaderbuffer = pwritedestinationbuffer;
								_uint32_t ui32atombuffersize = 0;

								//skip reserved stss atom
								pwritedestinationbuffer += sizeof(Qt_common_atom);
								ui32destinationbuffersize += sizeof(Qt_common_atom);
								ui32atombuffersize += sizeof(Qt_common_atom);

								write_dword(pwritedestinationbuffer, (_uint32_t)pSectionHeader[0].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStscAtom.ucVersion);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								_uint32_t ui32EntryCount = 0;
								for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
								{
									Qt_header_moov_trak_mdia_minf_stbl_stsc sStscAtom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStscAtom;
									ui32EntryCount += sStscAtom.vStscAtomCell.size();
								}

								write_dword(pwritedestinationbuffer, ui32EntryCount);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								_uint32_t ui32LastFirstChunk = 0;
								for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
								{
									Qt_header_moov_trak_mdia_minf_stbl_stsc sStscAtom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStscAtom;

									for (_uint32_t k = 0; k < sStscAtom.vStscAtomCell.size(); k++)
									{
										write_dword(pwritedestinationbuffer, sStscAtom.vStscAtomCell[k].ui32FirstChunk + ui32LastFirstChunk);
										pwritedestinationbuffer += sizeof(_uint32_t);
										ui32destinationbuffersize += sizeof(_uint32_t);
										ui32atombuffersize += sizeof(_uint32_t);

										write_dword(pwritedestinationbuffer, sStscAtom.vStscAtomCell[k].ui32SamplePerChunk);
										pwritedestinationbuffer += sizeof(_uint32_t);
										ui32destinationbuffersize += sizeof(_uint32_t);
										ui32atombuffersize += sizeof(_uint32_t);

										write_dword(pwritedestinationbuffer, sStscAtom.vStscAtomCell[k].ui32SampleDescriptionIndex);
										pwritedestinationbuffer += sizeof(_uint32_t);
										ui32destinationbuffersize += sizeof(_uint32_t);
										ui32atombuffersize += sizeof(_uint32_t);
									}

									Qt_header_moov_trak_mdia_minf_stbl_stco sStcoAtom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom;
									ui32LastFirstChunk += sStcoAtom.vSampleOffset.size();
								}

								//reset reserved stsc atom
								write_dword(preservedatomheaderbuffer, ui32atombuffersize);
								write_dword(preservedatomheaderbuffer + sizeof(_uint32_t), ATOM_MOOV_TRAK_MDIA_MINF_STBL_STSC);
								ui32reservedatomheaderbuffersize_stbl += ui32atombuffersize;
							}

							//stsz
							if (!sTrakAtom.sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.vSampleSize.empty())
							{
								_pbyte_t preservedatomheaderbuffer = pwritedestinationbuffer;
								_uint32_t ui32atombuffersize = 0;

								//skip reserved stss atom
								pwritedestinationbuffer += sizeof(Qt_common_atom);
								ui32destinationbuffersize += sizeof(Qt_common_atom);
								ui32atombuffersize += sizeof(Qt_common_atom);

								write_dword(pwritedestinationbuffer, (_uint32_t)pSectionHeader[0].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.ucVersion);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								write_dword(pwritedestinationbuffer, pSectionHeader[0].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.ui32SampleSize);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								_uint32_t ui32TotalSampleCount = 0;
								for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
								{
									ui32TotalSampleCount += pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.ui32SampleCount;
								}

								write_dword(pwritedestinationbuffer, ui32TotalSampleCount);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								if (pSectionHeader[0].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.ui32SampleSize == 0)
								{
									for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
									{
										Qt_header_moov_trak_mdia_minf_stbl_stsz sStszAtom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom;

										for (_uint32_t k = 0; k < sStszAtom.vSampleSize.size(); k++)
										{
											write_dword(pwritedestinationbuffer, sStszAtom.vSampleSize[k]);
											pwritedestinationbuffer += sizeof(_uint32_t);
											ui32destinationbuffersize += sizeof(_uint32_t);
											ui32atombuffersize += sizeof(_uint32_t);
										}
									}
								}
								else
								{
									if (!pSectionHeader[0].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.vSampleSize.empty())
									{
										write_dword(pwritedestinationbuffer, pSectionHeader[0].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStszAtom.vSampleSize[0]);
										pwritedestinationbuffer += sizeof(_uint32_t);
										ui32destinationbuffersize += sizeof(_uint32_t);
										ui32atombuffersize += sizeof(_uint32_t);
									}
								}

								//reset reserved stsz atom
								write_dword(preservedatomheaderbuffer, ui32atombuffersize);
								write_dword(preservedatomheaderbuffer + sizeof(_uint32_t), ATOM_MOOV_TRAK_MDIA_MINF_STBL_STSZ);
								ui32reservedatomheaderbuffersize_stbl += ui32atombuffersize;
							}

							//stco
							if (!sTrakAtom.sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom.vSampleOffset.empty())
							{
								_pbyte_t preservedatomheaderbuffer = pwritedestinationbuffer;
								_uint32_t ui32atombuffersize = 0;

								//skip reserved stss atom
								pwritedestinationbuffer += sizeof(Qt_common_atom);
								ui32destinationbuffersize += sizeof(Qt_common_atom);
								ui32atombuffersize += sizeof(Qt_common_atom);

								write_dword(pwritedestinationbuffer, (_uint32_t)pSectionHeader[0].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom.ucVersion);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								_uint32_t ui32EntryCount = 0;
								for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
								{
									Qt_header_moov_trak_mdia_minf_stbl_stco sStcoAtom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom;
									ui32EntryCount += sStcoAtom.vSampleOffset.size();
								}

								write_dword(pwritedestinationbuffer, ui32EntryCount);
								pwritedestinationbuffer += sizeof(_uint32_t);
								ui32destinationbuffersize += sizeof(_uint32_t);
								ui32atombuffersize += sizeof(_uint32_t);

								TrakStcoInfo sInfo;
								sInfo.pbufferptr = pwritedestinationbuffer;

								_uint32_t ui32StcoCellCount = 0;
								for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
								{
									Qt_header_moov_trak_mdia_minf_stbl_stco sStcoAtom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom;

									ui32StcoCellCount += sStcoAtom.vSampleOffset.size();
									for (_uint32_t k = 0; k < sStcoAtom.vSampleOffset.size(); k++)
									{
										//write_dword(pwritedestinationbuffer, sStcoAtom.vSampleOffset[k]);
										pwritedestinationbuffer += sizeof(_uint32_t);
										ui32destinationbuffersize += sizeof(_uint32_t);
										ui32atombuffersize += sizeof(_uint32_t);
									}	
								}

								sInfo.ui32cellcount = ui32StcoCellCount;
								vTrakStcoInfo.push_back(sInfo);

								//reset reserved stco atom
								write_dword(preservedatomheaderbuffer, ui32atombuffersize);
								write_dword(preservedatomheaderbuffer + sizeof(_uint32_t), ATOM_MOOV_TRAK_MDIA_MINF_STBL_STCO);
								ui32reservedatomheaderbuffersize_stbl += ui32atombuffersize;
							}			

							//reset reserved stbl atom
							write_dword(preservedatomheaderbuffer_stbl, ui32reservedatomheaderbuffersize_stbl);
							write_dword(preservedatomheaderbuffer_stbl + sizeof(_uint32_t), ATOM_MOOV_TRAK_MDIA_MINF_STBL);
							ui32reservedatomheaderbuffersize_minf += ui32reservedatomheaderbuffersize_stbl;
						}

						//reset reserved minf atom
						write_dword(preservedatomheaderbuffer_minf, ui32reservedatomheaderbuffersize_minf);
						write_dword(preservedatomheaderbuffer_minf + sizeof(_uint32_t), ATOM_MOOV_TRAK_MDIA_MINF);
						ui32reservedatomheaderbuffersize_mdia += ui32reservedatomheaderbuffersize_minf;
					}

					//reset reserved mdia atom
					write_dword(preservedatomheaderbuffer_mdia, ui32reservedatomheaderbuffersize_mdia);
					write_dword(preservedatomheaderbuffer_mdia + sizeof(_uint32_t), ATOM_MOOV_TRAK_MDIA);
					ui32reservedatomheaderbuffersize_trak += ui32reservedatomheaderbuffersize_mdia;
				}

				//reset reserved trak atom
				write_dword(preservedatomheaderbuffer_trak, ui32reservedatomheaderbuffersize_trak);
				write_dword(preservedatomheaderbuffer_trak + sizeof(_uint32_t), ATOM_MOOV_TRAK);
				ui32reservedatomheaderbuffersize_moov += ui32reservedatomheaderbuffersize_trak;
			}

			//reset reserved moov atom
			write_dword(preservedatomheaderbuffer_moov, ui32reservedatomheaderbuffersize_moov);
			write_dword(preservedatomheaderbuffer_moov + sizeof(_uint32_t), ATOM_MOOV);
		}

		//update stco atom in trak
		{
			vector<_uint32_t> vSectionMdatOffset;
			_uint32_t ui32MdatOffset = ui32destinationbuffersize + sizeof(Qt_common_atom);
			for (_uint32_t i = 0; i < pinfo->nSectionCount; i++)
			{
				vDataInfo[i].ui32AbsoluteDataOffset = ui32MdatOffset;
				vSectionMdatOffset.push_back(ui32MdatOffset);
				ui32MdatOffset += pSectionHeader[i].mdat.realsize;
			}

			for (_uint32_t i = 0; i < vTrakStcoInfo.size(); i++)
			{
				_pbyte_t pbuffer = vTrakStcoInfo[i].pbufferptr;
				_uint32_t uiTrackbufferCheck = 0;
				for (_uint32_t j = 0; j < pinfo->nSectionCount; j++)
				{
					Qt_header_moov_trak_mdia_minf_stbl_stco atom = pSectionHeader[j].moov.vTrakAtom[i].sMdiaAtom.sMinfAtom.sStblAtom.sStcoAtom;
					for (_uint32_t k = 0; k < atom.vSampleOffset.size(); k++)
					{
						_uint32_t offset = atom.vSampleOffset[k] - pSectionHeader[j].mdat.realoffset + vSectionMdatOffset[j];
						write_dword(pbuffer, offset);
						pbuffer += sizeof(_uint32_t);
						uiTrackbufferCheck++;
					}
				}

				assert(uiTrackbufferCheck == vTrakStcoInfo[i].ui32cellcount);
			}
		}

		//mdat
		{
			_uint32_t ui32MdatSize = sizeof(Qt_common_atom);
			for (_uint32_t i = 0; i < pinfo->nSectionCount; i++)
			{
				ui32MdatSize += pSectionHeader[i].mdat.realsize;
			}

			ui32destinationfilesize = ui32destinationbuffersize + ui32MdatSize;

			write_dword(pwritedestinationbuffer, ui32MdatSize);
			ui32destinationbuffersize +=sizeof(_uint32_t);
			write_dword(pwritedestinationbuffer + sizeof(_uint32_t), ATOM_MDAT);
			ui32destinationbuffersize +=sizeof(_uint32_t);
		}
	}

	m_bAnalyzeDone = true;
}