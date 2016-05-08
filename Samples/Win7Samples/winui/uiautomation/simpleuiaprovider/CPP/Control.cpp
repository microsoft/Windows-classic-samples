/*************************************************************************************************
 *
 * Description: Implements a simple custom control that supports UI Automation.
 * 
 *  Copyright (C) Microsoft Corporation.  All rights reserved.
 * 
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * 
 *************************************************************************************************/
#include "Control.h"


// Forward declarations.
LRESULT CALLBACK ControlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

/**** CustomButton methods ***/

CustomButton::CustomButton() : 
    m_buttonOn(false), m_provider(NULL), m_InvokedEventId(UiaLookupId(AutomationIdentifierType_Event, &Invoke_Invoked_Event_GUID))
{
}

CustomButton::~CustomButton()
{
    if (m_provider != NULL)
    {
        m_provider->Release();
        m_provider = NULL;
    }
}

IRawElementProviderSimple* CustomButton::GetUIAutomationProvider(HWND hwnd)
{
    if (m_provider == NULL)
    {
        m_provider = new (std::nothrow) Provider(hwnd);
    }
    return m_provider;
}


// Handle button click or invoke.

void CustomButton::InvokeButton(HWND hwnd)
{
    m_buttonOn = ! m_buttonOn;
    SetFocus(hwnd);
    if (UiaClientsAreListening())
    {
        // Raise an event.
        UiaRaiseAutomationEvent(GetUIAutomationProvider(hwnd), m_InvokedEventId);
    }
    InvalidateRect(hwnd, NULL, true);
}

// Ascertain whether button is in the "on" state.

bool CustomButton::IsButtonOn()
{
    return m_buttonOn;
}

// Register the control class.

void CustomButton::RegisterControl(HINSTANCE hInstance)
{
    WNDCLASS wc = {};
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = ControlWndProc;
    wc.hInstance        = hInstance;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName    = L"COLORBUTTON";

    RegisterClass(&wc);
}

// Control window procedure.

LRESULT CALLBACK ControlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_CREATE:
        {
            CustomButton* pControl = new (std::nothrow) CustomButton();
            if (pControl == NULL)
            {
                PostQuitMessage(-1);
            }
            // Save the class instance as extra window data so that members can be accessed
            //  from within this function.
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pControl));
            break;
        }

    case WM_DESTROY:
        {
            CustomButton* pControl = reinterpret_cast<CustomButton*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            delete pControl;
            break;
        }


        // Register with UI Automation.
    case WM_GETOBJECT:
        {
            // If the lParam matches the RootObjectId, send back the RawElementProvider
            if (static_cast<long>(lParam) == static_cast<long>(UiaRootObjectId))
            {
                CustomButton* pControl = reinterpret_cast<CustomButton*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                IRawElementProviderSimple* pRootProvider = pControl->GetUIAutomationProvider(hwnd);
                return UiaReturnRawElementProvider(hwnd, wParam, lParam, pRootProvider);
            }
            return 0;
        }

    case WM_PAINT:
        {
            CustomButton* pControl = reinterpret_cast<CustomButton*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            COLORREF color;
            if (pControl->IsButtonOn())
            {
                color = RGB(128, 0, 0);
            }
            else
            {
                color = RGB(0, 128, 0);
            }
            PAINTSTRUCT paintStruct;
            HDC hdc = BeginPaint(hwnd, &paintStruct);
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);

            // Shrink the colored rectangle so the focus rectangle will be outside it.
            InflateRect(&clientRect, -4, -4);

            // Paint the rectangle.
            HBRUSH brush = CreateSolidBrush(color);
            if (brush != NULL)
            {
                FillRect(hdc, &clientRect, brush);
                DeleteObject(brush);
            }
            EndPaint(hwnd, &paintStruct);
            break;
        }

    case WM_SETFOCUS:
        {
            HDC hdc = GetDC(hwnd);
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            DrawFocusRect(hdc, &clientRect); 
            ReleaseDC(hwnd, hdc);
            break;
        }
    case WM_KILLFOCUS:
        {
            HDC hdc = GetDC(hwnd);
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            DrawFocusRect(hdc, &clientRect); // Erases focus rect if there's one there.
            ReleaseDC(hwnd, hdc);
            break;
        }

    case WM_LBUTTONDOWN:
        {
            CustomButton* pControl = reinterpret_cast<CustomButton*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            pControl->InvokeButton(hwnd);
            break;
        }

    case WM_KEYDOWN:
        {
            switch (wParam)
            {
            case VK_SPACE:
                CustomButton* pControl = reinterpret_cast<CustomButton*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)); 
                pControl->InvokeButton(hwnd);
                break;
            }
            break;
        }
        break;
    }  // switch (message)

    return DefWindowProc(hwnd, message, wParam, lParam);
}
