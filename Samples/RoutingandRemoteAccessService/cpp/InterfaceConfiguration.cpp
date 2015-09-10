// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Utils.h"
#include <ipsectypes.h>
#include <objbase.h>
#include "InterfaceConfiguration.h"


//********************************************************************************************
// Function: SetCustomIpsecConfigurationOnInterface
//
// Description: Creates a demand dial interface and configures custom IPSec policies on it.
//
//********************************************************************************************
VOID SetCustomIpsecConfigurationOnInterface(
    _In_opt_ LPWSTR serverName
    )
{
    DWORD status = ERROR_SUCCESS;
    MPR_SERVER_HANDLE serverHandleAdmin = NULL;
    HANDLE serverHandleConfig = NULL;
    HANDLE interfaceHandleAdmin = NULL;
    HANDLE interfaceHandleConfig = NULL;

    wprintf(L"---------------------------------------------------------\n\n");
    wprintf(L"Executing SetCustomIpsecConfigurationOnInterface on '%s'\n", \
        (serverName == NULL) ? L"Current machine" : serverName);
    
    // Try connecting to remoteAccess (RRAS) server for both administration and and configuration to 
    // get the handles serverHandleAdmin and serverHandleConfig. We cannot create/modify/retrieve 
    // interfaces configurations, if it fails to get either of these handles.
    //
    status = RemoteAccessServerConenct(serverName, &serverHandleAdmin, &serverHandleConfig);

    // Both serverHandleAdmin and serverHandleConfig has to be valid.
    //
    if ((ERROR_SUCCESS != status) || 
        (NULL == serverHandleAdmin) ||
        (NULL == serverHandleConfig))
    {
        wprintf(L"RemoteAccessServerConenct failed. \
            The RemoteAccess service might be stopped - Try after starting the RemoteAccess service.\n");
        DisplayError(status);
        goto Done;
    }
    
    // Create a new interface
    //
    status = CreateNewDoDInterface(serverHandleAdmin, serverHandleConfig, &interfaceHandleAdmin, &interfaceHandleConfig);
    if (ERROR_SUCCESS != status)
    {
        wprintf(L"CreateNewDoDInterface failed.\n");
        DisplayError(status);
        goto Done;
    }

    // Set custom IPSec policies on interface
    //
    status = ConfigureCustomIPSecPolicyOnDoDInterface(serverHandleAdmin, serverHandleConfig, interfaceHandleAdmin, interfaceHandleConfig);
    if (ERROR_SUCCESS != status)
    {
        wprintf(L"ConfigureCustomIPSecPolicyOnDoDInterface failed.\n");
        DisplayError(status);
        goto Done;
    }

    // Retrieve and display the interface configuration
    //
    status = DisplayDoDInterfaceConfiguration(serverHandleAdmin, interfaceHandleAdmin);
    if (ERROR_SUCCESS != status)
    {
        wprintf(L"DisplayDoDInterfaceConfiguration failed.\n");
        DisplayError(status);
        goto Done;
    }

Done:
    RemoteAccessServerDisconenct(serverHandleAdmin, serverHandleConfig);
    
    if (status != ERROR_SUCCESS)
    {
         wprintf(L"SetCustomIpsecConfigurationOnInterface failed\n");
         DisplayError(status);
    }
    wprintf(L"---------------------------------------------------------\n\n");
}

