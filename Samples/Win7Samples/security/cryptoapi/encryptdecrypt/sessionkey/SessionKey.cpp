/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.
*/

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

const int IN_BUFFER_SIZE    = 64 * 1024;
// OUT_BUFFER_SIZE is 8 bytes larger than IN_BUFFER_SIZE
// When CALG_RC2 algorithm is used, encrypted data 
// will be 8 bytes larger than IN_BUFFER_SIZE
const int OUT_BUFFER_SIZE   = IN_BUFFER_SIZE + 8; // extra padding

void PrintUsage()
{
   _tprintf(_T("SessionKey <password> [</e>|</d>] <InputFile> <OutputFile>\n"));
   _tprintf(_T("/e for Encryption\n"));
   _tprintf(_T("/d for Decryption\n"));
}

void _tmain(int argc, TCHAR *argv[])
{
   BOOL fResult = FALSE;
   HCRYPTPROV hProv = NULL;
   HCRYPTHASH hHash = NULL;
   HCRYPTKEY hSessionKey = NULL;
   HANDLE hInFile = INVALID_HANDLE_VALUE;
   HANDLE hOutFile = INVALID_HANDLE_VALUE;
   BOOL fEncrypt = FALSE;
   BOOL finished = FALSE;
   BYTE pbBuffer[OUT_BUFFER_SIZE];
   DWORD dwByteCount = 0;
   DWORD dwBytesWritten = 0;

   if (argc != 5)
   {
      PrintUsage();
      return;
   }

   __try
   {
      /* Check whether the action to be performed is encrypt or decrypt */
      if (_tcsicmp(argv[2], _T("/e")) == 0)
      {
         fEncrypt = TRUE;
      }
      else if (_tcsicmp(argv[2], _T("/d")) == 0)
      {
         fEncrypt = FALSE;
      }
      else
      {
         PrintUsage();
         return;
      }

      // Open the input file to be encrypted or decrypted
      hInFile = CreateFile(argv[3],
                  GENERIC_READ,
                  0,
                  NULL,
                  OPEN_EXISTING, 
                  FILE_ATTRIBUTE_NORMAL,
                  NULL);
      if (hInFile == INVALID_HANDLE_VALUE)
      {
         _tprintf(_T("CreateFile failed with %d\n"), GetLastError());
         __leave;
      }

      // Open the output file to write the encrypted or decrypted data
      hOutFile = CreateFile(argv[4],
                  GENERIC_WRITE,
                  0,
                  NULL,
                  CREATE_ALWAYS, 
                  FILE_ATTRIBUTE_NORMAL,
                  NULL);
      if (hOutFile == INVALID_HANDLE_VALUE)
      {
         _tprintf(_T("CreateFile failed with %d\n"), GetLastError());
         __leave;
      } 
    
      // Acquire a handle to MS_DEF_PROV using CRYPT_VERIFYCONTEXT for dwFlags
      // parameter as we are going to do only session key encryption or decryption
      fResult = CryptAcquireContext(&hProv,
                                    NULL,
                                    MS_DEF_PROV, 
                                    PROV_RSA_FULL,
                                    CRYPT_VERIFYCONTEXT);
      if (!fResult)
      {
         _tprintf(_T("CryptAcquireContext failed with %X\n"), GetLastError());
         __leave;
      }

      fResult = CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash);
      if (!fResult)
      {
         _tprintf(_T("CryptCreateHash failed with %X\n"), GetLastError());
         __leave;
      }

      // Hash the supplied secret password
      fResult = CryptHashData(hHash, (LPBYTE)argv[1], (DWORD)_tcslen(argv[1]), 0);
      if (!fResult)
      {
         _tprintf(_T("CryptHashData failed with %X\n"), GetLastError());
         __leave;
      }

      // Derive a symmetric session key from password hash
      fResult = CryptDeriveKey(hProv, CALG_RC4, hHash, 0, &hSessionKey);
      if (!fResult)
      {
         _tprintf(_T("CryptDeriveKey failed with %X\n"), GetLastError());
         __leave;
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
            _tprintf(_T("ReadFile failed with %d\n"), GetLastError());
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
               _tprintf(_T("CryptEncrypt failed with %X\n"), GetLastError());
               __leave;
            }
         }
         else
         {
            fResult = CryptDecrypt(hSessionKey, 0, finished, 0, pbBuffer, &dwByteCount);
            if (!fResult)
            {
               _tprintf(_T("CryptDecrypt failed with %X\n"), GetLastError());
               __leave;
            }
         }

         // Write the encrypted/decrypted data to the output file.
         fResult = WriteFile(hOutFile, pbBuffer, dwByteCount,
            &dwBytesWritten, NULL);
         if (!fResult)
         {
            _tprintf(_T("WriteFile failed with %d\n"), GetLastError());
            __leave;
         }

      } while (!finished);

      if (fEncrypt)
         _tprintf(_T("File %s is encrypted successfully!\n"), argv[3]);
      else
         _tprintf(_T("File %s is decrypted successfully!\n"), argv[3]);
   }
   __finally
   {
      /* Cleanup */
      if (hInFile != INVALID_HANDLE_VALUE) CloseHandle(hInFile);
      if (hOutFile != INVALID_HANDLE_VALUE) CloseHandle(hOutFile);
      if (hSessionKey != NULL) CryptDestroyKey(hSessionKey);
      if (hHash != NULL) CryptDestroyHash(hHash);
      if (hProv != NULL) CryptReleaseContext(hProv, 0);
   }
}
