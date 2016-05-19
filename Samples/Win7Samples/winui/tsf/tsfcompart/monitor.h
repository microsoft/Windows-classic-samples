/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation. All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          Compart.h

   Description:   

**************************************************************************/

#ifndef COMPARTMENT_MONITOR_H
#define COMPARTMENT_MONITOR_H

/**************************************************************************
	#include statements
**************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <msctf.h>

typedef HRESULT (CALLBACK* PCOMPARTMENTMONITORPROC)(const GUID*, BOOL, LPARAM);

/**************************************************************************

	CCompartmentMonitor class definition

**************************************************************************/

class CCompartmentMonitor : public ITfCompartmentEventSink
{
public:
    CCompartmentMonitor(void);
    virtual ~CCompartmentMonitor();

    //
    // IUnknown methods
    //
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    //
    // ITfCompartmentEventSink
    //
    STDMETHODIMP OnChange(REFGUID rguid);

    //
    // APIs
    //
    HRESULT Initialize( const GUID *pguidCompartment,
                        PCOMPARTMENTMONITORPROC pCallback, 
                        LPARAM lParam);
    HRESULT Uninitialize(void);
    HRESULT GetStatus(BOOL *pfStatus);

private:
    DWORD                   m_dwRef;
    DWORD                   m_dwCookie;
    ITfCompartment          *m_pCompartment;
    GUID                    m_guidCompartment;
    PCOMPARTMENTMONITORPROC m_pCallback;
    LPARAM                  m_lParam;
};


#endif  //COMPARTMENT_MONITOR_H