//********************************************************************************************
// Function: RemoveCustomIpsecConfigurationFromInterface
//
// Description: Removes custom IPSec configuration from a demand dial interface.
//
//********************************************************************************************
VOID RemoveCustomIpsecConfigurationFromInterface(
    _In_opt_ LPWSTR serverName
    )
{
    DWORD status = ERROR_SUCCESS;
    MPR_SERVER_HANDLE serverHandleAdmin = NULL;
    HANDLE serverHandleConfig = NULL;
    HANDLE interfaceHandleAdmin = NULL;
    HANDLE interfaceHandleConfig = NULL;
    WCHAR interfaceName[256] = {0};
    MPR_IF_CUSTOMINFOEX interfaceCustomConfigCurrent;
    MPR_IF_CUSTOMINFOEX interfaceCustomConfigToBeUpdated;
    
    ZeroMemory(&interfaceCustomConfigCurrent, sizeof(MPR_IF_CUSTOMINFOEX));
    ZeroMemory(&interfaceCustomConfigToBeUpdated, sizeof(MPR_IF_CUSTOMINFOEX));


    wprintf(L"---------------------------------------------------------\n\n");
    wprintf(L"Executing RemoveCustomIpsecConfigurationFromInterface on '%s'\n", \
        (serverName == NULL) ? L"Current machine" : serverName);
    
    wprintf(L"\nEnter the interface name: ");
    wscanf_s(L"%s", interfaceName, 256);
    FlushCurrentLine();

    // Try connecting to remoteAccess (RRAS) server for both administration and and configuration to 
    // get the handles serverHandleAdmin and serverHandleConfig. We cannot create/modify/retrieve 
    // interfaces configurations, if it fails to get either of these handles.
    //
    status = RemoteAccessServerConenct(serverName, &serverHandleAdmin, &serverHandleConfig);

    // Both serverHandleAdmin and serverHandleConfig has to be valid.
    //
    if ((ERROR_SUCCESS != status) || 
        (NULL == serverHandleAdmin) ||
        (NULL == serverHandleConfig))
    {
        wprintf(L"RemoteAccessServerConenct failed. \
            The RemoteAccess service might be stopped - Try after starting the RemoteAccess service.\n");
        DisplayError(status);
        goto Done;
    }
    
    status = g_pMprAdminInterfaceGetHandle(
        serverHandleAdmin, 
        interfaceName, 
        &interfaceHandleAdmin, 
        FALSE
        );
    if (ERROR_SUCCESS != status)
    {
        if (ERROR_NO_SUCH_INTERFACE == status)
        {
            // Interface does not exist
            //
            wprintf(L"Interface '%s' does not exists.\n", interfaceName);
        }
        else
        {
            // some genuine error, we should return
            //
            wprintf(L"MprAdminInterfaceGetHandle failed.\n");
            DisplayError(status);
        }
        goto Done;
    }

    status = g_pMprConfigInterfaceGetHandle(
        serverHandleConfig, 
        interfaceName, 
        &interfaceHandleConfig
        );
    if (ERROR_SUCCESS != status)
    {
        if (ERROR_NO_SUCH_INTERFACE == status)
        {
            // Interface does not exist
            //
            wprintf(L"Interface '%s' does not exists.\n", interfaceName);
        }
        else
        {
            // some genuine error, we should return
            //
            wprintf(L"MprConfigInterfaceGetHandle failed.\n");
            DisplayError(status);
        }
        goto Done;
    }

    // Retrieve and display the custom IPSec configuration on the interface (before removing the custom configuration);
    //
    status = DisplayDoDInterfaceConfiguration(serverHandleAdmin, interfaceHandleAdmin);
    if (ERROR_SUCCESS != status)
    {
        wprintf(L"DisplayDoDInterfaceConfiguration failed.\n");
        DisplayError(status);
        goto Done;
    }
    
    // Retrieve the current custom configuration interface 
    //
    interfaceCustomConfigCurrent.Header.revision = MPRAPI_MPR_IF_CUSTOM_CONFIG_OBJECT_REVISION_2;
    interfaceCustomConfigCurrent.Header.type     = MPRAPI_OBJECT_TYPE_IF_CUSTOM_CONFIG_OBJECT;
    interfaceCustomConfigCurrent.Header.size     = sizeof(MPR_IF_CUSTOMINFOEX);

    status = g_pMprAdminInterfaceGetCustomInfoEx(
        serverHandleAdmin,       // hMprServer
        interfaceHandleAdmin,         // interfaceHandleAdmin
        &interfaceCustomConfigCurrent  // pCustomInfo
        );
    if (ERROR_SUCCESS != status)
    {
        // some genuine error, we should return
        //
        wprintf(L"MprAdminInterfaceGetCustomInfoEx failed.\n");
        DisplayError(status);
        goto Done;
    }

    // Check whether custom configuration is available on the specified interface
    //
    if (!(interfaceCustomConfigCurrent.dwFlags & MPRAPI_IF_CUSTOM_CONFIG_FOR_IKEV2))
    {
        wprintf(L"No custom configuration available on interface '%s'.\n", interfaceName);
        goto Done;
    }
    wprintf(L"Removing custom configuration from interface '%s'.\n", interfaceName);

    // Copy the current custom configuration object to the to be updated custom configuration
    // object.
    //
    memcpy(&interfaceCustomConfigToBeUpdated, &interfaceCustomConfigCurrent, sizeof(MPR_IF_CUSTOMINFOEX));

    // Remove the IPSec certificate configuration (if available)
    //
    if (interfaceCustomConfigCurrent.customIkev2Config.certificateName.cbData != 0 ||
        interfaceCustomConfigCurrent.customIkev2Config.certificateHash.cbData != 0 )
    {
        wprintf(L"Removing the certificate configuration from interface '%s'.\n", interfaceName);
        
        // In order to remove machine certificate configuration, 
        // set the cbData of certificateName (CERT_NAME_BLOB) to 0 and 
        // set the pbData of certificateName (CERT_NAME_BLOB) to NULL.
        //set the cbData of certificateHash (CRYPT_HASH_BLOB) to 0 and 
        // set the pbData of certificateHash (CRYPT_HASH_BLOB) to NULL.
        interfaceCustomConfigToBeUpdated.customIkev2Config.certificateName.cbData = 0;
        interfaceCustomConfigToBeUpdated.customIkev2Config.certificateName.pbData = NULL;
        interfaceCustomConfigToBeUpdated.customIkev2Config.certificateHash.cbData = 0;
        interfaceCustomConfigToBeUpdated.customIkev2Config.certificateHash.pbData = NULL;
        
        // Update the custom configuration on the interface
        //
        status = UpdateInterfaceCustomConfiguration(
            serverHandleAdmin,
            serverHandleConfig,
            interfaceHandleAdmin,
            interfaceHandleConfig,
            &interfaceCustomConfigToBeUpdated
            );
        if (ERROR_SUCCESS != status)
        {
            wprintf(L"UpdateInterfaceCustomConfiguration failed.\n");
            DisplayError(status);
            goto Done;
        }
        
        // Retrieve and display the updated interface configuration
        //
        status = DisplayDoDInterfaceConfiguration(serverHandleAdmin, interfaceHandleAdmin);
        if (ERROR_SUCCESS != status)
        {
            wprintf(L"DisplayDoDInterfaceConfiguration failed.\n");
            DisplayError(status);
            goto Done;
        }
    }

    // Remove the IPSec custom policy configuration (if available)
    //
    if (interfaceCustomConfigCurrent.customIkev2Config.customPolicy != NULL)
    {
        wprintf(L"Removing the custom IPSec policy configuration from interface '%s'.\n", interfaceName);

        // In order to remove custom IPsec policy set interfaceCustomConfigToBeUpdated.customPolicy to NULL
        //
        interfaceCustomConfigToBeUpdated.customIkev2Config.customPolicy = NULL;

        // Update the custom configuration on the interface
        //
        status = UpdateInterfaceCustomConfiguration(
            serverHandleAdmin,
            serverHandleConfig,
            interfaceHandleAdmin,
            interfaceHandleConfig,
            &interfaceCustomConfigToBeUpdated
            );
        if (ERROR_SUCCESS != status)
        {
            wprintf(L"UpdateInterfaceCustomConfiguration failed.\n");
            DisplayError(status);
            goto Done;
        }

        // Retrieve and display the updated interface configuration
        //
        status = DisplayDoDInterfaceConfiguration(serverHandleAdmin, interfaceHandleAdmin);
        if (ERROR_SUCCESS != status)
        {
            wprintf(L"DisplayDoDInterfaceConfiguration failed.\n");
            DisplayError(status);
            goto Done;
        }
    }


    // Remove the IPSec custom policy configuration 
    //
    wprintf(L"Removing all the custom configuration from interface '%s'.\n", interfaceName);

    // In order to remove custom IPsec configuration clear the MPRAPI_IF_CUSTOM_CONFIG_FOR_IKEV2 
    // flag from interfaceCustomConfigToBeUpdated.dwFlags
    //
    interfaceCustomConfigToBeUpdated.dwFlags &= ~(MPRAPI_IF_CUSTOM_CONFIG_FOR_IKEV2);

    // Update the custom configuration on the interface
    //
    status = UpdateInterfaceCustomConfiguration(
        serverHandleAdmin,
        serverHandleConfig,
        interfaceHandleAdmin,
        interfaceHandleConfig,
        &interfaceCustomConfigToBeUpdated
        );
    if (ERROR_SUCCESS != status)
    {
        wprintf(L"UpdateInterfaceCustomConfiguration failed.\n");
        DisplayError(status);
        goto Done;
    }

    // Retrieve and display the updated interface configuration
    //
    status = DisplayDoDInterfaceConfiguration(serverHandleAdmin, interfaceHandleAdmin);
    if (ERROR_SUCCESS != status)
    {
        wprintf(L"DisplayDoDInterfaceConfiguration failed.\n");
        DisplayError(status);
        goto Done;
    }

Done:
    
    // Free the interfaceCustomConfigCurrent object
    //
    FreeInterfaceCustomConfigObject(&interfaceCustomConfigCurrent);
    
    RemoteAccessServerDisconenct(serverHandleAdmin, serverHandleConfig);
    
    if (status != ERROR_SUCCESS)
    {
         wprintf(L"SetCustomIpsecConfigurationOnInterface failed\n");
         DisplayError(status);
    }
    wprintf(L"---------------------------------------------------------\n\n");
}


