// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <ipsectypes.h>
#include "Utils.h"

#define EAP_TLS             13      // Smartcard or other certificate (TLS)
#define EAP_PEAP            25      // PEAP
#define EAP_MSCHAPv2        26      // EAP Mschapv2

#define RDT_Tunnel_Pptp (0x8)
#define RDT_Tunnel_L2tp (0x9)
#define RDT_Tunnel_Sstp (0xe)
#define RDT_Tunnel_Ikev2 (0xf)

HINSTANCE g_hMpradminDll = NULL;

PMPRADMINSERVERCONNECT              g_pMprAdminServerConnect                    = NULL;
PMPRADMINSERVERDISCONNECT           g_pMprAdminServerDisconnect                 = NULL;
PMPRADMININTERFACECREATE            g_pMprAdminInterfaceCreate                  = NULL;
PMPRADMININTERFACEGETHANDLE         g_pMprAdminInterfaceGetHandle               = NULL;
PMPRADMININTERFACEGETINFO           g_pMprAdminInterfaceGetInfo                 = NULL;
PMPRADMININTERFACESETCUSTOMINFOEX   g_pMprAdminInterfaceSetCustomInfoEx         = NULL;
PMPRADMININTERFACEGETCUSTOMINFOEX   g_pMprAdminInterfaceGetCustomInfoEx         = NULL;
PMPRADMINBUFFERFREE                 g_pMprAdminBufferFree                       = NULL;
PMPRADMINSERVERGETINFOEX            g_pMprAdminServerGetInfoEx                  = NULL;
PMPRADMINSERVERSETINFOEX            g_pMprAdminServerSetInfoEx                  = NULL;
PMPRADMINCONNECTIONENUM             g_pMprAdminConnectionEnum                   = NULL;

PMPRCONFIGSERVERCONNECT             g_pMprConfigServerConnect                   = NULL;
PMPRCONFIGSERVERDISCONNECT          g_pMprConfigServerDisconnect                = NULL;
PMPRCONFIGINTERFACECREATE           g_pMprConfigInterfaceCreate                 = NULL;
PMPRCONFIGINTERFACEGETHANDLE        g_pMprConfigInterfaceGetHandle              = NULL;
PMPRCONFIGINTERFACESETCUSTOMINFOEX  g_pMprConfigInterfaceSetCustomInfoEx        = NULL;
PMPRCONFIGINTERFACEGETCUSTOMINFOEX  g_pMprConfigInterfaceGetCustomInfoEx        = NULL;
PMPRCONFIGBUFFERFREE                g_pMprConfigBufferFree                      = NULL;
PMPRCONFIGSERVERSETINFOEX           g_pMprConfigServerSetInfoEx                 = NULL;
PMPRCONFIGSERVERGETINFOEX           g_pMprConfigServerGetInfoEx                 = NULL;


#define SZ_MprAdminServerConnect                "MprAdminServerConnect"
#define SZ_MprAdminServerDisconnect             "MprAdminServerDisconnect"
#define SZ_MprAdminInterfaceCreate              "MprAdminInterfaceCreate"
#define SZ_MprAdminInterfaceGetInfo             "MprAdminInterfaceGetInfo"
#define SZ_MprAdminInterfaceGetHandle           "MprAdminInterfaceGetHandle"
#define SZ_MprAdminInterfaceSetCustomInfoEx     "MprAdminInterfaceSetCustomInfoEx"
#define SZ_MprAdminInterfaceGetCustomInfoEx     "MprAdminInterfaceGetCustomInfoEx"
#define SZ_MprAdminBufferFree                   "MprAdminBufferFree"
#define SZ_MprAdminServerGetInfoEx              "MprAdminServerGetInfoEx"
#define SZ_MprAdminServerSetInfoEx              "MprAdminServerSetInfoEx"
#define SZ_MprAdminConnectionEnum               "MprAdminConnectionEnum"

#define SZ_MprConfigServerConnect               "MprConfigServerConnect"
#define SZ_MprConfigServerDisconnect            "MprConfigServerDisconnect"
#define SZ_MprConfigInterfaceCreate             "MprConfigInterfaceCreate"
#define SZ_MprConfigInterfaceGetHandle          "MprConfigInterfaceGetHandle"
#define SZ_MprConfigInterfaceSetCustomInfoEx    "MprConfigInterfaceSetCustomInfoEx"
#define SZ_MprConfigInterfaceGetCustomInfoEx    "MprConfigInterfaceGetCustomInfoEx"
#define SZ_MprConfigBufferFree                  "MprConfigBufferFree"
#define SZ_MprConfigServerGetInfoEx             "MprConfigServerGetInfoEx"
#define SZ_MprConfigServerSetInfoEx             "MprConfigServerSetInfoEx"

