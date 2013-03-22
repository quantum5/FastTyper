#include <MainWindow.hpp>
#include <resource.h>
#include <commctrl.h>
#include <commdlg.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <shlobj.h>

#include <limits.h>

#include <DragAndDrop.h>

#define BGCOLOUR RGB(0xF0, 0xF0, 0xF0)
#define LEFT(x, y, cx, cy) x, y, cx, cy
#define RIGHT(x, y, cx, cy) (x - cx), y, cx, cy
#define BOTTOM(x, y, cx, cy) x, (y - cy), cx, cy
#define BOTTOMRIGHT(x, y, cx, cy) (x - cx), (y - cy), cx, cy
#define EXECUTE_BUTTON      0xDE01
#define TERMINATE_BUTTON    0xDE02
#define BROWSE_BUTTON       0xDE03

#ifndef FNERR_INVALIDFILENAME
#   define FNERR_INVALIDFILENAME 0x3002
#endif

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

LRESULT MainWindow::OnCreate()
{
    NONCLIENTMETRICS ncmMetrics = { sizeof(NONCLIENTMETRICS) };
    RECT client;

    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncmMetrics, 0);
    GetClientRect(m_hwnd, &client);
    
    hFont = CreateFontIndirect(&ncmMetrics.lfMessageFont);
    hBrush = CreateSolidBrush(BGCOLOUR);
    hFontMono = CreateFont(0, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, FF_MODERN, TEXT("Consolas"));
    
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
            0, 0, 0, 0, m_hwnd, (HMENU) TERMINATE_BUTTON, GetInstance(), NULL);
    
    m_scriptLabel = CreateWindowEx(0, WC_STATIC,
            TEXT("Script:"), WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_script = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT,
            NULL, WS_CHILDWINDOW | WS_VISIBLE | ES_LEFT,
            0, 0, 0, 0, m_hwnd, NULL, GetInstance(), NULL);
    m_scriptBrowse = CreateWindowEx(0, WC_BUTTON, TEXT("Browse..."),
            WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 0, 0, m_hwnd, (HMENU) BROWSE_BUTTON, GetInstance(), NULL);

    PostMessage(m_delayUD, UDM_SETRANGE32, 0, LONG_MAX);
    PostMessage(m_intervalUD, UDM_SETRANGE32, 0, LONG_MAX);
    PostMessage(m_repeatUD, UDM_SETRANGE32, 0, LONG_MAX);
    PostMessage(m_delayUD, UDM_SETPOS32, 0, 1000);
    PostMessage(m_repeatUD, UDM_SETPOS32, 0, 10);
    
    {
        udaSecondAccel = new UDACCEL[3];
        
        udaSecondAccel[0].nSec = 0;
        udaSecondAccel[0].nInc = 100;
        udaSecondAccel[1].nSec = 1;
        udaSecondAccel[1].nInc = 1000;
        udaSecondAccel[2].nSec = 3;
        udaSecondAccel[2].nInc = 10000;
        
        PostMessage(m_delayUD, UDM_SETACCEL, 3, (LPARAM) udaSecondAccel);
        PostMessage(m_intervalUD, UDM_SETACCEL, 3, (LPARAM) udaSecondAccel);
    }
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
    SETFONT(m_scriptLabel);
    SETFONT(m_script);
    SETFONT(m_scriptBrowse);
    SETFONT(m_executeButton);
    SETFONT(m_terminateButton);
#undef SETFONT
    PostMessage(m_message, WM_SETFONT, (WPARAM) hFontMono, (LPARAM) FALSE);
    Button_Enable(m_terminateButton, FALSE);
    SHAutoComplete(m_script, SHACF_FILESYSTEM | SHACF_USETAB);
    
    // Drag and Drop
    DragAcceptFiles(m_hwnd, TRUE);
    
    UINT cf = CF_TEXT;
    MyDragDropInit(GetProcessHeap());
    m_dropTarget = MyRegisterDragDrop(m_message, &cf, 1, WM_NULL,
                       MainWindow::s_OnDragDrop, this);
    
    return 0;
}

DWORD MainWindow::OnDragDrop(UINT cf, HGLOBAL hData, HWND hWnd,
                             DWORD dwKeyState, POINTL pt)
{
    if (cf == CF_TEXT) {
        LPVOID pData = GlobalLock(hData);
        if (pData == NULL)
            return DROPEFFECT_NONE;
        SendMessage(m_message, EM_REPLACESEL, 0, (LPARAM) pData);
        GlobalUnlock(hData);
        return DROPEFFECT_COPY;
    }/* else if (cf == CF_HDROP) {
        LPVOID pData = GlobalLock(hData);
        if (pData == NULL)
            return DROPEFFECT_NONE;
        
        LPDROPFILES data = (LPDROPFILES) pData;
        if (data->fWide) {
            LPWSTR file = (LPWSTR) (((LPBYTE) pData) + data->pFiles);
            SetWindowTextW(m_script, file);
        } else {
            LPSTR file = (LPSTR) (((LPBYTE) pData) + data->pFiles);
            SetWindowTextA(m_script, file);
        }
        
        GlobalUnlock(hData);
        return DROPEFFECT_COPY;
    }*/
    return DROPEFFECT_NONE;
}

