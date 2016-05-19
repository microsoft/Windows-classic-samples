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

#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

const int BUFFER_SIZE = 4096;

#define CheckAndLocalFree(ptr) \
            if (ptr != NULL) \
            { \
               LocalFree(ptr); \
               ptr = NULL; \
            }

void __cdecl MyPrintf(LPCTSTR lpszFormat, ...);
void PrintUsage();
BOOL SignVerifyFile(HCRYPTPROV hProv,
                    HCRYPTKEY hPubKey,
                    DWORD dwKeySpec,
                    ALG_ID HashAlgId,
                    LPTSTR szFileToSign,
                    LPTSTR szSigFile,
                    BOOL fSign);
BOOL GetRSAKeyFromCert(PCCERT_CONTEXT pCertContext,
                       BOOL fSign,
                       HCRYPTPROV *hProv,
                       HCRYPTKEY *hPubKey,
                       DWORD *dwKeySpec,
                       BOOL *fFreeProv);
BOOL GetRSAKeyFromContainer(LPTSTR szContainerName,
                            DWORD dwAcquireFlags,
                            DWORD dwKeySpec,
                            HCRYPTPROV *hProv,                            
                            HCRYPTKEY *hPubKey);
PCCERT_CONTEXT GetCertificateContextFromName(
                        LPTSTR lpszCertificateName,
                        LPTSTR lpszCertificateStoreName,
                        DWORD  dwCertStoreOpenFlags);

void _tmain(int argc, TCHAR *argv[])
{
   HCRYPTPROV hProv = NULL;
   HCRYPTKEY hPubKey = NULL;   
   PCCERT_CONTEXT pCertContext = NULL;
   LPTSTR szCertificateName = NULL;
   LPTSTR szStoreName = NULL;
   LPTSTR szContainerName = NULL;   
   LPTSTR szFileToSign = NULL;
   LPTSTR szSigFile = NULL;
   DWORD dwOpenFlags = CERT_SYSTEM_STORE_CURRENT_USER;
   DWORD dwAcquireFlags = 0;
   DWORD dwKeySpec = AT_SIGNATURE;
   ALG_ID AlgId;   
   BOOL fSign = FALSE;
   BOOL fResult = FALSE;
   BOOL fUseCert = FALSE;
   BOOL fFreeProv = FALSE;

   if (argc != 9)
   {
      PrintUsage();
      return;
   }

   __try
   {  
      // Determine hash algorithm
      if (lstrcmpi(argv[1], _T("sha1")) == 0)
      {
         AlgId = CALG_SHA1;
      }
      else if (lstrcmpi(argv[1], _T("md5")) == 0)
      {
         AlgId = CALG_MD5;
      }
      else
      {
         PrintUsage();
         return;
      }

      /* Check whether the action to be performed is to sign or verify */
      if (lstrcmpi(argv[2], _T("/s")) == 0)
      {
         fSign = TRUE;
      }
      else if (lstrcmpi(argv[2], _T("/v")) == 0)
      {
         fSign = FALSE;
      }
      else
      {
         PrintUsage();
         return;
      }

      szFileToSign = argv[3];
      szSigFile = argv[4];

      // check to see if user wants to use a certificate
      if (lstrcmpi(argv[5], _T("/cert")) == 0)
      {
         fUseCert = TRUE;
         
         szCertificateName = argv[6];
         szStoreName = argv[7];

         // Determine if we have to use user or machine store
         if (lstrcmpi(argv[8], _T("u")) == 0)
         {
            dwOpenFlags = CERT_SYSTEM_STORE_CURRENT_USER;
         }
         else if (lstrcmpi(argv[8], _T("m")) == 0)
         {
            dwOpenFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
         }
         else
         {
            PrintUsage();
            return;
         }
      }
      else if (lstrcmpi(argv[5], _T("/key")) == 0)
      {
         fUseCert = FALSE;

         szContainerName = argv[6];

         if (lstrcmpi(argv[7], _T("u")) == 0)
         {
            dwAcquireFlags = 0;
         }
         else if (lstrcmpi(argv[7], _T("m")) == 0)
         {
            dwAcquireFlags = CRYPT_MACHINE_KEYSET;
         }
         else
         {
            PrintUsage();
            return;
         }

         // Use exchange key or signature key
         if (lstrcmpi(argv[8], _T("x")) == 0)
         {
            dwKeySpec = AT_KEYEXCHANGE;
         }
         else if (lstrcmpi(argv[8], _T("s")) == 0)
         {
            dwKeySpec = AT_SIGNATURE;
         }
         else
         {
            PrintUsage();
            return;
         }
      }
      else
      {
         PrintUsage();
         return;
      }

      if (fUseCert)
      {
         pCertContext = GetCertificateContextFromName(szCertificateName,
                                                      szStoreName,
                                                      dwOpenFlags);
         if (!pCertContext) __leave;
         
         fResult = GetRSAKeyFromCert(pCertContext,
                                     fSign,
                                     &hProv,
                                     &hPubKey,
                                     &dwKeySpec,
                                     &fFreeProv);
         if (!fResult) __leave;
      }
      else
      {
         fResult = GetRSAKeyFromContainer(szContainerName,
                                          dwAcquireFlags,
                                          dwKeySpec,
                                          &hProv,                            
                                          &hPubKey);         
         if (!fResult) __leave;

         fFreeProv = TRUE;
      }
      
      fResult = SignVerifyFile(hProv,
                               hPubKey,
                               dwKeySpec,
                               AlgId,
                               szFileToSign,
                               szSigFile,
                               fSign);
      if (!fResult) __leave;
      
      if (fSign)
      {
         MyPrintf(_T("File %s hashed and signed successfully!\n"), szFileToSign);
      }
      else
      {
         MyPrintf(_T("File %s verified successfully!\n"), szSigFile);
      }
   }
   __finally
   {
      // Clean up      
      if (hPubKey != NULL) CryptDestroyKey(hPubKey);
      if (fFreeProv && (hProv != NULL)) CryptReleaseContext(hProv, 0);
      if (pCertContext != NULL) CertFreeCertificateContext(pCertContext);      
   }
}