//********************************************************************************************
// Function: LoadMprApiLibrary
//
// Description: Loads the MprAPI.dll and imports the required APIs
//
//********************************************************************************************
DWORD LoadMprApiLibrary()
{
    DWORD status = ERROR_SUCCESS;

    if (NULL != g_hMpradminDll)
        return 0;
    
    g_hMpradminDll = LoadLibraryEx(L"MPRAPI.DLL", NULL, 0);
    if (g_hMpradminDll == NULL)
    {
        status = GetLastError();
        printf("Failed to load MprApi.dll\n");
        DisplayError(status);
        goto Done;
    }

    g_pMprAdminInterfaceCreate = (PMPRADMININTERFACECREATE)GetProcAddress(g_hMpradminDll, SZ_MprAdminInterfaceCreate); \
    if (g_pMprAdminInterfaceCreate == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprAdminInterfaceCreate); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprAdminInterfaceGetHandle = (PMPRADMININTERFACEGETHANDLE)GetProcAddress(g_hMpradminDll, SZ_MprAdminInterfaceGetHandle); \
    if (g_pMprAdminInterfaceGetHandle == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprAdminInterfaceGetHandle); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprAdminInterfaceGetInfo = (PMPRADMININTERFACEGETINFO)GetProcAddress(g_hMpradminDll, SZ_MprAdminInterfaceGetInfo); \
    if (g_pMprAdminInterfaceGetInfo == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprAdminInterfaceGetInfo); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprAdminInterfaceSetCustomInfoEx = (PMPRADMININTERFACESETCUSTOMINFOEX)GetProcAddress(g_hMpradminDll, SZ_MprAdminInterfaceSetCustomInfoEx); \
    if (g_pMprAdminInterfaceSetCustomInfoEx == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprAdminInterfaceSetCustomInfoEx); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprAdminInterfaceGetCustomInfoEx = (PMPRADMININTERFACEGETCUSTOMINFOEX)GetProcAddress(g_hMpradminDll, SZ_MprAdminInterfaceGetCustomInfoEx); \
    if (g_pMprAdminInterfaceGetCustomInfoEx == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprAdminInterfaceGetCustomInfoEx); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprAdminBufferFree = (PMPRADMINBUFFERFREE)GetProcAddress(g_hMpradminDll, SZ_MprAdminBufferFree); \
    if (g_pMprAdminBufferFree == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprAdminBufferFree); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprAdminServerConnect = (PMPRADMINSERVERCONNECT)GetProcAddress(g_hMpradminDll, SZ_MprAdminServerConnect); \
    if (g_pMprAdminServerConnect == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprAdminServerConnect); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprAdminServerDisconnect = (PMPRADMINSERVERDISCONNECT)GetProcAddress(g_hMpradminDll, SZ_MprAdminServerDisconnect); \
    if (g_pMprAdminServerDisconnect == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprAdminServerDisconnect); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprAdminServerGetInfoEx = (PMPRADMINSERVERGETINFOEX)GetProcAddress(g_hMpradminDll, SZ_MprAdminServerGetInfoEx); \
    if (g_pMprAdminServerGetInfoEx == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprAdminServerGetInfoEx); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprAdminServerSetInfoEx = (PMPRADMINSERVERSETINFOEX)GetProcAddress(g_hMpradminDll, SZ_MprAdminServerSetInfoEx); \
    if (g_pMprAdminServerSetInfoEx == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprAdminServerSetInfoEx); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprAdminConnectionEnum = (PMPRADMINCONNECTIONENUM)GetProcAddress(g_hMpradminDll, SZ_MprAdminConnectionEnum); \
    if (g_pMprAdminConnectionEnum == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprAdminConnectionEnum); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprConfigBufferFree = (PMPRCONFIGBUFFERFREE)GetProcAddress(g_hMpradminDll, SZ_MprConfigBufferFree); \
    if (g_pMprConfigBufferFree == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprConfigBufferFree); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprConfigServerConnect = (PMPRCONFIGSERVERCONNECT)GetProcAddress(g_hMpradminDll, SZ_MprConfigServerConnect); \
    if (g_pMprConfigServerConnect == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprConfigServerConnect); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprConfigServerDisconnect = (PMPRCONFIGSERVERDISCONNECT)GetProcAddress(g_hMpradminDll, SZ_MprConfigServerDisconnect); \
    if (g_pMprConfigServerDisconnect == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprConfigServerDisconnect); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprConfigInterfaceCreate = (PMPRCONFIGINTERFACECREATE)GetProcAddress(g_hMpradminDll, SZ_MprConfigInterfaceCreate); \
    if (g_pMprConfigInterfaceCreate == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprConfigInterfaceCreate); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprConfigInterfaceGetHandle = (PMPRCONFIGINTERFACEGETHANDLE)GetProcAddress(g_hMpradminDll, SZ_MprConfigInterfaceGetHandle); \
    if (g_pMprConfigInterfaceGetHandle == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprConfigInterfaceGetHandle); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprConfigInterfaceSetCustomInfoEx = (PMPRCONFIGINTERFACESETCUSTOMINFOEX)GetProcAddress(g_hMpradminDll, SZ_MprConfigInterfaceSetCustomInfoEx); \
    if (g_pMprConfigInterfaceSetCustomInfoEx == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprConfigInterfaceSetCustomInfoEx); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprConfigInterfaceGetCustomInfoEx = (PMPRCONFIGINTERFACEGETCUSTOMINFOEX)GetProcAddress(g_hMpradminDll, SZ_MprConfigInterfaceGetCustomInfoEx); \
    if (g_pMprConfigInterfaceGetCustomInfoEx == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprConfigInterfaceGetCustomInfoEx); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprConfigServerGetInfoEx = (PMPRCONFIGSERVERGETINFOEX)GetProcAddress(g_hMpradminDll, SZ_MprConfigServerGetInfoEx); \
    if (g_pMprConfigServerGetInfoEx == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprConfigServerGetInfoEx); 
        DisplayError(status); 
        goto Done; 
    } 

    g_pMprConfigServerSetInfoEx = (PMPRCONFIGSERVERSETINFOEX)GetProcAddress(g_hMpradminDll, SZ_MprConfigServerSetInfoEx); \
    if (g_pMprConfigServerSetInfoEx == NULL) 
    { 
        status = GetLastError(); 
        printf("Failed to load function '%s'\n", SZ_MprConfigServerSetInfoEx); 
        DisplayError(status); 
        goto Done; 
    } 
    
