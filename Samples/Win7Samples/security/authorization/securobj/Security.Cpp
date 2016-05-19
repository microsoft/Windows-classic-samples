/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1997 - 2000.  Microsoft Corporation.  All rights reserved.


   Module Name:

       main.cpp

   Description:

       This module contains all the security specific functions:

         - Creating the private object SDs
         - Generating and managing user tokens
         - AccessCheck

--*/

#define UNICODE
#define _UNICODE


#include <aclapi.h>
#include "aclui.h"
#include "Main.h"
#include "resource.h"

//
// Global variables for our logon code
//
#define MAX_LOGONS   16
#define MAX_NAME     64
#define MAX_PASSWORD 64

typedef struct _tagLogon {
   HANDLE hLogon;
   HANDLE hImpToken;
   WCHAR   szUsername[MAX_NAME];
   WCHAR   szDomain[MAX_NAME];
   PSECURITY_DESCRIPTOR psdCreator;
} LOGON,*PLOGON;

LOGON Logons[MAX_LOGONS];

static WORD wLogonId;


//
// define our generic mapping structure for our private objects
//
GENERIC_MAPPING ObjMap =
{
    ACCESS_READ,
    ACCESS_MODIFY,
    0,
    ACCESS_ALL
};

#define SD_SIZE (65536 + SECURITY_DESCRIPTOR_MIN_LENGTH)

BOOL CheckAccess( POBJECT pObject, HANDLE hToken )
{
   DWORD                dwAccessDesired;
   PRIVILEGE_SET        PrivilegeSet;
   DWORD                dwPrivSetSize;
   DWORD                dwAccessGranted;
   BOOL                 fAccessGranted = FALSE;
   WCHAR                buffer[2048];

   if( !IsValidSecurityDescriptor( pObject->pSD ) )
   {
      return( FALSE );
   }

   //
   // Using AccessCheck, there are two different things we could
   // do:
   //

   //
   // 1) See if I have some access rights to the object
   //
   dwAccessDesired = ACCESS_READ;

   //
   // This only does something if we specify generic access rights
   // like GENERIC_ALL. We are not.
   //
   MapGenericMask( &dwAccessDesired, &ObjMap );

   dwPrivSetSize = sizeof( PRIVILEGE_SET );

   if( !AccessCheck( pObject->pSD,
                     hToken,
                     dwAccessDesired,
                     &ObjMap,
                     &PrivilegeSet,
                     &dwPrivSetSize,
                     &dwAccessGranted,
                     &fAccessGranted ) )

      swprintf_s( buffer, 2048, L"Error in AccessCheck : %lu\n", GetLastError() );
   else
   {
      if( fAccessGranted )
         swprintf_s( buffer, 2048, L"Access Was Granted Using Mask 0x%08lx\n", dwAccessGranted );
      else
         swprintf_s( buffer, 2048, L"Access was NOT granted!\n" );
   }

   MessageBox( NULL, buffer, L"AccessCheck Results (is access granted)", MB_OK );

   // 2) See what the maximum rights I have are

   dwAccessDesired = MAXIMUM_ALLOWED;

   if( !AccessCheck( pObject->pSD,
                  hToken,
                  dwAccessDesired,
                  &ObjMap,
                  &PrivilegeSet,
                  &dwPrivSetSize,
                  &dwAccessGranted,
                  &fAccessGranted ) )

      swprintf_s( buffer, 2048, L"Error in AccessCheck : %lu\n", GetLastError() );
   else
   {
      if( fAccessGranted )
         swprintf_s( buffer, 2048, L"Maximum Access Allowed = 0x%08lx\n", dwAccessGranted );
   }

   MessageBox( NULL, buffer, L"AccessCheck Results (maximum access)", MB_OK );

   return( TRUE );

}

