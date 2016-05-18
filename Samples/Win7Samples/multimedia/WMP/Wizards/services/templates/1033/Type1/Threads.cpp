#include "stdafx.h"
#include "[!output Safe_root].h"

////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
//    StartContentPartnerThread method
//
// ContentPartnerThreadProc function
// ContentPartnerMessageLoop function
// RemoveMessagesFromDownloadThreadQueue function
// RemoveMessagesFromDBuyThreadQueue function
// RemoveMessagesFromRefreshLicenseThreadQueue function
// RemoveMessagesFromLoginThreadQueue function
// RemoveMessagesFromUpadateDeviceThreadQueue function
// RemoveMessagesFromListThreadQueue function
// ShutdownThreads function
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////

HRESULT ContentPartnerMessageLoop(
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx,
   CComPtr<IWMPContentPartnerCallback> spCallback);

HRESULT HandleMessageForDownloadThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback);

VOID RemoveMessagesFromDownloadThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx);

HRESULT HandleMessageForBuyThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback);

VOID RemoveMessagesFromBuyThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx);

HRESULT HandleMessageForRefreshLicenseThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback);

VOID RemoveMessagesFromRefreshLicenseThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx);

HRESULT HandleMessageForLoginThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback);

VOID RemoveMessagesFromLoginThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx);

HRESULT HandleMessageForSendMessageThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback);

VOID RemoveMessagesFromSendMessageThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx);

HRESULT HandleMessageForUpdateDeviceThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback);

VOID RemoveMessagesFromUpdateDeviceThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx);

HRESULT HandleMessageForListThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback);

VOID RemoveMessagesFromListThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx);