Done:
    if (ERROR_SUCCESS != status)
    {
        ReleaseMprApiLibrary();
    }
    return status;
}

//********************************************************************************************
// Function: ReleaseMprApiLibrary
//
// Description: Free the MprAPI.dll if already loaded
//
//********************************************************************************************
VOID ReleaseMprApiLibrary()
{
    if (NULL != g_hMpradminDll)
    {
        HINSTANCE h;

        g_pMprAdminServerConnect                    = NULL;
        g_pMprAdminServerDisconnect                 = NULL;
        g_pMprAdminInterfaceCreate                  = NULL;
        g_pMprAdminInterfaceGetHandle               = NULL;
        g_pMprAdminInterfaceGetInfo                 = NULL;
        g_pMprAdminInterfaceSetCustomInfoEx         = NULL;
        g_pMprAdminInterfaceGetCustomInfoEx         = NULL;
        g_pMprAdminBufferFree                       = NULL;
        g_pMprAdminConnectionEnum                   = NULL;

        g_pMprConfigServerConnect                   = NULL;
        g_pMprConfigServerDisconnect                = NULL;
        g_pMprConfigInterfaceCreate                 = NULL;
        g_pMprConfigInterfaceGetHandle              = NULL;
        g_pMprConfigInterfaceSetCustomInfoEx        = NULL;
        g_pMprConfigInterfaceGetCustomInfoEx        = NULL;
        g_pMprConfigBufferFree                      = NULL;

        h = g_hMpradminDll;
        g_hMpradminDll = NULL;
        
        FreeLibrary(h);
    }
}

