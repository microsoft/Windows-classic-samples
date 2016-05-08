//------------------------------------------------------------------------------
// File: GargProp.cpp
//
// Desc: DirectShow sample code - implementation of CGargleProperties class.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>

// Eliminate two expected level 4 warnings from the Microsoft compiler.
// The class does not have an assignment or copy operator, and so cannot
// be passed by value.  This is normal.  This file compiles clean at the
// highest (most thorough) warning level (-W4).
#pragma warning(disable: 4511 4512)

#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <math.h>

#include "resource.h"
#include "igargle.h"
#include "gargprop.h"


//
// CreateInstance
//
// Override CClassFactory method.
// Set lpUnk to point to an IUnknown interface on a new CGargleProperties object
//
CUnknown * WINAPI CGargleProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    ASSERT(phr);
    
    CUnknown *punk = new CGargleProperties(lpunk, phr);
    if (punk == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

    return punk;

} // CreateInstance


//
// CGargleProperties constructor
//
// initialise a CGargleProperties object.
//
CGargleProperties::CGargleProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage(NAME("Gargle Property Page"), lpunk, 
                        IDD_GARGPROP, IDS_NAME)
    , m_pGargle(NULL)
{

} // (constructor)


//
// SetButtonText
//
// Utility function to set the text of the button to reflect the curent
// waveform being used
//
void SetButtonText(HWND hwnd, int iShape)
{
    if (iShape==0)
        SetDlgItemText(hwnd, IDB_SQUARE_TRIANGLE, TEXT("Triangle wave->square"));
    else
        SetDlgItemText(hwnd, IDB_SQUARE_TRIANGLE, TEXT("Square wave->triangle"));

} // SetButtonText


//
// ConvertToPosition
//
// Convert a frequency f (which is the gargle rate) to a slider position.
// f runs from 1 to 1000.
// p runs from 0 to 300.
// f = 10**(p/100)
// p = 100*log10(f)
//
int ConvertToPosition(int f)
{
    // protect against unexpected input
    if (f < 1)    f = 1;
    if (f > 1000) f = 1000;

    double x = f;
    x = 100.0 * log10(x);

    // protect against rounding at the ends
    int p = (int)x;
    if (p < 0)   p = 0;
    if (p > 300) p = 300;

    return p;

} // ConvertToPosition


//
// ConvertToFrequency
//
// Convert a slider position p to a frequency (gargle rate).
// f runs from 1 to 1000.
// p runs from 0 to 300.
// f = 10**(p/100)
// p = 100*log10(f)
//
int ConvertToFrequency(int p)
{
    // protect against unexpected input
    if (p < 0)   p = 0;
    if (p > 300) p = 300;

    double x = p;
    x = pow(10.0, x/100.0);

    // protect against rounding at the ends
    int f = (int)x;
    if (f < 1)    f = 1;
    if (f > 1000) f = 1000;

    return f;

} // ConvertToFrequency


//
// OnReceiveMessage
//
// Override CBasePropertyPage method.
// Handle windows messages for the dialog of the property sheet.
//
INT_PTR CGargleProperties::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (uMsg) 
    {
        case WM_INITDIALOG:

            m_hwndSlider = CreateSlider(hwnd);
            SetButtonText(hwnd, m_iGargleShape);
            ASSERT(m_hwndSlider);
            return TRUE;                // I don't call setfocus...

        case WM_VSCROLL:

            ASSERT(m_hwndSlider);
            OnSliderNotification(wParam);
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDB_SQUARE_TRIANGLE) 
            {
                // Cycle through shapes.  See note below.
                //
                int iShape;

                m_pGargle->get_GargleShape(&iShape);  // find what we now have
                iShape = 1-iShape;                            // change shapes

                m_pGargle->put_GargleShape(iShape);             // put it back
                SetButtonText(hwnd, iShape);              // reflect in dialog

            } 
            else if (LOWORD(wParam) == IDB_DEFAULT) 
            {
                // Restore the default settings.

                // Set the filter.
                //
                m_pGargle->put_DefaultGargleRate();
                m_pGargle->put_GargleShape(0);

                // Set the slider position on the screen to match.
                //
                int iPos;
                m_pGargle->get_GargleRate(&iPos);

                iPos = ConvertToPosition(iPos);
                SendMessage(m_hwndSlider, TBM_SETPOS, TRUE, iPos);

                // Set the button text to match.
                //
                int iShape;

                m_pGargle->get_GargleShape(&iShape);  // find out what we now have
                SetButtonText(hwnd, iShape);

            } else 
            {
                // Should not occur - debug!
                //
                TCHAR Buff[100];
                (void)StringCchPrintf(Buff, NUMELMS(Buff), TEXT("wParam=%08x\0"), wParam);

                #ifdef DEBUG
                    DbgBreakPoint( Buff, TEXT(__FILE__), __LINE__ );
                #else
                    ASSERT(!"Should not occur - debug!");
                #endif
            }

            return TRUE;

        case WM_DESTROY:

            DestroyWindow(m_hwndSlider);
            return TRUE;

        default:

            return FALSE;

    } // switch

} // OnReceiveMessage


