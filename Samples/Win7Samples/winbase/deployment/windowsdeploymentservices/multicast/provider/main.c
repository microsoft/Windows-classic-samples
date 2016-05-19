/*++

Copyright (c) 2007 Microsoft Corporation

Module Name:

    main.cpp

Abstract:

    Sample code demonstrating an implementation of the wds content provider.
    
Author:

   Blaine Young (blyoung)

Environment:

    User Mode

--*/

//
// Includes
//
#include <windows.h>
#include <stdlib.h>
#include <wdstpdi.h>

//
// Handy macros.
//

#define TO_UNICODE2(x)          L ## x
#define TO_UNICODE(x)           TO_UNICODE2(x)

#define __WFILE__               TO_UNICODE(__FILE__)
#define __WFUNCTION__           TO_UNICODE(__FUNCTION__)
#define __WFUNCSIG__            TO_UNICODE(__FUNCSIG__)

#define EXIT_ON_HR_ERROR(x) do{if(FAILED(x)){hr = x; WdsTransportServerTrace(g_hProvider, WDS_MC_TRACE_ERROR, L"%s: failing with error 0x%08x\n", __WFUNCTION__, hr); goto exit;}}while(0)
#define EXIT_ON_MALLOC_ERROR(x) do{if(NULL == x){hr = E_OUTOFMEMORY; WdsTransportServerTrace(g_hProvider, WDS_MC_TRACE_ERROR, L"%s: failing with error 0x%08x\n", __WFUNCTION__, hr);goto exit;}}while(0)
#define EXIT_ON_WIN_ERROR(x) do{if(ERROR_SUCCESS != x){hr = HRESULT_FROM_WIN32(x); WdsTransportServerTrace(g_hProvider, WDS_MC_TRACE_ERROR, L"%s: failing with error 0x%08x\n", __WFUNCTION__, hr);goto exit;}}while(0)
#define CHECK_ARG(x) do{if(NULL == x){hr = E_INVALIDARG; WdsTransportServerTrace(g_hProvider, WDS_MC_TRACE_ERROR, L"%s: failing with error 0x%08x\n", __WFUNCTION__, hr);goto exit;}}while(0)


//
// Global Instance Handle
//

HINSTANCE g_hInstance = NULL;

static HANDLE g_hProvider = NULL;

typedef struct _CALLBACK_ENTRY
{
    TRANSPORTPROVIDER_CALLBACK_ID CallbackId;
    PVOID pfnCallback;
} CALLBACK_ENTRY, *PCALLBACK_ENTRY;

//
// This provider does not implement every possible callback.  See wdstpdi.h or
// the reference documentation for a full list of valid callbacks.
//

CALLBACK_ENTRY Callbacks[] = {{WDS_TRANSPORTPROVIDER_CREATE_INSTANCE, WdsTransportProviderCreateInstance},
                              {WDS_TRANSPORTPROVIDER_COMPARE_CONTENT, WdsTransportProviderCompareContent},
                              {WDS_TRANSPORTPROVIDER_OPEN_CONTENT, WdsTransportProviderOpenContent},
                              {WDS_TRANSPORTPROVIDER_USER_ACCESS_CHECK, WdsTransportProviderUserAccessCheck},
                              {WDS_TRANSPORTPROVIDER_GET_CONTENT_SIZE, WdsTransportProviderGetContentSize},
                              {WDS_TRANSPORTPROVIDER_READ_CONTENT, WdsTransportProviderReadContent},
                              {WDS_TRANSPORTPROVIDER_CLOSE_CONTENT, WdsTransportProviderCloseContent},
                              {WDS_TRANSPORTPROVIDER_CLOSE_INSTANCE, WdsTransportProviderCloseInstance},
                              {WDS_TRANSPORTPROVIDER_SHUTDOWN, WdsTransportProviderShutdown}};
    

typedef struct _CALLBACK_DATA
{
    OVERLAPPED lpOverlapped;

    PVOID pvCallerData;

    DWORD dwReadCount;
} CALLBACK_DATA, *PCALLBACK_DATA;

BOOL
WINAPI
DllMain(
    __in HINSTANCE hInstance,
    IN DWORD dwReason,
    __in PVOID pReserved
    )
