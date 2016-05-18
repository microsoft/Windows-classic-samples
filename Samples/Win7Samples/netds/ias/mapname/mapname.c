/* Copyright (c) Microsoft Corporation. All rights reserved. */

#include "windows.h"
#include "authif.h"
#include "lmcons.h"
#include "ntsecapi.h"
#include "radutil.h"

/* The registry value where this extension is registered. */
LPCWSTR pwszDllType = AUTHSRV_EXTENSIONS_VALUE_W;

/* Global handle to the LSA. This is initialized once at start-up and reused
 * until shutdown. */
LSA_HANDLE hPolicy = NULL;


DWORD
WINAPI
RadiusExtensionInit( VOID )
{
   NTSTATUS status;
   LSA_OBJECT_ATTRIBUTES objectAttributes;

   memset(&objectAttributes, 0, sizeof(objectAttributes));
   status = LsaOpenPolicy(
               NULL,
               &objectAttributes,
               POLICY_LOOKUP_NAMES,
               &hPolicy
               );

   return LsaNtStatusToWinError(status);
}


VOID
WINAPI
RadiusExtensionTerm( VOID )
{
   LsaClose(hPolicy);
   hPolicy = NULL;
}


DWORD
WINAPI
RadiusExtensionProcess2(
   PRADIUS_EXTENSION_CONTROL_BLOCK pECB
   )
{
   PRADIUS_ATTRIBUTE_ARRAY pInAttrs;
   const RADIUS_ATTRIBUTE* pAttr;
   int nChar;
   WCHAR wszUserName[UNLEN];
   CHAR szUserName[UNLEN];
   LSA_UNICODE_STRING lusName;
   NTSTATUS status;
   PLSA_REFERENCED_DOMAIN_LIST pReferencedDomains;
   PLSA_TRANSLATED_SID pSids;
   DWORD cbDataLength, dwResult;
   RADIUS_ATTRIBUTE raNewName;

   /* We only process authentication. */
   if (pECB->repPoint != repAuthentication)
   {
      return NO_ERROR;
   }

   /* We only process Access-Requests. */
   if (pECB->rcRequestType != rcAccessRequest)
   {
      return NO_ERROR;
   }

   /* Don't process if it's already been rejected. */
   if (pECB->rcResponseType == rcAccessReject)
   {
      return NO_ERROR;
   }

   /* Get the attributes from the Access-Request. */
   pInAttrs = pECB->GetRequest(pECB);

   /* Only process Windows users */
   pAttr = RadiusFindFirstAttribute(pInAttrs, ratProvider);
   if ((pAttr == NULL) || (pAttr->dwValue != rapWindowsNT))
   {
      return NO_ERROR;
   }

   /* Retrieve the username. If it doesn't exist, there's nothing to do. */
   pAttr = RadiusFindFirstAttribute(pInAttrs, ratUserName);
   if (pAttr == NULL)
   {
      return NO_ERROR;
   }

   /* If the username already includes a domain name or if it's a user
    * principal name (UPN), then we don't have to do anything. */
   if (memchr(pAttr->lpValue, '\\', pAttr->cbDataLength) != NULL)
   {
      return NO_ERROR;
   }
   if (memchr(pAttr->lpValue, '@', pAttr->cbDataLength) != NULL)
   {
      return NO_ERROR;
   }

   /* Convert the username to Unicode. */
   nChar = MultiByteToWideChar(
              CP_ACP,
              MB_ERR_INVALID_CHARS,
              pAttr->lpValue,
              pAttr->cbDataLength,
              wszUserName,
              UNLEN
              );
   if (nChar == 0)
   {
      return GetLastError();
   }

   /* Pack the username into an LSA_UNICODE_STRING struct. */
   lusName.Length = nChar * sizeof(WCHAR);
   lusName.MaximumLength = lusName.Length;
   lusName.Buffer = wszUserName;

   /* Lookup the name. */
   status = LsaLookupNames(
               hPolicy,
               1,
               &lusName,
               &pReferencedDomains,
               &pSids
               );
   if (!LSA_SUCCESS(status))
   {
      return LsaNtStatusToWinError(status);
   }

   /* Get the domain which has the user account */
   lusName = pReferencedDomains->Domains[pSids[0].DomainIndex].Name;

   /* Convert to an ANSI string. */
   nChar = WideCharToMultiByte(
              CP_ACP,
              0,
              lusName.Buffer,
              (lusName.Length / sizeof(WCHAR)),
              szUserName,
              UNLEN,
              NULL,
              NULL
              );

   LsaFreeMemory(pReferencedDomains);
   LsaFreeMemory(pSids);

   if (nChar == 0)
   {
      return GetLastError();
   }

   /* New attribute is the domain plus delimiter plus username */
   cbDataLength = nChar + 1 + pAttr->cbDataLength;
   if (cbDataLength > UNLEN)
   {
      return ERROR_BAD_USERNAME;
   }

   /* Build the fully-qualified username. */
   szUserName[nChar] = '\\';
   memcpy(szUserName + nChar + 1, pAttr->lpValue, pAttr->cbDataLength);

   /* Fill in the RADIUS_ATTRIBUTE struct. */
   raNewName.fDataType = rdtString;
   raNewName.cbDataLength = cbDataLength;
   raNewName.lpValue = szUserName;

   /* Add as both the ratStrippedUserName and the ratFQUserName. */
   raNewName.dwAttrType = ratStrippedUserName;
   dwResult = RadiusReplaceFirstAttribute(pInAttrs, &raNewName);
   if (dwResult == NO_ERROR)
   {
      raNewName.dwAttrType = ratFQUserName;
      dwResult = RadiusReplaceFirstAttribute(pInAttrs, &raNewName);
   }

   return dwResult;
}
