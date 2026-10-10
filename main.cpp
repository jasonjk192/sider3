#define UNICODE

#include <stdio.h>
#include <windows.h>
#include "sider.h"


HMODULE dll;
HOOKPROC addr;
HHOOK handle;
HWND hWnd;
DWORD hookThreadId;

HWND heightLabel;
const wchar_t* currentStatus = L"Sider: Waiting for hook...";

bool _inited(false);

int init();

void UpdateStatus(const wchar_t* newText) 
{
    currentStatus = newText;
    if (heightLabel)
        SetWindowText(heightLabel, currentStatus);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case SIDER_MSG_ATTACH:
            if (wParam == SIDER_MSG_STATUS)
            {
                UpdateStatus(L"Sider: Attached to PES 2017.");
            } 
            break;

        case WM_DESTROY: 
        case SIDER_MSG_EXIT:
            // Exit the application when the window closes
            unsetHook();
            PostQuitMessage(1);
            return true;
    }
    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

bool InitApp(HINSTANCE hInstance, LPSTR lpCmdLine)
{
    WNDCLASSEX wcx;

    // cbSize - the size of the structure.
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = (WNDPROC)WindowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hInstance;
    wcx.hIcon = LoadIcon(hInstance, L"si");
    wcx.hCursor = LoadCursor(NULL,IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = SIDERCLS;
    wcx.hIconSm = LoadIcon(hInstance, L"si");

    // Detect already running sider
    HWND hwndPrev;
    if ((hwndPrev = FindWindow(wcx.lpszClassName, NULL)) != NULL) {
        SetForegroundWindow(hwndPrev);
        return false;
    }

    // Register the class with Windows
    if(!RegisterClassEx(&wcx))
        return false;

    return true;
}

bool LaunchGame()
{
    wstring path = game_exe_path();

    if (path.empty()) {
        log_(L"Main: Cannot launch game: executable path is empty.\n");
        return false;
    }

    // Use the game's directory as its working directory.
    wstring workingDir;
    size_t pos = path.find_last_of(L"\\/");

    if (pos != wstring::npos) {
        workingDir = path.substr(0, pos);
    }

    STARTUPINFOW si = {};
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi = {};

    BOOL success = CreateProcessW(
        path.c_str(),                   // Application path
        nullptr,                        // No additional command line
        nullptr,                        // Process security
        nullptr,                        // Thread security
        FALSE,                          // Do not inherit handles
        0,                              // Creation flags
        nullptr,                        // Inherit environment
        workingDir.empty()
        ? nullptr
        : workingDir.c_str(),       // Working directory
        &si,
        &pi
    );

    if (!success) {
        DWORD error = GetLastError();

        log_(L"Main: Failed to launch game. Error: %lu\n", error);
        UpdateStatus(L"Error: Could not launch PES 2017.");
        return false;
    }

    log_(L"Main: PES 2017 launched successfully.\n");

    // We don't need to retain these handles.
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    UpdateStatus(L"Sider: PES 2017 launched.");

    return true;
}

HWND BuildWindow(int nCmdShow)
{
    DWORD style, xstyle;
    HWND retval;

    style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    xstyle = WS_EX_LEFT;

    retval = CreateWindowEx(xstyle,
        L"SIDERCLS",      // class name
        L"Sider 3", // title for our window (appears in the titlebar)
        style,
        CW_USEDEFAULT,  // initial x coordinate
        CW_USEDEFAULT,  // initial y coordinate
        240, 90,   // width and height of the window
        NULL,           // no parent window.
        NULL,           // no menu
        NULL,           // no creator
        NULL);          // no extra data

    if (retval == NULL) return NULL;  // BAD.

    xstyle = WS_EX_LEFT;
    style = WS_CHILD | WS_VISIBLE;
    heightLabel = CreateWindowEx(
            xstyle, L"Static", 
            currentStatus, style,
            10, 10, 200, 50, 
            retval, NULL, NULL, NULL);

    HGDIOBJ hObj = GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(heightLabel, WM_SETFONT, (WPARAM)hObj, true);

    // Show the window
    if (start_minimized()) {
        ShowWindow(retval,
            SW_SHOWMINIMIZED|SW_SHOWMINNOACTIVE);
    }
    else {
        ShowWindow(retval,nCmdShow);
    }

    return retval; // return its handle for future use.
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    MSG msg; int retval;

    // init common controls
    //InitComCtls();

    if(InitApp(hInstance, lpCmdLine) == false)
        return 0;

    hWnd = BuildWindow(nCmdShow);
    if(hWnd == NULL) {
        return 0;
    }

    if (!_inited) {
        _inited = true;
        init();

        if (autorun_game()) {
            LaunchGame();
        }
    }

    //SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
    while((retval = GetMessage(&msg,NULL,0,0)) != 0)
    {
        if(retval == -1)
            return 0;   // an error occured while getting a message

        // need to call this to make WS_TABSTOP work
        if (!IsDialogMessage(hWnd, &msg)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}
 
int init()
{
    wstring version;
    version.reserve(64);
    get_module_version(NULL,version);
    start_log_(L"Sider: version %s\n", version.c_str());
    bool hookSuccess = setHook();
    if (hookSuccess) 
    {
        UpdateStatus(L"Sider: hook set up. Please run PES2017...");
        log_(L"Main: Init DONE - Hook active\n");
    }
    else
    {
        // Check if PES2017 is running to give a precise error message about Admin rights
        HWND hwndGame = FindWindow(NULL, L"Pro Evolution Soccer 2017"); // or check process list
        if (hwndGame != NULL) 
        {
            UpdateStatus(L"Error: PES 2017 running as Admin? Run Sider as Admin!");
            log_(L"Main: Hook failed. Possible permission mismatch (Administrator elevation required).\n");
        }
        else 
        {
            UpdateStatus(L"Status: Waiting for PES 2017 process...");
        }
    }
	return 0;
}
