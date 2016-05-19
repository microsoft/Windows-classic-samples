// RemoteHost.cpp: implementation of the RemoteHost class.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include "stdafx.h"
#include "RemoteHost.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CRemoteHost

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRemoteHost::CRemoteHost()
{

}

CRemoteHost::~CRemoteHost()
{

}

//***************************************************************************
// QueryService()
// API from IServiceProvider
//***************************************************************************
HRESULT CRemoteHost::QueryService(REFGUID /*guidService*/, REFIID riid, void ** ppv)
{
    return ppv? QueryInterface(riid, ppv) : E_POINTER;
}


//***************************************************************************
// GetServiceType()
// Always return Remote so that the player OCX runs at remote state
//***************************************************************************
HRESULT CRemoteHost::GetServiceType(BSTR * pbstrType)
{
    HRESULT hr = E_POINTER;
    if(pbstrType)
    {
        *pbstrType = ::SysAllocString(L"Remote");
        hr = *pbstrType? S_OK : E_POINTER;
    }
    return hr;
}

//***************************************************************************
// GetApplicationName()
// Return the application name. It will be shown in player's menu View >
// Switch to applications
//***************************************************************************
HRESULT CRemoteHost::GetApplicationName(BSTR * pbstrName)
{
    HRESULT     hr = E_POINTER;
    if(pbstrName)
    {
        CComBSTR    bstrAppName = L"";
        bstrAppName.LoadString(IDS_PROJNAME);
        *pbstrName = bstrAppName.Detach();
        hr = *pbstrName? S_OK : E_POINTER;
    }
    return hr;
}

//***************************************************************************
// GetScriptableObject()
// There is no scriptable object in this application
//***************************************************************************
HRESULT CRemoteHost::GetScriptableObject(BSTR * pbstrName, IDispatch ** ppDispatch)
{
    if(pbstrName)
    {
        *pbstrName = NULL;
    }
    if(ppDispatch)
    {
        *ppDispatch = NULL;
    }
    return E_NOTIMPL;
}

//***************************************************************************
// GetCustomUIMode()
// When UI mode of the player OCX is set to custom, this function is called
// to give the skin file path that will be loaded to the player OCX.
// 
//***************************************************************************
HRESULT CRemoteHost::GetCustomUIMode(BSTR * pbstrFile)
{
    HRESULT     hr = E_POINTER;
    if (pbstrFile) 
    {
        WCHAR       wszCurDir[MAX_PATH];
        CComBSTR    bstrSkinPath = L"file://";
        CComBSTR    bstrSkinFilename;

        GetCurrentDirectory(MAX_PATH, wszCurDir);
        bstrSkinFilename.LoadString(IDS_SKINFILE);

        bstrSkinPath.Append(wszCurDir);
        bstrSkinPath.Append(L"\\");
        bstrSkinPath.Append(bstrSkinFilename);

        *pbstrFile = bstrSkinPath.Detach();
        hr = *pbstrFile? S_OK : E_POINTER;
    }

    return hr;
}
