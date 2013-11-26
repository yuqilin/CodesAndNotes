
#pragma once

#include "../SubPic/ISubPic.h"

struct SubtitleInput {
    CComQIPtr<ISubStream> subStream;
    CComPtr<IBaseFilter> sourceFilter;

    SubtitleInput() {};
    SubtitleInput(CComQIPtr<ISubStream> subStream) : subStream(subStream) {};
    SubtitleInput(CComQIPtr<ISubStream> subStream, CComPtr<IBaseFilter> sourceFilter)
        : subStream(subStream), sourceFilter(sourceFilter) {};
};