void MainWindow::OnDrop(HDROP hDrop)
{
    TCHAR szFile[MAX_PATH];
    
    if (DragQueryFile(hDrop, 0, szFile, MAX_PATH))
        SetWindowText(m_script, szFile);
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
    DestroyWindow(m_scriptLabel);
    DestroyWindow(m_script);
    DestroyWindow(m_scriptBrowse);
    DestroyWindow(m_executeButton);
    DestroyWindow(m_terminateButton);
    
    MyRevokeDragDrop((PMYDROPTARGET) m_dropTarget);
    delete [] udaSecondAccel;
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
            REPOS(m_message,       LEFT(12, 110, client.right - 24, client.bottom - 183));
            REPOS(m_script,        BOTTOM(54, client.bottom - 41, client.right - 142, 25));
            REPOS(m_scriptLabel,   BOTTOM(12, client.bottom - 45, 35, 18));
            REPOS(m_scriptBrowse,  BOTTOMRIGHT(client.right - 12, client.bottom - 41, 70, 25));
            REPOS(m_executeButton,   BOTTOM(12, client.bottom - 12, 70, 25));
            REPOS(m_terminateButton, BOTTOM(88, client.bottom - 12, 70, 25));
#undef REPOS
            PostMessage(m_delayUD, UDM_SETBUDDY, (WPARAM) m_delay, 0);
            PostMessage(m_intervalUD, UDM_SETBUDDY, (WPARAM) m_interval, 0);
            PostMessage(m_repeatUD, UDM_SETBUDDY, (WPARAM) m_repeat, 0);
            return 0;
        }
        case WM_COMMAND:
            switch (wParam) {
                case EXECUTE_BUTTON:
                    MessageBox(m_hwnd, TEXT("Started"), NULL, 0);
                    Button_Enable(m_terminateButton, TRUE);
                    Button_Enable(m_executeButton, FALSE);
                    return 0;
                case TERMINATE_BUTTON:
                    MessageBox(m_hwnd, TEXT("Stopped"), NULL, 0);
                    Button_Enable(m_terminateButton, FALSE);
                    Button_Enable(m_executeButton, TRUE);
                    return 0;
                case BROWSE_BUTTON: {
                    OPENFILENAME ofn = { sizeof(OPENFILENAME) };
                    TCHAR current[MAX_PATH], select[MAX_PATH] = {0};
                    
                    GetWindowText(m_script, current, MAX_PATH);
                    ofn.hwndOwner = m_hwnd;
                    ofn.lpstrFilter = 
                        TEXT("Type Script File (*.tps)\0*.tps\0")
                        TEXT("All Files (*.*)\0*.*\0");
                    ofn.nFilterIndex = 1;
                    if (PathIsDirectory(current))
                        ofn.lpstrInitialDir = current;
                    else if (PathFileExists(current))
                        lstrcpy(select, current);
                    ofn.lpstrFile = select;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.Flags = OFN_EXPLORER;
                    
                    openfile:
                    if (GetOpenFileName(&ofn))
                        SetWindowText(m_script, select);
                    else {
                        DWORD error = CommDlgExtendedError();
                        if (error == FNERR_INVALIDFILENAME) {
                            ZeroMemory(select, MAX_PATH * sizeof(TCHAR));
                            goto openfile;
                        } else if (error != 0)
                            MessageBox(m_hwnd, TEXT("Failed to open dialog!"),
                                       TEXT("Error!"), MB_ICONERROR);
                    }
                    return 0;
                }
            }
            break;
        case WM_CTLCOLORSTATIC:
            SetBkColor((HDC) wParam, BGCOLOUR);
            return (INT_PTR) hBrush;
        case WM_DROPFILES:
            OnDrop((HDROP) wParam);
            return 0;
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

DWORD MainWindow::s_OnDragDrop(UINT cf, HGLOBAL hData, HWND hWnd,
                               DWORD dwKeyState, POINTL pt, void *pUserData)
{
    MainWindow *self = (MainWindow*) pUserData;
    return self->OnDragDrop(cf, hData, hWnd, dwKeyState, pt);
}
