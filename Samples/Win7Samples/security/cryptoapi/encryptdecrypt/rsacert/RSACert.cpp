/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1996 - 2002.  Microsoft Corporation.  All rights reserved.
*/
/*
#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0400
*/
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h> // for proper buffer handling

const int IN_BUFFER_SIZE    = 64 * 1024;
// OUT_BUFFER_SIZE is 8 bytes larger than IN_BUFFER_SIZE
// When CALG_RC2 algorithm is used, encrypted data 
// will be 8 bytes larger than IN_BUFFER_SIZE
const int OUT_BUFFER_SIZE   = IN_BUFFER_SIZE + 8; // extra padding

#define MY_ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

void PrintUsage();
PCCERT_CONTEXT GetCertificateContextFromName(
                        LPTSTR lpszCertificateName,
                        LPTSTR lpszCertificateStoreName,
                        DWORD  dwCertStoreOpenFlags);

BOOL AcquireAndGetRSAKey(PCCERT_CONTEXT pCertContext,
                         DWORD dwCertStoreOpenFlags,
                         HCRYPTPROV *phProv,
                         HCRYPTKEY *phRSAKey,
                         BOOL fEncrypt,
                         BOOL *fCallerFreeProv);
BOOL EncryptDecryptFile(LPTSTR lpszCertificateName,
                        LPTSTR lpszCertificateStoreName,
                        DWORD  dwCertStoreOpenFlags,
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
   MyPrintf(_T("RSACert [</e>|</d>] <CertName> <StoreName> [</u>|</m>] <InputFile> <OutputFile>\n"));
   MyPrintf(_T("/e for Encryption\n"));
   MyPrintf(_T("/d for Decryption\n"));
   MyPrintf(_T("/u for current user certificate store\n"));
   MyPrintf(_T("/m for local machine certificate store\n"));
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
   LPTSTR lpszCertificateName = NULL;
   LPTSTR lpszCertificateStoreName = NULL;
   LPTSTR lpszInputFileName = NULL;
   LPTSTR lpszOutputFileName = NULL;
   DWORD dwCertStoreOpenFlags = CERT_SYSTEM_STORE_CURRENT_USER;

   if (argc != 7)
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

   lpszCertificateName = argv[2];
   lpszCertificateStoreName = argv[3];

   /* Check whether the certificate store to be opened is user or machine */
   if (_tcsicmp(argv[4], _T("/u")) == 0)
   {
      dwCertStoreOpenFlags = CERT_SYSTEM_STORE_CURRENT_USER;
   }
   else if (_tcsicmp(argv[4], _T("/m")) == 0)
   {
      dwCertStoreOpenFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
   }
   else
   {
      PrintUsage();
      return;
   }
   
   lpszInputFileName = argv[5];
   lpszOutputFileName = argv[6];

   EncryptDecryptFile(lpszCertificateName,
                        lpszCertificateStoreName,
                        dwCertStoreOpenFlags,
                        lpszInputFileName,
                        lpszOutputFileName,
                        fEncrypt);
}

PCCERT_CONTEXT GetCertificateContextFromName(
                        LPTSTR lpszCertificateName,
                        LPTSTR lpszCertificateStoreName,
                        DWORD  dwCertStoreOpenFlags)
{
   PCCERT_CONTEXT pCertContext = NULL;
   HCERTSTORE hCertStore = NULL;
   LPSTR szStoreProvider;
   DWORD dwFindType; 

#ifdef UNICODE
   szStoreProvider = (LPSTR)CERT_STORE_PROV_SYSTEM_W;
#else
   szStoreProvider = (LPSTR)CERT_STORE_PROV_SYSTEM_A;
#endif

   // Open the specified certificate store
   hCertStore = CertOpenStore(szStoreProvider,
                     0,
                     NULL,
                     CERT_STORE_READONLY_FLAG|
                     dwCertStoreOpenFlags,
                     lpszCertificateStoreName);
   if (hCertStore == NULL)
   {
      MyPrintf(_T("CertOpenStore failed with %X\n"), GetLastError());
      return pCertContext;
   }

#ifdef UNICODE
   dwFindType = CERT_FIND_SUBJECT_STR_W;
#else
   dwFindType = CERT_FIND_SUBJECT_STR_A;
#endif

   // Find the certificate by CN.
   pCertContext = CertFindCertificateInStore(
                    hCertStore,
                    MY_ENCODING,
                    0,
                    dwFindType,
                    lpszCertificateName,
                    NULL);
   if (pCertContext == NULL)
   {
      MyPrintf(_T("CertFindCertificateInStore failed with %X\n"), GetLastError());
   }

   CertCloseStore(hCertStore, 0);

   return pCertContext;
}