//********************************************************************************************
// Function: CreateNewDoDInterface
//
// Description: Creates a new demand dial interface on the specified remote access server and 
//                   returns the handle to the newly created interface.
//
//********************************************************************************************
DWORD CreateNewDoDInterface(
    _In_ MPR_SERVER_HANDLE serverHandleAdmin,
    _In_ HANDLE serverHandleConfig,
    _Out_ HANDLE* interfaceHandleAdmin,
    _Out_ HANDLE* interfaceHandleConfig
    )
{
    DWORD status = ERROR_SUCCESS;
    MPR_INTERFACE_3 ifInfo;
    BOOL interfaceAlreadyExists = TRUE;
    int index = 0;
    HANDLE tempInterfaceHandle = NULL;

    *interfaceHandleAdmin = NULL;
    *interfaceHandleConfig = NULL;
    
    ZeroMemory(&ifInfo, sizeof(MPR_INTERFACE_3));

    // Create an interface with a name that does not exists
    //
    do 
    {
        StringCchPrintf(ifInfo.wszInterfaceName, ARRAYSIZE(ifInfo.wszInterfaceName), L"DoD-%d", index++);
        status = g_pMprAdminInterfaceGetHandle(
            serverHandleAdmin, 
            ifInfo.wszInterfaceName, 
            &tempInterfaceHandle, 
            FALSE
            );
        if (ERROR_SUCCESS != status)
        {
            if (ERROR_NO_SUCH_INTERFACE == status)
            {
                // Interface does not exists, we can use the current interface name stored in "ifInfo.wszInterfaceName"
                // Ignore the error.
                //
                interfaceAlreadyExists = FALSE;
                status = ERROR_SUCCESS;
            }
            else
            {
                // some genuine error, we should return
                wprintf(L"MprAdminInterfaceGetHandle failed.\n");
                DisplayError(status);
                goto Done;
            }
        }
    }
    while (interfaceAlreadyExists);
    
    // set the other interface properties
    //
    ifInfo.fEnabled = TRUE;
    ifInfo.dwIfType = ROUTER_IF_TYPE_FULL_ROUTER;

    StringCchCopy(ifInfo.szLocalPhoneNumber, ARRAYSIZE(ifInfo.szLocalPhoneNumber), L"131.107.0.2");
    ifInfo.dwfOptions = 
                MPRIO_RequireDataEncryption | 
                MPRIO_RequireMachineCertificates;

    ifInfo.dwfNetProtocols = MPRNP_Ip | MPRNP_Ipv6;

    StringCchCopy(ifInfo.szDeviceType, ARRAYSIZE(ifInfo.szDeviceType), MPRDT_Vpn);
    StringCchCopy(ifInfo.szDeviceName, ARRAYSIZE(ifInfo.szDeviceName), L"Fabrikam Inc 28800 External");


    ifInfo.dwIdleDisconnectSeconds = 300;

    ifInfo.dwType = MPRET_Vpn;

    ifInfo.dwEncryptionType = MPR_ET_Require;

    HRESULT hr = CoCreateGuid(&ifInfo.guidId);
    if(S_OK != hr)
    {
        status = HRESULT_CODE(hr);
        wprintf(L"CoCreateGuid failed.\n");
        DisplayError(status);
    }

    ifInfo.dwVpnStrategy = MPR_VS_Ikev2Only;

    // create an interface 
    //
    status = g_pMprAdminInterfaceCreate(
        serverHandleAdmin,       // hMprServer
        3,                  // dwLevel
        (LPBYTE ) &ifInfo,  // lpBuffer
        interfaceHandleAdmin           // phInterface
        );
    if (ERROR_SUCCESS != status)
    {
        // some genuine error, we should return
        //
        wprintf(L"MprAdminInterfaceCreate failed.\n");
        DisplayError(status);
        goto Done;
    }

    // Persists the interface configuration
    //
    status = g_pMprConfigInterfaceCreate(
        serverHandleConfig,      // hMprConfig
        0,                  // dwLevel
        (LPBYTE ) ((MPR_INTERFACE_0 *)&ifInfo),  // lpBuffer
        interfaceHandleConfig    // phInterface
        );
    if (ERROR_SUCCESS != status)
    {
        // some genuine error, we should return
        //
        wprintf(L"MprConfigInterfaceCreate failed.\n");
        DisplayError(status);
        goto Done;
    }
Done:
    if (ERROR_SUCCESS != status)
    {
         wprintf(L"SetCustomIpsecConfigurationOnInterface failed: %u\n", status);
    }

    return status;

}

