// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/***************************************************************

This example shows how to implement a custom certificate store provider


*****************************************************************/

#define CRYPT_OID_INFO_HAS_EXTRA_FIELDS

#include <windows.h>
#include <winerror.h>
#include <strsafe.h>
#include <wincrypt.h>
#include <stdio.h>

#ifndef ASSERT
#define ASSERT  __noop
#endif

// # of bytes for a hash.
#define MAX_HASH_LEN                20

static HMODULE hMyModule = NULL;

// The name of the store provider
#define sz_CERT_STORE_PROV_TEST_EXT     "TestExt"
#define TEST_EXT_OPEN_STORE_PROV_FUNC   "I_CertDllOpenSampleStoreProvW"

//
//  External Store Provider handle information
//
typedef struct _FIND_EXT_INFO FIND_EXT_INFO, *PFIND_EXT_INFO;
struct _FIND_EXT_INFO {
    DWORD               dwContextType;
    void                *pvContext;
};

typedef struct _EXT_STORE {
    HCERTSTORE          hExtCertStore;
} EXT_STORE, *PEXT_STORE;

//
//  External Store Provider Functions.
// 
static 
void WINAPI 
ExtStoreProvClose(
    HCERTSTOREPROV  hStoreProv,
    DWORD           dwFlags
    );

static 
BOOL WINAPI 
ExtStoreProvReadCert(
    HCERTSTOREPROV  hStoreProv,
    PCCERT_CONTEXT  pStoreCertContext,
    DWORD           dwFlags,
    PCCERT_CONTEXT *ppProvCertContext
    );

static 
BOOL WINAPI 
ExtStoreProvWriteCert(
    HCERTSTOREPROV  hStoreProv,
    PCCERT_CONTEXT  pCertContext,
                    DWORD           dwFlags
    );

static 
BOOL WINAPI 
ExtStoreProvDeleteCert(
    HCERTSTOREPROV  hStoreProv,
    PCCERT_CONTEXT  pCertContext,
    DWORD           dwFlags
    );

static 
BOOL WINAPI 
ExtStoreProvSetCertProperty(
    HCERTSTOREPROV  hStoreProv,
    PCCERT_CONTEXT  pCertContext,
    DWORD           dwPropId,
    DWORD           dwFlags,
    const void      *pvData
    );

static 
BOOL WINAPI 
ExtStoreProvReadCrl(
    HCERTSTOREPROV  hStoreProv,
    PCCRL_CONTEXT   pStoreCrlContext,
    DWORD           dwFlags,
    PCCRL_CONTEXT   *ppProvCrlContext
    );

static 
BOOL WINAPI 
ExtStoreProvWriteCrl(
    HCERTSTOREPROV  hStoreProv,
    PCCRL_CONTEXT   pCrlContext,
    DWORD           dwFlags
    );

static 
BOOL WINAPI 
ExtStoreProvDeleteCrl(
    HCERTSTOREPROV  hStoreProv,
    PCCRL_CONTEXT   pCrlContext,
    DWORD           dwFlags
    );

static 
BOOL WINAPI 
ExtStoreProvSetCrlProperty(
    HCERTSTOREPROV  hStoreProv,
    PCCRL_CONTEXT   pCrlContext,
    DWORD           dwPropId,
    DWORD           dwFlags,
    const void      *pvData
    );

static 
BOOL WINAPI 
ExtStoreProvReadCtl(
    HCERTSTOREPROV hStoreProv,
    PCCTL_CONTEXT   pStoreCtlContext,
    DWORD           dwFlags,
    PCCTL_CONTEXT   *ppProvCtlContext
    );

static 
BOOL WINAPI 
ExtStoreProvWriteCtl(
    HCERTSTOREPROV hStoreProv,
    PCCTL_CONTEXT pCtlContext,
    DWORD dwFlags
    );

static BOOL WINAPI ExtStoreProvDeleteCtl(
    HCERTSTOREPROV hStoreProv,
    PCCTL_CONTEXT pCtlContext,
    DWORD dwFlags
        );

static BOOL WINAPI ExtStoreProvSetCtlProperty(
    HCERTSTOREPROV hStoreProv,
    PCCTL_CONTEXT pCtlContext,
    DWORD dwPropId,
    DWORD dwFlags,
    const void *pvData
        );

static BOOL WINAPI ExtStoreProvControl(
    HCERTSTOREPROV hStoreProv,
    DWORD dwFlags,
    DWORD dwCtrlType,
    void const *pvCtrlPara
        );

static BOOL WINAPI ExtStoreProvFindCert(
    HCERTSTOREPROV hStoreProv,
    PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    PCCERT_CONTEXT pPrevCertContext,
    DWORD dwFlags,
    void **ppvStoreProvFindInfo,
    PCCERT_CONTEXT *ppProvCertContext
    );

static BOOL WINAPI ExtStoreProvFreeFindCert(
    HCERTSTOREPROV hStoreProv,
    PCCERT_CONTEXT pCertContext,
    void *pvStoreProvFindInfo,
    DWORD dwFlags
        );

static BOOL WINAPI ExtStoreProvGetCertProperty(
    HCERTSTOREPROV hStoreProv,
    PCCERT_CONTEXT pCertContext,
    DWORD dwPropId,
    DWORD dwFlags,
    void *pvData,
    DWORD *pcbData
    );

static BOOL WINAPI ExtStoreProvFindCrl(
    HCERTSTOREPROV hStoreProv,
    PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    PCCRL_CONTEXT pPrevCrlContext,
    DWORD dwFlags,
    void **ppvStoreProvFindInfo,
    PCCRL_CONTEXT *ppProvCrlContext
   );

