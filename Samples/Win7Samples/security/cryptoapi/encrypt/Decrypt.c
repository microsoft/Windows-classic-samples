/******************************************************************************\
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1996 - 2000.  Microsoft Corporation.  All rights reserved.

\******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wincrypt.h>


#ifdef USE_BLOCK_CIPHER
    // defines for RC2 block cipher
    #define ENCRYPT_ALGORITHM   CALG_RC2
    #define ENCRYPT_BLOCK_SIZE  8
#else
    // defines for RC4 stream cipher
    #define ENCRYPT_ALGORITHM   CALG_RC4
    #define ENCRYPT_BLOCK_SIZE  1
#endif

#define KEYLENGTH 0x00800000

static BOOL CAPIDecryptFile(PCHAR szSource, PCHAR szDestination, PCHAR szPassword);

/*****************************************************************************/
void _cdecl main(int argc, char *argv[])
{
    PCHAR szSource      = NULL;
    PCHAR szDestination = NULL;
    PCHAR szPassword    = NULL;

    // Validate argument count.
    if(argc != 3 && argc != 4) {
        printf("USAGE: decrypt <source file> <dest file> [ <password> ]\n");
        exit(1);
    }

    // Parse arguments.
    szSource       = argv[1];
    szDestination  = argv[2];
    if(argc == 4) {
        szPassword = argv[3];
    }

    if(!CAPIDecryptFile(szSource, szDestination, szPassword)) {
        printf("Error encrypting file!\n");
        exit(1);
    }

    exit(0);
}

/*****************************************************************************/
static BOOL CAPIDecryptFile(PCHAR szSource, PCHAR szDestination, PCHAR szPassword)
{
    FILE *hSource      = NULL;
    FILE *hDestination = NULL;
	errno_t err;
    INT eof = 0;

    HCRYPTPROV hProv   = 0;
    HCRYPTKEY hKey     = 0;
    HCRYPTHASH hHash   = 0;

    PBYTE pbKeyBlob = NULL;
    DWORD dwKeyBlobLen;

    PBYTE pbBuffer = NULL;
    DWORD dwBlockLen;
    DWORD dwBufferLen;
    DWORD dwCount;

    BOOL status = FALSE;

    // Open source file.
	err = fopen_s(&hSource,szSource,"rb");
    if(err !=0) {
        printf("Error opening Ciphertext file!\n");
        goto done;
    }

    // Open destination file.
	err = fopen_s(&hDestination,szDestination,"wb");
    if(err != 0) {
        printf("Error opening Plaintext file!\n");
        goto done;
    }

    // Get handle to the default provider.
    if(!CryptAcquireContext(&hProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, 0)) {
        printf("Error %x during CryptAcquireContext!\n", GetLastError());
        goto done;
    }

    if(szPassword == NULL) {
        // Decrypt the file with the saved session key.

        // Read key blob length from source file and allocate memory.
        fread(&dwKeyBlobLen, sizeof(DWORD), 1, hSource);
        if(ferror(hSource) || feof(hSource)) {
            printf("Error reading file header!\n");
            goto done;
        }
        if((pbKeyBlob = malloc(dwKeyBlobLen)) == NULL) {
            printf("Out of memory or improperly formatted source file!\n");
            goto done;
        }

        // Read key blob from source file.
        fread(pbKeyBlob, 1, dwKeyBlobLen, hSource);
        if(ferror(hSource) || feof(hSource)) {
            printf("Error reading file header!\n");
            goto done;
        }

        // Import key blob into CSP.
        if(!CryptImportKey(hProv, pbKeyBlob, dwKeyBlobLen, 0, 0, &hKey)) {
            printf("Error %x during CryptImportKey!\n", GetLastError());
            goto done;
        }
    } else {
        // Decrypt the file with a session key derived from a password.

        // Create a hash object.
        if(!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
            printf("Error %x during CryptCreateHash!\n", GetLastError());
            goto done;
        }

        // Hash in the password data.
        if(!CryptHashData(hHash, (PBYTE)szPassword, (DWORD)strlen(szPassword), 0)) {
            printf("Error %x during CryptHashData!\n", GetLastError());
            goto done;
        }

        // Derive a session key from the hash object.
        if(!CryptDeriveKey(hProv, ENCRYPT_ALGORITHM, hHash, KEYLENGTH, &hKey)) {
            printf("Error %x during CryptDeriveKey!\n", GetLastError());
            goto done;
        }

        // Destroy the hash object.
        CryptDestroyHash(hHash);
        hHash = 0;
    }

    // Determine number of bytes to decrypt at a time. This must be a multiple
    // of ENCRYPT_BLOCK_SIZE.
    dwBlockLen = 1000 - 1000 % ENCRYPT_BLOCK_SIZE;
    dwBufferLen = dwBlockLen;

    // Allocate memory.
    if((pbBuffer = malloc(dwBufferLen)) == NULL) {
        printf("Out of memory!\n");
        goto done;
    }

    // Decrypt source file and write to destination file.
    do {
        // Read up to 'dwBlockLen' bytes from source file.
        dwCount = (DWORD)fread(pbBuffer, 1, dwBlockLen, hSource);
        if(ferror(hSource)) {
            printf("Error reading Ciphertext!\n");
            goto done;
        }
        eof = feof(hSource);

        // Decrypt data
        if(!CryptDecrypt(hKey, 0, eof, 0, pbBuffer, &dwCount)) {
            printf("Error %x during CryptDecrypt!\n", GetLastError());
            goto done;
        }

        // Write data to destination file.
        fwrite(pbBuffer, 1, dwCount, hDestination);
        if(ferror(hDestination)) {
            printf("Error writing Plaintext!\n");
            goto done;
        }
    } while(!feof(hSource));

    status = TRUE;

    printf("OK\n");

    done:

    // Close files.
    if(hSource) fclose(hSource);
    if(hDestination) fclose(hDestination);

    // Free memory.
    if(pbKeyBlob) free(pbKeyBlob);
    if(pbBuffer) free(pbBuffer);

    // Destroy session key.
    if(hKey) CryptDestroyKey(hKey);

    // Destroy hash object.
    if(hHash) CryptDestroyHash(hHash);

    // Release provider handle.
    if(hProv) CryptReleaseContext(hProv, 0);

    return(status);
}
