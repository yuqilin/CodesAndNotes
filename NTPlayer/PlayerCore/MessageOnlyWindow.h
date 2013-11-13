#ifndef _MESSAGE_ONLY_WINDOW_H_
#define _MESSAGE_ONLY_WINDOW_H_

#include <atlwin.h>


class MessageOnlyWindow : public CWindowImpl<MessageOnlyWindow, CWindow, CWinTraits<> >
{
public:
    MessageOnlyWindow() {}

    ~MessageOnlyWindow()
    {
        ::DestroyWindow(m_hWnd);
        m_hWnd = NULL;
    }

    HWND Create()
    {
        return CWindowImpl<MessageOnlyWindow, CWindow, CWinTraits<> >::Create(HWND_MESSAGE);
    }
};

#define WM_PLAYER_BASE      WM_USER + 0x100
#define WM_PLAYER_OPEN      WM_PLAYER_BASE + 1
#define WM_PLAYER_CLOSE     WM_PLAYER_BASE + 2
#define WM_PLAYER_PLAY      WM_PLAYER_BASE + 3
#define WM_PLAYER_PAUSE     WM_PLAYER_BASE + 4
#define WM_PLAYER_STOP      WM_PLAYER_BASE + 5
#define WM_PLAYER_EXIT      WM_PLAYER_BASE + 6

class PlayerMessageRoutine : public MessageOnlyWindow
{
public:
    BEGIN_MSG_MAP(PlayerMessageRoutine)
        MESSAGE_HANDLER(WM_PLAYER_OPEN,  OnPlayerOpen);
        MESSAGE_HANDLER(WM_PLAYER_CLOSE, OnPlayerClose);
        MESSAGE_HANDLER(WM_PLAYER_PLAY,  OnPlayerPlay);
        MESSAGE_HANDLER(WM_PLAYER_PAUSE, OnPlayerPause);
        MESSAGE_HANDLER(WM_PLAYER_STOP,  OnPlayerStop);
        MESSAGE_HANDLER(WM_PLAYER_EXIT,  OnPlayerExit);
    END_MSG_MAP()

public:
     LRESULT OnPlayerOpen(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
     LRESULT OnPlayerClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
     LRESULT OnPlayerPlay(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
     LRESULT OnPlayerPause(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
     LRESULT OnPlayerStop(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
     LRESULT OnPlayerExit(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

};


#endif