/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1996 - 2000.  Microsoft Corporation.  All rights reserved.

Module Name:

    machacct.c

Abstract:

    The following sample illustrates how to create a machine account on the
    specified domain.

    Machine account types are defined by the following flags:

    UF_SERVER_TRUST_ACCOUNT (Backup domain controller)
    UF_WORKSTATION_TRUST_ACCOUNT (Workstation and server)
    UF_INTERDOMAIN_TRUST_ACCOUNT (Interdomain trust account)

    This sample attempts to create a workstation machine account, of type
    UF_WORKSTATION_TRUST_ACCOUNT.  This account type used for workstations
    and non-DC servers which are domain members.

    If the computer account creation fails with GetLastError ==
    ERROR_ACCESS_DENIED, the sample attempts to enable the
    SeMachineAccountPrivilege for the caller. If the privilege is enabled
    successfully, the computer account add operation is re-tried.

    Deleting machine accounts can be accomplished using the NetUserDel()
    Windows NT Lan Manager API call.

    Account update operations against a domain must be performed against
    the primary domain controller for the specified domain.  This sample
    uses the NetGetDCName Windows NT Lan Manager API call to determine
    the computer name of the primary domain controller.

    Commandline parameter argv[1] indicates the name of the account to
    create, which is typically the name of the machine.

    Commandline parameter argv[2] is optional and indicates the target domain.
    If this commandline argument is omitted, the machine account is created
    on the local domain.  It is recommended that you always supply a domain
    name in this sample, as this insures that the update occurs at the primary
    domain controller.

    The following commandline creates a machine account named WINBASE in
    the domain named CORP

    machacct.exe WINBASE CORP
--*/

#include <windows.h>
#include <stdio.h>
#include <lm.h>

BOOL
AddMachineAccount(
    LPWSTR wTargetComputer, // specifies where to add account
    LPWSTR MachineAccount,  // name of account
    DWORD AccountType       // account type
    );

BOOL
SetCurrentPrivilege(
    LPCTSTR Privilege,      // Privilege to enable/disable
    BOOL bEnablePrivilege,  // to enable or disable privilege
    BOOL * bPreviousPrivilege // returns previous state of the privilege
    );

void
DisplayError(
    LPSTR szAPI,    // pointer to failed API name
    DWORD dwLastError
    );

#define RTN_OK 0
#define RTN_USAGE 1
#define RTN_ERROR 13

//
// Unicode entry point and argv
//

int
__cdecl
wmain(
    int argc,
    wchar_t *argv[]
    )
{
    LPWSTR wMachineAccount;
    LPWSTR wPrimaryDC;
    LPTSTR MachineAccountPrivilege = SE_MACHINE_ACCOUNT_NAME;
    DWORD dwTrustAccountType = UF_WORKSTATION_TRUST_ACCOUNT;
    NET_API_STATUS nas;
    BOOL bSuccess;

    if(argc < 2) {
        fprintf(stderr, "Usage: %ls <machineaccountname> [domain]\n",
            argv[0]);
        return RTN_USAGE;
    }

    wMachineAccount = argv[1];

    //
    // if a domain name was specified, fetch the computer name of the
    // primary domain controller
    //
    if(argc == 3) {

        nas = NetGetDCName(NULL, argv[2], (LPBYTE *)&wPrimaryDC);

        if(nas != NERR_Success) {
            DisplayError("NetGetDCName", nas);
            return RTN_ERROR;
        }
    } else {
        //
        // default will operate on local machine.  Non-NULL wPrimaryDC will
        // cause buffer to be freed
        //
        wPrimaryDC = NULL;
    }

    bSuccess = AddMachineAccount(
        wPrimaryDC,         // primary DC computer name
        wMachineAccount,    // computer account name
        dwTrustAccountType  // computer account type
        );

    if(!bSuccess && GetLastError() == ERROR_ACCESS_DENIED ) {

        BOOL Previous;

        //
        // try to enable the SeMachineAccountPrivilege
        //
        if(SetCurrentPrivilege( MachineAccountPrivilege, TRUE, &Previous )) {

            //
            // enabled the privilege.  retry the add operation
            //
            bSuccess = AddMachineAccount(
                wPrimaryDC,
                wMachineAccount,
                dwTrustAccountType
                );

            //
            // disable the privilege
            //
            SetCurrentPrivilege( MachineAccountPrivilege, Previous, NULL );
        }
    }

    //
    // free the buffer allocated for the PDC computer name
    //
    if(wPrimaryDC) NetApiBufferFree(wPrimaryDC);

    if(!bSuccess) {
        DisplayError("AddMachineAccount", GetLastError());
        return RTN_ERROR;
    }

    return RTN_OK;
}

