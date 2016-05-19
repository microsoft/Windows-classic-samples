// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "ImageContent.h"

using namespace Gdiplus;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
   {
      return -1;  // Failure
   }

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
   {
      return -1;  // Failure
   }

   GetImageEncoders(num, size, pImageCodecInfo);

   for (UINT j = 0; j < num; ++j)
   {
      if ( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   
   return -1;  // Failure
}

CImageContent::CImageContent()
{
    m_contentID = CONTENT_ID_GLANCE;
    m_pwszFile = NULL;
}

CImageContent::CImageContent(CONTENT_ID id, LPWSTR pwszFile)
{
    m_contentID = id;

    //
    // Copy the filename locally so we can delay-load the file
    // 
    if (NULL != pwszFile)
    {
        size_t length = wcslen(pwszFile) + 1;
        m_pwszFile = new WCHAR[length];
        if (NULL != m_pwszFile)
        {           
            StringCchCopyW(m_pwszFile, length, pwszFile);
        }
    }
}

CImageContent::~CImageContent()
{
    if (NULL != m_pwszFile)
    {
        delete [] m_pwszFile;
        m_pwszFile = NULL;
    }
}

void CImageContent::CreateDeviceImage(
    LPWSTR wszFileName,
    LPWSTR wszDeviceImage,
    Rect rect)
{
    HRESULT hr = E_FAIL;
    
    //
    // Open the file handle
    //
    UINT newH, newW;
    WCHAR wszImagePath[MAX_PATH];
    WCHAR wszDirPath[MAX_PATH];
    wszImagePath[0] = 0;
    wszDirPath[0] = 0;
    BOOL retVal = SHGetSpecialFolderPath(
            NULL,
            wszDirPath,
            CSIDL_MYPICTURES,
            0);
    if (TRUE == retVal)
    {
        hr = StringCchCat(wszDirPath, MAX_PATH, L"\\");
    }
    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(wszImagePath, MAX_PATH, wszDirPath);
    }
    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(wszImagePath, MAX_PATH, wszFileName);
    }
    
    Image* pImage = NULL;
    if (SUCCEEDED(hr))
    {
        pImage = Image::FromFile(wszImagePath);
    }

    newH = rect.Height;
    newW = rect.Width;

    if (NULL != pImage)
    {
        UINT origH = pImage->GetHeight();
        UINT origW = pImage->GetWidth();

        if (origH > origW)
        {
            newH = rect.Height;
            newW = (int)(origW * ((float)rect.Height / (float)origH));
        }
        else
        {
            newW = rect.Width;
            newH = (int)(origH * ((float)rect.Width / (float)origW));
        }
    }
    
    if (NULL != pImage)
    {
        Status status;
        Bitmap* pBitmap = new Bitmap(newW, newH);
        Graphics graphics(pBitmap);
        status = graphics.DrawImage(pImage, 0, 0, newW, newH);
        
        //
        // Save the new version of the image to file.
        //
        if (NULL != pBitmap)
        {
            //
            // Create a temporary filename
            //
            WCHAR wszTempPath[MAX_PATH];
            DWORD dwResults = 0;
            dwResults = GetTempPath(MAX_PATH, wszTempPath);
            
            if (0 != dwResults)
            {
                dwResults = GetTempFileName(
                    wszTempPath,
                    wszFileName,
                    0,
                    wszDeviceImage
                    );
            }
            
            if (0 != dwResults)
            {
                CLSID jpgClsid;
                if (-1 != GetEncoderClsid(L"image/jpeg", &jpgClsid))
                {
                    status = pBitmap->Save(wszDeviceImage, &jpgClsid, NULL);
                }
            }
        }
        delete pImage;
        delete pBitmap;
    }
}

void CImageContent::LoadContent(DWORD* pdwSize, BYTE** ppbData, ISideShowCapabilities* pICapabilities)
{
    if (NULL == pdwSize ||
        NULL == ppbData ||
        NULL == m_pwszFile)
    {
        return;
    }

    WCHAR wszDeviceImage[MAX_PATH];
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HRESULT hr = E_FAIL;
    *ppbData = NULL;
    *pdwSize = 0;

    //
    // Examine the device capabilities and get the specific screen size
    // so that the bitmaps can be formated for the device.
    //
    PROPVARIANT pvHeight;
    PROPVARIANT pvWidth;
    PropVariantInit(&pvHeight);
    PropVariantInit(&pvWidth);
    hr = pICapabilities->GetCapability(SIDESHOW_CAPABILITY_CLIENT_AREA_WIDTH, &pvWidth);
    if (SUCCEEDED(hr))
    {
        hr = pICapabilities->GetCapability(SIDESHOW_CAPABILITY_CLIENT_AREA_HEIGHT, &pvHeight);
    }
    Rect rect(0, 0, pvWidth.uiVal, pvHeight.uiVal);
    CreateDeviceImage(m_pwszFile, wszDeviceImage, rect);
    
    //
    // Open the file handle
    //
    hFile = ::CreateFile(wszDeviceImage,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        //
        // Get the file length
        //
        DWORD dwFileSize = ::GetFileSize(hFile, NULL);
        DWORD dwBytesRead = 0;

        if (INVALID_FILE_SIZE != dwFileSize)
        {
            //
            // Allocate memory to read the file in
            //
            *ppbData = new BYTE[dwFileSize];
            if (NULL != *ppbData)
            {
                //
                // Read the file
                //
                if (FALSE == ::ReadFile(hFile, *ppbData, dwFileSize, &dwBytesRead, NULL))
                {
                    // XXX
                }

                //
                // Set the return size
                //
                *pdwSize = dwBytesRead;
            }
        }

        ::CloseHandle(hFile);
    }
    
    PropVariantClear(&pvHeight);
    PropVariantClear(&pvWidth);
}

void CImageContent::FreeContent(BYTE** ppbData)
{
    //
    // Free the memory allocated in LoadContent
    //
    if (NULL != ppbData)
    {
        delete [] *ppbData;
    }
}
