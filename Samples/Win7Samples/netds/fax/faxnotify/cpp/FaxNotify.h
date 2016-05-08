//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

#ifndef __COM_FAXNOTIFY_SAMPLE

#define __COM_FAXNOTIFY_SAMPLE

//
//Includes
//
//
//Includes
//
#include <atlbase.h>

//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;

#include <atlcom.h>
#include <faxcomex.h>
#include <windows.h>
#include <winbase.h>
#include <stdlib.h>
#include <objbase.h>
#include <tchar.h>
#include <assert.h>
#include <shellapi.h>
#include <strsafe.h>


#define VISTA 6
//
// Copies a simple (built-in data types such as int, long etc) property
// on the specified interface into the _dst_
// _pInterface_ is a com interface which has a method named _propMethod_
// _dst_ is of same type as the property
// _hRes_ is HRESULT
// _errorLabel_ is where the control will be transfered to, in case of errors
//
#define GET_SIMPLE_PROPERTY(_pInterface_,_propMethod_,_dst_,_hRes_,_errorLabel_) \
{ \
        (_hRes_) = (_pInterface_)->_propMethod_(&(_dst_)); \
        if (FAILED(_hRes_)) { \
                goto _errorLabel_; } \
}

//
// Copies a date (DATE) property of the specified interface into the _dst_
// _pInterface_ is a com interface which has a method named _propMethod_
// _dst_ is of type SYSTEMTIME
// _hRes_ is HRESULT
//
#define GET_UTC_DATE_PROPERTY(_pInterface_,_propMethod_,_dst_,_hRes_) \
{ \
    DATE _vDate_; \
    SYSTEMTIME _dtTemp_; \
    (_hRes_) = (_pInterface_)->_propMethod_(&_vDate_); \
    if (SUCCEEDED(_hRes_)) { \
        if (!VariantTimeToSystemTime(_vDate_, &(_dtTemp_))) { \
            _hRes_ = E_INVALIDARG; \
        } \
    } \
}

#define VISTA 6

class CFaxNotify
{
        public:
                CFaxNotify(): 
                        m_pFaxServer(NULL), 
                        m_lpwzServerName(NULL)                        
                { }
                HRESULT Initialize(LPTSTR lptstrServerName);
                HRESULT Listen(BOOL bServerNotify);
                HRESULT Terminate();
        private: 
                CComPtr<IFaxServer2> m_pFaxServer;
                CComPtr<IFaxAccount> m_pFaxAccount;
                LPWSTR m_lpwzServerName;
};

inline HRESULT ValidateFaxAccount(CComPtr<IFaxAccount> pFaxAccount);

void DisplayJobStatus(CComPtr<IFaxJobStatus> pJobStatus);

inline HRESULT ValidateFaxServer(CComPtr<IFaxServer2> pFaxServer);

HRESULT GetConnectionPoint(CComPtr<IUnknown> pSource,
                REFIID riidOutgoingInterface,
                IConnectionPoint **ppIConnPoint);

HRESULT AttachToConnectionPoint(CComPtr<IUnknown> pSource,
                CComPtr<IUnknown> pSink,
                REFIID riidOutgoingInterface,
                LPDWORD lpdwCookie);

HRESULT DetachFromConnectionPoint(CComPtr<IUnknown> pSource,
                REFIID riidOutgoingInterface,
                DWORD dwCookie);

#endif // __COM_FAXNOTIFY_SAMPLE

