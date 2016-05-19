/*--

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2000  Microsoft Corporation.  All rights reserved.

Module Name:

    klist.c

Abstract:

    Sample program that demonstrates how to:
       query Kerberos ticket cache
       purge Kerberos tickets from cache
       request service ticket

Author:

    David Mowers (davemo)   14-October-98

Revision History:

--*/


//
// Common include files.
//
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>      
#include <stdlib.h>     
#include <conio.h>      
#include <ntsecapi.h>
#define SECURITY_WIN32
#include <security.h>   

#define INTERACTIVE_PURGE 1

#define SEC_SUCCESS(Status) ((Status) >= 0) 

VOID
InitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString OPTIONAL
    );

VOID 
ShowLastError(
    LPSTR szAPI, 
    DWORD dwError
    );

VOID 
ShowNTError( 
    LPSTR szAPI, 
    NTSTATUS Status 
    );

BOOL 
PackageConnectLookup(
    HANDLE *pLogonHandle, 
    ULONG *pPackageId
    );

BOOL 
ShowTickets(
    HANDLE LogonHandle,
    ULONG PackageId,
    DWORD dwMode
    );

BOOL
ShowTgt(
    HANDLE LogonHandle,
    ULONG PackageId
    );

DWORD 
GetEncodedTicket(
    HANDLE LogonHandle,
    ULONG PackageId,
    wchar_t *Server
    );

int __cdecl
wmain(
    int argc, 
    wchar_t  *argv[]
    )
{

    HANDLE LogonHandle = NULL;
    ULONG PackageId;
    
    if (argc < 2)
    {
        printf("Usage: %S <tickets | tgt | purge | get> [service principal name(for get)]\n",argv[0]);
        return FALSE;
    }

    //
    // Get the logon handle and package ID from the
    // Kerberos package
    //
    if(!PackageConnectLookup(&LogonHandle, &PackageId))
        return FALSE;

    if(!_wcsicmp(argv[1],L"tickets"))
    {
        ShowTickets(LogonHandle, PackageId, 0);
    }
    else if(!_wcsicmp(argv[1],L"tgt"))
    {
        ShowTgt(LogonHandle, PackageId);
    }
    else if(!_wcsicmp(argv[1],L"purge"))
    {
        ShowTickets(LogonHandle, PackageId, INTERACTIVE_PURGE);
    }
    else if(!_wcsicmp(argv[1],L"get"))
    {
        if(argc < 3)
        {
            printf("Provide service principal name (SPN) of encoded ticket to retrieve\n");
        }
        else
            GetEncodedTicket(LogonHandle, PackageId, argv[2]);
    }
    else
    {
        printf("Usage: %S <tickets | tgt | purge | get> [service principal name(for get)]\n",argv[0]);
    }

    if (LogonHandle != NULL)
    {
        LsaDeregisterLogonProcess(LogonHandle);
    }

    return TRUE;
    
}

VOID
PrintKerbName(
    PKERB_EXTERNAL_NAME Name
    )
{
    ULONG Index;
    for (Index = 0; Index < Name->NameCount ; Index++ )
    {
        printf("%wZ",&Name->Names[Index]);
    if ((Index+1) < Name->NameCount)
	    printf("/");
    }
    printf("\n");
}

VOID 
PrintTime(
    LPSTR Comment,
    TimeStamp ConvertTime
    )
{

    printf( "%s", Comment );

    //
    // If the time is infinite,
    //  just say so.
    //
    if ( ConvertTime.HighPart == 0x7FFFFFFF && ConvertTime.LowPart == 0xFFFFFFFF ) {
        printf( "Infinite\n" );

    //
    // Otherwise print it more clearly
    //
    } else {

        SYSTEMTIME SystemTime;
        FILETIME LocalFileTime;

        if( FileTimeToLocalFileTime(
                (PFILETIME) &ConvertTime,
                &LocalFileTime
                ) &&
            FileTimeToSystemTime(
                &LocalFileTime,
                &SystemTime
                ) )
        {

            printf( "%ld/%ld/%ld %ld:%2.2ld:%2.2ld\n",
                    SystemTime.wMonth,
                    SystemTime.wDay,
                    SystemTime.wYear,
                    SystemTime.wHour,
                    SystemTime.wMinute,
                    SystemTime.wSecond );
        }
        	else
	    {
	        printf( "%ld\n", (long)(ConvertTime.QuadPart/(10*1000*1000)));
	    }
    }

}

