// PluginDLL.cpp : This plug-in simply creates an HWND tree (static controls and a bitmap)
//                 and returns the parent HWND. The host then reparents this HWND tree
//                 under itself. This is used to illustrate the type of DPI-scaling problems
//                 that can be seen when hosting external content within your process. The
//                 external content may not be DPI-aware and therefore might not scale as
//                 expected.
//

#include "stdafx.h"
#include "PluginHeader.h"
#include <windowsx.h>
#include "resource.h"
#include <Strsafe.h>
#include <windows.h>
#include <Commctrl.h>

#define DEFAULT_PADDING96          20
#define DEFAULT_CHAR_BUFFER        200
#define PROP_FONTSET               L"FONT_SET"

namespace PlugInDll
{

    void PlugInDll::ClassRegistration(HINSTANCE hInstance)
    {
        WNDCLASSEXW wcex = {};

        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
        wcex.lpszClassName = PLUGINWINDOWCLASSNAME;

        RegisterClassExW(&wcex);

    }

    int PlugInDll::ScaleToSystemDPI(int in, int mainMonitorDPI)
    {
        return MulDiv(in, mainMonitorDPI, 96);
    }

    LRESULT CALLBACK PlugInDll::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            case WM_CLOSE:
            {
                DestroyWindow(hWnd);
                return 0;
            }

