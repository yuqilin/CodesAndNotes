#pragma once

//////////////////////////////////////////////////////////////////////////
typedef enum tagPlayerState {
    kPlayerStateNothingSpecial,
    kPlayerStateOpening,
    //kPlayerStateBuffering,
    kPlayerStatePlaying,
    kPlayerStatePaused,
    kPlayerStateStopped,
    //kPlayerStateEnded,
    kPlayerStateClosing,
    kPlayerStateError,
} PlayerState;


