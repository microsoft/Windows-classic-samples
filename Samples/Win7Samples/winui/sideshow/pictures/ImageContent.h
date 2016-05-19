// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
#include <gdiplus.h>

#pragma once

class CImageContent :
    public CBaseContent
{
private:
    LPWSTR m_pwszFile;

protected:
    virtual void CreateDeviceImage(LPWSTR wszFileName, LPWSTR wszDeviceImage, Gdiplus::Rect rect);
    virtual void LoadContent(DWORD* pdwSize, BYTE** ppbData, ISideShowCapabilities* pICapabilities);
    virtual void FreeContent(BYTE** ppbData);

public:
    CImageContent();
    CImageContent(CONTENT_ID id, LPWSTR pwszFile);
    virtual ~CImageContent();
};
