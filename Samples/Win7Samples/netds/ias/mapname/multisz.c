/* Copyright (c) Microsoft Corporation. All rights reserved. */

#include "windows.h"
#include "wchar.h"
#include "multisz.h"
#include "radutil.h"

static wchar_t wszEmptyMultiSz[] = L"\0";

VOID
WINAPI
MultiSzInit(
    PMULTI_SZ pMultiSz
    )
{
   pMultiSz->pwszValue = wszEmptyMultiSz;
   pMultiSz->nChar = 1;
}

VOID
WINAPI
MultiSzFree(
    PMULTI_SZ pMultiSz
    )
{
   if (pMultiSz->pwszValue != wszEmptyMultiSz)
   {
      RadiusFree(pMultiSz->pwszValue);
   }

   memset(pMultiSz, 0, sizeof(MULTI_SZ));
}

LONG
WINAPI
MultiSzAppend(
    PMULTI_SZ pMultiSz,
    LPCWSTR pwszString
    )
{
   DWORD nChar;
   MULTI_SZ result;
   PWSTR pwszPos;

   /* Calculate the number of characters being added. */
   nChar = lstrlenW(pwszString) + 1;

   /* Allocate a new MULTI_SZ with enough room for the additional string. */
   result.nChar = pMultiSz->nChar + nChar;
   result.pwszValue = RadiusAlloc(result.nChar * sizeof(WCHAR));
   if (result.pwszValue == NULL)
   {
      return ERROR_NOT_ENOUGH_MEMORY;
   }

   /* Copy in the current MULTI_SZ. */
   memcpy(
      result.pwszValue,
      pMultiSz->pwszValue,
      (pMultiSz->nChar * sizeof(WCHAR))
      );

   /* Compute the position to add the new string. */
   pwszPos = result.pwszValue + pMultiSz->nChar - 1;

   /* Append the string and the extra null-terminator. */
   memcpy(
      pwszPos,
      pwszString,
      (nChar * sizeof(WCHAR))
      );
   pwszPos[nChar] = L'\0';

   /* Clean-up the old MULTI_SZ and swap in the new one. */
   MultiSzFree(pMultiSz);
   *pMultiSz = result;

   return NO_ERROR;
}

VOID
WINAPI
MultiSzErase(
    PMULTI_SZ pMultiSz,
    LPWSTR pwszString
    )
{
   DWORD nChar;
   PWSTR pwszNext, pwszFirst, pwszLast;

   /* Compute the number of characters being erased. */
   nChar = lstrlenW(pwszString) + 1;

   /* Slide everything left to erase the string. */
   pwszFirst = pwszString + nChar;
   pwszLast = pMultiSz->pwszValue + pMultiSz->nChar;
   for (pwszNext = pwszFirst; pwszNext < pwszLast; ++pwszNext)
   {
      *pwszString++ = *pwszNext;
   }

   /* Update the length of the MULTI_SZ. */
   pMultiSz->nChar -= nChar;
}

LPWSTR
WINAPI
MultiSzFind(
    PMULTI_SZ pMultiSz,
    LPCWSTR pwszString
    )
{
   LPWSTR pwszNext;

   for (pwszNext = pMultiSz->pwszValue;
        *pwszNext != L'\0';
        pwszNext += (lstrlenW(pwszNext) + 1))
   {
      if (lstrcmpiW(pwszNext, pwszString) == 0)
      {
         return pwszNext;
      }
   }

   return NULL;
}

LONG
WINAPI
MultiSzQuery(
    PMULTI_SZ pMultiSz,
    HKEY hKey,
    LPCWSTR pwszValueName
    )
{
   LONG lResult;
   DWORD dwType, cbData;
   LPBYTE lpData;

   /* Initialize the MULTI_SZ. */
   MultiSzInit(pMultiSz);

   /* Compute the size of the value. */
   cbData = 0;
   lResult = RegQueryValueExW(
                hKey,
                pwszValueName,
                NULL,
                &dwType,
                NULL,
                &cbData
                );
   if (lResult != NO_ERROR)
   {
      return lResult;
   }
   if (dwType != REG_MULTI_SZ)
   {
      return ERROR_INVALID_DATA;
   }

   /* Allocate a buffer to hold the MULTI_SZ. */
   lpData = RadiusAlloc(cbData);

   /* Get the value. */
   lResult = RegQueryValueExW(
                hKey,
                pwszValueName,
                NULL,
                &dwType,
                lpData,
                &cbData
                );
   if (lResult != NO_ERROR)
   {
      RadiusFree(lpData);
      return lResult;
   }
   if (dwType != REG_MULTI_SZ)
   {
      RadiusFree(lpData);
      return ERROR_INVALID_DATA;
   }

   /* Store the result. */
   pMultiSz->pwszValue = (LPWSTR)lpData;
   pMultiSz->nChar = cbData / sizeof(WCHAR);

   return NO_ERROR;
}

/* Write a MULTI_SZ to the registry. */
LONG
WINAPI
MultiSzSet(
    PMULTI_SZ pMultiSz,
    HKEY hKey,
    LPCWSTR pwszValueName
    )
{
   return RegSetValueExW(
             hKey,
             pwszValueName,
             0,
             REG_MULTI_SZ,
             (const BYTE *)pMultiSz->pwszValue,
             pMultiSz->nChar * sizeof(WCHAR)
             );
}
