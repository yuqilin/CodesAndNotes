#pragma once

int StdcFileOpen(const char* path, int mode);

int StdcFileClose();

int StdcFileRead(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read);

int StdcFileSeek(unsigned __int64 offset);

int StdcFileTell(unsigned __int64* offset);

int StdcFileGetSize(unsigned __int64* size);

int StdcFileGetAvailable(unsigned __int64* available);
