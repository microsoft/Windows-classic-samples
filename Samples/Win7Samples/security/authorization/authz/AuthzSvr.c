/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2006 Microsoft Corporation.  All rights reserved.

Module Name:

    AuthzSvr.c

Abstract:

    Sample Resource Manager.


--*/

#define SECURITY_WIN32

#include<windows.h>
#include<security.h>
#include<lmcons.h>
#include<authz.h>
#include<stdio.h>
#include"common.h"

#define READ_BUFFER_SIZE 512

#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))

DWORD VPSidBuf[] = {0x00000501, 0x05000000, 0x00000015, 0x17b85159, 0x255d7266, 0x0b3b6364, 0x00010001};
DWORD ManagerSidBuf[] = {0x00000501, 0x05000000, 0x00000015, 0x17b85159, 0x255d7266, 0x0b3b6364, 0x00010002};
DWORD EmployeeSidBuf[] = {0x00000501, 0x05000000, 0x00000015, 0x17b85159, 0x255d7266, 0x0b3b6364, 0x00010003};

PSID VPSid = (PSID)VPSidBuf;
PSID ManagerSid = (PSID)ManagerSidBuf;
PSID EmployeeSid = (PSID)EmployeeSidBuf;

DWORD MaxSpendingVP;
DWORD MaxSpendingManager;
DWORD MaxSpendingEmployee;


//
// struct that maintains the state of the fund
// 
typedef struct _FundsRM
{
   // The amount of money available in the fund
   //
   
   DWORD _dwFundsAvailable; 
   
   //
   // The resource manager, initialized with the callback functions
   //
   
   AUTHZ_RESOURCE_MANAGER_HANDLE _hRM;
   
   //
   // The security descriptor for the fund, containing a callback ACE
   // which causes the resource manager callbacks to be used
   //
   
   SECURITY_DESCRIPTOR _SD;

} FUNDSRM,*PFUNDSRM;



//
// The callback routines used with AuthZ
//

BOOL
WINAPI
AuthzAccessCheckCallback(
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
    IN PACE_HEADER pAce,
    IN PVOID pArgs OPTIONAL,
    IN OUT PBOOL pbAceApplicable
    )

/*++

    Routine Description
    
      This is the callback access check. It is registered with a 
      resource manager. AuthzAccessCheck calls this function when it
      encounters a callback type ACE, one of:
      ACCESS_ALLOWED_CALLBACK_ACE_TYPE 
      ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE
      ACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE
      
      This function determines if the given callback ACE applies to the
      client context (which has already had dynamic groups computed) and
      the optional arguments, in this case the request amount.
      
      The list of groups which apply to the user is traversed. If a group
      is found which allows the user the requested access, pbAceApplicable
      is set to true and the function returns. If the end of the group list
      is reached, pbAceApplicable is set to false and the function returns.
        
    Arguments
    
        hAuthzClientContext - handle to the AuthzClientContext.
        
        pAce - pointer to the Ace header.
        
        pArgs - optional arguments, in this case DWORD*, DWORD is  the spending
         request amount in cents
        
        pbAceApplicable - returns true iff the ACE allows the client's request

    Return value
    
        Bool, true on success, false on error
        
      
--*/
{
   
   DWORD dwTokenGroupsSize = 0;
   PVOID pvTokenGroupsBuf = NULL;
   DWORD i;
   PDWORD pAccessMask = NULL;
   DWORD dwRequestedSpending = ((PDWORD)pArgs)[0];
   
   //
   // By default, the ACE does not apply to the request
   //
   
   *pbAceApplicable = FALSE;

   //
   // The object's access mask (right after the ACE_HEADER)
   // The access mask determines types of expenditures allowed
   // from this fund
   //
   
   pAccessMask = (PDWORD) ((PBYTE)pAce + sizeof(ACE_HEADER));
   
 
   //
   // Get the TOKEN_GROUPS array 
   //
   
   while (!AuthzGetInformationFromContext(
                        hAuthzClientContext,
                        AuthzContextInfoGroupsSids,
                        dwTokenGroupsSize,
                        &dwTokenGroupsSize,
                        pvTokenGroupsBuf
                        ))
   {
      if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
      {
         HandleError(GetLastError(),TEXT("AuthzGetInformationFromContext"),TRUE,TRUE);
         break;
      }
      
      // free incase we got size and size grew before our next call.
      if (pvTokenGroupsBuf) free( pvTokenGroupsBuf);
      pvTokenGroupsBuf = malloc(dwTokenGroupsSize);
   }
   
   if (!pvTokenGroupsBuf) return FALSE; // error actually handled in loop
   
   
   //
   // Go through the groups until end is reached or a group applying to the
   // request is found
   //
   
   for( i = 0; 
       i < ((PTOKEN_GROUPS)pvTokenGroupsBuf)->GroupCount 
       && *pbAceApplicable != TRUE;
       i++ ) 
   {
      //
      // This is the business logic.
      // Each level of employee can approve different amounts.
      //
      
      //
      // VP
      //
      
      if(EqualSid(VPSid, ((PTOKEN_GROUPS)pvTokenGroupsBuf)->Groups[i].Sid) && (dwRequestedSpending <= MaxSpendingVP) )
      {
            *pbAceApplicable = TRUE;
      }
      
         
      // 
      // Manager
      //
      
      if(EqualSid(ManagerSid, ((PTOKEN_GROUPS)pvTokenGroupsBuf)->Groups[i].Sid) && (dwRequestedSpending <= MaxSpendingManager) )
      {
            *pbAceApplicable = TRUE;
      }
         
      //
      // Employee
      //
      
      if(EqualSid(EmployeeSid, ((PTOKEN_GROUPS)pvTokenGroupsBuf)->Groups[i].Sid) && (dwRequestedSpending <= MaxSpendingEmployee) )
      {
            *pbAceApplicable = TRUE;
      }
   }


   // return TRUE when access check completed (when a callback ace applies not)
   // If we had a runtime error, such as mem alloc errors we would return false
   return TRUE;
}




