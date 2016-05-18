//------------------------------------------------------------------------------
// File: GargProp.h
//
// Desc: DirectShow sample code - definition of CGargleProperties class,
//       providing a properties page derived from the property page base
//       class for minimum effort.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <strsafe.h>

#ifndef __GARGPROP__
#define __GARGPROP__

#ifdef __cplusplus
extern "C" {
#endif


const int MaxGargleRate = 1000;    // 1000Hz max rate
const int MinGargleRate = 1;       // 1Hz min rate
const int DefaultGargleRate = 10;  // 10 Hz default


class CGargleProperties : public CBasePropertyPage
{

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    // Overrides from CBasePropertyPage
    HRESULT OnConnect(IUnknown * punk);
    HRESULT OnDisconnect(void);

    HRESULT OnDeactivate(void);

    CGargleProperties(LPUNKNOWN lpunk, HRESULT *phr);

private:

    INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND        CreateSlider(HWND hwndParent);
    void        OnSliderNotification(WPARAM wParam);

    HWND        m_hwndSlider;   // handle of slider

    IGargle   *m_pGargle;       // pointer to the IGargle interface of the
                                // gargle filter.  Set up in OnConnect.

    int        m_iGargleRate;   // Remember gargle rate between
                                // Deactivate / Activate calls.
    int        m_iGargleShape;  // 0 = triangle (default), 1 = square wave.

};  // class CGargleProperties

#ifdef __cplusplus
}
#endif

#endif // __GARGPROP__
