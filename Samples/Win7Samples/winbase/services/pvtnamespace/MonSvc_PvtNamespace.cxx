// MonSvc_PvtNameSpace.cxx : Sample service to illustrate usage of private namespaces and service SIDs
// MonSvc creates a private namespace and sets an event there when the service is stopped. 
// Clients that rendezvous with this event will be guaranteed that it was created by MonSvc service.

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

//
// Globals
//
SERVICE_STATUS          ssService;
SERVICE_STATUS_HANDLE   hssService;
HANDLE                  g_hEvent;

// **************************************************************************
// Function Name: SvcDebugOut()
//
// Purpose: Output error message to the debugger. 
//
// Arguments:
//    IN LPTSTR String - Error message
//    IN DWORD Status - Error code
//
// Return Values:
//    None
VOID SvcDebugOut(LPTSTR String, DWORD Status);

// **************************************************************************
// Function Name: CreateNamespaceSD()
//
// Purpose: Create the SD for use with the private namespace. 
//
// Arguments:
//    OUT PSECURITY_ATTRIBUTES pSecurityAttributes - SECURITY_ATTRIBUTES structure
//    IN PSID ServiceSID - Service SID of the namespace creator service
//
// Return Values:
//    Win32 Exit code
DWORD CreateNamespaceSD(PSECURITY_ATTRIBUTES pSecurityAttributes, PSID ServiceSID);

// **************************************************************************
// Function Name: FreeSD()
//
// Purpose: Frees the Security Descriptor that is passed in. 
//
// Arguments:
//    IN PSECURITY_DESCRIPTOR pSecDesc - Security Descriptor
//
// Return Values:
//    None
VOID FreeSD(PSECURITY_DESCRIPTOR pSecDesc);

// **************************************************************************
// Function Name: ServiceStopAlert
//
// Purpose: Alert clients when the service is stopped. Uses a private namespace
//			to rendezvous with the client.
//
// Return Values:
//    Win32 Exit code
DWORD ServiceStopAlert (VOID);

// **************************************************************************
// Function Name: ServiceStart
//
// Purpose: Service entry point
//
// Return Values:
//    None
VOID WINAPI ServiceStart(DWORD argc, LPTSTR *argv);

DWORD WINAPI ServiceCtrlHandler(DWORD Opcode, DWORD EventType, PVOID pEventData, PVOID pContext);

/****************************************************************************/


int __cdecl wmain (DWORD argc, LPWSTR argv[])
{
	SERVICE_TABLE_ENTRY   DispatchTable[] =
	{
		{TEXT("MonSvc"),  ServiceStart},
		{NULL,  NULL}
	};

	StartServiceCtrlDispatcher(DispatchTable);

	ExitProcess ( 0 );

	return 0;
}


VOID WINAPI ServiceStart(DWORD argc, LPTSTR *argv)
{
	HANDLE      hAutostartEvent = NULL;
	DWORD       dwError = ERROR_SUCCESS;

	hssService = RegisterServiceCtrlHandlerEx(TEXT("MonSvc"), ServiceCtrlHandler, NULL);

	if (hssService == (SERVICE_STATUS_HANDLE)0)
	{
		dwError = GetLastError();
		SvcDebugOut(TEXT("RegisterServiceCtrlHandler failed - "), dwError);
		ExitProcess(dwError);               
	}

	// Initialize the service status structure
	ssService.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
	ssService.dwControlsAccepted        = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;                                    
	ssService.dwServiceSpecificExitCode = 0;

	// Notification for stopping service.
	g_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (g_hEvent == NULL)
	{
		dwError = GetLastError();
		SvcDebugOut(TEXT("CreateEvent error"), dwError);

		ssService.dwCurrentState       = SERVICE_STOPPED;
		ssService.dwCheckPoint         = 0;
		ssService.dwWaitHint           = 0;
		ssService.dwWin32ExitCode      = dwError;

		if (!SetServiceStatus (hssService, &ssService))
		{
			SvcDebugOut(TEXT("SetServiceStatus error - "), GetLastError());
		}

		return;
	}

	//
	// Initialization complete - report running status. We start accepting stop and
	// shutdown controls only after reporting a status of SERVICE_RUNNING. It is easier
	// to design services that don't accept stop/shutdown controls while they are
	// in SERVICE_START_PENDING state.
	//

	ssService.dwCurrentState       = SERVICE_RUNNING;
	ssService.dwCheckPoint         = 0;
	ssService.dwWaitHint           = 0;
	ssService.dwWin32ExitCode      = 0;

	SvcDebugOut(TEXT("Initialized and running - "), 0);

	if (!SetServiceStatus (hssService, &ssService))
	{
		SvcDebugOut(TEXT("SetServiceStatus error - "), GetLastError());
	}

	dwError = ServiceStopAlert();

	ssService.dwCurrentState       = SERVICE_STOPPED;
	ssService.dwCheckPoint         = 0;
	ssService.dwWaitHint           = 0;
	ssService.dwWin32ExitCode      = dwError;

	if (!SetServiceStatus (hssService, &ssService))
	{
		SvcDebugOut(TEXT("SetServiceStatus error - "), GetLastError());
	}

	if(g_hEvent != NULL)
	{
		CloseHandle(g_hEvent);
		g_hEvent = NULL;
	}

	return;
}

