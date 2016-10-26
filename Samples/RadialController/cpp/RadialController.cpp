#include "stdafx.h"
#include <tchar.h>
#include <StrSafe.h>
#include <wrl\implements.h>
#include <wrl\module.h>
#include <wrl\event.h>
#include <roapi.h>
#include <wrl.h>
#include "DeviceListener.h"
#include <ShellScalingApi.h>

#define CLASSNAME L"Radial Device Controller"
#define BUFFER_SIZE 2000

LRESULT CALLBACK WindowProc(
    __in HWND hWindow,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam);

HANDLE console;
WCHAR windowClass[MAX_PATH];

void PrintMsg(WCHAR *msg)
{
    size_t textLength;
    DWORD charsWritten;
    StringCchLength(msg, BUFFER_SIZE, &textLength);
    WriteConsole(console, msg, (DWORD)textLength, &charsWritten, NULL);
}

void PrintStartupMessage()
{
    wchar_t message[2000];

    swprintf_s(message, 2000, L"Press F10 to begin...\n");
    PrintMsg(message);
}

int _cdecl _tmain(
    int argc,
    TCHAR *argv[])
{
    // Register Window Class
    WNDCLASS wndClass = {
        CS_DBLCLKS, (WNDPROC)WindowProc,
        0,0,0,0,0,0,0, CLASSNAME
    };
    RegisterClass(&wndClass);

    HWND hwnd = CreateWindow(
        CLASSNAME,
        L"Message Listener Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_VSCROLL,
        CW_USEDEFAULT,       // default horizontal position 
        CW_USEDEFAULT,       // default vertical position 
        CW_USEDEFAULT,       // default width 
        CW_USEDEFAULT,       // default height
        NULL, NULL, NULL, NULL);

    console = GetStdHandle(STD_OUTPUT_HANDLE);
    PrintStartupMessage();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);  // Dispatch message to WindowProc

        if (msg.message == WM_QUIT)
        {
            Windows::Foundation::Uninitialize();
            break;
        }
    }

    SetConsoleTextAttribute(console, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

DeviceListener *listener = nullptr;
LRESULT CALLBACK WindowProc(
    __in HWND hWindow,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        DestroyWindow(hWindow);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SYSKEYUP: //Press F10
        if (!listener)
        {
            listener = new DeviceListener(console);
            listener->Init(hWindow);
        }
    default:
        return DefWindowProc(hWindow, uMsg, wParam, lParam);
    }

    return 0;
}