//********************************************************************************************
// Function: ConfigureCustomIPSecPolicyOnDoDInterface
//
// Description: Configures custom IPSec policies on the specified interface.
//
//********************************************************************************************
DWORD ConfigureCustomIPSecPolicyOnDoDInterface(
    _In_ MPR_SERVER_HANDLE serverHandleAdmin,
    _In_ HANDLE serverHandleConfig,
    _In_ HANDLE interfaceHandleAdmin,
    _In_ HANDLE interfaceHandleConfig
    )
{
    DWORD status = ERROR_SUCCESS;
    MPR_IF_CUSTOMINFOEX mprInterfaceCustomInfo;
    PROUTER_CUSTOM_IKEv2_POLICY0 customIkev2Policy = NULL;
    CERT_NAME_BLOB machineCertificate;
    CRYPT_HASH_BLOB machineCertificateHash;
    ZeroMemory(&machineCertificate, sizeof(CERT_NAME_BLOB));
    ZeroMemory(&machineCertificateHash, sizeof(CRYPT_HASH_BLOB));

    // Create a custom IKEv2 policy object
    //
    customIkev2Policy = (ROUTER_CUSTOM_IKEv2_POLICY0*) MPRAPI_SAMPLE_MALLOC(sizeof(ROUTER_CUSTOM_IKEv2_POLICY0));
    if (NULL == customIkev2Policy)
    {
        status = ERROR_OUTOFMEMORY;
        wprintf(L"Error:Failed to allocate memory.\n");
        DisplayError(status);
        goto Done;
    }
    customIkev2Policy->dwIntegrityMethod = IKEEXT_INTEGRITY_SHA_256;
    customIkev2Policy->dwEncryptionMethod = IKEEXT_CIPHER_AES_256;
    customIkev2Policy->dwCipherTransformConstant = IKEEXT_CIPHER_AES_256;
    customIkev2Policy->dwAuthTransformConstant = IPSEC_AUTH_SHA_256;
    customIkev2Policy->dwPfsGroup = IPSEC_PFS_2048;
    customIkev2Policy->dwDhGroup = IKEEXT_DH_GROUP_2;

    // Populate the interface custom info object
    //
    ZeroMemory(&mprInterfaceCustomInfo, sizeof(MPR_IF_CUSTOMINFOEX));

    // Set the Header info
    //
    mprInterfaceCustomInfo.Header.revision = MPRAPI_MPR_IF_CUSTOM_CONFIG_OBJECT_REVISION_2;
    mprInterfaceCustomInfo.Header.type     = MPRAPI_OBJECT_TYPE_IF_CUSTOM_CONFIG_OBJECT;
    mprInterfaceCustomInfo.Header.size     = sizeof(MPR_IF_CUSTOMINFOEX);
    mprInterfaceCustomInfo.dwFlags         = MPRAPI_IF_CUSTOM_CONFIG_FOR_IKEV2;

    // Get the machine certificate to be configured 
    // Certificate Hash is supported as multiple certificates with same name can exist on a machine
    //
    if (ERROR_SUCCESS != (status = GetCertificateNameAndHashBlob(&machineCertificate, &machineCertificateHash)))
    {
        // The method can return an error even if valid machineCertificate info is retrieved and machineCertificateHash is not.
        // Either of them is sufficient to configure the certificate on the interface 
        if(machineCertificate.cbData == 0)
        {
            wprintf(L"GetCertificateNameAndHashBlob failed\n");
            DisplayError(status);
            // we are not configuring the certficate on the interface 
        }
        // ignore the error.
    }
    
    mprInterfaceCustomInfo.customIkev2Config.dwSaLifeTime = 3600;    // 60 minutes
    mprInterfaceCustomInfo.customIkev2Config.dwSaDataSize = 10 * 1024 * 1024;    // 10 Megabyte
    mprInterfaceCustomInfo.customIkev2Config.certificateName.cbData = machineCertificate.cbData;
    mprInterfaceCustomInfo.customIkev2Config.certificateName.pbData = machineCertificate.pbData;
    mprInterfaceCustomInfo.customIkev2Config.certificateHash.cbData = machineCertificateHash.cbData;
    mprInterfaceCustomInfo.customIkev2Config.certificateHash.pbData = machineCertificateHash.pbData;
    mprInterfaceCustomInfo.customIkev2Config.customPolicy = customIkev2Policy;

    // Update the custom configuration on the interface
    //
    status = UpdateInterfaceCustomConfiguration(
        serverHandleAdmin,
        serverHandleConfig,
        interfaceHandleAdmin,
        interfaceHandleConfig,
        &mprInterfaceCustomInfo
        );
    if (ERROR_SUCCESS != status)
    {
        wprintf(L"UpdateInterfaceCustomConfiguration failed.\n");
        DisplayError(status);
        goto Done;
    }
    
Done:
    
    MPRAPI_SAMPLE_FREE(customIkev2Policy);

    if ((machineCertificate.cbData > 0) && 
        (NULL != machineCertificate.pbData))
    {
        MPRAPI_SAMPLE_FREE(machineCertificate.pbData);
    }

    if ((machineCertificateHash.cbData > 0) && 
        (NULL != machineCertificateHash.pbData))
    {
        MPRAPI_SAMPLE_FREE(machineCertificateHash.pbData);
    }
    
    return status;
}

