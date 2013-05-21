#include "stdafx.h"
#include "Mp4DownloadClient.h"
#include "MultiRequest.h"
#include "json.h"
#include "rapidxml/shxmlparser.hpp"

#ifdef _DEBUG
#pragma comment(lib, "json_libmtd.lib")
#else
#pragma comment(lib, "json_libmt.lib")
#endif

#pragma comment(lib, "Rpcrt4.lib")

string* CMp4DownloadClient::hotvrs_string_ptr = NULL;
string* CMp4DownloadClient::cdn_string_ptr = NULL;
string* CMp4DownloadClient::header_string_ptr = NULL;
string* CMp4DownloadClient::hotvrs_string_ptr_for_name = NULL;

CMp4DownloadClient::CMp4DownloadClient(void)
{

}

CMp4DownloadClient::~CMp4DownloadClient(void)
{
	FlushRequest();
}

void CMp4DownloadClient::InitialRequest(QtSequence* sequence_ptr, SHVideoInfo* info_ptr, vector<SHCDNInfo>* cdnInfolist_ptr, 
										EMediaDataRequestSource eTryResoure)
{
	m_eResoureType = eTryResoure;
	m_sequence_ptr = sequence_ptr;
	m_info_ptr = info_ptr;
	m_cdnInfolist_ptr = cdnInfolist_ptr;
}