/*++

Routine Description:

    DLL main function.

Arguments:

    hInstance - Module Handle.
    dwReason - Reason for calling.
    pReserved - Reserved.

Returns:

    TRUE on success, FALSE on failure.

--*/    
{
    HRESULT hr = S_OK;
    UNREFERENCED_PARAMETER( pReserved );

    switch ( dwReason )
    {
        case DLL_PROCESS_ATTACH:
            g_hInstance = hInstance;
            break;
            
        case DLL_PROCESS_DETACH:
            break;
        default:
            break;
    }

    return !FAILED(hr);
}

VOID 
CompleteRead(
    __in DWORD dwErrorCode,
    __in DWORD dwBytesCount,
    __in PCALLBACK_DATA pData
)
/**
Routine Description:

    Read completion routine.

Arguments:

    dwErrorCode - The overall result of the read.
    dwBytesCount - The number of bytes read.
    pData - The length of the buffer pSettings, in bytes.
    
**/
{
    HRESULT hr = S_OK;

    //
    // Translate any caller errors.
    //

    if (ERROR_SUCCESS != dwErrorCode)
    {
        hr = HRESULT_FROM_WIN32(dwErrorCode);
    }

    //
    // Inform the server that the read is complete.
    //
    
    WdsTransportServerCompleteRead(g_hProvider,
                                   dwBytesCount,
                                   pData->pvCallerData,
                                   hr); 

    //
    // Free our callback data since it is no longer needed.
    //

    free(pData);
}

HRESULT
WdsTransportProviderInit(
    __in PWDS_TRANSPORTPROVIDER_INIT_PARAMS pInParameters,
    __out_bcount(ulLength) PWDS_TRANSPORTPROVIDER_SETTINGS pSettings,
    __in ULONG ulLength
    )
/*++

Routine Description:

    Initializes the content provider globally.

Arguments:

    pInParameters - Pointer to a a PWDS_TRANSPORTPROVIDER_INIT_PARAMS structure 
                    that will contain information about the multicast server and 
                    the environment that the provider is running in.  Content 
                    providers should check themselves against these setting to 
                    ensure that this is an environment that they can run in.
    pSettings - Pointer to a PWDS_TRANSPORT_PROVIDER_SETTINGS structure that the 
                content provider uses to inform the server about itself.
    ulLength - The length of the buffer pSettings, in bytes. 


Returns:

    S_OK on success, an appropriate error code on failure.

--*/      
{
    HRESULT hr = S_OK;
    ULONG i = 0;

    CHECK_ARG(pInParameters);
    CHECK_ARG(pSettings);

    //
    // Validate structures.
    //

    if (ulLength < sizeof(WDS_TRANSPORTPROVIDER_SETTINGS))
    {
        EXIT_ON_HR_ERROR(E_INVALIDARG);
    }

    if (pInParameters->ulLength < sizeof(WDS_TRANSPORTPROVIDER_INIT_PARAMS))
    {
        EXIT_ON_HR_ERROR(E_INVALIDARG);
    }

    if (pInParameters->ulMcServerVersion < MC_SERVER_CURRENT_VERSION)
    {
        EXIT_ON_HR_ERROR(E_INVALIDARG);
    }

    //
    // Initialize our submodules.
    //

    g_hProvider = pInParameters->hProvider;


    //
    // Register our callbacks with the server.
    //

    for (i = 0; i < ARRAYSIZE(Callbacks); i++)
    {
        hr = WdsTransportServerRegisterCallback(g_hProvider,
                                                Callbacks[i].CallbackId,
                                                Callbacks[i].pfnCallback);
        EXIT_ON_HR_ERROR(hr);
    }

    //
    // Inform the server of the version of the api we use.
    //

    pSettings->ulProviderVersion = TRANSPORTPROVIDER_CURRENT_VERSION;
    pSettings->ulLength = sizeof(WDS_TRANSPORTPROVIDER_SETTINGS);

exit:

    return hr;
}

HRESULT
WDSTRANSPORTPROVIDERAPI
WdsTransportProviderCreateInstance(
    __in PCWSTR pwszConfigString,
    __out PHANDLE phInstance
    )