//
// Build a default SD for the object creator
//
PSECURITY_DESCRIPTOR GetCreatorSD(WORD wIndex)
{
   PSID pAdministratorsSid = NULL;
   EXPLICIT_ACCESS   ea;
   PACL              pOldAcl=NULL;
   PACL              pNewAcl=NULL;
   DWORD             dwResult;
   PSECURITY_DESCRIPTOR psdNewSD;
   SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
   WCHAR             buffer[1024];


   psdNewSD = LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH);

   if (NULL == psdNewSD)
   {
       return NULL;
   }

   if(!InitializeSecurityDescriptor(psdNewSD,SECURITY_DESCRIPTOR_REVISION))
   {
      swprintf_s(buffer, 1024, L"Error %d:InitializeSecurityDescriptor\n", GetLastError());
      OutputDebugString(buffer);
   }

   ea.grfAccessPermissions = ACCESS_ALL;
   ea.grfAccessMode = GRANT_ACCESS;
   ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
   ea.Trustee.pMultipleTrustee = NULL;
   ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
   ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
   ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
   GetLogonName(wIndex,buffer,1024);
   ea.Trustee.ptstrName = buffer;


   dwResult = SetEntriesInAcl( 1,
                               &ea,
                               pOldAcl,
                               &pNewAcl );
   if( dwResult != ERROR_SUCCESS )
   {
      swprintf_s( buffer, 1024, L"SetEntriesInAcl failed: %lu\n", dwResult );
      OutputDebugString( buffer );
   }


   pOldAcl=pNewAcl;

  if(!AllocateAndInitializeSid(
        &sia,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pAdministratorsSid
        )) {
        swprintf_s( buffer, 1024, L"AllocateAndInitializeSid failed: %lu\n", GetLastError() );
        OutputDebugString( buffer );

   }

   ea.grfAccessPermissions = ACCESS_ALL;
   ea.grfAccessMode = GRANT_ACCESS;
   ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
   ea.Trustee.pMultipleTrustee = NULL;
   ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
   ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
   ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
   ea.Trustee.ptstrName = (WCHAR *)pAdministratorsSid;


   dwResult = SetEntriesInAcl( 1,
                               &ea,
                               pOldAcl,
                               &pNewAcl );
   if( dwResult != ERROR_SUCCESS )
   {
      swprintf_s( buffer, 1024, L"SetEntriesInAcl failed: %lu\n", dwResult );
      OutputDebugString( buffer );
   }



   dwResult = SetSecurityDescriptorDacl( psdNewSD,
                                         TRUE,
                                         pNewAcl,
                                         FALSE );
   if(!dwResult)
   {
      swprintf_s( buffer, 1024, L"SetSecurityDescriptor failed: %lu\n", GetLastError() );
      OutputDebugString( buffer );
   }


//   LocalFree( pNewAcl ); // Second Acl produced
//   LocalFree( pOldAcl ); // First Acl produced

   return( psdNewSD );

}

//
// Build a new object based on the Parent object and Creator
//
POBJECT NewObject( POBJECT pobParent, BOOL fContainer, WORD wIndex )
{

   DWORD dwResult;
   POBJECT pobNew;

   pobNew = (POBJECT)LocalAlloc( LPTR, sizeof( OBJECT ) );

   if (NULL == pobNew)
   {
       return NULL;
   }

   pobNew->fContainer = fContainer;

   //
   // Create a new Private Object Security Descriptor
   //

   dwResult = CreatePrivateObjectSecurity(
      pobParent ? pobParent->pSD : NULL, // if there is a parent object, use it's sd
      pobParent ? NULL : GetClientDescriptor(wIndex), // if there is no parent, use the base sd
      &(pobNew->pSD),
      pobNew->fContainer,
      GetClientToken(wIndex),
      &ObjMap );

   if( !dwResult )
   {
          MessageBox(NULL, L"CreatePrivateObjectSecurity failed!", L"Error",MB_OK);
      return( FALSE );
   }

   return( pobNew );

}

//
// Get a user's token from the list
//
HANDLE GetClientToken( WORD wIndex )
{
   return( Logons[wIndex].hLogon );
}

//
// Get the impersonation token instead
//
HANDLE GetClientImpToken( WORD wIndex )
{
   return( Logons[wIndex].hImpToken );
}

//
// What is this client's name
//
void GetLogonName( WORD wIndex, WCHAR szUser[], SIZE_T cchUser )
{
   swprintf_s(szUser, cchUser, L"%s\\%s", Logons[wIndex].szDomain,Logons[wIndex].szUsername);
}

//
// Get a clients Security Descriptor
//
PSECURITY_DESCRIPTOR GetClientDescriptor( WORD wIndex )
{
   return( Logons[wIndex].psdCreator );
}