//********************************************************************************************
// Function: DisplayDoDInterfaceConfiguration
//
// Description: Retrieves and displays the custom IPSec policies configured on the specified interface
//
//********************************************************************************************
DWORD DisplayDoDInterfaceConfiguration(
    _In_ MPR_SERVER_HANDLE serverHandleAdmin,
    _In_ HANDLE interfaceHandleAdmin
    )
{
    DWORD status = ERROR_SUCCESS;
    MPR_INTERFACE_3* ifInfo = 
NULL;
    MPR_IF_CUSTOMINFOEX interfaceCustomConfig;
    
    ZeroMemory(&interfaceCustomConfig, sizeof(MPR_IF_CUSTOMINFOEX));
    
    // Retrieve the interface info
    //
    status = g_pMprAdminInterfaceGetInfo(
        serverHandleAdmin,       // hMprServer
        interfaceHandleAdmin,         // interfaceHandleAdmin
        3,                  // dwLevel
        (LPBYTE*)&ifInfo    // lplpbBuffer
        );
    if (ERROR_SUCCESS != status)
    {
        // some genuine error, we should return
        //
        wprintf(L"MprAdminInterfaceGetInfo failed.\n");
        DisplayError(status);
        goto Done;
    }
    wprintf(L"Interface Info:\n");
    PrintInterfaceInfo(ifInfo);

    // Retrieve the interface custom configuration
    //
    interfaceCustomConfig.Header.revision = MPRAPI_MPR_IF_CUSTOM_CONFIG_OBJECT_REVISION_2;
    interfaceCustomConfig.Header.type     = MPRAPI_OBJECT_TYPE_IF_CUSTOM_CONFIG_OBJECT;
    interfaceCustomConfig.Header.size     = sizeof(MPR_IF_CUSTOMINFOEX);

    status = g_pMprAdminInterfaceGetCustomInfoEx(
        serverHandleAdmin,       // hMprServer
        interfaceHandleAdmin,         // interfaceHandleAdmin
        &interfaceCustomConfig  // pCustomInfo
        );
    if (ERROR_SUCCESS != status)
    {
        // some genuine error, we should return
        //
        wprintf(L"MprAdminInterfaceGetCustomInfoEx failed.\n");
        DisplayError(status);
        goto Done;
    }
    wprintf(L"\t Custom configuration:\n");
    if (interfaceCustomConfig.dwFlags & MPRAPI_IF_CUSTOM_CONFIG_FOR_IKEV2)
    {
        PrintInterfaceCustomConfiguration(&interfaceCustomConfig.customIkev2Config);
    }
    else
    {
        wprintf(L"\t No custom configuration available for interface '%s'\n", ifInfo->wszInterfaceName);
    }

Done:
    if (NULL != ifInfo)
    {
        g_pMprAdminBufferFree(ifInfo);
    }
    FreeInterfaceCustomConfigObject(&interfaceCustomConfig);
    
    return status;
}

