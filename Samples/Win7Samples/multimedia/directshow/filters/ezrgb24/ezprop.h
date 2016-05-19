//------------------------------------------------------------------------------
// File: EZProp.h
//
// Desc: DirectShow sample code - definition of CEZrgb24Properties class.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <strsafe.h>

class CEZrgb24Properties : public CBasePropertyPage
{

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();

    void    GetControlValues();

    CEZrgb24Properties(LPUNKNOWN lpunk, HRESULT *phr);

    BOOL m_bIsInitialized;      // Used to ignore startup messages
    int m_effect;               // Which effect are we processing
    REFTIME m_start;            // When the effect will begin
    REFTIME m_length;           // And how long it will last for
    IIPEffect *m_pIPEffect;     // The custom interface on the filter

}; // EZrgb24Properties

