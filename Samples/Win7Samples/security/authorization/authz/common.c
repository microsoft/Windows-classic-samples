/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2006 Microsoft Corporation.  All rights reserved.

Module Name:

    common.c

Abstract:

    Pipe and error displaying routines for AuthzSample

--*/

#include<windows.h>
#include<stdio.h>
#include<tchar.h>


//////////////////////////////////////////////////////////////////////
DWORD DisplayAPIError(TCHAR *pszAPI,BOOL bConsole, 
                     BOOL bMsgBox, BOOL bExit)
{
   LPVOID   lpvMessageBuffer;
   TCHAR    szErrMsgBuffer[500];
   DWORD    nWrote;
   DWORD    dwError;

   dwError = GetLastError();
   
   FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, dwError, 
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&lpvMessageBuffer, 0, NULL);

    //... now display this string
    _stprintf_s( szErrMsgBuffer, 500 * sizeof(TCHAR),
               TEXT("ERROR: API = %s.\n"
                    "ERROR CODE = %d.\n"
                    "           = 0x%x.\n"
                    "MESSAGE = %s.\n"), 
               pszAPI, dwError,dwError,(char*)lpvMessageBuffer);

    if (bConsole)
       WriteConsole( GetStdHandle(STD_OUTPUT_HANDLE),szErrMsgBuffer,
                     lstrlen(szErrMsgBuffer),&nWrote,NULL);
    if (bMsgBox)
       MessageBox(GetDesktopWindow(),szErrMsgBuffer,TEXT("Execution Error"), MB_OK);

    OutputDebugString(szErrMsgBuffer);

    // Free the buffer allocated by the system
    LocalFree(lpvMessageBuffer);

    if (bExit)
      ExitProcess(dwError);

    return dwError;
}


void HandleError(DWORD dwErr, TCHAR *pszAPI, BOOL fAPI, BOOL fExit)
{
   if (fAPI) {
      DisplayAPIError(pszAPI,TRUE, TRUE, fExit);
   }
   else
   {
      _tprintf(TEXT("%s"),pszAPI);
      if (dwErr) 
         _tprintf(TEXT("%u\n"),dwErr);
         
      if (fExit)
         ExitProcess(0);
   }
   
   return;
}

BOOL
BuildGenericAccessAcl(
    PACL* ppAcl,
    PDWORD cbAclSize
    )
/*++

    Routine Description
    
      This function builds a Dacl which grants the creator of the objects
      GENERIC_ALL (Full Control) and Everyone GENERIC_READ, GENERIC_WRITE and
      GENERIC_EXECUTE access to the object.

      This Dacl allows for higher security than a NULL Dacl, as this only grants
      the creator/owner write access to the security descriptor, and grants 
      Everyone the ability to "use" the object. This scenario prevents a 
      malevolent user from disrupting service by preventing arbitrary access 
      manipulation.
      
    Arguments
    
      PACL* pAcl - Pointer to buffer for pointer to allocated PACL. Must be
                   freed with LocalFree
      
      PDWORD cbAclSize - Pointer to dword receiving size of acl.
      
    Return value
      
      Bool, true on success, false on error
    
--*/
{
    DWORD dwAclSize;

    SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY siaCreator = SECURITY_CREATOR_SID_AUTHORITY;

    PSID pEveryoneSid = NULL;
    PSID pOwnerSid = NULL;
    BOOL bSuccess = FALSE;

    __try {
       
      //
      // compute size of acl
      //
      dwAclSize = sizeof(ACL) +
        2 * ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
        GetSidLengthRequired( 1 ) + // well-known Everyone Sid
        GetSidLengthRequired( 1 ) ; // well-known Creator Owner Sid
      
      (*ppAcl) = LocalAlloc(0,dwAclSize);
      if(*ppAcl == NULL) {
         HandleError(GetLastError(), TEXT("LocalAlloc"), TRUE, TRUE);
         __leave;
      }
   
      *cbAclSize = dwAclSize;
       
      InitializeAcl(*ppAcl,dwAclSize,ACL_REVISION);
   
      //
      // build well known sids
      //
       
      // build EVERYONE SID
      AllocateAndInitializeSid( &siaWorld, 1, SECURITY_WORLD_RID,
                                0,0,0,0,0,0,0, 
                                &pEveryoneSid
                                );
       
      // build Creator/Owner SID
      AllocateAndInitializeSid( &siaCreator, 1, SECURITY_CREATOR_OWNER_RID,
                                0,0,0,0,0,0,0, 
                                &pOwnerSid
                                );
       
       
      if (!AddAccessAllowedAce( *ppAcl,
                                ACL_REVISION,
                                GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
                                pEveryoneSid
                                )) {
         HandleError(GetLastError(),TEXT("AddAccessAllowedAce"),TRUE,TRUE);
         __leave;
      }
   
       
       
      if (!AddAccessAllowedAce( *ppAcl,
                                ACL_REVISION,
                                GENERIC_ALL,
                                pOwnerSid
                                )) {
         HandleError(GetLastError(),TEXT("AddAccessAllowedAce (2)"),TRUE,TRUE);
         __leave;
      }
       
       
      bSuccess = TRUE;
   }
   __finally {
      if (pEveryoneSid) FreeSid(pEveryoneSid);
      if (pOwnerSid) FreeSid(pOwnerSid);
   }
    
    return bSuccess;
}