HRESULT STDMETHODCALLTYPE C[!output Safe_root]::StartContentPartnerThread(ContentPartnerThreadType threadType)
{
   DWORD waitResult = 0;
   HRESULT hr = S_OK;
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx = NULL;
   HANDLE newThreadHandle = NULL;
   DWORD newThreadId = 0;

   // What does the new thread need to get started?
   //    1. A member of the ContentPartnerThreadType enumeration
   //    2. IStream that it can use to unmarshal IWMPContentPartnerCallback
   //    3. Event handle it can use to signal that the message loop is started
   //    4. ID of the message to exit the message loop 
   //    5. IDs messages that will be sent to the thread   
   // Items 1 - 4 are the first four members of the CONTENT_PARTNER_THREAD_CONTEXT structure.
   // The fifth member of the CONTENT_PARTNER_THREAD_CONTEXT structure is a union.
   // It holds the IDs of messages that will be sent to the thread.

   pThreadCtx = new CONTENT_PARTNER_THREAD_CONTEXT();
   // This memory is freed in ContentPartnerThreadProc.


   if(!pThreadCtx)
   {
      ATLTRACE2("%x: StartThread(type %d): Failed to create CONTENT_PARTNER_THREAD_CONTEXT.\n", GetCurrentThreadId(), threadType);
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   ZeroMemory(pThreadCtx, sizeof(CONTENT_PARTNER_THREAD_CONTEXT));

   pThreadCtx->threadType = threadType;
      
   hr = CoMarshalInterThreadInterfaceInStream(
      __uuidof(IWMPContentPartnerCallback), 
      m_spCallback, 
      &pThreadCtx->pIStream);

   if(FAILED(hr))
   {
      ATLTRACE2("%x: StartThread(type %d): Failed to marshal IWMPContentPartnerCallback interface. %x\n", GetCurrentThreadId(), threadType, hr);
      goto cleanup; 
   }

   if(NULL == pThreadCtx->pIStream)
   {
      hr = E_UNEXPECTED;
      goto cleanup;
   }

   pThreadCtx->hInitialized = CreateEvent(NULL, TRUE, FALSE, NULL);

   if(NULL == pThreadCtx->hInitialized)
   {
      ATLTRACE2("%x: StartThread(type %d): CreateEvent failed.\n", GetCurrentThreadId(), threadType);
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }
 
   pThreadCtx->msgExitMessageLoop = m_msgExitMessageLoop;

   switch(threadType)
   {
   case ThreadTypeDownload:
      pThreadCtx->downloadThreadContext.msgDownloadBatch = m_msgDownloadBatch;  
      break;
   case ThreadTypeBuy:
      pThreadCtx->buyThreadContext.msgBuy = m_msgBuy;    
      break;
   case ThreadTypeRefreshLicense:
      pThreadCtx->refreshLicenseThreadContext.msgRefreshLicense = m_msgRefreshLicense;  
      break;
   case ThreadTypeLogin:
      pThreadCtx->loginThreadContext.msgAuthenticate = m_msgAuthenticate; 
      pThreadCtx->loginThreadContext.msgLogin = m_msgLogin;
      pThreadCtx->loginThreadContext.msgLogout = m_msgLogout; 
      break;
   case ThreadTypeSendMessage:
      pThreadCtx->sendMessageThreadContext.msgSendMessage = m_msgSendMessage;  
      break;
   case ThreadTypeUpdateDevice:
      pThreadCtx->updateDeviceThreadContext.msgUpdateDevice = m_msgUpdateDevice;  
      break;
   case ThreadTypeList:
      pThreadCtx->listThreadContext.msgGetListContents = m_msgGetListContents;
      break;
   default:
      ATLTRACE2("%x: StartThread: Unrecognized thread type.\n", GetCurrentThreadId(), 0);
      hr = E_INVALIDARG;
      goto cleanup;
   } // switch(threadType)

   // At this point, all members of pThreadCtx are initialized.

   newThreadHandle = CreateThread(
         NULL,
         0,
         ContentPartnerThreadProc,
         static_cast<LPVOID>(pThreadCtx),
         0,
         &newThreadId);

   // The documentation for CreateThread says this:
   // "If this function fails, the return value is NULL."
   if(NULL == newThreadHandle)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ATLTRACE2("%x: StartThread(type %d): CreateThread failed. %x\n", threadType, GetCurrentThreadId(), hr);   
      goto cleanup;
   }

   // The new thread handle is closed in ShutdownThreads.

   // Store the new thread handle and thread ID.
   switch(threadType)
   {
   case ThreadTypeDownload:
      m_downloadThreadHandle = newThreadHandle;
      m_downloadThreadId = newThreadId;  
      break;
   case ThreadTypeBuy:
      m_buyThreadHandle = newThreadHandle;
      m_buyThreadId = newThreadId;   
      break;
   case ThreadTypeRefreshLicense:
      m_refreshLicenseThreadHandle = newThreadHandle;
      m_refreshLicenseThreadId = newThreadId;    
      break;
   case ThreadTypeLogin:
      m_loginThreadHandle = newThreadHandle;
      m_loginThreadId = newThreadId;  
      break;
   case ThreadTypeSendMessage:
      m_sendMessageThreadHandle = newThreadHandle;
      m_sendMessageThreadId = newThreadId;  
      break;

   case ThreadTypeUpdateDevice:
      m_updateDeviceThreadHandle = newThreadHandle;
      m_updateDeviceThreadId = newThreadId;  
      break;

   case ThreadTypeList:
      m_listThreadHandle = newThreadHandle;
      m_listThreadId = newThreadId;  
      break;

   default:
      ATLTRACE2("%x: StartThread: Unrecognized thread type.\n", GetCurrentThreadId(), 0);
      hr = E_INVALIDARG;
      goto cleanup;
   } // switch(threadType)
   
   // Wait for the new thread to signal that it is initialized.
  
   waitResult = WaitForSingleObject(pThreadCtx->hInitialized, 30000);

   if(WAIT_FAILED == waitResult)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());   
      goto cleanup;
   }

   ATLTRACE2("%x: StartThread: Thread(type %d) signaled that it is initialized.\n", GetCurrentThreadId(), threadType);

   // At this point, we know the new thread called SetEvent to
   // let us know that we can stop waiting.
   // But that could mean one of two things:
   //   1. The new thread called SetEvent and then entered its message loop.
   //   2. The new thread failed to enter its message loop and 
   //      called SetEvent as part of its cleanup code.
   //
   // The point is this: we know that the new thread started, but we
   // do not know whether the new thread is still active.
   
 cleanup:

   if(pThreadCtx->hInitialized)
   {
      CloseHandle(pThreadCtx->hInitialized);
      pThreadCtx->hInitialized = 0;
   }

   if(NULL == newThreadHandle)
   {
      // The new thread did not start; that is, CreateThread failed.
      // The new thread will not be able to free the memory pointed
      // to by pThreadCtx, so we free it here.
 
      if(pThreadCtx)
      {
         delete pThreadCtx;
         pThreadCtx = NULL;
      }
   }

   return hr;

} // StartContentPartnerThread



