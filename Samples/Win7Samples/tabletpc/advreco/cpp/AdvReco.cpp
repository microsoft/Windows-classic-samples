// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:
//      AdvReco.cpp
//
// Description:
//      This sample demonstrates such advanced features of the
//      Microsoft Tablet PC Automation API used for handwriting
//      recognition, as
//          - enumerating the installed recognzers
//          - creating a recognition context with a specific language
//            recognizer
//          - setting recognition input scopes
//          - using guides to improve the recognition quality
//          - dynamic background recognition
//          - gesture recognition.
//
//      This application is discussed in the Getting Started guide.
//
//      (NOTE: For code simplicity, returned HRESULT is not checked
//             on failure in the places where failures are not critical
//             for the application or very unexpected)
//
//      The interfaces used are:
//      IInkRecognizers, IInkRecognizer, IInkRecoContext,
//      IInkRecognitionResult, IInkRecognitionGuide, IInkGesture
//      IInkCollector, IInkDisp, IInkRenderer, IInkStrokes, IInkStroke
//
// Requirements:
//      One or more handwriting recognizer must be installed on the system;
//      Appropriate Asian fonts need to be installed to output the results
//      of the Asian recognizers.
//
//--------------------------------------------------------------------------

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

// Windows header files
#include <windows.h>
#include <commctrl.h>       // need it to call CreateStatusWindow

// The following definitions may be not found in the old headers installed with VC6,
// so they're copied from the newer headers found in the Microsoft Platform SDK

#ifndef ListView_SetCheckState
#define ListView_SetCheckState(hwndLV, i, fCheck) \
ListView_SetItemState(hwndLV, i, INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1)), LVIS_STATEIMAGEMASK)
#endif

#ifndef ListView_GetCheckState
#define ListView_GetCheckState(hwndLV, i) \
((((UINT)(SNDMSG((hwndLV), LVM_GETITEMSTATE, (WPARAM)(i), LVIS_STATEIMAGEMASK))) >> 12) -1)
#endif

#ifndef MIIM_STRING
#define MIIM_STRING      0x00000040
#endif
#ifndef MIIM_FTYPE
#define MIIM_FTYPE       0x00000100
#endif

// A useful macro to determine the number of elements in the array
#define countof(array)  (sizeof(array)/sizeof(array[0]))

// ATL header files
#include <atlbase.h>        // defines CComModule, CComPtr, CComVariant
CComModule _Module;
#include <atlwin.h>         // defines CWindowImpl
#include <atlcom.h>         // defines IDispEventSimpleImpl

// Headers for Tablet PC Automation interfaces
#include <msinkaut.h>
#include <msinkaut_i.c>
#include <tpcerror.h>

// The application header files
#include "resource.h"       // main symbols, including command ID's
#include "EventSinks.h"     // defines the IInkEventsImpl and IInkRecognitionEventsImpl
#include "ChildWnds.h"      // definitions of the CInkInputWnd and CRecoOutputWnd
#include "AdvReco.h"        // contains the definition of CAddRecoApp

// The names of the supported Input Scopes.
// All these names are used both to create a menu item and
// as the parameter in IInkRecoContext::put_Factoid(name).
const LPOLESTR gc_pwsInputScopes[] = {
    L"(!IS_DEFAULT)",
    L"(!IS_URL)",
    L"(!IS_FILE_FULLFILEPATH)",
    L"(!IS_FILE_FILENAME)",
    L"(!IS_EMAIL_USERNAME)",
    L"(!IS_EMAIL_SMTPEMAILADDRESS)",
    L"(!IS_LOGINNAME)",
    L"(!IS_PERSONALNAME_FULLNAME)",
    L"(!IS_PERSONALNAME_PREFIX)",
    L"(!IS_PERSONALNAME_GIVENNAME)",
    L"(!IS_PERSONALNAME_MIDDLENAME)",
    L"(!IS_PERSONALNAME_SURNAME)",
    L"(!IS_PERSONALNAME_SUFFIX)",
    L"(!IS_ADDRESS_FULLPOSTALADDRESS)",
    L"(!IS_ADDRESS_POSTALCODE)",
    L"(!IS_ADDRESS_STREET)",
    L"(!IS_ADDRESS_STATEORPROVINCE)",
    L"(!IS_ADDRESS_CITY)",
    L"(!IS_ADDRESS_COUNTRYNAME)",
    L"(!IS_ADDRESS_COUNTRYSHORTNAME)",
    L"(!IS_CURRENCY_AMOUNTANDSYMBOL)",
    L"(!IS_CURRENCY_AMOUNT)",
    L"(!IS_DATE_FULLDATE)",
    L"(!IS_DATE_MONTH)",
    L"(!IS_DATE_DAY)",
    L"(!IS_DATE_YEAR)",
    L"(!IS_DATE_MONTHNAME)",
    L"(!IS_DATE_DAYNAME)",
    L"(!IS_DIGITS)",
    L"(!IS_NUMBER)",
    L"(!IS_ONECHAR)",
    L"(!IS_TELEPHONE_FULLTELEPHONENUMBER)",
    L"(!IS_TELEPHONE_COUNTRYCODE)",
    L"(!IS_TELEPHONE_AREACODE)",
    L"(!IS_TELEPHONE_LOCALNUMBER)",
    L"(!IS_TIME_FULLTIME)",
    L"(!IS_TIME_HOUR)",
    L"(!IS_TIME_MINORSEC)",
    L"((0|1|2|3|4|5|6|7|8|9) ?(0|1|2|3|4|5|6|7|8|9) ?(0|1|2|3|4|5|6|7|8|9) ?-? ?)?(0|1|2|3|4|5|6|7|8|9) ?(0|1|2|3|4|5|6|7|8|9) ?(0|1|2|3|4|5|6|7|8|9) ?-? ?(0|1|2|3|4|5|6|7|8|9) ?(0|1|2|3|4|5|6|7|8|9) ?(0|1|2|3|4|5|6|7|8|9) ?(0|1|2|3|4|5|6|7|8|9)",
    L"(!IS_PERSONALNAME_FULLNAME)|((!IS_PERSONALNAME_PREFIX)? +(!IS_PERSONALNAME_GIVENNAME)+ +(!IS_PERSONALNAME_MIDDLENAME)* +(!IS_PERSONALNAME_SURNAME)+)",
    L"MN(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)(A|B)(!IS_DIGITS)(X|Y)((0|1)*)"
};

// Specifies the maximum allowed length of menu items in the
// input scope menu.  Any items that exceed this value will
// be truncated.
const LONG gc_lMaxInputScopeMenuItemLength = 40;

// The set of the single stroke gestures known to this application
const InkApplicationGesture gc_igtSingleStrokeGestures[] = {
    IAG_Scratchout, IAG_Triangle, IAG_Square, IAG_Star, IAG_Check,
    IAG_Circle, IAG_DoubleCircle, IAG_Curlicue, IAG_DoubleCurlicue,
    IAG_SemiCircleLeft, IAG_SemiCircleRight,
    IAG_ChevronUp, IAG_ChevronDown, IAG_ChevronLeft,
    IAG_ChevronRight, IAG_Up, IAG_Down, IAG_Left, IAG_Right, IAG_UpDown, IAG_DownUp,
    IAG_LeftRight, IAG_RightLeft, IAG_UpLeftLong, IAG_UpRightLong, IAG_DownLeftLong,
    IAG_DownRightLong, IAG_UpLeft, IAG_UpRight, IAG_DownLeft, IAG_DownRight, IAG_LeftUp,
    IAG_LeftDown, IAG_RightUp, IAG_RightDown, IAG_Tap
};

// The following array of indices to the gc_igtSingleStrokeGestures makes the subset
// of gestures recommended for use in the mixed collection mode (ICM_InkAndGesture)
// (the others still can be used in the mixed mode but it's not recommended because
// of their similarity with some characters).
const UINT gc_nRecommendedForMixedMode[] = {
        0 /*Scratchout*/, 3/*Star*/, 6/*Double Circle*/,
        7 /*Curlicue*/, 8 /*Double Curlicue*/, 25 /*Down-Left Long*/ };

// The set of the multiple stroke gestures known to this application
const InkApplicationGesture gc_igtMultiStrokeGestures[] = {
    IAG_ArrowUp, IAG_ArrowDown, IAG_ArrowLeft,
    IAG_ArrowRight, IAG_Exclamation, IAG_DoubleTap
};

// The static members of the event sink templates are initialized here
// (defined in EventSinks.h)

const _ATL_FUNC_INFO IInkRecognitionEventsImpl<CAdvRecoApp>::mc_AtlFuncInfo =
        {CC_STDCALL, VT_EMPTY, 3, {VT_UNKNOWN, VT_VARIANT, VT_I4}};

