#include "stdafx.h"
#include "[!output root].h"

////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
//    RefreshLicense method
//
// HandleMessageForRefreshLicenseThread function
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////

      
HRESULT STDMETHODCALLTYPE C[!output Safe_root]::RefreshLicense( 
   DWORD dwCookie,
   VARIANT_BOOL fLocal,
   BSTR bstrURL,
   WMPStreamingType type,
   ULONG contentID,
   BSTR bstrRefreshReason,
   VARIANT *pReasonContext)
{
   REFRESH_LICENSE_CONTEXT* pRefrLicCtx = NULL; 
   BOOL postResult = 0;
   HRESULT hr = S_OK;
   
   if(NULL == bstrURL || NULL == bstrRefreshReason || NULL == pReasonContext)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   // If the refresh-license thread has not already been started,
   // start it now.
   
   if(0 == m_refreshLicenseThreadHandle)
   {  
      hr = this->StartContentPartnerThread(ThreadTypeRefreshLicense);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: RefreshLicense: StartContentPartnerThread failed. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      } 
      
      ATLTRACE2("%x: RefreshLicense: StartContentPartnerThread succeeded.\n", GetCurrentThreadId());     
   }

   // At this point, we khow the refresh-license thread started, but
   // we don't know whether it is still active.

   // When we post a refresh-license message, we must provide
   // all the information we were passed in the seven parameters
   // of this method. So we copy the seven parameters into a
   // REFRESH_LICENSE_CONTEXT structure.

   pRefrLicCtx = new REFRESH_LICENSE_CONTEXT();
   // This memory is freed in HandleMessageForRefreshLicenseThread.

   if(NULL == pRefrLicCtx)
   {
      ATLTRACE2("%x: RefreshLicense: Failed to create new REFRESH_LICENSE_CONTEXT.\n", GetCurrentThreadId());
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   ZeroMemory(pRefrLicCtx, sizeof(REFRESH_LICENSE_CONTEXT));

   pRefrLicCtx->dwCookie = dwCookie;
   pRefrLicCtx->fLocal = fLocal;

   pRefrLicCtx->bstrURL = SysAllocString(bstrURL);
   // This memory is freed in HandleMessageForRefreshLicenseThread. 

   pRefrLicCtx->type = type;
   pRefrLicCtx->contentID = contentID;

   pRefrLicCtx->bstrRefreshReason = SysAllocString(bstrRefreshReason);
   // This memory is freed in HandleMessageForRefreshLicenseThread. 

   VariantInit(pRefrLicCtx->pReasonContext);
   VariantCopy(pRefrLicCtx->pReasonContext, pReasonContext);
   // This memory is freed in HandleMessageForRefreshLicenseThread. 

   // If the refresh-license thread is not active, the following
   // call to PostThreadMessage will fail.

   postResult = PostThreadMessage(
      m_refreshLicenseThreadId,
      m_msgRefreshLicense,
      0,
      reinterpret_cast<LPARAM>(pRefrLicCtx) );

   if(0 == postResult)
   {     
      hr = HRESULT_FROM_WIN32(GetLastError()); 
      ATLTRACE2("%x: RefreshLicense: PostThreadMessage failed. %x\n", GetCurrentThreadId(), hr);
      goto cleanup;
   }
  
   ATLTRACE2("%x: RefreshLicense: PostThreadMessage succeeded.\n", GetCurrentThreadId());

   // We successfully posted the message to the refresh-license thread.
   // We have no more need for the pointer to the refresh-license context.
   pRefrLicCtx = NULL;

   // The refresh-license thread must free the memory 
   // that pRefrLicCtx pointed to previously.

   
cleanup:

   if(pRefrLicCtx)
   {
      // We failed to post a message to the refresh-license thread.
      // The refresh-license thread will not be able to free the memory
      // pointed to by pRefrLicCtx.  So we free it here.
      SysFreeString(pRefrLicCtx->bstrURL);
      SysFreeString(pRefrLicCtx->bstrRefreshReason);
      VariantClear(pRefrLicCtx->pReasonContext);
      delete pRefrLicCtx;
      pRefrLicCtx = NULL;
   }

   // If pRefrLicCtx is NULL, refresh-license thread will free the memory 
   // pointed to by pRefrLicCtx.  

   return hr;
}


// HandleMessageForRefreshLicenseThread runs in the context of the refresh-license thread.
HRESULT HandleMessageForRefreshLicenseThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback)
{
   REFRESH_LICENSE_CONTEXT* pRefLicCtx = NULL;
   HRESULT hr = S_OK;

   if(NULL == pMsg || NULL == pThreadCtx || NULL == spCallback)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   if(pThreadCtx->refreshLicenseThreadContext.msgRefreshLicense == pMsg->message)
   {
      // We must handle this message and free the message context.

      ATLTRACE2("%x: HandleMessageForRefreshLicenseThread: Received message to refresh license.\n", GetCurrentThreadId());
      
      pRefLicCtx = reinterpret_cast<REFRESH_LICENSE_CONTEXT*>(pMsg->lParam);

      if(NULL == pRefLicCtx)
      {
         hr = E_UNEXPECTED;
         goto cleanup;
      }

      // Simulate lengthy operation by sleeping two seconds.
      SleepEx(2000, FALSE);

      // Tell Windows Media Player that we have finished refreshing the license.
      hr = spCallback->RefreshLicenseComplete(
         pRefLicCtx->dwCookie, 
         pRefLicCtx->contentID, 
         S_OK);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: HandleMessageForRefreshLicenseThread: Failed to notify Windows Media Player that a license refresh is complete. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      }
   } // refresh-license message

   else
   {
      ATLTRACE2("%x: HandleMessageForRefreshLicenseThread: Received unrecognized message. %x\n", GetCurrentThreadId(), pMsg->message);
   }

cleanup:

   if(NULL != pThreadCtx && NULL != pMsg)
   {
      if(pThreadCtx->refreshLicenseThreadContext.msgRefreshLicense == pMsg->message)
      {
         // The thread that posted this message allocated a
         // REFRESH_LICENSE_CONTEXT structure.
         // We must free that memory here.

         if(NULL != pRefLicCtx)
         {
            SysFreeString(pRefLicCtx->bstrURL);
            SysFreeString(pRefLicCtx->bstrRefreshReason);
            VariantClear(pRefLicCtx->pReasonContext);
            delete pRefLicCtx;
            pRefLicCtx = NULL;
         }
      }
   }

   return hr;
}