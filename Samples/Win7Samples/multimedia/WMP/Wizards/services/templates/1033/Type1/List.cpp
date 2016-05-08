#include "stdafx.h"
#include "[!output root].h"

////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
//    GetListContents
//
// HandleMessageForListThread function
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////

extern CONTENT_PARTNER_GLOBALS g;

HRESULT STDMETHODCALLTYPE C[!output Safe_root]::GetListContents(
   BSTR location,
   VARIANT *pContext,
   BSTR bstrListType,
   BSTR bstrParams,
   DWORD dwListCookie)
{
   LIST_CONTEXT* pListCtx = NULL; 
   BOOL postResult = 0;
   HRESULT hr = S_OK;

   // If the list thread has not already been started,
   // start it now.
   
   if(0 == m_listThreadHandle)
   {  
      hr = this->StartContentPartnerThread(ThreadTypeList);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: GetListContents: StartContentPartnerThread failed. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      } 
      
      ATLTRACE2("%x: GetListContents: StartContentPartnerThread succeeded.\n", GetCurrentThreadId());     
   }

   // At this point, we khow the list thread started, but
   // we don't know whether it is still active.

   // When we post an list message, we must provide
   // all the information passed to us in the five
   // parameters of this method. So we copy that information
   // into a LIST_CONTEXT structure.
 
   // We must make our own copies of any BSTRs.

   pListCtx = new LIST_CONTEXT();
   // This memory is freed in HandleMessgeForListThread.


   if(NULL == pListCtx)
   {
      ATLTRACE2("%x: GetListContents: Failed to create new LIST_CONTEXT.\n", GetCurrentThreadId());
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   ZeroMemory(pListCtx, sizeof(LIST_CONTEXT)); 

   if(NULL != location)
   {
      pListCtx->location = SysAllocString(location);
      // This memory is freed in HandleMessageForListThread.
   }

   VariantInit( &(pListCtx->context) );

   if(NULL != pContext)
   {
      hr = VariantCopy( &(pListCtx->context), pContext);
      // If this causes any memory to be allocated, the memory is freed
      // by a call to VariantClear in HandleMessageForListThread.

      if(FAILED(hr))
      {
         goto cleanup;
      }
   }

   if(NULL != bstrListType)
   {
      pListCtx->bstrListType = SysAllocString(bstrListType);
      // This memory is freed in HandleMessageForListThread.
   }

   if(NULL != bstrParams)
   {
      pListCtx->bstrParams = SysAllocString(bstrParams);
      // This memory is freed in HandleMessageForListThread.
   }

   pListCtx->dwListCookie = dwListCookie;


   // If the list thread is not active, the following
   // call to PostThreadMessage will fail.

   postResult = PostThreadMessage(
      m_listThreadId,
      m_msgGetListContents,
      0,
      reinterpret_cast<LPARAM>(pListCtx) );

   if(0 == postResult)
   {     
      hr = HRESULT_FROM_WIN32(GetLastError()); 
      ATLTRACE2("%x: GetListContents: PostThreadMessage failed. %x\n", GetCurrentThreadId(), hr);
      goto cleanup;
   }
  
   ATLTRACE2("%x: GetListContents: PostThreadMessage succeeded.\n", GetCurrentThreadId());

   // We successfully posted the message to the list thread.
   // We have no more need for the pointer to the list context.
   pListCtx = NULL;

cleanup:

   if(NULL != pListCtx)
   {
      // We failed to post a message to the list thread.
      // The list thread will not be able to free the memory
      // pointed to by pListCtx.  So we free it here.   

      SysFreeString(pListCtx->location);  // OK to pass NULL. 
      VariantClear( &(pListCtx->context) );  
      SysFreeString(pListCtx->bstrListType);
      SysFreeString(pListCtx->bstrParams);        
      delete pListCtx;
      pListCtx = NULL;
   }

   return hr;

}  // GetListContents