//
// Do a new logon for a new client
//
BOOL NewLogon( HINSTANCE hAppInstance, WORD wCurrentLogons )
{
   INT_PTR wResult;

   wLogonId = wCurrentLogons;

   wResult = DialogBox( hAppInstance,
                        MAKEINTRESOURCE(IDD_LOGON),
                        NULL,
                        (DLGPROC)LogonDlgProc );

   if( wLogonId != wCurrentLogons )
   {
      // we had a successfull logon
      return TRUE;
   }
   else
      return FALSE;
}

//
// Our default user is the owner of this process
//
BOOL DefaultUser( )
{
   HANDLE hToken;
   HANDLE hImpToken;
   WCHAR InfoBuffer[1000],szAccountName[MAX_NAME], szDomainName[MAX_NAME];
   DWORD dwInfoBufferSize,dwAccountSize = MAX_NAME, dwDomainSize = MAX_NAME;

   PTOKEN_USER pTokenUser = (PTOKEN_USER)InfoBuffer;
   SID_NAME_USE snu;

   // this is our first user
   wLogonId = 0;

   OpenProcessToken( GetCurrentProcess(),
                     TOKEN_ALL_ACCESS,
                     &hToken );

   GetTokenInformation(hToken,TokenUser,InfoBuffer,
      1000, &dwInfoBufferSize);

   LookupAccountSid(NULL, pTokenUser->User.Sid, szAccountName,
      &dwAccountSize,szDomainName, &dwDomainSize, &snu);

   wcscpy_s( Logons[wLogonId].szUsername, MAX_NAME, szAccountName );
   wcscpy_s( Logons[wLogonId].szDomain, MAX_NAME, szDomainName );


   Logons[wLogonId].hLogon = hToken;

   DuplicateToken( hToken, SecurityImpersonation, &hImpToken );

   Logons[wLogonId].hImpToken = hImpToken;

   Logons[wLogonId].psdCreator = GetCreatorSD(wLogonId);

   wLogonId++;

   return( TRUE );

}

//////////////////////////////////////////////////////////////////
// Dialog that you use to enter the credentials for
// additional users
//
//////////////////////////////////////////////////////////////////
BOOL CALLBACK LogonDlgProc (HWND hwnd, UINT msg,
                     WPARAM wParam,  LPARAM lParam)
{

   WCHAR szUsername[MAX_NAME];
   WCHAR szDomain[MAX_NAME]=L"";
   WCHAR szPassword[MAX_PASSWORD];
   HANDLE hToken;
   HANDLE hImpToken;
   LPTSTR lpDomain=NULL;
   DWORD dwError;


   switch (msg)
   {

      case WM_INITDIALOG:
         return TRUE ;

      case WM_COMMAND:
         switch (LOWORD(wParam))
         {
            case IDOK:
               GetDlgItemText(hwnd,IDC_USERNAME,szUsername,MAX_NAME);
               GetDlgItemText(hwnd,IDC_PASSWORD,szPassword,MAX_PASSWORD);
               if(GetDlgItemText(hwnd,IDC_DOMAIN,szDomain,MAX_NAME))
                  lpDomain=szDomain;

               if( !LogonUser( szUsername ,
                              lpDomain,
                              szPassword,
                              //LOGON32_LOGON_INTERACTIVE,
                              LOGON32_LOGON_NETWORK, // simulates a clients connection
                              LOGON32_PROVIDER_DEFAULT,
                              &hToken ) )
               {
                  return(dwError=GetLastError());
                  // Error 5 means you need to give yourself "Act as part of OS" privilege
               }
               else
               {
                  wcscpy_s(Logons[wLogonId].szUsername,MAX_NAME,szUsername);
                  wcscpy_s(Logons[wLogonId].szDomain,MAX_NAME,szDomain);
                  Logons[wLogonId].hLogon = hToken;
                  DuplicateToken( hToken, SecurityImpersonation, &hImpToken );
                  Logons[wLogonId].hImpToken = hImpToken;
                  Logons[wLogonId].psdCreator = GetCreatorSD(wLogonId);
                  wLogonId++;
               }
            case IDCANCEL:
               DestroyWindow(hwnd);
               return TRUE;
         }
         break;
      default:
         break;

   }
   return FALSE;

}




