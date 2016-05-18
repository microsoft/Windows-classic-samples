/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1996 - 2002.  Microsoft Corporation.  All rights reserved.
*/

/*   
    Module: MYTOKEN.CPP

    When you log on to a Microsoft Windows NT based platform, NT generates
    an Access Token that describes who you are, what groups you belong to,
    and what privileges you have on that computer.

    The following sample code demonstrates how to extract this interesting
    information from the current thread/process access token. When I look
    at this information I tend to separate it into three categories

        1. User identification and miscellaneous info
        2. Group information
        3. Privileges

    Notes about the group information:

        1) You will see a group sid with the form NONE_MAPPED. This is 
        the login SID generated for this particular logon session. It is 
        unique until the server is rebooted.

        2) Many of the group SIDS are well-known SIDs and RIDs. Consult the 
        documentation for information about these well-known Identifiers.

    Notes about the privileges information:

        The attributes number is simply a bit flag. 1 indicates that the 
        privilege is enabled and 2 indicates that the privilege is enabled
        by default. 3, of course, indicates that it is enabled by default
        and currently enabled.
  

    This code sample requires the following import libraries:       
  
        advapi32.lib
        user32.lib
*/

#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <strsafe.h> // for proper buffer handling

LPVOID RetrieveTokenInformationClass(
            HANDLE hToken,
            TOKEN_INFORMATION_CLASS InfoClass,
            LPDWORD dwSize);
LPWSTR ConvertBinarySidToName(PSID pSid);
BOOL DisplayUserInfo(HANDLE hToken);
BOOL DisplayOwnerInfo(HANDLE hToken);
BOOL DisplayPrimaryGroupInfo(HANDLE hToken);
BOOL DisplayStatistics(HANDLE hToken);
BOOL DisplaySource(HANDLE hToken);
BOOL DisplayGroupsInfo(HANDLE hToken);
BOOL DisplayPrivileges(HANDLE hToken);
BOOL DisplayTokenInformation(HANDLE hToken);
BOOL DisplayCallerAccessTokenInformation();
void __cdecl MyPrintf(LPCTSTR lpszFormat, ...);

#define CheckAndLocalFree(ptr) \
            if (ptr != NULL) \
            { \
               LocalFree(ptr); \
               ptr = NULL; \
            }

void __cdecl MyPrintf(LPCWSTR lpszFormat, ...)
{
   WCHAR szOutput[2048];
   va_list v1 = NULL;
   HRESULT hr = S_OK;

   va_start(v1, lpszFormat);
   hr = StringCbVPrintf(szOutput, sizeof(szOutput), lpszFormat, v1);
   if (SUCCEEDED(hr))
   {
      OutputDebugString(szOutput);
      wprintf(szOutput);
   }
   else
   {
      wprintf(L"StringCbVPrintf failed with %X\n", hr);
   }
}

LPVOID RetrieveTokenInformationClass(
      HANDLE hToken,
      TOKEN_INFORMATION_CLASS InfoClass,
      LPDWORD lpdwSize)
{
   LPVOID pInfo = NULL;
   BOOL fSuccess = FALSE;

   __try
   {
      *lpdwSize = 0;

      //
      // Determine size of buffer needed
      //

      GetTokenInformation(
            hToken,
            InfoClass,
            NULL,
            *lpdwSize, lpdwSize);
      if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
      {
         MyPrintf(L"GetTokenInformation failed with %d\n", GetLastError());
         __leave;
      }

      //
      // Allocate a buffer for getting token information
      //
      pInfo = LocalAlloc(LPTR, *lpdwSize);
      if (pInfo == NULL)
      {
         MyPrintf(L"LocalAlloc failed with %d\n", GetLastError());
         __leave;
      }

      if (!GetTokenInformation(
            hToken,
            InfoClass,
            pInfo,
            *lpdwSize, lpdwSize))
      {
         MyPrintf(L"GetTokenInformation failed with %d\n", GetLastError());
         __leave;
      }

      fSuccess = TRUE;
   }
   __finally
   {
      // Free pDomainAndUserName only if failed.
      // Otherwise, the caller has to free after use
      if (fSuccess == FALSE)
      {
         CheckAndLocalFree(pInfo);
      }
   }

   return pInfo;
}

