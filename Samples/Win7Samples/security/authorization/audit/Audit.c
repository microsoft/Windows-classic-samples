/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1996 - 2000.  Microsoft Corporation.  All rights reserved.

Module Name:

    audit.c

Abstract:

    This module illustrates how to use the Windows NT LSA security API
    to manage the audit status on the local machine or a remote machine.

    Querying the current audit status is illustrated, in addition to
    changing the audit state of an audit event type.  Enabling and
    disabling all auditing is also illustrated.

    When targetting a domain controller for an audit update operation,
    be sure to target the primary domain controller for the domain.
    The audit settings are replicated by the primary domain controller
    to each backup domain controller as appropriate.  The NetGetDCName()
    Lan Manager API call can be used to get the primary domain controller
    computer name from a domain name.

    This sample will target the local machine if no command line argument is
    specified, or the machine specified on argv[1], eg. audit.exe \\winbase

    The sample relies on the ntsecapi.h header file found in the Win32SDK
    \mstools\security directory.

--*/

#include <windows.h>
#include <stdio.h>

#include "ntsecapi.h" // \mstools\samples\win32\winnt\security\include\ntsecapi.h

#define RTN_OK 0
#define RTN_USAGE 1
#define RTN_ERROR 13

//
// if you have the ddk, include ntstatus.h
//
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS              ((NTSTATUS)0x00000000L)
#endif

#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER    ((NTSTATUS)0xC000000DL)
#endif

NTSTATUS
DisplayAudit(
    LSA_HANDLE PolicyHandle
    );

void
DisplayAuditEventOption(
    DWORD EventTypeIndex,
    POLICY_AUDIT_EVENT_OPTIONS EventOption
    );

NTSTATUS
SetAuditEvent(
    LSA_HANDLE PolicyHandle,
    POLICY_AUDIT_EVENT_TYPE EventType,
    POLICY_AUDIT_EVENT_OPTIONS EventOption
    );

NTSTATUS
SetAuditMode(
    LSA_HANDLE PolicyHandle,
    BOOL bEnable
    );

//
// helper functions
//

NTSTATUS
OpenPolicy(
    LPWSTR ServerName,
    DWORD DesiredAccess,
    PLSA_HANDLE PolicyHandle
    );

void
InitLsaString(
    PLSA_UNICODE_STRING LsaString,
    LPWSTR String
    );

void
DisplayNtStatus(
    LPSTR szAPI,        // pointer to Ansi function name
    NTSTATUS Status     // NTSTATUS error value
    );

void
DisplayWinError(
    LPSTR szAPI,    // pointer to Ansi function name
    DWORD dwError   // DWORD WinError
    );

//
// unicode entry point and argv
//
int
__cdecl
wmain(
    int argc,
    wchar_t *argv[]
    )
{
    LPWSTR wComputerName;
    LSA_HANDLE PolicyHandle;
    NTSTATUS Status;

    //
    // pickup machine name if appropriate
    //
    if(argc == 2)
        wComputerName = argv[1];
    else
        wComputerName = NULL; // local machine

    //
    // display current audit state
    //

    Status = OpenPolicy(
                wComputerName,
                POLICY_VIEW_AUDIT_INFORMATION,
                &PolicyHandle
                );

    if(Status == STATUS_SUCCESS) {
        //
        // display current auditing status
        //
        Status = DisplayAudit(PolicyHandle);

        LsaClose(PolicyHandle);

        if(Status != STATUS_SUCCESS) {
            DisplayNtStatus("DisplayAudit", Status);
            return RTN_ERROR;
        }
    } else {
        DisplayNtStatus("OpenPolicy", Status);
        return RTN_ERROR;
    }

    //
    // enable success and failure audits of logon/logoff events
    //

    Status = OpenPolicy(
                wComputerName,
                POLICY_VIEW_AUDIT_INFORMATION |
                POLICY_SET_AUDIT_REQUIREMENTS,
                &PolicyHandle
                );

    if(Status == STATUS_SUCCESS) {

        //
        // enable success and failure auditing of logon/logoff
        //
        Status = SetAuditEvent(
            PolicyHandle,
            AuditCategoryLogon,
            POLICY_AUDIT_EVENT_SUCCESS | POLICY_AUDIT_EVENT_FAILURE
            );

        //
        // enable audits
        //
        if( Status == STATUS_SUCCESS )
            Status = SetAuditMode(PolicyHandle, TRUE);

        LsaClose(PolicyHandle);

        if(Status != STATUS_SUCCESS) {
            DisplayNtStatus("SetAuditMode", Status);
            return RTN_ERROR;
        }
    } else {
        DisplayNtStatus("OpenPolicy", Status);
        return RTN_ERROR;
    }

    return RTN_OK;
}


