#include "stdafx.h"
#include "SHDLNAMediaStream.h"
#include "SHDLNAMediaStreamController.h"

NPT_SET_LOCAL_LOGGER("shdlnaplayer.shdlnamediastream")

/*----------------------------------------------------------------------
|   CSHDLNAMediaStream::CSHDLNAMediaStream
+---------------------------------------------------------------------*/
CSHDLNAMediaStream::CSHDLNAMediaStream()
{
	m_Delegate = new CSHDLNAMediaStreamController; // new CSHDLNAStreamController
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaStream::~CSHDLNAMediaStream
+---------------------------------------------------------------------*/
CSHDLNAMediaStream::~CSHDLNAMediaStream()
{
	delete m_Delegate;
	m_Delegate = NULL;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaStream::Open
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaStream::Open(const char* url)
{
	if (m_Delegate != NULL)
	{
		return m_Delegate->Open(url);
	}
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaStream::Close
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaStream::Close()
{
	if (m_Delegate != NULL)
	{
		return m_Delegate->Close();
	}
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaStream::Read
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaStream::Read(void* buffer, NPT_Size bytes_to_read, NPT_Size* bytes_read)
{
	if (bytes_read != NULL)
		*bytes_read = 0;
	if (m_Delegate != NULL)
	{
		return m_Delegate->Read(buffer, bytes_to_read, bytes_read);
	}
	return NPT_FAILURE;	
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaStream::Seek
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaStream::Seek(NPT_Position offset)
{
	if (m_Delegate != NULL)
	{
		return m_Delegate->Seek(offset);
	}
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaStream::Tell
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaStream::Tell(NPT_Position& offset)
{
	offset = 0;
	if (m_Delegate != NULL)
	{
		return m_Delegate->Tell(&offset);
	}
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaStream::GetSize
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaStream::GetSize(NPT_LargeSize& size)
{
	size = 0;
	if (m_Delegate != NULL)
	{
		return m_Delegate->GetSize(&size);
	}
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaStream::GetAvailable
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaStream::GetAvailable(NPT_LargeSize& available)
{
	available = 0;
	if (m_Delegate != NULL)
	{
		return m_Delegate->GetAvailable(&available);
	}
	return NPT_FAILURE;
}
