// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef __MFERRORHANDLER__
#define __MFERRORHANDLER__

#include <mfidl.h>
#include <mfapi.h>
#include <mferror.h>
#include <strsafe.h>

class CMFErrorHandler
{
public:
    CMFErrorHandler()
        : m_hErrorModule(NULL)
    {
        m_hErrorModule = ::LoadLibraryEx(L"mferror.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
    }

    ~CMFErrorHandler()
    {
        ::FreeLibrary(m_hErrorModule);
    }
    
    void HandleMFError(const CAtlStringW& message, HRESULT hr)
    {
        MessageBox(m_hWndParent, message + L"\n" + HRToString(hr), LoadAtlString(IDS_MF_ERROR), MB_OK);
    }

    CAtlStringW HRToString(HRESULT hr)
    {
        CAtlStringW errStr;

        if(!m_hErrorModule || !IsMFError(hr))
        {
            UINT nID = 0;

            switch(hr) 
            {
                case E_OUTOFMEMORY:         nID = IDS_E_OUTOFMEMORY; break;
                case MF_E_UNSUPPORTED_BYTESTREAM_TYPE: nID = IDS_MF_E_UNSUPPORTED_BYTESTREAM_TYPE; break;
                case MF_E_INVALID_FORMAT:   nID = IDS_MF_E_INVALID_FORMAT; break;
                case MF_E_UNEXPECTED:       nID = IDS_MF_E_UNEXPECTED; break;
                case E_FAIL:                nID = IDS_E_FAIL; break;
                case E_POINTER:             nID = IDS_E_POINTER; break;
                case E_INVALIDARG:          nID = IDS_E_INVALIDARG; break;
                case E_NOTIMPL:             nID = IDS_E_NOTIMPL; break;
                case E_NOINTERFACE:         nID = IDS_E_NOINTERFACE; break;
                case REGDB_E_CLASSNOTREG:   nID = IDS_REGDB_E_CLASSNOTREG; break;
                case TED_E_TRANSCODE_PROFILES_FILE_INVALID: nID = IDS_E_TRANSCODE_PROFILES; break;
                case TED_E_INVALID_TRANSCODE_PROFILE:       nID = IDS_E_TRANSCODE_PROFILE; break;

            }

            if( nID )
            {
                (void)errStr.LoadString(nID);
            }
            else
            {
                WCHAR *pszDescription = errStr.GetBuffer(m_dwDescriptionLength);
                if( pszDescription )
                {
                    FormatMessage((FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS), NULL, hr, 0,
                        pszDescription, m_dwDescriptionLength, NULL);
                }
                errStr.ReleaseBuffer();
            }
        }
        else
        {
            WCHAR *pszDescription = errStr.GetBuffer(m_dwDescriptionLength);
            if( pszDescription )
            {
                FormatMessage((FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS), m_hErrorModule, hr, 0,
                    pszDescription, m_dwDescriptionLength, NULL);
            }
            errStr.ReleaseBuffer();
        }

        if( errStr.IsEmpty() )
        {
            errStr.FormatMessage(IDS_E_UNKNOWN, hr);
        }

        return errStr;
    }

    bool IsMFError(HRESULT hr)
    {
        return ( (hr & m_dwErrorMask) == m_dwMFErrorPrefix);
    }
    
    void SetParentWnd(HWND hWndParent)
    {
        m_hWndParent = hWndParent;
    }
    
private:
    HINSTANCE m_hErrorModule;
    HWND m_hWndParent;
    static const DWORD m_dwMFErrorPrefix = 0xc00d0000;
    static const DWORD m_dwErrorMask = 0xffff0000;
    static const DWORD m_dwDescriptionLength = 500;
};

#endif
