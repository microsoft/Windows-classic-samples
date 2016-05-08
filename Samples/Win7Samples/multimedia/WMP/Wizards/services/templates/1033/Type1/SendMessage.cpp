#include "stdafx.h"
#include "[!output Safe_root].h"

////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
//    SendMessage method
//
// HandleMessageForSendMessageThread function
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////

extern CONTENT_PARTNER_GLOBALS g;

HRESULT STDMETHODCALLTYPE C[!output Safe_root]::SendMessage( 
   BSTR bstrMsg,
   BSTR bstrParam)
{
   SEND_MESSAGE_CONTEXT* pSendMsgCtx = NULL; 
   BOOL postResult = 0;
   HRESULT hr = S_OK;
   
   if(NULL == bstrMsg || NULL == bstrParam)  // OK for bstrParam to be empty, but not NULL
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   // If the send-message thread has not already been started,
   // start it now.
   
   if(0 == m_sendMessageThreadHandle)
   {  
      hr = this->StartContentPartnerThread(ThreadTypeSendMessage);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: SendMessage: StartContentPartnerThread failed. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      } 
      
      ATLTRACE2("%x: SendMessage: StartContentPartnerThread succeeded.\n", GetCurrentThreadId());     
   }

   // At this point, we khow the send-message thread started, but
   // we don't know whether it is still active.

   // When we post a send-message message, we must provide
   // the information passed in the two parameters of this method. 
   // Those two parameters, bstrMsg and bstrParm, are BSTRs allocated
   // by the caller.
   // We must make our own copies of the two BSTRs.
   // We will store pointers to the copies in a SEND_MESSAGE_CONTEXT structure.

   pSendMsgCtx = new SEND_MESSAGE_CONTEXT();
   // This memory is freed in HandleMessgeForSendMessageThread.


   if(NULL == pSendMsgCtx)
   {
      ATLTRACE2("%x: SendMessage: Failed to create new SEND_MESSAGE_CONTEXT.\n", GetCurrentThreadId());
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   ZeroMemory(pSendMsgCtx, sizeof(SEND_MESSAGE_CONTEXT));

   // Make copies of bstrMsg and bstrParam.
   // Store pointer to the copies in our SEND_MESSAGE_CONTEXT strucuter.

   pSendMsgCtx->bstrMsg = SysAllocString(bstrMsg);
   pSendMsgCtx->bstrParam = SysAllocString(bstrParam);
   // Thes new BSTRs are freed in HandleMessageForSendMessageThread.

   if(NULL == pSendMsgCtx->bstrMsg || NULL == pSendMsgCtx->bstrParam)
   {
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   // If the send-message thread is not active, the following
   // call to PostThreadMessage will fail.

   postResult = PostThreadMessage(
      m_sendMessageThreadId,
      m_msgSendMessage,
      0,
      reinterpret_cast<LPARAM>(pSendMsgCtx) );

   if(0 == postResult)
   {     
      hr = HRESULT_FROM_WIN32(GetLastError()); 
      ATLTRACE2("%x: SendMessage: PostThreadMessage failed. %x\n", GetCurrentThreadId(), hr);
      goto cleanup;
   }
  
   ATLTRACE2("%x: SendMessage: PostThreadMessage succeeded.\n", GetCurrentThreadId());

   // We successfully posted the message to the send-message thread.
   // We have no more need for the pointer to the send-message context.
   pSendMsgCtx = NULL;

cleanup:

   if(NULL != pSendMsgCtx)
   {
      // We failed to post a message to the send-message thread.
      // The send-message thread will not be able to free the memory
      // pointed to by pSendMsgCtx.  So we free it here.   

      SysFreeString(pSendMsgCtx->bstrMsg);  // OK to pass NULL.
      SysFreeString(pSendMsgCtx->bstrParam);   
      delete pSendMsgCtx;
      pSendMsgCtx = NULL;
   }

   return hr;
}  // SendMessage

// HandleMessageForSendMessageThread runs in the context of the send-message thread.
HRESULT HandleMessageForSendMessageThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback)
{
   SEND_MESSAGE_CONTEXT* pSendMsgCtx = NULL;
   HRESULT hr = S_OK;

   if(NULL == pMsg || NULL == pThreadCtx || NULL == spCallback)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   if(pThreadCtx->sendMessageThreadContext.msgSendMessage == pMsg->message)
   {
      // We must handle this message and free the message context.

      ATLTRACE2("%x: HandleMessageForSendMessageThread: Received a send-message message.\n", GetCurrentThreadId());
      
      pSendMsgCtx = reinterpret_cast<SEND_MESSAGE_CONTEXT*>(pMsg->lParam);

      if(NULL == pSendMsgCtx)
      {
         hr = E_UNEXPECTED;
         goto cleanup;
      }

      VARIANT loginResult;
      VariantInit(&loginResult);

      if(NULL == pSendMsgCtx->bstrMsg || NULL == pSendMsgCtx->bstrParam)
      {
        hr = E_UNEXPECTED;
        goto cleanup;
      }

      if( 0 == wcscmp(L"AltLogin", pSendMsgCtx->bstrMsg) && 0 == wcscmp(L"success", pSendMsgCtx->bstrParam) )
      {
         InterlockedExchange( &(g.userLoggedIn), VARIANT_TRUE);
         loginResult.vt = VT_BOOL;
         loginResult.boolVal = VARIANT_TRUE;
     
         hr = spCallback->Notify(wmpcnLoginStateChange, &loginResult);

         if(FAILED(hr))
         {
            ATLTRACE2("%x: HandleMessageForSendMessageThread: Failed to notify Player of successful login. %x\n", GetCurrentThreadId(), hr);
         }

         hr = spCallback->SendMessageComplete(pSendMsgCtx->bstrMsg, pSendMsgCtx->bstrParam, NULL);

         if(FAILED(hr))
         {
            ATLTRACE2("%x: HandleMessageForSendMessageThred: Failed to notify Player that message processing is complete. %x\n", GetCurrentThreadId(), hr);
            goto cleanup; 
         }
      } // alternative log-in message

      else if( 0 == wcscmp(L"GetPluginVersion", pSendMsgCtx->bstrMsg) )
      {
         BSTR result = SysAllocString(L"2.0");

         hr = spCallback->SendMessageComplete(pSendMsgCtx->bstrMsg, pSendMsgCtx->bstrParam, result);

         if(FAILED(hr))
         {
            ATLTRACE2("%x: HandleMessageForSendMessageThread: Failed to notify Player that message processing is complete. %x\n", GetCurrentThreadId(), hr);
            goto cleanup;
         }

         SysFreeString(result);
      } // Get-plug-in-version message

      else
      {
         // A discovery page sent us a message that we don't recognize.
      }

   } // send-message message

   else
   {
      ATLTRACE2("%x: HandleMessageForSendMessageThread: Received unrecognized message. %x\n", GetCurrentThreadId(), pMsg->message);
   }

cleanup:

   if(NULL != pThreadCtx && NULL != pMsg)
   {
      if(pThreadCtx->sendMessageThreadContext.msgSendMessage == pMsg->message)
      {
         // The thread that posted this message allocated a
         // SEND_MESSAGE_CONTEXT structure.
         // We must free that memory here.

         if(NULL != pSendMsgCtx)
         {
            SysFreeString(pSendMsgCtx->bstrMsg);  // OK to pass NULL.
            SysFreeString(pSendMsgCtx->bstrParam);    
            delete pSendMsgCtx;
            pSendMsgCtx = NULL;
         }
      }
   }

   return hr;
}