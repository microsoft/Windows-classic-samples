#include "stdafx.h"
#include "[!output Safe_root].h"

////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
//    UpdateDevice
//
// HandleMessageForUpdateDeviceThread function
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////

extern CONTENT_PARTNER_GLOBALS g;

HRESULT STDMETHODCALLTYPE C[!output Safe_root]::UpdateDevice(
   BSTR bstrDeviceName)
{
   UPDATE_DEVICE_CONTEXT* pUpDevCtx = NULL; 
   BOOL postResult = 0;
   HRESULT hr = S_OK;
   
   if(NULL == bstrDeviceName)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   // If the update-device thread has not already been started,
   // start it now.
   
   if(0 == m_updateDeviceThreadHandle)
   {  
      hr = this->StartContentPartnerThread(ThreadTypeUpdateDevice);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: UpdateDevice: StartContentPartnerThread failed. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      } 
      
      ATLTRACE2("%x: UpdateDevice: StartContentPartnerThread succeeded.\n", GetCurrentThreadId());     
   }

   // At this point, we khow the update-device thread started, but
   // we don't know whether it is still active.

   // When we post an update-device message, we must provide
   // the name of the device to be updated.
   // Windows Media Player passed the device name to us in the
   // bstrDeviceName parameter of this method.
 
   // We must make our own copy of the device name. We will store a pointer
   // to our copy of the device name in an UPDATE_DEVICE_CONTEXT structure.

   pUpDevCtx = new UPDATE_DEVICE_CONTEXT();
   // This memory is freed in HandleMessgeForUpdateDeviceThread.


   if(NULL == pUpDevCtx)
   {
      ATLTRACE2("%x: UpdateDevice: Failed to create new UPDATE_DEVICE_CONTEXT.\n", GetCurrentThreadId());
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   ZeroMemory(pUpDevCtx, sizeof(UPDATE_DEVICE_CONTEXT)); 

   // Make a copy of the device name. Store a pointer to the new
   // copy in our UPDATE_DEVICE_CONTEXT structure.

   pUpDevCtx->bstrDeviceName = SysAllocString(bstrDeviceName);
   // This new BSTR is be freed in HandleMessageForUpdateDeviceThread.

   if(NULL == pUpDevCtx->bstrDeviceName)
   {
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   // If the update-device thread is not active, the following
   // call to PostThreadMessage will fail.

   postResult = PostThreadMessage(
      m_updateDeviceThreadId,
      m_msgUpdateDevice,
      0,
      reinterpret_cast<LPARAM>(pUpDevCtx) );

   if(0 == postResult)
   {     
      hr = HRESULT_FROM_WIN32(GetLastError()); 
      ATLTRACE2("%x: UpdateDevice: PostThreadMessage failed. %x\n", GetCurrentThreadId(), hr);
      goto cleanup;
   }
  
   ATLTRACE2("%x: UpdateDevice: PostThreadMessage succeeded.\n", GetCurrentThreadId());

   // We successfully posted the message to the update-device thread.
   // We have no more need for the pointer to the update-device context.
   pUpDevCtx = NULL;

cleanup:

   if(NULL != pUpDevCtx)
   {
      // We failed to post a message to the update-device thread.
      // The update-device thread will not be able to free the memory
      // pointed to by pUpDevCtx.  So we free it here.   

      SysFreeString(pUpDevCtx->bstrDeviceName);  // OK to pass NULL.  
      delete pUpDevCtx;
      pUpDevCtx = NULL;
   }

   return hr;
}  // UpdateDevice


// HandleMessageForUpdateDeviceThread runs in the context of the update-device thread.
HRESULT HandleMessageForUpdateDeviceThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback)
{
   UPDATE_DEVICE_CONTEXT* pUpDevCtx = NULL;
   HRESULT hr = S_OK;

   if(NULL == pMsg || NULL == pThreadCtx || NULL == spCallback)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   if(pThreadCtx->updateDeviceThreadContext.msgUpdateDevice == pMsg->message)
   {
      // We must handle this message and free the message context.

      ATLTRACE2("%x: HandleMessageForUpdateDeviceThread: Received a message to update a device.\n", GetCurrentThreadId());
      
      pUpDevCtx = reinterpret_cast<UPDATE_DEVICE_CONTEXT*>(pMsg->lParam);

      if(NULL == pUpDevCtx)
      {
         hr = E_UNEXPECTED;
         goto cleanup;
      }

      if(NULL == pUpDevCtx->bstrDeviceName)
      {
        hr = E_UNEXPECTED;
        goto cleanup;
        // Revisit
      }

      ATLTRACE2("%x: HandleMessageForUpdateDeviceThread: Canonical device name is %s.\n", GetCurrentThreadId(), pUpDevCtx->bstrDeviceName);

      // ToDo: Do whatever work needs to be done with the device.

      // Simulate a length operation by sleeping two seconds.
      SleepEx(2000, FALSE);

      // Notify Windows Media Player that we are finished working with the device.

      hr = spCallback->UpdateDeviceComplete(pUpDevCtx->bstrDeviceName);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: HandleMessageForUpdateDeviceThred: Failed to notify Player that a device update is complete. %x\n", GetCurrentThreadId(), hr);
         goto cleanup; 
      }
      
   } // updateDevice message

   else
   {
      ATLTRACE2("%x: HandleMessageUpdateDeviceThread: Received unrecognized message. %x\n", GetCurrentThreadId(), pMsg->message);
   }

cleanup:

   if(NULL != pThreadCtx && NULL != pMsg)
   {
      if(pThreadCtx->updateDeviceThreadContext.msgUpdateDevice == pMsg->message)
      {
         // The thread that posted this message allocated an
         // UPDATE_DEVICE_CONTEXT structure.
         // We must free that memory here.

         if(NULL != pUpDevCtx)
         {
            SysFreeString(pUpDevCtx->bstrDeviceName);  // OK to pass NULL.           
            delete pUpDevCtx;
            pUpDevCtx = NULL;
         }
      }
   }

   return hr;
}