//********************************************************************************************
// Function: UpdateInterfaceCustomConfiguration
//
// Description: Update the custom configuration of the speficied interface with 
//                   the supplied custom configuration information 'mprInterfaceCustomInfo'.
//
//********************************************************************************************
DWORD UpdateInterfaceCustomConfiguration(
    _In_ MPR_SERVER_HANDLE serverHandleAdmin,
    _In_ HANDLE serverHandleConfig,
    _In_ HANDLE interfaceHandleAdmin,
    _In_ HANDLE interfaceHandleConfig,
    _In_ MPR_IF_CUSTOMINFOEX* mprInterfaceCustomInfo
    )
{
    DWORD status = ERROR_SUCCESS;
    
    // Call the MprAdminInterfaceSetCustomInfoEx API to update the custom IPSec policies 
    // on the interface
    //
    status = g_pMprAdminInterfaceSetCustomInfoEx(
        serverHandleAdmin,       // hMprServer
        interfaceHandleAdmin,         // interfaceHandleAdmin
        mprInterfaceCustomInfo  // pCustomInfo
        );
    if (ERROR_SUCCESS != status)
    {
        // some genuine error, we should return
        //
        wprintf(L"MprAdminInterfaceSetCustomInfoEx failed.\n");
        DisplayError(status);
        goto Done;
    }
    
    // Call the MprAdminInterfaceSetCustomInfoEx API to persist the updated custom configuration on the 
    // the interface
    //
    status = g_pMprConfigInterfaceSetCustomInfoEx(
        serverHandleConfig,       // hMprConfig
        interfaceHandleConfig,         // hRouterInterface
        mprInterfaceCustomInfo  // pCustomInfo
        );
    if (ERROR_SUCCESS != status)
    {
        // some genuine error, we should return
        //
        wprintf(L"MprConfigInterfaceSetCustomInfoEx failed.\n");
        DisplayError(status);
        goto Done;
    }  

Done:
    return status;
}