BOOL
AddMachineAccount(
    LPWSTR wTargetComputer,
    LPWSTR MachineAccount,
    DWORD AccountType
    )
{
    WCHAR wAccount[MAX_COMPUTERNAME_LENGTH + 2];
    LPWSTR wPassword;
    USER_INFO_1 ui;
    DWORD cchAccount;
    DWORD cchLength;
    NET_API_STATUS nas;

    //
    // ensure a valid computer account type was passed
    // TODO SetLastError
    //
    if (AccountType != UF_WORKSTATION_TRUST_ACCOUNT &&
        AccountType != UF_SERVER_TRUST_ACCOUNT &&
        AccountType != UF_INTERDOMAIN_TRUST_ACCOUNT
        ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // obtain number of chars in computer account name
    //
    cchLength = cchAccount = lstrlenW(MachineAccount);

    //
    // ensure computer name doesn't exceed maximum length
    //
    if(cchLength > MAX_COMPUTERNAME_LENGTH) {
        SetLastError(ERROR_INVALID_ACCOUNT_NAME);
        return FALSE;
    }

    //
    // password is the computer account name converted to lowercase
    // convert the passed MachineAccount in place
    //
    wPassword = MachineAccount;

    //
    // copy MachineAccount to the wAccount buffer allocated while
    // converting computer account name to uppercase.
    // convert password (inplace) to lowercase
    //
    while(cchAccount--) {
        wAccount[cchAccount] = towupper( MachineAccount[cchAccount] );
        wPassword[cchAccount] = towlower( wPassword[cchAccount] );
    }

    //
    // computer account names have a trailing Unicode '$'
    //
    wAccount[cchLength] = L'$';
    wAccount[cchLength + 1] = L'\0'; // terminate the string

    //
    // if the password is greater than the max allowed, truncate
    //
    if(cchLength > LM20_PWLEN) wPassword[LM20_PWLEN] = L'\0';

    //
    // initialize USER_INFO_x structure
    //
    ZeroMemory(&ui, sizeof(ui));

    ui.usri1_name = wAccount;
    ui.usri1_password = wPassword;

    ui.usri1_flags = AccountType | UF_SCRIPT;
    ui.usri1_priv = USER_PRIV_USER;

    nas = NetUserAdd(
                wTargetComputer,    // target computer name
                1,                  // info level
                (LPBYTE) &ui,       // buffer
                NULL
                );

    //
    // indicate whether it was successful
    //
    if(nas == NERR_Success) {
        return TRUE;
    }
    else {
        SetLastError(nas);
        return FALSE;
    }
}

DWORD
SetCurrentPrivilege(
    LPCTSTR Privilege,      // Privilege to enable/disable
    BOOL bEnablePrivilege,  // to enable or disable privilege
    BOOL * bPreviousPrivilege // returns previous state of the privilege
    )
{
    DWORD dwError;
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;
    TOKEN_PRIVILEGES tpPrevious;
    DWORD cbPrevious=sizeof(TOKEN_PRIVILEGES);
    BOOL bSuccess=FALSE;

    if(!LookupPrivilegeValue(NULL, Privilege, &luid)) return ERROR_NO_SUCH_PRIVILEGE;

    if(!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
            &hToken
            )) return GetLastError();

    //
    // first pass.  get current privilege setting
    //
    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = 0;

    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tp,
            sizeof(TOKEN_PRIVILEGES),
            &tpPrevious,
            &cbPrevious
            );

    if(( dwError = GetLastError()) == ERROR_SUCCESS) {
        //
        // second pass.  set privilege based on previous setting
        //
        tpPrevious.PrivilegeCount     = 1;
        tpPrevious.Privileges[0].Luid = luid;

        if ( bPreviousPrivilege ) {

            *bPreviousPrivilege = (( tpPrevious.Privileges[0].Attributes & SE_PRIVILEGE_ENABLED ) != 0 );
        }

        if(bEnablePrivilege) {
            tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
        }
        else {
            tpPrevious.Privileges[0].Attributes &= ~(SE_PRIVILEGE_ENABLED);
        }

        AdjustTokenPrivileges(
                hToken,
                FALSE,
                &tpPrevious,
                cbPrevious,
                NULL,
                NULL
                );

        dwError = GetLastError();
    }

    CloseHandle(hToken);

    return dwError;
}

void
DisplayError(
    LPSTR szAPI,    // pointer to failed API name
    DWORD dwLastError
    )
{
    HMODULE hModule = NULL;
    LPSTR MessageBuffer;
    DWORD dwBufferLength;

    fprintf(stderr,"%s error! (rc=%lu)\n", szAPI, dwLastError);

    if(dwLastError >= NERR_BASE && dwLastError <= MAX_NERR) {
        hModule = LoadLibraryEx(
            TEXT("netmsg.dll"),
            NULL,
            LOAD_LIBRARY_AS_DATAFILE
            );
    }

    if(dwBufferLength=FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_FROM_SYSTEM |
        ((hModule != NULL) ? FORMAT_MESSAGE_FROM_HMODULE : 0),
        hModule, // module to get message from
        dwLastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
        (LPSTR) &MessageBuffer,
        0,
        NULL
        ))
    {
        DWORD dwBytesWritten;

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

    if(hModule != NULL)
        FreeLibrary(hModule);
}