BOOL AcquireAndGetRSAKey(PCCERT_CONTEXT pCertContext,
                         HCRYPTPROV *phProv,
                         HCRYPTKEY *phRSAKey,
                         BOOL fEncrypt,
                         BOOL *pfCallerFreeProv)
{
   BOOL fSuccess = FALSE;
   DWORD dwKeySpec = 0;

   __try
   {
      *phProv = NULL;
      *phRSAKey = NULL;
      *pfCallerFreeProv = FALSE;

      if (fEncrypt)
      {
         // Acquire context for RSA key
         fSuccess = CryptAcquireContext(phProv,
                        NULL,
                        MS_DEF_PROV,
                        PROV_RSA_FULL,
                        CRYPT_VERIFYCONTEXT);
         if (!fSuccess)
         {
            MyPrintf(_T("CryptAcquireContext failed with %X\n"), GetLastError());
            __leave;
         }

         // Import the RSA public key from the certificate context
         fSuccess = CryptImportPublicKeyInfo(*phProv,
                        MY_ENCODING,
                        &(pCertContext->pCertInfo->SubjectPublicKeyInfo),
                        phRSAKey);
         if (!fSuccess)
         {
            MyPrintf(_T("CryptImportPublicKeyInfo failed with %X\n"), GetLastError());
            __leave;
         }

         *pfCallerFreeProv = TRUE;
      }
      else
      {
         // Acquire the RSA private key
         fSuccess = CryptAcquireCertificatePrivateKey(pCertContext,
               CRYPT_ACQUIRE_USE_PROV_INFO_FLAG|CRYPT_ACQUIRE_COMPARE_KEY_FLAG, 
               NULL, phProv, &dwKeySpec, pfCallerFreeProv);
         if (!fSuccess)
         {
            MyPrintf(_T("CryptAcquireCertificatePrivateKey failed with %X\n"), GetLastError());
            __leave;
         }

         // Get the RSA key handle
         fSuccess = CryptGetUserKey(*phProv, dwKeySpec, phRSAKey);
         if (!fSuccess)
         {
            MyPrintf(_T("CryptGetUserKey failed with %X\n"), GetLastError());
            __leave;
         }
      }
   }
   __finally
   {
      if (fSuccess == FALSE)
      {
         if (phProv && *phProv != NULL)
         {
            CryptReleaseContext(*phProv, 0);
            *phProv = NULL;
         }
         if (phRSAKey && *phRSAKey != NULL)
         {
            CryptDestroyKey(*phRSAKey);
            *phRSAKey = NULL;
         }
      }
   }

   return fSuccess;
}

BOOL EncryptDecryptFile(LPTSTR lpszCertificateName,
                        LPTSTR lpszCertificateStoreName,
                        DWORD  dwCertStoreOpenFlags,
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
   BOOL fCallerFreeProv = FALSE;
   PCCERT_CONTEXT pCertContext = NULL;
   BOOL fSuccess = FALSE;

   __try
   {
      pCertContext = GetCertificateContextFromName(lpszCertificateName,
                        lpszCertificateStoreName,
                        dwCertStoreOpenFlags);
      if (pCertContext == NULL)
      {
         __leave;
      }

      fResult = AcquireAndGetRSAKey(pCertContext,
                        &hProv,
                        &hRSAKey,
                        fEncrypt,
                        &fCallerFreeProv);
      if (fResult == FALSE)
      {
         __leave;
      }

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
      if (pCertContext != NULL) CertFreeCertificateContext(pCertContext);
      if (hRSAKey != NULL) CryptDestroyKey(hRSAKey);
      if (hSessionKey != NULL) CryptDestroyKey(hSessionKey);
      if (fCallerFreeProv && hProv != NULL) CryptReleaseContext(hProv, 0);
      if (hInFile != INVALID_HANDLE_VALUE) CloseHandle(hInFile);
      if (hOutFile != INVALID_HANDLE_VALUE) CloseHandle(hOutFile);
   }

   return fSuccess;
}
