// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#define HALF_BYTE_TO_HEX_WCHAR(b) (((b) < 10) ? (L'0' + (b)) : (L'A' + (b) - 10))

DWORD
StringToSsid(
    __in LPCWSTR strSsid,
    __out PDOT11_SSID pDot11Ssid
    )
{
    DWORD dwError = NO_ERROR;
    BYTE  pbSsid[DOT11_SSID_MAX_LENGTH + 1];
    DWORD dwBytes;
 
    if (strSsid == NULL || pDot11Ssid == NULL || wcslen(strSsid) == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

    dwBytes = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
        strSsid, -1, (LPSTR)pbSsid, sizeof(pbSsid), NULL, NULL);

    if (dwBytes == 0)
    {
        // Conversion failed.
        dwError = GetLastError();
        
        if (dwError == ERROR_INSUFFICIENT_BUFFER)
        {
            dwError = ERROR_BAD_LENGTH;
        }

        goto error;
    }
    else if (dwBytes == 1)
    {
        // Zero-length SSID.
        dwError = ERROR_BAD_LENGTH;
        goto error;
    }
    else
    {
        // Conversion succeeded and length valid.
        pDot11Ssid->uSSIDLength = dwBytes - 1;
        memcpy(pDot11Ssid->ucSSID, pbSsid, pDot11Ssid->uSSIDLength);
    }
    
error:

    return dwError;
}

 __success(ERROR_SUCCESS)
