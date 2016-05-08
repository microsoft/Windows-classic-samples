// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "GlanceContent.h"
#include "TasksContent.h"

CGlanceContent::CGlanceContent(CTasksContent* pContent)
{
    m_pContent = pContent;
    m_contentID = CONTENT_ID_GLANCE;
}

CGlanceContent::~CGlanceContent()
{
}

void CGlanceContent::LoadContent(DWORD* pdwSize, BYTE** ppbData)
{
    if (NULL == pdwSize ||
        NULL == ppbData ||
        NULL == m_pContent)
    {
        return;
    }

    char szGlance[32] = "Tasks";
    int cTasks = m_pContent->Count();

    HRESULT hr = StringCchPrintfA(szGlance, sizeof(szGlance)/sizeof(char), "%d Tasks", cTasks);
    if (FAILED(hr))
    {
        //
        // Handle the error
        //        
    }
    else
    {
        //
        // Allocate size for the string, including the terminating NULL
        //
        *pdwSize = (DWORD)strlen(szGlance) + 1;
        *ppbData = new BYTE[*pdwSize];

        hr = StringCchCopyA((char*)*ppbData, *pdwSize, szGlance);
        if (FAILED(hr))
        {
            //
            // Handle the error
            //
        }
    }   
}

void CGlanceContent::FreeContent(BYTE** ppbData)
{
    //
    // Free the memory allocated in LoadContent
    //
    if (NULL != ppbData)
    {
        delete [] *ppbData;
    }
}