LPWSTR ConvertBinarySidToName(PSID pSid)
{
   LPWSTR pAccountName = NULL;
   LPWSTR pDomainName = NULL;
   LPWSTR pDomainAndUserName = NULL;
   DWORD dwAccountNameSize = 0;
   DWORD dwDomainNameSize = 0;
   DWORD dwSize = 0;
   SID_NAME_USE sidType;
   BOOL fSuccess = FALSE;
   HRESULT hr = S_OK;

   __try
   {
      LookupAccountSid(
            NULL,                      // lookup on local system
            pSid,
            pAccountName,              // buffer to recieve name
            &dwAccountNameSize,
            pDomainName,
            &dwDomainNameSize,
            &sidType);
      // If the SID cannot be resolved, LookupAccountSid will fail with
      // ERROR_NONE_MAPPED
      if (GetLastError() == ERROR_NONE_MAPPED)
      {
         LPWSTR pNoneMappedString = L"NONE_MAPPED";

         dwAccountNameSize = sizeof(L"NONE_MAPPED")/sizeof(WCHAR);
         pAccountName = (LPWSTR)LocalAlloc(LPTR, dwAccountNameSize * sizeof(WCHAR));
         if (pAccountName == NULL)
         {
            MyPrintf(L"LocalAlloc failed with %d\n", GetLastError());
            __leave;
         }
         hr = StringCchCopy(pAccountName, dwAccountNameSize, pNoneMappedString);
         if (FAILED(hr))
         {
            wprintf(L"StringCchCopy failed with %X\n", hr);
            __leave;
         }

         dwDomainNameSize = 0;
         pDomainName = NULL;
      }
      else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
      {
         pAccountName = (LPWSTR)LocalAlloc(LPTR, dwAccountNameSize * sizeof(WCHAR));
         if (pAccountName == NULL)
         {
            MyPrintf(L"LocalAlloc failed with %d\n", GetLastError());
            __leave;
         }

         pDomainName = (LPWSTR)LocalAlloc(LPTR, dwDomainNameSize * sizeof(WCHAR));
         if (pDomainName == NULL)
         {
            MyPrintf(L"LocalAlloc failed with %d\n", GetLastError());
            __leave;
         }

         if (!LookupAccountSid(
               NULL,                      // lookup on local system
               pSid,
               pAccountName,              // buffer to recieve name
               &dwAccountNameSize,
               pDomainName,
               &dwDomainNameSize,
               &sidType))
         {
            MyPrintf(L"LookupAccountSid failed with %d\n", GetLastError());
            __leave;
         }
      }
      // Any other error code
      else
      {
         MyPrintf(L"LookupAccountSid failed with %d\n", GetLastError());
         __leave;
      }

      dwSize = dwDomainNameSize +
                  1 /* For \\ character */ +
                  dwAccountNameSize +
                  1 /* For NULL character */;
   
      pDomainAndUserName = (LPWSTR)LocalAlloc(LPTR, dwSize * sizeof(WCHAR));
      if (pDomainAndUserName == NULL)
      {
         MyPrintf(L"LocalAlloc failed with %d\n", GetLastError());
         __leave;
      }

      pDomainAndUserName[0] = 0;
      if (dwDomainNameSize != 0)
      {
         hr = StringCchCopy(pDomainAndUserName, dwSize, pDomainName);
         if (FAILED(hr))
         {
            wprintf(L"StringCchCopy failed with %X\n", hr);
            __leave;
         }
         hr = StringCchCat(pDomainAndUserName, dwSize, L"\\");
         if (FAILED(hr))
         {
            wprintf(L"StringCchCat failed with %X\n", hr);
            __leave;
         }
      }
      if (dwAccountNameSize != 0)
      {
         hr = StringCchCat(pDomainAndUserName, dwSize, pAccountName);
         if (FAILED(hr))
         {
            wprintf(L"StringCchCat failed with %X\n", hr);
            __leave;
         }
      }

      fSuccess = TRUE;
   }
   __finally
   {
      CheckAndLocalFree(pAccountName);
      CheckAndLocalFree(pDomainName);
      // Free pDomainAndUserName only if failed.
      // Otherwise, the caller has to free after use
      if (fSuccess == FALSE)
      {
         CheckAndLocalFree(pDomainAndUserName);
      }
   }

   return pDomainAndUserName;
}