DWORD
SsidToDisplayName(
    __in PDOT11_SSID pDot11Ssid,
    __in BOOL bHexFallback,
    __out_ecount_opt(*pcchDisplayName) LPWSTR strDisplayName,
    __inout DWORD *pcchDisplayName
    )
{
    DWORD dwError = NO_ERROR;
    DWORD cchDisplayName, i, j;
    BYTE hb;
    WCHAR strHexResource[256];
    DWORD cchHexResource = sizeof(strHexResource);
    WCHAR strSSID[WLAN_MAX_NAME_LENGTH];
    LPWSTR pszArgs[1] = {NULL};

    if (pDot11Ssid == NULL || 
        pcchDisplayName == NULL ||
        (strDisplayName == NULL && *pcchDisplayName != 0) ||
        pDot11Ssid->uSSIDLength > DOT11_SSID_MAX_LENGTH)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

    LoadString (
        GetModuleHandle(NULL),
        IDS_ERROR_CANNOT_DISPLAY_SSID,
        strHexResource,
        256
    );
    strDisplayName[0] = L'\0';
    if (pDot11Ssid->uSSIDLength == 0)
    {
        cchDisplayName = 0;
    }
    else
    {
        // Convert to get length, not including null.
        cchDisplayName = MultiByteToWideChar(CP_ACP, 0,
            (LPCSTR)pDot11Ssid->ucSSID, pDot11Ssid->uSSIDLength, 
            NULL, 0);
    }

    if (pDot11Ssid->uSSIDLength ==0 || cchDisplayName > 0)
    {
        // Length including null.
        cchDisplayName ++;
        
        // Conversion succeeded.
        if (*pcchDisplayName < cchDisplayName)
        {
            // Insufficient buffer.
            *pcchDisplayName = cchDisplayName;
            dwError = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        {
            // Sufficient buffer.
            if (pDot11Ssid->uSSIDLength > 0)
            {
                cchDisplayName = MultiByteToWideChar(CP_ACP, 0,
                    (LPCSTR)pDot11Ssid->ucSSID, pDot11Ssid->uSSIDLength, 
                    strDisplayName, *pcchDisplayName);

                if (cchDisplayName == 0)
                {                
                    dwError = GetLastError();
                    goto error;
                }

                cchDisplayName++;
            }

            // Succeeded.
            if (cchDisplayName < *pcchDisplayName)
            {
                strDisplayName[cchDisplayName - 1] = L'\0';
                *pcchDisplayName = cchDisplayName;
            }
            else
            {
                // This should not happen, added to avoid prefast warning
                _ASSERT(FALSE);
                dwError = ERROR_INVALID_PARAMETER;
                goto error;
            }
        }
    }
    else if (bHexFallback)
    {
        // ACP Conversion failed. Try Hex conversion.
        
        // Display name length including null.
        cchDisplayName = cchHexResource + 2 * pDot11Ssid->uSSIDLength + 1;
        
        if (*pcchDisplayName < cchDisplayName)
        {
            // Insufficient buffer.
            *pcchDisplayName = cchDisplayName;
            dwError = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        {
            // Sufficient buffer.
            for (i = 0, j = 0; i < pDot11Ssid->uSSIDLength; i++)
            {
                hb = pDot11Ssid->ucSSID[i] >> 4;
                strSSID[j++] = HALF_BYTE_TO_HEX_WCHAR(hb);
                hb = pDot11Ssid->ucSSID[i] & 0x0F;
                strSSID[j++] = HALF_BYTE_TO_HEX_WCHAR(hb);
            }
            strSSID[j] = L'\0';

            pszArgs[0] = strSSID;
            cchDisplayName = FormatMessage(
                              FORMAT_MESSAGE_FROM_STRING 
                              | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                              strHexResource,
                              0,
                              0,
                              strDisplayName,
                              *pcchDisplayName,
                              (va_list *)pszArgs
                             );
            if (cchDisplayName == 0)
            {
                dwError = GetLastError();
                goto error;
            }
            *pcchDisplayName = cchDisplayName;
        }                    
    }    

error:
    return dwError;
}

DWORD 
ConvertPassPhraseKeyStringToBuffer(
    __in_ecount(dwLength) LPCWSTR strPassKeyString,     // Unicode string 
    __in DWORD dwLength,
    __in DOT11_AUTH_ALGORITHM dot11Auth,
    __out_ecount_opt(*pdwBufLen) UCHAR* strPassKeyBuf,  // NULL to get length required
    __inout DWORD *pdwBufLen                            // in: length of buffer; out: chars copied/required
    )

{
    DWORD dwError = NO_ERROR;
    DWORD dwActualLen = dwLength;
    UCHAR *lpstrKeyMaterial = NULL;
    DWORD dwAllocLen = 0;
    DWORD dwKeyBytes = 0;
    BOOL bUnmappableChar = FALSE;
    DWORD dwReqdBytes = 0;
    BOOL bPassPhrase = FALSE;

    if (!strPassKeyString)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL();
    }

    while ((dwActualLen != 0) && !strPassKeyString[dwActualLen-1])
    {
        dwActualLen--;
    }

    if (dwActualLen == 0)
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_ERROR(dwError);
    }

    dwAllocLen = WideCharToMultiByte(
        CP_ACP,
        WC_NO_BEST_FIT_CHARS,
        strPassKeyString,
        dwLength,
        NULL,
        0,
        NULL,
        NULL
        );
    if (!dwAllocLen)
    {
        dwError = GetLastError();
        BAIL_ON_ERROR(dwError);
    }
    
    lpstrKeyMaterial = (UCHAR *)WlanAllocateMemory(dwAllocLen);
    if (NULL == lpstrKeyMaterial)
    {
        dwError = GetLastError();
        BAIL_ON_ERROR(dwError);
    }

    ZeroMemory(
        lpstrKeyMaterial,
        dwAllocLen
    );

    dwKeyBytes = WideCharToMultiByte(
        CP_ACP,
        WC_NO_BEST_FIT_CHARS,
        strPassKeyString,
        dwLength,
        (LPSTR)lpstrKeyMaterial,
        dwAllocLen,
        NULL,
        &bUnmappableChar
        );
    
    if (!dwKeyBytes)
    {
        dwError = GetLastError();
        BAIL_ON_ERROR(dwError);
    }
    if (dwKeyBytes != dwAllocLen)
    {
        dwError = ERROR_INTERNAL_ERROR;
        BAIL_ON_ERROR(dwError);
    }

    if (bUnmappableChar)
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_ERROR(dwError);
    }

    _ASSERT(DOT11_AUTH_ALGO_RSNA_PSK == dot11Auth);
    switch (dot11Auth)
    {
        case DOT11_AUTH_ALGO_RSNA_PSK:
            if ((dwKeyBytes > 63) ||    // Max length
                (dwKeyBytes < 8))       // Min length
            {
                dwError = ERROR_BAD_FORMAT;
                BAIL_ON_ERROR(dwError);
            }

            // Include an extra byte for the NULL terminator in a passphrase
            dwReqdBytes = dwKeyBytes + 1;
            break;

        default:
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_ERROR(dwError);
    }

    if (!strPassKeyBuf || (*pdwBufLen < dwReqdBytes))
    {
        dwError = ERROR_MORE_DATA;
        BAIL_ON_ERROR(dwError);
    }

    ZeroMemory(
        strPassKeyBuf,
        dwReqdBytes
        );

    CopyMemory(
        strPassKeyBuf,
        lpstrKeyMaterial,
        dwKeyBytes
        );
error:
    if (pdwBufLen)
    {
        *pdwBufLen = dwReqdBytes;
    }

    if (lpstrKeyMaterial)
    {
        WlanFreeMemory(lpstrKeyMaterial);
    }

    
    return dwError;
}
