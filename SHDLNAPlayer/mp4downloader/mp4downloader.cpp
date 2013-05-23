// mp4downloader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "MP4DownloadTask.h"
#include "json.h"

typedef NPT_Reference<PLT_TaskManager> TaskManagerReference;

bool JsonParse(const void* xml, const unsigned int xmlsize, SHVideoInfo& videoinfo);

void PrintMessage(LPCTSTR msg) {
	::MessageBox(NULL, msg, _T(""), MB_OK);
}

int _tmain(int argc, _TCHAR* argv[])
{
	//NPT_String url("http://hot.vrs.sohu.com/vrs_flash.action?vid=1078239&ver=31&ref=0001");

	NPT_HttpUrl http_url("http://hot.vrs.sohu.com/vrs_flash.action?vid=1078239&ver=31&ref=0001");

	TaskManagerReference task_manager(new PLT_TaskManager());

// 	NPT_File file("E:\\temp\\json.txt");
// 	file.Open(NPT_FILE_OPEN_MODE_WRITE| NPT_FILE_OPEN_MODE_CREATE);
// 	NPT_OutputStreamReference stream;
// 	file.GetOutputStream(stream);

	NPT_StringOutputStreamReference stream(new NPT_StringOutputStream);
	NPT_OutputStreamReference output = stream;

	NPT_Reference<PLT_Downloader> downloader(new PLT_Downloader(task_manager.AsPointer(), http_url, (NPT_OutputStreamReference)stream));

	downloader->Start();

	while (PLT_DOWNLOADER_SUCCESS != downloader->GetState())
	{
		Sleep(1);
	}

	PrintMessage(_T("Request mp4 header info succeeded!"));

	NPT_String hotvrs_string = stream->GetString();

	SHVideoInfo video_info;

	bool bResult = JsonParse(hotvrs_string.GetChars(), hotvrs_string.GetLength(), video_info);

	size_t section_count = video_info.sections.size();


	

	return 0;
}

bool JsonParse(const void* xml, const unsigned int xmlsize, SHVideoInfo& videoinfo)
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
			videoinfo.allot = mbs2wcs(CP_UTF8, value.asCString()).c_str();
		}
		value = root["reserveIp"];
		if (!value.isNull())
		{
			videoinfo.reserveIp = mbs2wcs(CP_UTF8, value.asCString()).c_str();
		}
		value = root["url"];
		if (!value.isNull())
		{
			videoinfo.url = mbs2wcs(CP_UTF8, value.asCString()).c_str();
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
			videoinfo.catcode = mbs2wcs(CP_UTF8, value.asCString()).c_str();
		}

		Json::Value data = root["data"];
		if (data.isNull())
		{
			break;
		}
		value = data["coverImg"];
		if (!value.isNull())
		{
			videoinfo.coverImg = mbs2wcs(CP_UTF8, value.asCString()).c_str();
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
			videoinfo.name = mbs2wcs(CP_UTF8, value.asCString()).c_str();
		}
		if (videoinfo.name.IsEmpty())
		{
			value = data["tvName"];
			if (!value.isNull())
			{
				videoinfo.name = mbs2wcs(CP_UTF8, value.asCString()).c_str();
			}
		}
		value = data["ch"];
		if (!value.isNull())
		{
			videoinfo.ch = mbs2wcs(CP_UTF8, value.asCString()).c_str();
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
				urlVector.push_back( mbs2wcs(CP_UTF8, value[index].asCString()));
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
				keyVector.push_back(mbs2wcs(CP_UTF8, value[index].asCString()));
			}
		}
		vector<wstring> hashVector;
		value = data["hc"];
		if (!value.isNull())
		{
			for (size_t index = 0; index < value.size(); index++)
			{
				hashVector.push_back(mbs2wcs(CP_UTF8, value[index].asCString()));
			}
		}
		vector<wstring> newVector;
		value = data["su"];
		if (!value.isNull())
		{
			for (size_t index = 0; index < value.size(); index++)
			{
				newVector.push_back(mbs2wcs(CP_UTF8, value[index].asCString()));
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
				aspect.desc = mbs2wcs(CP_UTF8, value[index]["v"].asCString()).c_str();
				videoinfo.aspects.push_back(aspect);
			}
		}
		bRet = true;
	} while (0);

	return bRet;
}