//
// OnConnect
//
// Override CBasePropertyPage method.
// Get the interface to the filter.
// Set the member variables m_iGargleRate and m_iGargleShape.
//
HRESULT CGargleProperties::OnConnect(IUnknown * punk)
{
    // Get IGargle interface
    //
    if (punk == NULL) {
        DbgBreak("You can't call OnConnect with a NULL pointer!!");
        return(E_POINTER);
    }

    ASSERT(m_pGargle == NULL);

    HRESULT hr = punk->QueryInterface(IID_IGargle, (void **) &m_pGargle);
    if (FAILED(hr)) {
        DbgBreak("Can't get IGargle interface!");
        return E_NOINTERFACE;
    }

    CheckPointer(m_pGargle,E_FAIL);
    m_pGargle->get_GargleRate(&m_iGargleRate);
    m_pGargle->get_GargleShape(&m_iGargleShape);

    return NOERROR;

} // OnConnect


//
// OnDisconnect
//
// Override CBasePropertyPage method.
// Release the private interface.
//
HRESULT CGargleProperties::OnDisconnect()
{
    //
    // Release the interface
    //
    if (!m_pGargle)
        return E_UNEXPECTED;

    m_pGargle->Release();
    m_pGargle = NULL;

    return(NOERROR);

} // OnDisconnect



//
// OnDeactivate
//
// Destroy the dialog.
//
HRESULT CGargleProperties::OnDeactivate(void)
{
    ASSERT(m_pGargle);
    
    //
    // Remember the Gargle Rate and shape for the next Activate() call
    //
    m_pGargle->get_GargleRate(&m_iGargleRate);
    m_pGargle->get_GargleShape(&m_iGargleShape);

    return NOERROR;

} // OnDeactivate


//
// CreateSlider
//
// Create the slider (common control) to allow the user to
// adjust the gargling rate.
//
HWND CGargleProperties::CreateSlider(HWND hwndParent)
{
    // Find how to convert dialog units to screen units
    //
    LONG XUnit = GetDialogBaseUnits();
    LONG YUnit = XUnit>>16;
    XUnit = XUnit & 0x0000ffff;

    // Create the slider child window at a position which fits the dialog
    //
    HWND hwndSlider = CreateWindow( TRACKBAR_CLASS
                                  , TEXT("")
                                  , WS_CHILD | WS_VISIBLE | TBS_VERT | TBS_BOTH
                                  , 15*XUnit          // x
                                  , 0                 // y
                                  , 5*XUnit           // width
                                  , 5*YUnit           // Height
                                  , hwndParent
                                  , NULL              // menu
                                  , g_hInst
                                  , NULL              // CLIENTCREATESTRUCT
                                  );
    if (hwndSlider == NULL) {       
        DbgLog((LOG_ERROR, 1,
               TEXT("Could not create window.  error code: 0x%x"), GetLastError()));
        return NULL;
    }

    // Set the range to 0..300 which is converted into Hz as 10**(p/100)
    // where p is the slider position.  SeeConvertToFrequency() above.
    //
    SendMessage(hwndSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 300) );

    // Set a tick at the default of 10Hz which is 100 on the log scale.
    // Put another one at 100Hz which corresponds to 200 on the log scale.
    //
    SendMessage(hwndSlider, TBM_SETTIC, 0, 100L);
    SendMessage(hwndSlider, TBM_SETTIC, 0, 200L);

    // Set the slider position according to the value we obtain from
    // initialisation or from the last Deactivate() call.
    //
    int iPos = ConvertToPosition(m_iGargleRate);
    SendMessage(hwndSlider, TBM_SETPOS, TRUE, iPos);

    return hwndSlider;

}  // CreateSlider


//
// OnSliderNotification
//
// Handle the notification meesages from the slider control
//
void CGargleProperties::OnSliderNotification(WPARAM wParam)
{
    int iPos;

    switch (wParam) 
    {
        case TB_BOTTOM:
            iPos = ConvertToPosition(MinGargleRate);
            SendMessage(m_hwndSlider, TBM_SETPOS, TRUE, (LPARAM) iPos);
            break;

        case TB_TOP:
            iPos = ConvertToPosition(MaxGargleRate);
            SendMessage(m_hwndSlider, TBM_SETPOS, TRUE, (LPARAM) iPos);
            break;

        case TB_PAGEDOWN:
        case TB_PAGEUP:
            break;

        case TB_THUMBPOSITION:
        case TB_ENDTRACK:
        {
            int iRate = (int) SendMessage(m_hwndSlider, TBM_GETPOS, 0, 0L);
            iRate = ConvertToFrequency(iRate);
            m_pGargle->put_GargleRate(iRate);
            break;
        }

        case TB_THUMBTRACK: // default handling of these messages is ok.
        case TB_LINEDOWN:
        case TB_LINEUP:
            break;
    }

} // OnSliderNotification

#pragma warning(disable: 4514) // "unreferenced inline function has been removed"


