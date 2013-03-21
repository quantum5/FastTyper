#include <MainWindow.hpp>
#include <resource.h>
#include <commctrl.h>

#define GetStockBrush(id) reinterpret_cast<HBRUSH>(GetStockObject(id))
#define BGCOLOUR RGB(0xF0, 0xF0, 0xF0)
#define LEFT(x, y, cx, cy) x, y, cx, cy
#define RIGHT(x, y, cx, cy) (x - cx), y, cx, cy
#define BOTTOM(x, y, cx, cy) x, (y - cy), cx, cy
#define EXECUTE_BUTTON 0xDE01


LRESULT MainWindow::OnCreate()
{
    NONCLIENTMETRICS ncmMetrics = { sizeof(NONCLIENTMETRICS) };
    RECT client;

    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncmMetrics, 0);
    GetClientRect(m_hwnd, &client);
    
    hFont = CreateFontIndirect(&ncmMetrics.lfMessageFont);
    hBrush = CreateSolidBrush(BGCOLOUR);
    
    // Children
    m_delayLabel = CreateWindowEx(0, WC_STATIC,
            TEXT("Delay:"), WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_delay = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT,
            NULL, WS_CHILDWINDOW | WS_VISIBLE | ES_NUMBER | ES_LEFT,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_delayUD = CreateWindowEx(0, UPDOWN_CLASS, NULL,
            WS_CHILDWINDOW | WS_VISIBLE | UDS_AUTOBUDDY | UDS_SETBUDDYINT |
                UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_delayMS = CreateWindowEx(0, WC_STATIC,
            TEXT("ms"), WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);

    m_intervalLabel = CreateWindowEx(0, WC_STATIC,
            TEXT("Interval:"), WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_interval = CreateWindowEx(WS_EX_CLIENTEDGE , WC_EDIT,
            NULL, WS_CHILDWINDOW | WS_VISIBLE | ES_NUMBER | ES_LEFT,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_intervalUD = CreateWindowEx(0, UPDOWN_CLASS, NULL,
            WS_CHILDWINDOW | WS_VISIBLE | UDS_AUTOBUDDY | UDS_SETBUDDYINT |
                UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_intervalMS = CreateWindowEx(0, WC_STATIC,
            TEXT("ms"), WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);

    m_repeatLabel = CreateWindowEx(0, WC_STATIC,
            TEXT("Repeat:"), WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_repeat = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT,
            NULL, WS_CHILDWINDOW | WS_VISIBLE | ES_NUMBER | ES_LEFT,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_repeatUD = CreateWindowEx(0, UPDOWN_CLASS, NULL,
            WS_CHILDWINDOW | WS_VISIBLE | UDS_AUTOBUDDY | UDS_SETBUDDYINT |
                UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    
    m_messageLabel = CreateWindowEx(0, WC_STATIC,
            TEXT("Message:"), WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_message = CreateWindowEx(WS_EX_CLIENTEDGE , WC_EDIT,
            NULL, WS_CHILDWINDOW | WS_VISIBLE | ES_LEFT |
                  ES_MULTILINE | ES_AUTOVSCROLL,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);

    m_executeButton = CreateWindowEx(0, WC_BUTTON,
            TEXT("Execute"), WS_CHILDWINDOW | WS_VISIBLE | BS_DEFPUSHBUTTON,
            0, 0, 0, 0, m_hwnd, (HMENU) EXECUTE_BUTTON, GetInstance(), NULL);
    m_terminateButton = CreateWindowEx(0, WC_BUTTON,
            TEXT("Terminate"), WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);

#define SETFONT(hwnd) PostMessage(hwnd, WM_SETFONT, (WPARAM) hFont, (LPARAM) FALSE)
    SETFONT(m_delay);
    SETFONT(m_delayLabel);
    SETFONT(m_delayMS);
    SETFONT(m_interval);
    SETFONT(m_intervalLabel);
    SETFONT(m_intervalMS);
    SETFONT(m_repeat);
    SETFONT(m_repeatLabel);
    SETFONT(m_messageLabel);
    SETFONT(m_message);
    SETFONT(m_executeButton);
    SETFONT(m_terminateButton);
#undef SETFONT
    
    return 0;
}

LRESULT MainWindow::OnDestroy()
{
    DestroyWindow(m_delayUD);
    DestroyWindow(m_delay);
    DestroyWindow(m_delayLabel);
    DestroyWindow(m_delayMS);
    DestroyWindow(m_intervalUD);
    DestroyWindow(m_interval);
    DestroyWindow(m_intervalLabel);
    DestroyWindow(m_intervalMS);
    DestroyWindow(m_repeatUD);
    DestroyWindow(m_repeat);
    DestroyWindow(m_repeatLabel);
    DestroyWindow(m_messageLabel);
    DestroyWindow(m_message);
    DestroyWindow(m_executeButton);
    DestroyWindow(m_terminateButton);
    return 0;
}

HICON MainWindow::GetIcon()
{
    return LoadIcon(GetInstance(), MAKEINTRESOURCE(RID_ICON));
}

void MainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd, &ps);
    PaintContent(&ps);
    EndPaint(m_hwnd, &ps);
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_CREATE:
            return OnCreate();
        case WM_DESTROY:
            return OnDestroy();
        case WM_NCDESTROY:
            PostQuitMessage(0);
            break;
        case WM_PAINT:
            OnPaint();
            return 0;
        case WM_SIZE: {
            RECT client;
            GetClientRect(m_hwnd, &client);
#define REPOS(hwnd, k) SetWindowPos(hwnd, 0, k, SWP_NOACTIVATE|SWP_NOZORDER)
            REPOS(m_delay,         LEFT(70, 7, client.right - 107, 25));
            REPOS(m_delayLabel,    LEFT(12, 9, 41, 18));
            REPOS(m_delayMS,       RIGHT(client.right - 12, 9, 17, 18));
            REPOS(m_interval,      LEFT(70, 34, client.right - 107, 25));
            REPOS(m_intervalLabel, LEFT(12, 36, 59, 18));
            REPOS(m_intervalMS,    RIGHT(client.right - 12, 36, 17, 18));
            REPOS(m_repeat,        LEFT(70, 61, client.right - 107, 25));
            REPOS(m_repeatLabel,   LEFT(12, 63, 47, 18));
            REPOS(m_messageLabel,  LEFT(12, 91, 60, 18));
            REPOS(m_message,       LEFT(12, 110, client.right - 24, client.bottom - 152));
            REPOS(m_executeButton,   BOTTOM( 12, client.bottom - 12, 100, 25));
            REPOS(m_terminateButton, BOTTOM(124, client.bottom - 12, 100, 25));
#undef REPOS
            PostMessage(m_delayUD, UDM_SETBUDDY, (WPARAM) m_delay, 0);
            PostMessage(m_intervalUD, UDM_SETBUDDY, (WPARAM) m_interval, 0);
            PostMessage(m_repeatUD, UDM_SETBUDDY, (WPARAM) m_repeat, 0);
            MakeRectBottom(client, 12, client.bottom - 12, 100, 25);
            InvalidateRect(m_hwnd, &client, TRUE);
            return 0;
        }
        case WM_COMMAND:
            switch (wParam) {
                case EXECUTE_BUTTON:
                    MessageBox(m_hwnd, TEXT("!!!"), NULL, 0);
                    return 0;
            }
            break;
    }
    return __super::HandleMessage(uMsg, wParam, lParam);
}

MainWindow *MainWindow::Create(LPCTSTR szTitle)
{
    MainWindow *self = new MainWindow();
    if (self &&
        self->WinCreateWindow(0,
                szTitle, WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT, 484, 362,
                NULL, NULL)) {
        return self;
    }
    delete self;
    return NULL;
}
