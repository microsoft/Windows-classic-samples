// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <ole2.h>
#include <uiautomation.h>
#include <strsafe.h>

#include "UiaCleanShutdownControl.h"
#include "UiaProvider.h"

const Color textColor(255,0,0,0);
const Color backgroundColor(255,255,255,255);
const Color textColorToggled(255,255,255,25);
const Color backgroundColorToggled(255,0,30,255);

HMODULE thisModule;

BOOL APIENTRY DllMain( _In_ HMODULE hModule,
    _In_ DWORD  ul_reason_for_call,
    _In_ LPVOID lpReserved
    )
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        {
            thisModule = hModule;
            break;
        }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        UnregisterClass(L"UiaCleanShutdownControl", thisModule);
        break;
    }
    return TRUE;
}

HWND CreateCleanShutdownControl(_In_ HWND parent, _In_ int x, _In_ int y, _In_ int width, _In_ int height)
{
    return UiaCleanShutdownControl::Create(parent, thisModule, x, y, width, height);
}

HWND UiaCleanShutdownControl::Create(_In_ HWND parent, _In_ HINSTANCE instance, _In_ int x, _In_ int y, _In_ int width, _In_ int height)
{
    HWND returnHwnd = NULL;

    if (!initialized)
    {
        initialized = Initialize(instance);
    }

    if (initialized)
    {
        UiaCleanShutdownControl* control = new UiaCleanShutdownControl();
        if (control != NULL)
        {
            control->_hwnd = CreateWindow( L"UiaCleanShutdownControl", L"",
                WS_CHILD | WS_VISIBLE,
                x, y, width, height, parent, NULL, instance, static_cast<PVOID>(control));
            returnHwnd = control->_hwnd;
        }
    }
    return returnHwnd;
}


bool UiaCleanShutdownControl::Initialize(_In_ HINSTANCE instance)
{
    WNDCLASS wc;
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = UiaCleanShutdownControl::StaticWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = instance;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = L"UiaCleanShutdownControl";

    if (!RegisterClass(&wc))
    {
        return false;
    }

    return true;
}

UiaCleanShutdownControl::UiaCleanShutdownControl(): _providerCount(0),
                                                    _providerTotalRefcount(0),
                                                    _toggled(false),
                                                    _hwnd(NULL)
{
    InterlockedIncrement(&_controlCount);
}

UiaCleanShutdownControl::~UiaCleanShutdownControl()
{
    InterlockedDecrement(&_controlCount);
}

LRESULT CALLBACK UiaCleanShutdownControl::WndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    LRESULT lResult = 0;
    switch (message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            OnPaint(ps.hdc);
            EndPaint(hwnd, &ps);
            break;
        }
    case WM_GETOBJECT:
        {
            IRawElementProviderSimple * provider = new CleanShutdownProvider(hwnd, this);
            if (provider != NULL)
            {
                // It should be noted that this should be done regardless of lParam.
                // UiaReturnRawElementProvider checks the lParam for us and ignores, or sends the provider
                // or activates the Msaa Bridge based on its value.
                lResult = UiaReturnRawElementProvider(hwnd, wParam, lParam, provider);
                provider->Release();
            }
            break;
        }
    case WM_LBUTTONUP:
        {
            Toggle();
            break;
        }
    case WM_DESTROY:
        {
            // When the HWND is destroyed call Disconnect Provider
            // This will clean up all remote connections to the provider to allow it to
            // Release cleanly, as opposed to leaving references around that could cause
            // crashes after the dll is unloaded.
            IRawElementProviderSimple * provider = new CleanShutdownProvider(hwnd, this);
            if (provider != NULL)
            {
                // It is noteworthy that this is a new instance of the provider. 
                // UiaDisconnectProvider will actually disconnect ALL providers with the same
                // runtimeId as this provider, which in this case is based off the HWND associated
                // with the provider. We do not actually need to pass in the same provider object
                // as we returned for WM_GETOBJECT, hence using a new instance here.
                HRESULT hr = UiaDisconnectProvider(provider);
                provider->Release();
                if (FAILED(hr))
                {
                    // We check the HRESULT here specifically to determine if something went wrong with the
                    // disconnect. This should only actually error if the provider given is unable to resolve
                    // its Runtime Id properly, which indicates the problem is with the provider. A common
                    // cause of this is having IRawElementProviderFragment::GetRuntimeId fail when the window is gone,
                    // or having IRawElementProviderSimple::get_HostHwndProvider fail when the window is gone,
                    // or either of the two above methods being improperly implemented.
                    WCHAR errorMsg[200];
                    StringCchPrintf(errorMsg, ARRAYSIZE(errorMsg), L"UiaDisconnectProvider returned HRESULT 0x%8x", hr);

                    WCHAR errorTitle[200];
                    StringCchPrintf(errorTitle, ARRAYSIZE(errorTitle), L"UiaDisconnectProvider failed:");
                    MessageBox(NULL, errorMsg, errorTitle, MB_OK);
                }
            }
            delete this;
            break;
        }
    default:
        lResult = DefWindowProc(hwnd, message, wParam, lParam);
        break;
    }
            
    return lResult;
}