BOOL
WINAPI
AuthzComputeGroupsCallback(
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
    IN PVOID Args,
    OUT PSID_AND_ATTRIBUTES *pSidAttrArray,
    OUT PDWORD pSidCount,
    OUT PSID_AND_ATTRIBUTES *pRestrictedSidAttrArray,
    OUT PDWORD pRestrictedSidCount
    )

/*++

    Routine Description
    
        Resource manager callback to compute dynamic groups.  This is used by the RM
        to decide if the specified client context should be included in any RM defined groups.
        
        In this example, the employees are hardcoded into their roles. However, this is the 
        place you would normally retrieve data from an external source to determine the 
        users' additional roles.
        
    Arguments
    
        hAuthzClientContext - handle to client context.
        Args - optional parameter to pass information for evaluating group membership.
        pSidAttrArray - computed group membership SIDs
        pSidCount - count of SIDs
        pRestrictedSidAttrArray - computed group membership restricted SIDs
        pRestrictedSidCount - count of restricted SIDs
        
    Return Value 
        
        Bool, true for success, false on failure.



--*/    
{
   //
   // First, look up the user's SID from the context
   //
   
   DWORD dwSidSize = 0;
   PVOID pvSidBuf = NULL;
   PSID  psSidPtr = NULL;
   TCHAR UserName[256];
   DWORD cbName = 256;
   TCHAR Domain[256];
   DWORD cbDomain = 256;
   SID_NAME_USE eUse;

  
   //
   // Get the SID (inside a TOKEN_USER structure)
   //
   
   while (!AuthzGetInformationFromContext(
                        hAuthzClientContext,
                        AuthzContextInfoUserSid,
                        dwSidSize,
                        &dwSidSize,
                        pvSidBuf
                        )){
      
      if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
         HandleError(GetLastError(),TEXT("AuthzGetInformationFromContext"),TRUE,FALSE);
         break;
      }
      
      if (pvSidBuf) free (pvSidBuf);
      pvSidBuf = malloc(dwSidSize);
   }

   if (!pvSidBuf) return FALSE;
      
   psSidPtr = ((PTOKEN_USER)pvSidBuf)->User.Sid;
   
   //
   // Allocate the memory for the returns, which will be deallocated by FreeDynamicGroups
   // Only a single group will be returned, determining the employee type
   //
   
   *pSidCount = 1;
   *pSidAttrArray = (PSID_AND_ATTRIBUTES)malloc( sizeof(SID_AND_ATTRIBUTES) );
   
   //
   // No restricted group sids
   //
   
   pRestrictedSidCount = 0;
   *pRestrictedSidAttrArray = NULL;
   
   (*pSidAttrArray)[0].Attributes = SE_GROUP_ENABLED;
   
   //
   //       The hardcoded Sample logic:
   // 
   // Lookup the sid to get the username and grant dynamic sid based on username
   //  
   //    Bob is a VP
   //    Martha is a Manager
   //    Joe is an Employee
   //
   
   if (!LookupAccountSid(NULL,psSidPtr,UserName,&cbName,Domain,&cbDomain,&eUse)) {
      HandleError(GetLastError(),TEXT("LookupAccountSid"),TRUE,TRUE);
   }
   
   if( !lstrcmpi(UserName, TEXT("BOB")) )
   {
      (*pSidAttrArray)[0].Sid = VPSid;
   } 
   else if( !lstrcmpi(UserName, TEXT("Martha")) )
   {
      (*pSidAttrArray)[0].Sid = ManagerSid;
   }
   else if( !lstrcmpi(UserName, TEXT("Joe")) )
   {
      (*pSidAttrArray)[0].Sid = EmployeeSid;    
   }
   else
   {
      *pSidCount = 0;
      free(*pSidAttrArray);
      *pSidAttrArray = NULL;
   }

   if (pvSidBuf) free(pvSidBuf);
   
   return TRUE;   
}




