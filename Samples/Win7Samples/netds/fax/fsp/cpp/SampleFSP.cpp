//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

//
//This module implements a sample Windows NT Fax Service Provider
//

#include "SampleFSP.h"
#include <tchar.h>

HANDLE  g_HeapHandle = NULL;

// DeviceReceiveThread() is a thread to watch for an incoming fax transmission
DWORD WINAPI DeviceReceiveThread(LPDWORD pdwDeviceId);

//+---------------------------------------------------------------------------
//
//  function:   DllEntryPoint
//
//  Synopsis:   DLL entry-point function
//
//  Arguments:  [hInstance] - handle to the DLL
//                [dwReason] - specifies a flag indicating why the DLL entry-point function is being called
//
//  Returns:    TRUE on success
//
//----------------------------------------------------------------------------
BOOL WINAPI
DllEntryPoint(
                HINSTANCE  hInstance,
                DWORD      dwReason,
                LPVOID     pContext
             )
{
        // pDeviceInfo is a pointer to the virtual fax devices
        PDEVICE_INFO  pDeviceInfo;
        // pCurrentDeviceInfo is a pointer to the current virtual fax device
        PDEVICE_INFO  pCurrentDeviceInfo;

        if (dwReason == DLL_PROCESS_ATTACH) {
                // Set g_hInstance
                g_hInstance = hInstance;
                // Disable the DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications for the DLL
                DisableThreadLibraryCalls(hInstance);
        }
        else if (dwReason == DLL_PROCESS_DETACH) {
                if (g_pDeviceInfo != NULL) {
                        // Enumerate the virtual fax devices
                        for (pCurrentDeviceInfo = g_pDeviceInfo[0]; pCurrentDeviceInfo; pCurrentDeviceInfo = pDeviceInfo) {
                                pDeviceInfo = pCurrentDeviceInfo->pNextDeviceInfo;

                                if (pCurrentDeviceInfo->ExitEvent) {
                                        // Set the event to indicate the thread to watch for an incoming fax transmission is to exit
                                        SetEvent(pCurrentDeviceInfo->ExitEvent);
                                }

                                // Delete the critical section
                                DeleteCriticalSection(&pCurrentDeviceInfo->cs);
                                // Delete the virtual fax device data
                                MemFreeMacro(pCurrentDeviceInfo);
                        }
                        g_pDeviceInfo = NULL;
                }
                // Close the log file
                CloseLogFile();
        }
        return TRUE;
}

//+---------------------------------------------------------------------------
//
//  function:   FaxDevVirtualDeviceCreation
//
//  Synopsis:   The fax service calls the FaxDevVirtualDeviceCreation function during 
//                initialization to allow the fax service provider 
//                to present virtual fax devices
//
//  Arguments:  [DeviceCount] - pointer to a variable that receives the number of virtual 
//                fax devices the fax service must create for the fax service provider
//                [DeviceNamePrefix] - pointer to a variable that receives a string of the 
//                name prefix for the virtual fax devices
//                [DeviceIdPrefix] - pointer to a variable that receives a unique numeric 
//                value that identifies the virtual fax devices
//                [CompletionPort] - specifies a handle to an I/O completion port that the
//                fax service provider must use to post I/O completion port packets to the 
//                fax service for asynchronous line status events
//                [CompletionKey] - specifies a completion port key value
//
//  Returns:    TRUE on success
//
//----------------------------------------------------------------------------
BOOL WINAPI
FaxDevVirtualDeviceCreation(
                OUT LPDWORD    DeviceCount,
                OUT LPWSTR     DeviceNamePrefix,
                OUT LPDWORD    DeviceIdPrefix,
                IN  HANDLE     CompletionPort,
                IN  ULONG_PTR  CompletionKey
                )
{
        BOOL  bReturnValue;
        HRESULT hr = S_OK;

        WriteDebugString(L"---SampleFSP: FaxDevVirtualDeviceCreation Enter---\n");

        // Initialize the parameters
        *DeviceCount = 0;
        ZeroMemory(DeviceNamePrefix, 128 * sizeof(WCHAR));
        *DeviceIdPrefix = 0;

        // Copy the handle to the completion port
        g_CompletionPort = CompletionPort;
        g_CompletionKey = CompletionKey;

        // Get the registry data for the newfsp service provider
        bReturnValue = GetNewFspRegistryData(NULL,
                        NULL,
                        0,
                        NULL,
                        DeviceCount);

        if (bReturnValue == FALSE) {
                WriteDebugString(L"   ERROR: GetNewFspRegistryData Failed: 0x%08x\n", GetLastError());
                WriteDebugString(L"   ERROR: FaxDevVirtualDeviceCreation Failed\n");
                WriteDebugString(L"---SampleFSP: FaxDevVirtualDeviceCreation Exit---\n");

                return FALSE;
        }

        if (*DeviceCount == 0) {
                WriteDebugString(L"   ERROR: No Virtual Fax Devices Installed\n");
                WriteDebugString(L"   ERROR: FaxDevVirtualDeviceCreation Failed\n");
                WriteDebugString(L"---SampleFSP: FaxDevVirtualDeviceCreation Exit---\n");

                return FALSE;
        }

        // Copy the name prefix for the virtual fax devices, limited to 128 WCHARs including the termininating null character
        hr = StringCchCopy(DeviceNamePrefix, 128, NEWFSP_DEVICE_NAME_PREFIX);
        if(hr != S_OK)
        {
                WriteDebugString(L"StringCchCopy failed, hr = 0x%x for DeviceNamePrefix", hr);
                return FALSE;                
        }        
        // Copy the values that identifies the virtual fax devices
        *DeviceIdPrefix = NEWFSP_DEVICE_ID_PREFIX;

        WriteDebugString(L"---SampleFSP: FaxDevVirtualDeviceCreation Exit---\n");

        return TRUE;
}
//+---------------------------------------------------------------------------
//
//  function:   FaxLineCallback
//
//  Synopsis:   An application-defined callback function that the fax service calls
//                to deliver TAPI events to the fax service provider
//
//  Arguments:  [FaxHandle] - specifies a fax handle returned by the FaxDevStartJob function
//                [hDevice] - specifies a handle to either a line device or a call device
//                [dwMessage] - specifies a line device or a call device message
//                [dwInstance] - specifies job-specific instance data passed back to the application
//                [dwParam1] - specifies a parameter for this message
//                [dwParam2] - specifies a parameter for this message
//                [dwParam3] - specifies a parameter for this message
//  Returns:    TRUE on success
//
//----------------------------------------------------------------------------
VOID CALLBACK
FaxLineCallback(
                IN HANDLE     FaxHandle,
                IN DWORD      hDevice,
                IN DWORD      dwMessage,
                IN DWORD_PTR  dwInstance,
                IN DWORD_PTR  dwParam1,
                IN DWORD_PTR  dwParam2,
                IN DWORD_PTR  dwParam3
               )
{
        // pdwDeviceId is the pointer to the virtual fax device identifier
        LPDWORD   pdwDeviceId;
        // hThread is a handle to the thread to watch for an incoming fax transmission
        HANDLE    hThread;

        WriteDebugString(L"---SampleFSP: fnFaxLineCallback Enter---\n");

        // Wait for access to this virtual fax device
        EnterCriticalSection(&g_pDeviceInfo[hDevice - NEWFSP_DEVICE_ID_PREFIX]->cs);

        if ((dwParam1 == 0) && (g_pDeviceInfo[hDevice - NEWFSP_DEVICE_ID_PREFIX]->ExitEvent)) {
                // Receive has been disabled for this virtual fax device so set the event to indicate the thread to watch for an incoming fax transmission is to exit
                SetEvent(g_pDeviceInfo[hDevice - NEWFSP_DEVICE_ID_PREFIX]->ExitEvent);
                g_pDeviceInfo[hDevice - NEWFSP_DEVICE_ID_PREFIX]->ExitEvent = NULL;
        }
        else if ((dwParam1 != 0) && (g_pDeviceInfo[hDevice - NEWFSP_DEVICE_ID_PREFIX]->ExitEvent == NULL)) {
                // Allocate a block of memory for the virtual fax device identifier
                pdwDeviceId = (LPDWORD) MemAllocMacro(sizeof(DWORD));
                if (pdwDeviceId != NULL) {
                        // Copy the virtual fax device identifier
                        *pdwDeviceId = (hDevice - NEWFSP_DEVICE_ID_PREFIX);

                        // Receive has been enabled for this virtual fax device so create the thread to watch for an incoming fax transmission
                        g_pDeviceInfo[hDevice - NEWFSP_DEVICE_ID_PREFIX]->ExitEvent = CreateEvent(NULL,
                                        TRUE,
                                        FALSE,
                                        NULL);
                        if (g_pDeviceInfo[hDevice - NEWFSP_DEVICE_ID_PREFIX]->ExitEvent != NULL) {
                                hThread = CreateThread(NULL,
                                                0,
                                                (unsigned long (_stdcall*) (void*)) DeviceReceiveThread,
                                                pdwDeviceId,
                                                0,
                                                NULL);
                                if (hThread != NULL) {
                                        // Close the handle to the thread
                                        CloseHandle(hThread);
                                }
                                else {
                                        // Close the handle to the exit event
                                        CloseHandle(g_pDeviceInfo[hDevice - NEWFSP_DEVICE_ID_PREFIX]->ExitEvent);
                                        g_pDeviceInfo[hDevice - NEWFSP_DEVICE_ID_PREFIX]->ExitEvent = NULL;
                                }
                        }
                }
        }

        // Release access to this virtual fax device
        LeaveCriticalSection(&g_pDeviceInfo[hDevice - NEWFSP_DEVICE_ID_PREFIX]->cs);

        WriteDebugString(L"---SampleFSP: fnFaxLineCallback Exit---\n");

        return;
}
//+---------------------------------------------------------------------------
//
//  function:   FaxDevInitialize
//
//  Synopsis:   The fax service calls the FaxDevInitialize function each time the service 
//                starts to initialize communication between the fax service and the fax service provider DLL
//
//  Arguments:  [LineAppHandle] - specifies a handle to the fax service's registration with TAPI
//                [HeapHandle] - specifies a handle to a heap that the fax service provider must 
//                use for all memory allocations
//                [LineCallbackFunction] - pointer to a variable that receives a pointer to a TAPI 
//                line callback function
//                [FaxServiceCallback] - pointer to a service callback function
//
//  Returns:    TRUE on success
//
//----------------------------------------------------------------------------