//********************************************************************************************
// Function: PrintInterfaceCustomConfiguration
//
// Description: Prints various fields of the ROUTER_IKEv2_IF_CUSTOM_CONFIG0 structure in string format
//
//********************************************************************************************
VOID PrintInterfaceCustomConfiguration(
    _In_ ROUTER_IKEv2_IF_CUSTOM_CONFIG* customIkev2Config
    )
{
    if (customIkev2Config == NULL)
    {
        wprintf(L"\t\t NULL custom configuration\n");
        return;
    }
    wprintf(L"\t\t dwSaLifeTime: %u\n", customIkev2Config->dwSaLifeTime);
    wprintf(L"\t\t dwSaDataSize: %u\n", customIkev2Config->dwSaDataSize);
    wprintf(L"\t\t CertificateName configured: %s\n", (0 == customIkev2Config->certificateName.cbData) ? L"No" : L"Yes");
    wprintf(L"\t\t CertificateHash configured: %s\n", (0 == customIkev2Config->certificateHash.cbData) ? L"No" : L"Yes");
    if (customIkev2Config->customPolicy == NULL)
    {
        wprintf(L"\t\t IPsec custom policy not configured.\n");
    }
    else
    {
        PrintCustomIkev2Policy(L"\t\t", customIkev2Config->customPolicy);
    }
}