VOID 
PrintEType(
    int etype
    )
{

#define AddEtype(n) { n, TEXT(#n) }

    struct _etype {
        int etype;
        LPTSTR ename;
    } enames[] = {
        AddEtype(KERB_ETYPE_NULL),
        AddEtype(KERB_ETYPE_DES_CBC_CRC),
        AddEtype(KERB_ETYPE_DES_CBC_MD4),
        AddEtype(KERB_ETYPE_DES_CBC_MD5),
        AddEtype(KERB_ETYPE_DES_PLAIN),
        AddEtype(KERB_ETYPE_RC4_MD4),
        AddEtype(KERB_ETYPE_RC4_PLAIN2),
        AddEtype(KERB_ETYPE_RC4_LM),
        AddEtype(KERB_ETYPE_RC4_SHA),
        AddEtype(KERB_ETYPE_DES_PLAIN),
        AddEtype(KERB_ETYPE_RC4_HMAC_OLD),
        AddEtype(KERB_ETYPE_RC4_PLAIN_OLD),
        AddEtype(KERB_ETYPE_RC4_HMAC_OLD_EXP),
        AddEtype(KERB_ETYPE_RC4_PLAIN_OLD_EXP),
        AddEtype(KERB_ETYPE_RC4_PLAIN),
        AddEtype(KERB_ETYPE_RC4_PLAIN_EXP),
        AddEtype(KERB_ETYPE_DSA_SIGN),
        AddEtype(KERB_ETYPE_RSA_PRIV),
        AddEtype(KERB_ETYPE_RSA_PUB),
        AddEtype(KERB_ETYPE_RSA_PUB_MD5),
        AddEtype(KERB_ETYPE_RSA_PUB_SHA1),
        AddEtype(KERB_ETYPE_PKCS7_PUB),
        AddEtype(KERB_ETYPE_DES_CBC_MD5_NT),
        AddEtype(KERB_ETYPE_RC4_HMAC_NT),
        AddEtype(KERB_ETYPE_RC4_HMAC_NT_EXP),
        {-1, 0}
    };
    int i;

    for (i = 0; enames[i].ename != 0; i++) {
    if (etype == enames[i].etype) {
        printf("KerbTicket Encryption Type: (%d) %S\n",
           etype,
           enames[i].ename);
        return;
    }
    }
    printf("KerbTicket Encryption Type: %d\n", etype);
}


VOID
PrintTktFlags(
    ULONG flags
    )
{
    if (flags & KERB_TICKET_FLAGS_forwardable) {
        printf("forwardable ");
    }
    if (flags & KERB_TICKET_FLAGS_forwarded) {
        printf("forwarded ");
    }
    if (flags & KERB_TICKET_FLAGS_proxiable) {
        printf("proxiable ");
    }
    if (flags & KERB_TICKET_FLAGS_proxy) {
        printf("proxy ");
    }
    if (flags & KERB_TICKET_FLAGS_may_postdate) {
        printf("may_postdate ");
    }
    if (flags & KERB_TICKET_FLAGS_postdated) {
        printf("postdated ");
    }
    if (flags & KERB_TICKET_FLAGS_invalid) {
        printf("invalid ");
    }
    if (flags & KERB_TICKET_FLAGS_renewable) {
        printf("renewable ");
    }
    if (flags & KERB_TICKET_FLAGS_initial) {
        printf("initial ");
    }
    if (flags & KERB_TICKET_FLAGS_hw_authent) {
        printf("hw_auth ");
    }
    if (flags & KERB_TICKET_FLAGS_pre_authent) {
        printf("preauth ");
    }
    if (flags & KERB_TICKET_FLAGS_ok_as_delegate) {
        printf("delegate ");
    }
    printf("\n");
}

BOOL 
PackageConnectLookup(
    HANDLE *pLogonHandle, 
    ULONG *pPackageId
    )
{
    LSA_STRING Name;
    NTSTATUS Status;

    Status = LsaConnectUntrusted(
                pLogonHandle
                );

    if (!SEC_SUCCESS(Status))
    {
        ShowNTError("LsaConnectUntrusted", Status);
        return FALSE;
    }

    Name.Buffer = MICROSOFT_KERBEROS_NAME_A;
    Name.Length = strlen(Name.Buffer);
    Name.MaximumLength = Name.Length + 1;

    Status = LsaLookupAuthenticationPackage(
                *pLogonHandle,
                &Name,
                pPackageId
                );

    if (!SEC_SUCCESS(Status))
    {
        ShowNTError("LsaLookupAuthenticationPackage", Status);
        return FALSE;
    }

    return TRUE;

}

