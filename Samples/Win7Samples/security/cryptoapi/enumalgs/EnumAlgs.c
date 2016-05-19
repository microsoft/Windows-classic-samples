/******************************************************************************\
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1996 - 2000.  Microsoft Corporation.  All rights reserved.
\******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <windows.h>
#include <wincrypt.h>

/*****************************************************************************/
void _cdecl main(int argc, char *argv[])
{
   INT iReturn = 0;
   HCRYPTPROV hProv = 0;
   PROV_ENUMALGS EnumAlgs;
   DWORD dwDataLen;
   DWORD dwFlags;
   DWORD dwError = ERROR_SUCCESS;
   CHAR *pszAlgType;
 
   UNREFERENCED_PARAMETER(argc);
   UNREFERENCED_PARAMETER(argv);
	
   // Get handle to the default provider.
   if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0))
   {
      printf("Error %x during CryptAcquireContext!\n", GetLastError());
      goto Error;
   }

   // Set the CRYPT_FIRST flag the first time through the loop.
   dwFlags = CRYPT_FIRST;

   // Set size of data expected
   dwDataLen = sizeof(PROV_ENUMALGS);

   // Enumerate the supported algorithms.
   while (dwError == ERROR_SUCCESS)
   {
      // Retrieve information about an algorithm.
      if (!CryptGetProvParam(hProv, PP_ENUMALGS, (LPBYTE)&EnumAlgs, &dwDataLen, dwFlags))
      {  
		 dwError = GetLastError();
         if (dwError == ERROR_NO_MORE_ITEMS)
         {
            // Exit the loop.
            break;
         }

         printf("Error %x reading algorithm!\n", GetLastError());
         goto Error;
      }

      // Determine algorithm type.
      switch (GET_ALG_CLASS(EnumAlgs.aiAlgid))
      {
      case ALG_CLASS_DATA_ENCRYPT: pszAlgType = "Encrypt";
         break;
      case ALG_CLASS_HASH:         pszAlgType = "Hash";
         break;
      case ALG_CLASS_KEY_EXCHANGE: pszAlgType = "Exchange";
         break;
      case ALG_CLASS_SIGNATURE:    pszAlgType = "Signature";
         break;
      default:                     pszAlgType = "Unknown";
      }

      // Print information about algorithm.
      printf("Name:%-19s  Type:%-9s  Bits:%-4d  Algid:%8.8xh\n",
         EnumAlgs.szName, pszAlgType, EnumAlgs.dwBitLen, EnumAlgs.aiAlgid);

      // Clear the flags for the remaining interations of the loop.
      dwFlags = 0;
   }

Exit:

   // Release CSP handle (if open)
   if (hProv)
   {
      CryptReleaseContext(hProv, 0);
   }

   exit(iReturn);

Error:

   iReturn = 1;
   goto Exit;
}
