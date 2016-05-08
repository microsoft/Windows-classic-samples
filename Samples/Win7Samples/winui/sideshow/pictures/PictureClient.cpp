// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "PictureClient.h"
#include "PictureContent.h"

extern DWORD g_threadID;

//
// This APPLICATION_ID is the unique ID for this
// Windows SideShow application. It is a GUID.
//

// {00BA1AD8-5381-449b-9A13-551FA4ADE0F2}
static const APPLICATION_ID PICS_APPLICATION_ID =
{ 0xba1ad8, 0x5381, 0x449b, { 0x9a, 0x13, 0x55, 0x1f, 0xa4, 0xad, 0xe0, 0xf2 } };

CPictureClient::CPictureClient()
: m_pContent(NULL), m_pEvents(NULL), m_hThread(NULL)
{
    m_applicationID = PICS_APPLICATION_ID;

    if (!::InitializeCriticalSectionAndSpinCount(&m_cs, 0))
    {
        return;
    }

    //
    // Setup a thread that monitors the picture directory and handles
    // directory change events.
    //
    m_hThread = CreateThread(
              NULL,     // Default security attributes
              0,        // Default initial stack size
              CPictureClient::DirectoryThreadProc,
              this, // pass this as parameter
              0,        // Create flags
              NULL);    // No thread ID

}

CPictureClient::~CPictureClient()
{
    ::DeleteCriticalSection(&m_cs);

    if (NULL != m_pContent)
    {
        m_pContent->Release();
        m_pContent = NULL;
    }

    if (NULL != m_pEvents)
    {
        m_pEvents->Release();
        m_pEvents = NULL;
    }
}

void CPictureClient::AddContent()
{
    ::EnterCriticalSection(&m_cs);

    if (NULL != m_pContentMgr)
    {
        //
        // Create the objects that implement the COM interfaces;
        // remember we have to Release (rather than delete) them
        // later.
        //
        if (NULL == m_pContent)
        {
            m_pContent = new CPictureContent();
        }
        if (NULL == m_pEvents)
        {
            //
            // Set the event sink for platform events
            //
            m_pEvents = new CBaseEvents();
            
            if (NULL != m_pEvents)
            {
                //
                // Create the event handlers for the events of interest
                //
                CDeviceEvent<CPictureClient>* pDeviceAddHandler = new CDeviceEvent<CPictureClient>(this, &CPictureClient::DeviceAdd);
                m_pEvents->RegisterDeviceAddEvent(pDeviceAddHandler);
                
                CDeviceEvent<CPictureClient>* pDeviceRemoveHandler = new CDeviceEvent<CPictureClient>(this, &CPictureClient::DeviceRemove);
                m_pEvents->RegisterDeviceRemoveEvent(pDeviceRemoveHandler);
                
                CContentMissingEvent<CPictureClient>* pContentMissing = new CContentMissingEvent<CPictureClient>(this, &CPictureClient::ContentMissing);
                m_pEvents->RegisterContentMissingEvent(pContentMissing);
                
                (void)m_pContentMgr->SetEventSink(m_pEvents);
            }
        }

        //
        // Add the content and picture data to the platform
        //
        AddContentItem(m_pContent);
        
        //
        // If there are no pictures, then we send a content page that indicates
        // to the user that they need to add pictures to their picture
        // directory.
        //
        if (0 == m_pContent->GetPictureCount())
        {
            AddContentItem(m_pContent->GetContent(1));
        }

        //
        // First, add the XML page for each picture to the device.
        //
        for (int i = 0; i < m_pContent->GetPictureCount(); i++)
        {
            AddContentItem(m_pContent->GetContent(CID_XMLIMAGE_FIRST + i));
        }

        //
        // Then, add the actual content.
        //
        for (int i = 0; i < m_pContent->GetPictureCount(); i++)
        {
            AddContentItem(m_pContent->GetContent(CID_RAWIMAGE_FIRST + i));
        }
    }

    ::LeaveCriticalSection(&m_cs);
}

HRESULT CPictureClient::DeviceAdd(
        ISideShowCapabilities* /*pIDevice*/
        )
{
    AddContent();
    
    return S_OK;
}

HRESULT CPictureClient::DeviceRemove(
        ISideShowCapabilities* /*pIDevice*/
        )
{
    if (m_pContentMgr != NULL)
    {
        ISideShowCapabilitiesCollection* pCapabilitiesCollection = NULL;
        HRESULT hr = m_pContentMgr->GetDeviceCapabilities(&pCapabilitiesCollection);
        if (SUCCEEDED(hr))
        {
            if (pCapabilitiesCollection != NULL)
            {
                pCapabilitiesCollection->AddRef();
                
                //
                // If the count of devices == 0, then we can take down the
                // Gadget and let the SideShow lifetime manager bring it
                // back to life when a device is added.
                //
                DWORD cDevices = 0;
                hr = pCapabilitiesCollection->GetCount(&cDevices);
                if (SUCCEEDED(hr) && cDevices == 0)
                {
                    ::PostThreadMessage(g_threadID, 
                                        WM_QUIT, 
                                        NULL, 
                                        NULL);
                }
                
                pCapabilitiesCollection->Release();
            }
        }
    }
    
    return S_OK;
}

HRESULT CPictureClient::ContentMissing(
        const CONTENT_ID contentId,
        ISideShowContent** ppIContent
        )
{
    HRESULT hr = E_FAIL;

    if (NULL != m_pContent)
    {
        ISideShowContent* pIContent = NULL;

        //
        // Get the content for this content ID.
        //
        ::EnterCriticalSection(&m_cs);

        pIContent = m_pContent->GetContent(contentId);

        ::LeaveCriticalSection(&m_cs);

        if (NULL != pIContent)
        {
            //
            // QueryInterface to return an AddRef'ed pointer to the platform.
            // The platform will call Release automatically.
            //
            hr = pIContent->QueryInterface(IID_PPV_ARGS(ppIContent));
        }
    }
    return hr;
}

void CPictureClient::RemoveAllContent()
{
    ::EnterCriticalSection(&m_cs);

    CBaseClient::RemoveAllContent();
    delete m_pContent;
    m_pContent = NULL;

    ::LeaveCriticalSection(&m_cs);
}

DWORD WINAPI CPictureClient::DirectoryThreadProc(LPVOID ThreadParameter)
{
    WCHAR wszPicPath[MAX_PATH];
    BOOL retVal = SHGetSpecialFolderPath(
            NULL,
            wszPicPath,
            CSIDL_MYPICTURES,
            0);
            
    if (TRUE == retVal)
    {
        CPictureClient* pThis = (CPictureClient*)ThreadParameter;
        HANDLE dirChange = FindFirstChangeNotification(
                      wszPicPath,
                      FALSE,
                      FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE);
    
        while (INVALID_HANDLE_VALUE != dirChange)
        {
            WaitForSingleObject(dirChange, INFINITE);
            pThis->RemoveAllContent();
            pThis->AddContent();
            
            if (FALSE == FindNextChangeNotification(dirChange))
            {
                break;
            }
        }
    }
    
    return 0;
}