BOOL WINAPI
FaxDevInitialize(
                IN  HLINEAPP               LineAppHandle,
                IN  HANDLE                 HeapHandle,
                OUT PFAX_LINECALLBACK      *LineCallbackFunction,
                IN  PFAX_SERVICE_CALLBACK  FaxServiceCallback
                )
{
        // bLoggingEnabled indicates if logging is enabled
        BOOL          bLoggingEnabled;
        // szLoggingDirectory indicates the logging directory
        WCHAR         szLoggingDirectory[MAX_PATH_LEN];
        // pDeviceInfo is a pointer to the virtual fax devices
        PDEVICE_INFO  pDeviceInfo;
        // pCurrentDeviceInfo is a pointer to the current virtual fax device
        PDEVICE_INFO  pCurrentDeviceInfo;
        // dwNumDevices is the number of virtual fax devices
        DWORD         dwNumDevices;
        // dwIndex is a counter to enumerate each virtual fax device
        DWORD         dwIndex;
        // bReturnValue is the value to return to the fax service
        BOOL          bReturnValue;

        WriteDebugString(L"---SampleFSP: FaxDevInitialize Enter---\n");

        // Set g_hLineApp
        g_LineAppHandle = LineAppHandle;

        // Set g_hHeap
        MemInitializeMacro(HeapHandle);

        // Set LineCallbackFunction
        *LineCallbackFunction = FaxLineCallback;

        // Get the registry data for the newfsp service provider
        bLoggingEnabled = FALSE;
        ZeroMemory(szLoggingDirectory, sizeof(szLoggingDirectory));
        pDeviceInfo = NULL;
        dwNumDevices = 0;
        bReturnValue = GetNewFspRegistryData(&bLoggingEnabled,
                        szLoggingDirectory,
                        sizeof(szLoggingDirectory)/sizeof(szLoggingDirectory[0]),
                        &pDeviceInfo,
                        &dwNumDevices);

        if (bReturnValue == FALSE) {
                WriteDebugString(L"   ERROR: GetNewFspRegistryData Failed: 0x%08x\n", GetLastError());
                WriteDebugString(L"   ERROR: FaxDevInitialize Failed\n");
                WriteDebugString(L"---SampleFSP: FaxDevInitialize Exit---\n");

                return FALSE;
        }

        if (dwNumDevices == 0) {
                WriteDebugString(L"   ERROR: No Virtual Fax Devices Installed\n");
                WriteDebugString(L"   ERROR: FaxDevInitialize Failed\n");
                WriteDebugString(L"---SampleFSP: FaxDevInitialize Exit---\n");

                return FALSE;
        }

        // Open the log file
        bReturnValue = OpenLogFile(bLoggingEnabled, szLoggingDirectory);

        if (dwNumDevices > 0) {
                // Allocate a block of memory for the virtual fax device data
                g_pDeviceInfo = (PDEVICE_INFO*) MemAllocMacro(sizeof(PDEVICE_INFO) * dwNumDevices);
                if (g_pDeviceInfo == NULL) {
                        // Set the error code
                        SetLastError(ERROR_NOT_ENOUGH_MEMORY);

                        // Enumerate the virtual fax devices
                        for (pCurrentDeviceInfo = pDeviceInfo; pCurrentDeviceInfo; pCurrentDeviceInfo = pDeviceInfo) {
                                // Delete the virtual fax device data
                                pDeviceInfo = pCurrentDeviceInfo->pNextDeviceInfo;
                                MemFreeMacro(pCurrentDeviceInfo);
                        }

                        WriteDebugString(L"   ERROR: FaxDevInitialize Failed: ERROR_NOT_ENOUGH_MEMORY\n");
                        WriteDebugString(L"---SampleFSP: FaxDevInitialize Exit---\n");

                        return FALSE;
                }
        }
        else {
                g_pDeviceInfo = NULL;
        }

        // Marshall the virtual fax devices
        for (pCurrentDeviceInfo = pDeviceInfo, dwIndex = 0;
                        pCurrentDeviceInfo;
                        pCurrentDeviceInfo = pCurrentDeviceInfo->pNextDeviceInfo, dwIndex++) {
                g_pDeviceInfo[dwIndex] = pCurrentDeviceInfo;

                // Initialize the virtual fax device's critical section
                InitializeCriticalSection(&g_pDeviceInfo[dwIndex]->cs);
                // Initialize the virtual fax device's status to idle
                g_pDeviceInfo[dwIndex]->Status = DEVICE_IDLE;
                // Initialize the virtual fax device's handle to the exit event
                g_pDeviceInfo[dwIndex]->ExitEvent = NULL;
                // Initialize the virtual fax device's associated fax job
                g_pDeviceInfo[dwIndex]->pJobInfo = NULL;
        }

        WriteDebugString(L"---SampleFSP: FaxDevInitialize Exit---\n");

        return TRUE;
}
//+---------------------------------------------------------------------------
//
//  function:   FaxDevStartJob
//
//  Synopsis:   The fax service calls the FaxDevStartJob function to initialize a new fax job
//
//  Arguments:   [LineHandle] - specifies a handle to the open line device associated with the fax job
//                 [DeviceId] - specifies the TAPI line device identifier associated with the fax job
//                 [FaxHandle] - pointer to a variable that receives a fax handle associated with the fax job
//                 [CompletionPortHandle] - specifies a handle to an I/O completion port
//                 [CompletionKey] - specifies a completion port key value
//
//  Returns:    void
//
//----------------------------------------------------------------------------
BOOL WINAPI
FaxDevStartJob(
                IN  HLINE      LineHandle,
                IN  DWORD      DeviceId,
                OUT PHANDLE    FaxHandle,
                IN  HANDLE     CompletionPortHandle,
                IN  ULONG_PTR  CompletionKey
              )
{
        // pJobInfo is a pointer to the fax job data
        PJOB_INFO  pJobInfo;

        WriteDebugString(L"---SampleFSP: FaxDevStartJob Enter---\n");

        // Initialize the parameters
        *FaxHandle = NULL;

        // Allocate a block of memory for the fax job instance data
        pJobInfo = (PJOB_INFO) MemAllocMacro(sizeof(JOB_INFO));
        if (pJobInfo == NULL) {
                // Set the error code
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);

                WriteDebugString(L"   ERROR: FaxDevStartJob Failed: ERROR_NOT_ENOUGH_MEMORY\n");
                WriteDebugString(L"---SampleFSP: FaxDevStartJob Exit---\n");
                return FALSE;
        }

        // Set the FaxHandle
        *FaxHandle = (PHANDLE) pJobInfo;

        // Wait for access to this virtual fax device
        EnterCriticalSection(&g_pDeviceInfo[DeviceId]->cs);

        // Initialize the fax job data
        // Set the fax job's associated virtual fax device
        pJobInfo->pDeviceInfo = g_pDeviceInfo[DeviceId];
        // Copy the handle to the I/O completion port
        pJobInfo->CompletionPortHandle = CompletionPortHandle;
        // Copy the completion port key value
        pJobInfo->CompletionKey = CompletionKey;
        // Initialize the fax job type
        pJobInfo->JobType = JOB_UNKNOWN;
        // Set the fax job status to FS_INITIALIZING
        pJobInfo->Status = FS_INITIALIZING;
        // Copy the handle to the open line device associated with the fax job
        pJobInfo->LineHandle = LineHandle;
        // Initialize the handle to the active call associated with the fax job
        pJobInfo->CallHandle = (HCALL) 0;
        // Initialize the full path to the file that contains the data stream for the fax document
        pJobInfo->FileName = NULL;
        // Initialize the name of the calling device
        pJobInfo->CallerName = NULL;
        // Initialize the telephone number of the calling device
        pJobInfo->CallerNumber = NULL;
        // Initialize name of the receiving device
        pJobInfo->ReceiverName = NULL;
        // Initialize telephone number of the receiving device
        pJobInfo->ReceiverNumber = NULL;
        // Initialize number of retries associated with the fax job
        pJobInfo->RetryCount = 0;
        // Initialize whether the fax service provider should generate a brand at the top of the fax transmission
        pJobInfo->Branding = FALSE;
        // Initialize the number of pages associated with the fax job
        pJobInfo->PageCount = 0;
        // Initialize the identifier of the remote fax device
        pJobInfo->CSI = NULL;
        // Initialize the identifier of the calling fax device
        pJobInfo->CallerId = NULL;
        // Initialize the routing string associated with the fax job
        pJobInfo->RoutingInfo = NULL;

        // Set the virtual fax device status
        g_pDeviceInfo[DeviceId]->Status = DEVICE_START;
        // Set the virtual fax device's associated fax job
        g_pDeviceInfo[DeviceId]->pJobInfo = pJobInfo;

        // Release access to this virtual fax device
        LeaveCriticalSection(&g_pDeviceInfo[DeviceId]->cs);

        WriteDebugString(L"---SampleFSP: FaxDevStartJob Exit---\n");

        return TRUE;
}
//+---------------------------------------------------------------------------
//
//  function:   FaxDevEndJob
//
//  Synopsis:   The fax service calls the FaxDevEndJob function after the last operation in a fax job
//
//  Arguments:  [FaxHandle] - specifies a fax handle returned by the FaxDevStartJob function
//
//  Returns:    TRUE on success
//
//----------------------------------------------------------------------------