DWORD WINAPI ContentPartnerThreadProc(LPVOID lpParameter)
{
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx = NULL;
   CComPtr<IWMPContentPartnerCallback> spCallback;
   MSG msg = {0};
   HRESULT hr = S_OK;
   BOOL comInitialized = FALSE;

   if(NULL == lpParameter)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   pThreadCtx = static_cast<CONTENT_PARTNER_THREAD_CONTEXT*>(lpParameter);

   if(NULL == pThreadCtx->pIStream)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

   if(FAILED(hr))
   {
      ATLTRACE2("%x: ContentPartnerThreadProc(type %d): CoInitializeEx failed. %x\n", GetCurrentThreadId(), pThreadCtx->threadType, hr);
      goto cleanup;
   }  

   comInitialized = TRUE;

   // Get a pointer to an IWMPContentPartnerCallback interface.

   hr = CoGetInterfaceAndReleaseStream(
      pThreadCtx->pIStream,
      __uuidof(IWMPContentPartnerCallback),
      reinterpret_cast<LPVOID*>(&spCallback) );

   // The stream was released (even if CoGetInterfaceAndReleaseStream failed). 
   // Set the stream pointer to NULL.
   pThreadCtx->pIStream = NULL;

   if(FAILED(hr))
   {
      ATLTRACE2("%x: ContentPartnerThreadProc(type %d): Failed to get IWMPContentPartnerCallback interface. %x\n", GetCurrentThreadId(), pThreadCtx->threadType, hr);
      goto cleanup;
   }

   if(NULL == spCallback)
   {
      hr = E_UNEXPECTED;
      goto cleanup;
   }

   ATLTRACE2("%x: ContentPartnerThreadProc(type %d): Succeeded in getting IWMPContentPartnerCallback interface.\n", GetCurrentThreadId(), pThreadCtx->threadType);

   // Make sure we have a message queue.
   PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

   // Tell the main thread that we are ready to receive messages.
   SetEvent(pThreadCtx->hInitialized);

   hr = ContentPartnerMessageLoop(pThreadCtx, spCallback);

cleanup:

   if(NULL != pThreadCtx)
   {
      // Set this event here, just in case there was a "goto cleanup"
      // before the event was set.
      SetEvent(pThreadCtx->hInitialized);

      // The thread that started this thread allocated a
      // CONTENT_PARTNER_THREAD_CONTEXT structure.
      // We must free that memory here.
   
     if(NULL != pThreadCtx->pIStream)
      {
         // For some reason, CoGetInterfaceAndReleaseStream never got called.
         // So release the stream here.
         pThreadCtx->pIStream->Release();
         pThreadCtx->pIStream = NULL;
      }

      ATLTRACE2("%x: ContentPartnerThreadProc(type %d): Returning %x\n", GetCurrentThreadId(), pThreadCtx->threadType, hr);

      delete pThreadCtx;
      pThreadCtx = NULL;
   }
                   
   if(comInitialized)
   {
      CoUninitialize();
   } 

 
   return hr;
}


