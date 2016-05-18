// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:
//      MathInputControl.cpp
//
// Description:
//      This program demonstrates how you can:
//          create Math Input Control,
//          set up events between main application and Math Input Control,
//          recognize handwritten math inside the control and 
//          display recognition result inside the text box.
//
//      The interfaces used are:
//      IMathInputControl, _IMathInputControlEvents
//
//--------------------------------------------------------------------------

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

// Windows header files
#include <windows.h>

// Headers for Math Input Control automation interfaces
#include <micaut.h>
#include <micaut_i.c>

// Asserts header
#include "assert.h"
#define ASSERT assert

// The application header files
#include "resource.h"          // main symbols, including command IDs
#include "EventSinks.h"        // defines the IMathInputControlEvents
#include "MathInputControl.h"  // contains the definition of CMathInputControlHost

const WCHAR gc_wszAppName[] = L"Math Input Control SDK Sample Application";

// Global pointer to Math Input Control host object
CMathInputControlHost* g_pMathInputControlHost;

/////////////////////////////////////////////////////////

// CMathInputControlHost implementation /////////////////

/////////////////////////////////////////////////////////
//                                          
// CMathInputControlHost::Init
//
// Initialization of Math Input Control host object consist
// of creation of Math Input Control and Math Input Control
// event listener and advising events from the control to
// the listener.
//
// Parameters:
//        HWND hWnd      : [in] handle to main application window
//        UINT hWndEdit  : [in] handle to edit box control
//
// Return Values (HRESULT):
//      S_OK if initialization succeeded, error code otherwise
//
/////////////////////////////////////////////////////////
HRESULT CMathInputControlHost::Init(HWND hWnd, HWND hWndEdit)
{
    HRESULT hr;

    m_hWnd = hWnd;
    m_hWndEdit = hWndEdit;

    // Create Math Input Control object
    hr = CoCreateInstance(CLSID_MathInputControl, 
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IMathInputControl,
        (void **)&m_pIMathInputControl);

    if (FAILED(hr))
    {
        // There is nothing interesting that application can do without
        // the Math Input Control
        ASSERT("Math Input Control initializion failed" && FALSE);
        return hr;
    }

    // Enlarge Math Input Control and place it near the top/left corner of the screen
    LONG right = mc_left + mc_width;
    LONG bottom = mc_top + mc_height;
    hr = m_pIMathInputControl->SetPosition(mc_left, mc_top, right, bottom);
    if (FAILED(hr))
    {
        ASSERT("Failed to set Math Input Control position." && FALSE);
        return hr;
    }

    // Enable extended set of buttons in Math Input Control
    // (Undo and Redo buttons).
    // Return value is not checked because method is not critical and is
    // unlikely to fail.
    m_pIMathInputControl->EnableExtendedButtons(VARIANT_TRUE);

    m_pEventListener = new CMathInputControlEventListener(this);
    if (!m_pEventListener)
    {
        ASSERT("Failed to create event listener for Math Input Control.");
        return E_FAIL;
    }

    // Establish a connection to the collector's event source.
    // The application will be listening to clear, insert and close events.
    hr = m_pEventListener->AdviseMathInputControl(m_pIMathInputControl);
    if (FAILED(hr))
    {
        // There is nothing interesting that application can do without events
        // from the Math Input Control
        ASSERT("Failed to advise on MIC events" && FALSE);
        return hr;
    }

    return S_OK;
}

/////////////////////////////////////////////////////////
//                                          
// CMathInputControlHost::OnMICInsert
//
// The _IMathInputControlEvents's event handler.
// See the Windows 7 SDK API Reference for the
// detailed description of the event and its parameters.
//
// Parameters:
//
// Return Values (HRESULT):
//      always S_OK
//
/////////////////////////////////////////////////////////
HRESULT CMathInputControlHost::OnMICInsert(
        BSTR bstrRecoResultMathML
        )
{
    if (!m_hWndEdit)
    {
        ASSERT("Edit box control is not initialized." && FALSE);
        return E_UNEXPECTED;
    }

    // Display a recognition result (which is a string in MathML format)
    // in edit box control
    SetWindowText(m_hWndEdit, (LPCWSTR)bstrRecoResultMathML);

    // Hide Math Input Control
    HideMIC();

    return S_OK;
}

/////////////////////////////////////////////////////////
//                                          
// CMathInputControlHost::OnMICClose
//
// In this implementation close is equivalent to hide.
//
// Parameters:
//
// Return Values (HRESULT):
//      S_OK if succeeded, E_FAIL or E_UNEXPECTED otherwise
//
/////////////////////////////////////////////////////////
HRESULT CMathInputControlHost::OnMICClose(void)
{
    // Hide the control. 
    // Alternative implementation might choose to destroy the control
    // at this point and to create it again when needed.
    return HideMIC();
}

