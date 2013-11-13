#ifndef _PLAYERCORE_MEDIAINFO_H_
#define _PLAYERCORE_MEDIAINFO_H_

#include <string>

class MediaInfo
{
public:
    MediaInfo(const char* url);
    ~MediaInfo();

protected:
    std::string url_;
};

#endif