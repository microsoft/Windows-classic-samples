// Client_PvtNameSpace.cxx : Sample client to illustrate usage of private namespaces.
// Client opens a private namespace created by MonSvc and wait for the service stop event to be triggered.

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

// **************************************************************************
// Function Name: ClientDebugOut()
//
// Purpose: Output error message to the debugger. 
//
// Arguments:
//    IN LPTSTR String - Error message
//    IN DWORD Status - Error code
//
// Return Values:
//    None
VOID ClientDebugOut(LPTSTR String, DWORD Status);

// **************************************************************************
// Function Name: MonitorServiceStop
//
// Purpose: Opens the private namespace and waits on the service stop event to 
//			be signalled
//
// Return Values:
//    Win32 Exit code
DWORD MonitorServiceStop (VOID);

/****************************************************************************/


int __cdecl wmain (DWORD argc, LPWSTR argv[])
{
	MonitorServiceStop();

	ExitProcess ( 0 );

	return 0;
}

DWORD MonitorServiceStop(VOID)
{
	DWORD dwError = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	BOOL bRet = FALSE;
	HANDLE hNameSpace = NULL;
	HANDLE hEvent = NULL;
	WCHAR szBDName[] = L"MonSvcBD";
	WCHAR szDomainName[MAX_PATH + 1] = {0};
	DWORD cchDomainName = MAX_PATH + 1;
	PSID pSid = NULL;
	BYTE Sid[SECURITY_MAX_SID_SIZE];
	DWORD cbSid = SECURITY_MAX_SID_SIZE;
	SID_NAME_USE sidNameUse;
	HANDLE BoundaryDescriptor = NULL;

	// Get the Service SID
	pSid = (PSID) Sid;
	bRet = LookupAccountName(NULL, L"NT Service\\MonSvc", pSid, &cbSid, szDomainName, &cchDomainName, &sidNameUse);
	if(!bRet)
	{
		dwError = GetLastError();
		ClientDebugOut(TEXT("Error looking up service SID - "), dwError);
		goto FnExit;
	}

	// Create the Boundary Descriptor for the private namespace
	BoundaryDescriptor = CreateBoundaryDescriptor(szBDName, 0);
	if(BoundaryDescriptor == NULL)
	{
		dwError = GetLastError();
		ClientDebugOut(TEXT("Error creating Boundary Descriptor for private namespace - "), dwError);
		goto FnExit;
	}

	// Add the Service SID to the Boundary Descriptor
	bRet = AddSIDToBoundaryDescriptor(&BoundaryDescriptor, pSid);
	if(!bRet)
	{
		dwError = GetLastError();
		ClientDebugOut(TEXT("Error adding SID to Boundary Descriptor - "), dwError);
		goto FnExit;
	}

	// Open the private namespace
	hNameSpace = OpenPrivateNamespace(BoundaryDescriptor, L"MonSvcNamespace"); 
	if(hNameSpace == NULL)
	{
		dwError = GetLastError();
		ClientDebugOut(TEXT("Error opening private namespace - "), dwError);
		goto FnExit;
	}

	// Open the event in the private namespace
	hEvent = OpenEvent(GENERIC_READ | SYNCHRONIZE, FALSE, L"MonSvcNamespace\\StopAlert");
	if(hEvent == NULL)
	{
		dwError = GetLastError();
		ClientDebugOut(TEXT("Error opening event - "), dwError);
		goto FnExit;
	}

	while(TRUE)
	{
		// Wait for service to STOP
		dwStatus = WaitForSingleObjectEx(hEvent, INFINITE, FALSE);

		// Check if this was signaled due to a SERVICE_STOP control
		if(dwStatus == WAIT_OBJECT_0)
		{
			break;
		}
	}

	//Trigger alert pop-up
	MessageBox(NULL, TEXT("MonSvc was Stopped"), TEXT("Service Stop Alert"), MB_OK);

FnExit:

	if(BoundaryDescriptor != NULL)
	{
		DeleteBoundaryDescriptor(BoundaryDescriptor);	
		BoundaryDescriptor = NULL;
	}

	if(hNameSpace != NULL)
	{
		ClosePrivateNamespace(hNameSpace, 0);
		hNameSpace = NULL;
	}

	if(hEvent != NULL)
	{
		CloseHandle(hEvent);
		hEvent = NULL;
	}

	return dwError;
}

VOID ClientDebugOut(LPTSTR String, DWORD Status) 
{ 
	HRESULT hr = S_OK;
	TCHAR  Buffer[1024]; 
	hr = StringCchPrintf(Buffer, 1024, TEXT("%s %d"), String, Status); 
	if(hr == S_OK)
	{
		OutputDebugString(Buffer); 
	}
	else
	{
		OutputDebugString(TEXT("Error in Dbg string")); 
	}
}