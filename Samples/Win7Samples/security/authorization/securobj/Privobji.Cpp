/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1997 - 2000.  Microsoft Corporation.  All rights reserved.

   Module Name:

       PrivObSI.CPP


   Description:

       This file contains the implementation of the CPrivSecurity class

--*/

#define UNICODE
#define _UNICODE

#include <aclui.h>
#include "resource.h"
#include "Main.h"

//
// from Main.cpp
//
extern HINSTANCE g_hInstance;

//
// defined in Security.cpp
//
extern GENERIC_MAPPING ObjMap;
HANDLE GetClientToken( WORD wIndex );


#define INHERIT_FULL            (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE)

//
// DESCRIPTION OF ACCESS FLAG AFFECTS
//
// SI_ACCESS_GENERAL shows up on general properties page
// SI_ACCESS_SPECIFIC shows up on advanced page
// SI_ACCESS_CONTAINER shows on general page IF object is a container
//
// The following array defines the permission names for my objects.
//
SI_ACCESS g_siObjAccesses[] =
{
    { &GUID_NULL, ACCESS_READ,      MAKEINTRESOURCE(IDS_PRIV_READ),      SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
    { &GUID_NULL, ACCESS_MODIFY,    MAKEINTRESOURCE(IDS_PRIV_MODIFY),    SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
    { &GUID_NULL, ACCESS_DELETE,    MAKEINTRESOURCE(IDS_PRIV_DELETE),    SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
    { &GUID_NULL, ACCESS_ALL,       MAKEINTRESOURCE(IDS_PRIV_ALL),       SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
};

#define g_iObjDefAccess    1   // ACCESS_READ

// The following array defines the inheritance types for my containers.
SI_INHERIT_TYPE g_siObjInheritTypes[] =
{
    &GUID_NULL, CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,   MAKEINTRESOURCE(IDS_INH_ALL),
    &GUID_NULL, CONTAINER_INHERIT_ACE,                        MAKEINTRESOURCE(IDS_INH_CONTAINER),
    &GUID_NULL, INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE,        MAKEINTRESOURCE(IDS_INH_OBJECT),
    &GUID_NULL, 0,                                            MAKEINTRESOURCE(IDS_INH_NONE),
};


HRESULT
LocalAllocString(LPWSTR* ppResult, LPCWSTR pString)
{
	SIZE_T cch;
	SIZE_T maxlen = 1024;

    if (!ppResult || !pString)
        return E_INVALIDARG;

	cch = wcsnlen(pString, maxlen) + 1;
    *ppResult = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));

    if (!*ppResult)
        return E_OUTOFMEMORY;

	wcscpy_s(*ppResult, cch, pString);

    return S_OK;
}

void
LocalFreeString(LPWSTR* ppString)
{
    if (ppString)
    {
        if (*ppString)
            LocalFree(*ppString);
        *ppString = NULL;
    }
}

class CObjSecurity : public ISecurityInformation
{
protected:
    ULONG                   m_cRef;
    DWORD                   m_dwSIFlags;
    PSECURITY_DESCRIPTOR    *m_ppSD;
    WORD                    m_wClient;
    LPWSTR                  m_pszServerName;
    LPWSTR                  m_pszObjectName;

public:
    CObjSecurity() : m_cRef(1) {}
    virtual ~CObjSecurity();

    STDMETHOD(Initialize)(DWORD dwFlags,
                          PSECURITY_DESCRIPTOR *ppSD,
                          WORD wClient,
                          LPCWSTR pszServer,
                          BOOL fContainer);

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID, LPVOID *);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // ISecurityInformation methods
    STDMETHOD(GetObjectInformation)(PSI_OBJECT_INFO pObjectInfo);
    STDMETHOD(GetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault);
    STDMETHOD(SetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR pSD);
    STDMETHOD(GetAccessRights)(const GUID* pguidObjectType,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccess,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess);
    STDMETHOD(MapGeneric)(const GUID *pguidObjectType,
                          UCHAR *pAceFlags,
                          ACCESS_MASK *pmask);
    STDMETHOD(GetInheritTypes)(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes);
    STDMETHOD(PropertySheetPageCallback)(HWND hwnd,
                                         UINT uMsg,
                                         SI_PAGE_TYPE uPage);
};

///////////////////////////////////////////////////////////////////////////////
//
//  This is the entry point function called from our code that establishes
//  what the ACLUI interface is going to need to know.
//
//
///////////////////////////////////////////////////////////////////////////////

extern "C"
HRESULT
CreateObjSecurityInfo(
    DWORD dwFlags,           // e.g. SI_EDIT_ALL | SI_ADVANCED | SI_CONTAINER
    PSECURITY_DESCRIPTOR *ppSD,        // Pointer to security descriptor
    LPSECURITYINFO *ppObjSI,
    WORD wClient,           // Index for client token
    LPCWSTR pszServerName,  // Name of server on which SIDs will be resolved
    BOOL    fContainer)     // This is the only way to name my generic objects
{
    HRESULT hr;
    CObjSecurity *psi;

    *ppObjSI = NULL;

    psi = new CObjSecurity();
    if (!psi)
        return E_OUTOFMEMORY;

    hr = psi->Initialize(dwFlags, ppSD, wClient, pszServerName, fContainer);

    if (SUCCEEDED(hr))
        *ppObjSI = psi;
    else
        delete psi;

    return hr;
}


CObjSecurity::~CObjSecurity()
{
    LocalFreeString(&m_pszServerName);
    LocalFreeString(&m_pszObjectName);


}

STDMETHODIMP
CObjSecurity::Initialize(DWORD                      dwFlags,
                         PSECURITY_DESCRIPTOR       *ppSD,
                         WORD                       wClient,
                         LPCWSTR                    pszServer,
                         BOOL                       fContainer)
{
    HRESULT hr;

    m_dwSIFlags = dwFlags;
    if(fContainer)
        m_dwSIFlags |= SI_CONTAINER;
    m_ppSD = ppSD;
    m_wClient = wClient;

    hr = LocalAllocString(&m_pszServerName, pszServer);

    if(SUCCEEDED(hr))
    {
        if(fContainer)
            hr = LocalAllocString(&m_pszObjectName,L"Container");
        else
            hr = LocalAllocString(&m_pszObjectName,L"Object");
    }

    return hr;
}


///////////////////////////////////////////////////////////
//
// IUnknown methods
//
///////////////////////////////////////////////////////////

STDMETHODIMP_(ULONG)
CObjSecurity::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CObjSecurity::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CObjSecurity::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ISecurityInformation))
    {
        *ppv = (LPSECURITYINFO)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}