BOOL WINAPI
FaxDevEndJob(
                IN HANDLE  FaxHandle
            )
{
        // pJobInfo is a pointer to the fax job data
        PJOB_INFO     pJobInfo;
        // pDeviceInfo is a pointer to the virtual fax device data
        PDEVICE_INFO  pDeviceInfo;

        WriteDebugString(L"---SampleFSP: FaxDevEndJob Enter---\n");

        if (FaxHandle == NULL) {
                // Set the error code
                SetLastError(ERROR_INVALID_HANDLE);

                WriteDebugString(L"   ERROR: FaxDevEndJob Failed: ERROR_INVALID_HANDLE\n");
                WriteDebugString(L"---SampleFSP: FaxDevEndJob Exit---\n");
                return FALSE;
        }

        // Get the fax job data from FaxHandle
        pJobInfo = (PJOB_INFO) FaxHandle;
        // Get the virtual fax device data from the fax job data
        pDeviceInfo = (PDEVICE_INFO) pJobInfo->pDeviceInfo;

        // Wait for access to this virtual fax device
        EnterCriticalSection(&pDeviceInfo->cs);

        // Free the fax job data
        // Free the full path to the file that contains the data stream for the fax document
        if (pJobInfo->FileName != NULL) {
                MemFreeMacro(pJobInfo->FileName);
        }
        // Free the name of the calling device
        if (pJobInfo->CallerName != NULL) {
                MemFreeMacro(pJobInfo->CallerName);
        }
        // Free the telephone number of the calling device
        if (pJobInfo->CallerNumber != NULL) {
                MemFreeMacro(pJobInfo->CallerNumber);
        }
        // Free name of the receiving device
        if (pJobInfo->ReceiverName != NULL) {
                MemFreeMacro(pJobInfo->ReceiverName);
        }
        // Free telephone number of the receiving device
        if (pJobInfo->ReceiverNumber != NULL) {
                MemFreeMacro(pJobInfo->ReceiverNumber);
        }
        // Free the identifier of the remote fax device
        if (pJobInfo->CSI != NULL) {
                MemFreeMacro(pJobInfo->CSI);
        }
        // Free the identifier of the calling fax device
        if (pJobInfo->CallerId != NULL) {
                MemFreeMacro(pJobInfo->CallerId);
        }
        // Free the routing string associated with the fax job
        if (pJobInfo->RoutingInfo != NULL) {
                MemFreeMacro(pJobInfo->RoutingInfo);
        }
        // Free the fax job data
        MemFreeMacro(pJobInfo);

        // Set the virtual fax device status
        pDeviceInfo->Status = DEVICE_IDLE;
        // Initialize the virtual fax device's associated fax job
        pDeviceInfo->pJobInfo = NULL;

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        WriteDebugString(L"---SampleFSP: FaxDevEndJob Exit---\n");

        return TRUE;
}
//+---------------------------------------------------------------------------
//
//  function:   FaxDevSend
//
//  Synopsis:   The fax service calls the FaxDevSend function to signal a fax service 
//                provider that it must initiate an outgoing fax transmission
//
//  Arguments:  [FaxHandle] - specifies a fax handle returned by the FaxDevStartJob function
//                [FaxSend] - pointer to a FAX_SEND structure that contains the sending information
//                [FaxSendCallback] - pointer to a callback function that notifies the fax service 
//                of the call handle that TAPI assigns
//
//  Returns:    TRUE on success
//
//----------------------------------------------------------------------------