/////////////////////////////////////////////////////////
//                                          
// CMathInputControlHost::OnMICClear
//
// Clear the content of the Math Input Control. At the same
// time clear the edit box content.
//
// Parameters:
//
// Return Values (HRESULT):
//      S_OK if succeeded, E_FAIL or E_UNEXPECTED otherwise
//
/////////////////////////////////////////////////////////
HRESULT CMathInputControlHost::OnMICClear(void)
{
    HRESULT hr = S_OK;

    if (!m_pIMathInputControl)
    {
        ASSERT("Math Input Control not initialized" && FALSE);
        return E_UNEXPECTED;
    }

    if (!m_hWndEdit)
    {
        ASSERT("Edit box control is not initialized." && FALSE);
        return E_UNEXPECTED;
    }

    // Get current Math Input Control position and dimensions
    LONG left, right, top, bottom;
    hr = m_pIMathInputControl->GetPosition(&left, &top, &right, &bottom);
    if (FAILED(hr))
    {
        ASSERT("Failed to get minimal window position." && FALSE);
        return E_FAIL;
    }

    // Revert Math Input Control to initial dimensions
    // (autogrow during inking may have enlarged Math Input Control window)
    right = mc_left + mc_width;
    bottom = mc_top + mc_height;
    hr = m_pIMathInputControl->SetPosition(left, top, right, bottom);
    if (FAILED(hr))
    {
        ASSERT("Failed to set window position." && FALSE);
        return E_FAIL;
    }

    // Clear edit box that contains recognition result
    SetWindowText(m_hWndEdit, L"");

    return hr;
}

/////////////////////////////////////////////////////////
//                                          
// CMathInputControlHost::OnMICShow
//
// Show the Math Input Control if it is hidden. Control
// has to be initialized prior to calling show method.
//
// Parameters:
//      None
//
// Return Values (HRESULT):
//      S_OK if succeeded, E_FAIL or E_UNEXPECTED otherwise
//
/////////////////////////////////////////////////////////
LRESULT CMathInputControlHost::OnMICShow()
{
    HRESULT hr = S_OK;

    if (!m_pIMathInputControl)
    {
        ASSERT("Math Input Control not initialized" && FALSE);
        return E_UNEXPECTED;
    }

    // Check visibility
    VARIANT_BOOL vbShown = VARIANT_FALSE;
    hr = m_pIMathInputControl->IsVisible(&vbShown);
    if (FAILED(hr))
    {
        ASSERT("Failed to get visibility" && FALSE);
        return E_FAIL;
    }

    // Show Math Input Control.
    if (vbShown != VARIANT_TRUE)
    {
        hr = m_pIMathInputControl->Show();
        ASSERT("Failed to show Math Input Control window" && SUCCEEDED(hr));
    }

    return hr;
}

/////////////////////////////////////////////////////////
//                                          
// CMathInputControlHost::HideMIC
//
// Hide the Math Input Control window if it is visible. 
// Control is not destroyed.
//
// Return Values (HRESULT):
//      S_OK if succeeded, E_FAIL or E_UNEXPECTED otherwise
//
/////////////////////////////////////////////////////////
HRESULT CMathInputControlHost::HideMIC()
{
    HRESULT hr = S_OK;

    if (!m_pIMathInputControl)
    {
        ASSERT("Math Input Control not initialized" && FALSE);
        return E_UNEXPECTED;
    }

    // Check visibility
    VARIANT_BOOL vbShown = VARIANT_FALSE;
    hr = m_pIMathInputControl->IsVisible(&vbShown);
    if (FAILED(hr))
    {
        ASSERT("Failed to get visibility" && FALSE);
        return E_FAIL;
    }

    // Hide Math Input Control
    if (vbShown == VARIANT_TRUE)
    {
        hr = m_pIMathInputControl->Hide();
        ASSERT("Failed to hide Math Input Control window" && SUCCEEDED(hr));
    }

    return hr;
}

//
// Application
//

/////////////////////////////////////////////////////////
//
// Cleanup
//
//      This method releases all the COM pointers held by the
//      program. The order of the release does not matter.
//
/////////////////////////////////////////////////////////
void CleanUp()
{
    // Release all objects
    if (g_pMathInputControlHost != NULL)
    {
        delete g_pMathInputControlHost;
    }

    CoUninitialize();
}