int CMp4DownloadClient::HeaderRequest(const char* url, 
									  CString& mainfilename,
									  ProgressCallBack hotvr_processcb, 
									  ProgressCallBack cdn_processcb, 
									  ProgressCallBack header_processcb)
{
	if (url == NULL || m_sequence_ptr == NULL || m_info_ptr == NULL || m_cdnInfolist_ptr == NULL)
		return -1;

	bool bresult = false;

	hotvrs_string_ptr = new string[1];

	printf("hotvrs request...\n");

	CMultiRequest hotvrs_client(Response_Whole_DataType);
	//使用手动重置为无信号状态，初始化时无信号状态
	bresult = hotvrs_client.AddRequest(url, 0, HotVrsResponseCallBackFunc, hotvr_processcb);
	if (!bresult)
		return -1;

	CURLcode resultcode = hotvrs_client.Perform();
	if (resultcode > CURLE_OK)
		return -1;

	bresult = JsonParse(hotvrs_string_ptr[0].c_str(), hotvrs_string_ptr[0].size(), *m_info_ptr);
	if (!bresult)
		return -1;

	switch (m_info_ptr->version)
	{
	case VER_ORIGINAL:
		mainfilename = m_info_ptr->name + L"[原画版]";
		break;
	case VER_SUPER:
		mainfilename = m_info_ptr->name + L"[超清版]";
		break;
	case VER_HIGH:
		mainfilename = m_info_ptr->name + L"[高清版]";
		break;
	case VER_NORMAL:
		mainfilename = m_info_ptr->name + L"[普清版]";
		break;
	default:
		mainfilename = m_info_ptr->name;
	}

	mainfilename += ".mp4";

	size_t section_count = m_info_ptr->sections.size();

	printf("%d section cdn request...\n", section_count);

	cdn_string_ptr = new string[section_count];
	CMultiRequest cdn_client(Response_Whole_DataType);
	/* Allocate one CURL handle per transfer */
	for (size_t i=0; i < section_count; i++)
	{
		//请求调度
		wostringstream stream;
		stream<<L"http://"<<(LPCTSTR)m_info_ptr->allot<<L"/p2p?file="<<(LPCTSTR)m_info_ptr->sections[i].url<<L"&new="<<(LPCTSTR)m_info_ptr->sections[i].newAddress;
		stream<<L"&num="<<1<<L"&idc="<<m_info_ptr->tn<<L"&key="<<(LPCTSTR)m_info_ptr->sections[i].key;

		wstring wstream = stream.str();

		{
			//wprintf(L"[CDN %d]%s\n", i, wstream.c_str());
		}

		USES_CONVERSION;

		bresult = cdn_client.AddRequest(W2A(wstream.c_str()), 0, CdnResponseCallBackFunc, cdn_processcb);
		if (!bresult)
			return -1;
	}

	resultcode = cdn_client.Perform();
	if (resultcode > CURLE_OK)
		return -1;

	printf("%d section header request...\n", section_count);

	header_string_ptr = new string[section_count];
	CMultiRequest header_client(Response_Whole_DataType);
	for (size_t idx = 0; idx < section_count; idx++)
	{
		SHCDNInfo cdninfo;
		memset(&cdninfo, 0, sizeof(SHCDNInfo));
		XmlParse(cdn_string_ptr[idx].c_str(), cdn_string_ptr[idx].size(), cdninfo);
		m_cdnInfolist_ptr->push_back(cdninfo);

		wostringstream cdnstream;
		cdnstream<<L"http://"<<(LPCTSTR)cdninfo.ip.c_str()<<(LPCTSTR)cdninfo.url.c_str()<<"?key="<<(LPCTSTR)cdninfo.key.c_str()<<"plat=ifox"<<
			"&ch="<<(LPCTSTR)m_info_ptr->ch<<"&catcode="<<(LPCTSTR)m_info_ptr->catcode<<"&start="<<0<<"&startpos="<<0<<"&headeronly";

		wstring wstream = cdnstream.str();

		{
			//wprintf(L"[Header %d]%s\n", idx, wstream.c_str());
		}

		USES_CONVERSION;
		bresult = header_client.AddRequest(W2A(wstream.c_str()), 0, HeaderResponseCallBackFunc, header_processcb);
		if (!bresult)
			return -1;
	}

	resultcode = header_client.Perform();
	if (resultcode > CURLE_OK)
		return -1;

	SequenceInfo sequenceinfo;
	sequenceinfo.nSectionCount = section_count;
	sequenceinfo.pSectioninfo = new SectionInfo[section_count];

	for (unsigned int i = 0; i < section_count; i++)
	{
		sequenceinfo.pSectioninfo[i].pSectionInfoBuffer = (_pbyte_t)malloc(header_string_ptr[i].size());
		memcpy(sequenceinfo.pSectioninfo[i].pSectionInfoBuffer, header_string_ptr[i].c_str(), header_string_ptr[i].size());
		sequenceinfo.pSectioninfo[i].ui32SectionInfoBufferSize = header_string_ptr[i].size();
	}

	m_sequence_ptr->Fill(&sequenceinfo);

	for (unsigned int i = 0; i < section_count; i++)
	{
		if (sequenceinfo.pSectioninfo[i].pSectionInfoBuffer)
		{
			free(sequenceinfo.pSectioninfo[i].pSectionInfoBuffer);
			sequenceinfo.pSectioninfo[i].pSectionInfoBuffer = NULL;
		}

		sequenceinfo.pSectioninfo[i].ui32SectionInfoBufferSize = 0;
	}

	if (sequenceinfo.pSectioninfo)
	{
		delete [] sequenceinfo.pSectioninfo;
		sequenceinfo.pSectioninfo = NULL;
	}

	sequenceinfo.nSectionCount = 0;

	return 0;
}

void CMp4DownloadClient::GetRequestFileHeader(unsigned char** ppbuffer, unsigned int* pbuffersize)
{
	if (m_sequence_ptr == NULL)
		return;

	_pbyte_t pHeaderBuffer = NULL;
	_uint32_t ui32HeaderBufferSize = 0;
	m_sequence_ptr->getHeaderBuffer(&pHeaderBuffer, &ui32HeaderBufferSize);

	if (pHeaderBuffer == NULL || ui32HeaderBufferSize == 0)
	{
		return;
	}

	if (ppbuffer)
		*ppbuffer = pHeaderBuffer;

	if (pbuffersize)
		*pbuffersize = ui32HeaderBufferSize;
}

void CMp4DownloadClient::GetRequestFileInfo(unsigned int* pfilesize)
{
	if (m_sequence_ptr == NULL)
		return;

	if (pfilesize)
		m_sequence_ptr->getFileSize(pfilesize);
}