/*++

Routine Description:

    Initializes a new provider instance.  For more information about instances, 
    please see the Instances section in the api reference.

Arguments:

    pwszConfigString - Configuration information for this instance.  
                       Configuration information may change on an instance by 
                       instance basis.  The format of this string is determined 
                       by the provider.
    phInstance - Pointer to a handle that will be used to identify this 
                 instance to the provider in future calls.  This handle must be 
                 closed with a call to WdsTransportProviderCloseInstance. 

Returns:

    S_OK on success, an appropriate error code on failure.

--*/     
{
    //
    // This provider merely opens a handle to any file that is given to it,
    // thus it does not need to accept configuration information.
    //

    return S_OK;
}

HRESULT
WDSTRANSPORTPROVIDERAPI
WdsTransportProviderCompareContent(
    __in HANDLE hInstance,
    __in PCWSTR pwszContentName,
    __in HANDLE hContent,
    __out PBOOL pbContentMatches
    )
/*++

Routine Description:

    Compares a content name/configuration string pair to an already opened 
    content handle to determine if they are the exact same.

Arguments:

    hInstance - Handle to the instance against which this stream will be opened.  
                For more information, see the Instances section in the api
                documentation.
    pwszStreamName - The name of the Stream to compare.
    hStream - An already opened Stream to compare to.
    pbStreamsMatch - If the data streams pointed to by 
                     hInstance/pwszContentName and hStream are the exact same, 
                     this variable will be set to true.  It will be set to 
                     false otherwise. 

Returns:

    S_OK on success, an appropriate error code on failure.

--*/     
{
    HRESULT hr = S_OK;

    //
    // The multicast server performs basic file name matching, so if the request
    // gets this far, then we can assume that the file has not changed due to
    // the share access we asked for.
    //

    if (NULL == pbContentMatches)
    {
        EXIT_ON_HR_ERROR(E_INVALIDARG);
    }

    *pbContentMatches = TRUE;
    
exit:

    return hr;
}

HRESULT
WDSTRANSPORTPROVIDERAPI
WdsTransportProviderUserAccessCheck(
    __in HANDLE hContent,
    __in HANDLE hUserToken,
    __out PBOOL pbAccessAllowed
    )
/*++

Routine Description:

    Allows the content provider to grant or deny access to a user based on that
    user's token.

Arguments:

    hContent - Handle to the content to check access to.
    hUserToken - Windows token of the user requesting access.
    pbAccessAllowed - Pointer that will receive whether or not the provider
                      grants access to this file.

Returns:

    S_OK on success, an appropriate error code on failure.

--*/        
{
    //
    // Grant access to the entire world.
    //

    *pbAccessAllowed = TRUE;

    return ERROR_SUCCESS;
}

HRESULT
WDSTRANSPORTPROVIDERAPI
WdsTransportProviderOpenContent(
    __in HANDLE hInstance,
    __in PCWSTR pwszContentName,
    __out PHANDLE phContent
    )
/*++

Routine Description:

    The WdsTransportProviderOpenStream opens a new static view to contents. 

Arguments:

    hInstance - Handle to the instance against which this stream will be opened.  
                For more information, see the Instances section in the api
                documentation.
    pwszContentName - The name of the data stream that the user wants 
                      multicasted.  The interpretation of this Stream name is 
                      provider specific.
    phContent - Pointer to a handle that will be used to identify this data 
                stream to the provider. 


Returns:

    S_OK on success, an appropriate error code on failure.

--*/ 
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BOOL bResult = FALSE;
    HRESULT hr = S_OK;

    //
    // Create a file for exclusive access.  For most users, this will not be
    // sufficient since it's entirely possible for successive one-off sessions 
    // to tie up a file indefinately, preventing any sort of maintanence on that
    // file.
    //

    hFile = CreateFile(pwszContentName,
                       GENERIC_READ,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                       NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        EXIT_ON_WIN_ERROR(GetLastError());
    }    

    bResult = BindIoCompletionCallback(hFile,
                                       (LPOVERLAPPED_COMPLETION_ROUTINE)CompleteRead,
                                       0);
    if (FALSE == bResult)
    {
        EXIT_ON_WIN_ERROR(GetLastError());
    }


    //
    // Inform the caller of the handle we opened.
    //

    *phContent = hFile;