/////////////////////////////////////////////////////////
//
// WndProc
//
//        The WindowProc function is an application-defined
//        function that processes messages sent to a window
//
// Parameters:
//        HWND hWnd      : [in] handle to window
//        UINT uMsg      : [in] message identifier
//        WPARAM wParam  : [in] first message parameter
//        LPARAM lParam  : [in] second message parameter
//
// Return Values:
//        The return value is the result of the
//        message processing and depends on the message sent
//
/////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        {
            // Resize the edit box control over the whole main window.
            HWND hWndEdit = g_pMathInputControlHost->GetEditWindow();
            MoveWindow(
                hWndEdit,       // Handle to the window.
                0,              // Specifies the new position of the left side of the window.
                0,              // Specifies the new position of the top of the window.
                LOWORD(lParam), // Specifies the new width of the window.
                HIWORD(lParam), // Specifies the new height of the window.
                TRUE            // Specifies whether the window is to be repainted.
                );
        }
        break;

    case WM_COMMAND:
        if (wParam == ID_SHOW)
        {
            g_pMathInputControlHost->OnMICShow();
        }
        else
        {
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

/////////////////////////////////////////////////////////
//
// RegisterWindowClass
//
// The RegisterWindowClass function registers a window class for
// subsequent use in calls to the CreateWindow or CreateWindowEx function.
//
// Parameters:
//        HINSTANCE hInstance : [in] Handle to the instance that
//                              contains the window procedure for the class.
//
// Return Values:
//        TRUE : Success
//        FALSE : Failure to register the class
//
/////////////////////////////////////////////////////////
BOOL RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEX WndClassEx;

    WndClassEx.cbSize        = sizeof(WndClassEx);
    WndClassEx.style         = CS_HREDRAW | CS_VREDRAW;
    WndClassEx.lpfnWndProc   = WndProc;
    WndClassEx.cbClsExtra    = 0;
    WndClassEx.cbWndExtra    = 0;
    WndClassEx.hInstance     = hInstance;
    WndClassEx.hIcon         = NULL;
    WndClassEx.hIconSm       = NULL;
    WndClassEx.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WndClassEx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClassEx.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
    WndClassEx.lpszClassName = gc_wszAppName;

    if (!RegisterClassEx(&WndClassEx))
    {
        MessageBox(NULL, L"Failed to register window class!",
                   gc_wszAppName, MB_ICONERROR);
        false; 
    }

    return true;
}

/////////////////////////////////////////////////////////
//
// WinMain
//
//      The WinMain function is called by the system as the
//      initial entry point for a Win32-based application.
//      It contains typical boilerplate code to create and
//      show the main window, and pump messages.
//
// Parameters:
//      HINSTANCE hInstance,      : [in] handle to current instance
//      HINSTANCE hPrevInstance,  : [in] handle to previous instance
//      LPSTR lpCmdLine,          : [in] command line
//      int nCmdShow              : [in] show state
//
// Return Values:
//      0 : The function terminated before entering the message loop.
//      non zero: Value of the wParam when receiving the WM_QUIT message
//
/////////////////////////////////////////////////////////
int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE /* hPrevInstance */,
                      LPWSTR    /* lpCmdLine */,
                      int       nCmdShow)
{
    if (!RegisterWindowClass(hInstance))
    {
        return 0;
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        CleanUp();
        return 0;
    }

    // Create the application window
    HWND hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,     // Specifies the extended window style of the window being created.
        gc_wszAppName,        // If a string, it specifies the window class name.
        gc_wszAppName,        // Pointer to a null-terminated string that specifies the window name.
        WS_OVERLAPPEDWINDOW,  // Specifies the style of the window being created.
        CW_USEDEFAULT,        // Specifies the initial horizontal position of the window.
        CW_USEDEFAULT,        // Specifies the initial vertical position of the window.
        CW_USEDEFAULT,        // Specifies the width, in device units, of the window.
        CW_USEDEFAULT,        // Specifies the height, in device units, of the window.
        NULL,                 // Handle to the parent or owner window of the window being created.
        NULL,                 // Handle to a menu, or specifies a child-window identifier.
        hInstance,            // Handle to the instance of the module to be associated with the window.
        NULL                  // Pointer to a value to be passed to the window through the CREATESTRUCT structure.
        );

    if (NULL == hWnd)
    {
        MessageBox(NULL, L"Error creating the window", L"Error",
                   MB_OK | MB_ICONINFORMATION);
        CleanUp();
        return 0;
    }

    // Create the edit box inside application windows.
    // Inserted MathML will be placed inside this edit box.
    HWND hWndEdit = CreateWindow(
        L"edit",         // If a string, it specifies the window class name.
        NULL,            // Pointer to a null-terminated string that specifies the window name.
        // Specifies the style of the window being created.
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
        0,               // Specifies the initial horizontal position of the window.
        0,               // Specifies the initial vertical position of the window.
        0,               // Specifies the width, in device units, of the window.
        0,               // Specifies the height, in device units, of the window.
        hWnd,            // Handle to the parent or owner window of the window being created.
        (HMENU)ID_EDIT,  // Handle to a menu, or specifies a child-window identifier 
        hInstance,       // Handle to the instance of the module to be associated with the window.
        NULL             // Pointer to a value to be passed to the window through the CREATESTRUCT structure.
        );

    if (NULL == hWnd)
    {
        MessageBox(NULL, L"Error creating the edit box control", L"Error",
                   MB_OK | MB_ICONINFORMATION);
        CleanUp();
        return 0;
    }

    // Create host object for Math Input Control and Math Input Control event listener
    g_pMathInputControlHost = new CMathInputControlHost();
    if (!g_pMathInputControlHost)
    {
        ASSERT("Failed to create Math Input Control host.");
        CleanUp();
        return -1;
    }

    // Initialize Math Input Control host
    hr = g_pMathInputControlHost->Init(hWnd, hWndEdit);
    if (FAILED(hr))
    {
        ASSERT("Failed to initialize Math Input Control host.");
        CleanUp();
        return -1;
    }

    // Show the main window
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Start the boilerplate message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CleanUp();

    return (int)msg.wParam;
}
