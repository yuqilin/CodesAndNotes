#include "stdafx.h"
#include <assert.h>
#include "SHDLNAMediaDownloader.h"
#include "SHDLNAMediaPlayer.h"
#include "MultiRequest.h"
#include "json.h"
#include "rapidxml/shxmlparser.hpp"

NPT_SET_LOCAL_LOGGER("shdlnaplayer.shdlnamediadownloader");

#define DOWNLOAD_BREAK			-1
#define DOWNLOAD_CONTINUE		0

/*----------------------------------------------------------------------
|   CSHDLNAMediaDownloader::CSHDLNAMediaDownloader
+---------------------------------------------------------------------*/
CSHDLNAMediaDownloader::CSHDLNAMediaDownloader(const char* url, void* user)
: m_User(user)
, m_HeaderDownloadCompleted(false)
, m_Url(url)
, m_DownloadStatus(DOWNLOAD_CONTINUE)
, m_header_info_ptr(NULL)
, hotvrs_string_ptr(NULL)
, cdn_string_ptr(NULL)
, header_string_ptr(NULL)
{
	
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaDownloader::DoRun
+---------------------------------------------------------------------*/
void CSHDLNAMediaDownloader::Run()
{
	ATL::CString mainfilename;
	CSHDLNAMediaPlayer* player = (CSHDLNAMediaPlayer*)m_User;
	if (player == NULL || player->m_MediaInfo == NULL)
		goto download_failure;

	assert(player->m_MediaInfo->header_info == NULL);

	{
		NPT_AutoLock lock(player->m_MediaInfoLock);
		player->m_MediaInfo->header_info = new SHMP4HeaderInfo_t;
		if (player->m_MediaInfo->header_info == NULL)
			goto download_failure;
	}

	m_header_info_ptr = player->m_MediaInfo->header_info;

	if (NPT_SUCCEEDED(HotVrsRequestVideoInfo(mainfilename)))
	{
		player->m_MediaInfo->title = wcs2mbs(CP_UTF8, mainfilename).c_str();
		player->PushUrlToDevice();
	}
	else
		goto download_failure;

	if (NPT_SUCCEEDED(CDNRequestVideoHeader()))
	{
		player->OnOpenResult(NPT_SUCCESS);
	}

download_failure:
	player->OnOpenResult(NPT_FAILURE);

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaDownloader::RequestHeaderInfo
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaDownloader::HotVrsRequestVideoInfo(ATL::CString& mainfilename)
{
	NPT_Result result = NPT_SUCCESS;

	if(!hotvrs_string_ptr)
		hotvrs_string_ptr = new string[1];

	CMultiRequest hotvrs_client(Response_Whole_DataType);
	//使用手动重置为无信号状态，初始化时无信号状态
	bool bresult = hotvrs_client.AddRequest(
		m_Url,
		0,
		CSHDLNAMediaDownloader::HotVrsResponseCallBackFunc,
		CSHDLNAMediaDownloader::ProgressCallBack,
		NULL,
		this);
	if (!bresult)
		return NPT_FAILURE;

	CURLcode resultcode = hotvrs_client.Perform();
	if (resultcode > CURLE_OK)
		return NPT_FAILURE;

	SHVideoInfo* video_info_ptr = &m_header_info_ptr->info;

	bresult = JsonParse(hotvrs_string_ptr[0].c_str(), hotvrs_string_ptr[0].size(), *video_info_ptr);
	if (!bresult)
		return NPT_FAILURE;

	switch (video_info_ptr->version)
	{
	case VER_ORIGINAL:
		mainfilename = video_info_ptr->name + L"[原画版]";
		break;
	case VER_SUPER:
		mainfilename = video_info_ptr->name + L"[超清版]";
		break;
	case VER_HIGH:
		mainfilename = video_info_ptr->name + L"[高清版]";
		break;
	case VER_NORMAL:
		mainfilename = video_info_ptr->name + L"[普清版]";
		break;
	default:
		mainfilename = video_info_ptr->name;
		break;
	}
	mainfilename += ".mp4";

	return result;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaDownloader::CDNRequestVideoHeader
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaDownloader::CDNRequestVideoHeader()
{
	if (m_header_info_ptr == NULL)
		return NPT_FAILURE;

	SHVideoInfo* video_info_ptr = &m_header_info_ptr->info;

	bool bresult;
	size_t section_count = video_info_ptr->sections.size();

	NPT_LOG_FATAL_1("%d section cdn request...\n", section_count);

	if (!cdn_string_ptr)
		cdn_string_ptr = new string[section_count];

	CMultiRequest cdn_client(Response_Whole_DataType);
	/* Allocate one CURL handle per transfer */
	for (size_t i=0; i < section_count; i++)
	{
		//请求调度
		wostringstream stream;
		stream<<L"http://"<<(LPCTSTR)video_info_ptr->allot<<L"/p2p?file="<<(LPCTSTR)video_info_ptr->sections[i].url<<L"&new="<<(LPCTSTR)video_info_ptr->sections[i].newAddress;
		stream<<L"&num="<<1<<L"&idc="<<video_info_ptr->tn<<L"&key="<<(LPCTSTR)video_info_ptr->sections[i].key;

		wstring wstream = stream.str();

		{
			//wprintf(L"[CDN %d]%s\n", i, wstream.c_str());
		}

		USES_CONVERSION;

		bresult = cdn_client.AddRequest(
			W2A(wstream.c_str()),
			0,
			CSHDLNAMediaDownloader::CdnResponseCallBackFunc,
			CSHDLNAMediaDownloader::ProgressCallBack,
			NULL,
			this);
		if (!bresult)
			return NPT_FAILURE;
	}

	CURLcode resultcode = cdn_client.Perform();
	if (resultcode > CURLE_OK)
		return -1;

	printf("%d section header request...\n", section_count);

	NPT_LOG_FATAL_1("%d section header request...\n", section_count);

	if (!header_string_ptr)
		header_string_ptr= new string[section_count];

	vector<SHCDNInfo>* cdnInfolist_ptr = &m_header_info_ptr->cdninfo;

	CMultiRequest header_client(Response_Whole_DataType);
	for (size_t idx = 0; idx < section_count; idx++)
	{
		SHCDNInfo cdninfo;
		memset(&cdninfo, 0, sizeof(SHCDNInfo));
		XmlParse(cdn_string_ptr[idx].c_str(), cdn_string_ptr[idx].size(), cdninfo);
		cdnInfolist_ptr->push_back(cdninfo);

		wostringstream cdnstream;
		cdnstream<<L"http://"<<(LPCTSTR)cdninfo.ip.c_str()<<(LPCTSTR)cdninfo.url.c_str()<<"?key="<<(LPCTSTR)cdninfo.key.c_str()<<"plat=ifox"<<
			"&ch="<<(LPCTSTR)video_info_ptr->ch<<"&catcode="<<(LPCTSTR)video_info_ptr->catcode<<"&start="<<0<<"&startpos="<<0<<"&headeronly";

		wstring wstream = cdnstream.str();

		{
			//wprintf(L"[Header %d]%s\n", idx, wstream.c_str());
		}

		USES_CONVERSION;
		bresult = header_client.AddRequest(
			W2A(wstream.c_str()),
			0,
			CSHDLNAMediaDownloader::HeaderResponseCallBackFunc,
			CSHDLNAMediaDownloader::ProgressCallBack,
			NULL,
			this);
		if (!bresult)
			return NPT_FAILURE;
	}

	resultcode = header_client.Perform();
	if (resultcode > CURLE_OK)
		return NPT_FAILURE;

	SequenceInfo sequenceinfo;
	sequenceinfo.nSectionCount = section_count;
	sequenceinfo.pSectioninfo = new SectionInfo[section_count];

	for (unsigned int i = 0; i < section_count; i++)
	{
		sequenceinfo.pSectioninfo[i].pSectionInfoBuffer = (_pbyte_t)malloc(header_string_ptr[i].size());
		memcpy(sequenceinfo.pSectioninfo[i].pSectionInfoBuffer, header_string_ptr[i].c_str(), header_string_ptr[i].size());
		sequenceinfo.pSectioninfo[i].ui32SectionInfoBufferSize = header_string_ptr[i].size();
	}

	QtSequence* sequence_ptr = &m_header_info_ptr->sequence;
	sequence_ptr->Fill(&sequenceinfo);

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

	if (header_string_ptr)
	{
		delete [] header_string_ptr;
		header_string_ptr = NULL;
	}

	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaDownloader::Interrupt
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaDownloader::Interrupt()
{
	NPT_Result result = NPT_SUCCESS;

	NPT_AutoLock lock(m_Lock);
	m_DownloadStatus  = DOWNLOAD_BREAK;

	return result;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaDownloader::HotVrsResponseCallBackFunc
+---------------------------------------------------------------------*/
size_t CSHDLNAMediaDownloader::HotVrsResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)lpVoid);
	if( NULL == str)
		return -1;

	char* pData = (char*)buffer;

	CSHDLNAMediaDownloader* pThis = dynamic_cast<CSHDLNAMediaDownloader*>((CSHDLNAMediaDownloader *)str->m_customparam);
	if (pThis)
	{
		NPT_AutoLock lock(pThis->m_Lock);
		pThis->hotvrs_string_ptr[str->m_index].append(pData, size * nmemb);
		str->m_response_size += size * nmemb;
	}

	return nmemb;
}


size_t CSHDLNAMediaDownloader::CdnResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)lpVoid);
	if( NULL == str)
		return -1;

	char* pData = (char*)buffer;

	CSHDLNAMediaDownloader* pThis = dynamic_cast<CSHDLNAMediaDownloader*>((CSHDLNAMediaDownloader *)str->m_customparam);
	if (pThis)
	{
		NPT_AutoLock lock(pThis->m_Lock);
		pThis->cdn_string_ptr[str->m_index].append(pData, size * nmemb);
		str->m_response_size += size * nmemb;
	}

	return nmemb;
}