static BOOL WINAPI ExtStoreProvFreeFindCrl(
    HCERTSTOREPROV hStoreProv,
    PCCRL_CONTEXT pCrlContext,
    void *pvStoreProvFindInfo,
    DWORD dwFlags
        );

static BOOL WINAPI ExtStoreProvGetCrlProperty(
    HCERTSTOREPROV hStoreProv,
    PCCRL_CONTEXT pCrlContext,
    DWORD dwPropId,
    DWORD dwFlags,
    void *pvData,
    DWORD *pcbData
    );

static BOOL WINAPI ExtStoreProvFindCtl(
    HCERTSTOREPROV hStoreProv,
    PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    PCCTL_CONTEXT pPrevCtlContext,
    DWORD dwFlags,
    void **ppvStoreProvFindInfo,
    PCCTL_CONTEXT *ppProvCtlContext
    );

static BOOL WINAPI ExtStoreProvFreeFindCtl(
    HCERTSTOREPROV hStoreProv,
    PCCTL_CONTEXT pCtlContext,
    void *pvStoreProvFindInfo,
    DWORD dwFlags
    );

static BOOL WINAPI ExtStoreProvGetCtlProperty(
    HCERTSTOREPROV hStoreProv,
    PCCTL_CONTEXT pCtlContext,
    DWORD dwPropId,
    DWORD dwFlags,
    void *pvData,
    DWORD *pcbData
    );

static void * const rgpvExtStoreProvFunc[] = {
    // CERT_STORE_PROV_CLOSE_FUNC              0
    ExtStoreProvClose,
    // CERT_STORE_PROV_READ_CERT_FUNC          1
    ExtStoreProvReadCert,
    // CERT_STORE_PROV_WRITE_CERT_FUNC         2
    ExtStoreProvWriteCert,
    // CERT_STORE_PROV_DELETE_CERT_FUNC        3
    ExtStoreProvDeleteCert,
    // CERT_STORE_PROV_SET_CERT_PROPERTY_FUNC  4
    ExtStoreProvSetCertProperty,
    // CERT_STORE_PROV_READ_CRL_FUNC           5
    ExtStoreProvReadCrl,
    // CERT_STORE_PROV_WRITE_CRL_FUNC          6
    ExtStoreProvWriteCrl,
    // CERT_STORE_PROV_DELETE_CRL_FUNC         7
    ExtStoreProvDeleteCrl,
    // CERT_STORE_PROV_SET_CRL_PROPERTY_FUNC   8
    ExtStoreProvSetCrlProperty,
    // CERT_STORE_PROV_READ_CTL_FUNC           9
    ExtStoreProvReadCtl,
    // CERT_STORE_PROV_WRITE_CTL_FUNC          10
    ExtStoreProvWriteCtl,
    // CERT_STORE_PROV_DELETE_CTL_FUNC         11
    ExtStoreProvDeleteCtl,
    // CERT_STORE_PROV_SET_CTL_PROPERTY_FUNC   12
    ExtStoreProvSetCtlProperty,
    // CERT_STORE_PROV_CONTROL_FUNC            13
    ExtStoreProvControl,
    // CERT_STORE_PROV_FIND_CERT_FUNC          14
    ExtStoreProvFindCert,
    // CERT_STORE_PROV_FREE_FIND_CERT_FUNC     15
    ExtStoreProvFreeFindCert,
    // CERT_STORE_PROV_GET_CERT_PROPERTY_FUNC  16
    ExtStoreProvGetCertProperty,
    // CERT_STORE_PROV_FIND_CRL_FUNC           17
    ExtStoreProvFindCrl,
    // CERT_STORE_PROV_FREE_FIND_CRL_FUNC      18
    ExtStoreProvFreeFindCrl,
    // CERT_STORE_PROV_GET_CRL_PROPERTY_FUNC   19
    ExtStoreProvGetCrlProperty,
    // CERT_STORE_PROV_FIND_CTL_FUNC           20
    ExtStoreProvFindCtl,
    // CERT_STORE_PROV_FREE_FIND_CTL_FUNC      21
    ExtStoreProvFreeFindCtl,
    // CERT_STORE_PROV_GET_CTL_PROPERTY_FUNC   22
    ExtStoreProvGetCtlProperty
};

#define EXT_STORE_PROV_FUNC_COUNT (sizeof(rgpvExtStoreProvFunc)/sizeof(rgpvExtStoreProvFunc[0]))

//
//  CertStore allocation and free functions
//
static void *CSAlloc(
    size_t cbBytes
    )
{
    void *pv;
    pv = malloc(cbBytes);
    if (pv == NULL)
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}

static void CSFree(
    void *pv
    )
{
    if (pv)
        free(pv);
}

//
//  Create, add, remove and free external store find info functions
//
static PFIND_EXT_INFO CreateExtInfo(
    DWORD dwContextType,
    void *pvContext
    )
{
    PFIND_EXT_INFO pFindExtInfo;

    if ((pFindExtInfo = (PFIND_EXT_INFO) CSAlloc(sizeof(FIND_EXT_INFO))) != 0)
    {
        pFindExtInfo->dwContextType = dwContextType;
        pFindExtInfo->pvContext = pvContext;
    }
    return pFindExtInfo;
}

