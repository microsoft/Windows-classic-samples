// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __SECURITY_H__
#define __SECURITY_H__

#include "stdafx.h"
#include "resource.h"       // main symbols

#define USERNAMELEN 64

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CSecuritySub
Summary: Security Events Subscriber 
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CSecuritySub : 
    public CSysLCESub,
    public IComSecurityEvents
{
public:
    CSecuritySub()
    {
        m_pSpy = NULL;
    }
    CSecuritySub(CComSpy * pSpy)
    {
        m_pSpy = pSpy;
    }

    DECLARE_NOT_AGGREGATABLE(CSecuritySub)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CSecuritySub)
        COM_INTERFACE_ENTRY(IComSecurityEvents)
        COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
    END_COM_MAP()

    virtual EventEnum EventType() { return Security; }
    virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComSecurityEvents; }

    STDMETHODIMP OnAuthenticate(  COMSVCSEVENTINFO * pInfo, 
                                  REFGUID guidActivity, 
                                  ULONG64 ObjectID,
                                  REFGUID guidIID, 
                                  ULONG iMeth,
                                  ULONG cbByteOrig,  
                                  BYTE * pSidOriginalUser,
                                  ULONG cbByteCur,   
                                  BYTE * pSidCurrentUser, 
                                  BOOL bCurrentUserInpersonatingInProc
                            )
    {

        m_pSpy->AddEventToList(pInfo->perfCount, L"OnAuthenticate", GuidToBstr(pInfo->guidApp));
        CComBSTR bstrGuidActivity = GuidToBstr(guidActivity);
        m_pSpy->AddParamValueToList(L"guidActivity", bstrGuidActivity);

        WCHAR sObjId[32];
        wsprintfW(sObjId,L"%#016I64X", ObjectID);
        m_pSpy->AddParamValueToList(L"ObjectID", sObjId);


        CComBSTR bstrIID = GuidToBstr(guidIID);
        m_pSpy->AddParamValueToList(L"guidIID", bstrIID);

        WCHAR sMethod[16];
        wsprintfW(sMethod,L"%#08X", iMeth);
        m_pSpy->AddParamValueToList(L"iMeth", sMethod);


        WCHAR sOriginalUser[256];
        ZeroMemory(sOriginalUser, sizeof(sOriginalUser));

        if (GetDomainSlashUser(pSidOriginalUser, sOriginalUser, ARRAYSIZE(sOriginalUser)))
            m_pSpy->AddParamValueToList(L"Original User", sOriginalUser);
        else
            m_pSpy->AddParamValueToList(L"Original User", L"<Error converting SID>");

        WCHAR sDirectUser[256];
        ZeroMemory(sDirectUser, sizeof(sDirectUser));

        if (GetDomainSlashUser(pSidCurrentUser, sDirectUser, ARRAYSIZE(sDirectUser)))
            m_pSpy->AddParamValueToList(L"Direct User", sDirectUser);
        else
            m_pSpy->AddParamValueToList(L"Direct User", L"<Error converting SID>");

        m_pSpy->AddParamValueToList(L"Direct User Inpersonating InProc", bCurrentUserInpersonatingInProc ? L"Yes" : L"No");

        
        IF_AUDIT_DO(OnAuthenticate)(pInfo->perfCount,
                                    GuidToBstr(pInfo->guidApp),
                                    bstrGuidActivity,    
                                    sObjId, 
                                    bstrIID, 
                                    sMethod, 
                                    sOriginalUser,                        
                                    sDirectUser,            
                                    bCurrentUserInpersonatingInProc);

        return S_OK;
    }

    STDMETHODIMP OnAuthenticateFail(  COMSVCSEVENTINFO * pInfo, 
                                  REFGUID guidActivity, 
                                  ULONG64 ObjectID,
                                  REFGUID guidIID, 
                                  ULONG iMeth,
                                  ULONG cbByteOrig,  
                                  BYTE * pSidOriginalUser,
                                  ULONG cbByteCur,   
                                  BYTE * pSidCurrentUser, 
                                  BOOL bCurrentUserInpersonatingInProc
                               )
    {

        m_pSpy->AddEventToList(pInfo->perfCount, L"OnAuthenticateFail", GuidToBstr(pInfo->guidApp));
        CComBSTR bstrGuidActivity = GuidToBstr(guidActivity);
        m_pSpy->AddParamValueToList(L"guidActivity", bstrGuidActivity);


        WCHAR sObjId[32];
        wsprintfW(sObjId,L"%#016I64X", ObjectID);
        m_pSpy->AddParamValueToList(L"ObjectID", sObjId);

        CComBSTR bstrIID = GuidToBstr(guidIID);
        m_pSpy->AddParamValueToList(L"guidIID", bstrIID);

        WCHAR sMethod[16];
        wsprintfW(sMethod,L"%#08X", iMeth);
        m_pSpy->AddParamValueToList(L"iMeth", sMethod);


        WCHAR sOriginalUser[256];
        ZeroMemory(sOriginalUser, sizeof(sOriginalUser));

        if (GetDomainSlashUser(pSidOriginalUser, sOriginalUser, ARRAYSIZE(sOriginalUser)))
            m_pSpy->AddParamValueToList(L"Original User", sOriginalUser);
        else
            m_pSpy->AddParamValueToList(L"Original User", L"<Error converting SID>");

        WCHAR sDirectUser[256];
        ZeroMemory(sDirectUser, sizeof(sDirectUser));

        if (GetDomainSlashUser(pSidCurrentUser, sDirectUser, ARRAYSIZE(sDirectUser)))
            m_pSpy->AddParamValueToList(L"Direct User", sDirectUser);
        else
            m_pSpy->AddParamValueToList(L"Direct User", L"<Error converting SID>");

        m_pSpy->AddParamValueToList(L"Direct User Inpersonating InProc", bCurrentUserInpersonatingInProc ? L"Yes" : L"No");

        IF_AUDIT_DO(OnAuthenticateFail)(pInfo->perfCount,
                                    GuidToBstr(pInfo->guidApp),
                                    bstrGuidActivity,    
                                    sObjId, // objectID
                                    bstrIID, //interface ID
                                    sMethod, //
                                    sOriginalUser,                        
                                    sDirectUser,            
                                    bCurrentUserInpersonatingInProc);

        return S_OK;
    }


    bool GetDomainSlashUser(
         __in PSID pSid,
         __out_ecount(cchDomainSlashUser) LPWSTR pwszDomainSlashUser,
         __in DWORD cchDomainSlashUser)
    {
        WCHAR wszName[USERNAMELEN + 1];
        DWORD cchName = ARRAYSIZE(wszName);
        DWORD cbReferencedDomainName = cchDomainSlashUser;
        SID_NAME_USE eUse;
        if (!LookupAccountSid(NULL,    pSid, wszName, &cchName, pwszDomainSlashUser, &cbReferencedDomainName, &eUse))
            return false;

        // concat domain\user
        wcscat_s(pwszDomainSlashUser, cchDomainSlashUser, L"\\");
        wcscat_s(pwszDomainSlashUser, cchDomainSlashUser, wszName);
        return true;
    }

};

#endif //__SECURITYSUB_H__