exit:

    if (FAILED(hr))
    {
        if (INVALID_HANDLE_VALUE != hFile)
        {
            CloseHandle(hFile);
        }
    }

    return hr;
}

HRESULT
WDSTRANSPORTPROVIDERAPI
WdsTransportProviderGetContentSize(
    __in HANDLE hContent,
    __out PULARGE_INTEGER pContentSize
    )
/*++

Routine Description:

    Retrieves the static size of a previously opened content handle

Arguments:

    hContent - Content handle previously opened by a call to 
               WdsTransportProviderOpenStream.
    pContentSize - Pointer to a large integer that will receive the size of the 
                   contents. 

Returns:

    S_OK on success, an appropriate error code on failure.

--*/    
{
    HRESULT hr = S_OK;
    BOOL bResult = FALSE;

    LARGE_INTEGER llFileSize = {0};

    bResult = GetFileSizeEx(hContent, &llFileSize);
    if (FALSE == bResult)
    {
        EXIT_ON_WIN_ERROR(GetLastError());
    }

    pContentSize->QuadPart = llFileSize.QuadPart;
    
exit:

    return hr;
}

HRESULT
WDSTRANSPORTPROVIDERAPI
WdsTransportProviderReadContent(
    __in HANDLE hContent,
    __in_bcount(ulBytesToRead) PVOID pBuffer,
    __in ULONG ulBytesToRead,
    __in PULARGE_INTEGER pContentOffset,
    __in PVOID pvUserData
    )
/*++

Routine Description:

    Retrieves the static size of a previously opened content handle

Arguments:

    hContent - Content handle previously opened by a call to 
               WdsTransportProviderOpenStream.
    pContentSize - Pointer to a large integer that will receive the size of the 
                   contents. 

Returns:

    S_OK on success, an appropriate error code on failure.

--*/       
{
    HRESULT hr = S_OK;
    BOOL bResult = FALSE;

    DWORD dwError = 0;
    DWORD dwBytesRead = 0;
    PCALLBACK_DATA pData = NULL;

    pData = (PCALLBACK_DATA)malloc(sizeof(CALLBACK_DATA));
    EXIT_ON_MALLOC_ERROR(pData);

    ZeroMemory(pData, sizeof(CALLBACK_DATA));

    pData->lpOverlapped.Offset = pContentOffset->LowPart;
    pData->lpOverlapped.OffsetHigh = pContentOffset->HighPart;
    pData->pvCallerData = pvUserData;

    //
    // Read the file from the specified offset.
    //

    bResult = ReadFile(hContent,
                       pBuffer,
                       ulBytesToRead,
                       &pData->dwReadCount,
                       &pData->lpOverlapped);
    if (FALSE == bResult)
    {
        dwError = GetLastError();
        if (ERROR_IO_PENDING != dwError)
        {
            EXIT_ON_WIN_ERROR(dwError);
        }
    }

    //
    // Clear the callback data pointer, since it will now be freed in the read
    // complete function.
    //

    pData = NULL;
    
exit:

    if (NULL != pData)
    {
        free(pData);
    }

    return hr;
}

HRESULT
WDSTRANSPORTPROVIDERAPI
WdsTransportProviderCloseContent(
    __in HANDLE hContent
    )
/*++

Routine Description:

    Closes the data stream associated with a handle.

Arguments:

    hContent - Handle to the data stream to close.

--*/      
{
    //
    // The handle belongs to the file system, so let the file system deal with
    // it.
    //

    CloseHandle(hContent);
    return S_OK;
}

HRESULT
WDSTRANSPORTPROVIDERAPI
WdsTransportProviderCloseInstance(
    __in HANDLE hInstance
    )
/*++

Routine Description:

    Closes the instance associated with a handle.

Arguments:

    hInstance - Handle to the instance to close.

--*/       
{
    //
    // We don't keep track of instances, so noop this.
    //
    
    return S_OK;
}

HRESULT
WDSTRANSPORTPROVIDERAPI
WdsTransportProviderShutdown(
)
/*++

Routine Description:

    Globally deallocates the content provider.

--*/  
{
    //
    // We have nothing to clean up.
    //

    return S_OK;
}

