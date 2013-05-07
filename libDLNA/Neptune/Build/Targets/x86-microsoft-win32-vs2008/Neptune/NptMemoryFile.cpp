
#include "NptConfig.h"
#include "NptUtils.h"
#include "NptFile.h"
#include "NptThreads.h"
#include "NptInterfaces.h"
#include "NptStrings.h"
#include "NptDebug.h"


/*----------------------------------------------------------------------
|   NPT_MemoryFile
+---------------------------------------------------------------------*/
class NPT_MemoryFile: public NPT_FileInterface
{
public:
	// constructors and destructor
	NPT_MemoryFile(NPT_File& delegator);
	~NPT_MemoryFile();

	// NPT_FileInterface methods
	NPT_Result Open(OpenMode mode = NPT_FILE_OPEN_MODE_READ); /* read only */
	NPT_Result Close();
	NPT_Result GetInputStream(NPT_InputStreamReference& stream);
	NPT_Result GetOutputStream(NPT_OutputStreamReference& stream);

private:
	// members
	NPT_File&             m_Delegator;
	OpenMode              m_Mode;
	//NPT_StdcFileReference m_FileReference;
};

/*----------------------------------------------------------------------
|   NPT_MemoryFile::NPT_MemoryFile
+---------------------------------------------------------------------*/
NPT_MemoryFile::NPT_MemoryFile(NPT_File& delegator) :
m_Delegator(delegator),
m_Mode(0)
{}

/*----------------------------------------------------------------------
|   NPT_MemoryFile::~NPT_MemoryFile
+---------------------------------------------------------------------*/
NPT_MemoryFile::~NPT_MemoryFile()
{
	Close();
}

/*----------------------------------------------------------------------
|   NPT_MemoryFile::Close
+---------------------------------------------------------------------*/
NPT_Result
NPT_MemoryFile::Close()
{
	// release the file reference
	//m_FileReference = NULL;

	// reset the mode
	m_Mode = 0;

	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   NPT_MemoryFile::GetInputStream
+---------------------------------------------------------------------*/
NPT_Result 
NPT_MemoryFile::GetInputStream(NPT_InputStreamReference& stream)
{
	// default value
	stream = NULL;

	// check that the file is open
	//if (m_FileReference.IsNull()) return NPT_ERROR_FILE_NOT_OPEN;

	// check that the mode is compatible
	if (!(m_Mode & NPT_FILE_OPEN_MODE_READ)) {
		return NPT_ERROR_FILE_NOT_READABLE;
	}

	// create a stream
	stream = new CFlyfoxInputStream();

	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   NPT_MemoryFile::GetOutputStream
+---------------------------------------------------------------------*/
NPT_Result 
NPT_MemoryFile::GetOutputStream(NPT_OutputStreamReference& stream)
{
	// default value
	stream = NULL;

	// check that the file is open
	if (m_FileReference.IsNull()) return NPT_ERROR_FILE_NOT_OPEN;

	// check that the mode is compatible
	if (!(m_Mode & NPT_FILE_OPEN_MODE_WRITE)) {
		return NPT_ERROR_FILE_NOT_WRITABLE;
	}

	// create a stream
	stream = new NPT_StdcFileOutputStream(m_FileReference);

	return NPT_SUCCESS;
}