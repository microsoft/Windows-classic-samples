#include <stdio.h>
#include <windows.h>
#include <ntdsapi.h>
#include <dsgetdc.h>
#include <lm.h>
#include <security.h>
#include "spn.h"

// Creates a spn if the user hasn't specified one.
void MakeSpn(unsigned char **pszSpn)
{
    DWORD status = ERROR_SUCCESS;
    ULONG ulSpn = 1;
    unsigned char ** arrSpn = NULL;
    HANDLE hDS;
    PDOMAIN_CONTROLLER_INFO pDomainControllerInfo;
    char lpCompDN[128];
    ULONG ulCompDNSize = sizeof(lpCompDN);
    BOOL NoFailure = TRUE;

    status = DsGetSpn(DS_SPN_NB_HOST,
                      "umarsh",
                      NULL, // DN of this service.
                      0, // Use the default instance port.
                      0, // Number of additional instance names.
                      NULL, // No additional instance names.
                      NULL, // No additional instance ports.
                      &ulSpn, // Size of SPN array.
                      &arrSpn); // Returned SPN(s).	
				  
    printf_s("DsGetSpn returned 0x%x\n", status);
    if (status != ERROR_SUCCESS) {
        exit(status);
    }
	
    // Get the name of domain if it is domain-joined
    if (status = DsGetDcName(NULL,
                             NULL,
                             NULL,
                             NULL,
                             DS_RETURN_DNS_NAME,
                             &pDomainControllerInfo) != NO_ERROR) {
        printf_s("DsGetDcName returned %d\n", GetLastError());
        NoFailure = FALSE;
    }
	
    // if it is domain joined 
    if (NoFailure) {
        // Bind to the domain controller for our domain 
        if ((status = DsBind(NULL,
                             pDomainControllerInfo->DomainName,
                             &hDS)) != ERROR_SUCCESS) {
            printf_s("DsBind returned %d\n", GetLastError());
            NoFailure = FALSE;
        }
    }

    if (NoFailure) {
        if ((status = NetApiBufferFree(pDomainControllerInfo)) != NERR_Success) {
            printf_s("NetApiBufferFree returned %d\n", status);
            exit(status);
        }

        if (GetComputerObjectName(NameFullyQualifiedDN, lpCompDN, &ulCompDNSize) == 0) {
            printf_s("GetComputerObjectName returned %d\n", GetLastError());
            exit(status);
        }

        /* We could check whether the SPN is already registered for this
        computer's DN, but we don't have to.  Modification is performed
        permissiely by this function, so that adding a value that already 
        exists does not return an error.  This way we can opt for the internal
        check instead of doing it ourselves. */
	
        status = DsWriteAccountSpn(hDS, DS_SPN_ADD_SPN_OP, lpCompDN, ulSpn, arrSpn);
        if (status != NO_ERROR) {
            printf_s("DsWriteAccountSpn returned %d\n", status);
            exit(status);
        }
        DsUnBind(&hDS);
    }

    *pszSpn = *arrSpn;
}