DWORD ServiceStopAlert(VOID)
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
	SECURITY_ATTRIBUTES nameSpaceAttribute;
	
	// Get the Service SID
	pSid = (PSID) Sid;
	bRet = LookupAccountName(NULL, L"NT Service\\MonSvc", pSid, &cbSid, szDomainName, &cchDomainName, &sidNameUse);
	if(!bRet)
	{
		dwError = GetLastError();
		SvcDebugOut(TEXT("Error looking up service SID - "), dwError);
		goto FnExit;
	}

	// Create the Boundary Descriptor for the private namespace
	BoundaryDescriptor = CreateBoundaryDescriptor(szBDName, 0);
	if(BoundaryDescriptor == NULL)
	{
		dwError = GetLastError();
		SvcDebugOut(TEXT("Error creating Boundary Descriptor for private namespace - "), dwError);
		goto FnExit;
	}

	// Add the Service SID to the Boundary Descriptor
	bRet = AddSIDToBoundaryDescriptor(&BoundaryDescriptor, pSid);
	if(!bRet)
	{
		dwError = GetLastError();
		SvcDebugOut(TEXT("Error adding SID to Boundary Descriptor - "), dwError);
		goto FnExit;
	}

	// Create SD for the namespace
	nameSpaceAttribute.nLength = sizeof(SECURITY_ATTRIBUTES);
	nameSpaceAttribute.bInheritHandle = FALSE;
	dwError = CreateNamespaceSD(&nameSpaceAttribute, pSid);
	if(dwError != ERROR_SUCCESS)
	{
		goto FnExit;
	}

	// Create the private namespace
	hNameSpace = CreatePrivateNamespace(&nameSpaceAttribute, BoundaryDescriptor, L"MonSvcNamespace"); 
	if(hNameSpace == NULL)
	{
		dwError = GetLastError();
		SvcDebugOut(TEXT("Error creating private namespace - "), dwError);
		goto FnExit;
	}

	// Create an event in the private namespace
	hEvent = CreateEvent(&nameSpaceAttribute, FALSE, FALSE, L"MonSvcNamespace\\StopAlert");
	if(hEvent == NULL)
	{
		dwError = GetLastError();
		SvcDebugOut(TEXT("Error creating event - "), dwError);
		goto FnExit;
	}

	while(TRUE)
	{
		// Wait for STOP control
		dwStatus = WaitForSingleObjectEx(g_hEvent, INFINITE, FALSE);

		// Check if this was signaled due to a SERVICE_STOP control
		if(dwStatus == WAIT_OBJECT_0)
		{
			break;
		}
	}

	// Alert the clients about service STOP
	SetEvent(hEvent);

FnExit:

	if(BoundaryDescriptor != NULL)
	{
		DeleteBoundaryDescriptor(BoundaryDescriptor);	
		BoundaryDescriptor = NULL;
	}

	if(hNameSpace != NULL)
	{
		ClosePrivateNamespace(hNameSpace, PRIVATE_NAMESPACE_FLAG_DESTROY);
		hNameSpace = NULL;
	}

	if(hEvent != NULL)
	{
		CloseHandle(hEvent);
		hEvent = NULL;
	}

	FreeSD(nameSpaceAttribute.lpSecurityDescriptor);

	return dwError;
}

DWORD WINAPI ServiceCtrlHandler(DWORD Opcode, DWORD EventType, PVOID pEventData, PVOID pContext)
{
	switch(Opcode)
	{
	case SERVICE_CONTROL_SHUTDOWN:
		SvcDebugOut(TEXT("Shutdown command received - "), Opcode);

		//
		// Fall through to STOP case
		//

	case SERVICE_CONTROL_STOP:
		ssService.dwWin32ExitCode = ERROR_SUCCESS;
		ssService.dwCurrentState  = SERVICE_STOP_PENDING;
		ssService.dwCheckPoint    = 1;
		ssService.dwWaitHint      = 10000;
		break;

	default:
		SvcDebugOut(TEXT("Unrecognized opcode - "), Opcode);
	}

	if (!SetServiceStatus(hssService, &ssService))
	{
		SvcDebugOut(TEXT("SetServiceStatus error - "), GetLastError());
	}

	// Notify the service thread
	if (Opcode == SERVICE_CONTROL_STOP || Opcode == SERVICE_CONTROL_SHUTDOWN) 
	{
		SetEvent(g_hEvent);
	}

	return ( ERROR_SUCCESS );
}