int CMp4DownloadClient::DataRequest(unsigned int startpos, unsigned int endpos, unsigned int* pactualfinishpos, 
									ResponseCallBack cb, ProgressCallBack data_progresscb, void* customparam)
{
	if (cb == NULL || pactualfinishpos == NULL || m_sequence_ptr == NULL || m_info_ptr == NULL || m_cdnInfolist_ptr == NULL || startpos >= endpos)
		return -1;

	bool bresult = false;

	_uint32_t startclipnum = 0;
	_uint32_t endclipnum = 0;
	bresult = m_sequence_ptr->TranslateSectionRange(startpos, endpos, &startclipnum, &endclipnum);
	if (!bresult)
		return -1;

	printf("section %d - %d data request...\n", startclipnum, endclipnum);

	SectionDataInfo startsectioninfo;
	m_sequence_ptr->getSectionDataInfoByIndex(startclipnum, &startsectioninfo);

	SectionDataInfo endsectioninfo;
	m_sequence_ptr->getSectionDataInfoByIndex(endclipnum, &endsectioninfo);

	ClipOfRangeInfo sClipInfo;
	m_sequence_ptr->getSectionInfoByIndex(startclipnum, &sClipInfo);

	if (m_eResoureType == P2P_Local_RequestSource)
	{
		SHVideoInfo* video = m_info_ptr;

		unsigned int index = startclipnum;
		CString strUrl;
		CString strUid;
		strUid.Format(_T("%s_%d"),GetUuid().c_str(), index);

		SHVideoRequestParam param;
		param.index		= index;
		param.start			= 0;
		param.startpos	= sClipInfo.StartOffset;
		param.dnum		= index;
		param.pnum		= index;
		param.ptime		= 0;
		param.dtime		= 100000;
		param.cdnNum   = 1;
		param.shdtype   = SHDType_ClientPlay;

		int cdnNum = param.cdnNum > 0 ? param.cdnNum : video->tn;
		strUrl.Format(_T("http://127.0.0.1:8828/%s/%s?start=%d&vid=%d&hashid=%s&key=%s&num=%d&pnum=%d")
			_T("&dnum=%d&ptime=%d&dtime=%d&p2pflag=%d&size=%d&cdn=%d&new=%s&fname=%s&startpos=%d&r=%d&shdtype=%d&duration=%d&rld=%d&uid=%s&rip=%s")
			_T("&plat=%s&ch=%s&catcode=%s"),
			video->allot,video->sections[param.index].url,param.start,video->vid,video->sections[param.index].hashId,
			video->sections[param.index].key,param.index,param.pnum,param.dnum,param.ptime,param.dtime,param.p2pflag,
			video->sections[param.index].size,cdnNum,video->sections[param.index].newAddress,video->name,param.startpos,
			GetTickCount(),param.shdtype,param.duration,param.reportLocalData,strUid,video->reserveIp,
			_T("ifox"),video->ch,video->catcode);

		printf("Index %d: Range [%d-]\n", index, sClipInfo.StartOffset);

		USES_CONVERSION;
		const char* pRequest = W2A((LPCTSTR)strUrl);
		CMultiRequest data_client(Response_Partial_DataType);
		bresult = data_client.AddRequest(pRequest, startsectioninfo.ui32DataOffset + startsectioninfo.ui32DataSize - sClipInfo.StartOffset,
			cb, data_progresscb, NULL, customparam);
		if (!bresult)
			return -1;

		CURLcode resultcode = data_client.Perform();
		if (resultcode > CURLE_OK)
		{
			if (resultcode != CURLE_COULDNT_CONNECT)
				return -1;
			else
			{
				data_client.Done();
				printf("p2p request timeout, try to connect cdn directly!\n");
				goto cdn_lable;
			}
		}

		unsigned int response_data_size = data_client.GetResponseDataSizeByIndex(0);
		if (response_data_size > 0)
		{		
			assert(response_data_size <= (startsectioninfo.ui32DataOffset + startsectioninfo.ui32DataSize - sClipInfo.StartOffset));
		}

		for (_uint32_t j = startclipnum + 1; j <= endclipnum; j++)
		{
			SectionDataInfo sectioninfo;
			m_sequence_ptr->getSectionDataInfoByIndex(j, &sectioninfo);

			index = j;
			strUid.Format(_T("%s_%d"),GetUuid().c_str(), index);

			param.index		= index;
			param.startpos	= sectioninfo.ui32DataOffset;
			param.dnum		= index;
			param.pnum		= index;

			cdnNum = param.cdnNum > 0 ? param.cdnNum : video->tn;
			strUrl.Format(_T("http://127.0.0.1:8828/%s/%s?start=%d&vid=%d&hashid=%s&key=%s&num=%d&pnum=%d")
				_T("&dnum=%d&ptime=%d&dtime=%d&p2pflag=%d&size=%d&cdn=%d&new=%s&fname=%s&startpos=%d&r=%d&shdtype=%d&duration=%d&rld=%d&uid=%s&rip=%s")
				_T("&plat=%s&ch=%s&catcode=%s"),
				video->allot,video->sections[param.index].url,param.start,video->vid,video->sections[param.index].hashId,
				video->sections[param.index].key,param.index,param.pnum,param.dnum,param.ptime,param.dtime,param.p2pflag,
				video->sections[param.index].size,cdnNum,video->sections[param.index].newAddress,video->name,param.startpos,
				GetTickCount(),param.shdtype,param.duration,param.reportLocalData,strUid,video->reserveIp,
				_T("ifox"),video->ch,video->catcode);

			printf("Index %d: Range [%d-]\n", index, sectioninfo.ui32DataOffset);

			pRequest = W2A((LPCTSTR)strUrl);
			CMultiRequest data_client_ex(Response_Partial_DataType);
			bresult = data_client_ex.AddRequest(pRequest, sectioninfo.ui32DataOffset + sectioninfo.ui32DataSize - sectioninfo.ui32DataOffset, cb, data_progresscb, NULL, customparam);
			if (!bresult)
				return -1;

			resultcode = data_client_ex.Perform();
			if (resultcode > CURLE_OK)
				return -1;

			unsigned int response_data_size_ex = data_client_ex.GetResponseDataSizeByIndex(0);
			assert(response_data_size_ex <= (sectioninfo.ui32DataOffset + sectioninfo.ui32DataSize - sectioninfo.ui32DataOffset));
		}
	}
	else
	{
cdn_lable:
		m_eResoureType = CDN_Remote_RequestSource;

		wostringstream stream;
		stream<<L"http://"<<(LPCTSTR)(*m_cdnInfolist_ptr)[startclipnum].ip.c_str()<<(LPCTSTR)(*m_cdnInfolist_ptr)[startclipnum].url.c_str()<<"?key="<<(LPCTSTR)(*m_cdnInfolist_ptr)[startclipnum].key.c_str()<<"plat=ifox"<<
			"&ch="<<(LPCTSTR)m_info_ptr->ch<<"&catcode="<<(LPCTSTR)m_info_ptr->catcode<<"&start=0"<<"&rs=1";

		wstring wstream = stream.str();

		CMultiRequest data_client(Response_Partial_DataType);

		vector<pair<CURLoption, const char*> > vDataHeader;
		char strRange[100];
		sprintf(strRange, "Range:bytes=%d-%d", sClipInfo.StartOffset, startsectioninfo.ui32DataOffset + startsectioninfo.ui32DataSize - 1);
		printf("Index %d: Range [%d-%d]\n", startclipnum, sClipInfo.StartOffset, startsectioninfo.ui32DataOffset + startsectioninfo.ui32DataSize - 1);
		vDataHeader.push_back(pair<CURLoption, const char*>(CURLOPT_HTTPHEADER, strRange));

		USES_CONVERSION;
		const char* pRequest = W2A((LPCTSTR)wstream.c_str());
		bresult = data_client.AddRequest(pRequest, startsectioninfo.ui32DataOffset + startsectioninfo.ui32DataSize - sClipInfo.StartOffset,
			cb, data_progresscb, &vDataHeader, customparam);
		if (!bresult)
			return -1;

		CURLcode resultcode = data_client.Perform();
		if (resultcode > CURLE_OK)
			return -1;

		unsigned int response_data_size = data_client.GetResponseDataSizeByIndex(0);
		assert(response_data_size <= (startsectioninfo.ui32DataOffset + startsectioninfo.ui32DataSize - sClipInfo.StartOffset));

		for (_uint32_t j = startclipnum + 1; j <= endclipnum; j++)
		{
			SectionDataInfo sectioninfo;
			m_sequence_ptr->getSectionDataInfoByIndex(j, &sectioninfo);

			wostringstream stream;
			stream<<L"http://"<<(LPCTSTR)(*m_cdnInfolist_ptr)[j].ip.c_str()<<(LPCTSTR)(*m_cdnInfolist_ptr)[j].url.c_str()<<"?key="<<(LPCTSTR)(*m_cdnInfolist_ptr)[j].key.c_str()<<"plat=ifox"<<
				"&ch="<<(LPCTSTR)m_info_ptr->ch<<"&catcode="<<(LPCTSTR)m_info_ptr->catcode<<"&start=0"<<"&rs=1";

			wstring wstream = stream.str();

			CMultiRequest data_client_ex(Response_Partial_DataType);

			char strRange[100];
			sprintf(strRange, "Range:bytes=%d-%d", sectioninfo.ui32DataOffset, sectioninfo.ui32DataOffset + sectioninfo.ui32DataSize - 1);
			printf("Index %d: Range [%d-%d]\n", j, sectioninfo.ui32DataOffset, sectioninfo.ui32DataOffset + sectioninfo.ui32DataSize - 1);
			vDataHeader.push_back(pair<CURLoption, const char*>(CURLOPT_HTTPHEADER, strRange));

			USES_CONVERSION;
			const char* pRequest = W2A((LPCTSTR)wstream.c_str());
			bool bresult = data_client_ex.AddRequest(pRequest, sectioninfo.ui32DataOffset + sectioninfo.ui32DataSize - sectioninfo.ui32DataOffset,
				cb, data_progresscb, &vDataHeader, customparam);
			if (!bresult)
				return -1;

			resultcode = data_client_ex.Perform();
			if (resultcode > CURLE_OK)
				return -1;

			unsigned int response_data_size_ex = data_client_ex.GetResponseDataSizeByIndex(0);
			if (response_data_size_ex > 0)
				assert(response_data_size_ex <= (sectioninfo.ui32DataOffset + sectioninfo.ui32DataSize - sectioninfo.ui32DataOffset));
			else
				return -1;
		}
	}

	if (pactualfinishpos)
		*pactualfinishpos = endsectioninfo.ui32AbsoluteDataOffset + endsectioninfo.ui32DataSize;

	return 0;
}