BOOL SetupNamedPipe (PHANDLE phPipe, LPTSTR szPipeName)
{
   PACL     pDacl = NULL;
   DWORD    cbDacl;
   SECURITY_ATTRIBUTES sa = {0};
   SECURITY_DESCRIPTOR sd = {0};
   BOOL bSuccess = FALSE;


   __try
   {
      *phPipe = INVALID_HANDLE_VALUE;
      
      if (!InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION))  {
         HandleError(GetLastError(),TEXT("InitializeSecurityDescriptor"),TRUE,TRUE);
         __leave;
      }
      
      if (!BuildGenericAccessAcl(&pDacl,&cbDacl)) {
         HandleError(0,TEXT("Error setting up pipe dacl"),FALSE,TRUE);
         __leave;
      }
      
      if (!SetSecurityDescriptorDacl(&sd,TRUE,pDacl,FALSE)) {
         HandleError(GetLastError(),TEXT("SetSecurityDescriptorDacl"),TRUE,TRUE);
         __leave;         
      }
      
      sa.nLength = sizeof(SECURITY_ATTRIBUTES);
      sa.bInheritHandle = FALSE;
      sa.lpSecurityDescriptor = &sd;
         
      // setup pipe and wait for a connection
      *phPipe = CreateNamedPipe( szPipeName,
                     FILE_FLAG_OVERLAPPED|PIPE_ACCESS_DUPLEX,
                     PIPE_TYPE_MESSAGE |PIPE_READMODE_MESSAGE|PIPE_WAIT,1,0,0,
                     NMPWAIT_USE_DEFAULT_WAIT,&sa);
      if (*phPipe == INVALID_HANDLE_VALUE) {
         HandleError(GetLastError(),TEXT("CreateNamedPipe"),TRUE,TRUE);
         __leave;
      }
         
      bSuccess = TRUE;
   }
   __finally {
      // free mem BuildGenericAccessAcl allocated
      if (pDacl) { LocalFree(pDacl);}
   }
   
   return bSuccess;
}


BOOL WriteToPipe(HANDLE hPipe, BYTE* pData, DWORD cbData)
{
   DWORD       dwErr;
   BOOL        bRet        = TRUE;
   INT         nBytesWrote = 0;

   bRet = WriteFile(hPipe,pData,cbData,&nBytesWrote,NULL);
   if (!bRet)
   {
      dwErr = GetLastError();
      // if ERROR_BROKEN_PIPE or ERROR_NO_DATA then pipe naturally ended
      if ( !( dwErr == ERROR_BROKEN_PIPE || dwErr == ERROR_NO_DATA))
         HandleError(dwErr,TEXT("WriteFile"),TRUE,TRUE);
   }

   return bRet;
}

DWORD ReadFromPipe(HANDLE hPipe, BYTE* pBuffer, DWORD cbBuffer)
{
   DWORD       dwErr;
   BOOL        bRet   = TRUE;
   INT         nBytesRead = 0;


	bRet = ReadFile(hPipe, pBuffer, cbBuffer, &nBytesRead, NULL);
   if (!bRet)
   {
      dwErr = GetLastError();
      // if ERROR_BROKEN_PIPE or ERROR_NO_DATA then pipe naturally ended
      if ( !( dwErr == ERROR_BROKEN_PIPE || dwErr == ERROR_NO_DATA))
         HandleError(dwErr,TEXT("ReadFile"),TRUE,TRUE);
   }
  
   return nBytesRead;
}