            case WM_DESTROY:
            {
                return 0;
            }
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    // This method will create an HWND tree that is scaled to the system DPI
    // ("System DPI" is a global DPI that is based off of the scale factor of the primary display).
    // When the process that this code is running in is has a DPI_AWARENESS_CONTEXT of 
    // DPI_AWARENESS_CONTEXT_UNAWARE, the system DPI will be 96
    HWND PlugInDll::CreateContentHwnd(HINSTANCE hInstance, int nWidth, int nHeight)
    {
        // Register the window class
        ClassRegistration(hInstance);

        // Get the "System DPI"
        // Don't do this in per-monitor aware code as this will either
        // return 96 or the system DPI but will not return the per-monitor DPI
        int mainMonitorDPI = GetDpiForSystem();

        // Create an HWND tree that is parented to the message window (HWND_MESSAGE)
        HWND hWndExternalContent = CreateWindowExW(0L, PLUGINWINDOWCLASSNAME, HWND_NAME_EXTERNAL, WS_VISIBLE | WS_CHILD, 0, 0, nWidth, nHeight, HWND_MESSAGE, nullptr, hInstance, nullptr);
                
        // Add some child controls
        HWND hWndStatic = CreateWindowExW(WS_EX_LEFT, L"STATIC", L"External content static (text) control", SS_LEFT | WS_CHILD | WS_VISIBLE,
            ScaleToSystemDPI(DEFAULT_PADDING96, mainMonitorDPI), 
            ScaleToSystemDPI(DEFAULT_PADDING96, mainMonitorDPI), 
            ScaleToSystemDPI(nWidth - 2*DEFAULT_PADDING96, mainMonitorDPI),
            ScaleToSystemDPI(75, mainMonitorDPI),
            hWndExternalContent, nullptr, hInstance, nullptr);
        
        // Subclass the static control so that we can ignore WM_SETFONT from the host
        SetWindowSubclass(hWndStatic, PlugInDll::SubclassProc, 0, 0);

        // Set the font for the static control
        auto hFontOld = GetWindowFont(hWndStatic);
        LOGFONT lfText = {};
        SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lfText), &lfText, FALSE, mainMonitorDPI);
        HFONT hFontNew = CreateFontIndirect(&lfText);
        if (hFontNew)
        {
            SendMessage(hWndStatic, WM_SETFONT, (WPARAM)hFontNew, MAKELPARAM(TRUE, 0));
        }

        // Convert DPI awareness context to a string
        WCHAR awarenessContext[DEFAULT_CHAR_BUFFER];

        DPI_AWARENESS_CONTEXT dpiAwarenessContext = GetThreadDpiAwarenessContext();
        DPI_AWARENESS dpiAwareness = GetAwarenessFromDpiAwarenessContext(dpiAwarenessContext);

        // Convert DPI awareness to a string
        switch (dpiAwareness)
        {
        case DPI_AWARENESS_SYSTEM_AWARE:
            StringCchCopy(awarenessContext, ARRAYSIZE(awarenessContext), L"DPI_AWARENESS_CONTEXT_SYSTEM_AWARE");
            break;

        case DPI_AWARENESS_PER_MONITOR_AWARE:
            if (AreDpiAwarenessContextsEqual(dpiAwarenessContext, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
            {
                StringCchCopy(awarenessContext, ARRAYSIZE(awarenessContext), L"DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2");
            }
            else
            {
                StringCchCopy(awarenessContext, ARRAYSIZE(awarenessContext), L"DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE");
            }
            break;
        case DPI_AWARENESS_UNAWARE:
            // intentional fallthrough
        default:
            StringCchCopy(awarenessContext, ARRAYSIZE(awarenessContext), L"DPI_AWARENESS_CONTEXT_UNAWARE");
        }

        // Build the output string
        WCHAR result[DEFAULT_CHAR_BUFFER];
        StringCchPrintf(result, ARRAYSIZE(result), L"HWND content from an external source. The thread that created this content had a thread context of %s, with a DPI of: %d", awarenessContext, mainMonitorDPI);
        SetWindowText(hWndStatic, result);

        // Load a bitmap
        HMODULE hMod = GetModuleHandle(L"PluginDll.dll");
        HBITMAP hBmp = LoadBitmap(hMod, MAKEINTRESOURCE(IDB_BITMAP1));
		if (hBmp == NULL)
		{
			// Out of memory
			return NULL;
		}

        // Create a static control to put the image in to
		RECT rcClient = {};
        GetParentRelativeWindowRect(hWndStatic, &rcClient);
        HWND hWndImage = CreateWindowExW(WS_EX_LEFT, L"STATIC", L"External content static (bitmap) control", SS_BITMAP | WS_CHILD | WS_VISIBLE,
            ScaleToSystemDPI(DEFAULT_PADDING96, mainMonitorDPI),
            rcClient.bottom + ScaleToSystemDPI(DEFAULT_PADDING96, mainMonitorDPI),
            ScaleToSystemDPI(nWidth - 2 * DEFAULT_PADDING96, mainMonitorDPI),
            ScaleToSystemDPI(200, mainMonitorDPI),
            hWndExternalContent, nullptr, hInstance, nullptr);
        SendMessage(hWndImage, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);

        return hWndExternalContent;
    }

    BOOL PlugInDll::GetParentRelativeWindowRect(HWND hWnd, RECT* childBounds)
    {
        if (!GetWindowRect(hWnd, childBounds))
        {
            return FALSE;
        }

        MapWindowRect(HWND_DESKTOP, GetAncestor(hWnd, GA_PARENT), childBounds);

        return TRUE;
    }

    // Subclass the static control so that the parent can't send a new font when
    // the DPI changes. We want to illustrate how a child HWND can be bitmap
    // stretched by Windows. If the font were reset it would detract from 
    // illustrating this.
    LRESULT CALLBACK PlugInDll::SubclassProc(HWND hWnd, UINT uMsg,
        WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass,
        DWORD_PTR dwRefData)
    {
        switch (uMsg) 
        {
            // Store a flag indicating that the font has been set, then
            // don't let the font be set after that
            case WM_SETFONT:
            {
                BOOL bFontSet = PtrToInt(GetProp(hWnd, PROP_FONTSET));
                if (!bFontSet)
                {
                    // Allow the font set to happen
                    SetProp(hWnd, PROP_FONTSET, (HANDLE)TRUE);
                    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
                }
                else
                {
                    return 0;
                }
            }
        }

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }
}


