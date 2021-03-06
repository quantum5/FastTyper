#pragma once
#ifndef id68C60171_0140_4DE1_B7255EFF557A74F9
#define id68C60171_0140_4DE1_B7255EFF557A74F9

#include <Window.hpp>
#include <MyDropTarget.hpp>

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

    virtual HICON GetIcon();
    
    HWND m_delayUD, m_delay, m_delayLabel, m_delayMS;
    HWND m_intervalUD, m_interval, m_intervalLabel, m_intervalMS;
    HWND m_repeatUD, m_repeat, m_repeatLabel;
    HWND m_messageLabel, m_message;
    HWND m_executeButton, m_terminateButton;
    HWND m_scriptLabel, m_script, m_scriptBrowse;
    MyDropTarget m_dropTarget;
private:
    HFONT hFont, hFontMono;
    HBRUSH hBrush;
    UDACCEL *udaSecondAccel;
};

#endif // header
