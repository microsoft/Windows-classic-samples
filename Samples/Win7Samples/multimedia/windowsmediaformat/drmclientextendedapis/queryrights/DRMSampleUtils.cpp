///////////////////////////////////////////////////////////////////////////////
//
// DRMSampleUtils.cpp : Contains implementation of common functions for 
//  DRM samples.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////
#include "DRMSampleUtils.h"

///////////////////////////////////////////////////////////////////////////////
//
// Function: DisplayError
// Description: Displays a specified error string.
// Parameters: ErrorCode - HRESULT error code.
//             pwszMessage - Message to display.
// Notes: This function is proveded so that chages can be made to the way 
//  errors are reported without an impact on the reporting code.
//
///////////////////////////////////////////////////////////////////////////////
void DisplayError(HRESULT ErrorCode, const wchar_t* pwszMessage)
{
    wprintf(L"%s\nError Code = 0x%8X\n", pwszMessage, ErrorCode);
}


///////////////////////////////////////////////////////////////////////////////
//
// Function: ParseKIDFile
// Description: Parses a text file containing one or more KID strings into an
//  array of KID strings.
// Parameters: pwszInFile  - Path of the file to parse.
//             KIDStrings - Array of strings that receives the KID strings.
//              The array is allocated by this function. The function will fail
//              if it is not NULL on input.
//
// Notes: The following text shows the format of the input file"
//
// KIDFILE
// n
// <KIDString>
// ...
// 
// Where n is the number of KID strings in the file.
///////////////////////////////////////////////////////////////////////////////
HRESULT ParseKIDFile(WCHAR* pwszInFile, WCHAR*** pppKIDStrings, int* pNumStrings)
{
    HRESULT  hr    = S_OK;

    FILE*    pFile = NULL;

    int      error = 0;
    wchar_t  pwszTempString[g_TempStringSize];

    size_t   cchString           = 0;
    int      StringCompareCode = 0;

    // Check the array parameter.
    if (*pppKIDStrings != NULL)
    {
        hr = E_POINTER;
        return hr;
    }

    // Open the file for reading.
    if (SUCCEEDED(hr))
    {
        if (_wfopen_s(&pFile, pwszInFile, L"r") != 0)
        {
            hr = E_FAIL;
            DisplayError(hr, L"The specified filename is invalid.");
        }
    }    

    // Read the first token of the KID file.
    if (SUCCEEDED(hr))
    {
        error = fwscanf_s(pFile, L"%s", pwszTempString, g_TempStringSize);

        if (error != 1)
        {
            hr = E_FAIL;
            DisplayError(hr, L"KID file is improperly formatted.");
        }
    }
      
    // Check the read token against the expected file header.
    if (SUCCEEDED(hr))
    {
        // Get the length of the token.
        cchString = wcsnlen(pwszTempString, g_TempStringSize);

        // Compare the strings.
        StringCompareCode = wcsncmp(pwszTempString, 
                                    g_wszKIDFileHeaderString, 
                                    (cchString));
        
        if (StringCompareCode != 0)
        {
            hr = E_FAIL;
            DisplayError(hr, L"The specified file is not a valid KID File.");
        }
    }

    // Get the number of KID entries in the file.
    if (SUCCEEDED(hr))
    {
        error = fwscanf_s(pFile, L"%d", pNumStrings);

        if (error != 1)
        {
            hr = E_FAIL;
            DisplayError(hr, L"Error reading the number of KID entries.");
        }
        // Check that the number of strings is positive.
        else if (*pNumStrings <= 0)
        {
            hr = E_UNEXPECTED;
            DisplayError(hr, L"KID file specifies zero KID strings.");
        }
    }

    // Allocate memory for the KID string array.
    if (SUCCEEDED(hr))
    {
        // First allocate the array.
        *pppKIDStrings = new WCHAR*[*pNumStrings];

        if (*pppKIDStrings != NULL)
        {
            // Initialize the array.
            ZeroMemory(*pppKIDStrings, *pNumStrings * sizeof(WCHAR*)); 
        }
        else
        {
            hr = E_OUTOFMEMORY;
            DisplayError(hr, L"Couldn't allocate memory.");
        }
    }

    // Loop through the KID strings, allocating memory for each array member.
    for (int i = 0; i < *pNumStrings; i++)
    {
        // Get the next string.
        error = fwscanf_s(pFile, L"%s", pwszTempString, g_TempStringSize);

        if (error != 1)
        {
            hr = E_FAIL;
            DisplayError(hr, L"Could not read KID string from file.");
            break;
        }

        // Get the size of the retrieved string.
        cchString = wcsnlen(pwszTempString, g_TempStringSize);

        // Add one to the size to account for the terminator.
        cchString++;

        // Allocte memory for the string in the array.
        (*pppKIDStrings)[i] = new WCHAR[cchString];

        if ((*pppKIDStrings)[i] == NULL)
        {
            hr = E_OUTOFMEMORY;
            DisplayError(hr, L"Couldn't allocate memory.");
            break;
        }

        // Copy the string to the array.
        error = wcscpy_s((*pppKIDStrings)[i], cchString, pwszTempString);
        if (error != 0)
        {
            hr = E_FAIL;
            DisplayError(hr, L"Could not copy KID string.");
            break;
        }

        // Get ready for the next pass.
        cchString = 0;
        pwszTempString[0] = NULL;
    }

    // Clean up.

    // Release memory for the KID string array if failed.
    if ((FAILED(hr)) && (*pppKIDStrings != NULL))
    {
        for (int i = 0; i < *pNumStrings; i++)
        {
            SAFE_ARRAY_DELETE((*pppKIDStrings)[i]);
        }

        SAFE_ARRAY_DELETE(*pppKIDStrings);
    }

    // Close the file.
    SAFE_FILE_CLOSE(pFile);

    return hr;

}