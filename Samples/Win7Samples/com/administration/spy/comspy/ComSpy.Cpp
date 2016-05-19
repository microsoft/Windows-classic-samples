// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "ComSpy.h"
#include "ComSpyCtl.h"
#include "ComSpyCtl_i.c"
#include "ComSpy_i.c"
#include "SpyCon.h"


LONG CExeModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0)
    {
        if (CoSuspendClassObjects() == S_OK)
            PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    }
    return l;
}

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_SpyCon, CSpyCon)
END_OBJECT_MAP()


LPCWSTR FindOneOf(__in LPCWSTR pwszStringToSearch, __in LPCWSTR pwszCharactersToFind)
{
    while (*pwszStringToSearch != L'\0')
    {
        LPCWSTR pcCurrentCharacter = pwszCharactersToFind;
        while (*pcCurrentCharacter != L'\0')
        {
            if (*pwszStringToSearch == *pcCurrentCharacter++)
                return pwszStringToSearch+1;
        }
        pwszStringToSearch++;
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////
//
extern "C" int WINAPI wWinMain(HINSTANCE hInstance, 
    HINSTANCE /*hPrevInstance*/, LPWSTR lpCmdLine, int /*nShowCmd*/)
{
    lpCmdLine = GetCommandLine(); //this line necessary for _ATL_MIN_CRT
    HRESULT hr = CoInitialize(NULL);
    _ASSERTE(SUCCEEDED(hr));

    _Module.Init(ObjectMap, hInstance);
    _Module.dwThreadID = GetCurrentThreadId();
    WCHAR wszTokens[] = L"-/";

    int nRet = 0;
    bool bRun = true;
    LPCWSTR lpwszToken = FindOneOf(lpCmdLine, wszTokens);
    while (lpwszToken != NULL)
    {
        if (_wcsnicmp(lpwszToken, L"UnregServer", 11)==0)
        {
            _Module.UpdateRegistryFromResource(IDR_ComSpy, FALSE);
            nRet = _Module.UnregisterServer();
            bRun = false;
            break;
        }
        if (_wcsnicmp(lpwszToken, L"RegServer", 9)==0)
        {
            _Module.UpdateRegistryFromResource(IDR_ComSpy, TRUE);
            nRet = _Module.RegisterServer(TRUE);
            bRun = false;
            break;
        }
        lpwszToken = FindOneOf(lpwszToken, wszTokens);
    }

    if (bRun)
    {
        hr = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE);
        _ASSERTE(SUCCEEDED(hr));

        CComPtr<ISpyCon> spApp;
        hr = CoCreateInstance(CLSID_SpyCon, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&spApp));
        _ASSERTE(SUCCEEDED(hr));

        if (SUCCEEDED(spApp->Run()))
        {
            MSG msg;
            while (GetMessage(&msg, 0, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        _Module.RevokeClassObjects();
    }

    CoUninitialize();
    return nRet;
}
