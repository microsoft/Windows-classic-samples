/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.
*/

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h> // for proper buffer handling

const int IN_BUFFER_SIZE    = 64 * 1024;
// OUT_BUFFER_SIZE is 8 bytes larger than IN_BUFFER_SIZE
// When CALG_RC2 algorithm is used, encrypted data 
// will be 8 bytes larger than IN_BUFFER_SIZE
const int OUT_BUFFER_SIZE   = IN_BUFFER_SIZE + 8; // extra padding

void PrintUsage();
BOOL EncryptDecryptFile(LPTSTR lpszContainerName,
                        DWORD dwFlags,
                        LPTSTR lpszInputFileName,
                        LPTSTR lpszOutputFileName,
                        BOOL fEncrypt);
void __cdecl MyPrintf(LPCTSTR lpszFormat, ...);

#define CheckAndLocalFree(ptr) \
            if (ptr != NULL) \
            { \
               LocalFree(ptr); \
               ptr = NULL; \
            }

void PrintUsage()
{
   MyPrintf(_T("RSAKey [</e>|</d>] <KeyContainerName> [</u>|</m>] <InputFile> <OutputFile>\n"));
   MyPrintf(_T("/e for Encryption\n"));
   MyPrintf(_T("/d for Decryption\n"));
   MyPrintf(_T("/u for current user key container\n"));
   MyPrintf(_T("/m for local machine key container\n"));
}

void __cdecl MyPrintf(LPCTSTR lpszFormat, ...)
{
   TCHAR szOutput[2048];
   va_list v1 = NULL;
   HRESULT hr = S_OK;

   va_start(v1, lpszFormat);
   hr = StringCbVPrintf(szOutput, sizeof(szOutput), lpszFormat, v1);
   if (SUCCEEDED(hr))
   {
      OutputDebugString(szOutput);
      _tprintf(szOutput);
   }
   else
   {
      _tprintf(_T("StringCbVPrintf failed with %X\n"), hr);
   }
}

void _tmain(int argc, TCHAR **argv)
{
   BOOL fEncrypt = FALSE;
   LPTSTR lpszContainerName = NULL;
   LPTSTR lpszInputFileName = NULL;
   LPTSTR lpszOutputFileName = NULL;
   DWORD dwFlags = 0;

   if (argc != 6)
   {
      PrintUsage();
      return;
   }

   /* Check whether the action to be performed is encrypt or decrypt */
   if (_tcsicmp(argv[1], _T("/e")) == 0)
   {
      fEncrypt = TRUE;
   }
   else if (_tcsicmp(argv[1], _T("/d")) == 0)
   {
      fEncrypt = FALSE;
   }
   else
   {
      PrintUsage();
      return;
   }

   lpszContainerName = argv[2];
   
   /* Check whether to use current user or local machine key container */
   if (_tcsicmp(argv[3], _T("/u")) == 0)
   {
      dwFlags = 0;
   }
   else if (_tcsicmp(argv[3], _T("/m")) == 0)
   {
      dwFlags = CRYPT_MACHINE_KEYSET;
   }
   else
   {
      PrintUsage();
      return;
   }

   lpszInputFileName = argv[4];
   lpszOutputFileName = argv[5];

   EncryptDecryptFile(lpszContainerName,
                        dwFlags,
                        lpszInputFileName,
                        lpszOutputFileName,
                        fEncrypt);
}