BOOL 
PurgeTicket(
    HANDLE LogonHandle,
    ULONG PackageId,
    LPWSTR Server, 
    DWORD  cbServer,
    LPWSTR Realm,
    DWORD  cbRealm
    )
{
    NTSTATUS Status;
    PVOID Response;
    ULONG ResponseSize;
    NTSTATUS SubStatus=0;

    PKERB_PURGE_TKT_CACHE_REQUEST pCacheRequest = NULL;

    pCacheRequest = (PKERB_PURGE_TKT_CACHE_REQUEST)
        LocalAlloc(LMEM_ZEROINIT, 
        cbServer + cbRealm + sizeof(KERB_PURGE_TKT_CACHE_REQUEST));

    pCacheRequest->MessageType = KerbPurgeTicketCacheMessage;
    pCacheRequest->LogonId.LowPart = 0;
    pCacheRequest->LogonId.HighPart = 0;

    CopyMemory((LPBYTE)pCacheRequest+sizeof(KERB_PURGE_TKT_CACHE_REQUEST),
        Server,cbServer);
    CopyMemory((LPBYTE)pCacheRequest+sizeof(KERB_PURGE_TKT_CACHE_REQUEST)+cbServer,
        Realm,cbRealm);

    pCacheRequest->ServerName.Buffer = 
        (LPWSTR)((LPBYTE)pCacheRequest+sizeof(KERB_PURGE_TKT_CACHE_REQUEST));
    
    pCacheRequest->ServerName.Length = 
        (unsigned short)cbServer;
    
    pCacheRequest->ServerName.MaximumLength = 
        (unsigned short)cbServer;
    
    pCacheRequest->RealmName.Buffer = 
        (LPWSTR)((LPBYTE)pCacheRequest+sizeof(KERB_PURGE_TKT_CACHE_REQUEST)+cbServer);
    
    pCacheRequest->RealmName.Length = 
        (unsigned short)cbRealm;
    
    pCacheRequest->RealmName.MaximumLength = 
        (unsigned short)cbRealm;

    printf("\tDeleting ticket: \n");
    printf("\t   ServerName = %wZ (cb=%lu)\n",&pCacheRequest->ServerName,cbServer);
    printf("\t   RealmName  = %wZ (cb=%lu)\n",&pCacheRequest->RealmName,cbRealm);

    Status = LsaCallAuthenticationPackage(
                LogonHandle,
                PackageId,
                pCacheRequest,
                sizeof(KERB_PURGE_TKT_CACHE_REQUEST)+cbServer+cbRealm,
                &Response,
                &ResponseSize,
                &SubStatus
                );

    if (!SEC_SUCCESS(Status) || !SEC_SUCCESS(Status))
    {
        ShowNTError("LsaCallAuthenticationPackage(purge)", Status);
        printf("Substatus: 0x%x\n",SubStatus);
        ShowNTError("LsaCallAuthenticationPackage(purge SubStatus)", SubStatus);
        return FALSE;
    }
    else 
    {
        printf("\tTicket purged!\n");
        return TRUE;
    }

}


BOOL 
ShowTickets(
    HANDLE LogonHandle,
    ULONG PackageId,
    DWORD dwMode
    )
{
    NTSTATUS Status;
    KERB_QUERY_TKT_CACHE_REQUEST CacheRequest;
    PKERB_QUERY_TKT_CACHE_RESPONSE CacheResponse = NULL;
    ULONG ResponseSize;
    NTSTATUS SubStatus;
    ULONG Index;
    int ch;

    CacheRequest.MessageType = KerbQueryTicketCacheMessage;
    CacheRequest.LogonId.LowPart = 0;
    CacheRequest.LogonId.HighPart = 0;

    Status = LsaCallAuthenticationPackage(
                LogonHandle,
                PackageId,
                &CacheRequest,
                sizeof(CacheRequest),
                (PVOID *) &CacheResponse,
                &ResponseSize,
                &SubStatus
                );
    if (!SEC_SUCCESS(Status) || !SEC_SUCCESS(SubStatus))
    {
        ShowNTError("LsaCallAuthenticationPackage", Status);
        printf("Substatus: 0x%x\n",SubStatus);
        return FALSE;
    }

    printf("\nCached Tickets: (%lu)\n", CacheResponse->CountOfTickets);
    for (Index = 0; Index < CacheResponse->CountOfTickets ; Index++ )
    {
        printf("\n   Server: %wZ@%wZ\n",
            &CacheResponse->Tickets[Index].ServerName,
            &CacheResponse->Tickets[Index].RealmName);
        printf("      ");
        PrintEType(CacheResponse->Tickets[Index].EncryptionType);
        PrintTime("      End Time: ",CacheResponse->Tickets[Index].EndTime);
        PrintTime("      Renew Time: ",CacheResponse->Tickets[Index].RenewTime);
        printf("      TicketFlags: (0x%x) ", CacheResponse->Tickets[Index].TicketFlags);
        PrintTktFlags(CacheResponse->Tickets[Index].TicketFlags);
        printf("\n");

        if(dwMode == INTERACTIVE_PURGE)
        {
            printf("Purge? (y/n/q) : ");
            ch = _getche();
            if(ch == 'y' || ch == 'Y')
            {
                printf("\n");
                PurgeTicket( 
                    LogonHandle,
                    PackageId,
                    CacheResponse->Tickets[Index].ServerName.Buffer,
                    CacheResponse->Tickets[Index].ServerName.Length,
                    CacheResponse->Tickets[Index].RealmName.Buffer,
                    CacheResponse->Tickets[Index].RealmName.Length
                    );
            }
            else if(ch == 'q' || ch == 'Q')
                goto cleanup;
            else
                printf("\n\n");

        }
    }

cleanup:

    if (CacheResponse != NULL)
    {
        LsaFreeReturnBuffer(CacheResponse);
    }

    return TRUE;
}