HRESULT ContentPartnerMessageLoop(
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx,
   CComPtr<IWMPContentPartnerCallback> spCallback)
{
   MSG msg = {0};
   HRESULT hr = S_OK;

   if(NULL == pThreadCtx || NULL == spCallback)
   {
      hr = E_UNEXPECTED;
      goto cleanup;
   }

   // Windows message loop
   while(TRUE)
   {
      BOOL ret = 0;

      // Suppose we have several messages (for example, 
      // buy or download messages) in the queue, and then 
      // the user closes Windows Media Player. We would like
      // to let Windows Media Player close without waiting for all
      // those queued messages to be processed.
      //
      // So we use the following strategy.
      //
      // Peek into the message queue to see if there is an exit message.
      // If there is an exit message anywhere in the queue, break out of
      // the message loop even though there might be several
      // messages remaining in the queue.
      
      if( PeekMessage(
             &msg, 
             NULL, 
             pThreadCtx->msgExitMessageLoop, 
             pThreadCtx->msgExitMessageLoop,
             PM_REMOVE) )
      {
         ATLTRACE2("%x: ContentPartnerMessageLoop(type &d): PeekMessage retrieved an exit message.\n", GetCurrentThreadId(), pThreadCtx->threadType);
         goto cleanup;
      }

      ret = GetMessage(&msg, 0, 0, 0);
      if(-1 == ret)
      {
          ATLTRACE2("%x: ContentPartnerMessageLoop(type %d): GetMessage failed (returned -1).\n", GetCurrentThreadId(), pThreadCtx->threadType);
          hr = HRESULT_FROM_WIN32(GetLastError());
          goto cleanup;
      }

      if(pThreadCtx->msgExitMessageLoop == msg.message)
      {
         ATLTRACE2("%x: ContentPartnerMessageLoop(type %d): GetMessage retrieved an exit message.\n", GetCurrentThreadId(), pThreadCtx->threadType);
         break; // Break out of the message loop.
      }

      switch(pThreadCtx->threadType)
      {
      case ThreadTypeDownload:
         hr = HandleMessageForDownloadThread(&msg, pThreadCtx, spCallback);
         break;
      case ThreadTypeBuy:
         hr = HandleMessageForBuyThread(&msg, pThreadCtx, spCallback);
         break;
      case ThreadTypeRefreshLicense:
         hr = HandleMessageForRefreshLicenseThread(&msg, pThreadCtx, spCallback);
         break;
      case ThreadTypeLogin:
         hr = HandleMessageForLoginThread(&msg, pThreadCtx, spCallback);
         break;
      case ThreadTypeSendMessage:
         hr = HandleMessageForSendMessageThread(&msg, pThreadCtx, spCallback);
         break;
      case ThreadTypeUpdateDevice:
         hr = HandleMessageForUpdateDeviceThread(&msg, pThreadCtx, spCallback);
         break;
      case ThreadTypeList:
         hr = HandleMessageForListThread(&msg, pThreadCtx, spCallback);
         break;
      default:
         hr = E_UNEXPECTED;
      } // switch(threadType)
      

      if(FAILED(hr))
      {
         goto cleanup;
      }

   } // while(TRUE)

cleanup:

   if(NULL != pThreadCtx)
   {
      switch(pThreadCtx->threadType)
      {
      case ThreadTypeDownload:
         RemoveMessagesFromDownloadThreadQueue(pThreadCtx);
         break;
      case ThreadTypeBuy:
         RemoveMessagesFromDownloadThreadQueue(pThreadCtx);
         break;
      case ThreadTypeRefreshLicense:
         RemoveMessagesFromDownloadThreadQueue(pThreadCtx);
         break;
      case ThreadTypeLogin:
         RemoveMessagesFromDownloadThreadQueue(pThreadCtx);
         break;
      case ThreadTypeSendMessage:
         RemoveMessagesFromSendMessageThreadQueue(pThreadCtx);
         break;
      case ThreadTypeUpdateDevice:
         RemoveMessagesFromUpdateDeviceThreadQueue(pThreadCtx);
         break;
      case ThreadTypeList:
         RemoveMessagesFromListThreadQueue(pThreadCtx);
         break;
      } // switch(threadType)

      ATLTRACE2("%x: ContentPartnerMessageLoop(type %d): Returning %x.\n", GetCurrentThreadId(), pThreadCtx->threadType, hr);
   }

   return hr;
} // ContentPartnerMessageLoop



VOID RemoveMessagesFromDownloadThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx)
{
   DOWNLOAD_BATCH_CONTEXT* pBatchCtx = NULL;
   MSG msg = {0};

   while( PeekMessage( 
             &msg, 
             NULL, 
             pThreadCtx->downloadThreadContext.msgDownloadBatch, 
             pThreadCtx->downloadThreadContext.msgDownloadBatch, 
             PM_REMOVE ) )
      {  
         ATLTRACE2("%x: RemoveMessagesFromDownloadThreadQueue: PeekMessage in cleanup retrieved message to download a batch.\n", GetCurrentThreadId());
         pBatchCtx = reinterpret_cast<DOWNLOAD_BATCH_CONTEXT*>(msg.lParam);

         if(NULL != pBatchCtx)
         {
            if(NULL != pBatchCtx->pIStream)
            {
               pBatchCtx->pIStream->Release();
               pBatchCtx->pIStream = NULL;         
            }

            delete pBatchCtx;
            pBatchCtx = NULL;
         }

      } // while PeekMessage
}


VOID RemoveMessagesFromBuyThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx)
{
   BUY_CONTEXT* pBuyCtx = NULL;
   MSG msg = {0};

   while( PeekMessage( 
      &msg, 
      NULL, 
      pThreadCtx->buyThreadContext.msgBuy, 
      pThreadCtx->buyThreadContext.msgBuy, 
      PM_REMOVE ) )
   {  
      ATLTRACE2("%x: RemoveMessagesFromBuyThreadQueue: PeekMessage in cleanup retrieved message to buy.\n", GetCurrentThreadId());
      pBuyCtx = reinterpret_cast<BUY_CONTEXT*>(msg.lParam);

      if(NULL != pBuyCtx)
      {
         if(NULL != pBuyCtx->cookie)
         {
            pBuyCtx->pIStream->Release();
            pBuyCtx->pIStream = NULL;         
         }

         delete pBuyCtx;
         pBuyCtx = NULL;
      }
   } // while PeekMessage
}