static void FreeExtInfo(
    PFIND_EXT_INFO pFindExtInfo
    )
{
    void *pvContext;

    if (NULL == pFindExtInfo)
        return;

    pvContext = pFindExtInfo->pvContext;
    if (pvContext) {
        switch (pFindExtInfo->dwContextType) {
            case (CERT_STORE_CERTIFICATE_CONTEXT - 1):
                CertFreeCertificateContext((PCCERT_CONTEXT) pvContext);
                break;
            case (CERT_STORE_CRL_CONTEXT - 1):
                CertFreeCRLContext((PCCRL_CONTEXT) pvContext);
                break;
            case (CERT_STORE_CTL_CONTEXT - 1):
                CertFreeCTLContext((PCCTL_CONTEXT) pvContext);
                break;
            default:
                ASSERT(pFindExtInfo->dwContextType < CERT_STORE_CTL_CONTEXT);
        }
    }

    CSFree(pFindExtInfo);
}

//
//  Dll initialization
//
BOOL
WINAPI
DllMain(
        HMODULE hModule,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        hMyModule = hModule;
        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
        break;
    default:
        break;
    }

    return TRUE;

    UNREFERENCED_PARAMETER(lpReserved);
}

STDAPI  DllCanUnloadNow(void)
{
    // Allow unloading.
    return S_OK;
}

static HRESULT HError()
{
    DWORD dw = GetLastError();

    HRESULT hr;
    if ( dw <= 0xFFFF )
        hr = HRESULT_FROM_WIN32 ( dw );
    else
        hr = dw;

    if ( ! FAILED ( hr ) )
    {        
        hr = E_UNEXPECTED;
    }
    return hr;
}

static HRESULT GetDllFilename(
    WCHAR wszModule[_MAX_PATH]
    )
{
    char szModule[_MAX_PATH];
    LPSTR pszModule;
    int cchModule;

    // Get name of this DLL.
    if (0 == GetModuleFileNameA(hMyModule, szModule, _MAX_PATH))
        return HError();

    // Strip off the Dll filename's directory components
    cchModule = (int)strlen(szModule);
    pszModule = szModule + cchModule;
    while (cchModule-- > 0) {
        pszModule--;
        if ('\\' == *pszModule || ':' == *pszModule) {
            pszModule++;
            break;
        }
    }
    if (0 >= MultiByteToWideChar(
            CP_ACP,
            0,                      // dwFlags
            pszModule,
            -1,                     // null terminated
            wszModule,
            _MAX_PATH))
        return HError();

    return S_OK;
}

//
//  DllRegisterServer
//
STDAPI DllRegisterServer(void)
{
    HRESULT hr;
    WCHAR wszModule[_MAX_PATH];

    if (FAILED(hr = GetDllFilename(wszModule)))
        return hr;

    // Register the store provider
    if (!CryptRegisterOIDFunction(
            0,								//dwEncodingType
            CRYPT_OID_OPEN_STORE_PROV_FUNC,
            sz_CERT_STORE_PROV_TEST_EXT,
            wszModule,
            TEST_EXT_OPEN_STORE_PROV_FUNC
            )) {
        if (ERROR_FILE_EXISTS != GetLastError())
            return HError();
    }

    return S_OK;
}

//
//  DllUnregisterServer
//
STDAPI DllUnregisterServer(void)
{
    HRESULT hr;
    WCHAR wszModule[_MAX_PATH];

    if (FAILED(hr = GetDllFilename(wszModule)))
        return hr;
    if (!CryptUnregisterOIDFunction(
            0,								//dwEncodingType
            CRYPT_OID_OPEN_STORE_PROV_FUNC,
            sz_CERT_STORE_PROV_TEST_EXT
            )) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            return HError();
    }

    return S_OK;
}


//
//  Implement the "test" external store by opening the corresponding system
//  registry store.
//
BOOL
WINAPI
I_CertDllOpenSampleStoreProvW(
    LPCSTR lpszStoreProvider,
    DWORD dwEncodingType,
    HCRYPTPROV hCryptProv,
    DWORD dwFlags,
    const void *pvPara,
    HCERTSTORE hCertStore,
    PCERT_STORE_PROV_INFO pStoreProvInfo
    )
{
    BOOL fResult;
    PEXT_STORE pExtStore = NULL;

    if (0 == (dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK))
        dwFlags |= CERT_SYSTEM_STORE_CURRENT_USER;
    dwFlags |= CERT_STORE_NO_CRYPT_RELEASE_FLAG;

    if (dwFlags & CERT_STORE_DELETE_FLAG) {
        CertOpenStore(
            CERT_STORE_PROV_SYSTEM_W,
            dwEncodingType,
            hCryptProv,
            dwFlags,
            pvPara
            );
        pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_DELETED_FLAG;
        if (0 == GetLastError())
            return TRUE;
        else
            return FALSE;
    }

    if (NULL == (pExtStore = (PEXT_STORE) CSAlloc(sizeof(EXT_STORE))))
    {
        goto ErrorReturn;
    }
    memset(pExtStore, 0, sizeof(EXT_STORE));

    if (NULL == (pExtStore->hExtCertStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_W,
            dwEncodingType,
            hCryptProv,
            dwFlags,
            pvPara
            )))
    {
        goto ErrorReturn;
    }

    pStoreProvInfo->cStoreProvFunc = EXT_STORE_PROV_FUNC_COUNT;
    pStoreProvInfo->rgpvStoreProvFunc = (void **) rgpvExtStoreProvFunc;
    pStoreProvInfo->hStoreProv = (HCERTSTOREPROV) pExtStore;
    pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_EXTERNAL_FLAG;
    fResult = TRUE;

CommonReturn:
    return fResult;