void CMp4DownloadClient::FlushRequest(void)
{
	if (hotvrs_string_ptr)
	{
		delete [] hotvrs_string_ptr;
		hotvrs_string_ptr = NULL;
	}

	if (cdn_string_ptr)
	{
		delete [] cdn_string_ptr;
		cdn_string_ptr = NULL;
	}

	m_sequence_ptr = NULL;
	m_info_ptr = NULL;
	m_cdnInfolist_ptr = NULL;

	m_eResoureType = Invalid_RequestSource;

	return;
}

wstring CMp4DownloadClient::Utf82W(LPCSTR szContent)
{
	wstring strContent;
	int size = -1;
	int len = MultiByteToWideChar(CP_UTF8,0,szContent,size,NULL,0);
	strContent.resize(len);
	MultiByteToWideChar(CP_UTF8,0,szContent,size,(wchar_t*)strContent.c_str(),len);
	if(len > 0 && size == -1)
		strContent.resize(len-1);
	return strContent;
}

size_t CMp4DownloadClient::HotVrsResponseForNameCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)lpVoid);
	if( NULL == str)
		return -1;

	char* pData = (char*)buffer;
	hotvrs_string_ptr_for_name[str->m_index].append(pData, size * nmemb);

	str->m_response_size += size * nmemb;

	return nmemb;
}