NTSTATUS
DisplayAudit(
    LSA_HANDLE PolicyHandle
    )
{
    PPOLICY_AUDIT_EVENTS_INFO AuditEvents;
    NTSTATUS Status;
    DWORD i; // index into EventAuditingOptions

    //
    // obtain AuditEvents
    //
    Status = LsaQueryInformationPolicy(
                PolicyHandle,
                PolicyAuditEventsInformation,
                &AuditEvents
                );

    if(Status != STATUS_SUCCESS) return Status;

    //
    // successfully obtained AuditEventsInformation.  Now display.
    //
    if(AuditEvents->AuditingMode) {
        printf("Auditing Enabled\n");
    } else {
        printf("Auditing Disabled\n");
    }

    for(i = 0 ; i < AuditEvents->MaximumAuditEventCount ; i++) {
        DisplayAuditEventOption(i, AuditEvents->EventAuditingOptions[i]);
    }

    //
    // free allocated memory
    //
    LsaFreeMemory(AuditEvents);

    return Status;
}

void
DisplayAuditEventOption(
    DWORD EventTypeIndex,
    POLICY_AUDIT_EVENT_OPTIONS EventOption
    )
{
    printf("AuditCategory");

    switch (EventTypeIndex) {
        case AuditCategorySystem:
        printf("System");
        break;

        case AuditCategoryLogon:
        printf("Logon");
        break;

        case AuditCategoryObjectAccess:
        printf("ObjectAccess");
        break;

        case AuditCategoryPrivilegeUse:
        printf("PrivilegeUse");
        break;

        case AuditCategoryDetailedTracking:
        printf("DetailedTracking");
        break;

        case AuditCategoryPolicyChange:
        printf("PolicyChange");
        break;

        case AuditCategoryAccountManagement:
        printf("AccountManagement");
        break;

        default:
        printf("Unknown");
    }

    if(EventOption & POLICY_AUDIT_EVENT_SUCCESS)
        printf(" AUDIT_EVENT_SUCCESS");

    if(EventOption & POLICY_AUDIT_EVENT_FAILURE)
        printf(" AUDIT_EVENT_FAILURE");

    printf("\n");
}

NTSTATUS
SetAuditEvent(
    LSA_HANDLE PolicyHandle,
    POLICY_AUDIT_EVENT_TYPE EventType,
    POLICY_AUDIT_EVENT_OPTIONS EventOption
    )
{
    PPOLICY_AUDIT_EVENTS_INFO pae;
    NTSTATUS Status;
    DWORD i; // index into EventAuditingOptions

    //
    // obtain AuditEvents
    //
    Status = LsaQueryInformationPolicy(
                PolicyHandle,
                PolicyAuditEventsInformation,
                &pae
                );

    if(Status != STATUS_SUCCESS) return Status;

    //
    // insure we were passed a valid EventType and EventOption
    //
    if((ULONG)EventType > pae->MaximumAuditEventCount ||
      (!EventOption & POLICY_AUDIT_EVENT_MASK) ) {
        LsaFreeMemory(pae);
        return STATUS_INVALID_PARAMETER;
    }

    //
    // set all auditevents to the unchanged status...
    //
    for(i = 0 ; i < pae->MaximumAuditEventCount ; i++) {
        pae->EventAuditingOptions[i] = POLICY_AUDIT_EVENT_UNCHANGED;
    }

    //
    // ...and update only the specified EventType
    //
    pae->EventAuditingOptions[EventType] = EventOption;

    //
    // set the new AuditEvents
    //
    Status = LsaSetInformationPolicy(
                PolicyHandle,
                PolicyAuditEventsInformation,
                pae
                );

    //
    // free allocated memory
    //
    LsaFreeMemory(pae);

    return Status;
}

