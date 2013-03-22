#pragma once
#ifndef id68C60171_0140_4DE1_B7255EFF557A74F9
#define id68C60171_0140_4DE1_B7255EFF557A74F9

#include <Window.hpp>
#include <commctrl.h>
#include <shellapi.h>

class MainWindow : public Window {
public:
    virtual LPCTSTR ClassName() { return TEXT("FastTyper_Main"); }
    static MainWindow *Create(LPCTSTR szTitle);
protected:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate();
    LRESULT OnDestroy();
    void OnPaint();
    virtual void OnDrop(HDROP hDrop);
    virtual DWORD OnDragDrop(UINT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt);
    LPVOID m_dropTarget;

    virtual HICON GetIcon();
    
    HWND m_delayUD, m_delay, m_delayLabel, m_delayMS;
    HWND m_intervalUD, m_interval, m_intervalLabel, m_intervalMS;
    HWND m_repeatUD, m_repeat, m_repeatLabel;
    HWND m_messageLabel, m_message;
    HWND m_executeButton, m_terminateButton;
    HWND m_scriptLabel, m_script, m_scriptBrowse;
private:
    HFONT hFont, hFontMono;
    HBRUSH hBrush;
    UDACCEL *udaSecondAccel;
    static DWORD s_OnDragDrop(UINT cf, HGLOBAL hData, HWND hWnd,
                       DWORD dwKeyState, POINTL pt, void *pUserData);
};

#endif // header