//********************************************************************************************
// Function: RemoteAccessServerConenct
//
// Description: Connects to the RRAS server for configuration and administration.
//
//********************************************************************************************
DWORD RemoteAccessServerConenct(
    _In_opt_ LPWSTR serverName, 
    _Out_ MPR_SERVER_HANDLE* serverHandleAdmin,
    _Out_ HANDLE* serverHandleConfig
    )
{
    DWORD adminConnectStatus = ERROR_SUCCESS;
    DWORD configConnectStatus = ERROR_SUCCESS;
    adminConnectStatus = g_pMprAdminServerConnect(serverName, serverHandleAdmin);
    if ((ERROR_SUCCESS != adminConnectStatus) || 
        (NULL == *serverHandleAdmin))
    {
        wprintf(L"MprAdminServerConnect failed\n");
        DisplayError(adminConnectStatus);
    }
    
    configConnectStatus = g_pMprConfigServerConnect(serverName, serverHandleConfig);
    if ((ERROR_SUCCESS != configConnectStatus) || 
        (NULL == *serverHandleConfig))
    {
        wprintf(L"MprConfigServerConnect failed.\n");
        DisplayError(configConnectStatus);
    }
    
    return (ERROR_SUCCESS == adminConnectStatus) ? configConnectStatus : adminConnectStatus;
}

//********************************************************************************************
// Function: RemoteAccessServerDisconenct
//
// Description: Disconnects the connection with remote access server made in RemoteAccessServerDisconenct.
//
//********************************************************************************************
VOID RemoteAccessServerDisconenct(
    _In_ MPR_SERVER_HANDLE serverHandleAdmin,
    _In_ HANDLE serverHandleConfig
    )
{
    if (serverHandleAdmin != NULL)
        g_pMprAdminServerDisconnect(serverHandleAdmin);
    
    if (serverHandleConfig != NULL)
        g_pMprConfigServerDisconnect(serverHandleConfig);
}

//*********************************************************************************************************
// Function: GetCertificateNameAndHashBlob
//
// Description: Retrieves the firsts certificate from the system certificate store and returns it's certificate name  and hash blobs.
// The hash blob is supported to ensure that multiple certificates with same name are supported.
//
//*********************************************************************************************************
DWORD GetCertificateNameAndHashBlob(
    _Out_ CERT_NAME_BLOB* certName,
    _Out_ CRYPT_HASH_BLOB* certHash
    )
{
    HCERTSTORE  certStoreHandle = NULL;
    PCCERT_CONTEXT certContext = NULL;
    DWORD status = ERROR_SUCCESS;
    DWORD bufSize =0;

    certName->cbData = 0;
    certName->pbData = NULL;
   
    certHash->cbData = 0;
    certHash->pbData = NULL;

    certStoreHandle = CertOpenStore(
                          CERT_STORE_PROV_SYSTEM,
                          0,
                          (HCRYPTPROV )NULL,
                          CERT_SYSTEM_STORE_LOCAL_MACHINE |
                          CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG, 
                          (VOID *)L"My"
                          );
    if (NULL == certStoreHandle)
    {
        status = GetLastError();
        wprintf(L"Failed to open certificate store.\n"); 
        DisplayError(status);
        goto Done;
    }
    
    certContext = CertEnumCertificatesInStore(certStoreHandle, NULL);
    if (certContext == NULL)
    {
        wprintf(L"Error:No certificate found\n");
        DisplayError(GetLastError());
        goto Done;
    }
    
    ZeroMemory(certName, sizeof(CERT_NAME_BLOB));    
    certName->pbData = (PBYTE) MPRAPI_SAMPLE_MALLOC(certContext->pCertInfo->Subject.cbData);
    if (NULL == certName->pbData)
    {
        status = ERROR_OUTOFMEMORY;
        wprintf(L"Error:Failed to allocate memory for certName.\n");
        DisplayError(status);
        goto Done;
    }
    memcpy(certName->pbData, certContext->pCertInfo->Subject.pbData, certContext->pCertInfo->Subject.cbData);
    certName->cbData = certContext->pCertInfo->Subject.cbData;

    //
    // Get the certificate Hash. When we have multiple certificates with the same name, Hash can be used to point to a particular certificate to configure 
    //
    ZeroMemory(certHash, sizeof(CRYPT_HASH_BLOB));
    if(CertGetCertificateContextProperty(certContext, CERT_HASH_PROP_ID, NULL, &bufSize))
    {
        certHash->pbData = (PBYTE) MPRAPI_SAMPLE_MALLOC(bufSize);
        if (NULL == certHash->pbData)
        {
            status = ERROR_OUTOFMEMORY;
            wprintf(L"Error:Failed to allocate memory for certHash.\n");
            DisplayError(status);
            goto Done;
        }
        
        certHash->cbData = bufSize;
        if (!CertGetCertificateContextProperty(certContext, CERT_HASH_PROP_ID, certHash->pbData, &(certHash->cbData)))
        {
            status  = GetLastError();
            wprintf(L"CertGetCertificateContextProperty failed. with error %d\n",status);
            DisplayError(status);
            goto Done;
        }
    }
    else
    {
        status  = GetLastError();
        wprintf(L"CertGetCertificateContextProperty failed with error %d while getting buffer size for context property.\n",status);
        DisplayError(status);
        goto Done;
    }
    
Done:
    if (certContext != NULL)
        CertFreeCertificateContext(certContext);
    
    if(certStoreHandle != NULL)
        CertCloseStore(certStoreHandle, 0);

    return status;
}