BOOL WINAPI
FaxDevSend(
                IN HANDLE              FaxHandle,
                IN PFAX_SEND           FaxSend,
                IN PFAX_SEND_CALLBACK  FaxSendCallback
          )
{
        // pJobInfo is a pointer to the fax job data
        PJOB_INFO     pJobInfo;
        // pDeviceInfo is a pointer to the virtual fax device data
        PDEVICE_INFO  pDeviceInfo;

        // dwReceiverNumberAttributes is the file attributes of the directory specified in the telephone number of the receiving device
        DWORD         dwReceiverNumberAttributes;

        // hSourceFile is the handle to the source file
        HANDLE        hSourceFile = INVALID_HANDLE_VALUE;
        // lptstrFilePart is the filename
        LPTSTR          lptstrFilePart = NULL;
        // szFilePath is the source filename path
        WCHAR         szFilePath[MAX_PATH_LEN];
        // hDestinationFile is the handle to the destination file
        HANDLE        hDestinationFile = INVALID_HANDLE_VALUE;
        // szDestinationFilename is the destination filename
        WCHAR         szDestinationFilename[MAX_PATH_LEN + _MAX_FNAME + 6];
        // szDeviceFolder is the associated device folder.
        WCHAR         szDeviceFolder[MAX_PATH_LEN];

        // FileBytes is the bytes to be copied from the source file to the destination file
        BYTE          FileBytes[1024];
        // dwBytes is the number of bytes read from the source file
        DWORD         dwBytes;
        //iDestinationDevice is the destination device
        int iDestinationDevice;

        HRESULT hr = S_OK;
        BOOL bRetVal = FALSE;

        WriteDebugString(L"---SampleFSP: FaxDevSend Enter---\n");

        if (FaxHandle == NULL) {
                // Set the error code
                SetLastError(ERROR_INVALID_HANDLE);

                WriteDebugString(L"   ERROR: FaxDevSend Failed: ERROR_INVALID_HANDLE\n");
                WriteDebugString(L"---SampleFSP: FaxDevSend Exit---\n");
                return FALSE;
        }

        // Get the fax job data from FaxHandle
        pJobInfo = (PJOB_INFO) FaxHandle;
        // Get the virtual fax device data from the fax job data
        pDeviceInfo = (PDEVICE_INFO) pJobInfo->pDeviceInfo;

        // Wait for access to this virtual fax device
        EnterCriticalSection(&pDeviceInfo->cs);

        if (pDeviceInfo->Status == DEVICE_ABORTING) {
                goto ExitUserAbort;
        }

        // Set the virtual fax device status
        pDeviceInfo->Status = DEVICE_SEND;

        // Set the fax job type
        pJobInfo->JobType = JOB_SEND;
        // Copy the handle to the active call associated with the fax job
        pJobInfo->CallHandle = FaxSend->CallHandle;
        // Copy the full path to the file that contains the data stream for the fax document
        if (FaxSend->FileName != NULL) {
                pJobInfo->FileName = (LPWSTR) MemAllocMacro((lstrlen(FaxSend->FileName) + 1) * sizeof(WCHAR));
                if (pJobInfo->FileName) 
                {
                        hr = StringCchCopy(pJobInfo->FileName, (lstrlen(FaxSend->FileName) + 1),FaxSend->FileName);
                        if(hr != S_OK)
                        {
                                WriteDebugString(L"StringCchCopy failed, hr = 0x%x for pJobInfo->FileName", hr);
                                bRetVal = FALSE;
                                goto Exit;
                        }

                }
        }
        // Copy the name of the calling device
        if (FaxSend->CallerName != NULL) {
                pJobInfo->CallerName = (LPWSTR) MemAllocMacro((lstrlen(FaxSend->CallerName) + 1) * sizeof(WCHAR));
                if (pJobInfo->CallerName) {
                        hr = StringCchCopy(pJobInfo->CallerName, (lstrlen(FaxSend->CallerName) + 1),FaxSend->CallerName);
                        if(hr != S_OK)
                        {
                                WriteDebugString( L"StringCchCopy failed, hr = 0x%x for pJobInfo->CallerName", hr );
                                bRetVal = FALSE;
                                goto Exit;
                        }            
                }
        }
        // Copy the telephone number of the calling device
        if (FaxSend->CallerNumber != NULL) {
                pJobInfo->CallerNumber = (LPWSTR) MemAllocMacro((lstrlen(FaxSend->CallerNumber) + 1) * sizeof(WCHAR));
                if (pJobInfo->CallerNumber) {
                        hr = StringCchCopy(pJobInfo->CallerNumber, (lstrlen(FaxSend->CallerNumber) + 1),FaxSend->CallerNumber);
                        if(hr != S_OK)
                        {
                                WriteDebugString( L"StringCchCopy failed, hr = 0x%x for pJobInfo->CallerNumber", hr );
                                bRetVal = FALSE;
                                goto Exit;
                        }         
                }
        }
        // Copy the name of the receiving device
        if (FaxSend->ReceiverName != NULL) {
                pJobInfo->ReceiverName = (LPWSTR) MemAllocMacro((lstrlen(FaxSend->ReceiverName) + 1) * sizeof(WCHAR));
                if (pJobInfo->ReceiverName) {
                        hr = StringCchCopy(pJobInfo->ReceiverName, (lstrlen(FaxSend->ReceiverName) + 1),FaxSend->ReceiverName);
                        if(hr != S_OK)
                        {
                                WriteDebugString(L"StringCchCopy failed, hr = 0x%x for pJobInfo->ReceiverName", hr );
                                bRetVal = FALSE;
                                goto Exit;
                        }            
                }
        }
        // Copy the telephone number of the receiving device
        if (FaxSend->ReceiverNumber != NULL) {
                pJobInfo->ReceiverNumber = (LPWSTR) MemAllocMacro((lstrlen(FaxSend->ReceiverNumber) + 1) * sizeof(WCHAR));
                if (pJobInfo->ReceiverNumber)
                {
                        hr = StringCchCopy(pJobInfo->ReceiverNumber, (lstrlen(FaxSend->ReceiverNumber) + 1),FaxSend->ReceiverNumber);
                        if(hr != S_OK)
                        {
                                WriteDebugString( L"StringCchCopy failed, hr = 0x%x for pJobInfo->ReceiverNumber", hr );
                                bRetVal = FALSE;
                                goto Exit;
                        }            
                }
        }

        //We skip the 1st char "T" or "P" (Tone or Pulse) that the fax service added based on the dialing rule
        iDestinationDevice = _wtoi(_wcsinc(FaxSend->ReceiverNumber));
        if(iDestinationDevice > NEWFSP_DEVICE_LIMIT -1)
        {
                pJobInfo->Status = FS_BAD_ADDRESS;
                WriteDebugString( L"Receive Number is invalid. " );
                bRetVal = FALSE;
                goto Exit;
        
        }
        hr = StringCchCopy(szDeviceFolder, MAX_PATH_LEN,g_pDeviceInfo[iDestinationDevice]->Directory);
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchCopy failed, hr = 0x%x for szDeviceFolder", hr );
                bRetVal = FALSE;
                goto Exit;
        }        

        // Copy whether the fax service provider should generate a brand at the top of the fax transmission
        pJobInfo->Branding = FaxSend->Branding;

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        WriteDebugString(L"   FaxSend->SizeOfStruct   : 0x%08x\n", FaxSend->SizeOfStruct);
        WriteDebugString(L"   FaxSend->FileName       : %s\n", FaxSend->FileName);
        WriteDebugString(L"   FaxSend->CallerName     : %s\n", FaxSend->CallerName);
        WriteDebugString(L"   FaxSend->CallerNumber   : %s\n", FaxSend->CallerNumber);
        WriteDebugString(L"   FaxSend->ReceiverName   : %s\n", FaxSend->ReceiverName);
        WriteDebugString(L"   FaxSend->ReceiverNumber : %s\n", FaxSend->ReceiverNumber);
        WriteDebugString(L"   FaxSend->Branding       : %s\n", FaxSend->Branding ? L"TRUE" : L"FALSE");
        WriteDebugString(L"   FaxSend->CallHandle     : 0x%08x\n", FaxSend->CallHandle);
        WriteDebugString(L"   Dest. device Folder     : %s\n", szDeviceFolder);


        // Wait for access to this virtual fax device
        EnterCriticalSection(&pDeviceInfo->cs);

        if (pDeviceInfo->Status == DEVICE_ABORTING) {
                goto ExitUserAbort;
        }

        // Set the fax job status
        pJobInfo->Status = FS_INITIALIZING;
        // Post the FS_INITIALIZING line status event to the fax service
        PostJobStatus(pJobInfo->CompletionPortHandle,
                        pJobInfo->CompletionKey,
                        FS_INITIALIZING,
                        ERROR_SUCCESS);

        // Validate the folder of the receive device
        dwReceiverNumberAttributes = GetFileAttributes(szDeviceFolder);
        if ((dwReceiverNumberAttributes == 0xFFFFFFFF) || ((dwReceiverNumberAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)) {
                // The folder of the receive device is invalid
                goto ExitFatalError;
        }

        // Open the source file
        hSourceFile = CreateFile(FaxSend->FileName,
                        GENERIC_READ,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        if (hSourceFile == INVALID_HANDLE_VALUE) {
                // The source file failed to be opened
                goto ExitFatalError;
        }

        // Split the full path of the file that contains the data stream for the fax document

        //Separate path and file name
        DWORD dwRetVal = GetFullPathName(FaxSend->FileName,MAX_PATH_LEN, szFilePath, &lptstrFilePart); 
        if(dwRetVal == 0 )
        {
                WriteDebugString( L"GetFullPathName failed, ec = %d", GetLastError() );
                bRetVal = FALSE;
                goto Exit;
        }
        // Set the destination filename 
        hr = StringCchCopy(szDestinationFilename, MAX_PATH_LEN + _MAX_FNAME + 6,szDeviceFolder);
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchCopy failed, hr = 0x%x for szDestinationFilename szDeviceFolder", hr );
                bRetVal = FALSE;
                goto Exit;
        }    
        hr = StringCchCat(szDestinationFilename, MAX_PATH_LEN + _MAX_FNAME + 6,L"\\");
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchCat failed, hr = 0x%x for szDestinationFilename \\", hr );
                bRetVal = FALSE;
                goto Exit;
        }    

        WCHAR* strFileName = _tcschr(lptstrFilePart, '.');
        hr = StringCchCatN(szDestinationFilename, MAX_PATH_LEN + _MAX_FNAME + 6,lptstrFilePart, _tcslen(lptstrFilePart) - _tcslen(strFileName) );
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchCat failed, hr = 0x%x for szDestinationFilename \\", hr );
                bRetVal = FALSE;
                goto Exit;
        }    
        hr = StringCchCat(szDestinationFilename, MAX_PATH_LEN + _MAX_FNAME + 6,L".tif" );
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchCat failed, hr = 0x%x for szDestinationFilename \\", hr );
                bRetVal = FALSE;
                goto Exit;
        }    
        WriteDebugString( L"\n File Name %s ", szDestinationFilename );

        if ( wcslen(szDestinationFilename) >= MAX_PATH) {
                // The destination file path is too long
                goto ExitFatalError;
        }

        // Create the destination file
        hDestinationFile = CreateFile(szDestinationFilename,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        if (hDestinationFile == INVALID_HANDLE_VALUE) {
                // The destination file failed to be created
                goto ExitFatalError;
        }

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        // Wait for access to this virtual fax device
        EnterCriticalSection(&pDeviceInfo->cs);

        if (pDeviceInfo->Status == DEVICE_ABORTING) {
                goto ExitUserAbort;
        }

        // Set the fax job status
        pJobInfo->Status = FS_TRANSMITTING;
        // Post the FS_TRANSMITTING line status event to the fax service
        PostJobStatus(pJobInfo->CompletionPortHandle,
                        pJobInfo->CompletionKey,
                        FS_TRANSMITTING,
                        ERROR_SUCCESS);

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        while (TRUE) {
                // The following sleep statement slows the bit copy to a reasonable speed so that a FaxDevAbortOperation() 
                // call is possible
                Sleep(250);

                // Wait for access to this virtual fax device
                EnterCriticalSection(&pDeviceInfo->cs);

                if (pDeviceInfo->Status == DEVICE_ABORTING) {
                        goto ExitUserAbort;
                }

                // Read the bytes from the source file
                if (ReadFile(hSourceFile,
                                        FileBytes,
                                        sizeof(FileBytes),
                                        &dwBytes,
                                        NULL) == FALSE) {
                        // Failed to read the bytes from the source file
                        goto ExitFatalError;
                }

                if (dwBytes == 0) {
                        // The file pointer has reached the end of the source file
                        // Release access to this virtual fax device
                        LeaveCriticalSection(&pDeviceInfo->cs);
                        break;
                }

                // Write the bytes to the destination file
                if (WriteFile(hDestinationFile,
                                        FileBytes,
                                        dwBytes,
                                        &dwBytes,
                                        NULL) == FALSE) {
                        // Failed to write the bytes to the destination file
                        goto ExitFatalError;
                }

                // Release access to this virtual fax device
                LeaveCriticalSection(&pDeviceInfo->cs);
        }

        // Wait for access to this virtual fax device
        EnterCriticalSection(&pDeviceInfo->cs);

        if (pDeviceInfo->Status == DEVICE_ABORTING) {
                goto ExitUserAbort;
        }

        // Close the destination file
        CloseHandle(hDestinationFile);

        // Close the source file
        CloseHandle(hSourceFile);

        // Set the fax job status
        pJobInfo->Status = FS_COMPLETED;
        // Post the FS_COMPLETED line status event to the fax service
        PostJobStatus(pJobInfo->CompletionPortHandle,
                        pJobInfo->CompletionKey,
                        FS_COMPLETED,
                        ERROR_SUCCESS);

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        WriteDebugString(L"---SampleFSP: FaxDevSend Exit---\n");

        return TRUE;

ExitFatalError:
        // Set the fax job status
        pJobInfo->Status = FS_FATAL_ERROR;
        goto Exit;

ExitUserAbort:
        // Set the fax job status
        pJobInfo->Status = FS_USER_ABORT;
        goto Exit;

Exit:
        // Close and delete the destination file
        if (hDestinationFile != INVALID_HANDLE_VALUE) {
                CloseHandle(hDestinationFile);

                DeleteFile(szDestinationFilename);
        }

        // Close the source file
        if (hSourceFile != INVALID_HANDLE_VALUE) {
                CloseHandle(hSourceFile);
        }

        // Post the line status event to the fax service
        PostJobStatus(pJobInfo->CompletionPortHandle,
                        pJobInfo->CompletionKey,
                        pJobInfo->Status,
                        ERROR_SUCCESS);

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        WriteDebugString(L"---SampleFSP: FaxDevSend Exit---\n");

        return FALSE;
}

//+---------------------------------------------------------------------------
//
//  function:   FaxDevReceive
//
//  Synopsis:    The fax service calls the FaxDevReceive function to signal an incoming
//                 fax transmission to the fax service provider

//
//  Arguments:  [FaxHandle] - specifies a fax handle returned by the FaxDevStartJob function
//                [CallHandle] - specifies a TAPI call handle
//                [FaxReceive] - pointer to a FAX_RECEIVE stucture that contains information 
//                about an incoming fax document
//
//  Returns:    TRUE on success
//
//----------------------------------------------------------------------------