ErrorReturn:
    ExtStoreProvClose((HCERTSTOREPROV) pExtStore, 0);
    fResult = FALSE;
    goto CommonReturn;

    UNREFERENCED_PARAMETER(hCertStore);
    UNREFERENCED_PARAMETER(lpszStoreProvider);
}


//
//  Close the registry's store by closing its opened registry subkeys
//
static void WINAPI ExtStoreProvClose(
    HCERTSTOREPROV hStoreProv,
    DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    if (pExtStore) {
        if (pExtStore->hExtCertStore)
            CertCloseStore(pExtStore->hExtCertStore, 0);
        CSFree(pExtStore);
    }

    UNREFERENCED_PARAMETER(dwFlags);
}

//
//  Find certificate in system store corresponding to pCertContext
//
static PCCERT_CONTEXT FindCorrespondingCertificate (
    HCERTSTORE hExtCertStore,
    PCCERT_CONTEXT pCertContext
    )
{
    DWORD           cbHash = MAX_HASH_LEN;
    BYTE            aHash[MAX_HASH_LEN];
    CRYPT_HASH_BLOB HashBlob;

    if ( CertGetCertificateContextProperty(
             pCertContext,
             CERT_HASH_PROP_ID,
             aHash,
             &cbHash
             ) == FALSE )
    {
        return( NULL );
    }

    HashBlob.cbData = cbHash;
    HashBlob.pbData = aHash;

    return( CertFindCertificateInStore(
                hExtCertStore,
                X509_ASN_ENCODING,
                0,
                CERT_FIND_HASH,
                &HashBlob,
                NULL
                ) );
}

//
//  Find CRL in system store corresponding to pCrlContext
//
static PCCRL_CONTEXT FindCorrespondingCrl (
    HCERTSTORE hExtCertStore,
    PCCRL_CONTEXT pCrlContext
    )
{
    DWORD         cbHash = MAX_HASH_LEN;
    BYTE          aHash[MAX_HASH_LEN];
    DWORD         cbFindHash = MAX_HASH_LEN;
    BYTE          aFindHash[MAX_HASH_LEN];
    PCCRL_CONTEXT pFindCrl = NULL;
    DWORD         dwFlags = 0;

    if ( CertGetCRLContextProperty(
             pCrlContext,
             CERT_HASH_PROP_ID,
             aHash,
             &cbHash
             ) == FALSE )
    {
        return( NULL );
    }

    while ( ( pFindCrl = CertGetCRLFromStore(
                             hExtCertStore,
                             NULL,
                             pFindCrl,
                             &dwFlags
                             ) ) != NULL )
    {
        if ( CertGetCRLContextProperty(
                 pFindCrl,
                 CERT_HASH_PROP_ID,
                 aFindHash,
                 &cbFindHash
                 ) == TRUE )
        {
            if ( cbHash == cbFindHash )
            {
                if ( memcmp( aHash, aFindHash, cbHash ) == 0 )
                {
                    return( pFindCrl );
                }
            }
        }
    }

    return( NULL );
}

//
//  Find CTL in system store corresponding to pCtlContext
//
static PCCTL_CONTEXT FindCorrespondingCtl (
    HCERTSTORE hExtCertStore,
    PCCTL_CONTEXT pCtlContext
    )
{
    DWORD           cbHash = MAX_HASH_LEN;
    BYTE            aHash[MAX_HASH_LEN];
    CRYPT_HASH_BLOB HashBlob;

    if ( CertGetCTLContextProperty(
             pCtlContext,
             CERT_SHA1_HASH_PROP_ID,
             aHash,
             &cbHash
             ) == FALSE )
    {
        return( NULL );
    }

    HashBlob.cbData = cbHash;
    HashBlob.pbData = aHash;

    return( CertFindCTLInStore(
                hExtCertStore,
                X509_ASN_ENCODING,
                0,
                CTL_FIND_SHA1_HASH,
                &HashBlob,
                NULL
                ) );
}

//
//  Read the serialized copy of the certificate and its properties from
//  the registry and create a new certificate context.
//
static BOOL WINAPI ExtStoreProvReadCert(
    HCERTSTOREPROV hStoreProv,
    PCCERT_CONTEXT pStoreCertContext,
    DWORD dwFlags,
    PCCERT_CONTEXT *ppProvCertContext
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCERT_CONTEXT pProvCertContext;

    ASSERT(pExtStore && pExtStore->hExtCertStore);
    pProvCertContext = FindCorrespondingCertificate(
        pExtStore->hExtCertStore, pStoreCertContext);

    *ppProvCertContext = pProvCertContext;
    return NULL != pProvCertContext;

    UNREFERENCED_PARAMETER(dwFlags);
}

//
//  Serialize the encoded certificate and its properties and write to
//  the registry.
//
//  Called before the certificate is written to the store.
//
static BOOL WINAPI ExtStoreProvWriteCert(
    HCERTSTOREPROV hStoreProv,
    PCCERT_CONTEXT pCertContext,
    DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    DWORD dwAddDisposition;

    ASSERT(pExtStore && pExtStore->hExtCertStore);
    if (dwFlags & CERT_STORE_PROV_WRITE_ADD_FLAG)
        dwAddDisposition = (dwFlags >> 16) & 0xFFFF;
    else
        dwAddDisposition = 0;

    return CertAddCertificateContextToStore(
        pExtStore->hExtCertStore,
        pCertContext,
        dwAddDisposition,
        NULL                // ppStoreContext
        );
}


