// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "PictureContent.h"

static const WCHAR s_wszNoPictures[] = L"<body><content id=\"1\" title=\"Picture Viewer Sample\"><txt align=\"c\">No pictures were found in the Pictures folder</txt></content></body>";

/////////////////////////////////////////////////////////////////////////
//                                          
// UTF8FromWideStr()
// 
// This function, given a wide character string (Unicode), produces a string
// in UTF8 encoding.
//
// Parameters:
//      wszIn [in]
//        The input wide character string to be converted.
//      pwszOut [out]
//        A pointer to a buffer that upon return will contain a NULL terminated
//        string in UTF8 encoding.
//      pcbOut [out]
//        The number of bytes in the buffer pointed to by pwszOut.  The caller
//        is responsible for freeing the string by calling CoTaskMemFree().
//
// Return Values:
//        S_OK : All is well.
//
/////////////////////////////////////////////////////////////////////////
HRESULT UTF8FromWideStr(
    LPCWSTR wszIn,
    LPSTR* pwszOut,
    DWORD* pcbOut
    )
{
    HRESULT hr      = S_OK;
    LPSTR   pszData = NULL;
    DWORD   cbData  = 0;
    
    //
    // Determine the number of bytes needed for the string.
    //
    cbData = WideCharToMultiByte(CP_UTF8, 0, wszIn, -1, pszData, cbData, NULL, NULL);
    if (0 == cbData)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    
    //
    // If all went well, make the allocation.  Add one to allow for a NULL
    // terminator.
    //
    if (S_OK == hr)
    {
        pszData = (LPSTR)CoTaskMemAlloc(cbData);
        if (NULL == pszData)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    //
    // Allocate the string as UTF-8 and return it.
    //
    if (S_OK == hr)
    {
        cbData = WideCharToMultiByte(CP_UTF8, 0, wszIn, -1, pszData, cbData, NULL, NULL);
        if (0 == cbData)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            CoTaskMemFree(pszData);
            pszData = NULL;
        }
    }
    *pcbOut = cbData;
    *pwszOut = pszData;
    
    return hr;
}

CPictureContent::CPictureContent()
{
    m_contentID = CONTENT_ID_GLANCE;
    m_cPictures = 0;
}

CPictureContent::~CPictureContent()
{
    //
    // Since each stored object inherits from the
    // reference counted CBaseContent object,
    // the reference should be released.
    //
    for (int i = 0; i < m_cPictures; i++)
    {
        m_PictureXml[i]->Release();
        m_PictureRaw[i]->Release();
    }
}

void CNoPicturesContent::LoadContent(DWORD* pdwSize, BYTE** ppbData, ISideShowCapabilities* /*pICapabilities*/)
{
    if (NULL == pdwSize ||
        NULL == ppbData)
    {
        return;
    }

    *ppbData = NULL;
    *pdwSize = 0;
    
    UTF8FromWideStr(s_wszNoPictures, (LPSTR*)ppbData, pdwSize);
}

void CNoPicturesContent::FreeContent(BYTE** ppbData)
{
    CoTaskMemFree(*ppbData);
}

void CPictureContent::LoadContent(DWORD* pdwSize, BYTE** ppbData, ISideShowCapabilities* /*pICapabilities*/)
{
    if (NULL == pdwSize ||
        NULL == ppbData)
    {
        return;
    }

    char szGlance[64] = "Pictures";
    int cPicture = 0;

    WIN32_FIND_DATA fileData;
    HANDLE hSearch = INVALID_HANDLE_VALUE;

    //
    // Get all the pics in the my pictures directory
    //
    WCHAR wszPicPath[MAX_PATH];
    BOOL retVal = SHGetSpecialFolderPath(
            NULL,
            wszPicPath,
            CSIDL_MYPICTURES,
            0);
            
    if (TRUE == retVal)
    {
        retVal = SUCCEEDED(StringCchCat(wszPicPath, MAX_PATH, L"\\*.jpg"));
    }
            
    if (TRUE == retVal)
    {
        hSearch = ::FindFirstFile(wszPicPath, &fileData);
    }
    
    if (INVALID_HANDLE_VALUE != hSearch)
    {
        //
        // For each picture file...
        //
        do
        {
            //
            // Create picture content object
            //
            CPicture* pic = new CPicture(cPicture + CID_XMLIMAGE_FIRST,
                                         fileData.cFileName);

            //
            // Create raw image content item
            // First, prepend the path name to the filename
            //
            size_t length = wcslen(fileData.cFileName) + 14;
            WCHAR* szFilePath = new WCHAR[length];
            StringCchPrintfW(szFilePath, length, L"%s", fileData.cFileName);

            CImageContent* img = new CImageContent(cPicture + CID_RAWIMAGE_FIRST,
                                                   szFilePath);

            delete[] szFilePath;

            //
            // Acd the content item to the arrays
            //
            m_PictureXml[cPicture] = pic;
            m_PictureRaw[cPicture] = img;

            cPicture++;

        } while (::FindNextFile(hSearch, &fileData) && cPicture < MAX_PICTURES);

        //
        // Have the content wrap around at the edges
        //
        m_PictureXml[cPicture-1]->SetNextID(1);
        m_PictureXml[0]->SetPrevID(cPicture);

        //
        // Store the number of pictures (1-based)
        //
        m_cPictures = cPicture;

        //
        // Close the search handle
        //
        ::FindClose(hSearch);
    }

    StringCchPrintfA(szGlance, sizeof(szGlance)/sizeof(char), "There are %d pictures to view", m_cPictures);

    *pdwSize = (DWORD)strlen(szGlance) + 1;
    *ppbData = new BYTE[*pdwSize];

    StringCchCopyA((char*)*ppbData, *pdwSize, szGlance);
}

void CPictureContent::FreeContent(BYTE** ppbData)
{
    //
    // Free the memory allocated in LoadContent
    //
    if (NULL != ppbData)
    {
        delete [] *ppbData;
    }
}

ISideShowContent* CPictureContent::GetContent(CONTENT_ID id)
{
    ISideShowContent*   pContent = NULL;
    
    //
    // If there are no pictures, return a NULL content.
    //
    if (0 == m_cPictures)
    {
        return new CNoPicturesContent;
    }

    //
    // If the CONTENT_ID is in the range (CID_XMLIMAGE_FIRST, CID_RAWIMAGE_FIRST - 1),
    // then it corresponds to the XML content page.  Otherwise, it corresponds to
    // the raw image data.
    //
    if (CID_RAWIMAGE_FIRST > id && CID_XMLIMAGE_FIRST <= id)
    {
        //
        // We get the index by subtracting the CONTENT_ID from CID_XMLIMAGE_FIRST,
        // the first content ID of the XML page.
        //
        int index = id - CID_XMLIMAGE_FIRST;
        if (index < MAX_PICTURES)
        {
            pContent = m_PictureXml[index];                   
        }
    }
    else if (CID_RAWIMAGE_FIRST <= id)
    {
        //
        // We get the index by subtracting the CONTENT_ID from CID_RAWIMAGE_FIRST,
        // the first content ID of raw images.
        //
        int index = id - CID_RAWIMAGE_FIRST;
        if (index < MAX_PICTURES)
        {
            pContent = m_PictureRaw[index];                   
        }
    }

    return pContent;
}

int CPictureContent::GetPictureCount()
{
    return m_cPictures;
}