VOID
WINAPI
AuthzFreeGroupsCallback(
    IN PSID_AND_ATTRIBUTES pSidAttrArray
    )

/*++

    Routine Description
    
        Frees memory allocated for the dynamic group array.

    Arguments
    
        pSidAttrArray - array to free.
    
    Return Value
        None.                       
--*/        
{
   
    if (pSidAttrArray != NULL)
    {
      free(pSidAttrArray);
    }
    
}




BOOL InitializeFundsRM(PFUNDSRM pFundsRM, DWORD dwFundsAvailable) 
/*++

    Routine Description
    
        Initializes the Authz Resource Manager, providing it
        with the appropriate callback functions.
        It also creates a security descriptor for the fund, allowing only
        corporate and transfer expenditures, not personal. Additional logic
        could be added to allow VPs to override these restrictions, etc.

    Arguments
    
        PFUNDSRM pFundsRM - Pointer to a FUNDSRM struct that maintains the state
                            of the Resource Manager
                                           
        DWORD dwFundsAvailable - The amount of money in the fund managed by this
                         resource manager
    
    Return Value
        None.                       
--*/
{   
   PSID psidEveryone = NULL;
   SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
   PACL pDaclFund = NULL;
       
   MaxSpendingVP = 100000000;
   MaxSpendingManager = 1000000;
   MaxSpendingEmployee = 50000;
      
   //
   // The amount of money in the fund
   //
   
   pFundsRM->_dwFundsAvailable = dwFundsAvailable;
    
   //
   // Initialize the fund's resource manager
   //
   
   if (!AuthzInitializeResourceManager(
        0,    // no flags        
        AuthzAccessCheckCallback,
        AuthzComputeGroupsCallback,
        AuthzFreeGroupsCallback,
        L"SampRM", // no auditing
        &(pFundsRM->_hRM)
        ))
      HandleError(GetLastError(),TEXT("AuthzInitializeResourceManager"),TRUE,TRUE);
   
   else
       printf("Funds Resource Manager initialized - waiting for client\n\n");



   //
   // Create the fund's security descriptor
   // 
   
    if (!InitializeSecurityDescriptor(&(pFundsRM->_SD),SECURITY_DESCRIPTOR_REVISION))
       HandleError(GetLastError(),TEXT("InitializeSecurityDescriptor"),TRUE,TRUE);
       
    if (!SetSecurityDescriptorGroup(&(pFundsRM->_SD), NULL, FALSE))
       HandleError(GetLastError(),TEXT("SetSecurityDescriptorGroup"),TRUE,TRUE);
    
    if (!SetSecurityDescriptorSacl(&(pFundsRM->_SD), FALSE, NULL, FALSE))
       HandleError(GetLastError(),TEXT("SetSecurityDescriptorSacl"),TRUE,TRUE);

   // an owner must be specified.  Since VPs are the highest privileged group
   // this sample we'll make them the owner.
    if (!SetSecurityDescriptorOwner(&(pFundsRM->_SD), VPSid, FALSE))
      HandleError(GetLastError(),TEXT("SetSecurityDescriptorOwner"),TRUE,TRUE);

   //
   // Initialize the DACL for the fund
   //
   
   pDaclFund = (PACL)malloc(1024);
   InitializeAcl(pDaclFund, 1024, ACL_REVISION_DS);
   
   //
   // Add an access-allowed ACE for Everyone
   // Only company spending and transfers are allowed for this fund
   //
   
   // build EVERYONE SID
   if (!AllocateAndInitializeSid(
      &siaWorld, 1, SECURITY_WORLD_RID,
      0,0,0,0,0,0,0,
      &psidEveryone
      ))
      HandleError(GetLastError(),TEXT("AllocateAndInitializeSid"),TRUE,TRUE);

   if (!AddAccessAllowedAce(pDaclFund,
                  ACL_REVISION_DS,
                  ACCESS_FUND_CORPORATE | ACCESS_FUND_TRANSFER, 
                  psidEveryone))
      HandleError(GetLastError(),TEXT("AddAccessAllowedAce"),TRUE,TRUE);
      
   
   FreeSid(psidEveryone);
   
   //
   // Now set that ACE to a callback ACE
   //
   
   ((PACE_HEADER)FirstAce(pDaclFund))->AceType = 
                           ACCESS_ALLOWED_CALLBACK_ACE_TYPE;
   
   //
   // Add that ACL as the security descriptor's DACL
   //
   
   if (!SetSecurityDescriptorDacl(&(pFundsRM->_SD), TRUE, pDaclFund, FALSE))
      HandleError(GetLastError(),TEXT("SetSecurityDescriptorDacl"),TRUE,TRUE);
   
   return TRUE;
}