DWORD CreateNamespaceSD(PSECURITY_ATTRIBUTES pSecurityAttributes, PSID ServiceSid)
{
	BOOL bRet = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSECURITY_DESCRIPTOR pSecDesc = NULL;
	ULONG ACLSize = 0;
	PACL pACL = NULL;
	PSID pWorldSid = NULL;
	BYTE Sid[SECURITY_MAX_SID_SIZE];
	DWORD cbSid = SECURITY_MAX_SID_SIZE;

	pSecDesc = (PSECURITY_DESCRIPTOR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SECURITY_DESCRIPTOR));

	if(pSecDesc == NULL)
	{
		goto FnExit;
	}

	//  Initialize Security Descriptor
	bRet = InitializeSecurityDescriptor(pSecDesc, SECURITY_DESCRIPTOR_REVISION);
	if(!bRet)
	{
		goto FnExit;
	}

	//Get the World SID
	pWorldSid = (PSID) Sid;
	bRet = CreateWellKnownSid(WinWorldSid, NULL, pWorldSid, &cbSid);
	if(!bRet)
	{
		goto FnExit;
	}

	//  Calculate ACL size
	ACLSize = sizeof(ACL) + 
		(sizeof( ACCESS_ALLOWED_ACE ) - sizeof(DWORD)) * 2 + 
		GetLengthSid( ServiceSid ) + GetLengthSid(pWorldSid);

	pACL = (PACL) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ACLSize);

	if(NULL == pACL)
	{
		goto FnExit;
	}

	//  Initialize ACL
	bRet = InitializeAcl(pACL, ACLSize, ACL_REVISION);
	if(!bRet)
	{
		goto FnExit;
	}

	//  Add Service SID to Allowed Access ACE
	bRet = AddAccessAllowedAce(pACL, ACL_REVISION, GENERIC_ALL, ServiceSid);
	if(!bRet || !IsValidAcl(pACL))
	{
		goto FnExit;
	}

	//  Add World SID to Allowed Access ACE
	bRet = AddAccessAllowedAce(pACL, ACL_REVISION, GENERIC_READ | SYNCHRONIZE, pWorldSid);
	if(!bRet || !IsValidAcl(pACL))
	{
		goto FnExit;
	}

	//  Set the DACL in the security descriptor
	bRet = SetSecurityDescriptorDacl(pSecDesc, TRUE, pACL, FALSE);
	if(!bRet)
	{
		goto FnExit;
	}

	//  Set the Owner Sid to be the Service SID
	bRet = SetSecurityDescriptorOwner(pSecDesc, ServiceSid, FALSE);
	if(!bRet)
	{
		goto FnExit;
	}

	//  Set the group Sid to be the Service SID
	bRet = SetSecurityDescriptorGroup(pSecDesc, ServiceSid, FALSE);
	if(!bRet)
	{
		goto FnExit;
	}

	pSecurityAttributes->lpSecurityDescriptor = pSecDesc;

	if(IsValidSecurityDescriptor(pSecDesc))
	{
		return dwError;
	}

FnExit:

	if(pSecDesc != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pSecDesc);
	}

	if(pACL != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pACL);
	}

	dwError = GetLastError(); 
	SvcDebugOut(TEXT("Error creating security attributes - "), dwError);

	return dwError;
}

VOID FreeSD(PSECURITY_DESCRIPTOR pSecDesc)
{
	BOOL bDACLPresent = FALSE;
	BOOL bDACLDefault = FALSE;
	PACL pDACL = NULL;
	BOOL bRet = FALSE;

	if(pSecDesc != NULL)
	{
		bRet = GetSecurityDescriptorDacl(pSecDesc, &bDACLPresent, &pDACL, &bDACLDefault);
		if(bRet && bDACLPresent)
		{
			HeapFree(GetProcessHeap(), 0, pDACL);
		}

		HeapFree(GetProcessHeap(), 0, pSecDesc);
	}
}

VOID SvcDebugOut(LPTSTR String, DWORD Status) 
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