size_t CMp4DownloadClient::HotVrsResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)lpVoid);
	if( NULL == str)
		return -1;

	char* pData = (char*)buffer;
	hotvrs_string_ptr[str->m_index].append(pData, size * nmemb);

	str->m_response_size += size * nmemb;

	return nmemb;
}

size_t CMp4DownloadClient::CdnResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)lpVoid);
	if( NULL == str)
		return -1;

	char* pData = (char*)buffer;
	cdn_string_ptr[str->m_index].append(pData, size * nmemb);

	str->m_response_size += size * nmemb;

	return nmemb;
}

size_t CMp4DownloadClient::HeaderResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)lpVoid);
	if( NULL == str)
		return -1;

	char* pData = (char*)buffer;
	header_string_ptr[str->m_index].append(pData, size * nmemb);

	str->m_response_size += size * nmemb;

	return nmemb;
}

bool CMp4DownloadClient::JsonParse(const void* xml, const unsigned int xmlsize, SHVideoInfo& videoinfo)
{
	if (xml == NULL || xmlsize == 0)
		return false;

	Json::Reader reader;
	Json::Value root;
	bool bRet = false;

	do
	{
		const char* xmlBegin = (const char*)xml;
		const char* xmlEnd = (const char*)xml + xmlsize;
		//请求调度
		bRet = reader.parse(xmlBegin, xmlEnd, root);
		if (!bRet)
		{
			break;
		}
		Json::Value value = root["p2pflag"];
		if (!value.isNull())
		{
			videoinfo.p2pflag = value.asInt();
		}
		value = root["id"];
		if (!value.isNull())
		{
			videoinfo.vid = value.asInt();
		}
		value = root["plcatid"];
		if (!value.isNull())
		{
			videoinfo.cid = value.asInt();
		}
		value = root["pid"];
		if (!value.isNull())
		{
			videoinfo.playerListId = value.asInt();
		}
		value = root["vt"];
		videoinfo.longVideo = !value.isNull() && (value.asInt() == 1);
		value = root["tn"];
		if (!value.isNull())
		{
			videoinfo.tn = value.asInt();
		}
		value = root["status"];
		if (!value.isNull())
		{
			videoinfo.status = value.asInt();
		}
		value = root["play"];
		videoinfo.play = !value.isNull() && (value.asInt() == 1);
		//value = root["wexp"];	// 即将到期
		//info.bExpired = !value.isNull() && (value.asInt() == 1);
		value = root["fms"];
		videoinfo.fms = !value.isNull() && (value.asInt() != 0);
		value = root["fee"];
		videoinfo.fee = !value.isNull() && (value.asInt() == 1);
		value = root["allot"];
		if (!value.isNull())
		{
			videoinfo.allot = Utf82W(value.asCString()).c_str();
		}
		value = root["reserveIp"];
		if (!value.isNull())
		{
			videoinfo.reserveIp = Utf82W(value.asCString()).c_str();
		}
		value = root["url"];
		if (!value.isNull())
		{
			videoinfo.url = Utf82W(value.asCString()).c_str();
		}
		value = root["isdl"];
		videoinfo.downloadEnable = value.isNull() || (value.asInt() == 1);

		value = root["systype"];
		if (!value.isNull())
		{
			videoinfo.systype = value.asInt();
		}

		value = root["catcode"];
		if (!value.isNull())
		{
			videoinfo.catcode = Utf82W(value.asCString()).c_str();
		}

		Json::Value data = root["data"];
		if (data.isNull())
		{
			break;
		}
		value = data["coverImg"];
		if (!value.isNull())
		{
			videoinfo.coverImg = Utf82W(value.asCString()).c_str();
		}
		value = data["version"];
		if (!value.isNull())
		{
			videoinfo.version = value.asInt();
		}
		value = data["num"];
		if (!value.isNull())
		{
			videoinfo.num = value.asInt();
		}
		value = data["sT"];
		if (!value.isNull())
		{
			videoinfo.st = value.asInt();
		}
		value = data["eT"];
		if (!value.isNull())
		{
			videoinfo.et = value.asInt();
		}
		value = data["subName"];
		if (!value.isNull())
		{
			videoinfo.name = Utf82W(value.asCString()).c_str();
		}
		if (videoinfo.name.IsEmpty())
		{
			value = data["tvName"];
			if (!value.isNull())
			{
				videoinfo.name = Utf82W(value.asCString()).c_str();
			}
		}
		value = data["ch"];
		if (!value.isNull())
		{
			videoinfo.ch = Utf82W(value.asCString()).c_str();
		}
		value = data["fps"];
		if (!value.isNull())
		{
			videoinfo.fps = value.asInt();
		}
		value = data["height"];
		if (!value.isNull())
		{
			videoinfo.height = value.asInt();
		}
		value = data["width"];
		if (!value.isNull())
		{
			videoinfo.width = value.asInt();
		}
		value = data["norVid"];
		if (!value.isNull())
		{
			videoinfo.norVid = value.asInt();
		}
		value = data["highVid"];
		if (!value.isNull())
		{
			videoinfo.highVid = value.asInt();
		}
		value = data["superVid"];
		if (!value.isNull())
		{
			videoinfo.superVid = value.asInt();
		}
		value = data["oriVid"];
		if (!value.isNull())
		{
			videoinfo.oriVid = value.asInt();
		}
		vector<wstring> urlVector;
		value = data["clipsURL"];
		if (!value.isNull())
		{
			for (size_t index = 0; index < value.size(); index++)
			{
				urlVector.push_back(Utf82W(value[index].asCString()));
				if(_tcsstr(urlVector[index].c_str(),_T("http://data.vod.itc.cn/")) == urlVector[index].c_str())
					urlVector[index].erase(0,_tcslen(_T("http://data.vod.itc.cn/")));
			}
		}
		vector<int> sizeVector;
		value = data["clipsBytes"];
		if (!value.isNull())
		{
			for (size_t index = 0; index < value.size(); index++)
			{
				sizeVector.push_back(value[index].asInt());
			}
		}
		vector<double> durationVector;
		value = data["clipsDuration"];
		if (!value.isNull())
		{
			for (size_t index = 0; index < value.size(); index++)
			{
				durationVector.push_back(value[index].asDouble());
			}
		}
		vector<wstring> keyVector;
		value = data["ck"];
		if (!value.isNull())
		{
			for (size_t index = 0; index < value.size(); index++)
			{
				keyVector.push_back(Utf82W(value[index].asCString()));
			}
		}
		vector<wstring> hashVector;
		value = data["hc"];
		if (!value.isNull())
		{
			for (size_t index = 0; index < value.size(); index++)
			{
				hashVector.push_back(Utf82W(value[index].asCString()));
			}
		}
		vector<wstring> newVector;
		value = data["su"];
		if (!value.isNull())
		{
			for (size_t index = 0; index < value.size(); index++)
			{
				newVector.push_back(Utf82W(value[index].asCString()));
			}
		}
		for (size_t i = 0; i < newVector.size(); ++i)
		{
			SHVideoSection secInfo;
			if (i < durationVector.size())
			{
				secInfo.duration = durationVector[i];
			}
			if (i < sizeVector.size())
			{
				secInfo.size = sizeVector[i];
			}
			if (i < urlVector.size())
			{
				secInfo.url = urlVector[i].c_str();
			}
			if(i < hashVector.size())
			{
				secInfo.hashId = hashVector[i].c_str();
			}
			if (i < keyVector.size())
			{
				secInfo.key = keyVector[i].c_str();
			}
			if (i < newVector.size())
			{
				secInfo.newAddress = newVector[i].c_str();
			}
			videoinfo.sections.push_back(secInfo);
			if(secInfo.size == 0 || (int)secInfo.duration == 0)
			{
				break;
			}
		}
		value = data["eP"];
		if (!value.isNull())
		{
			for (size_t index = 0; index < value.size(); index++)
			{
				SHVideoAspect aspect;
				aspect.time = value[index]["k"].asInt();
				aspect.desc = Utf82W(value[index]["v"].asCString()).c_str();
				videoinfo.aspects.push_back(aspect);
			}
		}
		bRet = true;
	} while (0);

	return bRet;
}