// HandleMessageForListThread runs in the context of the list thread.
HRESULT HandleMessageForListThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback)
{
   LIST_CONTEXT* pListCtx = NULL;
   HRESULT hr = S_OK;

   if(NULL == pMsg || NULL == pThreadCtx || NULL == spCallback)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   if(pThreadCtx->listThreadContext.msgGetListContents == pMsg->message)
   {
      // We must handle this message and free the message context.

      ATLTRACE2("%x: HandleMessageForListThread: Received a message to get list contents.\n", GetCurrentThreadId());
      
      pListCtx = reinterpret_cast<LIST_CONTEXT*>(pMsg->lParam);

      if(NULL == pListCtx)
      {
         hr = E_UNEXPECTED;
         goto cleanup;
      }

      // The sample online store catalog has only one dynamic
      // list. The ID of that list is 8. 

      // There are two scenarios that would lead to this
      // method, GetListContents, being called.
      // 1. The user requests a dynamic list by interacting
      //    with a discovery page, and the discovery page
      //    calls external.changeViewOnlineList.
      // 2. The user requests a dynamic list by interacting 
      //    with Windows Media Player's user interface.


      // Let's see if it's the dynamic list in our catalog.
      BOOL isCatalogList = FALSE;

      if(NULL != pListCtx->location && NULL != pListCtx->bstrListType)
      {
         if( 0 == wcscmp(g_szCPListID, pListCtx->location) && VT_UI4 == pListCtx->context.vt)
         {
            // We have only one dynamic list in our catalog.
            // It is a list of tracks, and its ID is 8.
            if( 0 == wcscmp(g_szCPTrackID, pListCtx->bstrListType) && 8 == pListCtx->context.ulVal )
            {
               isCatalogList = TRUE;
            }
         }
      }

      if(TRUE == isCatalogList)
      {
         DWORD listIDs[] = {0, 3, 4};  
         hr = spCallback->AddListContents(pListCtx->dwListCookie, 3, listIDs);        
         // Ignore bstrParams  
         hr = S_OK;   
      }
      else 
      {
         // See if we can recognize the list by inspecting bstrParams.
         if(NULL != pListCtx->bstrParams && NULL != pListCtx->bstrListType)
         {
            // We have one dynamic list that is not in our catalog.
            // It is a list of tracks, and it is called Sally's Picks.
            // The library.htm discovery page, provided by the sample
            // onlilne store, requests this list by calling
            // external.changeViewOnlineList, passing "songs picked by Sally"
            // in the Params parameter.

            if( 0 == wcscmp(g_szCPTrackID, pListCtx->bstrListType) && 
                0 == wcscmp(L"songs picked by Sally", pListCtx->bstrParams) )
            {
               DWORD sallyListIDs[] = {1, 3};
               spCallback->AddListContents(pListCtx->dwListCookie, 2, sallyListIDs);  
               // Ignore location and pContext.  
               hr = S_OK;      
            }     
         }
      }

      HRESULT resultForListContentsComplete = hr;

      hr = spCallback->ListContentsComplete(pListCtx->dwListCookie, resultForListContentsComplete);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: HandleMessageForListThread: Unable to notify Player that list contents are complete. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      }
         
   } // msgGetListContents

   else
   {
      ATLTRACE2("%x: HandleMessageForListThread: Received unrecognized message. %x\n", GetCurrentThreadId(), pMsg->message);
   }

cleanup:

   if(NULL != pThreadCtx && NULL != pMsg)
   {
      if(pThreadCtx->listThreadContext.msgGetListContents == pMsg->message)
      {
         // The thread that posted this message allocated a LIST_CONTEXT structure.
         // We must free that memory here.

         if(NULL != pListCtx)
         {
            SysFreeString(pListCtx->location);  // OK to pass NULL. 
            VariantClear( &(pListCtx->context) );  
            SysFreeString(pListCtx->bstrListType);
            SysFreeString(pListCtx->bstrParams);        
            delete pListCtx;
            pListCtx = NULL;
         }
      }
   }

   return hr;
}