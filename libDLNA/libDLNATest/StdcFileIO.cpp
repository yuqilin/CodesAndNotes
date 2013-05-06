
#include "stdafx.h"
#include <stdio.h>
#include <sys/stat.h>
#include "StdcFileIO.h"

static FILE* s_pFile = NULL;

int StdcFileOpen(const char* path, int mode)
{
	if (s_pFile)
		fclose(s_pFile);
	return fopen_s(&s_pFile, path, "rb");
}

int StdcFileClose()
{
	if (s_pFile)
		fclose(s_pFile);
	s_pFile = NULL;
	return 0;
}

int StdcFileRead(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read)
{
	if (buffer == NULL)
		return -1;

	if (s_pFile)
	{
		size_t nb_read = fread(buffer, 1, bytes_to_read, s_pFile);
		if (bytes_read)
			*bytes_read = nb_read;
	}
	return 0;
}

int StdcFileSeek(unsigned __int64 offset)
{
	if (s_pFile)
	{
		return _fseeki64(s_pFile, offset, SEEK_SET);
	}
	return 0;
}

int StdcFileTell(unsigned __int64* offset)
{
	if (s_pFile)
	{
		__int64 pos = _ftelli64(s_pFile);
		if (offset)
			*offset = pos;
	}
	return 0;
}

int StdcFileGetSize(unsigned __int64* size)
{
	if (s_pFile)
	{
		__int64 file_size = 0;
		struct _stat64 stat;
		int fd = fileno(s_pFile);
		if (_fstat64(fd, &stat) == 0)
		{
			file_size = stat.st_size;
			if (size)
				*size = file_size;
		}
		else
			return -1;
	}
	return 0;
}

int StdcFileGetAvailable(unsigned __int64* available)
{
	if (s_pFile)
	{
		unsigned __int64 offset = 0;
		if (0 == StdcFileTell(&offset))
		{
			unsigned __int64 size = 0;
			if (0 == StdcFileGetSize(&size) && offset >= 0 && offset <= size)
			{
				if (available)
					*available = size - offset;
			}
			goto failure;
		}
		goto failure;
	}
	return 0;

failure:
	if (available)
		*available = 0;
	return -1;
}
