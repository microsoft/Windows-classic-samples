// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CPictureContent;
class CPictureEvents;

class CPictureClient :
    public CBaseClient
{
private:
    CPictureContent    *m_pContent;
    CBaseEvents        *m_pEvents;
    
    HANDLE              m_hThread;
    CRITICAL_SECTION    m_cs;

public:
    CPictureClient();
    virtual ~CPictureClient();

    virtual void AddContent();
    virtual void RemoveAllContent();
    
    HRESULT DeviceAdd(
        ISideShowCapabilities* pIDevice
        );
    
    HRESULT DeviceRemove(
        ISideShowCapabilities* pIDevice
        );
        
    HRESULT ContentMissing(
        const CONTENT_ID contentId,
        ISideShowContent** ppIContent
        );
        
    static DWORD WINAPI DirectoryThreadProc(LPVOID ThreadParameter);
};
