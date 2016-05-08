/* Copyright (c) Microsoft Corporation. All rights reserved. */

#include "windows.h"
#include "radutil.h"
#include "multisz.h"

LPVOID
WINAPI
RadiusAlloc(
   SIZE_T dwBytes
   )
{
   return HeapAlloc(GetProcessHeap(), 0, dwBytes);
}


VOID
WINAPI
RadiusFree(
   LPVOID lpMem
   )
{
   HeapFree(GetProcessHeap(), 0, lpMem);
}


DWORD
WINAPI
RadiusFindFirstIndex(
   PRADIUS_ATTRIBUTE_ARRAY pAttrs,
   DWORD dwAttrType
   )
{
   DWORD dwIndex, dwSize;
   const RADIUS_ATTRIBUTE *pAttr;

   if (pAttrs == NULL)
   {
      return RADIUS_ATTR_NOT_FOUND;
   }

   /* Get the number of attributes in the array */
   dwSize = pAttrs->GetSize(pAttrs);

   /* Iterate through the array ... */
   for (dwIndex = 0; dwIndex < dwSize; ++dwIndex)
   {
      /* ... looking for the first attribute that matches the type. */
      pAttr = pAttrs->AttributeAt(pAttrs, dwIndex);
      if (pAttr->dwAttrType == dwAttrType)
      {
         return dwIndex;
      }
   }

   return RADIUS_ATTR_NOT_FOUND;
}


const RADIUS_ATTRIBUTE*
WINAPI
RadiusFindFirstAttribute(
   PRADIUS_ATTRIBUTE_ARRAY pAttrs,
   DWORD dwAttrType
   )
{
   DWORD dwIndex;

   dwIndex = RadiusFindFirstIndex(pAttrs, dwAttrType);

   if (dwIndex != RADIUS_ATTR_NOT_FOUND)
   {
      return pAttrs->AttributeAt(pAttrs, dwIndex);
   }
   else
   {
      return NULL;
   }
}


DWORD
WINAPI
RadiusReplaceFirstAttribute(
   PRADIUS_ATTRIBUTE_ARRAY pAttrs,
   const RADIUS_ATTRIBUTE* pSrc
   )
{
   DWORD dwIndex;

   if ((pAttrs == NULL) || (pSrc == NULL))
   {
      return ERROR_INVALID_PARAMETER;
   }

   dwIndex = RadiusFindFirstIndex(pAttrs, pSrc->dwAttrType);

   if (dwIndex != RADIUS_ATTR_NOT_FOUND)
   {
      /* It already exists, so overwrite the existing attribute. */
      return pAttrs->SetAt(pAttrs, dwIndex, pSrc);
   }
   else
   {
      /* It doesn't exist, so add it to the end of the array. */
      return pAttrs->Add(pAttrs, pSrc);
   }

}


HRESULT
WINAPI
RadiusExtensionInstall(
    HMODULE hModule,
    LPCWSTR pwszType,
    BOOL fInstall
    )
{
   LONG lResult;
   HKEY hKey;
   DWORD nChar, dwError;
   WCHAR wszFileName[MAX_PATH];
   MULTI_SZ value;
   LPWSTR pwszSelf;

   if ((hModule == NULL) || (pwszType == NULL))
   {
      return ERROR_INVALID_PARAMETER;
   }

   /* Retrieve the full path to the extension DLL. */
   nChar = GetModuleFileNameW(
              hModule,
              wszFileName,
              MAX_PATH
              );
   if (nChar == 0)
   {
      dwError = GetLastError();
      return HRESULT_FROM_WIN32(dwError);
   }

   /* Open or create the parameters key. */
   lResult = RegCreateKeyExW(
                HKEY_LOCAL_MACHINE,
                AUTHSRV_PARAMETERS_KEY_W,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                (KEY_QUERY_VALUE | KEY_SET_VALUE),
                NULL,
                &hKey,
                NULL
                );
   if (lResult != NO_ERROR)
   {
      return HRESULT_FROM_WIN32(lResult);
   }

   /* Read the current registered extensions DLLs. */
   lResult = MultiSzQuery(
                &value,
                hKey,
                pwszType
                );
   switch (lResult)
   {
      case NO_ERROR:
      case ERROR_FILE_NOT_FOUND:
      case ERROR_INVALID_DATA:
         break;

      default:
         RegCloseKey(hKey);
         return HRESULT_FROM_WIN32(lResult);
   }

   /* Is this DLL currently registered? */
   pwszSelf = MultiSzFind(&value, wszFileName);

   if (fInstall)
   {
      if (pwszSelf == NULL)
      {
         /* We're installing and we're not currently registered, so we have to
          * add ourself to the list of DLLs. */
         lResult = MultiSzAppend(&value, wszFileName);
         if (lResult == NO_ERROR)
         {
            lResult = MultiSzSet(&value, hKey, pwszType);
         }
      }
      else
      {
         /* The DLL is already registered -- nothing to do.  */
         lResult = NO_ERROR;
      }
   }
   else
   {
      if (pwszSelf != NULL)
      {
         /* We're uninstalling and we're currently registered, so we have to
          * remove ourself from the list of DLLs. */
         MultiSzErase(&value, pwszSelf);
         if (value.nChar > 1)
         {
            lResult = MultiSzSet(&value, hKey, pwszType);
         }
         else
         {
            lResult = RegDeleteValueW(hKey, pwszType);
         }
      }
      else
      {
         /* The DLL isn't registered -- nothing to do. */
         lResult = NO_ERROR;
      }
   }

   /* Clean up the MULTI_SZ. */
   MultiSzFree(&value);

   return HRESULT_FROM_WIN32(lResult);
}