///////////////////////////////////////////////////////////
//
// ISecurityInformation methods
//
///////////////////////////////////////////////////////////

STDMETHODIMP
CObjSecurity::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{
    pObjectInfo->dwFlags = m_dwSIFlags;
    pObjectInfo->hInstance = g_hInstance;
    pObjectInfo->pszServerName = m_pszServerName;
    pObjectInfo->pszObjectName = m_pszObjectName;

    return S_OK;
}

STDMETHODIMP
CObjSecurity::GetSecurity(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault)
{
    DWORD dwLength = 0;
    DWORD dwErr = 0;

    *ppSD = NULL;

    if (fDefault)
        return E_NOTIMPL;

    //
    // Assume that required privileges have already been enabled
    //
    GetPrivateObjectSecurity(*m_ppSD, si, NULL, 0, &dwLength);
    if (dwLength)
    {
        *ppSD = LocalAlloc(LPTR, dwLength);
        if (*ppSD &&
            !GetPrivateObjectSecurity(*m_ppSD, si, *ppSD, dwLength, &dwLength))
        {
            dwErr = GetLastError();
            LocalFree(*ppSD);
            *ppSD = NULL;
        }
    }
    else
        dwErr = GetLastError();


    return HRESULT_FROM_WIN32(dwErr);
}

STDMETHODIMP
CObjSecurity::SetSecurity(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR pSD)
{
    DWORD dwErr = 0;

    //
    // Assume that required privileges have already been enabled
    //
    if (!SetPrivateObjectSecurity(si, pSD, m_ppSD, &ObjMap, GetClientToken(m_wClient)))
        dwErr = GetLastError();



    return HRESULT_FROM_WIN32(dwErr);
}

STDMETHODIMP
CObjSecurity::GetAccessRights(const GUID* /*pguidObjectType*/,
                               DWORD /*dwFlags*/,
                               PSI_ACCESS *ppAccesses,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess)
{
    *ppAccesses = g_siObjAccesses;
    *pcAccesses = sizeof(g_siObjAccesses)/sizeof(g_siObjAccesses[0]);
    *piDefaultAccess = g_iObjDefAccess;

    return S_OK;
}

STDMETHODIMP
CObjSecurity::MapGeneric(const GUID* /*pguidObjectType*/,
                          UCHAR * /*pAceFlags*/,
                          ACCESS_MASK *pmask)
{
    MapGenericMask(pmask, &ObjMap);

    return S_OK;
}

STDMETHODIMP
CObjSecurity::GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes)
{
    *ppInheritTypes = g_siObjInheritTypes;
    *pcInheritTypes = sizeof(g_siObjInheritTypes)/sizeof(g_siObjInheritTypes[0]);

    return S_OK;
}

STDMETHODIMP
CObjSecurity::PropertySheetPageCallback(HWND /*hwnd*/,
                                         UINT /*uMsg*/,
                                         SI_PAGE_TYPE /*uPage*/)
{
    return S_OK;
}
