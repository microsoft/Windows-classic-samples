//////////////////////////////////////////////////////////////////////////
//
// mfutils.h : Miscellaneous helper functions for Media Foundation.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <mftransform.h>

namespace MediaFoundationSamples
{
    const MFTIME ONE_SECOND = 10000000; // One second in hns
    const LONG   ONE_MSEC = 1000;       // One msec in hns 


    /*
        MFTimeToMsec

        CreateMediaSource
        GetCollectionObject
        GetEventObject
        GetStreamMajorType
        GetStreamCurrentMediaType
        GetTopoNodeObject
    */


    // MFTimeToMsec: Convert 100-ns time to seconds.
    inline LONG MFTimeToMsec(const LONGLONG& time)
    {
        return (LONG)(time / (ONE_SECOND / ONE_MSEC));
    }

    ///////////////////////////////////////////////////////////////////////
    //  Name: CreateMediaSource
    //  Description:  Create a media source from a URL.
    //
    //  sURL: The URL to open.
    //
    //  Note: This function uses the synchronous method on IMFSourceResolver 
    //  to create the media source. However, creating a media source can take 
    //  a noticeable amount of time, especially for a network source. For a 
    //  more responsive UI, use the asynchronous BeginCreateObjectFromURL 
    //  method.
    //
    /////////////////////////////////////////////////////////////////////////

    inline HRESULT CreateMediaSource(const WCHAR *sURL, IMFMediaSource **ppSource)
    {
        CheckPointer(sURL, E_POINTER);
        CheckPointer(ppSource, E_POINTER);

        HRESULT hr = S_OK;

        IMFSourceResolver   *pSourceResolver = NULL;
        IUnknown            *pSourceUnk = NULL;

        // Create the source resolver.
        if (SUCCEEDED(hr))
        {
            hr = MFCreateSourceResolver(&pSourceResolver);
        }

        // Use the source resolver to create the media source.
        if (SUCCEEDED(hr))
        {
            MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

            hr = pSourceResolver->CreateObjectFromURL(
                sURL,                       // URL of the source.
                MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
                NULL,                       // Optional property store.
                &ObjectType,                // Receives the created object type. 
                &pSourceUnk                 // Receives a pointer to the media source.
                );
        }

        // Get the IMFMediaSource interface from the media source.
        if (SUCCEEDED(hr))
        {
            hr = pSourceUnk->QueryInterface(__uuidof(IMFMediaSource), (void**)ppSource);
        }

        // Clean up
        SAFE_RELEASE(pSourceResolver);
        SAFE_RELEASE(pSourceUnk);

        return hr;
    }

    ///////////////////////////////////////////////////////////////////////
    //  Name: GetStreamMajorType
    //  Description:  Get the major media type from a stream descriptor.
    //
    //  Note: 
    //  To get the major media type from a stream descriptor, you need to go
    //  through the stream descriptor's media type handler. Use this helper
    //  function if you don't need the type handler for anything else.
    // 
    /////////////////////////////////////////////////////////////////////////

    inline HRESULT GetStreamMajorType(IMFStreamDescriptor *pSD, GUID *pguidMajorType)
    {
        CheckPointer(pSD, E_POINTER);
        CheckPointer(pguidMajorType, E_POINTER);

        HRESULT hr = S_OK;
        IMFMediaTypeHandler *pHandler = NULL;

        hr = pSD->GetMediaTypeHandler(&pHandler);
        if (SUCCEEDED(hr))
        {
            hr = pHandler->GetMajorType(pguidMajorType);
        }
        SAFE_RELEASE(pHandler);
        return hr;
    }


    //--------------------------------------------------------------------------------------
    // Name: AllocGetWindowText
    // Description: Helper function to get text from a window.
    //
    // This function allocates a buffer and returns it in pszText. The caller must
    // call CoTaskMemFree on the buffer.
    //
    // hwnd:     Handle to the window
    // pszText:  Receives a pointer to the string.
    // pcchLen:  Receives the length of the string, in characters, not including
    //           the terminating NULL character. This parameter can be NULL.
    //--------------------------------------------------------------------------------------

    inline HRESULT AllocGetWindowText(HWND hwnd, WCHAR **pszText, DWORD *pcchLen)
    {
        if (pszText == NULL)
        {
            return E_POINTER;
        }

        *pszText = NULL;  

        int cch = GetWindowTextLength(hwnd);  
        if (cch < 0) 
        {
            return E_UNEXPECTED; // This function should not return a negative value.
        }

        TCHAR *szTmp = (TCHAR*)CoTaskMemAlloc(sizeof(TCHAR) * (cch + 1)); // Include room for terminating NULL character

        if (!szTmp)
        {
            return E_OUTOFMEMORY;
        }

        if (cch == 0)
        {
            szTmp[0] = L'\0';  // No text.
        }
        else
        {
            int res = GetWindowText(hwnd, szTmp, (cch + 1));  // Size includes NULL character

            // GetWindowText returns 0 if (a) there is no text or (b) it failed.
            // We checked for (a) already, so 0 means failure here.
            if (res == 0)
            {
                CoTaskMemFree(szTmp);
                return __HRESULT_FROM_WIN32(GetLastError());
            }
        }

        // If we got here, szTmp is valid, so return it to the caller.
        *pszText = szTmp;
        if (pcchLen)
        {
            *pcchLen = static_cast<DWORD>(cch);  // Return the length NOT including the '\0' 
        }
        return S_OK;
    }
};