//
//  Delete the specified certificate from the registry.
//
//  Called before the certificate is deleted from the store.
//
static BOOL WINAPI ExtStoreProvDeleteCert(
    HCERTSTOREPROV hStoreProv,
    PCCERT_CONTEXT pCertContext,
    DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCERT_CONTEXT pExtContext;

    ASSERT(pExtStore && pExtStore->hExtCertStore);
    if ((pExtContext = FindCorrespondingCertificate(
            pExtStore->hExtCertStore, pCertContext)) != 0)
        return CertDeleteCertificateFromStore(pExtContext);
    else
        return FALSE;

    UNREFERENCED_PARAMETER(dwFlags);
}

//
//  Read the specified certificate from the registry and update its
//  property.
//
//  Note, ignore the CERT_SHA1_HASH_PROP_ID property which is implicitly
//  set before we write the certificate to the registry. If we don't ignore,
//  we will have indefinite recursion.
//
//  Called before setting the property of the certificate in the store.
//
static BOOL WINAPI ExtStoreProvSetCertProperty(
    HCERTSTOREPROV hStoreProv,
    PCCERT_CONTEXT pCertContext,
    DWORD dwPropId,
    DWORD dwFlags,
    const void *pvData
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCERT_CONTEXT pExtContext;

    ASSERT(pExtStore && pExtStore->hExtCertStore);
    if ((pExtContext = FindCorrespondingCertificate(
            pExtStore->hExtCertStore, pCertContext)) != 0) {
        BOOL fResult;

        fResult = CertSetCertificateContextProperty(
            pExtContext,
            dwPropId,
            dwFlags,
            pvData
            );
        CertFreeCertificateContext(pExtContext);
        return fResult;
    } else
        return FALSE;
}

//
//  Read the serialized copy of the CRL and its properties from
//  the registry and create a new CRL context.
//
static BOOL WINAPI ExtStoreProvReadCrl(
    HCERTSTOREPROV hStoreProv,
    PCCRL_CONTEXT pStoreCrlContext,
    DWORD dwFlags,
    PCCRL_CONTEXT *ppProvCrlContext
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCRL_CONTEXT pProvCrlContext;

    ASSERT(pExtStore && pExtStore->hExtCertStore);
    pProvCrlContext = FindCorrespondingCrl(
        pExtStore->hExtCertStore, pStoreCrlContext);

    *ppProvCrlContext = pProvCrlContext;
    return NULL != pProvCrlContext;

    UNREFERENCED_PARAMETER(dwFlags);
}

//
//  Serialize the encoded CRL and its properties and write to
//  the registry.
//
//  Called before the CRL is written to the store.
//
static BOOL WINAPI ExtStoreProvWriteCrl(
    HCERTSTOREPROV hStoreProv,
    PCCRL_CONTEXT pCrlContext,
    DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    DWORD dwAddDisposition;

    ASSERT(pExtStore && pExtStore->hExtCertStore);
    if (dwFlags & CERT_STORE_PROV_WRITE_ADD_FLAG)
        dwAddDisposition = (dwFlags >> 16) & 0xFFFF;
    else
        dwAddDisposition = 0;

    return CertAddCRLContextToStore(
        pExtStore->hExtCertStore,
        pCrlContext,
        dwAddDisposition,
        NULL                // ppStoreContext
        );
}


//
//  Delete the specified CRL from the registry.
//
//  Called before the CRL is deleted from the store.
//
static BOOL WINAPI ExtStoreProvDeleteCrl(
    HCERTSTOREPROV hStoreProv,
    PCCRL_CONTEXT pCrlContext,
    DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCRL_CONTEXT pExtContext;

    ASSERT(pExtStore && pExtStore->hExtCertStore);
    if ((pExtContext = FindCorrespondingCrl(
            pExtStore->hExtCertStore, pCrlContext)) != 0)
        return CertDeleteCRLFromStore(pExtContext);
    else
        return FALSE;

    UNREFERENCED_PARAMETER(dwFlags);
}

//
//  Read the specified CRL from the registry and update its
//  property.
//
//  Note, ignore the CERT_SHA1_HASH_PROP_ID property which is implicitly
//  set before we write the CRL to the registry. If we don't ignore,
//  we will have indefinite recursion.
//
//  Called before setting the property of the CRL in the store.
//
static BOOL WINAPI ExtStoreProvSetCrlProperty(
    HCERTSTOREPROV hStoreProv,
    PCCRL_CONTEXT pCrlContext,
    DWORD dwPropId,
    DWORD dwFlags,
    const void *pvData
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCRL_CONTEXT pExtContext;

    ASSERT(pExtStore && pExtStore->hExtCertStore);
    if ((pExtContext = FindCorrespondingCrl(
            pExtStore->hExtCertStore, pCrlContext)) != 0)
    {
        BOOL fResult;

        fResult = CertSetCRLContextProperty(
            pExtContext,
            dwPropId,
            dwFlags,
            pvData
            );
        CertFreeCRLContext(pExtContext);
        return fResult;
    } else
        return FALSE;
}

//
//  Read the serialized copy of the CTL and its properties from
//  the registry and create a new CTL context.
//
static BOOL WINAPI ExtStoreProvReadCtl(
    HCERTSTOREPROV hStoreProv,
    PCCTL_CONTEXT pStoreCtlContext,
    DWORD dwFlags,
    PCCTL_CONTEXT *ppProvCtlContext
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCTL_CONTEXT pProvCtlContext;

    ASSERT(pExtStore && pExtStore->hExtCertStore);
    pProvCtlContext = FindCorrespondingCtl(
        pExtStore->hExtCertStore, pStoreCtlContext);

    *ppProvCtlContext = pProvCtlContext;
    return NULL != pProvCtlContext;

    UNREFERENCED_PARAMETER(dwFlags);
}