bool CMp4DownloadClient::XmlParse(const void* buffer, const unsigned int buffersize, SHCDNInfo& cdnInfo)
{
	string xmlstring;
	xmlstring.assign((const char*)buffer, buffersize);
	sh_xml_document<sh_xml_encode_type_utf8> doc;
	if(!doc.parse(xmlstring.c_str()))
		return false;

	sh_xml_node* nodePtr = NULL;
	nodePtr = doc.first_node(_T("allot"));
	if(nodePtr == NULL)
		return false;
	//
	sh_xml_node* childPtr = nodePtr->first_node(_T("idc"));
	if(childPtr == NULL || childPtr->value() == NULL)
		return false;
	int nIdc = _wtoi(childPtr->value());
	//path
	wstring path;
	childPtr = nodePtr->first_node(_T("path"));
	if(childPtr == NULL || childPtr->value() == NULL)
		return false;
	path = childPtr->value();
	//
	nodePtr = nodePtr->first_node(_T("group"));
	if(nodePtr == NULL)
		return false;
	sh_xml_node* itemPtr = nodePtr->first_node(_T("item"));
	if(itemPtr)
	{
		childPtr = itemPtr->first_node(_T("ip"));
		if(childPtr == NULL || childPtr->value() == NULL)
			return false;
		wstring ip = childPtr->value();
		//
		childPtr = itemPtr->first_node(_T("hd"));
		if(childPtr == NULL || childPtr->value() == NULL)
			return false;
		wstring hd = childPtr->value();
		//
		childPtr = itemPtr->first_node(_T("key"));
		if(childPtr == NULL || childPtr->value() == NULL)
			return false;
		wstring key = childPtr->value();
		cdnInfo = SHCDNInfo(ip,key,wstring(hd) + wstring(L"/") + wstring(path));
	}

	return true;
}

