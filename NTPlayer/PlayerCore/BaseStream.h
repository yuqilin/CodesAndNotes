#ifndef _PLAYERCORE_BASESTREAM_H_
#define _PLAYERCORE_BASESTREAM_H_

class BaseStream
{
public:
    BaseStream() {}
    virtual ~BaseStream() {}

    virtual PlayerResult Open(const char* url) = 0;
    virtual PlayerResult Close() = 0;
    virtual PlayerResult GetSize(uint64_t *total, uint64_t* available) = 0;
    virtual PlayerResult Seek(uint64_t pos);
    virtual PlayerResult Read(uint8_t* buffer, uint32_t bytes_to_read, uint32_t* bytes_read);

};

#endif