BOOL AuthorizeAndExecuteExpense (PFUNDSRM pFundsRM,
                                 HANDLE hToken,
                                 EX_BUF exBuf,
                                 PDWORD pdwResult
                                 )
/*++

    Routine Description
    
        Setups the Authz context and makes call to AuthzAccessCheck.  Then 
        modifies the remaining funds depending on acccess and ammount
        
    Arguments
    
        PFUNDSRM pFundsRM - Pointer to a FUNDSRM struct that maintains the state
                            of the Resource Manager
                                           
        HANDLE hToken - The token representing the user were doing the access
                        check for
                        
        EX_BUF exBuf - struct that contains desired access and expense ammount                       
               
        PDWORD pdwResult - Pointer to DWORD to put result/error                                         
    
    Return Value
        bool True if no errors.                       
--*/
{
   AUTHZ_CLIENT_CONTEXT_HANDLE AuthzClient;
   LUID     luid = {0x0,0x0};
   AUTHZ_ACCESS_REQUEST AccessRequest = {0};
   AUTHZ_ACCESS_REPLY AccessReply = {0};
   BYTE     Buffer[1024];
   BOOL bRes = FALSE;  // Assume error

   __try
   {
   
      //
      // first we need an Authz context
      //

      if (!AuthzInitializeContextFromToken(
        0,
        hToken,
        pFundsRM->_hRM,
        NULL,
        luid,
        NULL,
        &AuthzClient )){
         HandleError(GetLastError(),TEXT("AuthzInitializeContextFromToken"),TRUE,FALSE);
         __leave;

      }

      // Do AccessCheck
      AccessRequest.DesiredAccess = exBuf.dwType;
      AccessRequest.PrincipalSelfSid = NULL;
      AccessRequest.ObjectTypeList = NULL;
      AccessRequest.ObjectTypeListLength = 0;
      AccessRequest.OptionalArguments = &(exBuf.dwAmmount); 

      //
      // The ResultListLength is set to the number of ObjectType GUIDs in the Request, indicating
      // that the caller would like detailed information about granted access to each node in the
      // tree.
      //
      RtlZeroMemory(Buffer, sizeof(Buffer));
      AccessReply.ResultListLength = 1;
      AccessReply.GrantedAccessMask = (PACCESS_MASK) (Buffer);
      AccessReply.Error = (PDWORD) (Buffer + sizeof(ACCESS_MASK));


      if (!AuthzAccessCheck( 0,
                             AuthzClient,
                             &AccessRequest,
                             NULL,
                             &(pFundsRM->_SD),
                             NULL,
                             0,
                             &AccessReply,
                             NULL) ) {
         HandleError(GetLastError(),TEXT("AuthzAccessCheck"),TRUE,FALSE);
         __leave;
      }
  
      if (( *(AccessReply.GrantedAccessMask) & exBuf.dwType) != exBuf.dwType) {
         // Access is denied get error from reply if there else Getlasterror.
         *pdwResult = *(AccessReply.Error);
         printf("Access denied\n\n");   
   
      }
      else {
         
         // Access is granted, is there enough funds, this could have been done
         // within the AuthzAccessCheckCallback but since this is more of an
         // execution problem than access checking we'll do it here.
         if (pFundsRM->_dwFundsAvailable < exBuf.dwAmmount) {
            
            // not enough in fund
            *pdwResult = ERROR_INSUFFICIENT_FUNDS;
            printf("Failed : NSF\n\n");   
         
         }
         else {
            pFundsRM->_dwFundsAvailable -= exBuf.dwAmmount;
            *pdwResult = EXPENSE_APPROVED;
            printf("Expense Approved. Remaining funds:%u cents.\n\n",
                                                 pFundsRM->_dwFundsAvailable);   
         }
      }

      bRes = TRUE;
   }

   __finally{
      AuthzFreeContext(AuthzClient);
   }

   return bRes;

}