//
//  Serialize the encoded CTL and its properties and write to
//  the registry.
//
//  Called before the CTL is written to the store.
//
static BOOL WINAPI ExtStoreProvWriteCtl(
    HCERTSTOREPROV hStoreProv,
    PCCTL_CONTEXT pCtlContext,
    DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    DWORD dwAddDisposition;

    ASSERT(pExtStore && pExtStore->hExtCertStore);
    if (dwFlags & CERT_STORE_PROV_WRITE_ADD_FLAG)
        dwAddDisposition = (dwFlags >> 16) & 0xFFFF;
    else
        dwAddDisposition = 0;

    return CertAddCTLContextToStore(
        pExtStore->hExtCertStore,
        pCtlContext,
        dwAddDisposition,
        NULL                // ppStoreContext
        );
}


//
//  Delete the specified CTL from the registry.
//
//  Called before the CTL is deleted from the store.
//
static BOOL WINAPI ExtStoreProvDeleteCtl(
    HCERTSTOREPROV hStoreProv,
    PCCTL_CONTEXT pCtlContext,
    DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCTL_CONTEXT pExtContext;

    ASSERT(pExtStore && pExtStore->hExtCertStore);
    if ((pExtContext = FindCorrespondingCtl(
            pExtStore->hExtCertStore, pCtlContext)) != 0)
        return CertDeleteCTLFromStore(pExtContext);
    else
        return FALSE;

    UNREFERENCED_PARAMETER(dwFlags);
}

//
//  Read the specified CTL from the registry and update its
//  property.
//
//  Note, ignore the CERT_SHA1_HASH_PROP_ID property which is implicitly
//  set before we write the CTL to the registry. If we don't ignore,
//  we will have indefinite recursion.
//
//  Called before setting the property of the CTL in the store.
//
static BOOL WINAPI ExtStoreProvSetCtlProperty(
    HCERTSTOREPROV hStoreProv,
    PCCTL_CONTEXT pCtlContext,
    DWORD dwPropId,
    DWORD dwFlags,
    const void *pvData
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCTL_CONTEXT pExtContext;

    ASSERT(pExtStore && pExtStore->hExtCertStore);
    if ((pExtContext = FindCorrespondingCtl(
            pExtStore->hExtCertStore, pCtlContext)) != 0)
    {
        BOOL fResult;

        fResult = CertSetCTLContextProperty(
            pExtContext,
            dwPropId,
            dwFlags,
            pvData
            );
        CertFreeCTLContext(pExtContext);
        return fResult;
    } else
        return FALSE;
}


static BOOL WINAPI ExtStoreProvControl(
    HCERTSTOREPROV hStoreProv,
    DWORD dwFlags,
    DWORD dwCtrlType,
    void const *pvCtrlPara
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    ASSERT(pExtStore && pExtStore->hExtCertStore);
    return CertControlStore(
        pExtStore->hExtCertStore,
        dwFlags,
        dwCtrlType,
        pvCtrlPara
        );
}

static BOOL WINAPI ExtStoreProvFindCert(
    HCERTSTOREPROV hStoreProv,
    PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    PCCERT_CONTEXT pPrevCertContext,
    DWORD dwFlags,
    void **ppvStoreProvFindInfo,
    PCCERT_CONTEXT *ppProvCertContext
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PFIND_EXT_INFO pFindExtInfo = (PFIND_EXT_INFO) *ppvStoreProvFindInfo;
    PCCERT_CONTEXT pPrevExtContext;
    PCCERT_CONTEXT pProvCertContext;

    if (pFindExtInfo) {
        ASSERT((CERT_STORE_CERTIFICATE_CONTEXT - 1) ==
            pFindExtInfo->dwContextType);
        pPrevExtContext = (PCCERT_CONTEXT) pFindExtInfo->pvContext;
        pFindExtInfo->pvContext = NULL;
    } else
        pPrevExtContext = NULL;

    ASSERT(pExtStore);
    ASSERT(pPrevCertContext == pPrevExtContext);

    if ((pProvCertContext = CertFindCertificateInStore(
            pExtStore->hExtCertStore,
            pFindInfo->dwMsgAndCertEncodingType,
            pFindInfo->dwFindFlags,
            pFindInfo->dwFindType,
            pFindInfo->pvFindPara,
            pPrevExtContext
            )) != 0)
    {
        if (pFindExtInfo)
            // Re-use existing Find Info
            pFindExtInfo->pvContext = (void *) CertDuplicateCertificateContext(
                pProvCertContext);
        else
        {
            if ((pFindExtInfo = CreateExtInfo(
                    CERT_STORE_CERTIFICATE_CONTEXT - 1,
                    (void *) pProvCertContext
                    )) != 0)
                pProvCertContext = CertDuplicateCertificateContext(
                    pProvCertContext);
            else
            {
                CertFreeCertificateContext(pProvCertContext);
                pProvCertContext = NULL;
            }
        }
    }
    else if (pFindExtInfo)
    {
        ExtStoreProvFreeFindCert(
            hStoreProv,
            pPrevCertContext,
            pFindExtInfo,
            0                       // dwFlags
            );
        pFindExtInfo = NULL;
    }

    *ppProvCertContext = pProvCertContext;
    *ppvStoreProvFindInfo = pFindExtInfo;
    return NULL != pProvCertContext;

    UNREFERENCED_PARAMETER(dwFlags);
}

