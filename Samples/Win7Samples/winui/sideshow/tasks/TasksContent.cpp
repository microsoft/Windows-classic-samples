// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"

#include "TasksContent.h"
#include "XmlDocument.h"

using namespace std;

CTasksContent::CTasksContent() :
    m_cTasks(0)
{
    m_contentID = CONTENT_ID_HOME;
    LoadTasks();
}

CTasksContent::~CTasksContent()
{
    for (int i = 0; i < m_cTasks; i++)
    {
        delete m_tasks[i];
        m_tasks[i] = NULL;
    }
}

CTask* CTasksContent::BuildTask(CXmlNode* pNode, int id)
{
    CTask* pTask = NULL;

    if (NULL != pNode)
    {
        BSTR name = NULL;
        BSTR details = NULL;
        BSTR category = NULL;
        BSTR dueDate = NULL;

        CXmlNodeList* pProperties = pNode->GetChildNodes();
        if (NULL != pProperties)
        {
            long cProperties = pProperties->Count();

            for (long iProp = 0; iProp < cProperties; iProp++)
            {
                CXmlNode* pProperty = pProperties->GetNode(iProp);
                if (NULL != pProperty)
                {
                    BSTR    bstrName;
                    BSTR    bstrValue;

                    //
                    // Get the name/value out of the node
                    //
                    bstrName = pProperty->GetName();
                    bstrValue = pProperty->GetInnerText();

                    //
                    // Compare to known properties
                    //
                    if (0 == wcscmp(bstrName, L"Name"))
                    {
                        name = bstrValue;
                    }
                    else if (0 == wcscmp(bstrName, L"Details"))
                    {
                        details = bstrValue;
                    }
                    else if (0 == wcscmp(bstrName, L"Category"))
                    {
                        category = bstrValue;
                    }
                    else if (0 == wcscmp(bstrName, L"DueDate"))
                    {
                        dueDate = bstrValue;
                    }
                    else
                    {                        
                        //
                        // We haven't saved the string off, so free it
                        //
                        ::SysFreeString(bstrValue);
                    }

                    ::SysFreeString(bstrName);

                    delete pProperty;
                }
            }

            delete pProperties;
        }

        //
        // Now, create a new CTask object with the parsed data.
        // The CONTENT_ID is the node id + 2.
        //
        pTask = new CTask(id + 2, name, details, category, dueDate);

        ::SysFreeString(name);

        ::SysFreeString(details);

        ::SysFreeString(category);

        ::SysFreeString(dueDate);
    }

    return pTask;
}

void CTasksContent::LoadTasks()
{
    CXmlDocument xDoc;
    BOOL fResult = FALSE;
    
    if (0 != m_cTasks)
    {
        return;
    }

    //
    // Read task settings (work or/and family tasks)
    //
    WCHAR wszEnabled[2]={0};

    GetPrivateProfileString(TASK_SECTION, SHOW_WORK_TASKS, L"0", wszEnabled, 2, (LPCWSTR)GetConfigFile());
    BOOL fShowWorkTasks = (0 != wcscmp(wszEnabled, L"0"));

    GetPrivateProfileString(TASK_SECTION, SHOW_FAMILY_TASKS, L"0", wszEnabled, 2, (LPCWSTR)GetConfigFile());
    BOOL fShowFamilyTasks = (0 != wcscmp(wszEnabled, L"0"));

    //
    // Load the tasks from an XML file in the current directory
    //
    fResult = xDoc.Load(GetTaskFile().GetBuffer());

    if (fResult)
    {
        CXmlNodeList* pChildren = xDoc.GetChildNodes();

        int cTasks = pChildren->Count();

        for (int i = 0; i < cTasks && i < MAX_TASKS; i++)
        {
            CXmlNode* pNode = pChildren->GetNode(i);
            if (NULL != pNode)
            {
                //
                // Create a CTask object from the XML
                //
                CTask* pTask = BuildTask(pNode, m_cTasks);

                if (NULL != pTask)
                {                
                    if (((0 == wcscmp(pTask->GetCategory(), L"Work")) && (TRUE == fShowWorkTasks)) ||
                        ((0 == wcscmp(pTask->GetCategory(), L"Family")) && (TRUE == fShowFamilyTasks)))
                    {
                        //
                        // Add the task to an array
                        //
                        m_tasks[m_cTasks] = pTask;
                        m_cTasks++;
                    }
                    else
                    {
                        delete pTask;
                    }
                }

                delete pNode;
            }
        }

        if (m_cTasks > 0)
        {
            //
            // Set the last task to roll around to the menu
            //
            m_tasks[m_cTasks-1]->SetNextID(1);
        }

        delete pChildren;
    }
}

LPSTR CTasksContent::BuildMenu()
{
    LPSTR pszXml = NULL;

    CXmlDocument xmlDoc;

    CXmlElement* pBody = xmlDoc.AddElement(L"body");
    CXmlElement* pMenu = pBody->AddElement(L"menu");
    pMenu->AddAttribute(L"id", CONTENT_ID_HOME);
    pMenu->AddAttribute(L"title", L"Task List");

    for (int nTask = 0; nTask < m_cTasks; nTask++)
    {
        CXmlElement* pItem = pMenu->AddElement(L"item");

        pItem->AddAttribute(L"target", m_tasks[nTask]->GetID());

        pItem->AddText(m_tasks[nTask]->GetName());
        
        delete pItem;
    }

    BSTR bstrXml;
    xmlDoc.GetXml(&bstrXml);
    printf("Sending Content: \n%ws\n\n", bstrXml);

    pszXml = AllocTaskUtf8String(bstrXml);

    ::SysFreeString(bstrXml);
    delete pBody;
    delete pMenu;

    return pszXml;
}

void CTasksContent::LoadContent(DWORD* pdwSize, BYTE** ppbData)
{
    if (NULL == pdwSize ||
        NULL == ppbData)
    {
        return;
    }

    //
    // Return size for the string, including the terminating NULL
    //
    LPSTR pszContent = BuildMenu();
    *ppbData = (BYTE*)pszContent;
    *pdwSize = (DWORD)strlen(pszContent) + 1;
}

void CTasksContent::FreeContent(BYTE** ppbData)
{
    //
    // Free the memory allocated in LoadContent (actually BuildMenu())
    //
    if (NULL != ppbData)
    {
        ::CoTaskMemFree(*ppbData);
    }
}


ISideShowContent* CTasksContent::GetTask(CONTENT_ID id)
{
    //
    // The array index is (CONTENT_ID - 2), so check the bounds
    //
    if (id < 2 || m_cTasks <= 0 || id-2 >= (CONTENT_ID)m_cTasks)
    {       
        return NULL;
    }

    return m_tasks[id-2];
}

int CTasksContent::Count()
{
    return m_cTasks;
}