BOOL 
ShowTgt(
    HANDLE LogonHandle,
    ULONG PackageId
    )
{
    NTSTATUS Status;
    KERB_QUERY_TKT_CACHE_REQUEST CacheRequest;
	PKERB_RETRIEVE_TKT_RESPONSE TicketEntry = NULL;
    PKERB_EXTERNAL_TICKET Ticket;
    ULONG ResponseSize;
    NTSTATUS SubStatus;
    BOOLEAN Trusted = TRUE;

    CacheRequest.MessageType = KerbRetrieveTicketMessage;
    CacheRequest.LogonId.LowPart = 0;
    CacheRequest.LogonId.HighPart = 0;

    Status = LsaCallAuthenticationPackage(
                LogonHandle,
                PackageId,
                &CacheRequest,
                sizeof(CacheRequest),
                (PVOID *) &TicketEntry,
                &ResponseSize,
                &SubStatus
                );

    if (!SEC_SUCCESS(Status) || !SEC_SUCCESS(SubStatus))
    {
        ShowNTError("LsaCallAuthenticationPackage", Status);
        printf("Substatus: 0x%x\n",SubStatus);
        return FALSE;
    }

    Ticket = &(TicketEntry->Ticket);

    printf("\nCached TGT:\n\n");

    printf("ServiceName: "); PrintKerbName(Ticket->ServiceName);

    printf("TargetName: "); PrintKerbName(Ticket->TargetName);

    printf("FullServiceName: "); PrintKerbName(Ticket->ClientName);

    printf("DomainName: %.*S\n",
        Ticket->DomainName.Length/sizeof(WCHAR),Ticket->DomainName.Buffer);

    printf("TargetDomainName: %.*S\n",
        Ticket->TargetDomainName.Length/sizeof(WCHAR),Ticket->TargetDomainName.Buffer);

    printf("AltTargetDomainName: %.*S\n",
        Ticket->AltTargetDomainName.Length/sizeof(WCHAR),Ticket->AltTargetDomainName.Buffer);
    
    printf("TicketFlags: (0x%x) ",Ticket->TicketFlags);
    PrintTktFlags(Ticket->TicketFlags);
    PrintTime("KeyExpirationTime: ",Ticket->KeyExpirationTime);
    PrintTime("StartTime: ",Ticket->StartTime);
    PrintTime("EndTime: ",Ticket->EndTime);
    PrintTime("RenewUntil: ",Ticket->RenewUntil);
    PrintTime("TimeSkew: ",Ticket->TimeSkew);
    PrintEType(Ticket->SessionKey.KeyType);


    if (TicketEntry != NULL)
    {
        LsaFreeReturnBuffer(TicketEntry);
    }

    return TRUE;
}