//********************************************************************************************
// Function: PrintIntegrityMethod
//
// Description: Prints the specified integrity method in string format 
//
//********************************************************************************************
VOID PrintIntegrityMethod(
    _In_ DWORD integrityMethod
    )
{
    switch(integrityMethod)
    {
    case IKEEXT_INTEGRITY_MD5:
        wprintf(L"IKEEXT_INTEGRITY_MD5");
        break;
    case IKEEXT_INTEGRITY_SHA1:
        wprintf(L"IKEEXT_INTEGRITY_SHA1");
        break;
    case IKEEXT_INTEGRITY_SHA_256:
        wprintf(L"IKEEXT_INTEGRITY_SHA_256");
        break;
    case IKEEXT_INTEGRITY_SHA_384:
        wprintf(L"IKEEXT_INTEGRITY_SHA_384");
        break;
    default:
        wprintf(L"UnKnown");
        break;
    }
}

//********************************************************************************************
// Function: PrintEncryptionMethod
//
// Description: Prints the specified encryption method in string format 
//
//********************************************************************************************
VOID PrintEncryptionMethod(
    _In_ DWORD encryptionMethod
    )
{
    switch(encryptionMethod)
    {
    case IKEEXT_CIPHER_DES:
        wprintf(L"IKEEXT_CIPHER_DES");
        break;
    case IKEEXT_CIPHER_3DES:
        wprintf(L"IKEEXT_CIPHER_3DES");
        break;
    case IKEEXT_CIPHER_AES_128:
        wprintf(L"IKEEXT_CIPHER_AES_128");
        break;
    case IKEEXT_CIPHER_AES_192:
        wprintf(L"IKEEXT_CIPHER_AES_192");
        break;
    case IKEEXT_CIPHER_AES_256:
        wprintf(L"IKEEXT_CIPHER_AES_256");
        break;
    default:
        wprintf(L"UnKnown");
        break;
    }
}

//********************************************************************************************
// Function: PrintCipherTransformConstant
//
// Description: Prints the specified cipher transform constant in string format 
//
//********************************************************************************************
VOID PrintCipherTransformConstant(
    _In_ DWORD cipherTransformConstant
    )
{
    switch(cipherTransformConstant)
    {
    case IPSEC_CIPHER_TYPE_DES:
        wprintf(L"IPSEC_CIPHER_TYPE_DES");
        break;
    case IPSEC_CIPHER_TYPE_3DES:
        wprintf(L"IPSEC_CIPHER_TYPE_3DES");
        break;
    case IPSEC_CIPHER_TYPE_AES_128:
        wprintf(L"IPSEC_CIPHER_TYPE_AES_128");
        break;
    case IPSEC_CIPHER_TYPE_AES_192:
        wprintf(L"IPSEC_CIPHER_TYPE_AES_192");
        break;
    case IPSEC_CIPHER_TYPE_AES_256:
        wprintf(L"IPSEC_CIPHER_TYPE_AES_256");
        break;
    default:
        wprintf(L"UnKnown");
        break;
    }
}

