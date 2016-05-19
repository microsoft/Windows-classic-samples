/************************************************************************
 *  This file is part of the Microsoft Windows SDK Code Samples.
 * 
 *  Copyright (C) Microsoft Corporation.  All rights reserved.
 * 
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * 
 ************************************************************************/


#include <dwrite.h>
#include <string.h>
#include <stdio.h>
#include <new>

// SafeRelease inline function.
template <class T> inline void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

void wmain()
{
    IDWriteFactory* pDWriteFactory = NULL;

    HRESULT hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&pDWriteFactory)
            );

    IDWriteFontCollection* pFontCollection = NULL;

    // Get the system font collection.
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory->GetSystemFontCollection(&pFontCollection);
    }

    UINT32 familyCount = 0;

    // Get the number of font families in the collection.
    if (SUCCEEDED(hr))
    {
        familyCount = pFontCollection->GetFontFamilyCount();
    }

    for (UINT32 i = 0; i < familyCount; ++i)
    {
        IDWriteFontFamily* pFontFamily = NULL;

        // Get the font family.
        if (SUCCEEDED(hr))
        {
            hr = pFontCollection->GetFontFamily(i, &pFontFamily);
        }

        IDWriteLocalizedStrings* pFamilyNames = NULL;
        
        // Get a list of localized strings for the family name.
        if (SUCCEEDED(hr))
        {
            hr = pFontFamily->GetFamilyNames(&pFamilyNames);
        }

        UINT32 index = 0;
        BOOL exists = false;
        
        wchar_t localeName[LOCALE_NAME_MAX_LENGTH];

        if (SUCCEEDED(hr))
        {
            // Get the default locale for this user.
            int defaultLocaleSuccess = GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);

            // If the default locale is returned, find that locale name, otherwise use "en-us".
            if (defaultLocaleSuccess)
            {
                hr = pFamilyNames->FindLocaleName(localeName, &index, &exists);
            }
            else
            {
                hr = pFamilyNames->FindLocaleName(L"en-us", &index, &exists);
            }
        }
        
        // If the specified locale doesn't exist, select the first on the list.
        if (!exists)
            index = 0;

        UINT32 length = 0;

        // Get the string length.
        if (SUCCEEDED(hr))
        {
            hr = pFamilyNames->GetStringLength(index, &length);
        }

        // Allocate a string big enough to hold the name.
        wchar_t* name = new (std::nothrow) wchar_t[length+1];
		if (name == NULL)
        {
			hr = E_OUTOFMEMORY;
        }

        // Get the family name.
        if (SUCCEEDED(hr))
        {
            hr = pFamilyNames->GetString(index, name, length+1);
        }
        if (SUCCEEDED(hr))
        {
            // Print out the family name.
            wprintf(L"%s\n", name);
        }

		SafeRelease(&pFontFamily);
		SafeRelease(&pFamilyNames);

        delete [] name;
    }

	SafeRelease(&pFontCollection);
	SafeRelease(&pDWriteFactory);
}

