#ifndef _PLAYERCORE_PLAYERERROR_H_
#define _PLAYERCORE_PLAYERERROR_H_

//
typedef long    PlayerResult;

//////////////////////////////////////////////////////////////////////////
// Error
#define kNoError                                ((PlayerResult)0)
#define kErrorUnknown                           ((PlayerResult)-1)

//
#define kErrorInvalidPointer                    ((PlayerResult)-10)
#define kErrorOutOfMemory                       ((PlayerResult)-11)
#define kErrorNotImplemented                    ((PlayerResult)-12)

//
#define kErrorMediaFormatNotSupported           ((PlayerResult)-100)







#endif