//********************************************************************************************
// Function: PrintAuthTransformConstant
//
// Description: Prints the specified auth transform constant in string format 
//
//********************************************************************************************
VOID PrintAuthTransformConstant(
    _In_ DWORD authTransformConstant
    )
{
    switch(authTransformConstant)
    {
    case IPSEC_AUTH_MD5:
        wprintf(L"IPSEC_AUTH_MD5");
        break;
    case IPSEC_AUTH_SHA_1:
        wprintf(L"IPSEC_AUTH_SHA_1");
        break;
    case IPSEC_AUTH_SHA_256:
        wprintf(L"IPSEC_AUTH_SHA_256");
        break;
    case IPSEC_AUTH_AES_128:
        wprintf(L"IPSEC_AUTH_AES_128");
        break;
    case IPSEC_AUTH_AES_192:
        wprintf(L"IPSEC_AUTH_AES_192");
        break;
    case IPSEC_AUTH_AES_256:
        wprintf(L"IPSEC_AUTH_AES_256");
        break;
    default:
        wprintf(L"UnKnown");
        break;
    }
}

//********************************************************************************************
// Function: PrintPfsGroup
//
// Description: Prints the specified PFS group in string format 
//
//********************************************************************************************
VOID PrintPfsGroup(
    _In_ DWORD pfsGroup
    )
{
    switch(pfsGroup)
    {
    case IPSEC_PFS_NONE:
        wprintf(L"IPSEC_PFS_NONE");
        break;
    case IPSEC_PFS_1:
        wprintf(L"IPSEC_PFS_1");
        break;
    case IPSEC_PFS_2:
        wprintf(L"IPSEC_PFS_2");
        break;
    case IPSEC_PFS_2048:
        wprintf(L"IPSEC_PFS_2048");
        break;
    case IPSEC_PFS_ECP_256:
        wprintf(L"IPSEC_PFS_ECP_256");
        break;
    case IPSEC_PFS_ECP_384:
        wprintf(L"IPSEC_PFS_ECP_384");
        break;
    case IPSEC_PFS_MM:
        wprintf(L"IPSEC_PFS_MM");
        break;
    case IPSEC_PFS_24:
        wprintf(L"IPSEC_PFS_24");
        break;
    default:
        wprintf(L"UnKnown");
        break;
    }
}

//********************************************************************************************
// Function: PrintDHGroup
//
// Description: Prints the specified DH group in string format 
//
//********************************************************************************************
VOID PrintDHGroup(
    _In_ DWORD dhGroup
    )
{
    switch(dhGroup)
    {
    case IKEEXT_DH_GROUP_NONE:
        wprintf(L"IKEEXT_DH_GROUP_NONE");
        break;
    case IKEEXT_DH_GROUP_1:
        wprintf(L"IKEEXT_DH_GROUP_1");
        break;
    case IKEEXT_DH_GROUP_2:
        wprintf(L"IKEEXT_DH_GROUP_2");
        break;
    case IKEEXT_DH_GROUP_14:
        wprintf(L"IKEEXT_DH_GROUP_14 / IKEEXT_DH_GROUP_2048");
        break;
    case IKEEXT_DH_ECP_256:
        wprintf(L"IKEEXT_DH_ECP_256");
        break;
    case IKEEXT_DH_ECP_384:
        wprintf(L"IKEEXT_DH_ECP_384");
        break;
    case IKEEXT_DH_GROUP_24:
        wprintf(L"IKEEXT_DH_GROUP_24");
        break;
    default:
        wprintf(L"UnKnown");
        break;
    }
}

//********************************************************************************************
// Function: PrintCustomIkev2Policy
//
// Description: Prints various fields of the specified ROUTER_CUSTOM_IKEv2_POLICY0 structure 
// in string format
//
//********************************************************************************************
VOID PrintCustomIkev2Policy(
    _In_ LPWSTR prefix, 
    _In_ ROUTER_CUSTOM_IKEv2_POLICY0* customIkev2Policy
    )
{
    if (customIkev2Policy == NULL)
    {
        wprintf(L"NULL custom policy\n");
        return;
    }
    wprintf(L"%s Integrity Method: ", prefix);
    PrintIntegrityMethod(customIkev2Policy->dwIntegrityMethod);
    wprintf(L"\n");
    
    wprintf(L"%s Encryption Method: ", prefix);
    PrintEncryptionMethod(customIkev2Policy->dwEncryptionMethod);
    wprintf(L"\n");
    
    wprintf(L"%s Cipher Transform Constant: ", prefix);
    PrintCipherTransformConstant(customIkev2Policy->dwCipherTransformConstant);
    wprintf(L"\n");
    
    wprintf(L"%s Auth Transform Constant: ", prefix);
    PrintAuthTransformConstant(customIkev2Policy->dwAuthTransformConstant);
    wprintf(L"\n");
    
    wprintf(L"%s Pfs Group: ", prefix);
    PrintPfsGroup(customIkev2Policy->dwPfsGroup);
    wprintf(L"\n");
    
    wprintf(L"%s DH Group: ", prefix);
    PrintDHGroup(customIkev2Policy->dwDhGroup);
    wprintf(L"\n");
}

