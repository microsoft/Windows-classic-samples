/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1997 - 2002 Microsoft Corporation.  All Rights Reserved.

SecPrint.C

This sample illustrates how to use the low-level access control APIs to
add permissions to a printer.

Note that the specific combination of parameters shown below are not the only
possible combination to set on a printer object. The combination below is the
only combination, however, that will be recognized by the printer control panel
as security privileges that correspond to "print access", "manage documents",
"manage printer", and "full control" (and "no access" on NT4 or earlier.)

Command line arguments: <printer name> <user name> <p|md|mp|f> [d]

        where P  = Print access
              MD = Manage documents
              MP = Manage printers
              F  = Full control <P + MD + MP>

Use <D> to deny specified access to the specified
group or user (otherwise specified access is granted.)

Note: On NT4 or earlier denied 'Full Control' is displayed
as 'No Access' by the Explorer Printer Permissions dialog.
Denying P, MD, or MP on NT4 or earlier will not be viewable
using this dialog eventhough the DACL is valid.

David Mowers (davemo)   1-Jun-98
David McPherson (davemm)   11-Jun-99 (Fixup, Demo Open/Get/SetPrinter, SEH)

--*/
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#define ACCESS_DENY 1
#define ACCESS_ALLOW 2

//
//  FUNCTION: GetPrinterDACL
//
//  PURPOSE: Obtains DACL from specified printer
//
//  RETURN VALUE:
//       TRUE or FALSE
//
//  COMMENTS:
//
BOOL GetPrinterDACL(LPTSTR szPrinterName, PACL *ppACL)
{
   HANDLE            hPrinter = NULL;
   PPRINTER_INFO_3   pPrnInfo3 = NULL;
   PRINTER_DEFAULTS  PrnDefs;
   DWORD             cbNeeded = 0;
   DWORD             cbBuf = 0;
   BOOL              bDaclPres;
   BOOL              bDef;
   BOOL              bRes = FALSE;
   ACL_SIZE_INFORMATION AclInfo;
   PACL              pACL = NULL;

   PrnDefs.DesiredAccess = READ_CONTROL;
   PrnDefs.pDatatype = NULL;
   PrnDefs.pDevMode = NULL;

   __try
   {
      if (!OpenPrinter(szPrinterName, &hPrinter, &PrnDefs))
         __leave;

      // Call GetPrinter twice to get size of printer info structure.

      while (!GetPrinter(hPrinter, 3, (LPBYTE)pPrnInfo3, cbBuf, &cbNeeded))
      {
         if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
         {
            cbBuf = cbNeeded;
            pPrnInfo3 = LocalAlloc(LPTR, cbNeeded);
            if (pPrnInfo3 == NULL)
                    __leave;
         }
         else
            __leave;
      }

      if (!GetPrinter(hPrinter, 3, (LPBYTE)pPrnInfo3, cbBuf, &cbNeeded))
         __leave;

      if (!GetSecurityDescriptorDacl(pPrnInfo3->pSecurityDescriptor,
                                       &bDaclPres, &pACL, &bDef))
         __leave;

      if (!GetAclInformation(pACL, &AclInfo,
                            sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
         __leave;


          // The caller just needs the DACL. So, make a copy of the DACL for the
          // caller and free the printer info structure. The caller must free
          // the allocated memory for this DACL copy.

      *ppACL = LocalAlloc(LPTR, AclInfo.AclBytesInUse);
      if (*ppACL == NULL)
         __leave;

      memcpy(*ppACL, pACL, AclInfo.AclBytesInUse);

      bRes = TRUE;
   }

   __finally
   {
      if (pPrnInfo3 != NULL) LocalFree((HLOCAL)pPrnInfo3);
      if (hPrinter != NULL) ClosePrinter(hPrinter);
   }

   return bRes;
}

//
//  FUNCTION: SetPrinterDACL
//
//  PURPOSE: Applies DACL to specified printer
//
//  RETURN VALUE:
//       TRUE or FALSE
//
//  COMMENTS:
//
BOOL SetPrinterDACL(LPTSTR szPrinterName, PACL pDacl)
{
   HANDLE               hPrinter = NULL;
   PRINTER_INFO_3       pi3;
   PRINTER_DEFAULTS     PrnDefs;
   SECURITY_DESCRIPTOR  NewSD;
   BOOL                 bRes = FALSE;

   PrnDefs.DesiredAccess = READ_CONTROL|WRITE_DAC;
   PrnDefs.pDatatype = NULL;
   PrnDefs.pDevMode = NULL;
   pi3.pSecurityDescriptor = &NewSD;


   __try
   {
      if (!OpenPrinter(szPrinterName, &hPrinter, &PrnDefs))
         __leave;

      if (!InitializeSecurityDescriptor(&NewSD, SECURITY_DESCRIPTOR_REVISION))
         __leave;

      if (!SetSecurityDescriptorDacl(&NewSD, TRUE, pDacl, FALSE))
         __leave;

      if (!SetPrinter(hPrinter, 3, (LPBYTE)&pi3, 0))
         __leave;

      bRes = TRUE;
   }

   __finally
   {
      if (hPrinter != NULL) ClosePrinter(hPrinter);
   }

   return bRes;
}

//
//  FUNCTION: AddAccessRights
//
//  PURPOSE: Applies ACE for access to specified object DACL
//
//  RETURN VALUE:
//       TRUE or FALSE
//
//  COMMENTS:
//
BOOL AddAccessRights(LPTSTR szPrinterName,
                      DWORD dwAccessMask,
                      BYTE bAceFlags,
                      LPTSTR szUserName,
                      BYTE bType)
{
   // User Name and Sid variables
   PSID           pSid = NULL;
   DWORD          cbSid = 0;
   LPTSTR         szDomainName = NULL;
   DWORD          cbDomainName = 0;
   SID_NAME_USE   SidType;

   // ACL variables
   PACL                 pACL = NULL;
   ACL_SIZE_INFORMATION AclInfo;
   DWORD                dwNewAceCount = 0;

   // New ACL variables
   PACL  pNewACL = NULL;
   DWORD dwNewACLSize;

   // Temporary ACE
   PACCESS_ALLOWED_ACE  pTempAce; // structure is same for access denied ace
   UINT                 CurrentAceIndex;

   __try
   {
      // Call LookupAccountName twice, once to just get buffer sizes

      while (!LookupAccountName(NULL, szUserName, pSid, &cbSid,
                                 szDomainName, &cbDomainName, &SidType))
      {
         if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
         {
            pSid = LocalAlloc(LPTR, cbSid);
            szDomainName = LocalAlloc(LPTR, (cbDomainName * sizeof(TCHAR)));
            if (pSid == NULL || szDomainName == NULL)
            {
               _tprintf(TEXT("LocalAlloc failed\n"));
               __leave;
            }
         }
         else
         {
            _tprintf(TEXT("Error %d - LookupAccountName\n"), GetLastError());
            __leave;
         }
      }

      _tprintf(TEXT("Adding ACE for %s\n"), szUserName);

      // Get security DACL for printer

      if (!GetPrinterDACL(szPrinterName, &pACL))
      {
         _tprintf(TEXT("Error %d getting printer DACL\n"), GetLastError());
         __leave;
      }

      // Compute size needed for the new ACL

      if (pACL)  // Get size of old ACL
      {
         if (!GetAclInformation(pACL, &AclInfo, sizeof(ACL_SIZE_INFORMATION),
            AclSizeInformation))
         {
            _tprintf(TEXT("Error %d: GetAclInformation\n"), GetLastError());
            __leave;
         }
      }

      if (pACL)  // Add room for new ACEs
      {
         dwNewACLSize = AclInfo.AclBytesInUse +
                        sizeof(ACCESS_ALLOWED_ACE) +
                        GetLengthSid(pSid) - sizeof(DWORD);
      }
      else
      {
         dwNewACLSize = sizeof(ACCESS_ALLOWED_ACE) +
                        sizeof(ACL) +
                        GetLengthSid(pSid) - sizeof(DWORD);
      }

      // Allocate and setup ACL.

      pNewACL = (PACL)LocalAlloc(LPTR, dwNewACLSize);
      if (pNewACL == NULL)
      {
         _tprintf(TEXT("LocalAlloc failed.\n"));
         __leave;
      }

      if (!InitializeAcl(pNewACL, dwNewACLSize, ACL_REVISION2))
      {
         _tprintf(TEXT("Error %d: InitializeAcl\n"),GetLastError());
         __leave;
      }

      // If new ACE is Access Denied ACE add it to front of new ACL

      if (bType == ACCESS_DENY)
      {
         // Add the access-denied ACE to the new DACL
         if (!AddAccessDeniedAce(pNewACL, ACL_REVISION2, dwAccessMask, pSid))
         {
            _tprintf(TEXT("Error %d: AddAccessDeniedAce\n"), GetLastError());
            __leave;
         }

         dwNewAceCount++;

         // get pointer to ace we just added, so we can change the AceFlags

         if (!GetAce(pNewACL,
                     0, // we know it is the first ace in the Acl
                     &pTempAce))
         {
            _tprintf(TEXT("Error %d: GetAce\n"), GetLastError());
            __leave;
         }

         pTempAce->Header.AceFlags = bAceFlags;
      }

      // If a DACL was present, copy the resident ACEs to the new DACL

      if (pACL)
      {
         // Copy the file's old ACEs to our new ACL

         if (AclInfo.AceCount)
         {
            for (CurrentAceIndex = 0; CurrentAceIndex < AclInfo.AceCount;
                                                         CurrentAceIndex++)
            {
               // Get an ACE

               if (!GetAce(pACL, CurrentAceIndex, &pTempAce))
               {
                  _tprintf(TEXT("Error %d: GetAce\n"), GetLastError());
                  __leave;
               }

               // Check to see if this ACE is identical to one were adding.
               // If it is identical don't copy it over.
               if (!EqualSid((PSID)&(pTempAce->SidStart),pSid) ||
                   (pTempAce->Mask != dwAccessMask)            ||
                   (pTempAce->Header.AceFlags != bAceFlags))
               {
                  // ACE is distinct, add it to the new ACL

                  if (!AddAce(pNewACL, ACL_REVISION, MAXDWORD, pTempAce,
                     ((PACE_HEADER)pTempAce)->AceSize))
                  {
                     _tprintf(TEXT("Error %d: AddAce\n"), GetLastError());
                     __leave;
                  }

                  dwNewAceCount++;
               }
            }
         }
      }

      // If the new ACE is an Access allowed ACE add it at the end.

      if (bType == ACCESS_ALLOW)
      {
         // Add the access-allowed ACE to the new DACL

         if (!AddAccessAllowedAce(pNewACL, ACL_REVISION2, dwAccessMask, pSid))
         {
            _tprintf(TEXT("Error %d: AddAccessAllowedAce\n"), GetLastError());
            __leave;
         }

         // Get pointer to ACE we just added, so we can change the AceFlags

         if (!GetAce(pNewACL,
                     dwNewAceCount, // Zero based position of the last ace
                     &pTempAce))
         {
            _tprintf(TEXT("Error %d: GetAce\n"), GetLastError());
            __leave;
         }

         pTempAce->Header.AceFlags = bAceFlags;
      }

      // Set the DACL to the object

      if (!SetPrinterDACL(szPrinterName, pNewACL))
      {
         _tprintf(TEXT("Error %d setting printer DACL\n"), GetLastError());
         __leave;
      }
   }

   __finally
   {
      // Free the memory allocated for the old and new ACL and user info
      if (pACL != NULL) LocalFree((HLOCAL)pACL);
      if (pNewACL != NULL) LocalFree((HLOCAL)pNewACL);
      if (pSid != NULL) LocalFree(pSid);
      if (szDomainName != NULL) LocalFree(szDomainName);
   }

   return(TRUE);
}

void _tmain(INT argc, TCHAR *argv[])
{
   BOOL bDeny = FALSE;

   if (argc < 4)
   {
      _tprintf(TEXT("USAGE: %s <printer name> <user name> <P|MD|MP|F> [D]\n"), argv[0]);
      _tprintf(TEXT(" Where P  = Print access\n"));
      _tprintf(TEXT("       MD = Manage documents\n"));
      _tprintf(TEXT("       MP = Manage printers\n"));
      _tprintf(TEXT("       F  = Full control <P + MD + MP>\n"));
      _tprintf(TEXT("\nUse <D> to deny specified access to the specified\n"));
      _tprintf(TEXT("group or user (otherwise specified access is granted.)\n"));
      _tprintf(TEXT("\nNote: On NT4 or earlier denied 'Full Control' is displayed\n"));
      _tprintf(TEXT("as 'No Access' by the Explorer Printer Permissions dialog.\n"));
      _tprintf(TEXT("Denying P, MD, or MP on NT4 or earlier will not be viewable\n"));
      _tprintf(TEXT("using this dialog eventhough the DACL is valid.\n"));

      exit(0);
   }

   if (argc >= 5 && (lstrcmpi(argv[4], TEXT("d")) == 0))
      bDeny = TRUE;

   if (!lstrcmpi(argv[3], TEXT("p")))
   {
      // Add an ACL for "Print" access

      if (!AddAccessRights(argv[1],
                           PRINTER_EXECUTE,
                           CONTAINER_INHERIT_ACE,
                           argv[2],
                           (BYTE)(bDeny ? ACCESS_DENY : ACCESS_ALLOW)))

         _tprintf(TEXT("Error Adding Access Rights\n"));
   }

   else if (!lstrcmpi(argv[3], TEXT("f")))
   {
      // Add ACLs for "Full Control" access

      if (!AddAccessRights(argv[1],
                           GENERIC_ALL,
                           OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE,
                           argv[2],
                           (BYTE)(bDeny ? ACCESS_DENY : ACCESS_ALLOW)))

         _tprintf(TEXT("Error Adding Access Rights\n"));

      if (!AddAccessRights(argv[1],
                           PRINTER_ALL_ACCESS,
                           CONTAINER_INHERIT_ACE,
                           argv[2],
                           (BYTE)(bDeny ? ACCESS_DENY : ACCESS_ALLOW)))

         _tprintf(TEXT("Error Adding Access Rights\n"));
   }

   else if (!lstrcmpi(argv[3], TEXT("md")))
   {
      // Add ACLs for "Manage Documents" access

      if (!AddAccessRights(argv[1],
                           GENERIC_ALL,
                           OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE,
                           argv[2],
                           (BYTE)(bDeny ? ACCESS_DENY : ACCESS_ALLOW)))

         _tprintf(TEXT("Error Adding Access Rights\n"));

      if (!AddAccessRights(argv[1],
                           READ_CONTROL,
                           CONTAINER_INHERIT_ACE,
                           argv[2],
                           (BYTE)(bDeny ? ACCESS_DENY : ACCESS_ALLOW)))

         _tprintf(TEXT("Error Adding Access Rights\n"));
   }
   else if (!lstrcmpi(argv[3], TEXT("mp")))
   {
      // Add ACLs for "Manage Printers" access

      if (!AddAccessRights(argv[1],
                           PRINTER_ACCESS_ADMINISTER|PRINTER_ACCESS_USE,
                           0,
                           argv[2],
                           (BYTE)(bDeny ? ACCESS_DENY : ACCESS_ALLOW)))

         _tprintf(TEXT("Error Adding Access Rights\n"));
   }
   else
      _tprintf(TEXT("Unknown Access type\n"));
}