void main (int argc, char *argv[])
{
   HANDLE   hPipe;
   DWORD    cbClientName = UNLEN + CNLEN + 2;
   TCHAR    ClientName[UNLEN + CNLEN + 2];
   DWORD    dwBytesRead;
   FUNDSRM  FundsRM;
   HANDLE   hToken = INVALID_HANDLE_VALUE;
   EX_BUF   exBuf = {0};
   DWORD    dwResult = ERROR_ACCESS_DENIED;

   InitializeFundsRM(&FundsRM,200000000);
   
   SetupNamedPipe(&hPipe,"\\\\.\\pipe\\AuthzSamplePipe");      


   while(TRUE)
   {
      //
      //  Wait for a clinet...
      //

      if (!ConnectNamedPipe(hPipe, NULL))
         HandleError(GetLastError(),TEXT("ConnectNamedPipe"),TRUE,TRUE);

      dwBytesRead = ReadFromPipe(hPipe,(PBYTE)&exBuf,sizeof(exBuf));
      if ( !dwBytesRead ) {
         printf("Error reading from client\n");
      }

      //
      // Get Token
      //

      if (!ImpersonateNamedPipeClient(hPipe))
         HandleError(GetLastError(),TEXT("ImpersonateNamedPipeClient"),TRUE,TRUE);


      if(!GetUserName(ClientName,&cbClientName))
         HandleError(GetLastError(),TEXT("GetUserName"),TRUE,TRUE);

      
      if (!OpenThreadToken(GetCurrentThread(),
                          TOKEN_QUERY|TOKEN_IMPERSONATE,FALSE,&hToken))         
         HandleError(GetLastError(),TEXT("OpenThreadToken"),TRUE,TRUE);
      
      
      RevertToSelf();
      
      printf("%s requests %s expense of %u cents\n",ClientName, ExNames[exBuf.dwType], exBuf.dwAmmount);
     

      //
      // use token to and context to vaidate - note that this sample uses the token
      // if we just had a user's sid we could build an authz context with that.
      //

      if (!AuthorizeAndExecuteExpense( &FundsRM,
                                       hToken,
                                       exBuf,
                                       &dwResult
                                       )) {
         printf("unknown error executing expense\n");
      }

      WriteToPipe(hPipe, (PBYTE)&dwResult, sizeof(DWORD));

      DisconnectNamedPipe(hPipe);
      ZeroMemory(ClientName,cbClientName);
      CloseHandle(hToken);
      hToken = INVALID_HANDLE_VALUE;
      cbClientName = UNLEN + CNLEN + 2;
   }

   return;
}