NTSTATUS
SetAuditMode(
    LSA_HANDLE PolicyHandle,
    BOOL bEnable
    )
{
    PPOLICY_AUDIT_EVENTS_INFO AuditEvents;
    NTSTATUS Status;
    DWORD i;

    //
    // obtain current AuditEvents
    //
    Status = LsaQueryInformationPolicy(
                PolicyHandle,
                PolicyAuditEventsInformation,
                &AuditEvents
                );

    if(Status != STATUS_SUCCESS) return Status;

    //
    // update the relevant member
    //
    AuditEvents->AuditingMode = bEnable;

    //
    // set all auditevents to the unchanged status...
    //
    for(i = 0 ; i < AuditEvents->MaximumAuditEventCount ; i++) {
        AuditEvents->EventAuditingOptions[i] = POLICY_AUDIT_EVENT_UNCHANGED;
    }

    //
    // set the new auditing mode (enabled or disabled)
    //
    Status = LsaSetInformationPolicy(
                PolicyHandle,
                PolicyAuditEventsInformation,
                AuditEvents
                );

    LsaFreeMemory(AuditEvents);

    return Status;
}

void
InitLsaString(
    PLSA_UNICODE_STRING LsaString,
    LPWSTR String
    )
{
    DWORD StringLength;

    if(String == NULL) {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;

        return;
    }

    StringLength = lstrlenW(String);
    LsaString->Buffer = String;
    LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
    LsaString->MaximumLength = (USHORT) (StringLength + 1) *
        sizeof(WCHAR);
}

NTSTATUS
OpenPolicy(
    LPWSTR ServerName,
    DWORD DesiredAccess,
    PLSA_HANDLE PolicyHandle
    )
{
    PLSA_UNICODE_STRING Server;
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;

    //
    // Always initialize the object attributes to all zeroes
    //
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

    if(ServerName != NULL) {
        //
        // Make a LSA_UNICODE_STRING out of the LPWSTR passed in
        //
        InitLsaString(&ServerString, ServerName);
        Server = &ServerString;
    } else {
        Server = NULL; // default to local machine
    }

    //
    // Attempt to open the policy and return NTSTATUS
    //
    return LsaOpenPolicy(
                Server,
                &ObjectAttributes,
                DesiredAccess,
                PolicyHandle
                );
}

void
DisplayNtStatus(
    LPSTR szAPI,
    NTSTATUS Status
    )
{
    //
    // convert the NTSTATUS to Winerror and DisplayWinError()
    //
    DisplayWinError(szAPI, LsaNtStatusToWinError(Status) );
}

void
DisplayWinError(
    LPSTR szAPI,    // pointer to Ansi function name
    DWORD dwError   // DWORD WinError
    )
{
    LPSTR MessageBuffer;
    DWORD dwBufferLength;

    //
    // TODO get this fprintf out of here!
    //
    fprintf(stderr,"%s error!\n", szAPI);

    if(dwBufferLength=FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR) &MessageBuffer,
            0,
            NULL
            ))
    {
        DWORD dwBytesWritten; // unused

        //
        // Output message string on stderr
        //
        WriteFile(
                GetStdHandle(STD_ERROR_HANDLE),
                MessageBuffer,
                dwBufferLength,
                &dwBytesWritten,
                NULL
                );

        //
        // free the buffer allocated by the system
        //
        LocalFree(MessageBuffer);
    }
}