LPWSTR ConvertLUIDToName(PLUID pLuid)
{
   LPWSTR pPrivilegeName = NULL;
   DWORD dwSize = 0;
   BOOL fSuccess = FALSE;

   __try
   {
      LookupPrivilegeName(
            NULL,                      // lookup on local system
            pLuid,
            pPrivilegeName,            // buffer to recieve name
            &dwSize);
      // Check for ERROR_INSUFFICIENT_BUFFER error code 
      if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
      {
         MyPrintf(L"LookupPrivilegeName failed with %d\n", GetLastError());
         __leave;
      }

      pPrivilegeName = (LPWSTR)LocalAlloc(LPTR, dwSize * sizeof(WCHAR));
      if (pPrivilegeName == NULL)
      {
         MyPrintf(L"LocalAlloc failed with %d\n", GetLastError());
         __leave;
      }

      if (!LookupPrivilegeName(
            NULL,                      // lookup on local system
            pLuid,
            pPrivilegeName,            // buffer to recieve name
            &dwSize))
      {
         MyPrintf(L"LookupPrivilegeName failed with %d\n", GetLastError());
         __leave;
      }

      fSuccess = TRUE;
   }
   __finally
   {
      // Free pPrivilegeName only if failed.
      // Otherwise, the caller has to free after use
      if (fSuccess == FALSE)
      {
         CheckAndLocalFree(pPrivilegeName);
      }
   }

   return pPrivilegeName;
}

BOOL DisplayUserInfo(HANDLE hToken)
{
   TOKEN_USER *pUserInfo = NULL;
   DWORD dwSize = 0;
   LPWSTR pName = NULL;

   //
   // Get User Information
   //

   pUserInfo = (TOKEN_USER *)RetrieveTokenInformationClass(hToken, TokenUser, &dwSize);
   if (pUserInfo == NULL)
   {
      return FALSE;
   }

   pName = ConvertBinarySidToName(pUserInfo->User.Sid);
   if (pName == NULL)
   {
      return FALSE;
   }

   MyPrintf(L"User : %s\n", pName);

   CheckAndLocalFree(pUserInfo);
   CheckAndLocalFree(pName);
   return TRUE;
}

BOOL DisplayOwnerInfo(HANDLE hToken)
{
   TOKEN_OWNER *pOwnerInfo = NULL;
   DWORD dwSize = 0;
   LPWSTR pName = NULL;

   //
   // Get Owner Information
   //

   pOwnerInfo = (TOKEN_OWNER *)RetrieveTokenInformationClass(hToken, TokenOwner, &dwSize);
   if (pOwnerInfo == NULL)
   {
      return FALSE;
   }

   pName = ConvertBinarySidToName(pOwnerInfo->Owner);
   if (pName == NULL)
   {
      return FALSE;
   }

   MyPrintf(L"Owner : %s\n", pName);

   CheckAndLocalFree(pOwnerInfo);
   CheckAndLocalFree(pName);
   return TRUE;
}

BOOL DisplayPrimaryGroupInfo(HANDLE hToken)
{
   TOKEN_PRIMARY_GROUP *pPrimaryGroupInfo = NULL;
   DWORD dwSize = 0;
   LPWSTR pName = NULL;

   //
   // Get Primary Group Information
   //

   pPrimaryGroupInfo = (TOKEN_PRIMARY_GROUP *)RetrieveTokenInformationClass(hToken, TokenPrimaryGroup, &dwSize);
   if (pPrimaryGroupInfo == NULL)
   {
      return FALSE;
   }

   pName = ConvertBinarySidToName(pPrimaryGroupInfo->PrimaryGroup);
   if (pName == NULL)
   {
      return FALSE;
   }

   MyPrintf(L"Primary Group : %s\n", pName);

   CheckAndLocalFree(pPrimaryGroupInfo);
   CheckAndLocalFree(pName);
   return TRUE;
}

BOOL DisplayStatistics(HANDLE hToken)
{
   TOKEN_STATISTICS *pStatistics = NULL;
   DWORD dwSize = 0;

   //
   // Get Token Statistics Information
   //

   pStatistics = (TOKEN_STATISTICS *)RetrieveTokenInformationClass(hToken, TokenStatistics, &dwSize);
   if (pStatistics == NULL)
   {
      return FALSE;
   }

   //
   // Display some of the token statistics information
   //

   MyPrintf(L"LUID for this instance of token %i64\n", pStatistics->TokenId);
   MyPrintf(L"LUID for this logon session     %i64\n", pStatistics->AuthenticationId);

   if (pStatistics->TokenType == TokenPrimary)
      MyPrintf(L"Token type is PRIMARY\n");
   else
      MyPrintf(L"Token type is IMPERSONATION\n");

   CheckAndLocalFree(pStatistics);
   return TRUE;
}

