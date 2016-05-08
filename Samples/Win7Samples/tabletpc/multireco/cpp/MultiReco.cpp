// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:
//      MultiReco.cpp
//
// Description:
//      This sample demonstrates such advanced features of the
//      Microsoft Tablet PC Automation API used for handwriting
//      recognition, as
//          - serializing recognition results with a stroke collection
//          - organizing stroke collections into a custom collection
//            within the ink object
//          - serializing ink object into/from an ISF file
//          - enumerating the installed recognizers
//          - creating a recognition context with a specific language
//            recognizer
//          - setting recognizer input guides
//          - synchronous and asynchronous recognition
//
//      This application is discussed in the Getting Started guide.
//
//      (NOTE: For code simplicity, returned HRESULT is not checked
//             for failure in the places where failures are not critical
//             for the application)
//
//      The interfaces used are:
//      IInkRecognizers, IInkRecognizer, IInkRecognizerContext,
//      IInkRecognitionResult, IInkRecognizerGuide,
//      IInkCollector, IInkDisp, IInkRenderer, IInkDrawingAttributes,
//      IInkCustomStrokes, IInkStrokes, IInkStroke
//
// Requirements:
//      One or more handwriting recognizer must be installed on the system.
//      Also, appropriate Asian fonts need to be installed to output the
//      results of the Asian recognizers.
//
//--------------------------------------------------------------------------

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

// Windows header files
#include <windows.h>
#include <commctrl.h>       // need it to call CreateStatusWindow
#include <commdlg.h>

// A useful macro to determine the number of elements in the array
#define countof(array)  (sizeof(array)/sizeof(array[0]))

// ATL header files
#include <atlbase.h>        // defines CComModule, CComPtr, CComVariant
CComModule _Module;
#include <atlwin.h>         // defines CWindowImpl
#include <atlcom.h>         // defines IDispEventSimpleImpl

// Header for the safe string manipulation
#include <strsafe.h>

// Headers for Tablet PC Automation interfaces
#include <msinkaut.h>
#include <msinkaut_i.c>

// The application header files
#include "resource.h"       // main symbols, including command ID's
#include "EventSinks.h"     // defines the IInkEventsImpl and IInkRecognitionEventsImpl
#include "ChildWnds.h"      // definitions of the CInkInputWnd and CRecoOutputWnd
#include "MultiReco.h"      // contains the definition of CAddRecoApp

// The static members of the event sink templates are initialized here
// (defined in EventSinks.h). These _ATL_FUNC_INFO structures contain type information
// used to describe a method or property on a dispinterface.

// Type information of the Recognition event of IInkRecognitionEvents
const _ATL_FUNC_INFO IInkRecognitionEventsImpl<CMultiRecoApp>::mc_AtlFuncInfo =
        {CC_STDCALL, VT_EMPTY, 3, {VT_BSTR, VT_VARIANT, VT_I4}};

// Type information of the Stroke event of IInkCollectorEvents
const _ATL_FUNC_INFO IInkCollectorEventsImpl<CMultiRecoApp>::mc_AtlFuncInfo =
        {CC_STDCALL, VT_EMPTY, 3, {VT_UNKNOWN, VT_UNKNOWN, VT_BOOL|VT_BYREF}};


// The sample uses different colors for different collections of strokes. Below is the palette.
const COLORREF CMultiRecoApp::mc_crColors[] = {
     RGB( 0, 0, 0)/*black*/, RGB(0xA0,0,0)/*red*/, RGB(0,0xA0,0)/*green*/, RGB(0,0,0xA0)/*blue*/,
     RGB(0xA0,0xA0,0)/**/, RGB(0xA0,0,0xA0)/**/, RGB(0,0xA0,0xA0)/**/, RGB(0xA0,0xA0,0xA0)/*gray*/
};

const WCHAR gc_szAppName[] = L"Multiple Language Recognition with Serialization";

/////////////////////////////////////////////////////////
//
// WinMain
//
// The WinMain function is called by the system as the
// initial entry point for a Win32-based application.
//
// Parameters:
//        HINSTANCE hInstance,      : [in] handle to current instance
//        HINSTANCE hPrevInstance,  : [in] handle to previous instance
//        LPSTR lpCmdLine,          : [in] command line
//        int nCmdShow              : [in] show state
//
// Return Values (int):
//        0 : The function terminated before entering the message loop.
//        non zero: Value of the wParam when receiving the WM_QUIT message
//
/////////////////////////////////////////////////////////
int APIENTRY WinMain(
        HINSTANCE hInstance,
        HINSTANCE /*hPrevInstance*/,   // not used here
        LPSTR     /*lpCmdLine*/,       // not used here
        int       nCmdShow
        )
{
    int iRet = 0;

    // Initialize the COM library and the application module
    if (S_OK == ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))
    {
        _Module.Init(NULL, hInstance);

        // Register the common control classes used by the application
        INITCOMMONCONTROLSEX icc;
        icc.dwSize = sizeof(icc);
        icc.dwICC = ICC_BAR_CLASSES;
        if (TRUE == ::InitCommonControlsEx(&icc))
        {
            // Call the boilerplate function of the application
            iRet = CMultiRecoApp::Run(nCmdShow);
        }
        else
        {
            ::MessageBoxW(NULL, L"Error initializing the common controls.",
                         gc_szAppName, MB_ICONERROR | MB_OK);
        }

        // Release the module and the COM library
        _Module.Term();
        ::CoUninitialize();
    }

    return iRet;
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::Run
//
// The static CMultiRecoApp::Run is the boilerplate of the application.
// It instantiates and initializes an CMultiRecoApp object and runs the
// application's message loop.
//
// Parameters:
//      int nCmdShow              : [in] show state
//
// Return Values (int):
//      0 : The function terminated before entering the message loop.
//      non zero: Value of the wParam when receiving the WM_QUIT message
//
/////////////////////////////////////////////////////////
int CMultiRecoApp::Run(
        int nCmdShow
        )
{

    CMultiRecoApp theApp;

    // Load and update the menu before creating the main window.
    // Create menu items for the installed recognizers and for the
    // supported factoids.
    HMENU hMenu = theApp.LoadMenu();
    if (NULL == hMenu)
        return 0;

    int iRet;

    // Load the icon from the resource and associate it with the window class
    WNDCLASSEX& wc = CMultiRecoApp::GetWndClassInfo().m_wc;
    wc.hIcon = wc.hIconSm = ::LoadIcon(_Module.GetResourceInstance(),
                                       MAKEINTRESOURCE(IDR_APPICON));

    // Create the application's main window
    if (theApp.Create(NULL, CWindow::rcDefault, gc_szAppName,
                      WS_OVERLAPPEDWINDOW, 0, (UINT)hMenu) != NULL)
    {
        // Initialize the application object:
        // Create an ink object
        theApp.SendMessage(WM_COMMAND, ID_FILE_NEW);
        // Create a recognition context using the first recognizer in the list
        // and start new stroke collection
        theApp.SendMessage(WM_COMMAND, ID_RECOGNIZER_FIRST);

        // Show and update the main window
        theApp.ShowWindow(nCmdShow);
        theApp.UpdateWindow();

        // Run the boilerplate message loop
        MSG msg;
        while (::GetMessage(&msg, NULL, 0, 0) > 0)
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        iRet = msg.wParam;
    }
    else
    {
        ::MessageBoxW(NULL, L"Error creating the window",
                     gc_szAppName, MB_ICONERROR | MB_OK);
        ::DestroyMenu(hMenu);
        iRet = 0;
    }

    return iRet;
}