LRESULT CALLBACK UiaCleanShutdownControl::StaticWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    LRESULT retVal = 0;
    UiaCleanShutdownControl * pThis = reinterpret_cast<UiaCleanShutdownControl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE)
    {
        CREATESTRUCT *createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<UiaCleanShutdownControl*>(createStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    
    if (message == WM_NCDESTROY)
    {
        pThis = NULL;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
    }
    
    if (pThis != NULL)
    {
        retVal = pThis->WndProc(hwnd, message, wParam, lParam);
    }
    else
    {
        retVal = DefWindowProc(hwnd, message, wParam, lParam);
    }
    return retVal;
}


void UiaCleanShutdownControl::OnPaint(_In_ HDC hdc)
{
    Graphics graphics(hdc);

    graphics.Clear(_toggled ? backgroundColor : backgroundColorToggled);

    SolidBrush textBrush(_toggled ? textColor : textColorToggled);

    graphics.SetCompositingMode(CompositingModeSourceOver);
    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    graphics.SetSmoothingMode(SmoothingModeHighQuality);

    // The control displays some text showing the number of controls,
    // the number of Uia providers associated with this control, and the total refcount of those providers
    WCHAR buffer[200];

    // In a production application this should be loaded from a resource so it can be localized
    StringCchPrintf(buffer, ARRAYSIZE(buffer), L"Loaded...\nControls:%d\nProviders:%d\nRefcount:%d", _controlCount, _providerCount, _providerTotalRefcount);

    Font font(L"Calibri", 12.0f, FontStyleRegular);
    graphics.DrawString(buffer, -1, &font, PointF(0, 0), &textBrush);
}

void UiaCleanShutdownControl::Toggle()
{
    _toggled = !_toggled;
    InvalidateRect(_hwnd, NULL, FALSE);
}

bool UiaCleanShutdownControl::IsToggled()
{
    return _toggled;
}

void UiaCleanShutdownControl::IncrementProviderCount()
{
    InterlockedIncrement(&_providerCount);
    InvalidateRect(_hwnd, NULL, FALSE);
}

void UiaCleanShutdownControl::DecrementProviderCount()
{
    InterlockedDecrement(&_providerCount);
    InvalidateRect(_hwnd, NULL, FALSE);
}

void UiaCleanShutdownControl::IncrementTotalRefCount()
{
    InterlockedIncrement(&_providerTotalRefcount);
    InvalidateRect(_hwnd, NULL, FALSE);
}

void UiaCleanShutdownControl::DecrementTotalRefCount()
{
    InterlockedDecrement(&_providerTotalRefcount);
    InvalidateRect(_hwnd, NULL, FALSE);
}

bool UiaCleanShutdownControl::initialized = false;
unsigned int UiaCleanShutdownControl::_controlCount = 0;