size_t CSHDLNAMediaDownloader::HeaderResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)lpVoid);
	if( NULL == str)
		return -1;

	char* pData = (char*)buffer;
	CSHDLNAMediaDownloader* pThis = dynamic_cast<CSHDLNAMediaDownloader*>((CSHDLNAMediaDownloader *)str->m_customparam);
	if (pThis)
	{
		NPT_AutoLock lock(pThis->m_Lock);
		pThis->header_string_ptr[str->m_index].append(pData, size * nmemb);
		str->m_response_size += size * nmemb;
	}

	return nmemb;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaDownloader::ProgressCallBack
+---------------------------------------------------------------------*/
int CSHDLNAMediaDownloader::ProgressCallBack(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)clientp);
	if (str)
	{
		CSHDLNAMediaDownloader* pThis = dynamic_cast<CSHDLNAMediaDownloader*>((CSHDLNAMediaDownloader*)str->m_customparam);
		NPT_AutoLock lock(pThis->m_Lock);
		return pThis->m_DownloadStatus;
	}

	return 0;
}

bool CSHDLNAMediaDownloader::JsonParse(const void* xml, const unsigned int xmlsize, SHVideoInfo& videoinfo)
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

bool CSHDLNAMediaDownloader::XmlParse(const void* buffer, const unsigned int buffersize, SHCDNInfo& cdnInfo)
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


wstring CSHDLNAMediaDownloader::Utf82W(LPCSTR szContent)
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