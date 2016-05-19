// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:
//      BasicRecognition.cpp
//
// Description:
//      This program demonstrates how you can build a basic handwriting
//      recognition application using Microsoft Tablet PC Automation API.
//
//      This program co-creates an InkCollector object to enable inking
//      in the window and a default recognition context object.
//      Upon receiving the "Recognize!" command, fired from the application's menu,
//      the collected ink strokes are passed to the recognition context
//      and the best result string is presented in a message box.
//
//      (NOTE: For code simplicity, returned HRESULT is not checked
//             on failure in the places where failures are not critical
//             for the application or very unexpected)
//
//      This application is discussed in the Getting Started guide.
//
//      The interfaces used are:
//      IInkCollector, IInkDisp, IInkStrokes
//      IInkRecognizerContext, IInkRecognitionResult,
//
//--------------------------------------------------------------------------

#include <windows.h>
#include <comdef.h>
#include <msinkaut.h>
#include <msinkaut_i.c>
#include "resource.h"

// Global constants and variables.
const TCHAR*    gc_szAppName = TEXT("Basic Recognition");

// Declare all necessary global interface pointers here
IInkCollector *     g_pIInkCollector    = NULL;
IInkDisp *          g_pIInkDisp         = NULL;
IInkRecognizerContext *   g_pIInkRecoContext  = NULL;

/////////////////////////////////////////////////////////
//
// Cleanup
//
//      This method releases all the COM pointers held by the
//      program. The order of the release does not matter.
//
/////////////////////////////////////////////////////////
void CleanUp()  // Release all objects
{
    if (g_pIInkRecoContext != NULL)
    {
        g_pIInkRecoContext->Release();
        g_pIInkRecoContext = NULL;
    }

    if (g_pIInkDisp != NULL)
    {
        g_pIInkDisp->Release();
        g_pIInkDisp = NULL;
    }

    if (g_pIInkCollector != NULL)
    {
        g_pIInkCollector->Release();
        g_pIInkCollector = NULL;
    }
}

/////////////////////////////////////////////////////////
//
// WndProc
//
// Description of function/method
//        The WindowProc function is an application-defined
//        function that processes messages sent to a window
//
// Parameters:
//        HWND hwnd      : [in] handle to window
//        UINT uMsg      : [in] message identifier
//        WPARAM wParam  : [in] first message parameter
//        LPARAM lParam  : [in] second message parameter
//
// Return Values:
//        The return value is the result of the
//        message processing and depends on the message sent
//
/////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            HRESULT hr;


            // Create a recognition context that uses the default recognizer.
            // The single context will be used for all the recognition.
            hr = CoCreateInstance(CLSID_InkRecognizerContext,
                                  NULL, CLSCTX_INPROC_SERVER,
                                  IID_IInkRecognizerContext,
                                  (void **) &g_pIInkRecoContext);
            if (FAILED(hr))
            {
                ::MessageBox(NULL, TEXT("There are no handwriting recognizers installed.\n"
                     TEXT("You need to have at least one in order to run this sample.\nExiting.")),
                     gc_szAppName, MB_ICONERROR);
                return -1;
            }

            // Create the InkCollector object.
            hr = CoCreateInstance(CLSID_InkCollector,
                                  NULL, CLSCTX_INPROC_SERVER,
                                  IID_IInkCollector,
                                  (void **) &g_pIInkCollector);
            if (FAILED(hr))
                return -1;

            // Get a pointer to the Ink object
            hr = g_pIInkCollector->get_Ink(&g_pIInkDisp);
            if (FAILED(hr))
                return -1;

            // Tell InkCollector the window to collect ink in
            hr = g_pIInkCollector->put_hWnd((long)hwnd);
            if (FAILED(hr))
                return -1;

            // Enable ink input in the window
            hr = g_pIInkCollector->put_Enabled(VARIANT_TRUE);
            if (FAILED(hr))
                return -1;

            break;
        }

        case WM_DESTROY:

            PostQuitMessage(0);
            break;

        case WM_COMMAND:

            if (wParam == ID_CLEAR)
            {
                // Delete all strokes from the Ink
                g_pIInkDisp->DeleteStrokes(0);

                // Update the window
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (wParam == ID_RECOGNIZE)
            {
                // change cursor to the system's Hourglass
                HCURSOR hCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
                // Get a pointer to the ink stroke collection
                // This collection is a snapshot of the entire ink object
                IInkStrokes* pIInkStrokes = NULL;
                HRESULT hr = g_pIInkDisp->get_Strokes(&pIInkStrokes);
                if (SUCCEEDED(hr))
                {
                    // Pass the stroke collection to the recognition context
                    hr = g_pIInkRecoContext->putref_Strokes(pIInkStrokes);
                    if (SUCCEEDED(hr))
                    {
                        // Recognize
                        IInkRecognitionResult* pIInkRecoResult = NULL;
                        InkRecognitionStatus RecognitionStatus;
                        hr = g_pIInkRecoContext->Recognize(&RecognitionStatus, &pIInkRecoResult);
                        if (SUCCEEDED(hr) && (pIInkRecoResult!= NULL))
                        {
                            // Get the best result of the recognition
                            BSTR bstrBestResult = NULL;
                            hr = pIInkRecoResult->get_TopString(&bstrBestResult);
                            pIInkRecoResult->Release();
                            pIInkRecoResult = NULL;

                            // Show the result string
                            if (SUCCEEDED(hr) && bstrBestResult)
                            {
                                MessageBoxW(hwnd, bstrBestResult,
                                            L"Recognition Results", MB_OK);
                                SysFreeString(bstrBestResult);
                            }
                        }
                        // Reset the recognition context
                        g_pIInkRecoContext->putref_Strokes(NULL);
                    }
                    pIInkStrokes->Release();
                }
                // restore the cursor
                ::SetCursor(hCursor);
            }
            else
            {
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
            }
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
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
    WndClassEx.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    WndClassEx.hIconSm       = WndClassEx.hIcon;
    WndClassEx.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WndClassEx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClassEx.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
    WndClassEx.lpszClassName = gc_szAppName;

    if (!RegisterClassEx(&WndClassEx))
    {
        MessageBox(NULL, TEXT("Failed to register window class!"),
                   gc_szAppName, MB_ICONERROR);
        return false;
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
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE /* hPrevInstance */,
                     LPSTR     /* lpCmdLine */,
                     int       nCmdShow)
{
    if (!RegisterWindowClass(hInstance))
        return 0;

    CoInitialize(NULL);

    // Create the application window
    HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, gc_szAppName, gc_szAppName,
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               NULL, NULL, hInstance, NULL);
    if (NULL == hWnd)
    {
        MessageBox(NULL, TEXT("Error creating the window"), TEXT("Error"),
                   MB_OK | MB_ICONINFORMATION);
        return 0;
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
    CoUninitialize();

    return msg.wParam;
}