static BOOL WINAPI ExtStoreProvFreeFindCert(
    HCERTSTOREPROV hStoreProv,
    PCCERT_CONTEXT pCertContext,
    void *pvStoreProvFindInfo,
    DWORD dwFlags
        )
{
    PFIND_EXT_INFO pFindExtInfo = (PFIND_EXT_INFO) pvStoreProvFindInfo;

    ASSERT(pFindExtInfo);
    if (pFindExtInfo) {
        ASSERT((CERT_STORE_CERTIFICATE_CONTEXT - 1) ==
            pFindExtInfo->dwContextType);
        FreeExtInfo(pFindExtInfo);
    }
    return TRUE;

    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(pCertContext);
    UNREFERENCED_PARAMETER(hStoreProv);
}

static BOOL WINAPI ExtStoreProvGetCertProperty(
    HCERTSTOREPROV hStoreProv,
    PCCERT_CONTEXT pCertContext,
    DWORD dwPropId,
    DWORD dwFlags,
    void *pvData,
    DWORD *pcbData
        )
{
    *pcbData = 0;
    SetLastError((DWORD) CRYPT_E_NOT_FOUND);
    return FALSE;

    UNREFERENCED_PARAMETER(pcbData);
    UNREFERENCED_PARAMETER(pvData);
    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(dwPropId);
    UNREFERENCED_PARAMETER(pCertContext);
    UNREFERENCED_PARAMETER(hStoreProv);
}

static PCCRL_CONTEXT WINAPI FindCrlInStore(
    HCERTSTORE hCertStore,
    DWORD dwCertEncodingType,
    DWORD dwFindFlags,
    DWORD dwFindType,
    const void *pvFindPara,
    PCCRL_CONTEXT pPrevCrlContext
    )
{
    DWORD dwFlags = 0;

    switch (dwFindType) {
        case CRL_FIND_ANY:
            return CertGetCRLFromStore(
                hCertStore,
                NULL,               // pIssuerContext,
                pPrevCrlContext,
                &dwFlags
                );
            break;

        case CRL_FIND_ISSUED_BY:
            {
                PCCERT_CONTEXT pIssuer = (PCCERT_CONTEXT) pvFindPara;

                return CertGetCRLFromStore(
                    hCertStore,
                    pIssuer,
                    pPrevCrlContext,
                    &dwFlags
                    );
            }
            break;

        case CRL_FIND_EXISTING:
            {
                PCCRL_CONTEXT pCrl = pPrevCrlContext;

                while ((pCrl = CertGetCRLFromStore(
                        hCertStore,
                        NULL,               // pIssuerContext,
                        pCrl,
                        &dwFlags)) != 0)
                {
                    PCCRL_CONTEXT pNew = (PCCRL_CONTEXT) pvFindPara;
                    if (pNew->dwCertEncodingType == pCrl->dwCertEncodingType &&
                            CertCompareCertificateName(
                                pNew->dwCertEncodingType,
                                &pCrl->pCrlInfo->Issuer,
                                &pNew->pCrlInfo->Issuer))
                        return pCrl;
                }
                return NULL;
            }
            break;

        default:
            SetLastError((DWORD) ERROR_NOT_SUPPORTED);
            return NULL;
    }

    UNREFERENCED_PARAMETER(dwFindFlags);
    UNREFERENCED_PARAMETER(dwCertEncodingType);
}

static BOOL WINAPI ExtStoreProvFindCrl(
    HCERTSTOREPROV hStoreProv,
    PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    PCCRL_CONTEXT pPrevCrlContext,
    DWORD dwFlags,
    void **ppvStoreProvFindInfo,
    PCCRL_CONTEXT *ppProvCrlContext
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PFIND_EXT_INFO pFindExtInfo = (PFIND_EXT_INFO) *ppvStoreProvFindInfo;
    PCCRL_CONTEXT pPrevExtContext;
    PCCRL_CONTEXT pProvCrlContext;

    if (pFindExtInfo) {
        ASSERT((CERT_STORE_CRL_CONTEXT - 1) ==
            pFindExtInfo->dwContextType);
        pPrevExtContext = (PCCRL_CONTEXT) pFindExtInfo->pvContext;
        pFindExtInfo->pvContext = NULL;
    } else
        pPrevExtContext = NULL;

    ASSERT(pExtStore);
    ASSERT(pPrevCrlContext == pPrevExtContext);

    if ((pProvCrlContext = FindCrlInStore(
            pExtStore->hExtCertStore,
            pFindInfo->dwMsgAndCertEncodingType,
            pFindInfo->dwFindFlags,
            pFindInfo->dwFindType,
            pFindInfo->pvFindPara,
            pPrevExtContext
            )) != 0)
    {
        if (pFindExtInfo)
            // Re-use existing Find Info
            pFindExtInfo->pvContext = (void *) CertDuplicateCRLContext(
                pProvCrlContext);
        else
        {
            if ((pFindExtInfo = CreateExtInfo(
                    CERT_STORE_CRL_CONTEXT - 1,
                    (void *) pProvCrlContext
                    )) != 0)
                pProvCrlContext = CertDuplicateCRLContext(
                    pProvCrlContext);
            else
            {
                CertFreeCRLContext(pProvCrlContext);
                pProvCrlContext = NULL;
            }
        }
    }
    else if (pFindExtInfo)
    {
        ExtStoreProvFreeFindCrl(
            hStoreProv,
            pPrevCrlContext,
            pFindExtInfo,
            0                       // dwFlags
            );
        pFindExtInfo = NULL;
    }

    *ppProvCrlContext = pProvCrlContext;
    *ppvStoreProvFindInfo = pFindExtInfo;
    return NULL != pProvCrlContext;

    UNREFERENCED_PARAMETER(dwFlags);
}