const _ATL_FUNC_INFO IInkCollectorEventsImpl<CAdvRecoApp>::mc_AtlFuncInfo[2] = {
        {CC_STDCALL, VT_EMPTY, 3, {VT_UNKNOWN, VT_UNKNOWN, VT_BOOL|VT_BYREF}},
        {CC_STDCALL, VT_EMPTY, 4, {VT_UNKNOWN, VT_UNKNOWN, VT_VARIANT, VT_BOOL|VT_BYREF}}
};

const TCHAR gc_szAppName[] = TEXT("Advanced Recognition");

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
        icc.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
        if (TRUE == ::InitCommonControlsEx(&icc))
        {
            // Call the boilerplate function of the application
            iRet = CAdvRecoApp::Run(nCmdShow);
        }
        else
        {
            ::MessageBox(NULL, TEXT("Error initializing the common controls."),
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
// CAdvRecoApp::Run
//
// The static CAdvRecoApp::Run is the boilerplate of the application.
// It instantiates and initializes an CAdvRecoApp object and runs the
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
int CAdvRecoApp::Run(
        int nCmdShow
        )
{

    CAdvRecoApp theApp;

    // Load and update the menu before creating the main window.
    // Create menu items for the installed recognizers and for the
    // supported input scopes.
    HMENU hMenu = theApp.LoadMenu();
    if (NULL == hMenu)
        return 0;

    int iRet;

    // Load the icon from the resource and associate it with the window class
    WNDCLASSEX& wc = CAdvRecoApp::GetWndClassInfo().m_wc;
    wc.hIcon = wc.hIconSm = ::LoadIcon(_Module.GetResourceInstance(),
                                       MAKEINTRESOURCE(IDR_APPICON));

    // Create the application's main window
    if (theApp.Create(NULL, CWindow::rcDefault, gc_szAppName,
                      WS_OVERLAPPEDWINDOW, 0, (UINT)hMenu) != NULL)
    {
        // Initialize the application object:
        // Use no guides
        theApp.SendMessage(WM_COMMAND, ID_GUIDE_NONE);
        // Set input scope to default
        theApp.SendMessage(WM_COMMAND, ID_INPUTSCOPE_FIRST);
        // Create a recognition context with the default recognizer
        theApp.SendMessage(WM_COMMAND, ID_RECOGNIZER_DEFAULT);
        // Set the collection mode to ICM_InkOnly
        theApp.SendMessage(WM_COMMAND, ID_MODE_INK);

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
        ::MessageBox(NULL, TEXT("Error creating the window"),
                     gc_szAppName, MB_ICONERROR | MB_OK);
        ::DestroyMenu(hMenu);
        iRet = 0;
    }

    return iRet;
}

// Window message handlers //////////////////////////////

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnCreate
//
// This WM_CREATE message handler creates and obtains interface
// pointers to the required Automation objects, sets their
// attributes, creates the child windows and enables pen input.
//
// Parameters:
//      defined in the ATL's macro MESSAGE_HANDLER,
//      none of them is used here
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CAdvRecoApp::OnCreate(
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

    // Get a pointer to the ink object interface.
    hr = m_spIInkCollector->get_Ink(&m_spIInkDisp);
    if (FAILED(hr))
        return -1;

    // Get an empty ink strokes collection from the ink object.
    // It'll be used as a storage for the ink
    hr = m_spIInkDisp->get_Strokes(&m_spIInkStrokes);
    if (FAILED(hr))
        return -1;

    // Establish a connection to the collector's event source.
    // Depending on the selected collection mode, the application will be
    // listening to either Stroke or Gesture events, or both.
    hr = IInkCollectorEventsImpl<CAdvRecoApp>::DispEventAdvise(m_spIInkCollector);
    // There is nothing interesting the application can do without events
    // from the ink collector
    if (FAILED(hr))
        return -1;

    // Set the recommended subset of gestures
    PresetGestures();

    // Enable ink input in the m_wndInput window
    hr = m_spIInkCollector->put_hWnd((long)m_wndInput.m_hWnd);
    if (FAILED(hr))
        return -1;
    hr = m_spIInkCollector->put_Enabled(VARIANT_TRUE);
    if (FAILED(hr))
        return -1;

    // Create a recognizer guide object
    if (SUCCEEDED(m_spIInkRecoGuide.CoCreateInstance(CLSID_InkRecognizerGuide)))
    {
        _InkRecoGuide irg;
        irg.midline = -1;                   // not use midline
        irg.cRows = irg.cColumns = 0;       // no guide lines
        ::SetRect(&irg.rectWritingBox, 0, 0, mc_iGuideColWidth, mc_iGuideRowHeight);
        // Make the guide's DrawnBox a little smaller than the the actual area
        // in which a user is expected to draw the ink (WritingBox)
        irg.rectDrawnBox = irg.rectWritingBox;
        ::InflateRect(&irg.rectDrawnBox, -mc_cxBoxMargin, -mc_cyBoxMargin);

        // Set the m_wndInput's guide data
        m_wndInput.SetGuide(irg);

        // Use IInkRenderer to transform the guide boxes rectangles
        // into ink space coordinates
        hr = m_spIInkCollector->get_Renderer(&m_spIInkRenderer);
        if (SUCCEEDED(hr))
        {
            HDC hdc = m_wndInput.GetDC();
            if (NULL != hdc)
            {
                // Convert the guide's writing box
                m_spIInkRenderer->PixelToInkSpace((long)hdc,
                                                  &irg.rectWritingBox.left,
                                                  &irg.rectWritingBox.top);
                m_spIInkRenderer->PixelToInkSpace((long)hdc,
                                                  &irg.rectWritingBox.right,
                                                  &irg.rectWritingBox.bottom);
                // and the drawn box
                m_spIInkRenderer->PixelToInkSpace((long)hdc,
                                                  &irg.rectDrawnBox.left,
                                                  &irg.rectDrawnBox.top);
                m_spIInkRenderer->PixelToInkSpace((long)hdc,
                                                  &irg.rectDrawnBox.right,
                                                  &irg.rectDrawnBox.bottom);
                // Initialize the guide
                hr = m_spIInkRecoGuide->put_GuideData(irg);
                ReleaseDC(hdc);
            }
            else
            {
                hr = E_FAIL;
            }
        }

        // Don't use m_spIInkRecoGuide if failed to set the guide data
        if (FAILED(hr))
        {
            m_spIInkRecoGuide.Release();
        }
    }

    return 0;
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnDestroy
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
LRESULT CAdvRecoApp::OnDestroy(
        UINT /*uMsg*/,
        WPARAM /*wParam*/,
        LPARAM /*lParam*/,
        BOOL& /*bHandled*/
        )
{
    // Disable ink input and release the InkCollector object
    if (m_spIInkCollector != NULL)
    {
        IInkCollectorEventsImpl<CAdvRecoApp>::DispEventUnadvise(m_spIInkCollector);
        m_spIInkCollector->put_Enabled(VARIANT_FALSE);
        m_spIInkCollector.Release();
    }

    // Detach the strokes collection from the recognition context
    // and stop listening to the recognition events
    if (m_spIInkRecoContext != NULL)
    {
        m_spIInkRecoContext->EndInkInput();
        IInkRecognitionEventsImpl<CAdvRecoApp>::DispEventUnadvise(m_spIInkRecoContext);
        m_spIInkRecoContext.Release();
    }

    // Release the other objects and collections
    m_spIInkRecoGuide.Release();
    m_spIInkStrokes.Release();
    m_spIInkRecognizers.Release();

    // Post a WM_QUIT message to the application's message queue
    ::PostQuitMessage(0);

    return 0;
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnSize
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
LRESULT CAdvRecoApp::OnSize(
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
// CAdvRecoApp::OnStroke
//
// The _IInkCollectorEvents's Stroke event handler.
// See the Tablet PC Automation API Reference for the
// detailed description of the event and its parameters.
//
// Parameters:
//      IInkCursor* pIInkCursor     : [in] not used here
//      IInkStrokeDisp* pInkStroke  : [in]
//      VARIANT_BOOL* pbCancel      : [in,out] option to cancel the gesture,
//                                    default value is FALSE, not modified here
//
// Return Values (HRESULT):
//      S_OK if succeeded, E_FAIL or E_INVALIDARG otherwise
//
/////////////////////////////////////////////////////////
HRESULT CAdvRecoApp::OnStroke(
        IInkCursor* /*pIInkCursor*/,
        IInkStrokeDisp* pIInkStroke,
        VARIANT_BOOL* /* pbCancel */
        )
{
    if (NULL == pIInkStroke)
        return E_INVALIDARG;

    if (m_spIInkStrokes == NULL)
        return S_OK;

    // Add the new stroke to the collection
    HRESULT hr = m_spIInkStrokes->Add(pIInkStroke);
    if (SUCCEEDED(hr) && m_spIInkRecoContext != NULL)
    {
        // Cancel the previous background recognition requests
        // which have not been processed yet
        m_spIInkRecoContext->StopBackgroundRecognition();

        // Update the recognition results
        CComVariant vCustomData;    // no custom data
        m_spIInkRecoContext->BackgroundRecognizeWithAlternates(vCustomData);
    }
    return hr;
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnGesture
//
// The _IInkCollectorEvents's Gesture event handler.
// See the Tablet PC Automation API Reference for the
// detailed description of the event and its parameters.
//
// Parameters:
//      IInkCursor* pIInkCursor  : [in] not used here
//      IInkStrokes* pInkStrokes : [in] the collection
//      VARIANT vGestures        : [in] safearray of IDispatch interface pointers
//                                 of the recognized  Gesture objects
//      VARIANT_BOOL* pbCancel   : [in,out] option to cancel the gesture,
//                                 default value is FALSE
//
// Return Values (HRESULT):
//      S_OK if succeeded, E_FAIL or E_INVALIDARG otherwise
//
/////////////////////////////////////////////////////////
HRESULT CAdvRecoApp::OnGesture(
        IInkCursor* /*pIInkCursor*/,
        IInkStrokes* pInkStrokes,
        VARIANT vGestures,
        VARIANT_BOOL* pbCancel
        )
{
    if (((VT_ARRAY | VT_DISPATCH) != vGestures.vt) || (NULL == vGestures.parray))
        return E_INVALIDARG;
    if (0 == vGestures.parray->rgsabound->cElements)
        return E_INVALIDARG;

    // The gestures in the array are supposed to be ordered by their recognition
    // confidence level. This sample picks up the top one.
    // NOTE: when in the InkAndGesture collection mode, besides the gestures expected
    // by the application there also can come a gesture object with the id IAG_NoGesture
    // This application cancels the event if the object with ISG_NoGesture has
    // the top confidence level (the first item in the array).
    InkApplicationGesture idGesture = IAG_NoGesture;
    IDispatch** ppIDispatch;
    HRESULT hr = ::SafeArrayAccessData(vGestures.parray, (void HUGEP**)&ppIDispatch);
    if (SUCCEEDED(hr))
    {
        CComQIPtr<IInkGesture> spIInkGesture(ppIDispatch[0]);
        if (spIInkGesture != NULL)
        {
            hr = spIInkGesture->get_Id(&idGesture);
        }
        ::SafeArrayUnaccessData(vGestures.parray);
    }

    // Load the name of the gesture from the resource string table
    UINT idGestureName;
    bool bAccepted;     // will be true, if the gesture is known to this application
    if (IAG_NoGesture != idGesture)
    {
        bAccepted = GetGestureName(idGesture, idGestureName);
    }
    else    // ignore the event (IAG_NoGesture had the highest confidence level,
            // or something has failed
    {
        bAccepted = false;
        idGestureName = 0;
    }

    // If the current collection mode is ICM_GestureOnly or if we accept
    // the gesture, the gesture's strokes will be removed from the ink object,
    // So, the window needs to be updated in the strokes' area.
    if (ID_MODE_GESTURES == m_nCmdMode || true == bAccepted)
    {
        // Get the rectangle to update.
        RECT rc;
        CComPtr<IInkRectangle> spIInkRect;
        if (m_spIInkRenderer != NULL
            && pInkStrokes != NULL
            && SUCCEEDED(pInkStrokes->GetBoundingBox(IBBM_Default, &spIInkRect))
            && SUCCEEDED(spIInkRect->GetRectangle(&rc.top, &rc.left,
                                                  &rc.bottom, &rc.right)))
        {
            // Transform the bounding box coordinates from ink space to screen
            HDC hdc = m_wndInput.GetDC();
            if (NULL != hdc)
            {
                if (FAILED(m_spIInkRenderer->InkSpaceToPixel((long)hdc, &rc.left, &rc.top))
                    || FAILED(m_spIInkRenderer->InkSpaceToPixel((long)hdc, &rc.right,
                                                                &rc.bottom)))
                {
                    // Failed mapping from ink space to screen,
                    // update entire client area of the input window
                    m_wndInput.GetClientRect(&rc);
                }
                ReleaseDC(hdc);
            }
            else
            {
                // Failed getting an hdc, update entire client area of the input window
                m_wndInput.GetClientRect(&rc);
            }
        }
        else
        {
            // Failed getting the bounding box, update entire client area of the input window
            m_wndInput.GetClientRect(&rc);
        }

        m_wndInput.InvalidateRect(&rc);
    }
    else // if something's failed,
         // or the gesture is either unknown or unchecked in the list
    {
        // Reject the gesture. The InkCollector will fire Stroke event(s)
        // for the strokes, so they'll be handled in the OnStroke method.
        *pbCancel = VARIANT_TRUE;
        idGestureName = IDS_GESTURE_UNKNOWN;
    }

    // Update the results window as well
    m_wndResults.SetGestureName(idGestureName);
    m_wndResults.Invalidate();

    return hr;
}

// Recognition event handlers ////////////////////////////

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnRecognitionWithAlternates
//
// The _IInkRecognitionEvents's RecognitionWithAlternates event handler
//
// Parameters:
//      IInkRecognitionResult* pIInkRecoResult  :
//      VARIANT vCustomParam                    : not used here
//      InkRecognitionStatus RecognitionStatus  : not used here
//
// Return Values (HRESULT):
//      S_OK if succeeded, E_FAIL or E_INVALIDARG otherwise
//
/////////////////////////////////////////////////////////
HRESULT CAdvRecoApp::OnRecognitionWithAlternates(
        IInkRecognitionResult* pIInkRecoResult,
        VARIANT /*vCustomParam*/,
        InkRecognitionStatus /*RecognitionStatus*/
        )
{
    if (NULL == pIInkRecoResult)
        return E_INVALIDARG;

    // Reset the old results
    m_wndResults.ResetResults();

    // Get the best lCount results
    HRESULT hr;
    CComPtr<IInkRecognitionAlternates> spIInkRecoAlternates;
    hr = pIInkRecoResult->AlternatesFromSelection(
        0,                              // in: selection start
        -1,                             // in: selection length; -1 means "up to the last one"
        CRecoOutputWnd::mc_iNumResults, // in: the number of alternates we're interested in
        &spIInkRecoAlternates           // out: the receiving pointer
        );

    // Count the returned alternates, it may be less then we asked for
    LONG lCount = 0;
    if (SUCCEEDED(hr) && SUCCEEDED(spIInkRecoAlternates->get_Count(&lCount)))
    {
        // Get the alternate strings
        IInkRecognitionAlternate* pIInkRecoAlternate = NULL;
        for (LONG iItem = 0; (iItem < lCount) && (iItem < CRecoOutputWnd::mc_iNumResults); iItem++)
        {
            // Get the alternate string if there is one
            if (SUCCEEDED(spIInkRecoAlternates->Item(iItem, &pIInkRecoAlternate)))
            {
                BSTR bstr = NULL;
                if (SUCCEEDED(pIInkRecoAlternate->get_String(&bstr)))
                {
                    m_wndResults.m_bstrResults[iItem].Attach(bstr);
                }
                pIInkRecoAlternate->Release();
            }
        }
    }

    // Update the output window with the new results
    m_wndResults.Invalidate();

    return S_OK;
}

// Command handlers /////////////////////////////////////

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnRecognizer
//
// This command handler is called when user selects a recognizer
// from the "Recognizer" submenu.
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
LRESULT CAdvRecoApp::OnRecognizer(
        WORD /*wNotifyCode*/,
        WORD wID,
        HWND /*hWndCtl*/,
        BOOL& /*bHandled*/
        )
{
    if (m_spIInkRecognizers == NULL || wID == m_nCmdRecognizer)
        return 0;

    // Get a pointer to the recognizer object from the recognizer collection
    // Use DefaultRecognizer method to get a pointer to the default recognizer
    // or use index for any other one
    HRESULT hr;
    CComPtr<IInkRecognizer> spIInkRecognizer;
    if (ID_RECOGNIZER_DEFAULT == wID)
    {
        // The first parameter is the language id, passing 0 means that the language
        // id will be retrieved using the user default-locale identifier
        hr = m_spIInkRecognizers->GetDefaultRecognizer(0, &spIInkRecognizer);
    }
    else
    {
        hr = m_spIInkRecognizers->Item(wID - ID_RECOGNIZER_FIRST, &spIInkRecognizer);
    }

    // Create new recognition context
    if (SUCCEEDED(hr) && UseRecognizer(spIInkRecognizer))
    {
        // Update the menu and the status bar
        UpdateMenuRadioItems(mc_iSubmenuRecognizers, wID, m_nCmdRecognizer);
        m_bstrCurRecoName.Empty();
        spIInkRecognizer->get_Name(&m_bstrCurRecoName);
        UpdateStatusBar();
        // Store the selected recognizer's command id
        m_nCmdRecognizer = wID;
    }

    return 0;
}
/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnInputScopeCoerce
//
// This command handler is called when user checks on or off
// the "Coerce to InputScope" menu item in the "InputScope" submenu.
//
// Parameters:
//      defined in the ATL's macro COMMAND_ID_HANDLER
//      Only wID - the id of the command associated
//      with the clicked menu item - is used here.
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CAdvRecoApp::OnInputScopeCoerce(
        WORD /*wNotifyCode*/,
        WORD wID,
        HWND /*hWndCtl*/,
        BOOL& /*bHandled*/
        )
{
    // Ignore the command if the recognition context
    // has not been created
    if (m_spIInkRecoContext == NULL )
        return 0;

    // Need to reset the recognition context to modify the property
    if (m_spIInkStrokes != NULL)
    {
        m_spIInkRecoContext->putref_Strokes(NULL);
    }

    // Reset the RecognitionFlags property of the recognition context
    if (FAILED(m_spIInkRecoContext->put_RecognitionFlags (m_bCoerceInputScope?IRM_None:IRM_Coerce)))
    {
        MessageBox(TEXT("Failed to reset the RecognitionFlags property!"),
                       gc_szAppName, MB_ICONERROR | MB_OK);
        // Re-attach the stroke collection to the context
        if (m_spIInkStrokes != NULL)
        {
            m_spIInkRecoContext->putref_Strokes(m_spIInkStrokes);
        }
            return 0;
        }
    m_bCoerceInputScope = !m_bCoerceInputScope;

    // Re-attach the stroke collection to the context
    if (m_spIInkStrokes != NULL)
    {
        m_spIInkRecoContext->putref_Strokes(m_spIInkStrokes);
    }

    // Update the recognition results
    CComVariant vCustomData;    // no custom data
    m_spIInkRecoContext->BackgroundRecognizeWithAlternates(vCustomData);

    // Update the menu item
    HMENU hMenu = GetMenu();
    if (NULL != hMenu)
    {
        HMENU hSubMenu = ::GetSubMenu(hMenu, mc_iSubmenuInputScopes);
        if (NULL != hSubMenu)
        {
            ::CheckMenuItem(hSubMenu, wID,
                            MF_BYCOMMAND | (m_bCoerceInputScope ? MF_CHECKED : MF_UNCHECKED));
        }
    }

    return 0;
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnInputScope
//
// This command handler is called when user selects
// a input scope name in the "Input Scope" submenu.
//
// Parameters:
//      defined in the ATL's macro COMMAND_RANGE_HANDLER
//      Only wID - the id of the command associated
//      with the clicked menu item - is used here.
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CAdvRecoApp::OnInputScope(
        WORD /*wNotifyCode*/,
        WORD wID,
        HWND /*hWndCtl*/,
        BOOL& /*bHandled*/
        )
{
    UINT iInputScope = wID - ID_INPUTSCOPE_FIRST;     // get the index to gc_pwsInputScopes
    // Return if user clicks on the currently selected Input Scope menu item or if
    // the command id is not in the valid range (shouldn't happen with error free code)
    if (wID == m_nCmdInputScope || iInputScope >= countof(gc_pwsInputScopes))
        return 0;

    // Tell the context to use the Input Scope, by assigning the input scope name
    // to the corresponding property of the context.
    if (m_spIInkRecoContext != NULL)
    {
        // Need to reset the recognition context in order to change the property
        if (m_spIInkStrokes != NULL)
        {
            m_spIInkRecoContext->putref_Strokes(NULL);
        }

        CComBSTR bstrInputScope(gc_pwsInputScopes[iInputScope]);
        HRESULT hr = m_spIInkRecoContext->put_Factoid(bstrInputScope);

        // Re-attach the stroke collection to the context
        if (m_spIInkStrokes != NULL)
        {
            m_spIInkRecoContext->putref_Strokes(m_spIInkStrokes);
        }

        if (SUCCEEDED(hr))
        {
            // Update the recognition results
            CComVariant vCustomData;    // no custom data
            m_spIInkRecoContext->BackgroundRecognizeWithAlternates(vCustomData);
        }
        else if (TPC_E_INVALID_PROPERTY == hr) // the input scope is not supported
        {
            MessageBox(TEXT("This factoid is not supported by the recognizer."),
                       gc_szAppName, MB_ICONINFORMATION | MB_OK);
            return 0;
        }
        else
        {
            MessageBox(TEXT("Failed to set the context's Factoid property!"),
                       gc_szAppName, MB_ICONERROR | MB_OK);
            return 0;
        }
    }

    // Update the menu and the status bar
    UpdateMenuRadioItems(mc_iSubmenuInputScopes, wID, m_nCmdInputScope);
    m_nCmdInputScope = wID;
    UpdateStatusBar();

    return 0;
}


/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnGuide
//
// This command handler is called when user selects a type
// of the recognition guide in the "Guide" submenu.
//
// Parameters:
//      defined in the ATL's macro COMMAND_RANGE_HANDLER
//      Only wID - the id of the command associated
//      with the clicked menu item - is used here.
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CAdvRecoApp::OnGuide(
        WORD /*wNotifyCode*/,
        WORD wID,
        HWND /*hWndCtl*/,
        BOOL& /*bHandled*/
        )
{
    // Do nothing, if the InkRecoGuide object was not created,
    // or user selected the currently used type of guide.
    if (m_spIInkRecoGuide == NULL || wID == m_nCmdGuide)
        return 0;

    // The sizes of the drawing and writing boxes of the guide were set
    // in CAdvRecoApp::OnCreate, when the guide object was created.
    // There is no need to modify them in order to switch the guide from
    // one type to another. This can be done by setting the guides properties
    // Rows and Columns to 0 or not 0.
    int cRows = 0, cColumns = 0;
    if (ID_GUIDE_LINES == wID || ID_GUIDE_BOXES == wID)
    {
        cRows = mc_iNumRowsCols;
        if (ID_GUIDE_BOXES == wID)
        {
            cColumns = mc_iNumRowsCols;
        }
    }

    // Put the new values
    if (SUCCEEDED(m_spIInkRecoGuide->put_Rows(cRows))
        && SUCCEEDED(m_spIInkRecoGuide->put_Columns(cColumns)))
    {
        HRESULT hr = S_OK;
        if (m_spIInkRecoContext != NULL)
        {
            // Need to reset the recognition context in order to change the property
            if (m_spIInkStrokes != NULL)
            {
                m_spIInkRecoContext->putref_Strokes(NULL);
            }

            // Set the updated guide
            hr = m_spIInkRecoContext->putref_Guide(m_spIInkRecoGuide);

            // Re-attach the stroke collection to the context
            if (m_spIInkStrokes != NULL)
            {
                m_spIInkRecoContext->putref_Strokes(m_spIInkStrokes);
            }

            // Update the recognition results
            CComVariant vCustomData;    // no custom data
            m_spIInkRecoContext->BackgroundRecognizeWithAlternates(vCustomData);
        }

        if (SUCCEEDED(hr))
        {
            // Update the input window (this call will force it to redraw the guide).
            m_wndInput.SetRowsCols(cRows, cColumns);

            // Update the menu
            UpdateMenuRadioItems(mc_iSubmenuGuides, wID, m_nCmdGuide);

            // store the selected guide's associated command id
            m_nCmdGuide = wID;
        }
        else
        {
            MessageBox(TEXT("Error setting the guide to the recognition context.\n"),
                       gc_szAppName, MB_ICONERROR | MB_OK);
        }
    }
    else
    {
        MessageBox(TEXT("Error setting the guide's Rows and/or Columns property.\n"),
                   gc_szAppName, MB_ICONERROR | MB_OK);
    }

    return 0;
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnMode
//
// This command handler is called when user selects
// a different collection mode from the "Mode" submenu.
// NOTE: Changing collection mode has no effect on
//       the recognition results of the existing strokes.
//
// Parameters:
//      defined in the ATL's macro COMMAND_RANGE_HANDLER
//      Only wID - the id of the command associated
//      with the clicked menu item - is used here.
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CAdvRecoApp::OnMode(
        WORD /*wNotifyCode*/,
        WORD wID,
        HWND /*hWndCtl*/,
        BOOL& /*bHandled*/
        )
{
    // Do nothing, id user selected the same mode.
    if (wID == m_nCmdMode)
        return 0;

    InkCollectionMode icm;
    switch (wID)
    {
        default:
            return 0;
        case ID_MODE_INK:
            icm = ICM_InkOnly;
            break;
        case ID_MODE_INK_AND_GESTURES:
            icm = ICM_InkAndGesture;
            break;
        case ID_MODE_GESTURES:
            icm = ICM_GestureOnly;
            break;
    }

    // Disable input to switch the collection mode
    if (m_spIInkCollector != NULL
        && SUCCEEDED(m_spIInkCollector->put_Enabled(VARIANT_FALSE)))
    {
        // Set the new mode
        if (SUCCEEDED(m_spIInkCollector->put_CollectionMode(icm)))
        {

            // Update the menu
            UpdateMenuRadioItems(mc_iSubmenuModes, wID, m_nCmdMode);
            m_nCmdMode = wID;  // store the selected mode's associated command id

            // Show or hide the gesture list views
            UpdateLayout();
        }
        else
        {
            TCHAR* pszErrorMsg;
            if (ID_MODE_INK == m_nCmdMode)
            {
                pszErrorMsg = TEXT("Unable to change the CollectionMode property on the ")
                              TEXT("InkCollector.\nA possible reason is that the selected ")
                              TEXT("mode requires there to be a gesture recognizer installed.");
            }
            else
            {
                pszErrorMsg = TEXT("Unable to change the CollectionMode property ")
                              TEXT("on the InkCollector.");
            }
            MessageBox(pszErrorMsg, gc_szAppName, MB_ICONERROR | MB_OK);
        }
        // Enable input
        if (FAILED(m_spIInkCollector->put_Enabled(VARIANT_TRUE)))
        {
            MessageBox(TEXT("Error enabling InkCollector after changing collection mode!"),
                       gc_szAppName, MB_ICONERROR | MB_OK);
        }
    }

    return 0;
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnRecognize
//
// This command handler is called when user clicks on "Recognize"
// in the Ink menu.
// It's required for East Asian recognizers to finalize the recognition
//
// Parameters:
//      defined in the ATL's macro COMMAND_ID_HANDLER
//      none of them is used here
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CAdvRecoApp::OnRecognize(
        WORD /*wNotifyCode*/,
        WORD /*wID*/,
        HWND /*hWndCtl*/,
        BOOL& /*bHandled*/
        )
{
    if (m_spIInkRecoContext != NULL)
    {
        m_spIInkRecoContext->EndInkInput();

        // Recognize
        CComPtr<IInkRecognitionResult> spIInkRecoResult;
        InkRecognitionStatus ink_reco_status = IRS_NoError;
        if (SUCCEEDED(m_spIInkRecoContext->Recognize(&ink_reco_status, &spIInkRecoResult)))
        {
            CComVariant vCustomData;    // no custom data
            OnRecognitionWithAlternates(spIInkRecoResult, vCustomData, ink_reco_status);
        }
        else
        {
            MessageBox(TEXT("Error code returned from the context's Recognize method."),
                       gc_szAppName, MB_ICONERROR | MB_OK);
        }

        // Re-attach the stroke collection to the context
        if (m_spIInkStrokes != NULL)
        {
            if (FAILED(m_spIInkRecoContext->putref_Strokes(m_spIInkStrokes)))
            {
                MessageBox(TEXT("Failed to attach the stroke collection to the recognition context!"),
                           gc_szAppName, MB_ICONERROR | MB_OK);
            }
        }
    }

    return 0;
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnClear
//
// This command handler is called when user clicks on "Clear"
// in the Ink menu. It's supposed to delete the collected ink,
// and update the child windows after that.
//
// Parameters:
//      defined in the ATL's macro COMMAND_ID_HANDLER
//      none of them is used here
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CAdvRecoApp::OnClear(
        WORD /*wNotifyCode*/,
        WORD /*wID*/,
        HWND /*hWndCtl*/,
        BOOL& /*bHandled*/
        )
{
    if (m_spIInkDisp != NULL)
    {
        // Delete all strokes from the Ink object, ignore returned value
        m_spIInkDisp->DeleteStrokes(0);

        // Release the old stroke collection and get an empty one
        m_spIInkStrokes.Release();
        CComVariant vt(0);
        if (FAILED(m_spIInkDisp->CreateStrokes(vt, &m_spIInkStrokes)))
        {
            MessageBox(TEXT("Failed to get the stroke collection from the Ink object!"),
                       gc_szAppName, MB_ICONERROR | MB_OK);
        }

        // Attach the new stroke collection to the recognition context.
        // If get_Strokes has failed, m_spIInkStrokes is NULL, so no
        // stroke collection will be attached to the context
        if (m_spIInkRecoContext != NULL)
        {
            if (FAILED(m_spIInkRecoContext->putref_Strokes(m_spIInkStrokes)))
            {
                MessageBox(TEXT("Failed to attach the stroke collection to the recognition context!"),
                           gc_szAppName, MB_ICONERROR | MB_OK);
            }
        }
    }

    // Update the child windows
    m_wndResults.ResetResults();    // empties the strings
    m_wndResults.Invalidate();
    m_wndInput.Invalidate();

    return 0;
}


/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnExit
//
// This command handler is called when user clicks
// on "Exit" in the Ink menu.
//
// Parameters:
//      defined in the ATL's macro COMMAND_ID_HANDLER
//      none of them is used here
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CAdvRecoApp::OnExit(
        WORD /*wNotifyCode*/,
        WORD /*wID*/,
        HWND /*hWndCtl*/,
        BOOL& /*bHandled*/
        )
{
    // Close the application window
    SendMessage(WM_CLOSE);
    return 0;
}

// Helper methods //////////////////////////////

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::LoadMenu
//
// This method instantiates an enumerator object for the installed
// recognizers, loads the main menu resource and creates a menu item
// for each recognizer from the collection.
// Also, it fills the Input Scope menu with the items for the supported
// Input Scopes.
//
// Parameters:
//      none
//
// Return Values (HMENU):
//      The return value is a handle of the menu
//      that'll be used for the main window
//
/////////////////////////////////////////////////////////
HMENU CAdvRecoApp::LoadMenu()
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
        ::MessageBox(NULL, TEXT("There are no handwriting recognizers installed.\n")
                     TEXT("You need to have at least one in order to run this sample.\nExiting."),
                     gc_szAppName, MB_ICONERROR | MB_OK);
        return NULL;
    }

    // Load the menu of the main window
    HMENU hMenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MENU));
    if (NULL == hMenu)
        return NULL; // Not normal

    MENUITEMINFOW miinfo;
    memset(&miinfo, 0, sizeof(miinfo));
    miinfo.cbSize = sizeof(miinfo);
    miinfo.fMask = MIIM_ID | MIIM_STATE | MIIM_FTYPE | MIIM_STRING;
    miinfo.fType = MFT_RADIOCHECK | MFT_STRING;

    // Get the submenu id for the recognizers menu.
    // This submenu will contain the list of installed recognizers
    HMENU hSubMenu = ::GetSubMenu(hMenu, mc_iSubmenuRecognizers);
    if (hSubMenu)
    {
        CComPtr<IInkRecognizer> spIInkRecognizer;

        // The ID_RECOGNIZER_FIRST is defined in the resource.h file.
        // It's the base id for the commands that fill this submenu
        miinfo.wID = ID_RECOGNIZER_FIRST;
        miinfo.fState = 0;
        for (LONG i = 0; i < lCount; i++, miinfo.wID++)
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
                    CComBSTR bstrName;
                    if (SUCCEEDED(spIInkRecognizer->get_Name(&bstrName)))
                    {
                        miinfo.dwTypeData = bstrName;
                        ::InsertMenuItemW(hSubMenu, (UINT)-1, TRUE, &miinfo);
                    }
                }
                spIInkRecognizer.Release();
            }
        }
    }

    // Get the Input Scope submenu and create command items
    // for each input scope name defined in gc_pwsInputScopes[]
    hSubMenu = ::GetSubMenu(hMenu, mc_iSubmenuInputScopes);
    if (hSubMenu)
    {
        // The ID_INPUTSCOPE_FIRST is defined in the resource.h file.
        // It's the base id for the commands that fill this submenu
        miinfo.wID = ID_INPUTSCOPE_FIRST;
        miinfo.fState = 0;
        lCount = countof(gc_pwsInputScopes);
        for(LONG i = 0; i < lCount; i++, miinfo.wID++)
        {
            // Set the input scope menu item text.  If the text
            // length exceeds the maximum allowed value, then truncate
            // it and append "...".
            if (wcslen(gc_pwsInputScopes[i]) <= gc_lMaxInputScopeMenuItemLength)
            {
                miinfo.dwTypeData = gc_pwsInputScopes[i];
            }
            else
            {
                CComBSTR bstrInputScope(gc_lMaxInputScopeMenuItemLength-3, gc_pwsInputScopes[i]);
                bstrInputScope += L"...";
                miinfo.dwTypeData = bstrInputScope;
            }

            ::InsertMenuItemW(hSubMenu, (UINT)-1, TRUE, &miinfo);
        }
    }

    return hMenu;
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::UpdateInputScopeMenu
//
// This helper method updates the enabled/disabled state
// of submenus in the InputScope menu.
// It's called whenever the user selects a different
// recognizer. Verification of support for the InputScope
// is accomplished by calling put_Factoid, and checking
// the returned HRESULT.
//
// Parameters:
//      none
//
// Return Values (void):
//      none
//
/////////////////////////////////////////////////////////
void CAdvRecoApp::UpdateInputScopeMenu()
{
    HMENU hMenu = GetMenu();
    if (NULL != hMenu)
    {
        HMENU hSubMenu = ::GetSubMenu(hMenu, mc_iSubmenuInputScopes);
        if (NULL != hSubMenu)
        {
            if (m_spIInkRecoContext != NULL)
            {
                // Cache the current Factoid property so that we can revert later
                CComBSTR bstrInputScope;
                if (FAILED(m_spIInkRecoContext->get_Factoid(&bstrInputScope)))
                {
                    MessageBox(TEXT("Failed to get the context's Factoid property."),
                                gc_szAppName, MB_ICONERROR | MB_OK);
                    return;
                }

                // Need to reset the recognition context in order to change the Factoid property
                if (m_spIInkStrokes != NULL)
                {
                    m_spIInkRecoContext->putref_Strokes(NULL);
                }

                // Enable or disable the InputScope submenu items
                MENUITEMINFO miinfo;
                memset(&miinfo, 0, sizeof(miinfo));
                miinfo.cbSize = sizeof(miinfo);
                miinfo.fMask = MIIM_STATE;
                miinfo.wID = ID_INPUTSCOPE_FIRST;
                for(LONG i = 0; i < countof(gc_pwsInputScopes); i++, miinfo.wID++)
                {
                    CComBSTR bstrTestInputScope(gc_pwsInputScopes[i]);
                    HRESULT hr = m_spIInkRecoContext->put_Factoid(bstrTestInputScope);
                    if (FAILED(hr))
                    {
                        miinfo.fState = MFS_DISABLED;
                    }
                    else
                    {
                        miinfo.fState = MFS_ENABLED;
                    }
                    ::SetMenuItemInfo(hSubMenu, miinfo.wID, FALSE, &miinfo);

                }

                // Revert to the cached Factoid property
                if (FAILED(m_spIInkRecoContext->put_Factoid(bstrInputScope)))
                {
                    MessageBox(TEXT("Failed to set the context's Factoid property."),
                                gc_szAppName, MB_ICONERROR | MB_OK);
                }

                // Re-attach the stroke collection to the context
                if (m_spIInkStrokes != NULL)
                {
                    m_spIInkRecoContext->putref_Strokes(m_spIInkStrokes);
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::UpdateMenuRadioItems
//
// As it follows from the name, this helper method updates
// the specified radio items in the submenu.
// It's called for the appropriate items, whenever user selects
// a different recognizer, input scope, or guide mode.
//
// Parameters:
//      UINT iSubMenu   : [in] the submenu to make updates in
//      UINT idCheck    : [in] the menu item to check
//      UINT idUncheck  : [in] the menu item to uncheck
//
// Return Values (void):
//      none
//
/////////////////////////////////////////////////////////
void CAdvRecoApp::UpdateMenuRadioItems(
        UINT iSubMenu,
        UINT idCheck,
        UINT idUncheck
        )
{
    // Update the menu
    HMENU hMenu = GetMenu();
    if (NULL != hMenu)
    {
        HMENU hSubMenu = ::GetSubMenu(hMenu, iSubMenu);
        if (NULL != hSubMenu)
        {
            MENUITEMINFO miinfo;
            miinfo.cbSize = sizeof(miinfo);
            miinfo.fMask = MIIM_STATE | MIIM_FTYPE;
            ::GetMenuItemInfo(hSubMenu, idCheck, FALSE, &miinfo);
            miinfo.fType |= MFT_RADIOCHECK;
            miinfo.fState |= MFS_CHECKED;
            ::SetMenuItemInfo(hSubMenu, idCheck, FALSE, &miinfo);
            if (0 != idUncheck)
            {
                ::GetMenuItemInfo(hSubMenu, idUncheck, FALSE, &miinfo);
                miinfo.fType |= MFT_RADIOCHECK;
                miinfo.fState &= ~MFS_CHECKED;
                ::SetMenuItemInfo(hSubMenu, idUncheck, FALSE, &miinfo);
            }
        }
    }
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::UseRecognizer
//
// As it follows from the name, this helper method updates
// the specified radio items in the submenu.
// It's called for the appropriate items, whenever user selects
// a different recognizer, input scope, or guide mode.
//
// Parameters:
//      IInkRecognizer* pIInkRecognizer   : [in] the newly selected recognizer
//
// Return Values (bool):
//      true if succeeded creating new recognition context, false otherwise
//
/////////////////////////////////////////////////////////
bool CAdvRecoApp::UseRecognizer(
        IInkRecognizer* pIInkRecognizer
        )
{
    if (NULL == pIInkRecognizer)
        return false;

    // Create a new recognition context
    CComPtr<IInkRecognizerContext> spNewContext;
    HRESULT hr = pIInkRecognizer->CreateRecognizerContext(&spNewContext);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Error creating a new recognition context!"),
                   gc_szAppName, MB_ICONERROR | MB_OK);
        return false;
    }

    // Change cursor to the system's Hourglass
    HCURSOR hCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));

    // Detach and release the old context
    if (m_spIInkRecoContext != NULL)
    {
        // Disconnect from the recognition events source
        IInkRecognitionEventsImpl<CAdvRecoApp>::DispEventUnadvise(m_spIInkRecoContext);

        // Reset and release the recognition context
        m_spIInkRecoContext->putref_Strokes(NULL);
        m_spIInkRecoContext.Release();
    }

    // Establish a connection with the recognition context's event source
    hr = IInkRecognitionEventsImpl<CAdvRecoApp>::DispEventAdvise(spNewContext);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Error connecting to the recognition context's event source!"),
                   gc_szAppName, MB_ICONERROR | MB_OK);
    }

    // Get the recognizer's capabilities flags.
    // (use IRC__DontCare if get_Capabilities fails)
    InkRecognizerCapabilities dwCapabilities;
    if (FAILED(pIInkRecognizer->get_Capabilities(&dwCapabilities)))
    {
        dwCapabilities = IRC_DontCare;
    }

    // Update the Guide menu items
    const UINT nGuideCmds[] = {ID_GUIDE_BOXES, ID_GUIDE_LINES, ID_GUIDE_NONE};
    const UINT nGuideFlags[] = {IRC_BoxedInput, IRC_LinedInput, IRC_FreeInput};
    HMENU hMenu = GetMenu();
    HMENU hSubMenu = hMenu ? ::GetSubMenu(hMenu, mc_iSubmenuGuides) : NULL;
    UINT nCmdGuid = 0;
    for (ULONG i = 0; i < countof(nGuideCmds) && i < countof(nGuideFlags); i++)
    {
        UINT nFlags;
        if (m_spIInkRecoGuide != NULL &&
            (((nGuideFlags[i] & dwCapabilities) == nGuideFlags[i])
            || ((IRC_DontCare & dwCapabilities) == IRC_DontCare)))
        {
            nFlags = MF_BYCOMMAND | MF_ENABLED;
            if ((0 == nCmdGuid) || (nGuideCmds[i] == m_nCmdGuide))
            {
                nCmdGuid = nGuideCmds[i];
            }
        }
        else
        {
            nFlags = MF_BYCOMMAND | MF_GRAYED;
        }
        // Update the menu item
        if (NULL != hSubMenu)
        {
            ::EnableMenuItem(hSubMenu, nGuideCmds[i], nFlags);
        }
    }

    // Change the guide selection if the current guide is not supported by the recognizer
    if (m_nCmdGuide != nCmdGuid)
    {
        SendMessage(WM_COMMAND, nCmdGuid);
    }

    // Set the recognition context properties before attaching the stroke collection to it

    // Set the guide
    if (m_spIInkRecoGuide != NULL && 0 != m_nCmdGuide)
    {
        if (FAILED(spNewContext->putref_Guide(m_spIInkRecoGuide)))
        {
            MessageBox(TEXT("Failed to set guide to the new recognition context!"),
                       gc_szAppName, MB_ICONERROR | MB_OK);
        }
    }

    // Clear currently set input scope. -1 is being used to indicate no input scope
    m_nCmdInputScope = -1;
    m_bCoerceInputScope = false;

    // Reset the Input Scope to baseline
    CComBSTR bstrFactoid(FACTOID_DEFAULT);
    if (FAILED(spNewContext->put_Factoid(bstrFactoid)))
    {
        MessageBox(TEXT("Failed to set factoid to the new recognition context!"),
                         gc_szAppName, MB_ICONERROR | MB_OK);
    }

    // Attach the stroke collection to the context
    hr = spNewContext->putref_Strokes(m_spIInkStrokes);
    if (FAILED(hr))
    {
        MessageBox(TEXT("Error attaching the stroke collection to the new recognition context!"),
                   gc_szAppName, MB_ICONERROR | MB_OK);
    }

    // Use the new context
    m_spIInkRecoContext.Attach(spNewContext.Detach());

    // Update the radio item check state of the input scope menu
    UpdateMenuRadioItems(mc_iSubmenuInputScopes, ID_INPUTSCOPE_FIRST, m_nCmdInputScope);

    // Update the enabled/disabled state of the input scope menu
    UpdateInputScopeMenu();

    // Update the coerce input scope menu item
    hSubMenu = hMenu ? ::GetSubMenu(hMenu, mc_iSubmenuInputScopes) : NULL;
    if (NULL != hSubMenu)
    {
        ::CheckMenuItem(hSubMenu, ID_INPUTSCOPE_COERCE, MF_BYCOMMAND | MF_UNCHECKED);
    }

    // Select an appropriate font for the recognition output
    LANGID wLandId = ::GetUserDefaultLangID();
    CComVariant vLangIDs;
    if (SUCCEEDED(pIInkRecognizer->get_Languages(&vLangIDs)) && NULL != vLangIDs.parray)
    {
        WORD* pwLIDs;
        if (SUCCEEDED(::SafeArrayAccessData(vLangIDs.parray, (void HUGEP**)&pwLIDs)))
        {
            wLandId = pwLIDs[0];
            ::SafeArrayUnaccessData(vLangIDs.parray);
        }
    }
    if (false == m_wndResults.UpdateFont(wLandId))
    {
        MessageBox(TEXT("Can not find an appropriate font for the selected recognizer.\n")
                   TEXT("The default font will be used for text output"), gc_szAppName);
    }

    // Reset the current results
    m_wndResults.ResetResults();
    m_wndResults.Invalidate();

    // Update the recognition results
    CComVariant vCustomData;    // no custom data
    m_spIInkRecoContext->BackgroundRecognizeWithAlternates(vCustomData);

    // restore the cursor
    ::SetCursor(hCursor);

    return true;
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnLVColumnClick
//
// When user clicks on the column header in either of the gesture
// listview controls, the CAdvRecoApp object receives a WM_NOTIFY
// message that is mapped to this handler for the actual processing.
// The application checks or unchecks all the items in the control
// the notification came from.
//
// Parameters:
//      defined in the ATL's macro NOTIFY_HANDLER
//
// Return Values (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CAdvRecoApp::OnLVColumnClick(
        int idCtrl,
        LPNMHDR /*pnmh*/,
        BOOL& bHandled
        )
{
    if (mc_iSSGestLVId == idCtrl)
    {
        m_bAllSSGestures = !m_bAllSSGestures;
        ListView_SetCheckState(m_hwndSSGestLV, -1, m_bAllSSGestures);
    }
    else if (mc_iMSGestLVId == idCtrl)
    {
        m_bAllMSGestures = !m_bAllMSGestures;
        ListView_SetCheckState(m_hwndMSGestLV, -1, m_bAllMSGestures);
    }
    else
    {
        bHandled = FALSE;
    }

    return 0;
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::OnLVItemChanging
//
// When user checks or unchecks an item in either of the gesture
// listview controls, the state of the item is changing and the CAdvRecoApp
// object receives a WM_NOTIFY message that is mapped to this handler
// for the actual processing.
// The application set or reset the gesture status in the InkCollector.
//
// Parameters:
//      defined in the ATL's macro NOTIFY_HANDLER
//
// Return Values (LRESULT):
//      TRUE to
//
/////////////////////////////////////////////////////////
LRESULT CAdvRecoApp::OnLVItemChanging(
        int idCtrl,
        LPNMHDR pnmh,
        BOOL& /*bHandled*/
        )
{
    if (m_spIInkCollector == NULL)
        return FALSE;

    LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pnmh;

    LRESULT lRet;

    // Ignore all the changes which are not of the item state,
    // and the item state changes other then checked/unchecked (LVIS_STATEIMAGEMASK)
    if (LVIF_STATE == pnmv->uChanged && 0 != (LVIS_STATEIMAGEMASK & pnmv->uNewState))
    {
        lRet = TRUE;   // prevent the change if something is wrong
        BOOL bChecked = ((LVIS_STATEIMAGEMASK & pnmv->uNewState) >> 12) == 2;
        InkApplicationGesture igtGesture = IAG_NoGesture;
        if (mc_iSSGestLVId == idCtrl)
        {
            if (pnmv->iItem >= 0 && pnmv->iItem < countof(gc_igtSingleStrokeGestures))
                igtGesture = gc_igtSingleStrokeGestures[pnmv->iItem];
        }
        else if (mc_iMSGestLVId == idCtrl)
        {
            if (pnmv->iItem >= 0 && pnmv->iItem < countof(gc_igtMultiStrokeGestures))
                igtGesture = gc_igtMultiStrokeGestures[pnmv->iItem];
        }

        if (IAG_NoGesture != igtGesture && SUCCEEDED(
            m_spIInkCollector->SetGestureStatus(igtGesture, bChecked ? VARIANT_TRUE : VARIANT_FALSE)))
        {
            // Allow the change in the control's item state
            lRet = FALSE;
        }
    }
    else
    {
        // Allow all the other changes
        lRet = FALSE;
    }

    return lRet;
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::CreateChildWindows
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
bool CAdvRecoApp::CreateChildWindows()
{
    if ((m_wndInput.Create(m_hWnd, CWindow::rcDefault, NULL,
                           WS_CHILD, WS_EX_CLIENTEDGE, (UINT)mc_iInputWndId) == NULL)
        || (m_wndResults.Create(m_hWnd, CWindow::rcDefault, NULL,
                                WS_CHILD, WS_EX_CLIENTEDGE, (UINT)mc_iOutputWndId) == NULL))
    {
        return false;
    }


    HINSTANCE hInst = _Module.GetResourceInstance();

    // Create a listview control for the list of the single stroke gestures
    m_hwndSSGestLV = ::CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
                                      WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT,
                                      0, 0, 1, 1,
                                      m_hWnd, (HMENU)mc_iSSGestLVId,
                                      _Module.GetModuleInstance(), NULL);
    if (NULL == m_hwndSSGestLV)
        return false;

    //
    ListView_SetExtendedListViewStyleEx(m_hwndSSGestLV, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);

    // Create a column
    LV_COLUMN lvC;
    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvC.fmt = LVCFMT_LEFT;
    lvC.iSubItem = 0;
    lvC.cx = mc_cxGestLVWidth - 22;
    lvC.pszText = TEXT("Single Stroke Gestures");
    if (-1 == ListView_InsertColumn(m_hwndSSGestLV, lvC.iSubItem, &lvC))
        return false;

    // Insert items - the names of the single stroke gestures.
    TCHAR szText[100];  // large enough to load a gesture name into
    LV_ITEM lvItem;
    lvItem.mask = LVIF_TEXT /*| LVIF_IMAGE*/ | LVIF_STATE;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.pszText = szText;
    lvItem.iSubItem = 0;
    for (ULONG i = 0; i < mc_cNumSSGestures; i++)
    {
        lvItem.iItem = i;
        // Load the names from the application resource, there should be
        // mc_cNumSSGestures names there with sequential id's starting
        // with IDS_SSGESTURE_FIRST
        ::LoadString(hInst, IDS_SSGESTURE_FIRST + i, szText, countof(szText));
        if (-1 == ListView_InsertItem(m_hwndSSGestLV, &lvItem))
            return false;
    }


    // Create a listview control for the list of the multi-stroke gestures
    m_hwndMSGestLV = ::CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
                                      WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT,
                                      0, 0, 1, 1,
                                      m_hWnd, (HMENU)mc_iMSGestLVId,
                                      _Module.GetModuleInstance(), NULL);
    if (NULL == m_hwndMSGestLV)
        return false;
    //
    ListView_SetExtendedListViewStyleEx(m_hwndMSGestLV, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);

    // Create a column
    lvC.pszText = TEXT("Multiple Stroke Gestures");
    ListView_InsertColumn(m_hwndMSGestLV, lvC.iSubItem, &lvC);

    // Insert items - the names of the single stroke gestures.
    for (ULONG i = 0; i < mc_cNumMSGestures; i++)
    {
        lvItem.iItem = i;
        // Load the names from the application resource, there should be
        // mc_cNumMSGestures names there with sequential id's starting
        // with IDS_MSGESTURE_FIRST
        ::LoadString(hInst, IDS_MSGESTURE_FIRST + i, szText, countof(szText));
        if (-1 == ListView_InsertItem(m_hwndMSGestLV, &lvItem))
            return false;
    }

    // Create a status bar (Ignore if it fails, the application can live without it).
    m_hwndStatusBar = ::CreateStatusWindow(
                        WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|SBARS_SIZEGRIP,
                        NULL, m_hWnd, (UINT)mc_iStatusWndId);
    if (NULL != m_hwndStatusBar)
    {
        ::SendMessage(m_hwndStatusBar,
                      WM_SETFONT,
                      (LPARAM)::GetStockObject(DEFAULT_GUI_FONT), FALSE);
    }

    // Update the child windows' positions and sizes so that they cover
    // entire client area of the main window.
    UpdateLayout();

    return true;
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::UpdateLayout
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
void CAdvRecoApp::UpdateLayout()
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

    // update the size and position of the gesture listviews
    if (::IsWindow(m_hwndSSGestLV) && ::IsWindow(m_hwndMSGestLV))
    {
        if (ID_MODE_INK != m_nCmdMode)
        {
            // calculate the rectangle covered by the list views
            RECT rcGest = rect;
            if (rcGest.right < mc_cxGestLVWidth)
            {
                rcGest.left = 0;
            }
            else
            {
                rcGest.left = rcGest.right - mc_cxGestLVWidth;
            }

            rect.right = rcGest.left;

            if (ID_MODE_GESTURES == m_nCmdMode)
            {
                int iHeight;
                RECT rcItem;
                if (TRUE == ListView_GetItemRect(m_hwndMSGestLV, 0, &rcItem, LVIR_BOUNDS))
                {
                    iHeight = rcItem.top + (rcItem.bottom - rcItem.top)
                                            * (countof(gc_igtMultiStrokeGestures) + 1);
                }
                else
                {
                    iHeight = (rcGest.bottom - rcGest.top) / 3;
                }

                // show the multiple stroke gesture listview control
                ::SetWindowPos(m_hwndMSGestLV, NULL,
                               rcGest.left, rcGest.bottom - iHeight,
                               rcGest.right - rcGest.left, iHeight,
                               SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
                rcGest.bottom -= iHeight;
            }
            else if (WS_VISIBLE ==
                    (((DWORD)::GetWindowLong(m_hwndMSGestLV, GWL_STYLE)) & WS_VISIBLE))
            {
                // hide the multiple stroke gesture listview control
                ::ShowWindow(m_hwndMSGestLV, SW_HIDE);
            }

            // show the single stroke gesture listview control
            ::SetWindowPos(m_hwndSSGestLV, NULL,
                           rcGest.left, rcGest.top,
                           rcGest.right - rcGest.left, rcGest.bottom - rcGest.top,
                           SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }
        else
        {
            // hide the single stroke gesture listview control
            if (WS_VISIBLE ==
                    (((DWORD)::GetWindowLong(m_hwndSSGestLV, GWL_STYLE)) & WS_VISIBLE))
            {
                ::ShowWindow(m_hwndSSGestLV, SW_HIDE);
            }
            // hide the multiple stroke gesture listview control
            if (WS_VISIBLE ==
                    (((DWORD)::GetWindowLong(m_hwndMSGestLV, GWL_STYLE)) & WS_VISIBLE))
            {
                ::ShowWindow(m_hwndMSGestLV, SW_HIDE);
            }
        }
    }

    // update the size and position of the output window
    if (::IsWindow(m_wndResults.m_hWnd))
    {
        int cyResultsWnd = m_wndResults.GetBestHeight();
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
// CAdvRecoApp::UpdateStatusBar
//
// This helper function outputs the names of the currently selected
// recognizer and input scope to the application's status bar.
//
// Parameters:
//      none
//
// Return Values (void):
//      none
//
/////////////////////////////////////////////////////////
void CAdvRecoApp::UpdateStatusBar()
{
    if (NULL == m_hwndStatusBar)
        return;

    UINT iInputScope = m_nCmdInputScope - ID_INPUTSCOPE_FIRST;
    if (iInputScope >= sizeof(gc_pwsInputScopes)/sizeof(gc_pwsInputScopes[0]))
    {
        iInputScope = 0;
    }

    CComBSTR bstrStatus(m_bstrCurRecoName);
    // If an input scope is set, add to the status bar string
    if (m_nCmdInputScope != -1)
    {
        bstrStatus += L"; Input Scope: ";
        bstrStatus += gc_pwsInputScopes[iInputScope];
    }

    // Set the new text int the status bar
    ::SendMessage(m_hwndStatusBar, SB_SETTEXTW, NULL, (LPARAM)bstrStatus.m_str);
}


/////////////////////////////////////////////////////////
//
// CAdvRecoApp::GetGestureName
//
// This helper function returns the resource id of
// the string with the name of the given gesture.
//
// Parameters:
//      InkApplicationGesture idGesture    : [in] the gesture's id
//      UINT& idGestureName         : [out] the id of the string with the the name
//                                    of the gesturein the resource string table
// Return Values (bool):
//      true if the gesture is known to the application, false otherwise
//
/////////////////////////////////////////////////////////
bool CAdvRecoApp::GetGestureName(
        InkApplicationGesture igtGesture,
        UINT& idGestureName
        )
{
    idGestureName = IDS_GESTURE_UNKNOWN;

    // This function should not be called when the collection mode is ICM_InkOnly
    if (ID_MODE_INK == m_nCmdMode)
        return false;

    // First, try to find the gesture among the single stroke ones
    ULONG iCount = countof(gc_igtSingleStrokeGestures);
    ULONG i;
    for (i = 0; i < iCount; i++)
    {
        if (gc_igtSingleStrokeGestures[i] == igtGesture)
        {
            idGestureName = IDS_SSGESTURE_FIRST + i;
            break;
        }
    }

    // If this is not a known single stroke gesture and the current collection mode
    // is ICM_GestureOnly, this may be a multi-stroke one.
    if (i == iCount && ID_MODE_GESTURES == m_nCmdMode)
    {
        iCount = countof(gc_igtMultiStrokeGestures);
        for (i = 0; i < iCount; i++)
        {
            if (gc_igtMultiStrokeGestures[i] == igtGesture)
            {
                idGestureName = IDS_MSGESTURE_FIRST + i;
                break;
            }
        }
    }

    return (IDS_GESTURE_UNKNOWN != idGestureName);
}

/////////////////////////////////////////////////////////
//
// CAdvRecoApp::PresetGestures
//
// Sets the status of the recommended subset of gestures
// to TRUE in InkCollector
//
// Parameters:
//      none
//
// Return Values (void):
//      none
//
/////////////////////////////////////////////////////////
void CAdvRecoApp::PresetGestures()
{
    // This function should not be called before the listview controls have been created
    if (0 == ::IsWindow(m_hwndSSGestLV) || 0 == ::IsWindow(m_hwndMSGestLV))
        return;

    // Set the status of the single stroke gestures
    ULONG iNumGestures = countof(gc_igtSingleStrokeGestures);
    ULONG iNumSubset = countof(gc_nRecommendedForMixedMode);
    for (ULONG i = 0; i < iNumSubset; i++)
    {
        if (gc_nRecommendedForMixedMode[i] < iNumGestures)
            ListView_SetCheckState(m_hwndSSGestLV, gc_nRecommendedForMixedMode[i], TRUE);
    }

    // Set the status of the multiple stroke gestures
    iNumGestures = countof(gc_igtMultiStrokeGestures);
    for (ULONG i = 0; i < iNumGestures; i++)
    {
        ListView_SetCheckState(m_hwndMSGestLV, i, TRUE);
    }
}
