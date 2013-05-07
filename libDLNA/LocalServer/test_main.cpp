#include "StreamControl.h"
#include <iostream>
#include <fstream>
using namespace std;

int main()
{
	CStreamCtrl stream;
	char* name="π¶∑Ú–‹√®2[≥¨«Â∞Ê]";
	int type=2;
	//char* name="D:\\Àÿ≤ƒ\\∑÷∂Œ∫œ≤¢\\π¶∑Ú–‹√®2\\π¶∑Ú–‹√®2[≥¨«Â∞Ê].mp4";
	//int type=1;
	stream.Open(name,type);
	unsigned __int64 fileSize;
	stream.GetSize(&fileSize);
	const unsigned int Step=128*1024;
	unsigned int bytes_read=0;
	unsigned int pos=0;
	unsigned char* buffer=new unsigned char[Step];

	ofstream out(L"D:\\π¶∑Ú–‹√®2[≥¨«Â∞Ê]Ω” ’.mp4",ios::out|ios::binary);
	bool isheader=true;
	while(pos<fileSize)
	{
		stream.Read(buffer,Step,&bytes_read);
		if(bytes_read==0)
			break;
		pos+=bytes_read;
		out.write((char*)buffer,bytes_read);
	}

	out.close();
	delete []buffer;
	buffer=NULL;
	return 0;
}