static BOOL WINAPI ExtStoreProvFreeFindCrl(
    HCERTSTOREPROV hStoreProv,
    PCCRL_CONTEXT pCrlContext,
    void *pvStoreProvFindInfo,
    DWORD dwFlags
        )
{
    PFIND_EXT_INFO pFindExtInfo = (PFIND_EXT_INFO) pvStoreProvFindInfo;

    ASSERT(pFindExtInfo);

    if (pFindExtInfo)
    {
        ASSERT((CERT_STORE_CRL_CONTEXT - 1) ==
            pFindExtInfo->dwContextType);
        FreeExtInfo(pFindExtInfo);
    }
    return TRUE;

    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(pCrlContext);
    UNREFERENCED_PARAMETER(hStoreProv);
}

static BOOL WINAPI ExtStoreProvGetCrlProperty(
    HCERTSTOREPROV hStoreProv,
    PCCRL_CONTEXT pCrlContext,
    DWORD dwPropId,
    DWORD dwFlags,
    void *pvData,
    DWORD *pcbData
        )
{
    *pcbData = 0;
    SetLastError((DWORD) CRYPT_E_NOT_FOUND);
    return FALSE;

    UNREFERENCED_PARAMETER(pcbData);
    UNREFERENCED_PARAMETER(pvData);
    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(dwPropId);
    UNREFERENCED_PARAMETER(pCrlContext);
    UNREFERENCED_PARAMETER(hStoreProv);
}

static BOOL WINAPI ExtStoreProvFindCtl(
    HCERTSTOREPROV hStoreProv,
    PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    PCCTL_CONTEXT pPrevCtlContext,
    DWORD dwFlags,
    void **ppvStoreProvFindInfo,
    PCCTL_CONTEXT *ppProvCtlContext
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PFIND_EXT_INFO pFindExtInfo = (PFIND_EXT_INFO) *ppvStoreProvFindInfo;
    PCCTL_CONTEXT pPrevExtContext;
    PCCTL_CONTEXT pProvCtlContext;

    if (pFindExtInfo)
    {
        ASSERT((CERT_STORE_CTL_CONTEXT - 1) ==
            pFindExtInfo->dwContextType);
        pPrevExtContext = (PCCTL_CONTEXT) pFindExtInfo->pvContext;
        pFindExtInfo->pvContext = NULL;
    } else
        pPrevExtContext = NULL;

    ASSERT(pExtStore);
    ASSERT(pPrevCtlContext == pPrevExtContext);

    if ((pProvCtlContext = CertFindCTLInStore(
            pExtStore->hExtCertStore,
            pFindInfo->dwMsgAndCertEncodingType,
            pFindInfo->dwFindFlags,
            pFindInfo->dwFindType,
            pFindInfo->pvFindPara,
            pPrevExtContext
            )) != 0)
    {
        if (pFindExtInfo)
            // Re-use existing Find Info
            pFindExtInfo->pvContext = (void *) CertDuplicateCTLContext(
                pProvCtlContext);
        else {
            if ((pFindExtInfo = CreateExtInfo(
                    CERT_STORE_CTL_CONTEXT - 1,
                    (void *) pProvCtlContext
                    )) != 0)
                pProvCtlContext = CertDuplicateCTLContext(pProvCtlContext);
            else
            {
                CertFreeCTLContext(pProvCtlContext);
                pProvCtlContext = NULL;
            }
        }
    } else if (pFindExtInfo) {
        ExtStoreProvFreeFindCtl(
            hStoreProv,
            pPrevCtlContext,
            pFindExtInfo,
            0                       // dwFlags
            );
        pFindExtInfo = NULL;
    }

    *ppProvCtlContext = pProvCtlContext;
    *ppvStoreProvFindInfo = pFindExtInfo;
    return NULL != pProvCtlContext;

    UNREFERENCED_PARAMETER(dwFlags);
}

static BOOL WINAPI ExtStoreProvFreeFindCtl(
    HCERTSTOREPROV hStoreProv,
    PCCTL_CONTEXT pCtlContext,
    void *pvStoreProvFindInfo,
    DWORD dwFlags
    )
{
    PFIND_EXT_INFO pFindExtInfo = (PFIND_EXT_INFO) pvStoreProvFindInfo;

    ASSERT(pFindExtInfo);
    if (pFindExtInfo)
    {
        ASSERT((CERT_STORE_CTL_CONTEXT - 1) ==
            pFindExtInfo->dwContextType);
        FreeExtInfo(pFindExtInfo);
    }
    return TRUE;

    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(pCtlContext);
    UNREFERENCED_PARAMETER(hStoreProv);
}

static BOOL WINAPI ExtStoreProvGetCtlProperty(
    HCERTSTOREPROV hStoreProv,
    PCCTL_CONTEXT pCtlContext,
    DWORD dwPropId,
    DWORD dwFlags,
    void *pvData,
    DWORD *pcbData
    )
{
    *pcbData = 0;
    SetLastError((DWORD) CRYPT_E_NOT_FOUND);
    return FALSE;

    UNREFERENCED_PARAMETER(pcbData);
    UNREFERENCED_PARAMETER(pvData);
    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(dwPropId);
    UNREFERENCED_PARAMETER(pCtlContext);
    UNREFERENCED_PARAMETER(hStoreProv);
}