BOOL DisplaySource(HANDLE hToken)
{
   TOKEN_SOURCE *pSource = NULL;
   DWORD dwSize = 0;
   DWORD i = 0;

   //
   // Display source of access token
   //

   pSource = (TOKEN_SOURCE *)RetrieveTokenInformationClass(hToken, TokenSource, &dwSize);
   if (pSource == NULL)
   {
      return FALSE;
   }

   MyPrintf(L"Token source is <");
   for (i = 0; i < TOKEN_SOURCE_LENGTH; i++)
   {
      MyPrintf(L"%c", pSource->SourceName[i]);
   }
   MyPrintf(L">\n");

   CheckAndLocalFree(pSource);
   return TRUE;
}

BOOL DisplayGroupsInfo(HANDLE hToken)
{
   TOKEN_GROUPS *pGroupInfo = NULL;
   DWORD dwSize = 0;
   DWORD i;

   //
   //  List all groups in the access token
   //

   pGroupInfo = (PTOKEN_GROUPS)RetrieveTokenInformationClass(hToken, TokenGroups, &dwSize);
   if (pGroupInfo == NULL)
   {
      return FALSE;
   }

   MyPrintf(L"\nRetrieving Group information from the access token\n");
   for (i = 0; i < pGroupInfo->GroupCount; i++)
   {
      LPWSTR pName = NULL;

      pName = ConvertBinarySidToName(pGroupInfo->Groups[i].Sid);
      if (pName != NULL)
      {
         MyPrintf(L"SID %d Group: %s\n", i, pName);
         CheckAndLocalFree(pName);
      }
   }

   CheckAndLocalFree(pGroupInfo);
   return TRUE;
}

BOOL DisplayPrivileges(HANDLE hToken)
{
   TOKEN_PRIVILEGES *pPrivileges = NULL;
   DWORD dwSize = 0;
   DWORD i;

   //
   // Display privileges associated with this access token
   //

   pPrivileges = (TOKEN_PRIVILEGES *)RetrieveTokenInformationClass(hToken, TokenPrivileges, &dwSize);
   if (pPrivileges == NULL)
   {
      return FALSE;
   }

   MyPrintf(L"\nPrivileges associated with this token (%lu)\n", pPrivileges->PrivilegeCount);
   for (i = 0; i < pPrivileges->PrivilegeCount; i++)
   {
      LPWSTR pPrivilegeName = NULL;

      pPrivilegeName = ConvertLUIDToName(&(pPrivileges->Privileges[i].Luid));
      if (pPrivilegeName != NULL)
      {
         MyPrintf(L"%s - (attributes) %lu\n", pPrivilegeName, pPrivileges->Privileges[i].Attributes);
         CheckAndLocalFree(pPrivilegeName);
      }
   }

   CheckAndLocalFree(pPrivileges);
   return TRUE;
}

BOOL DisplayTokenInformation(HANDLE hToken)
{
   //
   // Display access token information
   //

   if (!DisplayUserInfo(hToken))
   {
      return FALSE;
   }
   if (!DisplayOwnerInfo(hToken))
   {
      return FALSE;
   }
   if (!DisplayPrimaryGroupInfo(hToken))
   {
      return FALSE;
   }
   if (!DisplayStatistics(hToken))
   {
      return FALSE;
   }
   if (!DisplaySource(hToken))
   {
      return FALSE;
   }
   if (!DisplayGroupsInfo(hToken))
   {
      return FALSE;
   }
   if (!DisplayPrivileges(hToken))
   {
      return FALSE;
   }

   return TRUE;
}

BOOL DisplayCallerAccessTokenInformation()
{
   HANDLE hToken = NULL;
   BOOL bResult = FALSE;

   // Use OpenThreadToken() API first, to determine
   // if the calling thread is running under impersonation
   bResult = OpenThreadToken(GetCurrentThread(),
      TOKEN_QUERY | TOKEN_QUERY_SOURCE, TRUE, &hToken);
   if (bResult == FALSE && GetLastError() == ERROR_NO_TOKEN)
   {
      // Otherwise, use process access token
      bResult = OpenProcessToken(GetCurrentProcess(),
         TOKEN_QUERY | TOKEN_QUERY_SOURCE, &hToken);
   }
   if (bResult)
   {
      bResult = DisplayTokenInformation(hToken);
      CloseHandle(hToken);
   }
   else
      MyPrintf(L"OpenThread/ProcessToken failed with %d\n", GetLastError());

   return bResult;
}

void wmain()
{
   DisplayCallerAccessTokenInformation();
}
