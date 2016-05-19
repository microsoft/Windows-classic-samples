#include "stdafx.h"
#include "[!output root].h"

////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
//    Download method
//    DownloadTrackComplete method
//
// HandleMessageForDownloadThread function
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////


extern CONTENT_PARTNER_GLOBALS g;


HRESULT STDMETHODCALLTYPE C[!output Safe_root]::Download( 
   IWMPContentContainerList *pInfo,
   DWORD cookie)
{  
   DOWNLOAD_BATCH_CONTEXT* pBatchCtx = NULL; 
   BOOL postResult = FALSE;
   HRESULT hr = S_OK;
   
   if(NULL == pInfo)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   // If the download thread has not already been started,
   // start it now.
   
   if(0 == m_downloadThreadHandle)
   {  
      hr = this->StartContentPartnerThread(ThreadTypeDownload);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: Download: StartThread failed. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      } 
     
      ATLTRACE2("%x: Download: StartThread succeeded.\n", GetCurrentThreadId());
   }

   // At this point, we know the download thread started, but
   // we don't know whether it is still active.

   // When we post a download-batch message, we must provide
   // two things: a cookie that represents the batch and an
   // IStream interface that the download thread can use to
   // obtain an IWMPContentContainerList interface. Those two
   // things are the members of DOWNLOAD_BATCH_CONTEXT.

   pBatchCtx = new DOWNLOAD_BATCH_CONTEXT();
   // This memory is freed in HandleMessageForDownloadThread.


   if(NULL == pBatchCtx)
   {
      ATLTRACE2("%x: Download: Failed to create new DOWNLOAD_BATCH_CONTEXT.\n", GetCurrentThreadId());
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   ZeroMemory(pBatchCtx, sizeof(DOWNLOAD_BATCH_CONTEXT));
   
   hr = CoMarshalInterThreadInterfaceInStream(
      __uuidof(IWMPContentContainerList),
      pInfo,
      &pBatchCtx->pIStream); 

   if(FAILED(hr))
   {
      ATLTRACE2("%x: Download: Failed to marshal IWMPContentContainerList interface. %x\n", GetCurrentThreadId(), hr);
      goto cleanup;
   } 

   if(NULL == pBatchCtx->pIStream)
   {
      hr = E_UNEXPECTED;
      goto cleanup;
   }

   ATLTRACE2("%x: Download: Successfully marshaled IWMPContentContainerList interface.\n", GetCurrentThreadId());
  
   pBatchCtx->cookie = cookie;

   // If the download thread is not active, the following
   // call to PostThreadMessage will fail. 

   postResult = PostThreadMessage(
      m_downloadThreadId,
      m_msgDownloadBatch,
      0,
      reinterpret_cast<LPARAM>(pBatchCtx) );

   if(0 == postResult)
   {    
      hr = HRESULT_FROM_WIN32(GetLastError());  
      ATLTRACE2("%x: Download: PostThreadMessage failed. %x\n", GetCurrentThreadId(), hr);
      goto cleanup;
   }
   
   ATLTRACE2("%x: Download: PostThreadMessage succeeded.\n", GetCurrentThreadId());

   // We successfully posted the message to the download thread.
   // We have no more need for the pointer to the batch context.
   pBatchCtx = NULL;

cleanup:

   if(NULL != pBatchCtx)
   {
      // We failed to post a message to the download thread.
      // The download thread will not be able to free the memory
      // pointed to by pBatchCtx.  So we free it here.      
      delete pBatchCtx;
      pBatchCtx = NULL;
   }

   return hr;
} // Download

        
HRESULT STDMETHODCALLTYPE C[!output Safe_root]::DownloadTrackComplete( 
   HRESULT hrResult,
   ULONG contentID,
   BSTR /*downloadTrackParam*/)
{
   if(FAILED(hrResult))
   {
      ATLTRACE2("%x: DownloadTrackComplete: Windows Media Player failed to download track %d.\n", GetCurrentThreadId(), contentID);
      ++g.totalDownloadFailures;
   }
   else
   {
      ATLTRACE2("%x: DownloadTrackComplete: Windows Media Player has completed the download for track %d.\n", GetCurrentThreadId(), contentID);
   }

   return S_OK;
} // DownloadTrackComplete



