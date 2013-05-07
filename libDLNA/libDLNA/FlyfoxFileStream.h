#ifndef _FLYFOX_FILE_STREAM_H_
#define _FLYFOX_FILE_STREAM_H_

// enum {
// 	FILE_TYPE_LOCAL		= 1,
// 	FILE_TYPE_ONLINE	= 2,
// };

#include "FlyfoxStreamCtrl.h"

class CFlyfoxFileInputStream : public NPT_InputStream
{
public:
	CFlyfoxFileInputStream(const char* path, int file_type/*, CFlyfoxStreamCtrl* stream*//*IOCallbacks* io*/)
		: m_FilePath(path), m_FileType(file_type)/*, m_StreamCtrl(stream)*/
	{
		m_StreamCtrl = new CFlyfoxStreamCtrl;
		Open(path, file_type);
	};
	virtual ~CFlyfoxFileInputStream()
	{
// 		Close();
// 		delete m_StreamCtrl;
// 		m_StreamCtrl = NULL;
	};

	virtual NPT_Result Open(const char* path, int type)
	{
		m_FilePath = path;
// 		if (m_IOCallbacks && m_IOCallbacks->Open)
// 			return m_IOCallbacks->Open(path, type);
// 		elsez
// 			return NPT_ERROR_NOT_IMPLEMENTED;
		if (m_StreamCtrl)
			return m_StreamCtrl->Open(path, type);

		return NPT_ERROR_NOT_IMPLEMENTED;
	};

	virtual NPT_Result Close()
	{
// 		if (m_IOCallbacks && m_IOCallbacks->Close)
// 			return m_IOCallbacks->Close();
// 		else
		if (m_StreamCtrl)
			return m_StreamCtrl->Close();
		
		return NPT_ERROR_NOT_IMPLEMENTED;
	};

	virtual NPT_Result Read(void* buffer, NPT_Size bytes_to_read, NPT_Size* bytes_read = NULL)
	{
// 		if (m_IOCallbacks && m_IOCallbacks->Read)
// 			return m_IOCallbacks->Read(buffer, bytes_to_read, bytes_read);
// 		else

		if (m_StreamCtrl)
			return m_StreamCtrl->Read(buffer, bytes_to_read, bytes_read);
		return NPT_ERROR_NOT_IMPLEMENTED;
	}

	virtual NPT_Result Seek(NPT_Position offset)
	{
// 		if (m_IOCallbacks && m_IOCallbacks->Seek)
// 			return m_IOCallbacks->Seek(offset);
		if (m_StreamCtrl)
			return m_StreamCtrl->Seek(offset);
		return NPT_ERROR_NOT_IMPLEMENTED;
	}

	virtual NPT_Result Tell(NPT_Position& offset)
	{
// 		if (m_IOCallbacks && m_IOCallbacks->Tell)
// 			return m_IOCallbacks->Tell(&offset);
		if (m_StreamCtrl)
			return m_StreamCtrl->Tell(&offset);
		return NPT_ERROR_NOT_IMPLEMENTED;
	}

	virtual NPT_Result GetSize(NPT_LargeSize& size)
	{
// 		if (m_IOCallbacks && m_IOCallbacks->GetSize)
// 			return m_IOCallbacks->GetSize(&size);
		if (m_StreamCtrl)
			return m_StreamCtrl->GetSize(&size);
		return NPT_ERROR_NOT_IMPLEMENTED;
	}

	virtual NPT_Result GetAvailable(NPT_LargeSize& available)
	{
// 		if (m_IOCallbacks && m_IOCallbacks->GetAvailable)
// 			return m_IOCallbacks->GetAvailable(&available);
		if  (m_StreamCtrl)
			return m_StreamCtrl->GetAvailable(&available);
		return NPT_ERROR_NOT_IMPLEMENTED;
	}

	// methods
	const NPT_String&   GetPath() { return m_FilePath; }

private:
	//IOCallbacks* m_IOCallbacks;
	CFlyfoxStreamCtrl*		m_StreamCtrl;
	NPT_String				m_FilePath;
	int						m_FileType;
};

// class CFlyfoxFileOutpuStream : public NPT_OutputStream
// {
// public:
// 	CFlyfoxFileOutpuStream() {};
// 	~CFlyfoxFileOutpuStream() {};
// };

// class CFlyfoxFileStream : public CFlyfoxFileInputStream
// 						/*, public CFlyfoxFileOutpuStream*/
// {
// public:
// 	CFlyfoxFileStream(IOCallbacks* io)
// 	{
// 		m_IOCallbacks = io;/*CLibDLNA::GetIOCallbacks();*/
// 	}
// 
// 	~CFlyfoxFileStream() {
// 	}
// 
// 	// NPT_FileInterface methods
// 	virtual NPT_Result Open(const char* path, int type) {
// 		m_Path = path;
// 		if (m_IOCallbacks && m_IOCallbacks->Open)
// 			return m_IOCallbacks->Open(path, type);
// 		else
// 			return NPT_ERROR_NOT_IMPLEMENTED;
// 	};
// 
// 	virtual NPT_Result Close() {
// 		if (m_IOCallbacks && m_IOCallbacks->Close)
// 			return m_IOCallbacks->Close();
// 		else
// 			return NPT_ERROR_NOT_IMPLEMENTED;
// 	};
// 
// 	virtual NPT_Result GetInputStream(NPT_InputStreamReference& stream) {
// 		stream = new CFlyfoxInputStream();
// 		return NPT_SUCCESS;
// 	};
// 
// // 	virtual NPT_Result GetOutputStream(NPT_OutputStreamReference& stream) {
// // 		stream = NULL;
// // 		return NPT_SUCCESS;
// // 	};
// 
// 	// NPT_InputStream methods
// 	virtual NPT_Result Read(void* buffer, NPT_Size bytes_to_read, NPT_Size* bytes_read = NULL) {
// 		if (m_IOCallbacks && m_IOCallbacks->Read)
// 			return m_IOCallbacks->Read(buffer, bytes_to_read, bytes_read);
// 		else 
// 			return NPT_ERROR_NOT_IMPLEMENTED;
// 	}
// 	virtual NPT_Result Seek(NPT_Position offset) {
// 		if (m_IOCallbacks && m_IOCallbacks->Seek)
// 			return m_IOCallbacks->Seek(offset);
// 		return NPT_ERROR_NOT_IMPLEMENTED;
// 	}
// 	virtual NPT_Result Tell(NPT_Position& offset) {
// 		if (m_IOCallbacks && m_IOCallbacks->Tell)
// 			return m_IOCallbacks->Tell(&offset);
// 		return NPT_ERROR_NOT_IMPLEMENTED;
// 	}
// 	virtual NPT_Result GetSize(NPT_LargeSize& size) {
// 		if (m_IOCallbacks && m_IOCallbacks->GetSize)
// 			return m_IOCallbacks->GetSize(&size);
// 		return NPT_ERROR_NOT_IMPLEMENTED;
// 	}
// 	virtual NPT_Result GetAvailable(NPT_LargeSize& available) {
// 		if (m_IOCallbacks && m_IOCallbacks->GetAvailable)
// 			return m_IOCallbacks->GetAvailable(&available);
// 		return NPT_ERROR_NOT_IMPLEMENTED;
// 	}
// 
// 	// methods
// 	const NPT_String&   GetPath() { return m_Path; }
// 
// protected:
// 	NPT_String			m_Path;
// 	IOCallbacks*		m_IOCallbacks;
// };


#endif