wstring CMp4DownloadClient::GetUuid()
{
	USES_CONVERSION;
	wstring strUid;
	UUID uid;
	if(UuidCreate(&uid) == RPC_S_OK)
	{
		RPC_WSTR szUid = NULL;
		if(UuidToString(&uid,&szUid) == RPC_S_OK)
		{
			strUid = (wchar_t *)szUid;
			RpcStringFree(&szUid);
		}
	}
	return strUid;
}

int CMp4DownloadClient::DataDebugCallBackFunc(CURL *, curl_infotype itype, char * pData, size_t size, void *)
{
	switch (itype)
	{
	case CURLINFO_TEXT:
		printf("[TEXT] %s\n", pData);
		break;
	case CURLINFO_HEADER_IN:
		printf("[HEADER_IN] %s\n", pData);
		break;
	case CURLINFO_HEADER_OUT:
		printf("[HEADER_OUT] %s\n", pData);
		break;
	case CURLINFO_DATA_IN:
		//printf("[DATA_IN] size = %s\n", pData);
		break;
	case CURLINFO_DATA_OUT:
		//printf("[DATA_OUT] %s\n", pData);
		break;
	}

	return 0;
}

int CMp4DownloadClient::GetFileName(const char* url, CString& mainfilename)
{
	if (url == NULL)
		return -1;

	bool bresult = false;

	if (!hotvrs_string_ptr_for_name)
	{
		hotvrs_string_ptr_for_name = new string[1];
	}


	printf("hotvrs request...\n");

	CMultiRequest hotvrs_client(Response_Whole_DataType);
	//使用手动重置为无信号状态，初始化时无信号状态
	bresult = hotvrs_client.AddRequest(url, 0, HotVrsResponseForNameCallBackFunc, ProgressCB);
	if (!bresult)
		return -1;

	CURLcode resultcode = hotvrs_client.Perform();
	if (resultcode > CURLE_OK)
		return -1;

	SHVideoInfo info_ptr;
	bresult = JsonParse(hotvrs_string_ptr_for_name[0].c_str(), hotvrs_string_ptr_for_name[0].size(), info_ptr);
	if (!bresult)
		return -1;
	switch (info_ptr.version)
	{
	case VER_ORIGINAL:
		mainfilename = info_ptr.name + L"[原话版]";
		break;
	case VER_SUPER:
		mainfilename = info_ptr.name + L"[超清版]";
		break;
	case VER_HIGH:
		mainfilename = info_ptr.name + L"[高清版]";
		break;
	case VER_NORMAL:
		mainfilename = info_ptr.name + L"[普清版]";
		break;
	default:
		mainfilename = info_ptr.name;
		break;
	}
	if (hotvrs_string_ptr_for_name)
	{
		delete []hotvrs_string_ptr_for_name;
		hotvrs_string_ptr_for_name=NULL;
	}
	return 0;
}


int CMp4DownloadClient::ProgressCB(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	return 0;
}

