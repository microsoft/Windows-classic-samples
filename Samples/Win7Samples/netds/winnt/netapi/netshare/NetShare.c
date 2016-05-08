/*++

Copyright (c) 1995 - 2000  Microsoft Corporation

Module Name:

    netshare.c

Abstract:

    This module illustrates how to use the Windows NT Lan Manager API
    in conjunction with the Win32 security API to create a new share
    on an arbitrary machine with permissions that grant an arbitrary
    user/group Full Access to the share.

Author:

    Scott Field (sfield)    01-Oct-95

--*/

#include <windows.h>
#include <lm.h>
#include <stdio.h>

#define RTN_OK 0
#define RTN_USAGE 1
#define RTN_ERROR 13

//
// Note: UNICODE entry point and argv.  This way, we don't need to bother
// with converting commandline args to Unicode
//

int
__cdecl
wmain(
    int argc,
    wchar_t *argv[]
    )
{
    LPWSTR DirectoryToShare;
    LPWSTR Sharename;
    LPWSTR Username;
    LPWSTR Server;

    PSID pSid = NULL;
    DWORD cbSid;

    WCHAR RefDomain[DNLEN + 1];
    DWORD cchDomain = DNLEN + 1;
    SID_NAME_USE peUse;

    SECURITY_DESCRIPTOR sd;
    PACL pDacl = NULL;
    DWORD dwAclSize;

    SHARE_INFO_502 si502;
    NET_API_STATUS nas;

    BOOL bSuccess = FALSE; // assume this function fails

    if(argc < 4) {
        printf("Usage: %ls <directory> <sharename> <user/group> [\\\\Server]\n", argv[0]);
        printf(" directory is fullpath of directory to share\n");
        printf(" sharename is name of share on server\n");
        printf(" user/group is an WinNT user/groupname (REDMOND\\sfield, Administrators, etc)\n");
        printf(" optional Server is the name of the computer to create the share on\n");
        printf("\nExample: %ls c:\\public public Everyone\n", argv[0]);
        printf("c:\\public shared as public granting Everyone full access\n");
        printf("\nExample: %ls c:\\private cool$ REDMOND\\sfield \\\\WINBASE\n", argv[0]);
        printf("c:\\private on \\\\WINBASE shared as cool$ (hidden) granting REDMOND\\sfield access\n");

        return RTN_USAGE;
    }

    //
    // since the commandline was Unicode, just provide pointers to
    // the relevant items
    //

    DirectoryToShare = argv[1];
    Sharename = argv[2];
    Username = argv[3];

    if( argc > 4 ) {
        Server = argv[4];
    } else {
        Server = NULL; // local machine
    }

    //
    // initial allocation attempt for Sid
    //
#define SID_SIZE 96
    cbSid = SID_SIZE;

    pSid = (PSID)HeapAlloc(GetProcessHeap(), 0, cbSid);
    if(pSid == NULL) {
        printf("HeapAlloc error!\n");
        return RTN_ERROR;
    }

    //
    // get the Sid associated with the supplied user/group name
    // force Unicode API since we always pass Unicode string
    //

    if(!LookupAccountNameW(
        NULL,       // default lookup logic
        Username,   // user/group of interest from commandline
        pSid,       // Sid buffer
        &cbSid,     // size of Sid
        RefDomain,  // Domain account found on (unused)
        &cchDomain, // size of domain in chars
        &peUse
        )) {

        //
        // if the buffer wasn't large enough, try again
        //

        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            PSID psidTemp;

            psidTemp = (PSID)HeapReAlloc(GetProcessHeap(), 0, pSid, cbSid);

            if(psidTemp == NULL) {
                printf("HeapReAlloc error!\n");
                goto cleanup;
            }
            else
            {
                pSid = psidTemp;
            }

            cchDomain = DNLEN + 1;

            if(!LookupAccountNameW(
                NULL,       // default lookup logic
                Username,   // user/group of interest from commandline
                pSid,       // Sid buffer
                &cbSid,     // size of Sid
                RefDomain,  // Domain account found on (unused)
                &cchDomain, // size of domain in chars
                &peUse
                )) {
                    printf("LookupAccountName error! (rc=%lu)\n", GetLastError());
                    goto cleanup;
                }

        } else {
            printf("LookupAccountName error! (rc=%lu)\n", GetLastError());
            goto cleanup;
        }
    }

    //
    // compute size of new acl
    //

    dwAclSize = sizeof(ACL) +
        1 * ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
        GetLengthSid(pSid) ;

    //
    // allocate storage for Acl
    //

    pDacl = (PACL)HeapAlloc(GetProcessHeap(), 0, dwAclSize);
    if(pDacl == NULL) goto cleanup;

    if(!InitializeAcl(pDacl, dwAclSize, ACL_REVISION))
        goto cleanup;

    //
    // grant GENERIC_ALL (Full Control) access
    //

    if(!AddAccessAllowedAce(
        pDacl,
        ACL_REVISION,
        GENERIC_ALL,
        pSid
        )) goto cleanup;

    if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
        goto cleanup;

    if(!SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE)) {
        fprintf(stderr, "SetSecurityDescriptorDacl error! (rc=%lu)\n",
            GetLastError());
        goto cleanup;
    }

    //
    // setup share info structure
    //

    si502.shi502_netname = (LMSTR) Sharename;
    si502.shi502_type = STYPE_DISKTREE;
    si502.shi502_remark = NULL;
    si502.shi502_permissions = 0;
    si502.shi502_max_uses = SHI_USES_UNLIMITED;
    si502.shi502_current_uses = 0;
    si502.shi502_path = (LMSTR) DirectoryToShare;
    si502.shi502_passwd = NULL;
    si502.shi502_reserved = 0;
    si502.shi502_security_descriptor = &sd;

    nas = NetShareAdd(
        (LMSTR) Server,         // share is on local machine
        502,            // info-level
        (LPBYTE)&si502, // info-buffer
        NULL            // don't bother with parm
        );

    if(nas != NO_ERROR) {
        printf("NetShareAdd error! (rc=%lu)\n", nas);
        goto cleanup;
    }

    bSuccess = TRUE; // indicate success

cleanup:

    //
    // free allocated resources
    //
    if(pDacl != NULL)
        HeapFree(GetProcessHeap(), 0, pDacl);

    if(pSid != NULL)
        HeapFree(GetProcessHeap(), 0, pSid);

    if(!bSuccess) {
        return RTN_ERROR;
    }

    return RTN_OK;
}

