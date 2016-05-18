
#include "stdafx.h"
#include "[!output root].h"

////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
//    CanBuySilent method
//    Buy method
//    CompareContainerListPrices method
//
// HandleMessageForBuyThread function
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////


HRESULT BuyMessageLoop(
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx,
   CComPtr<IWMPContentPartnerCallback> spCallback);

        
HRESULT STDMETHODCALLTYPE C[!output Safe_root]::CanBuySilent( 
   IWMPContentContainerList *pInfo,
   BSTR *pbstrTotalPrice,
   VARIANT_BOOL *pSilentOK)
{
   HRESULT hr = S_OK;

   // Initialize output parameters in case we fail.

   if(pbstrTotalPrice)
   {
      *pbstrTotalPrice = NULL;
   }

   if(pSilentOK)
   {
      *pSilentOK = VARIANT_FALSE;
   }


   if(NULL == pInfo || NULL == pbstrTotalPrice || NULL == pSilentOK)
   {
      return E_INVALIDARG;
   }

   ATLTRACE2("%x: CanBuySilent\n", GetCurrentThreadId());

   // ToDo: Determine whether this content can be purchased silently.

   *pbstrTotalPrice = SysAllocString(L"12 apples");

   if(NULL == *pbstrTotalPrice)
   {
      hr = E_OUTOFMEMORY;
   }
   
   *pSilentOK = VARIANT_TRUE; // or VARIANT_FALSE

   return hr;
}

       
HRESULT STDMETHODCALLTYPE C[!output Safe_root]::Buy( 
   IWMPContentContainerList *pInfo,
   DWORD cookie)
{
   BUY_CONTEXT* pBuyCtx = NULL; 
   BOOL postResult = FALSE;
   HRESULT hr = S_OK;
   
   if(NULL == pInfo)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   // If the buy thread has not already been started,
   // start it now.
   
   if(!m_buyThreadHandle)
   {  
      hr = this->StartContentPartnerThread(ThreadTypeBuy);
      if(FAILED(hr))
      {
         ATLTRACE2("%x: Buy: StartContentPartnerThread failed. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      } 
      
      ATLTRACE2("%x: Buy: StartContentPartnerThread succeeded.\n", GetCurrentThreadId());    
   }

   // At this point, we know the buy thread started, but
   // we don't know whether it is still active.

   // When we post a buy message, we must provide
   // two things: a cookie that represents the purchase and an
   // IStream interface that the buy thread can use to
   // obtain an IWMPContentContainerList interface. Those two
   // things are the members of BUY_CONTEXT.

   pBuyCtx = new BUY_CONTEXT();

   if(NULL == pBuyCtx)
   {
      ATLTRACE2("%x: Buy: Failed to create new BUY_CONTEXT.\n", GetCurrentThreadId());
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   ZeroMemory(pBuyCtx, sizeof(BUY_CONTEXT));
   
   hr = CoMarshalInterThreadInterfaceInStream(
      __uuidof(IWMPContentContainerList),
      pInfo,
      &pBuyCtx->pIStream); 

   if(FAILED(hr))
   {
      ATLTRACE2("%x: Buy: Failed to marshal IWMPContentContainerList interface. %x\n", GetCurrentThreadId(), hr);
      goto cleanup;
   } 

   if(NULL == pBuyCtx->pIStream)
   {
      hr = E_UNEXPECTED;
      goto cleanup;
   }

   ATLTRACE2("%x: Buy: Successfully marshaled IWMPContentContainerList interface.\n", GetCurrentThreadId());
  
   pBuyCtx->cookie = cookie;

   // If the buy thread is not active, the following
   // call to PostThreadMessage will fail.

   postResult = PostThreadMessage(
      m_buyThreadId,
      m_msgBuy,
      0,
      reinterpret_cast<LPARAM>(pBuyCtx) );

   if(0 == postResult)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ATLTRACE2("%x: Buy: PostThreadMessage failed. %x\n", GetCurrentThreadId(), hr);        
      goto cleanup;
   }
   
   ATLTRACE2("%x: Buy: PostThreadMessage %x succeeded.\n", GetCurrentThreadId());

   // We successfully posted the message to the buy thread.
   // We have no more need for the pointer to the buy context.
   pBuyCtx = NULL;

   // The buy thread must free the memory pointed to by pBuyCtx.

cleanup:

   if(pBuyCtx)
   {
      // We failed to post a message to the buy thread.
      // The buy thread will not be able to free the memory
      // pointed to by pBuyCtx.  So we free it here.      
      delete pBuyCtx;
      pBuyCtx = NULL;
   }

   // If pBuyCtx is NULL, buy thread will free the memory 
   // pointed to by pBuyCtx.  

   return hr;
}


HRESULT STDMETHODCALLTYPE C[!output Safe_root]::CompareContainerListPrices( 
   IWMPContentContainerList *pListBase,
   IWMPContentContainerList *pListCompare,
   long *pResult)
{
   // Initialize output parameter in case we fail.
   if(pResult)
   {
      *pResult = 0;
   }


   if(NULL == pListBase || NULL == pListCompare || NULL == pResult)
   {
      return E_INVALIDARG;
   }

   ATLTRACE2("%x: CompareContainerListPrices\n", GetCurrentThreadId());

   // ToDo: Compare the prices of the two container lists.

   *pResult = 0; // or -1 or 1

   return S_OK;
}

// HandleMessageForBuyThread runs in the context of the buy thread.
HRESULT HandleMessageForBuyThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback)
{
   BUY_CONTEXT* pBuyCtx = NULL;
   HRESULT hr = S_OK; 
   CComPtr<IWMPContentContainerList> spContainerList = NULL;
   ULONG numContainers = 0;

   if(NULL == pMsg || NULL == pThreadCtx || NULL == spCallback)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   if(pThreadCtx->buyThreadContext.msgBuy == pMsg->message)
   {
      // We must handle this message and free the message context.

      ATLTRACE2("%x: HandleMessageForBuyThread: Received message to buy.\n", GetCurrentThreadId());     

      pBuyCtx = reinterpret_cast<BUY_CONTEXT*>(pMsg->lParam);

      if(NULL == pBuyCtx)
      {
         hr = E_UNEXPECTED;
         goto cleanup;
      }

      if(NULL == pBuyCtx->pIStream)
      {
         hr = E_UNEXPECTED;
         goto cleanup;
      }

      // Get a pointer to an IWMPContentContainerList interface.

      hr = CoGetInterfaceAndReleaseStream(
         pBuyCtx->pIStream,
         __uuidof(IWMPContentContainerList),
         reinterpret_cast<LPVOID*>(&spContainerList) );

      // The stream was released (even if CoGetInterfaceAndReleaseStream failed). 
      // Set the stream pointer to NULL.
      pBuyCtx->pIStream = NULL;

      if(FAILED(hr))
      {
         ATLTRACE2("%x: HandleMessageForBuyThread: Failed to get IWMPContentContainerList interface. %x\n", GetCurrentThreadId(), hr);     
         goto cleanup;
      }

      if(NULL == spContainerList)
      {
         hr = E_UNEXPECTED;
         goto cleanup;
      }

      ATLTRACE2("%x: HandleMessageForBuyThread: Successfully obtained IWMPContentContainerList interface.\n", GetCurrentThreadId());
         
      hr = spContainerList->GetContainerCount(&numContainers);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: HandleMessageForBuyThread: GetContainerCount failed. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      }

      ATLTRACE2("%x: HandleMessageForBuyThread: numContainers = %d.\n", GetCurrentThreadId(), numContainers);

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

         for(ULONG k = 0; k < numItems; ++k)
         {
            ULONG itemID = 0;

            hr = spContainer->GetContentID(k, &itemID);

            if(FAILED(hr))
            {
               break;  // Break out of the for-k loop.
            }

            // ToDo: Update records to reflect the purchase of this item.

            ATLTRACE2("%x: HandleMessageForBuyThread: Buying item %d.\n",GetCurrentThreadId(), itemID);

         } // for k
      } // for j   

      // Tell Windows Media Player that we have finished 
      // processing the purchase request.

      hr = spCallback->BuyComplete(S_OK, pBuyCtx->cookie);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: HandleMessageForBuyThread: Failed to notify Windows Media Player that a purchase is complete. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      }
   } // buy message

   else
   {
      ATLTRACE2("%x: HandleMessageForBuyThread: Received unrecognized message. %x\n", GetCurrentThreadId(), pMsg->message);
   }
    
cleanup:

   if(NULL != pThreadCtx && NULL != pMsg)
   {

      if(pThreadCtx->buyThreadContext.msgBuy == pMsg->message)
      {
         // The thread that posted this message allocated a 
         // BUY_CONTEXT structure.
         // We must free that memory here.

         if(NULL != pBuyCtx)
         {
            if(NULL != pBuyCtx->pIStream)
            {
               pBuyCtx->pIStream->Release();
               pBuyCtx->pIStream = NULL;
            }

            delete pBuyCtx;
            pBuyCtx = NULL;
         }
      }
   }
   return hr;
}