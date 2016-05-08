        // --------------------------------------------------------------------
        //
        //  Copyright (c) Microsoft Corporation.  All rights reserved
        //
        // THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
        // ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
        // THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
        // PARTICULAR PURPOSE.
        //
        // --------------------------------------------------------------------


        /* ************************************************************************ */
        /*                                                                          */
        /* Main file of the application MQ API test.                                */
        /*                                                                          */
        /* ************************************************************************ */

        //
        // MainFrm.cpp : implementation of the CMainFrame class
        //



        #include "stdafx.h"
        #include "MQApitst.h"
        #include <afxtempl.h>

        #include "MainFrm.h"
        #include "CrQDlg.h"
        #include "DelQDlg.h"
        #include "OpenQDlg.h"
        #include "ClosQDlg.h"
        #include "SendMDlg.h"
        #include "RecvMDlg.h"
        #include "RecWDlg.h"
        #include "LocatDlg.h"

        #ifdef _DEBUG
        #define new DEBUG_NEW
        #undef THIS_FILE
        static char THIS_FILE[] = __FILE__;
        #endif

        LPSTR UnicodeStringToAnsiString(LPCWSTR lpcsUnicode)
        {
            LPSTR lpAnsiString = NULL;

            if (lpcsUnicode)
            {
                                size_t dwSize=0;
                wcstombs_s(&dwSize, NULL, 0, lpcsUnicode, 0);
                lpAnsiString = new char[dwSize];
                                size_t nResultedStringSize=0;
                size_t rc = wcstombs_s(
                                               &nResultedStringSize,
                                               lpAnsiString,
                                               dwSize,
                                               lpcsUnicode,
                                               dwSize
                                               );
                if (rc != (size_t)(0) )
                {
                    AfxMessageBox(_T("An unsupported wide character was encountered. Exiting..."));
                    exit(1);
                }
            }

            return lpAnsiString;
        }

        void AnsiStringToUnicode(LPWSTR lpsUnicode, LPSTR  lpsAnsi, DWORD  nSize)
        {
            if (lpsUnicode == NULL)
            {
                return;
            }

            ASSERT(lpsAnsi != NULL);
                        size_t nResultedStringSize=0;
            size_t rc = mbstowcs_s(
                                        &nResultedStringSize,
                                        lpsUnicode,
                                        nSize,
                                        lpsAnsi,
                                        nSize);
            if (rc != (size_t)(0))
            {
                AfxMessageBox(_T("An unsupported wide character was encountered. Exiting..."));
                exit(1);
            }

        }

        #ifdef UNICODE
        #define _mqscpy(dest, src)  ASSERT(src != NULL); \
                                    wcsncpy_s(dest, BUFFERSIZE, src, _TRUNCATE);

        #else
        #define _mqscpy(dest, src)  ASSERT(src != NULL); \
                                    AnsiStringToUnicode(dest, src, _tcslen(src)+1);

        #endif

        BOOL GetTextualSid(
            PSID pSid,          // Binary SID
            LPTSTR TextualSID,  // Buffer for textual representation of an SID
            LPDWORD dwBufferLen // Required/provided size of the textual SID buffer
            )
        {
            PSID_IDENTIFIER_AUTHORITY psia;
            DWORD dwSubAuthorities;
            DWORD dwSidRev = SID_REVISION;
            DWORD dwCounter;
            DWORD dwSidSize;

            // Obtain the identifier-authority value in the SID.
            psia = &((SID *)pSid)->IdentifierAuthority;

            // Obtain the number of subauthorities in the SID.
            dwSubAuthorities = (DWORD)((SID *)pSid)->SubAuthorityCount;

            //
            // Compute the buffer length.
            // S-SID_REVISION + identifier authority + subauthorities + NULL
            //
            dwSidSize = (15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

            //
            // Check the buffer length provided.
            // If it is not large enough, indicate the proper size and call SetLastError.
            //
            if (*dwBufferLen < dwSidSize)
            {
                *dwBufferLen = dwSidSize;
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return FALSE;
            }

            //
            // Obtain a textual representation of S-SID_REVISION.
            //
            TextualSID += _stprintf_s(TextualSID, *dwBufferLen, TEXT("S-%lu-"), dwSidRev );

            //
            // Obtain a textual representation of the identifier-authority value.
            //
            if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
            {
                TextualSID += _stprintf_s(TextualSID,
                                                          *dwBufferLen,
                              TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                              (USHORT)psia->Value[0],
                              (USHORT)psia->Value[1],
                              (USHORT)psia->Value[2],
                              (USHORT)psia->Value[3],
                                  (USHORT)psia->Value[4],
                              (USHORT)psia->Value[5]);
            }
            else
            {
                TextualSID += _stprintf_s(TextualSID,
                                                          *dwBufferLen,
                                                          TEXT("%lu"),
                                  (ULONG)(psia->Value[5]      )   +
                              (ULONG)(psia->Value[4] <<  8)   +
                              (ULONG)(psia->Value[3] << 16)   +
                              (ULONG)(psia->Value[2] << 24)   );
            }

            //
            // Loop through the subauthorities.
            //
            for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
            {
                TextualSID += _stprintf_s(TextualSID, *dwBufferLen, TEXT("-%lu"),
                            ((SID *)pSid)->SubAuthority[ dwCounter] );
            }

            return TRUE;
        }

        /////////////////////////////////////////////////////////////////////////////
        // CMainFrame

        IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

        BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
           //{{AFX_MSG_MAP(CMainFrame)
           ON_WM_CREATE()
           ON_COMMAND(ID_API_CREATE_QUEUE, OnApiCreateQueue)
           ON_COMMAND(ID_API_DELETE_QUEUE, OnApiDeleteQueue)
           ON_COMMAND(ID_API_OPEN_QUEUE, OnApiOpenQueue)
           ON_COMMAND(ID_API_CLOSE_QUEUE, OnApiCloseQueue)
           ON_COMMAND(ID_API_SEND_MESSAGE, OnApiSendMessage)
           ON_COMMAND(ID_API_RECEIVE_MESSAGE, OnApiReceiveMessage)
           ON_COMMAND(ID_API_LOCATE, OnApiLocate)
           //}}AFX_MSG_MAP
        END_MESSAGE_MAP()

        static UINT indicators[] =
        {
           ID_SEPARATOR,           // status line indicator
           ID_INDICATOR_CAPS,
           ID_INDICATOR_NUM,
           ID_INDICATOR_SCRL,
        };

        /////////////////////////////////////////////////////////////////////////////
        // CMainFrame construction/destruction

        CMainFrame::CMainFrame()
        {
           // TODO: Add member initialization code here.
        }

        CMainFrame::~CMainFrame()
        {
        }

        int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
        {
            if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
                return -1;

            if (!m_wndToolBar.Create(this) ||
                !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
            {
                TRACE0("The attempt to load the toolbar failed.\n");
                return -1;
            }

            if (!m_wndStatusBar.Create(this) ||
                !m_wndStatusBar.SetIndicators(indicators,
                sizeof(indicators)/sizeof(UINT)))
            {
                TRACE0("The attempt to create the toolbar or set its indicators failed.\n");
                return -1;
            }

            // TODO: Remove this if you don't want tool tips or a resizeable toolbar.
            m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
                CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

            // TODO: Delete these three lines if you don't want the toolbar to
            //  be dockable.
            m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
            EnableDocking(CBRS_ALIGN_ANY);
            DockControlBar(&m_wndToolBar);

            return 0;
        }

        BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
        {
            // TODO: Modify the Window class or styles here by modifying
            //  the CREATESTRUCT cs.

            return CFrameWnd::PreCreateWindow(cs);
        }

        /////////////////////////////////////////////////////////////////////////////
        // CMainFrame diagnostics

        #ifdef _DEBUG
        void CMainFrame::AssertValid() const
        {
            CFrameWnd::AssertValid();
        }

        void CMainFrame::Dump(CDumpContext& dc) const
        {
            CFrameWnd::Dump(dc);
        }

        #endif //_DEBUG

        /////////////////////////////////////////////////////////////////////////////
        // CMainFrame message handlers

        #define MAXINDEX 31


        /* ************************************************************************ */
        /*                        RemoveFromPathNameArray                           */
        /* ************************************************************************ */
        /* This function goes through the path name array and compares a given      */
        /* path name to the path names of the items in the array.                   */
        /* If a match is found, the item is removed from the array and the function */
        /* returns a pointer to the item; otherwise, a NULL pointer is returned.    */
        /* ************************************************************************ */
        ARRAYQ* CMainFrame::RemoveFromPathNameArray(TCHAR *szFormatPathName)
        {
            int Index;
            int MaxIndex = m_PathNameArray.GetSize();
            ARRAYQ* pQueue;

            //
            // Loop through the path name array.
            //
            for (Index=0; Index<MaxIndex; Index++)
            {
                if (m_PathNameArray[Index]->bFormatNameOnly==TRUE)
                {
                    if (_tcscmp(szFormatPathName, m_PathNameArray[Index]->szFormatName) == 0)
                    {
                        //
                        // A match was found.
                        //
                        pQueue = m_PathNameArray[Index];
                        m_PathNameArray.RemoveAt(Index);
                        return pQueue;
                    }
                }
                else
                {
                    if (_tcscmp(szFormatPathName, m_PathNameArray[Index]->szPathName) == 0)
                    {
                        //
                        // A match was found.
                        //
                        pQueue = m_PathNameArray[Index];
                        m_PathNameArray.RemoveAt(Index);
                        return pQueue;
                    }
                }
            }
            return NULL; // No match was found.
        }

        /* ************************************************************************ */
        /*                             CleanPathNameArray                           */
        /* ************************************************************************ */
        /* This function goes through the path name array and deletes all the       */
        /* items in it. The function frees the allocated memory.                    */
        /* ************************************************************************ */
        void CMainFrame::CleanPathNameArray()
        {
            ARRAYQ* pQueue;

            while (m_PathNameArray.GetSize() > 0)
            {
                pQueue = m_PathNameArray[0];
                m_PathNameArray.RemoveAt(0);
                delete pQueue;
            }
        }

        /* ************************************************************************ */
        /*                        RemoveFromOpenedQueuePathNameArray                */
        /* ************************************************************************ */
        /* This function goes through the open queue path name array and compares   */
        /* a given path name to the path names of the items in the array.           */
        /* If a match is found, the item is removed from the array and the function */
        /* returns a pointer to the item; otherwise, a NULL pointer is returned.    */
        /* ************************************************************************ */
        ARRAYQ* CMainFrame::RemoveFromOpenedQueuePathNameArray(TCHAR *szFormatPathName)
        {
            int Index;
            int MaxIndex = m_OpenedQueuePathNameArray.GetSize();
            ARRAYQ* pQueue;

            //
            // Loop through the open queue path name array.
            //
            for (Index=0; Index<MaxIndex; Index++)
            {
                if (m_OpenedQueuePathNameArray[Index]->bFormatNameOnly==TRUE)
                {
                    if (_tcscmp(szFormatPathName, m_OpenedQueuePathNameArray[Index]->szFormatName) == 0)
                    {
                        pQueue = m_OpenedQueuePathNameArray[Index];
                        m_OpenedQueuePathNameArray.RemoveAt(Index);
                        return pQueue;
                    }
                }
                else
                {
                    if (_tcscmp(szFormatPathName, m_OpenedQueuePathNameArray[Index]->szPathName) == 0)
                    {
                        //
                        // A match was found.
                        //
                        pQueue = m_OpenedQueuePathNameArray[Index];
                        m_OpenedQueuePathNameArray.RemoveAt(Index);
                        return pQueue;
                    }
                }
            }
            return NULL; // No match was found.
        }

        /* ************************************************************************ */
        /*                           IsOpenedQueueArrayEmpty                        */
        /* ************************************************************************ */
        /* This function checks whether the size of the open queue path name array  */
        /* is equal to or less than zero and, if so, it returns TRUE; otherwise,    */
        /* it returns FALSE.                                                        */
        /* ************************************************************************ */
        BOOL CMainFrame::IsOpenedQueueArrayEmpty()
        {
            if (m_OpenedQueuePathNameArray.GetSize() <= 0)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }

        /* ************************************************************************ */
        /*                        MoveToOpenedQueuePathNameArray                    */
        /* ************************************************************************ */
        /* This function moves an item from the path name array to the open queue   */
        /* path name array. It also it updates the handle and the access rights to  */
        /* the moved queue.                                                         */
        /* ************************************************************************ */
        void CMainFrame::MoveToOpenedQueuePathNameArray(TCHAR *szPathName,TCHAR *szFormatName,
                                                        QUEUEHANDLE hQueue, DWORD dwAccess)
        {
            ARRAYQ* pQueue=NULL;
            //check whether pQueue is listed in the Located Queue.
            pQueue = RemoveFromPathNameArray(szPathName);
            //if not found in located Q:Allocated new ARRAYQ.
            if (pQueue==NULL)
            {
                pQueue=new ARRAYQ;
                if (pQueue==NULL)
                {
                    AfxMessageBox(_T("Could not Allocate memory in function CMainFrame::MoveToOpenedQueuePathNameArray. Exiting"));
                    exit (1);
                }
                _tcsncpy_s(
                                     pQueue->szFormatName,
                                     sizeof(pQueue->szFormatName)/sizeof(pQueue->szFormatName[0]),
                                     szFormatName,
                                     sizeof(pQueue->szFormatName)/sizeof(pQueue->szFormatName[0])-1
                                     );
                _tcsncpy_s(
                                     pQueue->szPathName,
                                     sizeof(pQueue->szPathName)/sizeof(pQueue->szPathName[0]),
                                     szPathName,
                                     sizeof(pQueue->szPathName)/sizeof(pQueue->szPathName[0])-1
                                     );
                if (_tcscmp(pQueue->szPathName,_T(""))==0)
                {
                    pQueue->bFormatNameOnly=true;
                }
            }
            pQueue->hHandle = hQueue;                     // add Queue Handle.
            pQueue->dwAccess = dwAccess;                  // add Queue Access rights.

            Add2OpenedQueuePathNameArray(pQueue);
        }

        /* ************************************************************************ */
        /*                              MoveToPathNameArray                         */
        /* ************************************************************************ */
        /* This function moves an item from the open queue path name array to the   */
        /* path name array.                                                         */
        /* ************************************************************************ */
        void CMainFrame::MoveToPathNameArray(TCHAR *pszPathName)
        {
            ARRAYQ* pQueue;

            pQueue = RemoveFromOpenedQueuePathNameArray(pszPathName);
            Add2PathNameArray(pQueue);
        }

        /* ************************************************************************ */
        /*                             UpdatePathNameArrays                         */
        /* ************************************************************************ */
        /* This function goes through the open queue path name array, and for every */
        /* item in it, it checks whether the item is found in the path name array   */
        /* as well. If so, the function removes the item from the path name array.  */
        /* ************************************************************************ */
        void CMainFrame::UpdatePathNameArrays()
        {
            int PathNameIndex;
            int OpenedPathNameIndex;
            int MaxOpenedPathNameIndex = m_OpenedQueuePathNameArray.GetSize();
            ARRAYQ* pQueue;

            //
            // Loop through the open queue path name array.
            //
            for (OpenedPathNameIndex=0; OpenedPathNameIndex<MaxOpenedPathNameIndex; OpenedPathNameIndex++)
            {
                for (PathNameIndex=0; PathNameIndex<m_PathNameArray.GetSize(); PathNameIndex++)
                {
                    if (_tcscmp(m_OpenedQueuePathNameArray[OpenedPathNameIndex]->szPathName,
                        m_PathNameArray[PathNameIndex]->szPathName) == 0)
                    {
                        //
                        // A match was found, so remove it from path name array.
                        //
                        pQueue = m_PathNameArray[PathNameIndex];
                        m_PathNameArray.RemoveAt(PathNameIndex);
                        delete pQueue;
                        //
                        // Get out of the inner for loop.
                        //
                        break;
                    }
                }
            }
        }


        /* ************************************************************************ */
        /*                              GetQueueHandle                              */
        /* ************************************************************************ */
        /* This function goes through the open queue path name array and retrieves  */
        /* the handle to the queue which matches a given path name. If no mathch    */
        /* is found, the function returns FALSE.                                    */
        /* ************************************************************************ */
        BOOL CMainFrame::GetQueueHandle(TCHAR *szPathFormatName,QUEUEHANDLE* phClosedQueueHandle)
        {
            int Index;
            int MaxIndex = m_OpenedQueuePathNameArray.GetSize();
            ARRAYQ* pQueue;

            //
            // Loop through the open queue path name array.
            //
            for (Index = 0; Index<MaxIndex; Index++)
            {
                if (m_OpenedQueuePathNameArray[Index]->bFormatNameOnly==TRUE)
                {
                    if (_tcscmp(szPathFormatName, m_OpenedQueuePathNameArray[Index]->szFormatName)==0)
                    {
                        //
                        // A match was found.
                        //
                        pQueue = m_OpenedQueuePathNameArray[Index];
                        *phClosedQueueHandle = pQueue->hHandle;
                        return TRUE;
                    }
                }
                else
                {

                    if (_tcscmp(szPathFormatName, m_OpenedQueuePathNameArray[Index]->szPathName)==0)
                    {
                        //
                        // A match was found.
                        //
                        pQueue = m_OpenedQueuePathNameArray[Index];
                        *phClosedQueueHandle = pQueue->hHandle;
                        return TRUE;
                    }
                }
            }
            return FALSE; // No match was found.
        }

        /* ************************************************************************ */
        /*                       TranslatePathNameToFormatName                      */
        /* ************************************************************************ */
        /* This function goes retrieves the Format Name using the MSMQ API          */
        /* MQPathNameToFormatName. if no format name is found, the function checks  */
        /* whether format name was given.                                           */
        /* returns TRUE is succeeded or FALSE if not                                */
        /* ************************************************************************ */
        BOOL CMainFrame::TranslatePathNameToFormatName(TCHAR *szPathName, size_t PathNameSize, TCHAR *szFormatName, size_t FormatNameSize)
        {
            //
            //check valid parameters
            //
            if (szPathName==NULL)
            {
                return FALSE;
            }
            DWORD dwFormatNameSize=FormatNameSize;
            HRESULT hr;

#ifdef UNICODE
            hr = MQPathNameToFormatName(szPathName,szFormatName,&dwFormatNameSize);
#else
            WCHAR szwPathName[MAX_Q_FORMATNAME_LEN+1]={0};
            WCHAR szwFormatName[MAX_Q_FORMATNAME_LEN+1]={0};
                        size_t nResultedStringSize=0;
            size_t rc = mbstowcs_s(
                                      &nResultedStringSize,
                                      szwPathName,
                                      sizeof(szwPathName)/sizeof(szwPathName[0]),
                                      szPathName,
                                      PathNameSize);
            if (rc != (size_t)(0))
            {
                AfxMessageBox(_T("An unsupported wide character was encountered. Exiting..."));
                exit(1);
            }


                        dwFormatNameSize = sizeof(szwFormatName)/sizeof(szwFormatName[0]);
            hr = MQPathNameToFormatName(szwPathName,szwFormatName,&dwFormatNameSize);
                        nResultedStringSize = 0;
            rc = wcstombs_s(
                                 &nResultedStringSize,
                                 szFormatName,
                                 FormatNameSize,
                                 szwFormatName,
                                 dwFormatNameSize
                                 );
#endif
            if (FAILED(hr))
            {
                if (IsFormatName(szPathName)==TRUE)
                {
                    _tcsncpy_s(
                                             szFormatName,
                                             FormatNameSize,
                                             szPathName,
                                             PathNameSize);
                    return TRUE;

                }
                return FALSE;
            }
            return TRUE;

        }


        /* ************************************************************************ */
        /*                         DisplayPathNameArray                             */
        /* ************************************************************************ */
        /* This function goes through the path name array and prints it to sreeen.  */
        /* ************************************************************************ */
        void CMainFrame::DisplayPathNameArray()
        {
            int Index;
            int MaxIndex = m_PathNameArray.GetSize();
            TCHAR szMsgBuffer[BUFFERSIZE];

            _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("   Located Queues :"));
            PrintToScreen(szMsgBuffer);
            //
            // Loop through the path name array.
            //
            for (Index=0; Index<MaxIndex; Index++)
            {
                //
                // Print the path names or format names.
                //

                if (m_PathNameArray[Index]->bFormatNameOnly==TRUE)
                {
                    _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("\t%d. %s"),Index+1, m_PathNameArray[Index]->szFormatName);
                    PrintToScreen(szMsgBuffer);

                }
                else
                {
                    _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("\t%d. %s"),Index+1, m_PathNameArray[Index]->szPathName);
                    PrintToScreen(szMsgBuffer);
                }
            }
        }

        /* ************************************************************************ */
        /*                    DisplayOpenedQueuePathNameArray                       */
        /* ************************************************************************ */
        /* This function goes through the open queue path name array and prints it  */
        /* to screen.                                                               */
        /* ************************************************************************ */
        void CMainFrame::DisplayOpenedQueuePathNameArray()
        {
            int Index;
            int MaxIndex = m_OpenedQueuePathNameArray.GetSize();
            TCHAR szMsgBuffer[BUFFERSIZE];

            _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("   Currently Opened Queues :"));
            PrintToScreen(szMsgBuffer);
            //
            // Loop through the open queue path name array.
            //
            for (Index=0; Index<MaxIndex; Index++)
            {
                //
                // Print the path names or format names.
                //

                if (m_OpenedQueuePathNameArray[Index]->bFormatNameOnly==TRUE)
                {
                    _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("\t%d. %s"),Index+1, m_OpenedQueuePathNameArray[Index]->szFormatName);
                    PrintToScreen(szMsgBuffer);

                }
                else
                {

                    _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("\t%d. %s"),Index+1, m_OpenedQueuePathNameArray[Index]->szPathName);
                    PrintToScreen(szMsgBuffer);
                }
            }
        }

        /* ************************************************************************ */
        /*                          GetMsgClassStatus                               */
        /* ************************************************************************ */
        /* This function sets the proper status string based on a given MQMSG class.*/
        /* ************************************************************************ */

        struct
        {
            unsigned short  mclass;
            LPTSTR          pszDescription;
        } StringClass[] =
          {
            { MQMSG_CLASS_NORMAL, TEXT("The Message was received successfully.")},
            { MQMSG_CLASS_ACK_REACH_QUEUE, TEXT("The REACH QUEUE ACK Message was read successfully.")},
            { MQMSG_CLASS_ACK_RECEIVE, TEXT("The RECEIVE ACK Message was read successfully.")},
            { MQMSG_CLASS_NACK_BAD_DST_Q, TEXT("The DESTINATION QUEUE HANDLE INVALID Nack Message was read successfully.")},
            { MQMSG_CLASS_NACK_RECEIVE_TIMEOUT, TEXT("The TIME TO RECEIVE EXPIRED Nack Message was read successfully.")},
            { MQMSG_CLASS_NACK_REACH_QUEUE_TIMEOUT, TEXT("The TIME TO REACH QUEUE EXPIRED Nack Message was read successfully.")},
            { MQMSG_CLASS_NACK_Q_EXCEED_QUOTA, TEXT("The QUEUE IS FULL Nack Message was read successfully.")},
            { MQMSG_CLASS_NACK_ACCESS_DENIED, TEXT("The SENDER HAVE NO SEND ACCESS RIGHTS ON QUEUE Nack Message was read successfully.")},
            { MQMSG_CLASS_NACK_HOP_COUNT_EXCEEDED, TEXT("The HOP COUNT EXCEEDED Nack Message was read successfully.")},
            { MQMSG_CLASS_NACK_BAD_SIGNATURE, TEXT("The MESSAGE RECEIVED WITH BAD SIGNATURE Nack Message was read successfully.")},
            { MQMSG_CLASS_NACK_BAD_ENCRYPTION, TEXT("The MSG COULD NOT DECRYPTED Nack Message was read successfully.")},
            { MQMSG_CLASS_NACK_COULD_NOT_ENCRYPT, TEXT("The SOURCE QM COULD NOT ENCRYPT MSG FOR DEST QM Nack Message was read successfully.")},
            { 0, NULL}
          };

        void CMainFrame::ClassToString(unsigned short MsgClass,LPTSTR pszStatus, size_t sizeinChars)
        {
            //
            // Loop through the StringClass array to find MsgClass.
            //
            ASSERT(pszStatus != NULL);

            DWORD dwIndex = 0;
            while (StringClass[dwIndex].pszDescription != NULL)
            {
                if (StringClass[dwIndex].mclass == MsgClass)
                {
                    _tcsncpy_s(pszStatus, sizeinChars, StringClass[dwIndex].pszDescription, sizeinChars -1);
                    return;
                }
                dwIndex++;
            }

            //
            // MsgClass was not found, so print the default error.
            //
            _stprintf_s(pszStatus, sizeinChars, TEXT("The NACK (0x%X) message was read successfully."), MsgClass);
        }


        /* ************************************************************************ */
        /*                            OnApiCreateQueue                              */
        /* ************************************************************************ */
        /* This function opens a dialog box and asks the user for the queue's       */
        /* path name and label. Then it creates the specified queue.                */
        /*                                                                          */
        /* Uses: MQCreateQueue.                                                     */
        /* ************************************************************************ */
        void CMainFrame::OnApiCreateQueue()
        {
            // TODO: Add your command handler code here.

            TCHAR szMsgBuffer[BUFFERSIZE];

            MQQUEUEPROPS QueueProps;
            MQPROPVARIANT aVariant[MAXINDEX];
            QUEUEPROPID aPropId[MAXINDEX];
            DWORD PropIdCount = 0;
            HRESULT hr;

            PSECURITY_DESCRIPTOR pSecurityDescriptor;

            TCHAR szPathNameBuffer[MAX_Q_PATHNAME_LEN+1];
            TCHAR szFormatNameBuffer[MAX_Q_FORMATNAME_LEN+1];
            TCHAR szLabelBuffer[MAX_Q_PATHNAME_LEN+1];
            DWORD dwFormatNameBufferLength = sizeof(szFormatNameBuffer)/sizeof(szFormatNameBuffer[0]);


            //
            // Display the Create Queue dialog box.
            //
            CCreateQueueDialog CreateQueueDialog;

            if(CreateQueueDialog.DoModal() == IDCANCEL)
            {
                return;
            }
            CreateQueueDialog.GetPathName(szPathNameBuffer);
            CreateQueueDialog.GetLabel(szLabelBuffer);

            //
            // Get the input fields from the dialog box
            // and prepare the property array PROPVARIANT.
            //


            //
            // Set the PROPID_Q_PATHNAME property.
            //
            aPropId[PropIdCount] = PROPID_Q_PATHNAME;    // PropId
            aVariant[PropIdCount].vt = VT_LPWSTR;        // Type
            aVariant[PropIdCount].pwszVal = new WCHAR[MAX_Q_PATHNAME_LEN+1];
            _mqscpy(aVariant[PropIdCount].pwszVal, szPathNameBuffer); // Value

            PropIdCount++;

            //
            // Set the PROPID_Q_LABEL property.
            //
            aPropId[PropIdCount] = PROPID_Q_LABEL;    // PropId
            aVariant[PropIdCount].vt = VT_LPWSTR;     // Type
            aVariant[PropIdCount].pwszVal = new WCHAR[MAX_Q_PATHNAME_LEN+1];
            _mqscpy(aVariant[PropIdCount].pwszVal, szLabelBuffer); // Value

            PropIdCount++;


            //
            // Initialize the MQEUEUPROPS structure.
            //
            QueueProps.cProp = PropIdCount;           //Number of properties
            QueueProps.aPropID = aPropId;             //Identifiers of properties
            QueueProps.aPropVar = aVariant;           // Value of properties
            QueueProps.aStatus = NULL;                //No error reports

            //
            // No security is defined (default).
            //
            pSecurityDescriptor = NULL;

            //
            // Create the queue.
            //
        #ifdef UNICODE
            hr = MQCreateQueue(
                    pSecurityDescriptor,            //Security
                    &QueueProps,                    //Queue properties
                    szFormatNameBuffer,             //Output: Format name
                    &dwFormatNameBufferLength       //Output: Format name length
                    );
        #else
            WCHAR szwFormatNameBuffer[MAX_Q_FORMATNAME_LEN+1];
                        dwFormatNameBufferLength = sizeof(szwFormatNameBuffer)/sizeof(szwFormatNameBuffer[0]);
            hr = MQCreateQueue(
                    pSecurityDescriptor,            //Security
                    &QueueProps,                    //Queue properties
                    szwFormatNameBuffer,            //Output: Format name
                    &dwFormatNameBufferLength       //Output: Format name length
                    );

            if (SUCCEEDED(hr))
            {
                                size_t nResultedStringSize=0;
                size_t rc =wcstombs_s(
                                              &nResultedStringSize,
                                              szFormatNameBuffer,
                          sizeof(szFormatNameBuffer)/sizeof(szFormatNameBuffer[0]),
                                              szwFormatNameBuffer,
                                              dwFormatNameBufferLength
                                              );
                if (rc != (size_t)(0))
                {
                    AfxMessageBox("An unsupported wide character was encountered. Exiting...");
                    exit(1);
                }
            }
        #endif

            if (FAILED(hr))
            {
                //
                // An error occurred, so display a message.
                //
                _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("MQCreateQueue failed. Error code: 0x%x."),hr);
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
            }
            else
            {
                //
                // Success was achieved, so write to the edit control.
                //
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("The queue %s was created successfully. ( FormatName: %s )"),
                    szPathNameBuffer, szFormatNameBuffer);
                PrintToScreen(szMsgBuffer);

                //
                // Add the new queue to the path name array.
                //
                ARRAYQ* pNewQueue = new ARRAYQ;
                if (pNewQueue == NULL)
                    return;

                //
                // Save the path name and the format name in the ARRAYQ structure.
                //
                _tcsncpy_s (
                                    pNewQueue->szPathName,
                                    sizeof(pNewQueue->szPathName)/sizeof(pNewQueue->szPathName[0]),
                                    szPathNameBuffer,
                                    sizeof(pNewQueue->szPathName)/sizeof(pNewQueue->szPathName[0])-1
                                    );
                _tcsncpy_s (
                                    pNewQueue->szFormatName,
                                    sizeof(pNewQueue->szFormatName)/sizeof(pNewQueue->szFormatName[0]),
                                    szFormatNameBuffer,
                                    sizeof(pNewQueue->szFormatName)/sizeof(pNewQueue->szFormatName[0])-1
                                    );

                Add2PathNameArray(pNewQueue);

            }

            //
            // Free the allocated memory.
            //
            delete   aVariant[0].pwszVal;
            delete   aVariant[1].pwszVal;
        }

        /* ************************************************************************ */
        /*                            OnApiDeleteQueue                              */
        /* ************************************************************************ */
        /* This function opens a dialog box and asks the user for the queue's       */
        /* path name. Then it deletes the specified queue.                          */
        /*                                                                          */
        /* Uses: MQDeleteQueue.                                                     */
        /* ************************************************************************ */
        void CMainFrame::OnApiDeleteQueue()
        {
            // TODO: Add your command handler code here.
            TCHAR szPathNameBuffer[MAX_Q_PATHNAME_LEN+1];
            TCHAR szFormatNameBuffer[MAX_Q_PATHNAME_LEN+1];
            TCHAR szMsgBuffer[BUFFERSIZE];

            HRESULT hr;

            DWORD dwFormatNameBufferLength = sizeof(szFormatNameBuffer)/sizeof(szFormatNameBuffer[0]);

            CDeleteQueueDialog DeleteQueueDialog(&m_PathNameArray);

            //
            // Display the Delete Queue dialog box.
            //
            if (DeleteQueueDialog.DoModal() == IDCANCEL)
            {
                return;
            }

            DeleteQueueDialog.GetPathName(szPathNameBuffer);

            //
            // Translate the path name to a format name using the ARRAYQ array.
            //
            if (TranslatePathNameToFormatName(szPathNameBuffer, _tcslen(szPathNameBuffer), szFormatNameBuffer, sizeof(szFormatNameBuffer)/sizeof(szFormatNameBuffer[0]) ) == FALSE)
            {
                //
                // An error occurred, so display a message.
                //
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("The queue was not found."));
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
                return;
            }

            //
            // Delete the queue.
            //
        #ifdef UNICODE
            hr = MQDeleteQueue(szFormatNameBuffer);  // Format name of the queue to be deleted
        #else
            WCHAR szwFormatNameBuffer[MAX_Q_FORMATNAME_LEN+1];
                        size_t nResultedStringSize=0;
            size_t rc = mbstowcs_s(
                                        &nResultedStringSize,
                                        szwFormatNameBuffer,
                                        sizeof(szwFormatNameBuffer)/sizeof(szwFormatNameBuffer[0]),
                                        szFormatNameBuffer,
                                        _tcslen(szFormatNameBuffer));
            if (rc != (size_t)(0))
            {
                AfxMessageBox("An unsupported wide character was encountered. Exiting...");
                exit(1);
            }
            hr = MQDeleteQueue(szwFormatNameBuffer);  // Format name of the queue to be deleted
        #endif

            if (FAILED(hr))
            {
                //
                // An error occurred, so display a message.
                //
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("MQDeleteQueue failed. Error code: 0x%x."),hr);
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
            }
            else
            {
                //
                // Success was achieved, so write in the edit control.
                //
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("The queue %s was deleted successfully."), szPathNameBuffer);
                PrintToScreen(szMsgBuffer);
                //
                // Delete the name from the path name array.
                //
                ARRAYQ* DeletedQueue = RemoveFromPathNameArray(szPathNameBuffer);
                if (DeletedQueue != NULL)
                {
                    delete DeletedQueue;
                }
            }
        }

        /* ************************************************************************ */
        /*                             OnApiOpenQueue                               */
        /* ************************************************************************ */
        /* This function opens a dialog box and asks the user for the queue's       */
        /* path name. Then it opens the specified queue.                             */
        /*                                                                          */
        /* Uses: MQOpenQueue.                                                       */
        /* ************************************************************************ */
        void CMainFrame::OnApiOpenQueue()
        {
            // TODO: Add your command handler code here.
            TCHAR szPathNameBuffer[MAX_Q_PATHNAME_LEN+1];
            TCHAR szFormatNameBuffer[MAX_Q_PATHNAME_LEN+1];
            TCHAR szAccessBuffer[ACCESSBUFFERSIZE];
            TCHAR szMsgBuffer[BUFFERSIZE];
            WCHAR szwFormatNameBuffer[MAX_Q_FORMATNAME_LEN+1]={0};
            DWORD dwFormatNameSize=sizeof(szwFormatNameBuffer)/sizeof(szwFormatNameBuffer[0]);
            HRESULT hrFormatName;
            HRESULT hrPathName;

            DWORD dwFormatNameBufferLength = sizeof(szFormatNameBuffer)/sizeof(szFormatNameBuffer[0]);
            DWORD dwAccess;

            QUEUEHANDLE hQueue;

            COpenQueueDialog OpenQueueDialog(&m_PathNameArray);

            //
            // Display the Open Queue dialog box.
            //
            if (OpenQueueDialog.DoModal() == IDCANCEL)
            {
                return;
            }

            OpenQueueDialog.GetPathName(szPathNameBuffer);
            OpenQueueDialog.GetFormatName( szFormatNameBuffer, sizeof(szFormatNameBuffer)/sizeof(szFormatNameBuffer[0]) );
            dwAccess = OpenQueueDialog.GetAccess();
            //
            // Set the access buffer string.
            //
            switch (dwAccess)
            {
            case (MQ_RECEIVE_ACCESS | MQ_SEND_ACCESS):
                _tcsncpy_s(szAccessBuffer, sizeof(szAccessBuffer)/sizeof(szAccessBuffer[0]), TEXT("MQ_RECEIVE_ACCESS, MQ_SEND_ACCESS."), 34);
                break;

            case MQ_RECEIVE_ACCESS:

                _tcsncpy_s(szAccessBuffer, sizeof(szAccessBuffer)/sizeof(szAccessBuffer[0]), TEXT("MQ_RECEIVE_ACCESS."), 18);
                break;

            case MQ_SEND_ACCESS:

                _tcsncpy_s(szAccessBuffer, sizeof(szAccessBuffer)/sizeof(szAccessBuffer[0]), TEXT("MQ_SEND_ACCESS."), 15);
                break;

            default:

                _tcsncpy_s(szAccessBuffer, sizeof(szAccessBuffer)/sizeof(szAccessBuffer[0]), TEXT("NONE."), 5);
                break;
            }

            //Try Open The Q. using format name given.

#ifdef UNICODE
            hrFormatName = MQOpenQueue(szFormatNameBuffer,dwAccess,0,&hQueue);
#else
                        size_t nResultedStringSize=0;
            size_t rc = mbstowcs_s(
                                        &nResultedStringSize,
                                        szwFormatNameBuffer,
                                        sizeof(szwFormatNameBuffer)/sizeof(szwFormatNameBuffer[0]),
                                        szFormatNameBuffer,
                                        _tcslen(szFormatNameBuffer));
            if (rc != (size_t)(0))
            {
                AfxMessageBox("An unsupported wide character was encountered. Exiting...");
                exit(1);
            }

            hrFormatName = MQOpenQueue(
                    szwFormatNameBuffer,    // Format name of the queue to be opened.
                    dwAccess,               // Access rights to the queue.
                    0,                      // No exclusive receive.
                    &hQueue                 // OUT: handle to the opened queue.
                    );

#endif
            //
            //If failed to open Q. using format name,open The Q. using path name given.
            //
            if (FAILED(hrFormatName))
            {

#ifdef UNICODE
                //Traslate PathName to FormatName
                                dwFormatNameSize = sizeof(szwFormatNameBuffer)/sizeof(szwFormatNameBuffer[0]);
                MQPathNameToFormatName(szPathNameBuffer,szwFormatNameBuffer,&dwFormatNameSize);
                //Open The Q.
                hrPathName = MQOpenQueue(szwFormatNameBuffer,dwAccess,0,&hQueue);
#else
                WCHAR szwPathNameBuffer[MAX_Q_FORMATNAME_LEN+1]={0};
                size_t nResultedStringSize=0;
                size_t rc = mbstowcs_s(
                                                &nResultedStringSize,
                                                szwPathNameBuffer,
                                                sizeof(szwPathNameBuffer)/sizeof(szwPathNameBuffer[0]),
                                                szPathNameBuffer,
                                                _tcslen(szPathNameBuffer));
                if (rc != (size_t)(0))
                {
                    AfxMessageBox("An unsupported wide character was encountered. Exiting...");
                    exit(1);
                }


                                dwFormatNameSize = sizeof(szwFormatNameBuffer)/sizeof(szwFormatNameBuffer[0]);
                MQPathNameToFormatName(szwPathNameBuffer,szwFormatNameBuffer,&dwFormatNameSize);

                hrPathName = MQOpenQueue(
                        szwFormatNameBuffer,    // Format name of the queue to be opened.
                        dwAccess,               // Access rights to the queue.
                        0,                      // No exclusive receive.
                        &hQueue                 // OUT: handle to the opened queue.
                        );

#endif

            }

            //Results
            if (FAILED(hrPathName)&&FAILED(hrFormatName))
            {

            //
            // An error occurred, so display a message.
            //
                if (szFormatNameBuffer[0]!=0)
                {
                    _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("MQOpenQueue failed. Error code: 0x%x."),hrFormatName);
                }
                else
                {
                    _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("MQOpenQueue failed. Error code: 0x%x."),hrPathName);
                }
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);

            }
            else
            {
                //
                // Success was achieved, so write in the edit control.
                //
                _stprintf_s(szMsgBuffer,
                      sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]),
                      TEXT("The queue %s was opened successfully.\r\n\tQueueHandle: 0x%xI\r\n\tQueue Access : %s"),
                      szPathNameBuffer,
                      (ptrdiff_t)hQueue,
                      szAccessBuffer);
					PrintToScreen(szMsgBuffer);

                //
                // Move the queue to the array of opened queue path names.
                //

                MoveToOpenedQueuePathNameArray(szPathNameBuffer,szFormatNameBuffer,hQueue, dwAccess);
            }

        }

        /* ************************************************************************ */
        /*                            OnApiCloseQueue                               */
        /* ************************************************************************ */
        /* This function opens a dialog box and asks the user for the queue's       */
        /* path name. Then it closes the specified queue.                           */
        /*                                                                          */
        /* Uses: MQCloseQueue.                                                      */
        /* ************************************************************************ */
        void CMainFrame::OnApiCloseQueue()
        {

            TCHAR szPathNameBuffer[MAX_Q_PATHNAME_LEN+1];
            TCHAR szMsgBuffer[BUFFERSIZE];

            HRESULT hr;

            DWORD dwFormatNameBufferLength = sizeof(szPathNameBuffer)/sizeof(szPathNameBuffer[0]);

            QUEUEHANDLE hClosedQueueHandle;

            //
            // Display the Close Queue dialog box.
            //
            CCloseQueueDialog CloseQueueDialog(&m_OpenedQueuePathNameArray);

            if (CloseQueueDialog.DoModal() == IDCANCEL)
            {
                return;
            }

            CloseQueueDialog.GetPathName(szPathNameBuffer);

            //
            // Get the handle of the queue to be closed.
            //
            if (GetQueueHandle(szPathNameBuffer, &hClosedQueueHandle) == FALSE)
            {
                //
                // An error occurred, so display a message.
                //
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("The queue could not be closed because it was not opened before."));
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
                return;
            }

            //
            // Close the queue.
            //
            hr = MQCloseQueue(hClosedQueueHandle);   // The handle of the queue to be closed is passed.
            if (FAILED(hr))
            {
                //
                // An error occurred, so display a message.
                //
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("MQCloseQueue failed. Error code: 0x%x."),hr);
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
            }
            else
            {
                //
                // Success was achieved, so write in the edit control.
                //
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("The queue %s was closed successfully."), szPathNameBuffer);
                PrintToScreen(szMsgBuffer);
                //
                // Move the queue from the open queue path name array to the path name array.
                //
                MoveToPathNameArray(szPathNameBuffer);
            }
        }

        /* ************************************************************************ */
        /*                            OnApiSendMessage                              */
        /* ************************************************************************ */
        /* This function opens a dialog box and asks the user for the queue's path  */
        /* name and some message properties. Then it sends the message to the       */
        /* specified queue.                                                         */
        /*                                                                          */
        /* Uses: MQSendMessage.                                                     */
        /* ************************************************************************ */

        //
        // Declare two static buffers to hold the last message body and label for the next time.
        //
        TCHAR szLastMessageBody[BUFFERSIZE];
        TCHAR szLastMessageLabel[BUFFERSIZE];

        void CMainFrame::OnApiSendMessage()
        {
            // TODO: Add your command handler code here.

            TCHAR szPathNameBuffer[MAX_Q_PATHNAME_LEN+1]={0};
            TCHAR szAdminPathNameBuffer[MAX_Q_PATHNAME_LEN+1]={0};
            TCHAR szAdminFormatNameBuffer[MAX_Q_FORMATNAME_LEN+1]={0};
            TCHAR szMsgBuffer[BUFFERSIZE]={0};

            MQMSGPROPS MsgProps;
            MQPROPVARIANT aVariant[MAXINDEX];
            MSGPROPID aPropId[MAXINDEX];
            DWORD PropIdCount = 0;

            HRESULT hr;

            unsigned char bPriority;
            unsigned char bDelivery;
            unsigned char bJournal;
            unsigned char bDeadLetter;
            unsigned char bAuthenticated;
            unsigned char bEncrypted;
            unsigned char bAcknowledge;

            WCHAR szMessageBodyBuffer [BUFFERSIZE];
            WCHAR szMessageLabelBuffer[BUFFERSIZE];
            DWORD dwTimeToReachQueue;
            DWORD dwTimeToBeReceived;

            QUEUEHANDLE hQueue;

            CSendMessageDialog SendMessageDialog(&m_OpenedQueuePathNameArray);

            //
            // Display the Send Message dialog box.
            //
            if (SendMessageDialog.DoModal() == IDCANCEL)
            {
                return;
            }

            //
            // Retrieve the properties from the dialog box.
            //
            SendMessageDialog.GetPathName(szPathNameBuffer);
            SendMessageDialog.GetAdminPathName(szAdminPathNameBuffer);
            bPriority = SendMessageDialog.GetPriority();
            bDelivery = SendMessageDialog.GetDelivery();
            bJournal = SendMessageDialog.GetJournal();
            bDeadLetter = SendMessageDialog.GetDeadLetter();
            bAuthenticated = SendMessageDialog.GetAuthenticated();
            bEncrypted = SendMessageDialog.GetEncrypted();
            bAcknowledge = SendMessageDialog.GetAcknowledge();
            SendMessageDialog.GetMessageBody(szLastMessageBody);
            SendMessageDialog.GetMessageLabel(szLastMessageLabel);
            dwTimeToReachQueue = SendMessageDialog.GetTimeToReachQueue();
            dwTimeToBeReceived = SendMessageDialog.GetTimeToBeReceived();

            //
            // Update the properties of the last message.
            //
            _mqscpy(szMessageBodyBuffer, szLastMessageBody);
            _mqscpy(szMessageLabelBuffer, szLastMessageLabel);

            //
            // Get the target queue handle.
            //
            if (GetQueueHandle(szPathNameBuffer, &hQueue) == FALSE)
            {
                //
                // An error occurred, so display a message.
                //
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("GetQueueHandle failed. Queue not opened yet."));
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
                return;
            }

            //
            // Get the administration queue format name.
            //
            if (szAdminPathNameBuffer[0] != 0)
            {
                if (TranslatePathNameToFormatName(szAdminPathNameBuffer, _tcslen(szAdminPathNameBuffer), szAdminFormatNameBuffer, sizeof(szAdminFormatNameBuffer)/sizeof(szAdminFormatNameBuffer[0])) == FALSE)
                {
                    //
                    // An error occurred, so display a message.
                    //
                    _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("TranslatePathNameToFormatName failed, Queue has not been opened yet."));
                    MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
                    return;
                }
            }

            //
            // Prepare the property array PROPVARIANT.
            //

            //
            // Set the PROPID_M_PRIORITY property.
            //
            aPropId[PropIdCount] = PROPID_M_PRIORITY;    // PropId
            aVariant[PropIdCount].vt = VT_UI1;           // Type
            aVariant[PropIdCount].bVal = bPriority;      // Value

            PropIdCount++;

            //
            // Set the PROPID_M_DELIVERY property.
            //
            aPropId[PropIdCount] = PROPID_M_DELIVERY;    // PropId
            aVariant[PropIdCount].vt = VT_UI1;           // Type
            aVariant[PropIdCount].bVal = bDelivery;      // Value

            PropIdCount++;

            //
            // Set the PROPID_M_ACKNOWLEDGE property.
            //
            aPropId[PropIdCount] = PROPID_M_ACKNOWLEDGE; // PropId
            aVariant[PropIdCount].vt = VT_UI1;           // Type
            aVariant[PropIdCount].bVal = bAcknowledge;   // Value

            PropIdCount++;

            //
            // Set the PROPID_M_BODY property.
            //
            aPropId[PropIdCount] = PROPID_M_BODY;                  // PropId
            aVariant[PropIdCount].vt = VT_VECTOR|VT_UI1;           // Type
            aVariant[PropIdCount].caub.cElems =
                (wcslen(szMessageBodyBuffer) + 1) * sizeof(WCHAR); // Value
            aVariant[PropIdCount].caub.pElems = (unsigned char *)szMessageBodyBuffer;

            PropIdCount++;

            //
            // Set the PROPID_M_LABEL property.
            //
            aPropId[PropIdCount] = PROPID_M_LABEL;                  // PropId
            aVariant[PropIdCount].vt = VT_LPWSTR;                   // Type
            aVariant[PropIdCount].pwszVal = szMessageLabelBuffer;     // Value

            PropIdCount++;

            //
            // Set the PROPID_M_TIME_TO_REACH_QUEUE property.
            //
            aPropId[PropIdCount] = PROPID_M_TIME_TO_REACH_QUEUE;    // PropId
            aVariant[PropIdCount].vt = VT_UI4;                      // Type
            aVariant[PropIdCount].ulVal = dwTimeToReachQueue;       // Value

            PropIdCount++;

            //
            // Set the PROPID_M_TIME_TO_BE_RECEIVED property.
            //
            aPropId[PropIdCount] = PROPID_M_TIME_TO_BE_RECEIVED;    // PropId
            aVariant[PropIdCount].vt = VT_UI4;                      // Type
            aVariant[PropIdCount].ulVal = dwTimeToBeReceived;       // Value

            PropIdCount++;


            if (bJournal || bDeadLetter)
            {
                //
                // Set the PROPID_M_JOURNAL property.
                //
                aPropId[PropIdCount] = PROPID_M_JOURNAL;            // PropId
                aVariant[PropIdCount].vt = VT_UI1;                  // Type

                if (bJournal)
                    aVariant[PropIdCount].bVal = MQMSG_JOURNAL;
                else
                    aVariant[PropIdCount].bVal = 0;
                if (bDeadLetter)
                    aVariant[PropIdCount].bVal |= MQMSG_DEADLETTER;

                PropIdCount++;
            }


            if (bAuthenticated)
            {
                //
                // Set the PROPID_M_AUTH_LEVEL property.
                //
                aPropId[PropIdCount] = PROPID_M_AUTH_LEVEL;            // PropId
                aVariant[PropIdCount].vt = VT_UI4;                     // Type
                aVariant[PropIdCount].ulVal = MQMSG_AUTH_LEVEL_ALWAYS; // Value

                PropIdCount++;
            }

            if (bEncrypted)
            {
                //
                // Set the PROPID_M_ENCRYPTION_ALG property.
                //
                aPropId[PropIdCount] = PROPID_M_PRIV_LEVEL;                 // PropId
                aVariant[PropIdCount].vt = VT_UI4;                          // Type
                aVariant[PropIdCount].ulVal = MQMSG_PRIV_LEVEL_BODY_BASE;   // Value

                PropIdCount++;
            }


            //
            // Set the PROPID_M_ADMIN_QUEUE property.
            //
            if (szAdminFormatNameBuffer[0] != 0)
            {
                aPropId[PropIdCount] = PROPID_M_ADMIN_QUEUE;               // PropId
                aVariant[PropIdCount].vt = VT_LPWSTR;                      // Type
            #ifdef UNICODE
                aVariant[PropIdCount].pwszVal = szAdminFormatNameBuffer;   // Value
            #else
                WCHAR szwAdminFormatNameBuffer[MAX_Q_FORMATNAME_LEN+1];
                                size_t nResultedStringSize=0;
                size_t rc = mbstowcs_s(
                                                &nResultedStringSize,
                                                szwAdminFormatNameBuffer,
                                                sizeof(szwAdminFormatNameBuffer)/sizeof(szwAdminFormatNameBuffer[0]),
                        szAdminFormatNameBuffer,
                        _tcslen(szAdminFormatNameBuffer));
                if (rc != (size_t)(0))
                {
                    AfxMessageBox("An unsupported wide character was encountered. Exiting...");
                    exit(1);
                }
                aVariant[PropIdCount].pwszVal = szwAdminFormatNameBuffer;  // Value
            #endif

            PropIdCount++;
            }
            //
            // Initialize the MQMSGPROPS structure.
            //
            MsgProps.cProp = PropIdCount;       // Number of properties
            MsgProps.aPropID = aPropId;         // Identifiers of properties
            MsgProps.aPropVar = aVariant;       // Value of properties
            MsgProps.aStatus  = NULL;           // No error report

            //
            // Send the message.
            //
            hr = MQSendMessage(
                    hQueue,                     // Handle to the queue.
                    &MsgProps,                  // Message properties to be sent
                    NULL                        // No transaction
                    );

            if (FAILED(hr))
            {
                //
                // An error occurred, so display a message.
                //
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("MQSendMessage failed. Error code: 0x%x."),hr);
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
            }
            else
            {
                //
                // Success was achieved, so write in the edit control.
                //
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("The Message \"%s\" was sent successfully."), szLastMessageLabel);
                PrintToScreen(szMsgBuffer);
            }
        }

        /* ************************************************************************ */
        /*                           OnApiReceiveMessage                            */
        /* ************************************************************************ */
        /* This function opens a dialog box and asks the user for the queue's       */
        /* path name and the time to wait for the message. Then it tries to get a   */
        /* message from the specified queue within the given time.                  */
        /*                                                                          */
        /* Uses: MQReceiveMessage, MQFreeMemory.                                    */
        /* ************************************************************************ */
        void CMainFrame::OnApiReceiveMessage()
        {
            TCHAR szPathNameBuffer[MAX_Q_PATHNAME_LEN+1];
            TCHAR szMsgBuffer[2*BUFFERSIZE];
            TCHAR szDomainName[BUFFERSIZE];
             TCHAR szAccountName[BUFFERSIZE];
            DWORD dwActNameSize = sizeof(szAccountName);
            DWORD dwDomNameSize = sizeof(szDomainName);
            TCHAR szTextSid[BUFFERSIZE];
            DWORD dwTextSidSize = sizeof(szTextSid);
            BYTE  blobBuffer[BUFFERSIZE];

            MQMSGPROPS MsgProps;
            MQPROPVARIANT aVariant[MAXINDEX];
            MSGPROPID aPropId[MAXINDEX];
            DWORD PropIdCount = 0;

            HRESULT hr;

            WCHAR szMessageLabelBuffer[BUFFERSIZE];
            DWORD dwTimeout;

            QUEUEHANDLE hQueue;

            CReceiveWaitDialog    WaitDialog;
            CReceiveMessageDialog ReceiveMessageDialog(&m_OpenedQueuePathNameArray);

            //
            // Display the Receive Message dialog box.
            //
            if (ReceiveMessageDialog.DoModal() == IDCANCEL)
            {
                return;
            }

            ReceiveMessageDialog.DestroyWindow();
            ReceiveMessageDialog.GetPathName(szPathNameBuffer);

            //
            // Get the queue handle.
            //
            if (GetQueueHandle(szPathNameBuffer, &hQueue) == FALSE)
            {
                //
                // An error occurred, so display a message.
                //
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("GetQueueHandle failed. The queue was not found in array of opened queues."));
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
                return;
            }

            //
            // Retrieve the properties form the dialog box.
            //
            dwTimeout = ReceiveMessageDialog.GetTimeout();


            //
            // Prepare the property array PROPVARIANT of
            // message properties that we want to receive.
            //

            //
            // Set the PROPID_M_BODY property.
            //
            aPropId[PropIdCount] = PROPID_M_BODY;                                // PropId
            aVariant[PropIdCount].vt = VT_VECTOR|VT_UI1;                         // Type
            aVariant[PropIdCount].caub.cElems = ReceiveMessageDialog.GetBodySize();
            aVariant[PropIdCount].caub.pElems = (unsigned char *) new
                                       char [ aVariant[PropIdCount].caub.cElems ];

            int iBodyIndex = PropIdCount;
            PropIdCount++;

            //
            // Set the PROPID_M_LABEL property.
            //
            aPropId[PropIdCount] = PROPID_M_LABEL;                   // PropId
            aVariant[PropIdCount].vt = VT_LPWSTR;                    // Type
            aVariant[PropIdCount].pwszVal = szMessageLabelBuffer;

            PropIdCount++;

            //
            // Set the PROPID_M_PRIORITY property.
            //
            aPropId[PropIdCount] = PROPID_M_PRIORITY;               // PropId
            aVariant[PropIdCount].vt = VT_UI1;                      // Type

            PropIdCount++;

            //
            // Set the PROPID_M_CLASS property.
            //
            aPropId[PropIdCount] = PROPID_M_CLASS;                  // PropId
            aVariant[PropIdCount].vt = VT_UI2;                      // Type

            PropIdCount++;

            //
            // Set the PROPID_M_AUTHENTICATED property.
            //
            aPropId[PropIdCount] = PROPID_M_AUTHENTICATED;          // PropId
            aVariant[PropIdCount].vt = VT_UI1;                      // Type

            PropIdCount++;

            //
            // Set the PROPID_M_SENDERID property.
            //
            aPropId[PropIdCount] = PROPID_M_SENDERID;               // PropId
            aVariant[PropIdCount].vt = VT_UI1|VT_VECTOR;            // Type
            aVariant[PropIdCount].blob.pBlobData = blobBuffer;
            aVariant[PropIdCount].blob.cbSize = sizeof(blobBuffer);

            PropIdCount++;

            //
            // Set the PROPID_M_PRIV_LEVEL property.
            //
            aPropId[PropIdCount] = PROPID_M_PRIV_LEVEL;             // PropId
            aVariant[PropIdCount].vt = VT_UI4;                      // Type

            PropIdCount++;

            //
            // Set the PROPID_M_LABEL_LEN property.
            //
            aPropId[PropIdCount] = PROPID_M_LABEL_LEN;              // PropId
            aVariant[PropIdCount].vt = VT_UI4;                      // Type
            aVariant[PropIdCount].ulVal = BUFFERSIZE;               // Value

            PropIdCount++;


            //
            // Initialize the MQMSGPROPS structure.
            //
            MsgProps.cProp = PropIdCount;       // Number of properties
            MsgProps.aPropID = aPropId;         // Identifiers of properties
            MsgProps.aPropVar = aVariant;       // Value of properties
            MsgProps.aStatus  = NULL;           // No error report

            //
            // Display a message window until the message is received from the queue.
            //
            WaitDialog.Create(IDD_WAIT_DIALOG,pMainView);
            WaitDialog.ShowWindow(SW_SHOWNORMAL);
            WaitDialog.UpdateWindow();
            WaitDialog.CenterWindow();
            pMainView->RedrawWindow();

            //
            // Receive the message.
            //
            hr = MQReceiveMessage(
                       hQueue,               // Handle to the queue
                       dwTimeout,            // Max time (msec) to wait for the message
                       MQ_ACTION_RECEIVE,    // Action
                       &MsgProps,            // Properties to retrieve
                       NULL,                 // No OVERLAPPED structure
                       NULL,                 // No callback function
                       NULL,                 // No cursor
                       NULL                  // No transaction
                       );

            WaitDialog.ShowWindow(SW_HIDE);


            if(hr == MQ_ERROR_IO_TIMEOUT)
            {
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("MQReceiveMessage failed, The time-out period expired."));
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
            }
            else if(hr != MQ_OK)
            {
                //
                // An error occurred, so display a message.
                //
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("MQReceiveMessage failed. Error code: 0x%x."),hr);
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
            }
            else
            {
                //
                // Success was achieved, so write in the edit control.
                //
                ClassToString(aVariant[3].uiVal, szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]));
                PrintToScreen(szMsgBuffer);

                //
                // Print some of the message properties.
                //
        #ifdef UNICODE
                _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("\tLabel: %s"), (WCHAR *)(aVariant[1].pwszVal));
        #else
                {
                    PCHAR lpLable = UnicodeStringToAnsiString((WCHAR *)(aVariant[1].pwszVal));
                    _stprintf_s(szMsgBuffer,  sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("\tLabel: %s"), lpLable);
                    delete [] lpLable;
                }
        #endif
                PrintToScreen(szMsgBuffer);

                //
                // Print the body only if the message is not an acknowledgment or report message.
                // (This is done because there is no message body in acknowledgment messages.)
                //
                if (aVariant[3].bVal == MQMSG_CLASS_NORMAL)
                {
        #ifdef UNICODE
                    _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("\tBody : %s"), (WCHAR *)(aVariant[0].caub.pElems));
        #else
                    {
                        PCHAR pBody = UnicodeStringToAnsiString((WCHAR *)(aVariant[0].caub.pElems));
                        _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("\tBody : %s"), pBody);
                        delete [] pBody;
                    }
        #endif
                    PrintToScreen(szMsgBuffer);
                }

                _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("\tPriority : %d"), aVariant[2].bVal);
                PrintToScreen(szMsgBuffer);


                //
                // Print Sender ID
                //
                //
                // See if we're running on NT or Win95.
                //
                OSVERSIONINFO OsVerInfo;

                OsVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
                GetVersionEx(&OsVerInfo);

                if (OsVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
                {
                    //
                    //  On NT
                    //
                    SID_NAME_USE peUse;
                    if (LookupAccountSid(NULL,
                                         blobBuffer,
                                         szAccountName,
                                         &dwActNameSize,
                                         szDomainName,
                                         &dwDomNameSize,
                                         &peUse) )
                    {
                        _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("\tUser: %s\\%s"), szDomainName, szAccountName);
                        PrintToScreen(szMsgBuffer);
                    }
                }
                else
                {
                    //
                    // LookupAccountSid is not implemented on Win95,
                    // Instead print the textual SID.
                    //
                    if ( GetTextualSid((PSID)blobBuffer, szTextSid, &dwTextSidSize))
                    {
                        _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("\tUser SID : %s"), szTextSid);
                        PrintToScreen(szMsgBuffer);
                    }
                }
                //
                // Print "authenticated" or "not authenticated."
                //
                if (aVariant[4].bVal)
                    PrintToScreen(TEXT("\tMessage is authenticated."));
                else
                    PrintToScreen(TEXT("\tMessage is not authenticated."));


                //
                // Print "encrypted" or "not encrypted."
                //
                if (aVariant[6].ulVal)
                    PrintToScreen(TEXT("\tMessage is encrypted."));
                else
                    PrintToScreen(TEXT("\tMessage is not Encrypted."));
           }

           delete aVariant[ iBodyIndex ].caub.pElems;
        }

        /* ************************************************************************ */
        /*                               OnApiLocate                                */
        /* ************************************************************************ */
        /* This function opens a dialog box and asks the user to give a label. Then */
        /* it locates all the queues in the DS with a matching label.               */
        /* The function updates the path name array with those queues.              */
        /*                                                                          */
        /* Uses: MQLocateBegin, MQLocateNext, MQLocateEnd,                          */
        /*       MQInstanceToFormatName, MQFreeMemory.                              */
        /* ************************************************************************ */
        void CMainFrame::OnApiLocate()
        {
            // TODO: Add your command handler code here.
            TCHAR szMsgBuffer[BUFFERSIZE];
            TCHAR szLabelBuffer[BUFFERSIZE];

            HRESULT hr;

            MQPROPERTYRESTRICTION PropertyRestriction;
            MQRESTRICTION  Restriction;
            MQCOLUMNSET    Column;
            QUEUEPROPID    aPropId[2]; // only two properties to retrieve
            HANDLE         hEnum;
            DWORD       cQueue;
            MQPROPVARIANT   aPropVar[MAX_VAR] = {0};
            ARRAYQ*         pArrayQ;
            DWORD       i;
            DWORD           dwColumnCount = 0;
            DWORD dwFormatNameLength = MAX_Q_FORMATNAME_LEN;

            CLocateDialog LocateDialog;

            //
            // Display the Receive Message dialog box.
            //
            if (LocateDialog.DoModal() == IDCANCEL)
            {
                return;
            }

            //
            // Retrieve the label from the dialog box.
            //
            LocateDialog.GetLabel(szLabelBuffer);

            //
            // Clean the path name array before locating queues.
            //
            CleanPathNameArray();

            //
            // Prepare parameters for locating queues.
            //

            //
            // Prepare a property restriction.
            // Restriction: All queues with PROPID_Q_LABEL equal to "MQ API test."
            //
            PropertyRestriction.rel = PREQ;
            PropertyRestriction.prop = PROPID_Q_LABEL;
            PropertyRestriction.prval.vt = VT_LPWSTR;
        #ifdef UNICODE
            PropertyRestriction.prval.pwszVal = szLabelBuffer;
        #else
            DWORD size = _tcslen(szLabelBuffer) +1;
            PropertyRestriction.prval.pwszVal = new WCHAR[size];
            AnsiStringToUnicode(PropertyRestriction.prval.pwszVal, szLabelBuffer,size);
        #endif

            //
            // Prepare a restriction with one property restriction.
            //
            Restriction.cRes = 1;
            Restriction.paPropRes = &PropertyRestriction;

            //
            // Specify the properties that should be returned by the query.
            // Only the path name is important.
            //
            aPropId[dwColumnCount] = PROPID_Q_PATHNAME;
            dwColumnCount++;

            aPropId[dwColumnCount] = PROPID_Q_INSTANCE;
            dwColumnCount++;

            Column.cCol = dwColumnCount;
            Column.aCol = aPropId;

            //
            // Run the query to locate the queues.
            //
            hr = MQLocateBegin(
                    NULL,           // Start search at the top.
                    &Restriction,   // Restriction
                    &Column,        // Properties to be retrieved
                    NULL,           // No sort order
                    &hEnum          // Enumeration handle
                    );

            if(FAILED(hr))
            {
                //
                // An error occurred, so display a message.
                //
                _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]),
                    TEXT("MQLocateBegin failed. Error code: 0x%x."),hr);
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
                return;
            }

            //
            // Get the results.
            //
            cQueue = MAX_VAR;

            //
            // If cQueue == 0, no Variants were retrieved in the last call to MQLocateNext.
            //
            while (cQueue != 0)
            {
                hr = MQLocateNext(
                        hEnum,      // Handle returned by MQLocateBegin
                        &cQueue,    // Size of aPropVar
                        aPropVar    // OUT: an array of MQPROPVARIANT for receiving the results
                        );

                if(FAILED(hr))
                {
                    //
                    // An error occurred, so display a message.
                    //
                    _stprintf_s(szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]),
                        TEXT("MQLocateNext failed. Error code: 0x%x."),hr);
                    MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
                    return;
                }

                for (i=0; i<cQueue; i++)
                {
                    //
                    // Add the new path names to the path name array.
                    //
                    pArrayQ = new ARRAYQ;
                    if (!pArrayQ)
                        return;

        #ifdef UNICODE
                    wcsncpy_s (
                                            pArrayQ->szPathName,
                                            sizeof(pArrayQ->szPathName)/sizeof(pArrayQ->szPathName[0]),
                                            aPropVar[i].pwszVal,
                                            sizeof(pArrayQ->szPathName)/sizeof(pArrayQ->szPathName[0])-1
                                            );

        #else
                                        size_t   nResultedStringSize=0;
                    size_t rc = wcstombs_s(
                                                        &nResultedStringSize,
                                                        pArrayQ->szPathName,
                                                        sizeof(pArrayQ->szPathName)/sizeof(pArrayQ->szPathName[0]),
                                                        aPropVar[i].pwszVal,
                                                        sizeof(pArrayQ->szPathName)/sizeof(pArrayQ->szPathName[0])-1
                                                        );
                    if (rc != (size_t)(0))
                    {
                        AfxMessageBox("An unsupported wide character was encountered. Ignoring queue.");
                        MQFreeMemory(aPropVar[i].pwszVal);
                        i = i + 1;
                        delete pArrayQ;
                        continue;
                    }
        #endif


                    //
                    // Free the memory allocated by MSMQ.
                    //
                    MQFreeMemory(aPropVar[i].pwszVal);


                    //
                    // Move to the next property.
                    //
                    i = i + 1;

                    //
                    // Get the format name of the queue and add the queue to the path name array.
                    //
        #ifdef UNICODE
                                        dwFormatNameLength = sizeof(pArrayQ->szFormatName)/sizeof(pArrayQ->szFormatName[0]);
                    hr = MQInstanceToFormatName(aPropVar[i].puuid, pArrayQ->szFormatName, &dwFormatNameLength);
        #else
                    WCHAR szwFormatNameBuffer[MAX_Q_FORMATNAME_LEN+1];
                                        dwFormatNameLength = sizeof(szwFormatNameBuffer)/sizeof(szwFormatNameBuffer[0]);
                    hr = MQInstanceToFormatName(aPropVar[i].puuid, szwFormatNameBuffer, &dwFormatNameLength);
                    if (SUCCEEDED(hr))
                    {
                                                size_t rwc = wcstombs_s(
                                                               &nResultedStringSize,
                                                               pArrayQ->szFormatName,
                                                               sizeof(pArrayQ->szFormatName)/sizeof(pArrayQ->szFormatName[0]),
                                                               szwFormatNameBuffer,
                                                               dwFormatNameLength
                                                               );

                        if (rwc != (size_t)(0))
                        {
                            AfxMessageBox("An unsupported wide character was encountered. Exiting...");
                            exit(1);
                        }
                    }
        #endif

                    if(FAILED(hr))
                    {
                        //
                        // An error occurred, so display a message.
                        //
                        _stprintf_s (szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]),
                            TEXT("MQInstanceToFormatName failed. Error code: 0x%x."),hr);
                        MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
                    }

                    //
                    // Add the new queue to the path name array.
                    //
                    Add2PathNameArray(pArrayQ);
                }
            }

            //
            // End the locate operation.
            //
            hr = MQLocateEnd(hEnum);   // handle returned by MQLocateBegin
            if(FAILED(hr))
            {
                //
                // An error occurred, so display a message.
                //
                _stprintf_s (szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]),
                    TEXT("MQLocateEnd failed. Error code: 0x%x."),hr);
                MessageBox(szMsgBuffer, TEXT("ERROR"), MB_OK);
                return;
            }

            //
            // Display the queues found during the query.
            //
            _stprintf_s (szMsgBuffer, sizeof(szMsgBuffer)/sizeof(szMsgBuffer[0]), TEXT("Locate Operation completed successfully"));
            PrintToScreen(szMsgBuffer);
            UpdatePathNameArrays();
            DisplayPathNameArray();
            DisplayOpenedQueuePathNameArray();
        }

        /* ************************************************************************ */
        /*                           OnUpdateFrameTitle                             */
        /* ************************************************************************ */
        void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
        {
            SetWindowText (TEXT("MQ API test"));
        }


BOOL CMainFrame::IsFormatName(TCHAR *pszName)
{
    int Index;
    int MaxIndex = m_OpenedQueuePathNameArray.GetSize();

    for (Index=0; Index<MaxIndex; Index++)
    {
        if (_tcscmp(pszName, m_OpenedQueuePathNameArray[Index]->szFormatName) == 0)
        {
            return TRUE;
        }
    }

    MaxIndex=m_PathNameArray.GetSize();
    for (Index=0; Index<MaxIndex; Index++)
    {
        if (_tcscmp(pszName, m_PathNameArray[Index]->szFormatName) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}
