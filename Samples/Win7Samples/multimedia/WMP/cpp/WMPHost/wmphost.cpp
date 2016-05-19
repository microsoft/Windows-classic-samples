// wmphost.cpp : Main window procedure
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "CWMPHost.h"
#include <commctrl.h>

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
//
extern "C" int WINAPI wWinMain(HINSTANCE hInstance,
    HINSTANCE /*hPrevInstance*/, __in LPWSTR lpCmdLine, int /*nShowCmd*/)
{
    lpCmdLine = GetCommandLine(); //this line necessary for _ATL_MIN_CRT

    CoInitialize(0);
    _Module.Init( ObjectMap, hInstance, &LIBID_ATLLib );

    ::InitCommonControls();

    RECT rcPos = { CW_USEDEFAULT, 0, 0, 0 };
    HMENU hMenu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MENU1));
    HICON hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON1));

    CWMPHost frame;
    frame.GetWndClassInfo().m_wc.hIcon = hIcon;
    frame.Create(GetDesktopWindow(), rcPos, L"WMP Host Container", 0, 0, (UINT)hMenu);
    frame.ShowWindow(SW_SHOWNORMAL);

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    _Module.Term();
    CoUninitialize();
    return 0;
}