DWORD 
GetEncodedTicket(
    HANDLE LogonHandle,
    ULONG PackageId,
    wchar_t *Server
    )
{
    NTSTATUS Status;
    PKERB_RETRIEVE_TKT_REQUEST CacheRequest = NULL;
    PKERB_RETRIEVE_TKT_RESPONSE CacheResponse = NULL;
    PKERB_EXTERNAL_TICKET Ticket;
    ULONG ResponseSize;
    NTSTATUS SubStatus;
    BOOLEAN Trusted = TRUE;
    BOOLEAN Success = FALSE;
    UNICODE_STRING Target = {0}; 
    UNICODE_STRING Target2 = {0};

    InitUnicodeString( &Target2, Server);

    CacheRequest = (PKERB_RETRIEVE_TKT_REQUEST)
            LocalAlloc(LMEM_ZEROINIT, Target2.Length + sizeof(KERB_RETRIEVE_TKT_REQUEST));

    CacheRequest->MessageType = KerbRetrieveEncodedTicketMessage ;
    CacheRequest->LogonId.LowPart = 0;
    CacheRequest->LogonId.HighPart = 0;

 
    Target.Buffer = (LPWSTR) (CacheRequest + 1);
    Target.Length = Target2.Length;
    Target.MaximumLength = Target2.MaximumLength;

    CopyMemory(
        Target.Buffer,
        Target2.Buffer,
        Target2.Length
        );

    CacheRequest->TargetName = Target;

    Status = LsaCallAuthenticationPackage(
                LogonHandle,
                PackageId,
                CacheRequest,
                Target2.Length + sizeof(KERB_RETRIEVE_TKT_REQUEST),
                (PVOID *) &CacheResponse,
                &ResponseSize,
                &SubStatus
                );

    if (!SEC_SUCCESS(Status) || !SEC_SUCCESS(SubStatus))
    {
        ShowNTError("LsaCallAuthenticationPackage", Status);
        printf("Substatus: 0x%x\n",SubStatus);
        ShowNTError("Substatus:", SubStatus);

    }
    else
    {
        Ticket = &(CacheResponse->Ticket);

        printf("\nEncoded Ticket:\n\n");

        printf("ServiceName: "); PrintKerbName(Ticket->ServiceName);

        printf("TargetName: "); PrintKerbName(Ticket->TargetName);

        printf("ClientName: "); PrintKerbName(Ticket->ClientName);
        
        printf("DomainName: %.*S\n",
            Ticket->DomainName.Length/sizeof(WCHAR),Ticket->DomainName.Buffer);

        printf("TargetDomainName: %.*S\n",
            Ticket->TargetDomainName.Length/sizeof(WCHAR),Ticket->TargetDomainName.Buffer);

        printf("AltTargetDomainName: %.*S\n",
            Ticket->AltTargetDomainName.Length/sizeof(WCHAR),Ticket->AltTargetDomainName.Buffer);

        printf("TicketFlags: (0x%x) ",Ticket->TicketFlags);
        PrintTktFlags(Ticket->TicketFlags);
        PrintTime("KeyExpirationTime: ",Ticket->KeyExpirationTime);
        PrintTime("StartTime: ",Ticket->StartTime);
        PrintTime("EndTime: ",Ticket->EndTime);
        PrintTime("RenewUntil: ",Ticket->RenewUntil);
        PrintTime("TimeSkew: ",Ticket->TimeSkew);
        PrintEType(Ticket->SessionKey.KeyType);

        Success = TRUE;
        
    }

    if (CacheResponse)
    {
        LsaFreeReturnBuffer(CacheResponse);
    }
    if (CacheRequest)
    {
        LocalFree(CacheRequest);
    }

    return Success;
}

VOID
InitUnicodeString(
	PUNICODE_STRING DestinationString,
    PCWSTR SourceString OPTIONAL
    )
{
    ULONG Length;

    DestinationString->Buffer = (PWSTR)SourceString;
    if (SourceString != NULL) {
        Length = wcslen( SourceString ) * sizeof( WCHAR );
        DestinationString->Length = (USHORT)Length;
        DestinationString->MaximumLength = (USHORT)(Length + sizeof(UNICODE_NULL));
        }
    else {
        DestinationString->MaximumLength = 0;
        DestinationString->Length = 0;
        }
}

VOID 
ShowLastError(
	LPSTR szAPI, 
	DWORD dwError
	)
{
   #define MAX_MSG_SIZE 256

   static WCHAR szMsgBuf[MAX_MSG_SIZE];
   DWORD dwRes;

   printf("Error calling function %s: %lu\n", szAPI, dwError);

   dwRes = FormatMessage (
      FORMAT_MESSAGE_FROM_SYSTEM,
      NULL,
      dwError,
      MAKELANGID (LANG_ENGLISH, SUBLANG_ENGLISH_US),
      szMsgBuf,
      MAX_MSG_SIZE,
      NULL);
   if (0 == dwRes) {
      printf("FormatMessage failed with %d\n", GetLastError());
      ExitProcess(EXIT_FAILURE);
   }

   printf("%S",szMsgBuf);
}

VOID 
ShowNTError( 
	LPSTR szAPI, 
	NTSTATUS Status 
	) 
{     
    // 
    // Convert the NTSTATUS to Winerror. Then call ShowLastError().     
    // 
    ShowLastError(szAPI, LsaNtStatusToWinError(Status));
} 