VOID RemoveMessagesFromRefreshLicenseThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx)
{
   REFRESH_LICENSE_CONTEXT* pRefLicCtx = NULL;
   MSG msg = {0};

   while( PeekMessage( 
      &msg, 
      NULL, 
      pThreadCtx->refreshLicenseThreadContext.msgRefreshLicense, 
      pThreadCtx->refreshLicenseThreadContext.msgRefreshLicense, 
      PM_REMOVE ) )
   {  
      ATLTRACE2("%x: RemoveMessagesFromRefreshLicenseThreadQueue: PeekMessage in cleanup retrieved message to refresh a license.\n", GetCurrentThreadId());
      pRefLicCtx = reinterpret_cast<REFRESH_LICENSE_CONTEXT*>(msg.lParam);

      if(NULL != pRefLicCtx)
      {
         delete pRefLicCtx;
         pRefLicCtx = NULL;
      }

   } // while PeekMessage
}

VOID RemoveMessagesFromLoginThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx)
{
   LOGIN_CONTEXT* pLoginCtx = NULL;
   MSG msg = {0};

   while( PeekMessage( 
      &msg, 
      NULL, 
      pThreadCtx->loginThreadContext.msgAuthenticate, 
      pThreadCtx->loginThreadContext.msgAuthenticate, 
      PM_REMOVE ) )
   {  
      ATLTRACE2("%x: RemoveMessagesFromLoginQueue: PeekMessage in cleanup retrieved message to authenticate.\n", GetCurrentThreadId());
      pLoginCtx = reinterpret_cast<LOGIN_CONTEXT*>(msg.lParam);

      if(NULL != pLoginCtx)
      {
         delete pLoginCtx;
         pLoginCtx = NULL;
      }

   } // while PeekMessage

   while( PeekMessage( 
      &msg, 
      NULL, 
      pThreadCtx->loginThreadContext.msgLogin, 
      pThreadCtx->loginThreadContext.msgLogin, 
      PM_REMOVE ) )
   {  
      ATLTRACE2("x RemoveMessagesFromLoginQueue: PeekMessage in cleanup retrieved message to log in.\n", GetCurrentThreadId());
      pLoginCtx = reinterpret_cast<LOGIN_CONTEXT*>(msg.lParam);

      if(NULL != pLoginCtx)
      {
         delete pLoginCtx;
         pLoginCtx = NULL;
      }

   } // while PeekMessage

   while( PeekMessage( 
      &msg, 
      NULL, 
      pThreadCtx->loginThreadContext.msgLogout, 
      pThreadCtx->loginThreadContext.msgLogout, 
      PM_REMOVE ) )
   {  
      ATLTRACE2("%x: RemoveMessagesFromLoginQueue: PeekMessage in cleanup retrieved message to log out.\n", GetCurrentThreadId());
      
      // There is no context associated with a log-out message.

   } // while PeekMessage
}




VOID RemoveMessagesFromSendMessageThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx)
{
   SEND_MESSAGE_CONTEXT* pSendMsgCtx = NULL;
   MSG msg = {0};

   while( PeekMessage( 
      &msg, 
      NULL, 
      pThreadCtx->sendMessageThreadContext.msgSendMessage,
      pThreadCtx->sendMessageThreadContext.msgSendMessage, 
      PM_REMOVE ) )
   {  
      ATLTRACE2("%x: RemoveMessagesFromSendMessageThreadQueue: PeekMessage in cleanup retrieved a send-message message.\n", GetCurrentThreadId());
      pSendMsgCtx = reinterpret_cast<SEND_MESSAGE_CONTEXT*>(msg.lParam);

      if(NULL != pSendMsgCtx)
      {
         SysFreeString(pSendMsgCtx->bstrMsg);  // OK to pass NULL.
         SysFreeString(pSendMsgCtx->bstrParam);
         delete pSendMsgCtx;
         pSendMsgCtx = NULL;
      }

   } // while PeekMessage
}