BOOL EncryptDecryptFile(LPTSTR lpszContainerName,
                        DWORD dwFlags,
                        LPTSTR lpszInputFileName,
                        LPTSTR lpszOutputFileName,
                        BOOL fEncrypt)
{
   BOOL fResult = FALSE;
   HCRYPTPROV hProv = NULL;
   HCRYPTKEY hRSAKey = NULL;
   HCRYPTKEY hSessionKey = NULL;
   HANDLE hInFile = INVALID_HANDLE_VALUE;
   HANDLE hOutFile = INVALID_HANDLE_VALUE;
   BOOL finished = FALSE;
   BYTE pbBuffer[OUT_BUFFER_SIZE];
   DWORD dwByteCount = 0;
   DWORD dwBytesRead = 0;
   DWORD dwBytesWritten = 0;
   LPBYTE pbSessionKeyBlob = NULL;
   DWORD dwSessionKeyBlob = 0;
   BOOL fSuccess = FALSE;

   __try
   {
      // Open the input file to be encrypted or decrypted
      hInFile = CreateFile(lpszInputFileName,
                  GENERIC_READ,
                  0,
                  NULL,
                  OPEN_EXISTING, 
                  FILE_ATTRIBUTE_NORMAL,
                  NULL);
      if (hInFile == INVALID_HANDLE_VALUE)
      {
         MyPrintf(_T("CreateFile failed with %d\n"), GetLastError());
         __leave;
      }

      // Open the output file to write the encrypted or decrypted data
      hOutFile = CreateFile(lpszOutputFileName,
                  GENERIC_WRITE,
                  0,
                  NULL,
                  CREATE_ALWAYS, 
                  FILE_ATTRIBUTE_NORMAL,
                  NULL);
      if (hOutFile == INVALID_HANDLE_VALUE)
      {
         MyPrintf(_T("CreateFile failed with %d\n"), GetLastError());
         __leave;
      } 
    
      // Acquire context for RSA key
      fResult = CryptAcquireContext(&hProv,
                     lpszContainerName,
                     MS_DEF_PROV,
                     PROV_RSA_FULL,
                     dwFlags);
      if (!fResult)
      {
         if (GetLastError() == NTE_BAD_KEYSET)
         {
            // Create a key container if one does not exist.
            fResult = CryptAcquireContext(&hProv,
                        lpszContainerName,
                        MS_DEF_PROV,
                        PROV_RSA_FULL,
                        CRYPT_NEWKEYSET | dwFlags);
            if (!fResult)
            {
               MyPrintf(_T("CryptAcquireContext (2) failed with %X\n"), GetLastError());
               __leave;
            }
        }
        else
        {
            MyPrintf(_T("CryptAcquireContext (1) failed with %X\n"), GetLastError());
            __leave;
        }
      }

      // Get the RSA key handle
      fResult = CryptGetUserKey(hProv, AT_KEYEXCHANGE, &hRSAKey);
      if (!fResult)
      {
         if (GetLastError() == NTE_NO_KEY)
         {
            // Create a key if one does not exist.
            fResult = CryptGenKey(hProv,
                        AT_KEYEXCHANGE,
                        CRYPT_EXPORTABLE,
                        &hRSAKey);
            if (!fResult)
            {
               MyPrintf(_T("CryptGenKey failed with %X\n"), GetLastError());
               __leave;
            }
         }
         else
         {
            MyPrintf(_T("CryptGetUserKey failed with %X\n"), GetLastError());
            __leave;
         }
      }

      if (fEncrypt)
      {
         fResult = CryptGenKey(hProv, CALG_RC4, CRYPT_EXPORTABLE, &hSessionKey);
         if (!fResult)
         {
            MyPrintf(_T("CryptGenKey failed with %X\n"), GetLastError());
            __leave;
         }

         // The first call to ExportKey with NULL gets the key size.
         dwSessionKeyBlob = 0;
         fResult = CryptExportKey(hSessionKey, hRSAKey, SIMPLEBLOB, 0,
                     NULL, &dwSessionKeyBlob);
         if (!fResult)
         {
            MyPrintf(_T("CryptExportKey failed with %X\n"), GetLastError());
            __leave;
         }
      
         // Allocate memory for Encrypted Session key blob
         pbSessionKeyBlob = (LPBYTE)LocalAlloc(LPTR, dwSessionKeyBlob);
         if (!pbSessionKeyBlob)
         {
            MyPrintf(_T("LocalAlloc failed with %d\n"), GetLastError());
            __leave;
         }

         fResult = CryptExportKey(hSessionKey, hRSAKey, SIMPLEBLOB, 0,
                     pbSessionKeyBlob, &dwSessionKeyBlob);
         if (!fResult)
         {
            MyPrintf(_T("CryptExportKey failed with %X\n"), GetLastError());
            __leave;
         }

         // Write the size of key blob, then the key blob itself, to output file.
         fResult = WriteFile(hOutFile, &dwSessionKeyBlob,
                     sizeof(dwSessionKeyBlob),
                     &dwBytesWritten, NULL);
         if (!fResult)
         {
            MyPrintf(_T("WriteFile failed with %d\n"), GetLastError());
            __leave;
         }
         fResult = WriteFile(hOutFile, pbSessionKeyBlob,
                     dwSessionKeyBlob,
                     &dwBytesWritten, NULL);
         if (!fResult)
         {
            MyPrintf(_T("WriteFile failed with %d\n"), GetLastError());
            __leave;
         }
      }
      else
      {
         // Read in key block size, then key blob itself from input file.
         fResult = ReadFile(hInFile, &dwByteCount, sizeof(dwByteCount), 
               &dwBytesRead, NULL);
         if (!fResult)
         {
            MyPrintf(_T("ReadFile failed with %d\n"), GetLastError());
            __leave;
         }

         fResult = ReadFile(hInFile, pbBuffer, dwByteCount, &dwBytesRead, NULL);
         if (!fResult)
         {
            MyPrintf(_T("ReadFile failed with %d\n"), GetLastError());
            __leave;
         }

         // import key blob into "CSP"
         fResult = CryptImportKey(hProv, pbBuffer, dwByteCount, hRSAKey, 0, &hSessionKey);
         if (!fResult)
         {
            MyPrintf(_T("CryptImportKey failed with %X\n"), GetLastError());
            __leave;
         }
      }

      do
      {
         dwByteCount = 0;

         // Now read data from the input file 64K bytes at a time.
         fResult = ReadFile(hInFile, pbBuffer, IN_BUFFER_SIZE, &dwByteCount, NULL);

         // If the file size is exact multiple of 64K, dwByteCount will be zero after
         // all the data has been read from the input file. In this case, simply break
         // from the while loop. The check to do this is below
         if (dwByteCount == 0)
            break;

         if (!fResult)
         {
            MyPrintf(_T("ReadFile failed with %d\n"), GetLastError());
            __leave;
         }

         finished = (dwByteCount < IN_BUFFER_SIZE);

         // Encrypt/Decrypt depending on the required action.
         if (fEncrypt)
         {
            fResult = CryptEncrypt(hSessionKey, 0, finished, 0, pbBuffer, &dwByteCount,
                           OUT_BUFFER_SIZE);
            if (!fResult)
            {
               MyPrintf(_T("CryptEncrypt failed with %X\n"), GetLastError());
               __leave;
            }
         }
         else
         {
            fResult = CryptDecrypt(hSessionKey, 0, finished, 0, pbBuffer, &dwByteCount);
            if (!fResult)
            {
               MyPrintf(_T("CryptDecrypt failed with %X\n"), GetLastError());
               __leave;
            }
         }

         // Write the encrypted/decrypted data to the output file.
         fResult = WriteFile(hOutFile, pbBuffer, dwByteCount,
            &dwBytesWritten, NULL);
         if (!fResult)
         {
            MyPrintf(_T("WriteFile failed with %d\n"), GetLastError());
            __leave;
         }

      } while (!finished);

      if (fEncrypt)
         MyPrintf(_T("File %s is encrypted successfully!\n"), lpszInputFileName);
      else
         MyPrintf(_T("File %s is decrypted successfully!\n"), lpszInputFileName);

      fSuccess = TRUE;
   }
   __finally
   {
      /* Cleanup */
      CheckAndLocalFree(pbSessionKeyBlob);
      if (hRSAKey != NULL) CryptDestroyKey(hRSAKey);
      if (hSessionKey != NULL) CryptDestroyKey(hSessionKey);
      if (hProv != NULL) CryptReleaseContext(hProv, 0);
      if (hInFile != INVALID_HANDLE_VALUE) CloseHandle(hInFile);
      if (hOutFile != INVALID_HANDLE_VALUE) CloseHandle(hOutFile);
   }

   return fSuccess;
}
