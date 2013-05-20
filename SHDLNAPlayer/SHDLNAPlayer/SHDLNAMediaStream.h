#ifndef _SH_DLNA_MEDIA_STREAM_H_
#define _SH_DLNA_MEDIA_STREAM_H_

/*----------------------------------------------------------------------
|   interface IMediaStream
+---------------------------------------------------------------------*/
class IMediaStream 
{
public:
	virtual ~IMediaStream() {}
	virtual int Open(const char* url) = 0;
	virtual int Close() = 0;
	virtual int Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read) = 0;
	virtual int Seek(unsigned __int64 offset) = 0;
	virtual int Tell(unsigned __int64* offset) = 0;
	virtual int GetSize(unsigned __int64* size) = 0;
	virtual int GetAvailable(unsigned __int64* available) = 0;
};

/*----------------------------------------------------------------------
|   class CSHDLNAMediaStream
+---------------------------------------------------------------------*/
class CSHDLNAMediaStream : public NPT_InputStream
{
public:
	CSHDLNAMediaStream();
	virtual ~CSHDLNAMediaStream();

	virtual NPT_Result Open(const char* url);
	virtual NPT_Result Close();

	// NPT_InputStream methods
	virtual NPT_Result Read(void* buffer, NPT_Size bytes_to_read, NPT_Size* bytes_read = NULL);
	virtual NPT_Result Seek(NPT_Position offset);
	virtual NPT_Result Tell(NPT_Position& offset);
	virtual NPT_Result GetSize(NPT_LargeSize& size);
	virtual NPT_Result GetAvailable(NPT_LargeSize& available);

protected:
	IMediaStream*	m_Delegate;
};

#endif // _SH_DLNA_FILE_STREAM_H_