VOID RemoveMessagesFromUpdateDeviceThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx)
{
   UPDATE_DEVICE_CONTEXT* pUpDevCtx = NULL;
   MSG msg = {0};

   while( PeekMessage( 
      &msg, 
      NULL, 
      pThreadCtx->updateDeviceThreadContext.msgUpdateDevice,
      pThreadCtx->updateDeviceThreadContext.msgUpdateDevice, 
      PM_REMOVE ) )
   {  
      ATLTRACE2("%x: RemoveMessagesFromUpdateDeviceThreadQueue: PeekMessage in cleanup retrieved an update-device message.\n", GetCurrentThreadId());
      pUpDevCtx = reinterpret_cast<UPDATE_DEVICE_CONTEXT*>(msg.lParam);

      if(NULL != pUpDevCtx)
      {
         SysFreeString(pUpDevCtx->bstrDeviceName);  // OK to pass NULL.
         delete pUpDevCtx;
         pUpDevCtx = NULL;
      }

   } // while PeekMessage
}

VOID RemoveMessagesFromListThreadQueue(CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx)
{
   LIST_CONTEXT* pListCtx = NULL;
   MSG msg = {0};

   while( PeekMessage( 
      &msg, 
      NULL, 
      pThreadCtx->listThreadContext.msgGetListContents,
      pThreadCtx->listThreadContext.msgGetListContents, 
      PM_REMOVE ) )
   {  
      ATLTRACE2("%x: RemoveMessagesFromListThreadQueue: PeekMessage in cleanup retrieved a get-list-contents message.\n", GetCurrentThreadId());
      pListCtx = reinterpret_cast<LIST_CONTEXT*>(msg.lParam);

      if(NULL != pListCtx)
      {
         SysFreeString(pListCtx->location);  // OK to pass NULL. 
         VariantClear( &(pListCtx->context) );  
         SysFreeString(pListCtx->bstrListType);
         SysFreeString(pListCtx->bstrParams);       
         delete pListCtx;
         pListCtx = NULL;
      }

   } // while PeekMessage
}