//********************************************************************************************
// Function: PrintInterfaceType
//
// Description:  Prints the specified interface type in string format  
//
//********************************************************************************************
VOID PrintInterfaceType(
    _In_ ROUTER_INTERFACE_TYPE ifType
    )
{
    static LPWSTR ifTypeStr[] = {
        L"CLIENT",
        L"HOME_ROUTER",
        L"FULL_ROUTER",
        L"DEDICATED",
        L"INTERNAL",
        L"LOOPBACK",
        L"TUNNEL1",
        L"DIALOUT"
        };
    if (ifType < ARRAYSIZE(ifTypeStr))
        wprintf(L"%s", ifTypeStr[ifType]);
    else
        wprintf(L"Unknown");
        
}

//********************************************************************************************
// Function: PrintConnectionState
//
// Description: Prints the specified connection state in string format  
//
//********************************************************************************************
VOID PrintConnectionState(
    _In_ ROUTER_CONNECTION_STATE connState
    )
{
    static LPWSTR connectionStateStr[] = {
        L"ROUTER_IF_STATE_UNREACHABLE",
        L"ROUTER_IF_STATE_DISCONNECTED",
        L"ROUTER_IF_STATE_CONNECTING",
        L"ROUTER_IF_STATE_CONNECTED"
        };
    if (connState < ARRAYSIZE(connectionStateStr))
        wprintf(L"%s", connectionStateStr[connState]);
    else
        wprintf(L"Unknown");
        
}

//********************************************************************************************
// Function: PrintEncryptionType
//
// Description: Prints the specified encryption type in string format  
//
//********************************************************************************************
VOID PrintEncryptionType(
    _In_ DWORD encryptionType
    )
{
    static LPWSTR encryptionTypeStr[] = {
        L"MPR_ET_None",
        L"MPR_ET_Require",
        L"MPR_ET_RequireMax",
        L"MPR_ET_Optional"
        };
    if (encryptionType < ARRAYSIZE(encryptionTypeStr))
        wprintf(L"%s", encryptionTypeStr[encryptionType]);
    else
        wprintf(L"Unknown");
        
}

//********************************************************************************************
// Function: PrintEntryType
//
// Description: Prints the specified entry type in string format  
//
//********************************************************************************************
VOID PrintEntryType(
    _In_ DWORD entryType
    )
{
    switch(entryType)
    {
    case MPRET_Phone:
        wprintf(L"MPRET_Phone");
        break;
    case MPRET_Vpn:
        wprintf(L"MPRET_Vpn");
        break;
    default:
        wprintf(L"UnKnown");
        break;
    }
}

//********************************************************************************************
// Function: PrintVpnStrategy
//
// Description: Prints the specified VPN strategy in string format  
//
//********************************************************************************************
VOID PrintVpnStrategy(
    _In_ DWORD vpnStrategy
    )
{
    switch(vpnStrategy)
    {
    case MPR_VS_Default:
        wprintf(L"MPR_VS_Default");
        break;
    case MPR_VS_PptpOnly:
        wprintf(L"MPR_VS_PptpOnly");
        break;
    case MPR_VS_PptpFirst:
        wprintf(L"MPR_VS_PptpFirst");
        break;
    case MPR_VS_L2tpOnly:
        wprintf(L"MPR_VS_L2tpOnly");
        break;
    case MPR_VS_L2tpFirst:
        wprintf(L"MPR_VS_L2tpFirst");
        break;
    case MPR_VS_Ikev2Only:
        wprintf(L"MPR_VS_Ikev2Only");
        break;
    case MPR_VS_Ikev2First:
        wprintf(L"MPR_VS_Ikev2First");
        break;
    default:
        wprintf(L"UnKnown");
        break;
    }
}

//********************************************************************************************
// Function: PrintFileTime
//
// Description: Converts file time to local time, to display to the user
//
//********************************************************************************************