// Window message handlers //////////////////////////////

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::OnCreate
//
// This WM_CREATE message handler creates the child windows,
// creates the required objects, sets their attributes,
// and enables pen input.
//
// Parameters:
//      defined in the ATL's macro MESSAGE_HANDLER,
//      none of them is used here
//
// Return Values (LRESULT):
//      0 if succeeded, or -1 otherwise
//
/////////////////////////////////////////////////////////
LRESULT CMultiRecoApp::OnCreate(
        UINT /*uMsg*/,
        WPARAM /*wParam*/,
        LPARAM /*lParam*/,
        BOOL& /*bHandled*/
        )
{
    // Create child windows for ink input and recognition output,
    // listview controls for the lists of gestures, and a status bar
    if (false == CreateChildWindows())
        return -1;

    HRESULT hr;

    // Create an ink collector object.
    hr = m_spIInkCollector.CoCreateInstance(CLSID_InkCollector);
    if (FAILED(hr))
        return -1;

    // Establish a connection to the collector's event source.
    hr = IInkCollectorEventsImpl<CMultiRecoApp>::DispEventAdvise(m_spIInkCollector);
    // There is nothing interesting the application can do without events
    // from the ink collector
    if (FAILED(hr))
        return -1;

    // Enable ink input in the m_wndInput window
    hr = m_spIInkCollector->put_hWnd((long)m_wndInput.m_hWnd);
    if (FAILED(hr))
        return -1;
    hr = m_spIInkCollector->put_Enabled(VARIANT_TRUE);
    if (FAILED(hr))
        return -1;

    // Create recognition guide objects
    CreateRecoGuides();

    return 0;
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::OnDestroy
//
// The WM_DESTROY message handler.
// Used for clean up and also to post a quit message to
// the application itself.
//
// Parameters:
//      defined in the ATL's macro MESSAGE_HANDLER,
//      none is used here
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CMultiRecoApp::OnDestroy(
        UINT /*uMsg*/,
        WPARAM /*wParam*/,
        LPARAM /*lParam*/,
        BOOL& /*bHandled*/
        )
{
    // Disable ink input and release the InkCollector object
    if (m_spIInkCollector != NULL)
    {
        IInkCollectorEventsImpl<CMultiRecoApp>::DispEventUnadvise(m_spIInkCollector);
        m_spIInkCollector->put_Enabled(VARIANT_FALSE);
        m_spIInkCollector.Release();
    }

    // Detach the strokes collection from the recognition context
    // and stop listening to the recognition events
    if (m_spIInkRecoContext != NULL)
    {
        m_spIInkRecoContext->EndInkInput();
        IInkRecognitionEventsImpl<CMultiRecoApp>::DispEventUnadvise(m_spIInkRecoContext);
        m_spIInkRecoContext.Release();
    }

    // Release the other objects and collections
    m_spIInkRecoGuideLined.Release();
    m_spIInkRecoGuideBoxed.Release();
    m_spIInkStrokes.Release();
    m_spIInkRecognizers.Release();

    // Post a WM_QUIT message to the application's message queue
    ::PostQuitMessage(0);

    return 0;
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::OnSize
//
// The WM_SIZE message handler is needed to update
// the layout of the child windows
//
// Parameters:
//      defined in the ATL's macro MESSAGE_HANDLER,
//      wParam of the WN_SIZE message is the only used here.
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CMultiRecoApp::OnSize(
        UINT /*uMsg*/,
        WPARAM wParam,
        LPARAM /*lParam*/,
        BOOL& /*bHandled*/
        )
{
    if (wParam != SIZE_MINIMIZED)
    {
        UpdateLayout();
    }
    return 0;
}


// InkCollector event handlers ///////////////////////////

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::OnStroke
//
// The _IInkCollectorEvents's Stroke event handler.
// See the Tablet PC Automation API Reference for the
// detailed description of the event and its parameters.
//
// Parameters:
//      IInkCursor* pIInkCursor     : [in] not used here
//      IInkStrokeDisp* pInkStroke  : [in] the new stroke's interface pointer
//      VARIANT_BOOL* pbCancel      : [in,out] option to cancel the gesture,
//                                    default value is FALSE, not modified here
//
// Return Values (HRESULT):
//      S_OK if succeeded, E_FAIL or E_INVALIDARG otherwise
//
/////////////////////////////////////////////////////////
HRESULT CMultiRecoApp::OnStroke(
        IInkCursor* /*pIInkCursor*/,
        IInkStrokeDisp* pIInkStroke,
        VARIANT_BOOL* pbCancel
        )
{
    if (m_spIInkStrokes == NULL)    // shouldn't happen
        return E_FAIL;

    if (NULL == pIInkStroke)
        return E_INVALIDARG;

    HRESULT hr;

    // Add the new stroke to the current collection
    hr = m_spIInkStrokes->Add(pIInkStroke);

    if (SUCCEEDED(hr))
    {
        // Cancel the previous background recognition requests
        // which have not been processed yet
        m_spIInkRecoContext->StopBackgroundRecognition();

        // Ask the context to update the recognition results with newly added strokes
        // When the results are ready, the recognition context will return them
        // via the corresponding event RecognitionWithAlternates
        CComVariant vCustomData;
        m_spIInkRecoContext->BackgroundRecognize(vCustomData);
    }
    return hr;
}

// Recognition event handler ////////////////////////////

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::OnRecognition
//
// The _IInkRecognitionEvents's Recognition event handler
//
// Parameters:
//      BSTR bstrRecognizedString : the top result of the recognition
//      VARIANT vCustomParam      : not used here
//      InkRecognitionStatus RecognitionStatus : not used here
//
// Return Values (HRESULT):
//      always S_OK
//
/////////////////////////////////////////////////////////
HRESULT CMultiRecoApp::OnRecognition(
        BSTR bstrRecognizedString,
        VARIANT /*vCustomParam*/,
        InkRecognitionStatus /*RecognitionStatus*/
        )
{
    if (m_wndResults.IsWindow())
    {
        // Update the output window with the new results
        m_wndResults.UpdateString(bstrRecognizedString);
    }

    if (NULL != bstrRecognizedString)
        ::SysFreeString(bstrRecognizedString);

    return S_OK;
}

// Command handlers /////////////////////////////////////

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::OnClear
//
// Deletes all strokes and recognition results from the
// ink object and clears the windows.
//
// Parameters:
//      defined in the ATL's macro COMMAND_RANGE_HANDLER
//      Here only wID - the id of the command associated
//      with the recognizer menu item -  is used.
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CMultiRecoApp::OnClear(
        WORD /*wNotifyCode*/,
        WORD /*wID*/,
        HWND /*hWndCtl*/,
        BOOL& /*bHandled*/
        )
{
    if (m_spIInkDisp == NULL)   // there is nothing to clear
        return 0;

    // Detach the current stroke collection from the recognition context and release it
    if (m_spIInkRecoContext != NULL)
        m_spIInkRecoContext->putref_Strokes(NULL);
    m_spIInkStrokes.Release();

    // Clear the custom strokes collection
    if (m_spIInkCustomStrokes != NULL)
        m_spIInkCustomStrokes->Clear();

    // Delete all strokes from the Ink object
    m_spIInkDisp->DeleteStrokes(NULL);  // Passing NULL as a stroke collection pointer
                                        // means asking to delete all strokes

    // Get a new stroke collection from the ink object

    CComVariant v;  // This varaint is used to pass an array of ID's of the strokes
                    // which to be included into the new collection.
    // Ask for an empty collection by passing an empty variant
    if (SUCCEEDED(m_spIInkDisp->CreateStrokes(v, &m_spIInkStrokes)))
    {
        // Attach it to the recognition context
        if (FAILED(m_spIInkRecoContext->putref_Strokes(m_spIInkStrokes)))
        {
            MessageBoxW(L"Unable to attach the stroke collection to "
                        L"the recognition context",
                        gc_szAppName, MB_ICONERROR | MB_OK);
        }
    }
    else
    {
        MessageBoxW(L"Error creating new stroke collection.",
                    gc_szAppName, MB_ICONERROR | MB_OK);
    }

    // Update the child windows
    m_wndInput.Invalidate();
    m_wndResults.ResetResults();    // resets the text and updates the window

    // Tell the results window that a new stroke collection
    // is started, and set the color for it.
    m_wndResults.AddString(
            mc_crColors[m_iColor],  // new current color
            NULL                    // recognition result string, none yet
            );

    return 0;
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::OnNewStrokes
//
// This command handler is called when user selects a recognizer
// in the "New Strokes" submenu. It saves the current stroke collection,
// creates a new recognition context if user selected a different language
// recognizer, and then starts a new collection of strokes attached to
// the new recognition context.
//
// Parameters:
//      defined in the ATL's macro COMMAND_RANGE_HANDLER
//      Here only wID - the id of the command associated
//      with the recognizer menu item -  is used.
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CMultiRecoApp::OnNewStrokes(
        WORD /*wNotifyCode*/,
        WORD wID,
        HWND /*hWndCtl*/,
        BOOL& /*bHandled*/
        )
{
    // Change cursor to the system's Hourglass
    HCURSOR hCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));

    // Save the current stroke collection if there is any
    if (m_spIInkRecoContext != NULL)
    {
        // Cancel the previous background recognition requests
        // which have not been processed yet
        m_spIInkRecoContext->StopBackgroundRecognition();

        // Let the context know that there'll be no more input
        // for the attached stroke collection
        m_spIInkRecoContext->EndInkInput();

        // Add the stroke collection to the Ink object's CustomStrokes collection
        SaveStrokeCollection();
    }

    // If a different recognizer was selected, create a new recognition context
    // Else, reuse the same recognition context
    if (wID != m_nCmdRecognizer)
    {
        // Get a pointer to the recognizer object from the recognizer collection
        CComPtr<IInkRecognizer> spIInkRecognizer;
        if ((m_spIInkRecognizers == NULL)
            || FAILED(m_spIInkRecognizers->Item(wID - ID_RECOGNIZER_FIRST,
                                                &spIInkRecognizer))
            || (false == CreateRecoContext(spIInkRecognizer)))
        {
            // restore the cursor
            ::SetCursor(hCursor);
            return 0;
        }

        // Update the status bar
        m_bstrCurRecoName.Empty();
        spIInkRecognizer->get_Name(&m_bstrCurRecoName);
        UpdateStatusBar();

        // Store the selected recognizer's command id
        m_nCmdRecognizer = wID;
    }

    // Create a new stroke collection
    StartNewStrokeCollection();

    // restore the cursor
    ::SetCursor(hCursor);

    return 0;
}

// Helper methods //////////////////////////////

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::StartNewStrokeCollection
//
// Creates an empty stroke collection and attaches it
// to the recognition context.
//
// Parameters: none
//
// Return Values (void):
//      ignore failures
//
/////////////////////////////////////////////////////////
void CMultiRecoApp::StartNewStrokeCollection()
{
    if ((m_spIInkDisp == NULL) || (m_spIInkRecoContext == NULL))
        return;

    // Create a new empty stroke collection and attach it to the recognition context
    CComVariant vt(0);
    if (SUCCEEDED(m_spIInkDisp->CreateStrokes(vt, &m_spIInkStrokes)))
    {
        if (FAILED(m_spIInkRecoContext->putref_Strokes(m_spIInkStrokes)))
        {
            MessageBoxW(L"Failed to attach the stroke collection to "
                        L"the recognition context",
                        gc_szAppName, MB_ICONERROR | MB_OK);
        }

        // Change the color of the pen and text output
        CComPtr<IInkDrawingAttributes> spIInkDrawAttrs;
        if (SUCCEEDED(m_spIInkCollector->get_DefaultDrawingAttributes(&spIInkDrawAttrs)))
        {
            spIInkDrawAttrs->put_Color(mc_crColors[m_iColor]);
        }
        // Tell the output window that the new stroke collection is current now and
        // it's future recognition results should be shown in the RichEdit control
        // with a different color
        m_wndResults.AddString(
                mc_crColors[m_iColor],  // new current color
                NULL                    // recognition result string, none yet
                );
    }
    else
    {
        MessageBoxW(L"Error creating new stroke collection.",
                    gc_szAppName, MB_ICONERROR | MB_OK);
    }
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::LoadMenu
//
// This method instantiates an enumerator object for the installed
// recognizers, loads the main menu resource and creates a menu item
// for the recognizers from the collection.
//
// Parameters:
//      none
//
// Return Values (HMENU):
//      The return value is a handle of the menu
//      that'll be used for the main window
//
/////////////////////////////////////////////////////////
HMENU CMultiRecoApp::LoadMenu()
{
    HRESULT hr = S_OK;

    // Create the enumerator for the installed recognizers
    hr = m_spIInkRecognizers.CoCreateInstance(CLSID_InkRecognizers);
    if (FAILED(hr))
        return NULL;

    // Get the number of the recognizers known to the system
    LONG lCount = 0;
    hr = m_spIInkRecognizers->get_Count(&lCount);
    if (0 == lCount)
    {
        ::MessageBoxW(NULL, L"There are no handwriting recognizers installed.\n"
             L"You need to have at least one in order to run this sample.\nExiting.",
             gc_szAppName, MB_ICONERROR | MB_OK);
        return NULL;
    }

    // Load the menu of the main window
    HMENU hMenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MENU));
    if (NULL == hMenu)
        return NULL; // Not normal

    MENUITEMINFOW miinfo;
    miinfo.cbSize = sizeof(miinfo);
    miinfo.fMask = MIIM_ID | MIIM_STATE | MIIM_TYPE;
    miinfo.fType = MFT_STRING;

    // Get the submenu id for the recognizers menu.
    // This submenu will contain the list of installed recognizers
    HMENU hSubMenu = ::GetSubMenu(hMenu, mc_iSubmenuRecognizers);
    if (NULL == hSubMenu)
    {
        ::DestroyMenu(hMenu);
        return NULL; // Not normal
    }

    // Remove the placeholding separator
    ::DeleteMenu(hSubMenu, 0, MF_BYPOSITION);

    CComPtr<IInkRecognizer> spIInkRecognizer;

    // Make sure that the number of menu items won't exceed
    // the predefined range of the corresponding command ID's
    if (lCount > (ID_RECOGNIZER_LAST - ID_RECOGNIZER_FIRST + 1))
        lCount = ID_RECOGNIZER_LAST - ID_RECOGNIZER_FIRST + 1;

    miinfo.fState = 0;
    bool bAllFailed = true;
    for (LONG i = 0; i < lCount; i++)
    {
        if (FAILED(m_spIInkRecognizers->Item(i, &spIInkRecognizer)))
            continue;

        // Filter out non-language recognizers by checking for
        // the languages supported by the recognizer - there'll be
        // none if it's a gesture or object recognizer.
        CComVariant vLanguages;
        if (SUCCEEDED(spIInkRecognizer->get_Languages(&vLanguages)))
        {
            if ((VT_ARRAY == (VT_ARRAY & vLanguages.vt))            // it should be an array
                && (NULL != vLanguages.parray)
                && (0 < vLanguages.parray->rgsabound[0].cElements)) // with at least one element
            {
                // This is a language recognizer. Add its name to the menu.
                CComBSTR bstrName;
                if (SUCCEEDED(spIInkRecognizer->get_Name(&bstrName)))
                {
                    // The ID_RECOGNIZER_FIRST is defined in the resource.h file.
                    // It's the base id for the commands that fill this submenu
                    miinfo.wID = ID_RECOGNIZER_FIRST + i;
                    miinfo.dwTypeData = bstrName;
                    if ((::InsertMenuItemW(hSubMenu, (UINT)-1, TRUE, &miinfo) != 0)
                        && (true == bAllFailed))
                    {
                        bAllFailed = false;
                    }
                }
            }
        }
        spIInkRecognizer.Release();
    }

    if (bAllFailed)
    {
        ::MessageBoxW(
            NULL,
            L"Unable to find or load handwriting text recognizers.\n"
            L"You need to have at least one in order to run this sample.\nExiting.",
            gc_szAppName,
            MB_ICONERROR | MB_OK);

        ::DestroyMenu(hMenu);

        return NULL;
    }

    return hMenu;
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::DoOpen
//
// This helper function creates new ink object or takes
// one loaded from a file and attaches it to the ink collector,
// obtains a pointer to the custom strokes of the ink
// and populates the stored recognition results from the stroke
// collections.
//
// Parameters:
//      bool bNew : true - create new ink object
//                  false - let the user pick an .isf file
//                  and load ink object from it.
//
// Return Values (bool):
//      true - new ink object created or loaded from a file
//      false - if failed to create ink object, or the user
//              canceled the "Open" dialog box
//
/////////////////////////////////////////////////////////
bool CMultiRecoApp::DoOpen(
        bool bNew
        )
{
    // Disable inking
    m_spIInkCollector->put_Enabled(VARIANT_FALSE);

    // Create a new ink object
    CComPtr<IInkDisp> spIInkDisp;
    if (FAILED(spIInkDisp.CoCreateInstance(CLSID_InkDisp)))
    {
        MessageBoxW(L"Unable to create an ink object.",
                    gc_szAppName, MB_ICONERROR | MB_OK);
        // Enable inking
        m_spIInkCollector->put_Enabled(VARIANT_TRUE);
        return false;
    }

    // Let user select an .isf file to open
    if (false == bNew)
    {
        if (false == LoadFile(spIInkDisp))
        {
            // Don't proceed if opening a file or reading from it failed
            // or if the user canceled the operation
            // Enable inking
            m_spIInkCollector->put_Enabled(VARIANT_TRUE);
            return false;
        }
    }
    else
    {
        // Reset the file name and update the window's title
        UpdateFilename();
    }

    // Replace the old Ink object with the new one
    m_spIInkDisp.Attach(spIInkDisp.Detach());   // Attach/Detach don't call AddRef/Release
    m_spIInkCollector->putref_Ink(m_spIInkDisp);

    // Release the current stroke collections
    m_spIInkStrokes.Release();
    m_spIInkCustomStrokes.Release();
    m_wndResults.ResetResults();

    // Obtain a pointer to the custom strokes collection of the new ink object
    long lCount = 0;
    if (SUCCEEDED(m_spIInkDisp->get_CustomStrokes(&m_spIInkCustomStrokes)))
    {
        m_spIInkCustomStrokes->get_Count(&lCount);
        for (CComVariant vItem(0); vItem.lVal < lCount; vItem.lVal++)
        {
            IInkStrokes* pIInkStrokes;
            if (SUCCEEDED(m_spIInkCustomStrokes->Item(vItem, &pIInkStrokes)))
            {
                // Get the color of the strokes or use GRAY if fail
                COLORREF clr = RGB(0x80,0x80,0x80);
                CComPtr<IInkStrokeDisp> spIInkStroke;
                CComPtr<IInkDrawingAttributes> spIDrawAttrs;
                if (SUCCEEDED(pIInkStrokes->Item(0, &spIInkStroke))
                    && SUCCEEDED(spIInkStroke->get_DrawingAttributes(&spIDrawAttrs)))
                {
                    spIDrawAttrs->get_Color((long*)&clr);
                }
                // Get the best result string
                CComBSTR bstrToString;
                if (SUCCEEDED(pIInkStrokes->ToString(&bstrToString)))
                {
                    // Add the string to the RichEdit control
                    m_wndResults.AddString(
                            clr,                // the strokes color
                            bstrToString        // the recognition result string
                            );
                }
                pIInkStrokes->Release();
            }
        }

    }

    // Create an empty new stroke collection and attach it to the recognition context
    m_iColor = WORD(lCount) % countof(mc_crColors);   // next color to use
    StartNewStrokeCollection();

    // Reset the Dirty flag
    m_spIInkDisp->put_Dirty(VARIANT_FALSE);

    // Update the input window
    m_wndInput.Invalidate();

    // Enable inking
    m_spIInkCollector->put_Enabled(VARIANT_TRUE);
    return true;
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::DoSave
//
// This function saves the current ink object into file.
//
//
// Parameters:
//      bool bSaveAs:  true - open "Save As..." common dialog box
//                            for the user to specify the filename
//                     false - save changes into the current file
//      bool bAskUser: true - ask user whether she'd like to save
//                            the changes; it's used when DoSave
//                            is called from ID_FILE_NEW or ID_FILE_OPEN
//                            command handlers
//                     false - don't ask, just save
//
// Return Values (bool):
//      true    - succeeded or there's nothing to save or the user was
//                was asked and said "No" to saving
//      false   - failed to save or canceled by the user
//
/////////////////////////////////////////////////////////
bool CMultiRecoApp::DoSave(
        bool bSaveAs,
        bool bAskUser
        )
{
    if (m_spIInkDisp == NULL)
        return true;

    // If there's been no changes since the last save,
    // and it's not a SaveAs command, do nothing and return true
    VARIANT_BOOL bDirty = VARIANT_TRUE;
    if ((false == bSaveAs)
        && SUCCEEDED(m_spIInkDisp->get_Dirty(&bDirty))
        && VARIANT_FALSE == bDirty)
    {
        return true;
    }

    if (true == bAskUser)
    {
        int iRet = MessageBoxW(L"The ink object has been changed.\n"
                               L"Do you want to save the changes?",
                               gc_szAppName, MB_ICONQUESTION | MB_YESNOCANCEL);
        if (IDCANCEL == iRet)
            return false;
        if (IDNO == iRet)
            return true;
    }

    WCHAR wchFile[MAX_PATH];
    WCHAR wchFileTitle[MAX_PATH];
    wchFile[0] = wchFileTitle[0] = 0;

    if (false == bSaveAs && 0 == m_wchFile[0])
        bSaveAs = true;

    // If the function is called from the ID_FILE_SAVEAS command handler,
    // or the file is new, display the "SaveAs" common dialog box
    // for the user to specify the file name
    if (true == bSaveAs)
    {
        // Initialize the OPENFILENAME members.
        OPENFILENAMEW ofn;
        memset(&ofn, 0, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrDefExt = L"isf";
        ofn.lpstrFilter = L"*.isf\0";
        ofn.lpstrFileTitle= wchFileTitle;
        ofn.nMaxFileTitle = countof(wchFileTitle);
        ofn.lpstrFile= wchFile;
        ofn.nMaxFile = countof(wchFile);
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

        // Display the dialog box
        if (::GetSaveFileNameW(&ofn) == 0)
            return false;   // canceled by the user
    }

    // Add the current stroke collection to the Ink's CustomStrokes and release it
    SaveStrokeCollection();

    // Save the ink object into a variant using the ISF format
    bool bOk = false;
    CComVariant vData;
    if (SUCCEEDED(m_spIInkDisp->Save(
                         IPF_InkSerializedFormat, // save as ISF
                         IPCM_Default,            // use default compression
                         &vData)                  // the output
                         ))
    {
        BYTE* pbData;
        if (SUCCEEDED(::SafeArrayAccessData(vData.parray, (void**)&pbData)))
        {
            // Create new or overwrite the existing file
            HANDLE hFile = ::CreateFileW(
                                bSaveAs ? wchFile : m_wchFile,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                0,
                                NULL);
            if (INVALID_HANDLE_VALUE != hFile)
            {
                // Write the data into the file
                ULONG ulWritten, ulSize = vData.parray->rgsabound[0].cElements;
                if (TRUE == ::WriteFile(hFile, pbData, ulSize, &ulWritten, NULL))
                {
                    bOk = true;
                    m_spIInkDisp->put_Dirty(VARIANT_FALSE);
                }
                else
                {
                    MessageBoxW(L"Error writing file!",
                                gc_szAppName, MB_ICONERROR | MB_OK);
                }
                ::CloseHandle(hFile);

                // Update the filename string
                if (true == bSaveAs)
                {
                    UpdateFilename(wchFile, wchFileTitle);
                }
            }
            else
            {
                MessageBoxW(L"Unable to create the file.",
                            gc_szAppName, MB_ICONERROR | MB_OK);
            }
            ::SafeArrayUnaccessData(vData.parray);
        }
    }

    return bOk;
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::LoadFile
//
// Loads the ink object from the specified file.
//
// Parameters:
//      IInkDisp* pIInkDisp : pointer to the ink object
//
// Return Values (bool):
//      true    - succeeded
//      false   - failed
//
/////////////////////////////////////////////////////////
bool CMultiRecoApp::LoadFile(
        IInkDisp* pIInkDisp
        )
{
    WCHAR wchFile[MAX_PATH];
    WCHAR wchFileTitle[MAX_PATH];
    wchFile[0] = wchFileTitle[0] = 0;

    // Initialize the OPENFILENAME members.
    OPENFILENAMEW ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = L"*.isf\0";
    ofn.lpstrFileTitle= wchFileTitle;
    ofn.nMaxFileTitle = countof(wchFileTitle);
    ofn.lpstrFile= wchFile;
    ofn.nMaxFile = countof(wchFile);
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    // Display the "Open File..." common dialog box
    if (::GetOpenFileNameW(&ofn) == 0)
    {
        return false;
    }

    // Open the file and populate the Ink object from it
    HANDLE hFile = ::CreateFileW(wchFile, GENERIC_READ, 0, NULL,
                                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        MessageBoxW(L"Error opening the file!", gc_szAppName, MB_ICONERROR | MB_OK);
        return false;
    }

    // Load the file into a safearrray of bytes, then load the ink object from the array
    bool bOk = false;
    DWORD dwRead, dwSize;
    dwSize = ::GetFileSize(hFile, NULL);
    if (dwSize != DWORD(-1))
    {
        VARIANT vtData;
        VariantInit(&vtData);
        vtData.vt = VT_ARRAY | VT_UI1;
        vtData.parray = ::SafeArrayCreateVector(VT_UI1, 0, dwSize);
        if (vtData.parray)
        {
            BYTE *pbData;
            if (SUCCEEDED(::SafeArrayAccessData(vtData.parray, (void**)&pbData)))
            {
                if (TRUE == ::ReadFile(hFile, pbData, dwSize, &dwRead, NULL)
                    && SUCCEEDED(pIInkDisp->Load(vtData)))
                {
                    bOk = true;
                }
                ::SafeArrayUnaccessData(vtData.parray);
            }
            ::SafeArrayDestroy(vtData.parray);
        }

        if (false == bOk)
        {
            MessageBoxW(L"Error loading ink object from the file.",
                        gc_szAppName, MB_ICONERROR | MB_OK);
        }
    }
    else
    {
        MessageBoxW(L"Error reading data from the file.\nUnknown file format.",
                    gc_szAppName, MB_ICONERROR | MB_OK);
    }

    // Close the file
    ::CloseHandle(hFile);

    // Store the filenames and update the title
    if (true == bOk)
    {
        UpdateFilename(wchFile, wchFileTitle);
    }

    return bOk;
}


/////////////////////////////////////////////////////////
//
// CMultiRecoApp::CreateRecoGuides
//
// Create recognizer guides for boxed and lined input.
// They are not required but recommended for getting
// better recognition results.
//
// Parameters: none
//
// Return Values (void):
//      the result is not important - the application just
//      won't use the guides if failed to create or initialize them
//
/////////////////////////////////////////////////////////
void CMultiRecoApp::CreateRecoGuides()
{
    CComPtr<IInkRenderer> spIInkRenderer;
    if ((m_spIInkCollector == NULL)
        || FAILED(m_spIInkCollector->get_Renderer(&spIInkRenderer)))
        return;

    HDC hdc = m_wndInput.GetDC();
    if (NULL == hdc)
        return;

    // Initialize the recognizer guide data structure
    _InkRecoGuide irg;
    irg.midline = -1;                   // not use midline
    irg.cRows = mc_iNumRowsCols;
    irg.cColumns = mc_iNumRowsCols;
    ::SetRect(&irg.rectWritingBox, 0, 0, mc_iGuideColWidth, mc_iGuideRowHeight);
    // Make the guide's DrawnBox a little smaller than the the actual area
    // in which a user is expected to draw the ink (WritingBox)
    irg.rectDrawnBox = irg.rectWritingBox;
    ::InflateRect(&irg.rectDrawnBox, -mc_cxBoxMargin, -mc_cyBoxMargin);

    // Set the m_wndInput's guide data with screen coordinates
    m_wndInput.SetGuide(irg);

    // Use IInkRenderer to transform the guide boxes
    // rectangles into ink space coordinates
    // - the guide's writing box
    if (   SUCCEEDED(spIInkRenderer->PixelToInkSpace((long)hdc,
                                                     &irg.rectWritingBox.left,
                                                     &irg.rectWritingBox.top))
        && SUCCEEDED(spIInkRenderer->PixelToInkSpace((long)hdc,
                                                     &irg.rectWritingBox.right,
                                                     &irg.rectWritingBox.bottom))
        // and the drawn box
        && SUCCEEDED(spIInkRenderer->PixelToInkSpace((long)hdc,
                                                     &irg.rectDrawnBox.left,
                                                     &irg.rectDrawnBox.top))
        && SUCCEEDED(spIInkRenderer->PixelToInkSpace((long)hdc,
                                                     &irg.rectDrawnBox.right,
                                                     &irg.rectDrawnBox.bottom))
        )
    {
        // Create the boxed guide
        if (SUCCEEDED(m_spIInkRecoGuideBoxed.CoCreateInstance(CLSID_InkRecognizerGuide)))
        {
            // Initialize it
            if (FAILED(m_spIInkRecoGuideBoxed->put_GuideData(irg)))
            {
                m_spIInkRecoGuideBoxed.Release();
            }
        }

        // Create the lined guide
        if (SUCCEEDED(m_spIInkRecoGuideLined.CoCreateInstance(CLSID_InkRecognizerGuide)))
        {
            // Initialize it
            irg.cColumns = 0;  // no columns, just rows
            if (FAILED(m_spIInkRecoGuideLined->put_GuideData(irg)))
            {
                m_spIInkRecoGuideLined.Release();
            }
        }
    }

    // Clean up
    ReleaseDC(hdc);
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::SaveStrokeCollection
//
// The method finalize the recognition of the current
// stroke collection and saves the best result with it.
// Then this stroke collection is added to the custom strokes
// collection of the ink object.
//
// Parameters:
//      none
//
// Return Values (void):
//      none - ignore any failures
//
/////////////////////////////////////////////////////////
void CMultiRecoApp::SaveStrokeCollection()
{
    if (m_spIInkStrokes == NULL)
        return;

    long lCount = 0;
    if (m_spIInkRecoContext != NULL)
    {
        if (SUCCEEDED(m_spIInkStrokes->get_Count(&lCount)) && 0 != lCount)
        {
            CComPtr<IInkRecognitionResult> spIInkRecoResult;
            InkRecognitionStatus RecognitionStatus;
            if (SUCCEEDED(m_spIInkRecoContext->Recognize(&RecognitionStatus, &spIInkRecoResult)) && (spIInkRecoResult != NULL))
            {
                if (SUCCEEDED(spIInkRecoResult->SetResultOnStrokes()))
                {
                    CComBSTR bstr;
                    spIInkRecoResult->get_TopString(&bstr);
                    m_wndResults.UpdateString(bstr);
                }
                else
                {
                    MessageBoxW(L"Unable to save recognition result "
                                L"in the stroke collection.",
                                gc_szAppName, MB_ICONERROR | MB_OK);
                }
            }
            else
            {
                MessageBoxW(L"Recognition has failed. No results will be "
                            L"stored in the stroke collection.",
                            gc_szAppName, MB_ICONERROR | MB_OK);
            }
        }
        // Detach the stroke collection from the old recognition context
        m_spIInkRecoContext->putref_Strokes(NULL);
    }

    // Now add it to the ink's custom strokes collection
    // Each item (stroke collection) of the custom strokes must be identified
    // by a unique string. Here we generate a GUID for this.
    if ((0 != lCount) && (m_spIInkCustomStrokes != NULL))
    {
        GUID guid;
        WCHAR szGuid[40]; // format: "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"
        if (SUCCEEDED(::CoCreateGuid(&guid))
            && (::StringFromGUID2(guid, szGuid, countof(szGuid)) != 0))
        {
            CComBSTR bstrGuid(szGuid);
            if (FAILED(m_spIInkCustomStrokes->Add(bstrGuid, m_spIInkStrokes)))
            {
                MessageBoxW(L"Failed to add the strokes to the Ink object's "
                            L"custom stroke collection",
                            gc_szAppName, MB_ICONERROR | MB_OK);
            }
        }
        else
        {
            MessageBoxW(L"Failed to create a unique string id for the stroke collection",
                        gc_szAppName, MB_ICONERROR | MB_OK);
        }

        // Use another color for the next collection
        m_iColor++;
        if (m_iColor >= countof(mc_crColors))
        {
            m_iColor= 0;
        }
    }

    m_spIInkStrokes.Release();
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::CreateRecoContext
//
// As it follows from the name, this helper method, given
// a pointer to the recognizer, creates and initializes
// new recognition context.
//
// Parameters:
//      IInkRecognizer* pIInkRecognizer
//              : [in] the recognizer to create a context with
//
// Return Values (bool):
//      true if succeeded creating new recognition context, false otherwise
//
/////////////////////////////////////////////////////////
bool CMultiRecoApp::CreateRecoContext(
        IInkRecognizer* pIInkRecognizer
        )
{
    // Create a new recognition context
    CComPtr<IInkRecognizerContext> spNewContext;
    if (FAILED(pIInkRecognizer->CreateRecognizerContext(&spNewContext)))
        return false;

    // Replace the current context with the new one
    if (m_spIInkRecoContext != NULL)
    {
        // Close the connection to the recognition events source
        IInkRecognitionEventsImpl<CMultiRecoApp>::DispEventUnadvise(m_spIInkRecoContext);
    }
    m_spIInkRecoContext.Attach(spNewContext.Detach());

    // Establish a connection with the recognition context's event source
    if (FAILED(IInkRecognitionEventsImpl<CMultiRecoApp>::DispEventAdvise(m_spIInkRecoContext)))
    {
        MessageBoxW(L"Failed connect to the recognition context's event source.\n"
                    L"The input won't be recognized dynamically.",
                    gc_szAppName, MB_ICONERROR | MB_OK);
    }

    // Set the guide if it's supported by the recognizer and has been created
    int cRows = 0, cColumns = 0;
    bool fRequiresBoxGuide = false;
    InkRecognizerCapabilities dwCapabilities = IRC_DontCare;
    if (SUCCEEDED(pIInkRecognizer->get_Capabilities(&dwCapabilities)))
    {
        // Find out the language chosen. In the case of Japanese, Chinese and Korean we will
        // create a box guide.
        CComVariant vLanguages;
        if (SUCCEEDED(pIInkRecognizer->get_Languages(&vLanguages)))
        {
            if ((VT_ARRAY == (VT_ARRAY & vLanguages.vt))            // it should be an array
                && (NULL != vLanguages.parray)
                && (0 < vLanguages.parray->rgsabound[0].cElements)) // with at least one element
            {
                ULONG ulSize = vLanguages.parray->rgsabound[0].cElements;
                short *pbData;
                if (SUCCEEDED(::SafeArrayAccessData(vLanguages.parray, (void**)&pbData)))
                {
                    for(ULONG i = 0; i < ulSize; i++)
                    {
                        if (  0x411 == pbData[i] ||   // Japanese
                              0x404 == pbData[i] ||   // Chinese
                              0x804 == pbData[i] ||   // Chinese
                              0x412 == pbData[i])     // Korean
                        {
                            fRequiresBoxGuide = true;
                            break;
                        }
                    }
                    ::SafeArrayUnaccessData(vLanguages.parray);
                }
            }
        }
        // Try the boxed guide in case it is required
        if (((IRC_BoxedInput & dwCapabilities) == IRC_BoxedInput)
            && (m_spIInkRecoGuideBoxed != NULL) && fRequiresBoxGuide)
        {
            HRESULT hr = m_spIInkRecoContext->putref_Guide(m_spIInkRecoGuideBoxed);
            if (SUCCEEDED(hr))
            {
                cRows = cColumns = mc_iNumRowsCols;
            }
        }
        // or the lined one
        else if (((IRC_LinedInput & dwCapabilities) == IRC_LinedInput)
            && (m_spIInkRecoGuideLined != NULL))
        {
            if (SUCCEEDED(m_spIInkRecoContext->putref_Guide(m_spIInkRecoGuideLined)))
            {
                cRows = mc_iNumRowsCols;
            }
        }
    }
    // Update the input window
    m_wndInput.SetRowsCols(cRows, cColumns);

    return true;
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::CreateChildWindows
//
// This helper method is called from WM_CREATE message handler.
// The child windows and controls are created and initialized here.
//
// Parameters:
//      none
//
// Return Values (bool):
//      true if the windows have been created successfully,
//      false otherwise
//
/////////////////////////////////////////////////////////
bool CMultiRecoApp::CreateChildWindows()
{
    // Create windows for input and output
    if ((m_wndResults.Create(m_hWnd, mc_iOutputWndId) == NULL)
        || (m_wndInput.Create(m_hWnd, CWindow::rcDefault, NULL,
                             WS_CHILD, WS_EX_CLIENTEDGE, (UINT)mc_iInputWndId) == NULL))
    {
        return false;
    }

    // Create a status bar (Ignore if it fails, the application can live without it).
    m_hwndStatusBar = ::CreateStatusWindow(
                        WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|SBARS_SIZEGRIP,
                        NULL, m_hWnd, (UINT)mc_iStatusWndId);

    if (NULL != m_hwndStatusBar)
    {
        // Set the status bar font
        ::SendMessage(m_hwndStatusBar,
                      WM_SETFONT,
                      (LPARAM)::GetStockObject(DEFAULT_GUI_FONT),
                      FALSE);
    }

    // Update the child windows' positions and sizes so that they cover
    // entire client area of the main window.
    UpdateLayout();

    return true;
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::UpdateLayout
//
// This helper method is called when the size of the main
// window has been changed and the child windows' positions
// need to be updated so that they cover entire client area
// of the main window.
//
// Parameters:
//      none
//
// Return Values (void):
//      none
//
/////////////////////////////////////////////////////////
void CMultiRecoApp::UpdateLayout()
{
    RECT rect;
    GetClientRect(&rect);

    // update the size and position of the status bar
    if (::IsWindow(m_hwndStatusBar)
        && ((DWORD)::GetWindowLong(m_hwndStatusBar, GWL_STYLE) & WS_VISIBLE))
    {
        ::SendMessage(m_hwndStatusBar, WM_SIZE, 0, 0);
        RECT rectStatusBar;
        ::GetWindowRect(m_hwndStatusBar, &rectStatusBar);
        if (rect.bottom > rectStatusBar.bottom - rectStatusBar.top)
        {
            rect.bottom -= rectStatusBar.bottom - rectStatusBar.top;
        }
        else
        {
            rect.bottom = 0;
        }
    }

    // update the size and position of the output window
    if (m_wndResults.IsWindow())
    {
        int cyResultsWnd = 100;
        if (cyResultsWnd > rect.bottom)
        {
            cyResultsWnd = rect.bottom;
        }
        ::SetWindowPos(m_wndResults.m_hWnd, NULL,
                       rect.left, rect.bottom - cyResultsWnd,
                       rect.right - rect.left, cyResultsWnd - rect.top,
                       SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        rect.bottom -= cyResultsWnd;
    }

    // update the size and position of the ink input window
    if (::IsWindow(m_wndInput.m_hWnd))
    {
        ::SetWindowPos(m_wndInput.m_hWnd, NULL,
                       rect.left, rect.top,
                       rect.right - rect.left, rect.bottom - rect.top,
                       SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::UpdateStatusBar
//
// This helper function sets the name of the currently selected
// recognizer to appear on the application's status bar.
//
// Parameters:
//      none
//
// Return Values (void):
//      none
//
/////////////////////////////////////////////////////////
void CMultiRecoApp::UpdateStatusBar()
{
    if (::IsWindow(m_hwndStatusBar))
    {
        ::SendMessage(m_hwndStatusBar, SB_SETTEXTW, NULL, (LPARAM)m_bstrCurRecoName.m_str);
    }
}

/////////////////////////////////////////////////////////
//
// CMultiRecoApp::UpdateFilename
//
// This helper function stores the new file name
// and update the window's caption
//
// Parameters:
//      WCHAR   pwsFile      : the fule name of the opened file
//      WCHAR   pwsFileTitle : the name and extension of the file, without path
//
// Return Values (void):
//      none
//
/////////////////////////////////////////////////////////
void CMultiRecoApp::UpdateFilename(
        WCHAR* pwsFile,
        WCHAR* pwsFileTitle
        )
{
    // The title that appears in the window's caption bar
    CComBSTR bstrTitle;

    if ((NULL != pwsFile) && (NULL != pwsFileTitle))
    {
        // Copy the strings
        StringCchCopyNExW(m_wchFile, MAX_PATH, pwsFile, MAX_PATH, NULL, NULL, STRSAFE_NULL_ON_FAILURE);
        StringCchCopyNExW(m_wchFileTitle, MAX_PATH, pwsFileTitle, MAX_PATH, NULL, NULL, STRSAFE_NULL_ON_FAILURE);

        bstrTitle = m_wchFileTitle;
    }
    else
    {
        m_wchFile[0] = m_wchFileTitle[0] = 0;
        bstrTitle = L"New File";
    }

    // The application's title is combined from the application name and
    // the name of the currently open file
    bstrTitle += L" - ";
    bstrTitle += gc_szAppName;

    // Update the window's caption
    ::SetWindowTextW(m_hWnd, bstrTitle);
}
