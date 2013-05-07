
#ifndef _FLYFOX_STREAM_CTRL_H_
#define _FLYFOX_STREAM_CTRL_H_

#include "Neptune.h"
#include "Platinum.h"
#include "P2PRead.h"

// for test
#define CDNIPAddress	"127.0.0.1"
#define P2PIPAddress	"127.0.0.1"
#define CDNPort			8080
#define P2PPort			8082

class CFlyfoxStreamCtrl
{
public:
	CFlyfoxStreamCtrl();
	virtual ~CFlyfoxStreamCtrl();
	int Open(const char* name, int file_type);
	int Close();
	int Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read);
	int Seek(unsigned __int64 offset);
	int Tell(unsigned __int64* offset);
	int GetSize(unsigned __int64* size);
	int GetAvailable(unsigned __int64* available);
protected:
	bool RecvHeaderData(NPT_HttpUrl url,NPT_DataBuffer& buffer);
	bool RecvMediaData(NPT_HttpUrl url,int in_nStartPos, int in_nEndPos,NPT_DataBuffer& buffer);
private:
	unsigned __int64 m_fileSize;
	unsigned __int64 m_available;
	unsigned __int64 m_curPos;
	int m_type;
	FILE *m_openFile;
	//char *m_openFileName;
	NPT_String m_openFileName;
	QtSequence *m_sSequence;
};

#endif