VOID PrintFileTime(
    _In_ FILETIME* time
    )
{
    SYSTEMTIME stUTC, stLocal;

    // Convert filetime to local time.
    //
    FileTimeToSystemTime(time, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);      
    wprintf(L"%02u/%02u/%u  %02u:%02u:%02u", 
           stLocal.wMonth, stLocal.wDay, stLocal.wYear,stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
}     

//********************************************************************************************
// Function: DisplayError
//
// Description: Displays error description
//
//********************************************************************************************
VOID DisplayError(
    _In_ DWORD error
    )
{
    wprintf(L"Error code: %u\n", error);
}

//********************************************************************************************
// Function: FlushCurrentLine
//
// Description: Clears any input lingering in the STDIN buffer
//
//********************************************************************************************
VOID FlushCurrentLine()
{
    int i;
    while ((i = getc(stdin)) != EOF && i != '\n')
    {
        continue;
    }
}


//********************************************************************************************
// Function: GetCertificateEkus
//
// Description: Set Client Authentication and Server Authentication as the certificate EKU's 
//
//********************************************************************************************

DWORD GetCertificateEkus(
    _Out_ PMPR_CERT_EKU     *certEKUs,
    _Out_ DWORD*            TotalEKUs
    )
{
    DWORD           status          = ERROR_SUCCESS;
    MPR_CERT_EKU    *tempCertEKUs   = NULL ; 
    const DWORD     size            = 2; //Using two EKU's
    size_t          dwEkuSize       = 0;
    WCHAR*          Eku             = NULL;
    EKU_Info        tempEKUs[size];

    for(DWORD i=0;i<size; i++)
    {
        ZeroMemory(&tempEKUs[i], sizeof(EKU_Info));
    }

    // Setting some default EKU values for the sample
    tempEKUs[0].Name = L"1.3.6.1.5.5.7.3.2";//Client Authentication
    tempEKUs[0].IsOid = true;
    tempEKUs[1].Name = L"Server Authentication";//Server Authentication
    tempEKUs[1].IsOid = false;
    
    if(certEKUs == NULL || TotalEKUs == NULL)
    {
        status = ERROR_INVALID_PARAMETER;
        wprintf(L"Invalid Parameter\n");
        goto Done;
    }
    
    //Setting the default values to 0
    *certEKUs = NULL;
    *TotalEKUs = 0;

    tempCertEKUs = (MPR_CERT_EKU*)MPRAPI_SAMPLE_MALLOC(size * sizeof(MPR_CERT_EKU));
    if (NULL == tempCertEKUs)
    {
        status = ERROR_OUTOFMEMORY;
        wprintf(L"Error:Failed to allocate memory.\n");
        DisplayError(status);
        goto Done;
    }

    for(DWORD i=0;i<size; i++)
    {
        dwEkuSize = wcslen(tempEKUs[i].Name) + 1;
        Eku = (WCHAR*)MPRAPI_SAMPLE_MALLOC (sizeof(WCHAR)*(dwEkuSize));
        if (NULL == Eku)
        {
            status = ERROR_OUTOFMEMORY;
            wprintf(L"Error:Failed to allocate memory.\n");
            DisplayError(status);
            goto Done;
        }
        wcscpy_s(Eku, dwEkuSize, tempEKUs[i].Name);
        tempCertEKUs[i].pwszEKU = Eku;
        tempCertEKUs[i].dwSize = (DWORD)dwEkuSize;
        tempCertEKUs[i].IsEKUOID = tempEKUs[i].IsOid;
        
    }
        
    // Setting EKU's 
    *TotalEKUs = size; 
    *certEKUs = tempCertEKUs;
    
Done:
    return status;
}

//********************************************************************************************
// Function: FreeCertificateEKU
//
// Description: Releases the various fields of the MPR_CERT_EKU structure,
// those are allocated in the sample.
//
//********************************************************************************************

VOID FreeCertificateEKU(
    _In_ PMPR_CERT_EKU  certificateEKUs,
    _In_ DWORD          dwTotalEkus)
{
    DWORD i = 0;
    if (certificateEKUs != NULL)
    {
        for(i = 0;i < dwTotalEkus;i++)
        {
            if(certificateEKUs[i].pwszEKU != NULL)
            {
               MPRAPI_SAMPLE_FREE(certificateEKUs[i].pwszEKU);
                certificateEKUs[i].pwszEKU = NULL;
                certificateEKUs[i].dwSize = 0;
            }
        }
        MPRAPI_SAMPLE_FREE(certificateEKUs);
        certificateEKUs = NULL;
    }
}