BOOL WINAPI
FaxDevReceive(
                IN     HANDLE        FaxHandle,
                IN     HCALL         CallHandle,
                IN OUT PFAX_RECEIVE  FaxReceive
             )
{
        // pJobInfo is a pointer to the fax job data
        PJOB_INFO        pJobInfo;
        // pDeviceInfo is a pointer to the virtual fax device data
        PDEVICE_INFO     pDeviceInfo;

        // hSourceFile is the handle to the source file
        HANDLE           hSourceFile = INVALID_HANDLE_VALUE;
        // szSourceFilename is the source filename
        WCHAR            szSourceFilename[MAX_PATH + MAX_PATH_LEN + 1];

        // hFindFile is a find file handle
        HANDLE           hFindFile = INVALID_HANDLE_VALUE;
        // FindData is a WIN32_FIND_DATA structure
        WIN32_FIND_DATA  FindData;
        // szSearchPath is the search path
        WCHAR            szSearchPath[MAX_PATH];

        // hDestinationFile is the handle to the destination file
        HANDLE           hDestinationFile = INVALID_HANDLE_VALUE;

        // FileBytes is the bytes to be copied from the source file to the destination file
        BYTE             FileBytes[1024];
        // dwBytes is the number of bytes read from the source file
        DWORD            dwBytes;
        HRESULT hr = S_OK;
        BOOL bRetVal = FALSE;

        WriteDebugString(L"---SampleFSP: FaxDevReceive Enter---\n");

        if (FaxHandle == NULL) {
                // Set the error code
                SetLastError(ERROR_INVALID_HANDLE);

                WriteDebugString(L"   ERROR: FaxDevReceive Failed: ERROR_INVALID_HANDLE\n");
                WriteDebugString(L"---SampleFSP: FaxDevReceive Exit---\n");
                return FALSE;
        }

        // Get the fax job data from FaxHandle
        pJobInfo = (PJOB_INFO) FaxHandle;
        // Get the virtual fax device data from the fax job data
        pDeviceInfo = (PDEVICE_INFO) pJobInfo->pDeviceInfo;

        // Wait for access to this virtual fax device
        EnterCriticalSection(&pDeviceInfo->cs);

        if (pDeviceInfo->Status == DEVICE_ABORTING) {
                goto ExitUserAbort;
        }

        // Set the virtual fax device status
        pDeviceInfo->Status = DEVICE_RECEIVE;

        // Set the fax job type
        pJobInfo->JobType = JOB_RECEIVE;
        // Copy the handle to the active call associated with the fax job
        pJobInfo->CallHandle = CallHandle;

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        WriteDebugString(L"   CallHandle                 : 0x%08x\n", CallHandle);
        WriteDebugString(L"   FaxReceive->SizeOfStruct   : 0x%08x\n", FaxReceive->SizeOfStruct);
        WriteDebugString(L"   FaxReceive->FileName       : %s\n", FaxReceive->FileName);
        WriteDebugString(L"   FaxReceive->ReceiverName   : %s\n", FaxReceive->ReceiverName);
        WriteDebugString(L"   FaxReceive->ReceiverNumber : %s\n", FaxReceive->ReceiverNumber);

        // Wait for access to this virtual fax device
        EnterCriticalSection(&pDeviceInfo->cs);

        if (pDeviceInfo->Status == DEVICE_ABORTING) {
                goto ExitUserAbort;
        }

        // Set the fax job status
        pJobInfo->Status = FS_ANSWERED;
        // Post the FS_ANSWERED line status event to the fax service
        PostJobStatus(pJobInfo->CompletionPortHandle,
                        pJobInfo->CompletionKey,
                        FS_ANSWERED,
                        ERROR_SUCCESS);

        // Set the search path
        hr = StringCchCopy(szSearchPath, MAX_PATH, pDeviceInfo->Directory);
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchCopy failed, hr = 0x%x for szSearchPath ", hr );
                bRetVal = FALSE;
                goto Exit;
        }    
        hr = StringCchCat(szSearchPath, MAX_PATH, L"\\*.tif");
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchCat failed, hr = 0x%x for szSearchPath ", hr );
                bRetVal = FALSE;
                goto Exit;
        }    
        // Initialize the find file data
        ZeroMemory(&FindData, sizeof(FindData));
        FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;

        // Find the incoming fax file
        hFindFile = FindFirstFile(szSearchPath, &FindData);
        if (hFindFile == INVALID_HANDLE_VALUE) {
                // The incoming fax file was not found
                goto ExitFatalError;
        }

        // Close the find file handle
        FindClose(hFindFile);
        hFindFile = INVALID_HANDLE_VALUE;

        // Set the source filename
        hr = StringCchCopy(szSourceFilename, MAX_PATH + MAX_PATH_LEN + 1, pDeviceInfo->Directory);
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchCopy failed, hr = 0x%x for szSourceFilename ", hr );
                bRetVal = FALSE;
                goto Exit;
        }    
        hr = StringCchCat(szSourceFilename, MAX_PATH + MAX_PATH_LEN + 1, L"\\");
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchCat failed, hr = 0x%x for szSourceFilename ", hr );
                bRetVal = FALSE;
                goto Exit;
        }
        hr = StringCchCat(szSourceFilename, MAX_PATH + MAX_PATH_LEN + 1, FindData.cFileName);
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchCat failed, hr = 0x%x for szSourceFilename ", hr );
                bRetVal = FALSE;
                goto Exit;
        }

        if (wcslen(szSourceFilename) >= MAX_PATH) {
                // The source file name is too long
                goto ExitFatalError;
        }

        // Open the source file
        hSourceFile = CreateFile(szSourceFilename,
                        GENERIC_READ,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        if (hSourceFile == INVALID_HANDLE_VALUE) {
                // The source file failed to be opened
                goto ExitFatalError;
        }

        // Open the destination file
        hDestinationFile = CreateFile(FaxReceive->FileName,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
        if (hDestinationFile == INVALID_HANDLE_VALUE) {
                // The destination file failed to be created
                goto ExitFatalError;
        }

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        // Wait for access to this virtual fax device
        EnterCriticalSection(&pDeviceInfo->cs);

        if (pDeviceInfo->Status == DEVICE_ABORTING) {
                goto ExitUserAbort;
        }

        // Set the fax job status
        pJobInfo->Status = FS_RECEIVING;
        // Post the FS_RECEIVING line status event to the fax service
        PostJobStatus(pJobInfo->CompletionPortHandle,
                        pJobInfo->CompletionKey,
                        FS_RECEIVING,
                        ERROR_SUCCESS);

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        while (TRUE) {
                // The following sleep statement slows the bit copy to a reasonable speed so that a FaxDevAbortOperation() call is possible
                Sleep(250);

                // Wait for access to this virtual fax device
                EnterCriticalSection(&pDeviceInfo->cs);

                if (pDeviceInfo->Status == DEVICE_ABORTING) {
                        goto ExitUserAbort;
                }

                // Read the bytes from the source file
                if (ReadFile(hSourceFile,
                                        FileBytes,
                                        sizeof(FileBytes),
                                        &dwBytes,
                                        NULL) == FALSE) {
                        // Failed to read the bytes from the source file
                        goto ExitFatalError;
                }

                if (dwBytes == 0) {
                        // The file pointer has reached the end of the source file
                        // Release access to this virtual fax device
                        LeaveCriticalSection(&pDeviceInfo->cs);
                        break;
                }

                // Write the bytes to the destination file
                if (WriteFile(hDestinationFile,
                                        FileBytes,
                                        dwBytes,
                                        &dwBytes,
                                        NULL) == FALSE) {
                        // Failed to write the bytes to the destination file
                        goto ExitFatalError;
                }

                // Release access to this virtual fax device
                LeaveCriticalSection(&pDeviceInfo->cs);
        }

        // Wait for access to this virtual fax device
        EnterCriticalSection(&pDeviceInfo->cs);

        if (pDeviceInfo->Status == DEVICE_ABORTING) {
                goto ExitUserAbort;
        }

        // Close the destination file
        CloseHandle(hDestinationFile);

        // Close the source file
        CloseHandle(hSourceFile);

        //Source file is now copied, delete the source file
        DeleteFile(szSourceFilename);

        // Set the fax job status
        pJobInfo->Status = FS_COMPLETED;
        // Post the FS_COMPLETED line status event to the fax service
        PostJobStatus(pJobInfo->CompletionPortHandle,
                        pJobInfo->CompletionKey,
                        FS_COMPLETED,
                        ERROR_SUCCESS);

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        WriteDebugString(L"---SampleFSP: FaxDevReceive Exit---\n");

        return TRUE;

ExitFatalError:
        // Set the fax job status
        pJobInfo->Status = FS_FATAL_ERROR;
        goto Exit;

ExitUserAbort:
        // Set the fax job status
        pJobInfo->Status = FS_USER_ABORT;
        goto Exit;

Exit:
        // Close the destination file
        if (hDestinationFile != INVALID_HANDLE_VALUE) {
                CloseHandle(hDestinationFile);
        }

        // Close the source file
        if (hSourceFile != INVALID_HANDLE_VALUE) {
                CloseHandle(hSourceFile);
        }

        // Post the line status event to the fax service
        PostJobStatus(pJobInfo->CompletionPortHandle,
                        pJobInfo->CompletionKey,
                        pJobInfo->Status,
                        ERROR_SUCCESS);

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        WriteDebugString(L"---SampleFSP: FaxDevReceive Exit---\n");

        return FALSE;
}
//+---------------------------------------------------------------------------
//
//  function:   FaxDevReportStatus
//
//  Synopsis:   The fax service calls the FaxDevReportStatus function to query a fax 
//                service provider for status information about an individual active 
//                fax operation or for status information after a failed fax operation
//
//  Arguments:  [FaxHandle] - specifies a fax handle returned by the FaxDevStartJob function
//                [FaxStatus] - pointer to a FAX_DEV_STATUS structure that receives status and 
//                identification information
//                [FaxStatusSize] - specifies the size, in bytes, of the buffer pointer to by 
//                the FaxStatus parameter
//                [FaxStatusSizeRequired] - pointer to a variable that receives the calculated 
//                size, in bytes, of the buffer required to hold the FAX_DEV_STATUS structure
//
//  Returns:    TRUE on success
//
//----------------------------------------------------------------------------

BOOL WINAPI
FaxDevReportStatus(
                IN  HANDLE           FaxHandle OPTIONAL,
                OUT PFAX_DEV_STATUS  FaxStatus,
                IN  DWORD            FaxStatusSize,
                OUT LPDWORD          FaxStatusSizeRequired
                )
{
        // pJobInfo is a pointer to the fax job data
        PJOB_INFO     pJobInfo;
        // pDeviceInfo is a pointer to the virtual fax device data
        PDEVICE_INFO  pDeviceInfo;
        // dwSize is the size of the completion packet
        DWORD         dwSize;
        // upString is the offset of the strings within the completion packet
        UINT_PTR      upStringOffset;
        HRESULT hr = S_OK;
        BOOL bRetVal = FALSE;

        WriteDebugString(L"---SampleFSP: FaxDevReportStatus Enter---\n");

        if (FaxHandle == NULL) {
                // Set the error code
                SetLastError(ERROR_INVALID_HANDLE);

                WriteDebugString(L"   ERROR: FaxDevReportStatus Failed: ERROR_INVALID_HANDLE\n");
                WriteDebugString(L"---SampleFSP: FaxDevReportStatus Exit---\n");
                return FALSE;
        }

        if (FaxStatusSizeRequired == NULL) {
                // Set the error code
                SetLastError(ERROR_INVALID_PARAMETER);

                WriteDebugString(L"   ERROR: FaxDevReportStatus Failed: ERROR_INVALID_PARAMETER\n");
                WriteDebugString(L"---SampleFSP: FaxDevReportStatus Exit---\n");
                return FALSE;
        }

        if ((FaxStatus == NULL) && (FaxStatusSize != 0)) {
                // Set the error code
                SetLastError(ERROR_INVALID_PARAMETER);

                WriteDebugString(L"   ERROR: FaxDevReportStatus Failed: ERROR_INVALID_PARAMETER\n");
                WriteDebugString(L"---SampleFSP: FaxDevReportStatus Exit---\n");
                return FALSE;
        }

        // Get the fax job data from FaxHandle
        pJobInfo = (PJOB_INFO) FaxHandle;
        // Get the virtual fax device data from the fax job data
        pDeviceInfo = (PDEVICE_INFO) pJobInfo->pDeviceInfo;

        // Wait for access to this virtual fax device
        EnterCriticalSection(&pDeviceInfo->cs);

        // Initialize the size of the completion packet
        dwSize = sizeof(FAX_DEV_STATUS);
        if (pJobInfo->CSI != NULL) {
                // Increase the size of the completion packet for the remote fax device indentifier
                dwSize += (lstrlen(pJobInfo->CSI) + 1) * sizeof(WCHAR);
        }
        if (pJobInfo->CallerId != NULL) {
                // Increase the size of the completion packet for the calling fax device identifier
                dwSize += (lstrlen(pJobInfo->CallerId) + 1) * sizeof(WCHAR);
        }
        if (pJobInfo->RoutingInfo != NULL) {
                // Increase the size of the completion packet for the routing string
                dwSize += (lstrlen(pJobInfo->RoutingInfo) + 1) * sizeof(WCHAR);
        }

        // Set the calculated size of the buffer required to hold the completion packet
        *FaxStatusSizeRequired = dwSize;

        if ((FaxStatus == NULL) && (FaxStatusSize == 0)) {
                // Release access to this virtual fax device
                LeaveCriticalSection(&pDeviceInfo->cs);
                WriteDebugString(L"---SampleFSP: FaxDevReportStatus Exit---\n");
                return TRUE;
        }

        if (FaxStatusSize < dwSize) {
                // Set the error code
                SetLastError(ERROR_INSUFFICIENT_BUFFER);

                // Release access to this virtual fax device
                LeaveCriticalSection(&pDeviceInfo->cs);

                WriteDebugString(L"   ERROR: FaxDevReportStatus Failed: ERROR_INSUFFICIENT_BUFFER\n");
                WriteDebugString(L"---SampleFSP: FaxDevReportStatus Exit---\n");
                return FALSE;
        }

        // Initialize upStringOffset
        upStringOffset = sizeof(FAX_DEV_STATUS);

        // Set the completion packet's structure size
        FaxStatus->SizeOfStruct = sizeof(FAX_DEV_STATUS);
        // Copy the completion packet's fax status identifier
        FaxStatus->StatusId = pJobInfo->Status;
        // Set the completion packet's string resource identifier to 0
        FaxStatus->StringId = 0;
        // Copy the completion packet's current page number
        FaxStatus->PageCount = pJobInfo->PageCount;
        // Copy the completion packet's remote fax device identifier
        if (pJobInfo->CSI != NULL) {
                FaxStatus->CSI = (LPWSTR) ((UINT_PTR) FaxStatus + upStringOffset);
                hr = StringCchCopy(FaxStatus->CSI, (lstrlen(pJobInfo->CSI) + 1) ,pJobInfo->CSI);
                if(hr != S_OK)
                {
                        WriteDebugString(L"StringCchCopy failed, hr = 0x%x for FaxStatus->CSI", hr);
                        LeaveCriticalSection(&pDeviceInfo->cs);
                        return FALSE;
                }                
                upStringOffset += (lstrlen(pJobInfo->CSI) + 1) * sizeof(WCHAR);
        }
        // Set the completion packet's calling fax device identifier to NULL
        FaxStatus->CallerId = NULL;
        if (pJobInfo->CallerId != NULL) {
                FaxStatus->CallerId = (LPWSTR) ((UINT_PTR) FaxStatus + upStringOffset);
                hr = StringCchCopy(FaxStatus->CallerId, (lstrlen(pJobInfo->CallerId) + 1) ,pJobInfo->CallerId);
                if(hr != S_OK)
                {
                        WriteDebugString(L"StringCchCopy failed, hr = 0x%x for FaxStatus->CallerId", hr);
                        LeaveCriticalSection(&pDeviceInfo->cs);
                        return FALSE;
                }                
                upStringOffset += (lstrlen(pJobInfo->CallerId) + 1) * sizeof(WCHAR);
        }
        // Set the completion packet's routing string to NULL
        FaxStatus->RoutingInfo = NULL;
        if (pJobInfo->RoutingInfo != NULL) {
                FaxStatus->RoutingInfo = (LPWSTR) ((UINT_PTR) FaxStatus + upStringOffset);
                hr = StringCchCopy(FaxStatus->RoutingInfo, (lstrlen(pJobInfo->RoutingInfo) + 1) ,pJobInfo->RoutingInfo);
                if(hr != S_OK)
                {
                        WriteDebugString(L"StringCchCopy failed, hr = 0x%x for FaxStatus->RoutingInfo", hr);
                        LeaveCriticalSection(&pDeviceInfo->cs);
                        return FALSE;
                }                
                upStringOffset += (lstrlen(pJobInfo->RoutingInfo) + 1) * sizeof(WCHAR);
        }
        // Copy the completion packet's Win32 error code
        FaxStatus->ErrorCode = ERROR_SUCCESS;

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        WriteDebugString(L"---SampleFSP: FaxDevReportStatus Exit---\n");

        return TRUE;
}

//+---------------------------------------------------------------------------
//
//  function:   FaxDevAbortOperation
//
//  Synopsis:   The fax service calls the FaxDevAbortOperation function to request 
//                that the fax service provider terminate the active fax operation for 
//                the fax job specified by the FaxHandle parameter

//  Arguments:  [FaxHandle] - specifies a fax handle returned by the FaxDevStartJob function
//
//  Returns:    TRUE on success
//
//----------------------------------------------------------------------------

BOOL WINAPI
FaxDevAbortOperation(
                IN HANDLE  FaxHandle
                )
{
        // pJobInfo is a pointer to the fax job data
        PJOB_INFO     pJobInfo;
        // pDeviceInfo is a pointer to the virtual fax device data
        PDEVICE_INFO  pDeviceInfo;

        WriteDebugString(L"---SampleFSP: FaxDevAbortOperation Enter---\n");

        if (FaxHandle == NULL) {
                // Set the error code
                SetLastError(ERROR_INVALID_HANDLE);

                WriteDebugString(L"   ERROR: FaxDevAbortOperation Failed: ERROR_INVALID_HANDLE\n");
                WriteDebugString(L"---SampleFSP: FaxDevAbortOperation Exit---\n");
                return FALSE;
        }

        // Get the fax job data from FaxHandle
        pJobInfo = (PJOB_INFO) FaxHandle;
        // Get the virtual fax device data from the fax job data
        pDeviceInfo = (PDEVICE_INFO) pJobInfo->pDeviceInfo;

        // Wait for access to this virtual fax device
        EnterCriticalSection(&pDeviceInfo->cs);

        // Set the virtual fax device status
        pDeviceInfo->Status = DEVICE_ABORTING;

        // Release access to this virtual fax device
        LeaveCriticalSection(&pDeviceInfo->cs);

        WriteDebugString(L"---SampleFSP: FaxDevAbortOperation Exit---\n");

        return TRUE;
}

//+---------------------------------------------------------------------------
//
//  function:   DeviceReceiveThread
//
//  Synopsis:   Thread to watch for an incoming fax transmission
//
//  Arguments:  [pdwDeviceId] - pointer to the virtual fax device identifier
//
//  Returns:    DWORD
//
//----------------------------------------------------------------------------
DWORD WINAPI DeviceReceiveThread(LPDWORD pdwDeviceId)       
{
        // hChangeNotification is a handle to a notification change in a directory
        HANDLE         hChangeNotification;
        // hWaitObjects are the handles to the wait objects
        HANDLE         hWaitObjects[2];
        // dwDeviceId is the virtual fax device identifier
        DWORD          dwDeviceId;
        // pLineMessage is a pointer to LINEMESSAGE structure to signal an incoming fax transmission to the fax service
        LPLINEMESSAGE  pLineMessage;

        WriteDebugString(L"---SampleFSP: DeviceReceiveThread Enter---\n");

        // Copy the virtual fax device identifier
        dwDeviceId = *pdwDeviceId;
        MemFreeMacro(pdwDeviceId);

        // Create the change notification handle
        hChangeNotification = FindFirstChangeNotification(g_pDeviceInfo[dwDeviceId]->Directory,
                        FALSE, 
                        FILE_NOTIFY_CHANGE_ATTRIBUTES);
        if (hChangeNotification == INVALID_HANDLE_VALUE) {
                goto Exit;
        }

        // Wait for access to this virtual fax device
        EnterCriticalSection(&g_pDeviceInfo[dwDeviceId]->cs);

        // Set hWaitObjects
        hWaitObjects[0] = g_pDeviceInfo[dwDeviceId]->ExitEvent;
        hWaitObjects[1] = hChangeNotification;

        // Release access to this virtual fax device
        LeaveCriticalSection(&g_pDeviceInfo[dwDeviceId]->cs);

        while (TRUE) {
                // Wait for the exit event or notification change to be signaled
                if ( WaitForMultipleObjects(2,
                                        hWaitObjects,
                                        FALSE,
                                        INFINITE) == WAIT_OBJECT_0) {
                        break;
                }

                // Wait for access to this virtual fax device
                EnterCriticalSection(&g_pDeviceInfo[dwDeviceId]->cs);

                if (g_pDeviceInfo[dwDeviceId]->Status == DEVICE_IDLE) {
                        // Allocate a block of memory for the completion packet
                        pLineMessage = (LPLINEMESSAGE) LocalAlloc(LPTR, sizeof(LINEMESSAGE));
                        if (pLineMessage != NULL) {
                                // Initialize the completion packet
                                // Set the completion packet's handle to the virtual fax device
                                pLineMessage->hDevice = dwDeviceId + NEWFSP_DEVICE_ID_PREFIX;
                                // Set the completion packet's virtual fax device message
                                pLineMessage->dwMessageID = 0;
                                // Set the completion packet's instance data
                                pLineMessage->dwCallbackInstance = 0;
                                // Set the completion packet's first parameter
                                pLineMessage->dwParam1 = LINEDEVSTATE_RINGING;
                                // Set the completion packet's second parameter
                                pLineMessage->dwParam2 = 0;
                                // Set the completion packet's third parameter
                                pLineMessage->dwParam3 = 0;

                                WriteDebugString(L"---SampleFSP: DeviceReceiveThread Signaling Fax Service...---\n");

                                // Post the completion packet
                                PostQueuedCompletionStatus(g_CompletionPort, 
                                                sizeof(LINEMESSAGE),
                                                g_CompletionKey,
                                                (LPOVERLAPPED) pLineMessage);
                        }
                }

                // Release access to this virtual fax device
                LeaveCriticalSection(&g_pDeviceInfo[dwDeviceId]->cs);

                // Find the next notification change
                FindNextChangeNotification(hChangeNotification);
        }

Exit:
        if (hChangeNotification != INVALID_HANDLE_VALUE) {
                // Close the handle to the change notification
                FindCloseChangeNotification(hChangeNotification);
        }

        // Close the handle to the exit event
        CloseHandle(hWaitObjects[0]);

        WriteDebugString(L"---SampleFSP: DeviceReceiveThread Exit---\n");

        return 0;
}

//+---------------------------------------------------------------------------
//
//  function:   DllRegisterServer
//
//  Synopsis:   Function for the in-process server to create its registry entries
//
//  Arguments:  void
//
//  Returns:    S_OK on success
//
//----------------------------------------------------------------------------
STDAPI DllRegisterServer()        
{
        // hModWinfax is the handle to the winfax module
        HMODULE                       hModWinfax;

        // szCurrentDirectory is the name of the current directory
        WCHAR                        szCurrentDirectory[MAX_PATH_LEN];
        // szCurrentFilename is the name of the current filename
        WCHAR                        szCurrentFilename[_MAX_FNAME];
        // szDestinationFilename is the name of the destination filename
        WCHAR                        szDestinationFilename[MAX_PATH];
        // szWinFaxDllLocation is the location of the winfax.dll
        WCHAR szWinFaxDllLocation[MAX_PATH+12]={0};

        // pFaxRegisterServiceProvider is a pointer to the FaxRegisterServiceProvider() winfax api
        PFAXREGISTERSERVICEPROVIDER  pFaxRegisterServiceProvider;

        // pDeviceInfo is a pointer to the virtual fax devices
        PDEVICE_INFO                 pDeviceInfo;
        // pCurrentDeviceInfo is a pointer to the current virtual fax device
        PDEVICE_INFO                 pCurrentDeviceInfo;
        // dwIndex is a counter to enumerate each virtual fax device
        DWORD                        dwIndex;

        BOOL bRetVal = FALSE;
        HRESULT hr = S_OK;
        // Open the log file
        OpenLogFile(FALSE, NULL);

        WriteDebugString(L"---SampleFSP: DllRegisterServer Enter---\n");

        // Get the current directory
        if (GetCurrentDirectory(MAX_PATH_LEN, szCurrentDirectory) == 0) {
                WriteDebugString(L"   ERROR: GetCurrentDirectory Failed: 0x%08x\n", GetLastError());
                WriteDebugString(L"   ERROR: DllRegisterServer Failed\n");
                WriteDebugString(L"---SampleFSP: DllRegisterServer Exit---\n");

                // Close the log file
                CloseLogFile();

                return E_UNEXPECTED;
        }
        // Set the current filename

        hr = StringCchCopy(szCurrentFilename, _MAX_FNAME,szCurrentDirectory);
        if(hr != S_OK)
        {
                WriteDebugString(L"StringCchCopy failed, hr = 0x%x for szCurrentFilename", hr);
                return hr;
        }
        hr = StringCchCat(szCurrentFilename, _MAX_FNAME,L"\\SampleFSP.dll");
        if(hr != S_OK)
        {
                WriteDebugString(L"StringCchCat failed, hr = 0x%x for szCurrentFilename ", hr);
                return hr;
        }
        // Get the destination filename
        if (ExpandEnvironmentStrings(NEWFSP_PROVIDER_IMAGENAME, szDestinationFilename, MAX_PATH) == 0) {
                WriteDebugString(L"   ERROR: ExpandEnvironmentStrings Failed: 0x%08x\n", GetLastError());
                WriteDebugString(L"   ERROR: DllRegisterServer Failed\n");
                WriteDebugString(L"---SampleFSP: DllRegisterServer Exit---\n");

                // Close the log file
                CloseLogFile();

                return E_UNEXPECTED;
        }

        if (lstrcmpi(szDestinationFilename, szCurrentFilename) != 0) {
                // Copy the current filename to the destination filename
                if (CopyFile(L"SampleFSP.dll", szDestinationFilename, FALSE) == FALSE) {
                        WriteDebugString(L"   ERROR: CopyFile Failed: 0x%08x\n", GetLastError());
                        WriteDebugString(L"   ERROR: DllRegisterServer Failed\n");
                        WriteDebugString(L"---SampleFSP: DllRegisterServer Exit---\n");

                        // Close the log file
                        CloseLogFile();

                        return E_UNEXPECTED;
                }
        }

        //we assume that winfax.dll is located in SystemDirectory path

        if (GetSystemDirectory(szWinFaxDllLocation, MAX_PATH+1) == 0)
        {
                WriteDebugString( L"GetSystemDirectory failed, ec = %d\n", GetLastError() );
                // Close the log file
                CloseLogFile();
                return E_UNEXPECTED;       
        }

        hr = StringCchCat(szWinFaxDllLocation,MAX_PATH+12,L"\\winfax.dll" );
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchCat failed, hr = 0x%x for szWinFaxDllLocation", hr );
                return hr;
        }

        if (wcslen(szWinFaxDllLocation) >= MAX_PATH) {
                WriteDebugString(L"   ERROR: Winfax.dll location is too long\n");
                // Close the log file
                CloseLogFile();

                return E_UNEXPECTED;
        }

        // Load the winfax dll
        hModWinfax = LoadLibrary( szWinFaxDllLocation );
        if (hModWinfax == NULL) {
                WriteDebugString(L"   ERROR: LoadLibrary Failed: 0x%08x\n", GetLastError());
                WriteDebugString(L"   ERROR: DllRegisterServer Failed\n");
                WriteDebugString(L"---SampleFSP: DllRegisterServer Exit---\n");

                // Close the log file
                CloseLogFile();

                return E_UNEXPECTED;
        }

        pFaxRegisterServiceProvider = (PFAXREGISTERSERVICEPROVIDER) GetProcAddress(hModWinfax, "FaxRegisterServiceProviderW");
        if (pFaxRegisterServiceProvider == NULL) {
                WriteDebugString(L"   ERROR: GetProcAddress Failed: 0x%08x\n", GetLastError());
                WriteDebugString(L"   ERROR: DllRegisterServer Failed\n");

                FreeLibrary(hModWinfax);

                WriteDebugString(L"---SampleFSP: DllRegisterServer Exit---\n");

                // Close the log file
                CloseLogFile();

                return E_UNEXPECTED;
        }

        // Register the fax service provider
        if (pFaxRegisterServiceProvider(NEWFSP_PROVIDER,
                                NEWFSP_PROVIDER_FRIENDLYNAME,
                                NEWFSP_PROVIDER_IMAGENAME,
                                NEWFSP_PROVIDER_PROVIDERNAME) == FALSE) {
                WriteDebugString(L"   ERROR: FaxRegisterServiceProvider Failed: 0x%08x\n", GetLastError());
                WriteDebugString(L"   ERROR: DllRegisterServer Failed\n");

                FreeLibrary(hModWinfax);

                WriteDebugString(L"---SampleFSP: DllRegisterServer Exit---\n");

                // Close the log file
                CloseLogFile();

                return E_UNEXPECTED;
        }

        FreeLibrary(hModWinfax);

        // Set g_hHeap
        MemInitializeMacro(GetProcessHeap());

        // Create the virtual fax devices
        for (dwIndex = 0, pCurrentDeviceInfo = NULL, pDeviceInfo = NULL; dwIndex < NEWFSP_DEVICE_LIMIT; dwIndex++) {
                // Allocate a block of memory for the virtual fax device data
                if (pCurrentDeviceInfo == NULL) {
                        pCurrentDeviceInfo = (PDEVICE_INFO) MemAllocMacro(sizeof(DEVICE_INFO));
                        if (pCurrentDeviceInfo == NULL) {
                                continue;
                        }
                }
                else {
                        pCurrentDeviceInfo->pNextDeviceInfo = (PDEVICE_INFO) MemAllocMacro(sizeof(DEVICE_INFO));
                        if (pCurrentDeviceInfo->pNextDeviceInfo == NULL) {
                                continue;
                        }

                        // Set the pointer to the current virtual fax device
                        pCurrentDeviceInfo = pCurrentDeviceInfo->pNextDeviceInfo;
                }

                // Set the indentifier of the virtual fax device
                pCurrentDeviceInfo->DeviceId = dwIndex;
                // Set the virtual fax device's incoming fax directory to the current directory

                hr = StringCchCopy(pCurrentDeviceInfo->Directory,MAX_PATH_LEN,szCurrentDirectory );
                if(hr != S_OK)
                {
                        WriteDebugString( L"StringCchCopy failed, hr = 0x%x for pCurrentDeviceInfo->Directory", hr );
                        return hr;
                }
                //lstrcpy(pCurrentDeviceInfo->Directory, szCurrentDirectory);

                if (pDeviceInfo == NULL) {
                        // Set the pointer to the virtual fax devices
                        pDeviceInfo = pCurrentDeviceInfo;
                }
        }

        // Set the registry data for the newfsp service provider
        SetNewFspRegistryData(FALSE, szCurrentDirectory, pDeviceInfo);

        // Enumerate the virtual fax devices
        for (pCurrentDeviceInfo = pDeviceInfo; pCurrentDeviceInfo; pCurrentDeviceInfo = pDeviceInfo) {
                // Delete the virtual fax device data
                pDeviceInfo = pCurrentDeviceInfo->pNextDeviceInfo;
                MemFreeMacro(pCurrentDeviceInfo);
        }

        WriteDebugString(L"---SampleFSP: DllRegisterServer Exit---\n");

        // Close the log file
        CloseLogFile();

        return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   DllUnregisterServer
//
//  Synopsis:   Function for the in-process server to delete its registry entries
//
//  Arguments:  VOID
//
//  Returns:    S_OK on success
//
//----------------------------------------------------------------------------
STDAPI DllUnregisterServer()
{
        // hModWinfax is the handle to the winfax module
        HMODULE                       hModWinfax;
        // hServiceProvidersKey is the handle to the fax service providers registry key
        HKEY          hServiceProvidersKey = NULL;
        // hNewFspKey is the handle to the newfsp service provider registry key
        HKEY          hNewFspKey = NULL;
        // hNewFspKey is the handle to the newfsp service provider registry key
        HKEY          hDevicesKey = NULL;

        // szWinFaxDllLocation is the location of the winfax.dll
        WCHAR szWinFaxDllLocation[MAX_PATH+12]={0};

        // pFaxRegisterServiceProvider is a pointer to the FaxRegisterServiceProvider() winfax api
        PFAXUNREGISTERSERVICEPROVIDER  pFaxUnregisterServiceProvider;

        BOOL bRetVal = FALSE;
        DWORD dwIndex = 0;
        HRESULT hr = S_OK;
        WCHAR strIndex[20] = {0};
        // Open the log file
        OpenLogFile(FALSE, NULL);

        WriteDebugString(L"---SampleFSP: DllUnregisterServer Enter---\n");
        //we assume that winfax.dll is located in SystemDirectory path
        if (GetSystemDirectory(szWinFaxDllLocation, MAX_PATH+1) == 0)
        {
                WriteDebugString( L"GetSystemDirectory failed, ec = %d\n", GetLastError() );
                // Close the log file
                CloseLogFile();
                return E_UNEXPECTED;       
        }

        hr = StringCchCat(szWinFaxDllLocation,MAX_PATH+12,L"\\winfax.dll" );
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchCat failed, hr = 0x%x for szWinFaxDllLocation", hr );
                return hr;
        }

        if (wcslen(szWinFaxDllLocation) >= MAX_PATH) {
                WriteDebugString(L"   ERROR: Winfax.dll location is too long\n");
                // Close the log file
                CloseLogFile();

                return E_UNEXPECTED;
        }

        // Load the winfax dll
        hModWinfax = LoadLibrary( szWinFaxDllLocation );
        if (hModWinfax == NULL) {
                WriteDebugString(L"   ERROR: LoadLibrary Failed: 0x%08x\n", GetLastError());
                WriteDebugString(L"   ERROR: DllUnregisterServer Failed\n");
                WriteDebugString(L"---SampleFSP: DllUnregisterServer Exit---\n");

                // Close the log file
                CloseLogFile();

                return E_UNEXPECTED;
        }

        pFaxUnregisterServiceProvider = (PFAXUNREGISTERSERVICEPROVIDER) GetProcAddress(hModWinfax, "FaxUnregisterServiceProviderW");
        if (pFaxUnregisterServiceProvider == NULL) {
                WriteDebugString(L"   ERROR: GetProcAddress Failed: 0x%08x\n", GetLastError());
                WriteDebugString(L"   ERROR: DllUnregisterServer Failed\n");

                FreeLibrary(hModWinfax);

                WriteDebugString(L"---SampleFSP: DllUnregisterServer Exit---\n");

                // Close the log file
                CloseLogFile();

                return E_UNEXPECTED;
        }

        // Open the fax service providers registry key
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                FAX_PROVIDERS_REGKEY,
                                0,
                                KEY_ALL_ACCESS,
                                &hServiceProvidersKey) != ERROR_SUCCESS) {
                WriteDebugString(L"   ERROR: RegOpenKeyEx Failed: 0x%08x\n", GetLastError());
                goto Exit;
        }

        // Open the newfsp service provider registry key
        if (RegOpenKeyEx(hServiceProvidersKey, 
                                NEWFSP_PROVIDER,
                                0,
                                KEY_ALL_ACCESS,
                                &hNewFspKey) != ERROR_SUCCESS) {
                WriteDebugString(L"   ERROR: RegOpenKeyEx Failed: 0x%08x\n", GetLastError());
                goto Exit;
        }

        // Open the newfsp service provider registry key
        if (RegOpenKeyEx(hNewFspKey, 
                                NEWFSP_DEVICES,
                                0,
                                KEY_ALL_ACCESS,
                                &hDevicesKey) != ERROR_SUCCESS) {
                WriteDebugString(L"   ERROR: RegOpenKeyEx Failed: 0x%08x\n", GetLastError());
                goto Exit;
        }

        // Create the virtual fax devices
        for (dwIndex = 0; dwIndex < NEWFSP_DEVICE_LIMIT; dwIndex++) 
        {
                _itot_s(dwIndex, strIndex,10);
                if (RegDeleteKey(hDevicesKey, strIndex)!= ERROR_SUCCESS) {
                        WriteDebugString(L"   ERROR: RegDeleteKey Failed: 0x%08x\n", GetLastError());
                        goto Exit;
                }
        }

        if (RegDeleteKey(hNewFspKey, NEWFSP_DEVICES) != ERROR_SUCCESS) {
                WriteDebugString(L"   ERROR: RegDeleteKey Failed: 0x%08x\n", GetLastError());
                goto Exit;
        }

        // Register the fax service provider
        if (pFaxUnregisterServiceProvider(NEWFSP_PROVIDER) == FALSE) {
                WriteDebugString(L"   ERROR: FaxUnregisterServiceProvider Failed: 0x%08x\n", GetLastError());
                WriteDebugString(L"   ERROR: DllUnregisterServer Failed\n");

                FreeLibrary(hModWinfax);

                WriteDebugString(L"---SampleFSP: DllUnregisterServer Exit---\n");

                // Close the log file
                CloseLogFile();

                return E_UNEXPECTED;
        }

        FreeLibrary(hModWinfax);
        WriteDebugString(L"---SampleFSP: DllRegisterServer Exit---\n");
        bRetVal = TRUE;
Exit:
        // Close the log file
        CloseLogFile();

        if(hNewFspKey)
        {
                RegCloseKey(hNewFspKey);
                hNewFspKey = NULL;
        }
        if(hServiceProvidersKey)
        {
                RegCloseKey(hServiceProvidersKey);
                hServiceProvidersKey = NULL;
        }
        if(bRetVal == TRUE)
                return S_OK;
        else
                return E_UNEXPECTED;
}





