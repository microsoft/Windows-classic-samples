/////////////////////////////////////////////////////////////////////////////
//
// SetContentDistributor.cpp : Utility to set the content distributor ID in a media file.
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "wmsdkidl.h"

HRESULT SetContentDistributor(WCHAR *pszFilename, WCHAR *pszProvider)
{
    CComPtr<IWMMetadataEditor> spEditor;
    HRESULT hr = WMCreateEditor(&spEditor);
    if (SUCCEEDED(hr))
    {
        hr = spEditor->Open(pszFilename);
        if (SUCCEEDED(hr))
        {
            CComPtr<IWMHeaderInfo> spHeaderInfo;
            hr = spEditor->QueryInterface(__uuidof(IWMHeaderInfo), reinterpret_cast<void**>(&spHeaderInfo));
            if (SUCCEEDED(hr))
            {
                WORD wLength = sizeof(WCHAR)*(wcslen(pszProvider) + 1);
                hr = spHeaderInfo->SetAttribute(0, g_wszWMContentDistributor, WMT_TYPE_STRING, (const BYTE*)pszProvider, wLength);
            }
            spEditor->Flush();
            spEditor->Close();
        }
    }

    return hr;
}

int wmain(int argc, WCHAR* argv[])
{
    if (3 != argc)
    {
        wprintf(L"SetContentDistributor <path> <content distributor>\n\n");
        wprintf(L"Arguments:\n");
        wprintf(L"    <path>         Path to media file\n");
        wprintf(L"    <distributor>  ID of content distributor\n");
        return -1;
    }
    
    HRESULT hr = SetContentDistributor(argv[1], argv[2]);

    if (FAILED(hr))
    {
        wprintf(L"Error setting provider: %08X\n");
        return -1;
    }

    return 0;
}

