#include "PlayerCodecs.h"

PlayerCodecs::PlayerCodecs()
{

}

PlayerCodecs::~PlayerCodecs()
{

}

void PlayerCodecs::SetCodecsPath(const char* path)
{
    if (path)
    {
        ATL::CString pathw = CStringA(path);

        if (PathIsDirectory(pathw) && PathFileExists(pathw))
        {
            codecs_path_ = pathw;
        }
    }
}


PlayerResult PlayerCodecs::LoadCodecs()
{
    PlayerResult res = kNoError;

    return res;
}

void PlayerCodecs::FreeCodecs()
{

}

PlayerResult 