BOOL SignVerifyFile(HCRYPTPROV hProv,
                    HCRYPTKEY hPubKey,
                    DWORD dwKeySpec,
                    ALG_ID HashAlgId,
                    LPTSTR szFileToSign,
                    LPTSTR szSigFile,
                    BOOL fSign)
{
   HCRYPTHASH hHash = NULL;
   HANDLE hDataFile = INVALID_HANDLE_VALUE;
   HANDLE hSigFile = INVALID_HANDLE_VALUE;   
   LPBYTE pbSignature = NULL;
   DWORD dwSignature;         
   BYTE pbBuffer[BUFFER_SIZE];
   DWORD dwBytesRead, dwBytesWritten;
   BOOL fResult;
   BOOL fFinished = FALSE; 
   BOOL fReturn = FALSE;

   __try
   {  
      // Open Data file
      hDataFile = CreateFile(szFileToSign, 
                             GENERIC_READ, 
                             0, 
                             NULL, 
                             OPEN_EXISTING, 
                             FILE_ATTRIBUTE_NORMAL, 
                             NULL);
      if (hDataFile == INVALID_HANDLE_VALUE)
      {
         MyPrintf(_T("CreateFile failed with %d\n"), GetLastError());
         __leave;
      }

      // Open/Create signature file
      hSigFile = CreateFile(szSigFile, 
                            GENERIC_READ|GENERIC_WRITE, 
                            0, 
                            NULL, 
                            OPEN_ALWAYS, 
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL);
      if (hSigFile == INVALID_HANDLE_VALUE)
      {
         MyPrintf(_T("CreateFile failed with %d\n"), GetLastError());
         __leave;
      }

      // Create Hash
      fResult = CryptCreateHash(hProv, HashAlgId, 0, 0, &hHash);
      if (!fResult)
      {
         MyPrintf(_T("CryptCreateHash failed with %x\n"), GetLastError());
         __leave;
      }

      // Loop through file and hash file contents
      do
      {
         dwBytesRead = 0;

         fResult = ReadFile(hDataFile, pbBuffer, BUFFER_SIZE, &dwBytesRead, NULL);

         if (dwBytesRead == 0) break;

         if (!fResult)
         {
            MyPrintf(_T("ReadFile failed with %d\n"), GetLastError());
            __leave;
         }

         fFinished = (dwBytesRead < BUFFER_SIZE);

         fResult = CryptHashData(hHash, pbBuffer, dwBytesRead, 0);
         if (!fResult)
         {
            MyPrintf(_T("CryptHashData failed with %x\n"), GetLastError());
            __leave;
         }

      } while (fFinished == FALSE);

      if (fSign)
      {
         // Get Signature size
         fResult = CryptSignHash(hHash, dwKeySpec, NULL, 0, NULL, &dwSignature);
         if (!fResult)
         {
            MyPrintf(_T("CryptSignHash failed with %x\n"), GetLastError());
            __leave;
         }

         // Allocate signature bytes
         pbSignature = (LPBYTE)LocalAlloc(LPTR, dwSignature);
         if (!pbSignature)
         {
            MyPrintf(_T("LocalAlloc failed with %d\n"), GetLastError());
            __leave;
         }

         // Sign and get back signature
         fResult = CryptSignHash(hHash, dwKeySpec, NULL, 0, pbSignature, &dwSignature);
         if (!fResult)
         {
            MyPrintf(_T("CryptSignHash failed with %x\n"), GetLastError());
            __leave;
         }

         // Write signature to file
         fResult = WriteFile(hSigFile, pbSignature, dwSignature, &dwBytesWritten, NULL);
         if (!fResult)
         {
            MyPrintf(_T("WriteFile failed with %d\n"), GetLastError());
            __leave;
         }
      }
      else
      {
         // Get size of signature file
         dwSignature = GetFileSize(hSigFile, NULL);
         if (dwSignature == INVALID_FILE_SIZE)
         {
            MyPrintf(_T("GetFileSize failed with %d\n"), GetLastError());
            __leave;
         }

         // Allocate signature bytes
         pbSignature = (LPBYTE)LocalAlloc(LPTR, dwSignature);
         if (!pbSignature)
         {
            MyPrintf(_T("LocalAlloc failed with %d\n"), GetLastError());
            __leave;
         }

         // Read Signature
         fResult = ReadFile(hSigFile, pbSignature, dwSignature, &dwBytesRead, NULL);
         if (!fResult)
         {
            MyPrintf(_T("ReadFile failed with %d\n"), GetLastError());
            __leave;
         }

         // Verify Signature
         fResult = CryptVerifySignature(hHash, pbSignature, dwSignature, hPubKey, NULL, 0);
         if (!fResult)
         {
            MyPrintf(_T("CryptVerifySignature failed with %x\n"), GetLastError());
            __leave;
         }
      }

      fReturn = TRUE;
   }
   __finally
   {
      // Clean up
      if (hHash != NULL) CryptDestroyHash(hHash);      
      if (hDataFile != INVALID_HANDLE_VALUE) CloseHandle(hDataFile);
      if (hSigFile != INVALID_HANDLE_VALUE) CloseHandle(hSigFile);
      CheckAndLocalFree(pbSignature);
   }

   return fReturn;
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
                              dwCertStoreOpenFlags|
                              CERT_STORE_READONLY_FLAG,
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
   pCertContext = CertFindCertificateInStore(hCertStore,
                                             ENCODING,
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

BOOL GetRSAKeyFromCert(PCCERT_CONTEXT pCertContext,
                       BOOL fSign,
                       HCRYPTPROV *hProv,
                       HCRYPTKEY *hPubKey,
                       DWORD *dwKeySpec,
                       BOOL *fFreeProv)
{   
   BOOL fResult;
   BOOL fReturn = FALSE;
   
   __try
   {  
      if (hProv == NULL || hPubKey == NULL || 
          dwKeySpec == NULL || fFreeProv == NULL)
      {
         __leave;
      }

      *hProv = 0;
      *hPubKey = 0;
      *fFreeProv = FALSE;
    
      if (fSign)
      {
         // Acquire the certificate's private key
         fResult = CryptAcquireCertificatePrivateKey(pCertContext,
                                                     CRYPT_ACQUIRE_USE_PROV_INFO_FLAG|
                                                     CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
                                                     NULL,
                                                     hProv,
                                                     dwKeySpec,
                                                     fFreeProv);
         if (!fResult)
         {
            MyPrintf(_T("CryptAcquireCertificatePrivateKey failed with %x\n"), GetLastError());
            __leave;
         }
      }
      else
      {
         fResult = CryptAcquireContext(hProv, 
                                       NULL, 
                                       MS_DEF_PROV,
                                       PROV_RSA_FULL, 
                                       CRYPT_VERIFYCONTEXT);
         if (!fResult)
         {
            MyPrintf(_T("CryptAcquireContext failed with %x\n"), GetLastError());
            __leave;
         }

         *fFreeProv = TRUE;

         // Import the public key from the certificate so we can verify
         fResult = CryptImportPublicKeyInfo(*hProv,
                                            ENCODING,
                                            &(pCertContext->pCertInfo->SubjectPublicKeyInfo),
                                            hPubKey);
         if (!fResult)
         {
            MyPrintf(_T("CryptImportPublicKeyInfo failed with %x\n"), GetLastError());
            __leave;
         }         
      }

      fReturn = TRUE;
   }
   __finally
   {
      if (!fReturn)
      {
         if (*hPubKey != NULL) 
         {
            CryptDestroyKey(*hPubKey);
            *hPubKey = NULL;
         }

         if ((*fFreeProv == TRUE) && (*hProv != NULL)) 
         {
            CryptReleaseContext(*hProv, 0);
            *hProv = NULL;
            *fFreeProv = FALSE;
         }
      }
   }

   return fReturn;
}

BOOL GetRSAKeyFromContainer(LPTSTR szContainerName,
                            DWORD dwAcquireFlags,
                            DWORD dwKeySpec,
                            HCRYPTPROV *hProv,                            
                            HCRYPTKEY *hPubKey)
{
   BOOL fResult;
   BOOL fReturn = FALSE;

   __try
   {
      if (hProv == NULL || hPubKey == NULL)
      {
         __leave;
      }

      // acquire crypto context using container name
      fResult = CryptAcquireContext(hProv, 
                                    szContainerName, 
                                    MS_DEF_PROV, 
                                    PROV_RSA_FULL, 
                                    dwAcquireFlags);
      if (!fResult)
      {
         MyPrintf(_T("CryptAcquireContext failed with %x\n"), GetLastError());
         __leave;
      }
      
      // Get the key handle for verification
      fResult = CryptGetUserKey(*hProv, dwKeySpec, hPubKey);
      if (!fResult)
      {
         MyPrintf(_T("CryptGetUserKey failed with %x\n"), GetLastError());
         __leave;
      }

      fReturn = TRUE;
   }
   __finally
   {
      if (!fReturn)
      {
         if (*hPubKey != NULL) 
         {
            CryptDestroyKey(*hPubKey);
            *hPubKey = NULL;
         }

         if (*hProv != NULL) 
         {
            CryptReleaseContext(*hProv, 0);
            *hProv = NULL;
         }
      }
   }

   return fReturn;
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

void PrintUsage()
{
   MyPrintf(_T("SignHash [md5|sha1] [</s>|</v>] <DataFile> <SigFile> [</cert>|</key>]\n"));
   MyPrintf(_T("/s to Sign.\n"));
   MyPrintf(_T("/v to Verify.\n"));
   MyPrintf(_T("/cert <CertName> <StoreName> [<u>|<m>] - use a certificate.\n"));
   MyPrintf(_T("/key <ContainerName> [<u>|<m>] [<x>|<s>] use container with exchange or signature key.\n"));
}
