// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <gdiplus.h>
using namespace Gdiplus;

class UiaCleanShutdownControl
{
public:
    UiaCleanShutdownControl();
    ~UiaCleanShutdownControl();
    static HWND Create(_In_ HWND parent, _In_ HINSTANCE instance, _In_ int x, _In_ int y, _In_ int width, _In_ int height);
    void Toggle();
    bool IsToggled();

    void IncrementProviderCount();
    void DecrementProviderCount();
    void IncrementTotalRefCount();
    void DecrementTotalRefCount();

private:
    static bool Initialize(_In_ HINSTANCE instance);
    static LRESULT CALLBACK StaticWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);
    LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);
    void OnPaint(_In_ HDC hdc);

    HWND _hwnd;
    static bool initialized;
    static unsigned int _controlCount;
    unsigned int _providerCount;
    unsigned int _providerTotalRefcount;
    bool _toggled;
};

