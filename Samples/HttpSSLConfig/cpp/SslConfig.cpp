//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Abstract:
//
//     This sample demonstrates how to set, query, and delete a SSL
//     configuration for Hostname:Port based SSL endpoint binding(SNI)
//     or port based wildcard binding when central certificate storage(CCS)
//     is deployed.
//
//     To use this sample, certificate must be present in some store under the
//     Local Machine for a Hostname:Port based SSL endpoint binding.
//
//     SslConfig <Port> [<Hostname> <CertSubjectName> <StoreName>]
//
//     where:
//
//     Port             is port number for either hostname based or port based
//                      SSL binding.
//
//     Hostname         is the hostname for the hostname based binding.
//
//     CertSubjectName  is the subject name of the certificate to be associated
//                      with the SSL endpoint binding. This must be provided
//                      if hostname based binding is specified.
//
//     StoreName        is the store under the Local Machine where certificate
//                      is present. This must be provided if hostname based
//                      binding is specified.
//

#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4214)   // bit field types other than int

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <http.h>
#include <wincrypt.h>
#include <mstcpip.h>

//
// A unique identifier that can be used to identify this application
//

static const GUID AppId = {0xAAAABBBB, 0xCCCC, 0xDDDD, 0xEE, 0xEE, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

//
// Function prototypes.
//

_Must_inspect_result_
_Success_(return == ERROR_SUCCESS)
DWORD
ParseParameters(
    int Argc,
    _In_reads_(Argc) PWCHAR Argv[],
    _Out_ PUSHORT Port,
    _Outptr_result_maybenull_ PWSTR* Hostname,
    _Outptr_result_maybenull_ PWSTR* CertSubjectName,
    _Outptr_result_maybenull_ PWSTR* StoreName
    );

DWORD
SniConfiguration(
    USHORT Port,
    _In_ PCWSTR Hostname,
    _In_ PCWSTR CertSubjectName,
    _In_ PCWSTR StoreName
    );

DWORD
CcsConfiguration(
    USHORT Port
    );

int
__cdecl
wmain(
    int Argc,
    _In_reads_(Argc) PWCHAR Argv[]
    )
{
    DWORD Error = ERROR_SUCCESS;
    PWSTR Hostname = NULL;
    PWSTR CertSubjectName = NULL;
    USHORT Port = 0;
    PWSTR StoreName = NULL;
    HTTPAPI_VERSION Version = HTTPAPI_VERSION_2;
    BOOL ApiInitialized = FALSE;

    //
    // Get parameters from input command line.
    //

    Error = ParseParameters(Argc,
                            Argv,
                            &Port,
                            &Hostname,
                            &CertSubjectName,
                            &StoreName);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

    //
    // Initialize HTTPAPI in config mode.
    //

    Error = HttpInitialize(Version, HTTP_INITIALIZE_CONFIG, NULL);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

    ApiInitialized = TRUE;

    if (Hostname != NULL &&
        CertSubjectName != NULL &&
        StoreName != NULL)
    {
        Error = SniConfiguration(Port,
                                 Hostname,
                                 CertSubjectName,
                                 StoreName);
    }
    else
    {
        Error = CcsConfiguration(Port);
    }

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

    wprintf(L"SSL set/query/delete configuration succeed.\n");

exit:

    if (ApiInitialized)
    {
        HttpTerminate(HTTP_INITIALIZE_CONFIG, NULL);
        ApiInitialized = FALSE;
    }

    if (Error != ERROR_SUCCESS)
    {
        wprintf(L"Error code = %#lx.\n", Error);
    }

    return Error;
}

/***************************************************************************++

Routine Description:

    Get parameters from input command line.

Arguments:

    Argc - The number of parameters.

    Argv - The parameter array.

    Port - Supplies a pointer to access the port.

    Hostname - Supplies a pointer to access the hostname.

    CertSubjectName - Supplies a pointer to access the subject name of a
        certificate.

    StoreName - Supplies a pointer to access the name of the store under Local
        Machine where certificate is present.

Return Value:

    Status.

--***************************************************************************/
_Must_inspect_result_
_Success_(return == ERROR_SUCCESS)
DWORD
ParseParameters(
    int Argc,
    _In_reads_(Argc) PWCHAR Argv[],
    _Out_ PUSHORT Port,
    _Outptr_result_maybenull_ PWSTR* Hostname,
    _Outptr_result_maybenull_ PWSTR* CertSubjectName,
    _Outptr_result_maybenull_ PWSTR* StoreName
    )
{
    DWORD Error = ERROR_SUCCESS;
    DWORD TempPort = 0;
    PWSTR HostnameString = NULL;
    PWSTR PortString = NULL;
    PWSTR CertSubjectNameString = NULL;
    PWSTR StoreNameString = NULL;
    PWSTR Remaining = NULL;

    if (Argc == 2)
    {
        PortString = Argv[1];
    }
    else if (Argc == 5)
    {
        PortString = Argv[1];
        HostnameString = Argv[2];
        CertSubjectNameString = Argv[3];
        StoreNameString = Argv[4];
    }
    else
    {
        wprintf(L"Usage: SslConfig <Port> [<Hostname> <CertSubjectName> <StoreName>]\n");
        Error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    TempPort = wcstoul(PortString, &Remaining, 0);

    if (*Remaining != L'\0' ||
        TempPort == 0 ||
        TempPort > USHRT_MAX)
    {
        wprintf(L"%s is not a valid port.\n", PortString);
        Error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    //
    // Everything succeeded. Commit results.
    //

    *Hostname = HostnameString;
    *Port = (USHORT)TempPort;
    *CertSubjectName = CertSubjectNameString;
    *StoreName = StoreNameString;

    Error = ERROR_SUCCESS;

exit:

    return Error;
}

/***************************************************************************++

Routine Description:

    Get certificate hash from the certificate subject name.

Arguments:

    CertSubjectName - Subject name of the certificate to find.

    StoreName       - Name of the store under Local Machine where certificate
                      is present.

    CertHash        - Buffer to return certificate hash.

    CertHashLength  - Buffer length on input, hash length on output (element
                      count).

Return Value:

    Status.

--***************************************************************************/
DWORD
GetCertificateHash(
    _In_ PCWSTR CertSubjectName,
    _In_ PCWSTR StoreName,
    _Out_writes_bytes_to_(*CertHashLength, *CertHashLength) PBYTE CertHash,
    _Inout_ PDWORD CertHashLength
    )
{
    HCERTSTORE SystemStore = NULL;
    PCCERT_CONTEXT CertContext = NULL;
    DWORD Error = ERROR_SUCCESS;

    RtlZeroMemory(CertHash, *CertHashLength);

    //
    // Open the store under local machine.
    //

    SystemStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                                0,
                                (HCRYPTPROV_LEGACY)NULL,
                                CERT_SYSTEM_STORE_LOCAL_MACHINE,
                                StoreName);

    if (SystemStore == NULL)
    {
        Error = GetLastError();
        goto exit;
    }

    //
    // Find the certificate from the subject name and get the hash.
    //

    CertContext = CertFindCertificateInStore(SystemStore,
                                             X509_ASN_ENCODING,
                                             0,
                                             CERT_FIND_SUBJECT_STR,
                                             CertSubjectName,
                                             NULL);

    if (CertContext == NULL)
    {
        Error = GetLastError();
        goto exit;
    }

    if (!CertGetCertificateContextProperty(CertContext,
                                           CERT_HASH_PROP_ID,
                                           CertHash,
                                           CertHashLength))
    {

        Error = GetLastError();
        goto exit;
    }

exit:

    //
    // Free the certificate context.
    //

    if (CertContext != NULL)
    {
        CertFreeCertificateContext(CertContext);
    }

    //
    // Close the certificate store if it was opened.
    //

    if (SystemStore != NULL)
    {
        CertCloseStore(SystemStore, 0);
    }

    return Error;
}

/***************************************************************************++

Routine Description:

    Get certificate hash and set the SNI configuration.

Arguments:

    SniKey          - SSL endpoint key: host and port.

    CertSubjectName - Subject name of the certificate to find.

    StoreName       - Name of the store under Local Machine where certificate
                      is present.

Return Value:

    Status.

--***************************************************************************/
DWORD
SetSniConfiguration(
    _In_ PHTTP_SERVICE_CONFIG_SSL_SNI_KEY SniKey,
    _In_ PCWSTR CertSubjectName,
    _In_ PCWSTR StoreName
    )
{
    DWORD Error = ERROR_SUCCESS;
    BYTE CertHash[50] = {};
    DWORD CertHashLength = ARRAYSIZE(CertHash);
    HTTP_SERVICE_CONFIG_SSL_SNI_SET SniConfig = {};

    Error = GetCertificateHash(CertSubjectName,
                               StoreName,
                               CertHash,
                               &CertHashLength);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

    SniConfig.KeyDesc = *SniKey;
    SniConfig.ParamDesc.pSslHash = CertHash;
    SniConfig.ParamDesc.SslHashLength = CertHashLength;
    SniConfig.ParamDesc.pSslCertStoreName = (PWSTR)StoreName;
    SniConfig.ParamDesc.AppId = AppId;

    Error = HttpSetServiceConfiguration(NULL,
                                        HttpServiceConfigSslSniCertInfo,
                                        &SniConfig,
                                        sizeof(SniConfig),
                                        NULL);

exit:

    return Error;
}

/***************************************************************************++

Routine Description:

    Query the SNI configuration.

Arguments:

    SniKey - SSL endpoint key: host and port.

Return Value:

    Status.

--***************************************************************************/
DWORD
QuerySniConfiguration(
    _In_ PHTTP_SERVICE_CONFIG_SSL_SNI_KEY SniKey
    )
{
    DWORD Error = ERROR_SUCCESS;
    PHTTP_SERVICE_CONFIG_SSL_SNI_SET SniConfig = NULL;
    HTTP_SERVICE_CONFIG_SSL_SNI_QUERY SniQuery = {};
    DWORD ReturnLength = 0;

    SniQuery.KeyDesc = *SniKey;
    SniQuery.QueryDesc = HttpServiceConfigQueryExact;

    //
    // Get the size of the buffer required to query the config.
    //

    Error = HttpQueryServiceConfiguration(NULL,
                                          HttpServiceConfigSslSniCertInfo,
                                          &SniQuery,
                                          sizeof(SniQuery),
                                          NULL,
                                          0,
                                          &ReturnLength,
                                          NULL);

    if (Error != ERROR_INSUFFICIENT_BUFFER)
    {
        goto exit;
    }

    //
    // Allocate buffer and query again.
    //

    SniConfig = (PHTTP_SERVICE_CONFIG_SSL_SNI_SET)malloc(ReturnLength);

    if (SniConfig == NULL)
    {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    Error = HttpQueryServiceConfiguration(NULL,
                                          HttpServiceConfigSslSniCertInfo,
                                          &SniQuery,
                                          sizeof(SniQuery),
                                          SniConfig,
                                          ReturnLength,
                                          &ReturnLength,
                                          NULL);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

exit:

    if (SniConfig != NULL)
    {
        free(SniConfig);
        SniConfig = NULL;
    }

    return Error;
}

/***************************************************************************++

Routine Description:

    Delete the SNI configuration.

Arguments:

    SniKey - SSL endpoint key: host and port.

Return Value:

    Status.

--***************************************************************************/
DWORD
DeleteSniConfiguration(
    _In_ PHTTP_SERVICE_CONFIG_SSL_SNI_KEY SniKey
    )
{
    DWORD Error = ERROR_SUCCESS;
    HTTP_SERVICE_CONFIG_SSL_SNI_SET SniConfig = {};

    SniConfig.KeyDesc = *SniKey;

    Error = HttpDeleteServiceConfiguration(NULL,
                                           HttpServiceConfigSslSniCertInfo,
                                           &SniConfig,
                                           sizeof(SniConfig),
                                           NULL);
    return Error;
}

/***************************************************************************++

Routine Description:

    Demonstrate how to set, query, and delete the hostname based SSL configuration.

Arguments:

    Port - Port for the hostname based SSL binding.

    Hostname - Hostname for the SSL binding.

    CertSubjectName - Subject name of the certificate using in the SSL binding.

    StoreName       - Name of the store under Local Machine where certificate
                      is present.

Return Value:

    Status.

--***************************************************************************/
DWORD
SniConfiguration(
    USHORT Port,
    _In_ PCWSTR Hostname,
    _In_ PCWSTR CertSubjectName,
    _In_ PCWSTR StoreName
    )
{
    DWORD Error = ERROR_SUCCESS;
    HTTP_SERVICE_CONFIG_SSL_SNI_KEY SniKey = {};

    //
    // Create SniKey.
    // N.B. An SNI binding is IP version agnostic but for API purposes we are
    // required to specify the IP address to be (IPv4) 0.0.0.0 in the key.
    //

    SniKey.Host = (PWSTR)Hostname;
    IN4ADDR_SETANY((PSOCKADDR_IN)&SniKey.IpPort);
    SS_PORT(&SniKey.IpPort) = htons(Port);

    //
    // Create the SNI binding.
    //

    Error = SetSniConfiguration(&SniKey, CertSubjectName, StoreName);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

    //
    // Query the SNI configuration.
    //

    Error = QuerySniConfiguration(&SniKey);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

    //
    // Delete the SNI configuration.
    //

    Error = DeleteSniConfiguration(&SniKey);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

exit:

    return Error;
}

/***************************************************************************++

Routine Description:

    Create the port based SSL binding.

Arguments:

    CcsKey - CCS endpoint key.

Return Value:

    Status.

--***************************************************************************/
DWORD
SetCcsConfiguration(
    _In_ PHTTP_SERVICE_CONFIG_SSL_CCS_KEY CcsKey
    )
{
    DWORD Error = ERROR_SUCCESS;
    HTTP_SERVICE_CONFIG_SSL_CCS_SET CcsConfig = {};

    CcsConfig.KeyDesc = *CcsKey;
    CcsConfig.ParamDesc.AppId = AppId;

    Error = HttpSetServiceConfiguration(NULL,
                                        HttpServiceConfigSslCcsCertInfo,
                                        &CcsConfig,
                                        sizeof(CcsConfig),
                                        NULL);

    return Error;
}

/***************************************************************************++

Routine Description:

    Query the port based SSL binding.

Arguments:

    CcsKey - CCS endpoint key: 0.0.0.0:Port.

Return Value:

    Status.

--***************************************************************************/
DWORD
QueryCcsConfiguration(
    _In_ PHTTP_SERVICE_CONFIG_SSL_CCS_KEY CcsKey
    )
{
    DWORD Error = ERROR_SUCCESS;
    PHTTP_SERVICE_CONFIG_SSL_CCS_SET CcsConfig = NULL;
    HTTP_SERVICE_CONFIG_SSL_CCS_QUERY CcsQuery = {};
    DWORD ReturnLength = 0;

    CcsQuery.KeyDesc = *CcsKey;
    CcsQuery.QueryDesc = HttpServiceConfigQueryExact;

    //
    // Get the size of the buffer required to query the config.
    //

    Error = HttpQueryServiceConfiguration(NULL,
                                          HttpServiceConfigSslCcsCertInfo,
                                          &CcsQuery,
                                          sizeof(CcsQuery),
                                          NULL,
                                          0,
                                          &ReturnLength,
                                          NULL);

    if (Error != ERROR_INSUFFICIENT_BUFFER)
    {
        goto exit;
    }

    //
    // Allocate buffer and query again.
    //

    CcsConfig = (PHTTP_SERVICE_CONFIG_SSL_CCS_SET)malloc(ReturnLength);

    if (CcsConfig == NULL)
    {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    Error = HttpQueryServiceConfiguration(NULL,
                                          HttpServiceConfigSslCcsCertInfo,
                                          &CcsQuery,
                                          sizeof(CcsQuery),
                                          CcsConfig,
                                          ReturnLength,
                                          &ReturnLength,
                                          NULL);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

exit:

    if (CcsConfig != NULL)
    {
        free(CcsConfig);
        CcsConfig = NULL;
    }

    return Error;
}

/***************************************************************************++

Routine Description:

    Delete the port based SSL binding.

Arguments:

    CcsKey - CCS endpoint key.

Return Value:

    Status.

--***************************************************************************/
DWORD
DeleteCcsConfiguration(
    _In_ PHTTP_SERVICE_CONFIG_SSL_CCS_KEY CcsKey
    )
{
    DWORD Error = ERROR_SUCCESS;
    HTTP_SERVICE_CONFIG_SSL_CCS_SET CcsConfig = {};

    CcsConfig.KeyDesc = *CcsKey;

    Error = HttpDeleteServiceConfiguration(NULL,
                                           HttpServiceConfigSslCcsCertInfo,
                                           &CcsConfig,
                                           sizeof(CcsConfig),
                                           NULL);
    return Error;
}

/***************************************************************************++

Routine Description:

    Demonstrate how to set, query, and delete a port based SSL binding.

Arguments:

    Port            - CCS binding port.

Return Value:

    Status.

--***************************************************************************/
DWORD
CcsConfiguration(
    USHORT Port
    )
{
    DWORD Error = ERROR_SUCCESS;
    HTTP_SERVICE_CONFIG_SSL_CCS_KEY CcsKey = {};

    //
    // Create CcsKey.
    // N.B. A CCS binding is IP version agnostic but for API purposes we are
    // required to specify the IP address to be (IPv4) 0.0.0.0 in the key.
    //

    IN4ADDR_SETANY((PSOCKADDR_IN)&CcsKey.LocalAddress);
    SS_PORT(&CcsKey.LocalAddress) = htons(Port);

    //
    // Create the port based SSL binding.
    //

    Error = SetCcsConfiguration(&CcsKey);

    if (Error == ERROR_SUCCESS)
    {
        goto exit;
    }

    //
    // Query the port based SSL binding.
    //

    Error = QueryCcsConfiguration(&CcsKey);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

    //
    // Delete the port based SSL binding.
    //

    Error = DeleteCcsConfiguration(&CcsKey);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

exit:

    return Error;
}