//********************************************************************************************
// Function: PrintInterfaceInfo
//
// Description: Prints various fields of the speficied MPR_INTERFACE_3 structure in string format 
//
//********************************************************************************************
VOID PrintInterfaceInfo(
    _In_ MPR_INTERFACE_3* ifInfo
    )
{
    WCHAR   strGUID[256] = {0};
    StringFromGUID2(ifInfo->guidId, strGUID, ARRAYSIZE(strGUID));

    wprintf(L"\t Interface name: %s\n", ifInfo->wszInterfaceName);
    wprintf(L"\t Interface GUID: %s\n", strGUID);
    wprintf(L"\t Destination: %s\n", ifInfo->szLocalPhoneNumber);
    wprintf(L"\t Interface status: %s\n", (ifInfo->fEnabled) ? L"ENABLED" : L"DISABLED");
    
    wprintf(L"\t Interface type: ");
    PrintInterfaceType(ifInfo->dwIfType);
    wprintf(L"\n");
        
    wprintf(L"\t Connection State: ");
    PrintConnectionState(ifInfo->dwConnectionState);
    wprintf(L"\n");
    
    wprintf(L"\t Encryption Type: ");
    PrintEncryptionType(ifInfo->dwEncryptionType);
    wprintf(L"\n");

    wprintf(L"\t Entry Type: ");
    PrintEntryType(ifInfo->dwType);
    wprintf(L"\n");
    
    wprintf(L"\t Vpn Strategy: ");
    PrintVpnStrategy(ifInfo->dwVpnStrategy);
    wprintf(L"\n");
    
    wprintf(L"\t UnReachabilityReasons: %u\n", ifInfo->fUnReachabilityReasons);
    wprintf(L"\t Last Error: %u\n", ifInfo->dwLastError);
    wprintf(L"\t dwfOptions: %u\n", ifInfo->dwfOptions);
    wprintf(L"\t dwfNetProtocols: %u\n", ifInfo->dwfNetProtocols);
    wprintf(L"\t DeviceType: %s\n", ifInfo->szDeviceType);
    wprintf(L"\t DeviceName: %s\n", ifInfo->szDeviceName);
    wprintf(L"\t IdleDisconnectSeconds: %u\n", ifInfo->dwIdleDisconnectSeconds);

}

//********************************************************************************************
// Function: FreeInterfaceCustomConfigObject
//
// Description: Releases the various fields of the MPR_IF_CUSTOMINFOEX
//                  structure, those are allocated by the remote access server.
//
//********************************************************************************************
VOID FreeInterfaceCustomConfigObject(
    _In_ MPR_IF_CUSTOMINFOEX* customConfig
    )
{
    if (customConfig == NULL)
        return;

    if (customConfig->dwFlags & MPRAPI_IF_CUSTOM_CONFIG_FOR_IKEV2)
    {
        if (customConfig->customIkev2Config.certificateName.cbData != 0)
        {
            g_pMprAdminBufferFree(customConfig->customIkev2Config.certificateName.pbData);
            customConfig->customIkev2Config.certificateName.cbData = 0;
            customConfig->customIkev2Config.certificateName.pbData = NULL;
        }

        if(customConfig->Header.revision == MPRAPI_MPR_IF_CUSTOM_CONFIG_OBJECT_REVISION_2 
            && customConfig->customIkev2Config.certificateHash.cbData != 0)
        {
            g_pMprAdminBufferFree(customConfig->customIkev2Config.certificateHash.pbData);
            customConfig->customIkev2Config.certificateHash.cbData = 0;
            customConfig->customIkev2Config.certificateHash.pbData = NULL;
        }

        if (customConfig->customIkev2Config.customPolicy != NULL)
        {
            g_pMprAdminBufferFree(customConfig->customIkev2Config.customPolicy);
            customConfig->customIkev2Config.customPolicy = NULL;
        }
    }
}