// HandleMessageForDownloadThread runs in the context of the download thread.
HRESULT HandleMessageForDownloadThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback)
{
   DOWNLOAD_BATCH_CONTEXT* pBatchCtx = NULL;
   CComPtr<IWMPContentContainerList> spContainerList;
   ULONG numContainers = 0;
   ULONG availableUrlStrings = sizeof(g_tracks)/sizeof(g_tracks[0]);
   HRESULT hr = S_OK;

   if(NULL == pMsg || NULL == pThreadCtx || NULL == spCallback)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   if(pThreadCtx->downloadThreadContext.msgDownloadBatch == pMsg->message)
   {
      // We must handle this message and free the message context.

      ATLTRACE2("%x: HandleMessageForDownloadThread: Received message to download a batch of media items.\n", GetCurrentThreadId());          

      pBatchCtx = reinterpret_cast<DOWNLOAD_BATCH_CONTEXT*>(pMsg->lParam);

      if(NULL == pBatchCtx->pIStream)
      {
         hr = E_UNEXPECTED;        
         goto cleanup;
      }   

      // Get a pointer to an IWMPContentContainerList interface.

      hr = CoGetInterfaceAndReleaseStream(
         pBatchCtx->pIStream,
         __uuidof(IWMPContentContainerList),
         reinterpret_cast<LPVOID*>(&spContainerList) );

      // The stream was released (even if CoGetInterfaceAndReleaseStream failed). 
      // Set the stream pointer to NULL.
      pBatchCtx->pIStream = NULL;

      if(FAILED(hr))
      {
         ATLTRACE2("%x: HandleMessageForDownloadThread: Failed to get IWMPContentContainerList interface. %x\n", GetCurrentThreadId(), hr);          
         goto cleanup;
      }

      if(NULL == spContainerList)
      {
         hr = E_UNEXPECTED;      
         goto cleanup;
      }

      ATLTRACE2("%x: HandleMessageForDownloadThread: Successfully obtained IWMPContentContainerList interface.\n", GetCurrentThreadId());
     
      hr = spContainerList->GetContainerCount(&numContainers);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: HandleMessageForDownloadThread: GetContainerCount failed. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      }

      ATLTRACE2("%x: HandleMessageForDownloadThread: numContainers = %d.\n", GetCurrentThreadId(), numContainers);       
  
      for(ULONG j = 0; j < numContainers; ++j)
      {
         ULONG numItems = 0;
         CComPtr<IWMPContentContainer> spContainer;

         hr = spContainerList->GetContainer(j, &spContainer);

         if(FAILED(hr))
         {
            break;  // Break out of the for-j loop.
         }
            
         hr = spContainer->GetContentCount(&numItems); 

         if(FAILED(hr))
         {
            // Make sure we don't enter the for-k loop.
            numItems = 0;
         }      
                
         ATLTRACE2("%x: HandleMessageForDownloadThread: Container has %d items.\n", GetCurrentThreadId(), numItems);

         for(ULONG k = 0; k < numItems; ++k)
         {
            ULONG itemID = 0;
            HRESULT hrDownload = S_OK;
            BSTR bstrUrl = NULL;
            WCHAR url[INTERNET_MAX_URL_LENGTH] = L"";
   
            hr = spContainer->GetContentID(k, &itemID);

            if(FAILED(hr))
            {
               break;  // Break out of the for-k loop.
               // This means we won't call DownloadTrack.
            }

            wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_audioRootURL);
                  
            if(itemID < availableUrlStrings)
            {    
               wcscat_s(url, INTERNET_MAX_URL_LENGTH, g_tracks[itemID]);
            }
            else
            {
               wcscat_s(url, INTERNET_MAX_URL_LENGTH, g_placeholderTrack);       
            }

            bstrUrl = SysAllocString(url);

            if(NULL == bstrUrl)
            {
               hrDownload = E_OUTOFMEMORY;
            }                

            // Tell Windows Media Player to download this one track (or not).
            hr = spCallback->DownloadTrack(
               pBatchCtx->cookie, 
               bstrUrl,
               itemID, 
               NULL, 
               hrDownload);

            // Ignore hr.

            SysFreeString(bstrUrl);

         } // for k
      } // for j  
   } // download message

   else
   {
      ATLTRACE2("%x: HandleMessageForDownloadThread: Received unrecognized message. &x\n", GetCurrentThreadId(), pMsg->message);
   }

cleanup:

   if(NULL != pThreadCtx && NULL != pMsg)
   {
      if(pThreadCtx->downloadThreadContext.msgDownloadBatch == pMsg->message)
      {
         // The thread that posted this message allocated a 
         // DOWNLOAD_BATCH_CONTEXT structure. We must free that
         // memory here.

         if(NULL != pBatchCtx)
         {
            if(NULL != pBatchCtx->pIStream)
            {
               // For some reason, CoGetInterfaceAndReleaseStream never got called.
               pBatchCtx->pIStream->Release();
               pBatchCtx->pIStream = NULL;
            }
     
            delete pBatchCtx;
            pBatchCtx = NULL;
         } 
      }
   }

   return hr;    
     
} // HandleMessageForDownloadThread