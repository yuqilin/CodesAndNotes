#include <iostream>
#include <cstdio>
#include "Neptune.h"
#include "Platinum.h"
#include "NptLogging.h"
#include "NptDynamicRequestHandler.h"

#include <string>
#include <Windows.h>

using namespace std;

int main()
{
	NPT_LogManager::GetDefault().Configure("plist:.level=ALL;.handlers=FileHandler;"
		".FileHandler.filename=C:\\log\\p2pserver.log;"
		".FileHandler.filter=9;.FileHandler.append=false");

	PLT_HttpServer cdn_server(NPT_IpAddress::Any,8082);
	char* url="/功夫熊猫2[超清版]";
	NPT_HttpDynamicRequestHandler dynamic_handle(true,NULL,"Z:\\Shared\\Media\\功夫熊猫2");
	cdn_server.AddRequestHandler(&dynamic_handle,url);
	NPT_String url2="/人再囧途之泰囧[高清版]";
	cdn_server.AddRequestHandler(&dynamic_handle,url2.GetChars());

	cdn_server.Start();
	cout<<"The P2P server is running..."<<endl;

	getc(stdin);

	cdn_server.Stop();
	cout<<"The P2P server is turned off."<<endl;

	return 0;
}