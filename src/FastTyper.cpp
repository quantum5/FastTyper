#include <MainWindow.hpp>
#include <commctrl.h>
#include <objbase.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "ole32.lib")

class COleInitialize {
public:
    COleInitialize() : m_hr(OleInitialize(NULL)) { }
    ~COleInitialize() { if (SUCCEEDED(m_hr)) OleUninitialize(); }
    operator HRESULT() const { return m_hr; }
    HRESULT m_hr;
};

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    COleInitialize oleInit;
    InitCommonControls();

    MainWindow *win = MainWindow::Create(TEXT("FastTyper"));
    if (win) {
        ShowWindow(win->GetHWND(), nCmdShow);
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}
