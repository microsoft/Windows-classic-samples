// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "Picture.h"
#include "ImageContent.h"

#define MAX_PICTURES    100

class CNoPicturesContent : 
    public CBaseContent
{
public:
    CNoPicturesContent(){m_contentID = 1;}
    virtual ~CNoPicturesContent(){}
    
    virtual void LoadContent(DWORD* pdwSize, BYTE** ppbData, ISideShowCapabilities* pICapabilities);
    virtual void FreeContent(BYTE** ppbData);
};

class CPictureContent :
    public CBaseContent
{
private:
    CPicture*       m_PictureXml[MAX_PICTURES];
    CImageContent*  m_PictureRaw[MAX_PICTURES];
    int             m_cPictures;

protected:
    virtual void LoadContent(DWORD* pdwSize, BYTE** ppbData, ISideShowCapabilities* pICapabilities);
    virtual void FreeContent(BYTE** ppbData);

public:
    CPictureContent();
    virtual ~CPictureContent();

    int GetPictureCount();

    ISideShowContent* GetContent(CONTENT_ID id);
};