HRESULT STDMETHODCALLTYPE C[!output Safe_root]::ShutdownThreads()
{
   BOOL postResult = 0;
   HRESULT hr = S_OK;
   HANDLE threadHandles[16] = {0};
   ULONG numThreadHandles = 0;

   // Tell the download thread to exit its message loop.
   if(m_downloadThreadHandle)
   {
      postResult = 0;
      postResult = PostThreadMessage(m_downloadThreadId, m_msgExitMessageLoop, 0, 0);

      if(0 == postResult)
      {
         hr = HRESULT_FROM_WIN32(GetLastError());
         ATLTRACE2("%x: ShutdownThreads: Unable to post m_msgExitMessageLoop to the download thread. %x\n", GetCurrentThreadId(), hr);     
      }
      else
      {
         // There is a download thread, and we successfully 
         // posted an exit message to the download thread.
         ATLTRACE2("%x: ShutdownThreads: Successfully posted m_msgExitMessageLoop to the download thread.\n", GetCurrentThreadId());

         // Add the download thread handle to the array of handles.
         threadHandles[numThreadHandles] = m_downloadThreadHandle;
         ++numThreadHandles;
      }     
   }

   // Tell the buy thread to exit its message loop.
   if(m_buyThreadHandle)
   {
      postResult = 0;
      postResult = PostThreadMessage(m_buyThreadId, m_msgExitMessageLoop, 0, 0);

      if(0 == postResult)
      {
         hr = HRESULT_FROM_WIN32(GetLastError());  
         ATLTRACE2("%x: ShutdownThreads: Unable to post m_msgExitMessageLoop to the buy thread. %x\n", GetCurrentThreadId(), hr);
      }
      else
      {
         // There is a buy thread, and we successfully 
         // posted an exit message to the buy thread.
         ATLTRACE2("%x: ShutdownThreads: Successfully posted m_msgExitMessageLoop to the buy thread.\n", GetCurrentThreadId());

         // Add the buy thread handle to the array of handles.
         threadHandles[numThreadHandles] = m_buyThreadHandle;
         ++numThreadHandles;
      }     
   }

   // Tell the refresh-license thread to exit its message loop.
   if(m_refreshLicenseThreadHandle)
   {
      postResult = 0;
      postResult = PostThreadMessage(m_refreshLicenseThreadId, m_msgExitMessageLoop, 0, 0);

      if(0 == postResult)
      {
         hr = HRESULT_FROM_WIN32(GetLastError());  
         ATLTRACE2("%x: ShutdownThreads: Unable to post m_msgExitMessageLoop to the refresh-license thread. %x\n", GetCurrentThreadId(), hr);    
      }
      else
      {
         // There is a refresh-license thread, and we successfully 
         // posted an exit message to the refresh-license thread.
         ATLTRACE2("%x: ShutdownThreads: Successfully posted m_msgExitMessageLoop to the refresh-license thread.\n", GetCurrentThreadId());

         // Add the refresh-license thread handle to the array of handles.
         threadHandles[numThreadHandles] = m_refreshLicenseThreadHandle;
         ++numThreadHandles;
      }     
   }

   // Tell the log-in thread to exit its message loop.
   if(m_loginThreadHandle)
   {
      postResult = 0;
      postResult = PostThreadMessage(m_loginThreadId, m_msgExitMessageLoop, 0, 0);

      if(0 == postResult)
      {
         hr = HRESULT_FROM_WIN32(GetLastError());  
         ATLTRACE2("%x: ShutdownThreads: Unable to post m_msgExitMessageLoop to the log-in thread. %x\n", GetCurrentThreadId(), hr);    
      }
      else
      {
         // There is a log-in thread, and we successfully 
         // posted an exit message to the log-in thread.
         ATLTRACE2("%x: ShutdownThreads: Successfully posted m_msgExitMessageLoop to the log-in thread.\n", GetCurrentThreadId());

         // Add the log-in thread handle to the array of handles.
         threadHandles[numThreadHandles] = m_loginThreadHandle;
         ++numThreadHandles;
      }     
   }

   // Tell the send-message thread to exit its message loop.
   if(m_sendMessageThreadHandle)
   {
      postResult = 0;
      postResult = PostThreadMessage(m_sendMessageThreadId, m_msgExitMessageLoop, 0, 0);

      if(0 == postResult)
      {
         hr = HRESULT_FROM_WIN32(GetLastError());  
         ATLTRACE2("%x: ShutdownThreads: Unable to post m_msgExitMessageLoop to the send-message thread. %x\n", GetCurrentThreadId(), hr);    
      }
      else
      {
         // There is a send-message thread, and we successfully 
         // posted an exit message to the send-message thread.
         ATLTRACE2("%x: ShutdownThreads: Successfully posted m_msgExitMessageLoop to the send-message thread.\n", GetCurrentThreadId());

         // Add the send-message thread handle to the array of handles.
         threadHandles[numThreadHandles] = m_sendMessageThreadHandle;
         ++numThreadHandles;
      }     
   }

   // Tell the update-device thread to exit its message loop.
   if(m_updateDeviceThreadHandle)
   {
      postResult = 0;
      postResult = PostThreadMessage(m_updateDeviceThreadId, m_msgExitMessageLoop, 0, 0);

      if(0 == postResult)
      {
         hr = HRESULT_FROM_WIN32(GetLastError());  
         ATLTRACE2("%x: ShutdownThreads: Unable to post m_msgExitMessageLoop to the update-device thread. %x\n", GetCurrentThreadId(), hr);    
      }
      else
      {
         // There is an update-device thread, and we successfully 
         // posted an exit message to the update-device thread.
         ATLTRACE2("%x: ShutdownThreads: Successfully posted m_msgExitMessageLoop to the update-device thread.\n", GetCurrentThreadId());

         // Add the update-device thread handle to the array of handles.
         threadHandles[numThreadHandles] = m_updateDeviceThreadHandle;
         ++numThreadHandles;
      }     
   }

   // Tell the list thread to exit its message loop.
   if(m_listThreadHandle)
   {
      postResult = 0;
      postResult = PostThreadMessage(m_listThreadId, m_msgExitMessageLoop, 0, 0);

      if(0 == postResult)
      {
         hr = HRESULT_FROM_WIN32(GetLastError());  
         ATLTRACE2("%x: ShutdownThreads: Unable to post m_msgExitMessageLoop to the list thread. %x\n", GetCurrentThreadId(), hr);    
      }
      else
      {
         // There is an list thread, and we successfully 
         // posted an exit message to the list thread.
         ATLTRACE2("%x: ShutdownThreads: Successfully posted m_msgExitMessageLoop to the list thread.\n", GetCurrentThreadId());

         // Add the list thread handle to the array of handles.
         threadHandles[numThreadHandles] = m_listThreadHandle;
         ++numThreadHandles;
      }     
   }



   // If there are any active threads, wait for them to exit.
   if(numThreadHandles)
   {
      DWORD waitResult = WaitForMultipleObjects(
         numThreadHandles,                                   
         threadHandles,                       
         TRUE,
         30000);

      if(WAIT_FAILED == waitResult)
      {
         hr = HRESULT_FROM_WIN32(GetLastError()); 
         ATLTRACE2("%x: ShutdownThreads: WaitForMultipleObjects failed. %x\n", GetCurrentThreadId(), hr);        
         goto cleanup;
      }

      ATLTRACE2("%x: ShutdownThreads: Successfully waited for %d threads to exit.\n", GetCurrentThreadId(), numThreadHandles);     
   }
       

cleanup:

   if(m_downloadThreadHandle)
   {
      CloseHandle(m_downloadThreadHandle);
      m_downloadThreadHandle = 0;
      m_downloadThreadId = 0;
   }

   if(m_buyThreadHandle)
   {
      CloseHandle(m_buyThreadHandle);
      m_buyThreadHandle = 0;
      m_buyThreadId = 0;
   }

   if(m_refreshLicenseThreadHandle)
   {
      CloseHandle(m_refreshLicenseThreadHandle);
      m_refreshLicenseThreadHandle = 0;
      m_refreshLicenseThreadId = 0;
   }

   if(m_loginThreadHandle)
   {
      CloseHandle(m_loginThreadHandle);
      m_loginThreadHandle = 0;
      m_loginThreadId = 0;
   }

   if(m_sendMessageThreadHandle)
   {
      CloseHandle(m_sendMessageThreadHandle);
      m_sendMessageThreadHandle = 0;
      m_sendMessageThreadId = 0;
   }

   if(m_updateDeviceThreadHandle)
   {
      CloseHandle(m_updateDeviceThreadHandle);
      m_updateDeviceThreadHandle = 0;
      m_updateDeviceThreadId = 0;
   }

   if(m_listThreadHandle)
   {
      CloseHandle(m_listThreadHandle);
      m_listThreadHandle = 0;
      m_listThreadId = 0;
   }

   ATLTRACE2("%x: ShutdownThreads: m_downloadThreadHandle has a value of %x.\n", GetCurrentThreadId(), m_downloadThreadHandle);
   ATLTRACE2("%x: ShutdownThreads: m_downloadThreadId has a value of %x.\n", GetCurrentThreadId(), m_downloadThreadId);
   ATLTRACE2("%x: ShutdownThreads: m_buyThreadHandle has a value of %x.\n", GetCurrentThreadId(), m_buyThreadHandle);
   ATLTRACE2("%x: ShutdownThreads: m_buyThreadId has a value of %x.\n", GetCurrentThreadId(), m_buyThreadId);
   ATLTRACE2("%x: ShutdownThreads: m_refreshLicenseThreadHandle has a value of %x.\n", GetCurrentThreadId(), m_refreshLicenseThreadHandle);
   ATLTRACE2("%x: ShutdownThreads: m_refreshLicenseThreadId has a value of %x.\n", GetCurrentThreadId(), m_refreshLicenseThreadId);
   ATLTRACE2("%x: ShutdownThreads: m_loginThreadHandle has a value of %x.\n", GetCurrentThreadId(), m_loginThreadHandle);
   ATLTRACE2("%x: ShutdownThreads: m_loginThreadId has a value of %x.\n", GetCurrentThreadId(), m_loginThreadId);
   ATLTRACE2("%x: ShutdownThreads: m_sendMessageThreadHandle has a value of %x.\n", GetCurrentThreadId(), m_sendMessageThreadHandle);
   ATLTRACE2("%x: ShutdownThreads: m_sendMessageThreadId has a value of %x.\n", GetCurrentThreadId(), m_sendMessageThreadId);
   ATLTRACE2("%x: ShutdownThreads: m_updateDeviceThreadHandle has a value of %x.\n", GetCurrentThreadId(), m_updateDeviceThreadHandle);
   ATLTRACE2("%x: ShutdownThreads: m_updateDeviceThreadId has a value of %x.\n", GetCurrentThreadId(), m_updateDeviceThreadId);

   ATLTRACE2("%x: ShutdownThreads: m_listThreadHandle has a value of %x.\n", GetCurrentThreadId(), m_listThreadHandle);
   ATLTRACE2("%x: ShutdownThreads: m_listThreadId has a value of %x.\n", GetCurrentThreadId(), m_listThreadId);

   return hr;
}