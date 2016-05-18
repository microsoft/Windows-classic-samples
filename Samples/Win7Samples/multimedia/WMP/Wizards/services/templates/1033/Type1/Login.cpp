#include "stdafx.h"
#include "[!output root].h"

////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
//    Login method
//    Logout method
//    Authenticate method
//    VerifyPermission method
//
// CacheCredentials function
// DeleteCachedCredentials function
// HaveCachedCredentials function
// LoginUsingCachedCredentials function
// LoginUsingSuppliedCredentials function
// HandleMessageForLoginThread function
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////

extern CONTENT_PARTNER_GLOBALS g;

HRESULT LoginUsingCachedCredentials(CComPtr<IWMPContentPartnerCallback> spCallback);
HRESULT LoginUsingSuppliedCredentials(LOGIN_CONTEXT* pLoginCtx, CComPtr<IWMPContentPartnerCallback> spCallback);
HRESULT DeleteCachedCredentials();


HRESULT STDMETHODCALLTYPE C[!output Safe_root]::Login( 
   BLOB userInfo,
   BLOB pwdInfo,
   VARIANT_BOOL fUsedCachedCreds,
   VARIANT_BOOL fOkToCache)
{
   LOGIN_CONTEXT* pLoginCtx = NULL; 
   BOOL postResult = 0;
   HRESULT hr = S_OK;
   errno_t copyResult = 0;

   if
   (  
      VARIANT_FALSE == fUsedCachedCreds && 
      (
         NULL == userInfo.pBlobData || 
         userInfo.cbSize <= 0       ||
         NULL == pwdInfo.pBlobData  || 
         pwdInfo.cbSize <= 0
      ) 
   )
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   // If the login thread has not already been started,
   // start it now.
   
   if(0 == m_loginThreadHandle)
   {  
      hr = this->StartContentPartnerThread(ThreadTypeLogin);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: Login: StartContentPartnerThread failed. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      } 
      
      ATLTRACE2("%x: Login: StartContentPartnerThread succeeded.\n", GetCurrentThreadId());     
   }

   // At this point, we know the login thread has started, but
   // we don't know whether it is still active.

   // When we post a login message, we must provide
   // all the information we were passed in the four parameters
   // of this method. So we copy the four parameters into a
   // LOGIN_CONTEXT structure.

   pLoginCtx = new LOGIN_CONTEXT();
   // This memory is freed in HandleMessageForLoginThread.


   if(NULL == pLoginCtx)
   {
      ATLTRACE2("%x: Login: Failed to create new LOGIN_CONTEXT.\n", GetCurrentThreadId());
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   ZeroMemory(pLoginCtx, sizeof(LOGIN_CONTEXT));

   pLoginCtx->fUsedCachedCreds = fUsedCachedCreds;
   pLoginCtx->fOkToCahce = fOkToCache;

   if(VARIANT_FALSE == fUsedCachedCreds)
   {
      // We must rely on credentials suppllied in userInfo and pwdInfo.

      // Copy the username BLOB. 
      // This includes making our own copy of the blob data.
      // HandleMessageForLoginThread will free our copy of the data.
      pLoginCtx->userInfo.cbSize = userInfo.cbSize;
      pLoginCtx->userInfo.pBlobData = reinterpret_cast<BYTE*>(malloc(userInfo.cbSize));

      if(NULL == pLoginCtx->userInfo.pBlobData)
      {
         hr = E_OUTOFMEMORY;
         goto cleanup;
      } 

      copyResult = memcpy_s(pLoginCtx->userInfo.pBlobData, userInfo.cbSize, userInfo.pBlobData, userInfo.cbSize);

      if(0 != copyResult)
      {
         hr = HRESULT_FROM_WIN32(copyResult);
         goto cleanup;
      }

      // Copy the password BLOB. 
      // This includes making our own copy of the blob data.
      // HandleMessageForLoginThread will free our copy of the data.
      pLoginCtx->pwdInfo.cbSize = pwdInfo.cbSize;
      pLoginCtx->pwdInfo.pBlobData = reinterpret_cast<BYTE*>(malloc(pwdInfo.cbSize));

      if(NULL == pLoginCtx->pwdInfo.pBlobData)
      {
         hr = E_OUTOFMEMORY;
         goto cleanup;
      } 

      copyResult = memcpy_s(pLoginCtx->pwdInfo.pBlobData, pwdInfo.cbSize, pwdInfo.pBlobData, pwdInfo.cbSize);

      if(0 != copyResult)
      {
         hr = HRESULT_FROM_WIN32(copyResult);
         goto cleanup;
      }

   }  // if(VARIANT_FALSE == fUsedCachedCreds)


   // If the login thread is not active, the following
   // call to PostThreadMessage will fail.
   
   postResult = PostThreadMessage(
      m_loginThreadId,
      m_msgLogin,
      0,
      reinterpret_cast<LPARAM>(pLoginCtx) );

   if(0 == postResult)
   {     
      hr = HRESULT_FROM_WIN32(GetLastError()); 
      ATLTRACE2("%x: Login: PostThreadMessage failed. %x\n", GetCurrentThreadId(), hr);
      goto cleanup;
   }
  
   ATLTRACE2("%x: Login: PostThreadMessage succeeded.\n", GetCurrentThreadId());

   // We successfully posted the message to the log-in thread.
   // We have no more need for the pointer to the log-in context.
   pLoginCtx = NULL;
  
cleanup:

   if(pLoginCtx)
   {
      // We failed to post a message to the log-in thread.
      // The log-in thread will not be able to free the memory
      // pointed to by pLoginCtx.  So we free it here.   

      if(NULL != pLoginCtx->userInfo.pBlobData)
      {
         free(pLoginCtx->userInfo.pBlobData);
         pLoginCtx->userInfo.pBlobData = NULL;
      }

      if(NULL != pLoginCtx->pwdInfo.pBlobData)
      {
         free(pLoginCtx->pwdInfo.pBlobData);
         pLoginCtx->pwdInfo.pBlobData = NULL;
      }
   
      delete pLoginCtx;
      pLoginCtx = NULL;
   }
   // Otherwise, the log-in thread will free the memory 
   // that was pointed to by pLoginCtx.  

   return hr;
}

HRESULT STDMETHODCALLTYPE C[!output Safe_root]::Logout(void)
{
   BOOL postResult = 0;
   HRESULT hr = S_OK;

   // If the log-in thread has not already been started,
   // start it now.
   
   if(0 == m_loginThreadHandle)
   {  
      hr = this->StartContentPartnerThread(ThreadTypeLogin);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: Logout: StartContentPartnerThread failed. %x\n",GetCurrentThreadId(),  hr);
         goto cleanup;
      } 
      
      ATLTRACE2("%x: Logout: StartContentPartnerThread succeeded.\n", GetCurrentThreadId());     
   }

   // At this point, we know the login thread started, but
   // don't know whether it is still active.

   // If the login thread is not active, the following 
   // call to PostThreadMessage will fail.

   postResult = PostThreadMessage(
      m_loginThreadId,
      m_msgLogout,
      0,
      NULL);

   if(0 == postResult)
   {     
      hr = HRESULT_FROM_WIN32(GetLastError()); 
      ATLTRACE2("%x: Logout: PostThreadMessage failed. %x\n", GetCurrentThreadId(), hr);
      goto cleanup;
   }
  
   ATLTRACE2("%x: Logout: PostThreadMessage succeeded.\n", GetCurrentThreadId());

cleanup:
   return hr;
}


HRESULT STDMETHODCALLTYPE C[!output Safe_root]::Authenticate(
   BLOB userInfo,
   BLOB pwdInfo)
{
   LOGIN_CONTEXT* pLoginCtx = NULL;
   BOOL postResult = 0;
   HRESULT hr = S_OK;
   errno_t copyResult = 0;

   if(NULL == userInfo.pBlobData || userInfo.cbSize <= 0 || 
      NULL == pwdInfo.pBlobData || pwdInfo.cbSize <= 0)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   // If the log-in thread has not already been started,
   // start it now.
   
   if(0 == m_loginThreadHandle)
   {  
      hr = this->StartContentPartnerThread(ThreadTypeLogin);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: Authenticate: StartContentPartnerThread failed. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      } 
      
      ATLTRACE2("%x: Authenticate: StartContentPartnerThread succeeded.\n", GetCurrentThreadId());     
   }

   // At this point, we know the login thread started, but
   // we don't knwo whether it is still active.

   // When we post an authenticate message, we must provide the
   // information that was passed in the the two parameters of this method. 
   // So we copy the two parameters into the first two members of 
   // a LOGIN_CONTEXT structure.

   pLoginCtx = new LOGIN_CONTEXT();
   // This memory is freed in HandleMessageForLoginThread.


   if(NULL == pLoginCtx)
   {
      ATLTRACE2("%x: Authenticate: Failed to create new LOGIN_CONTEXT.\n", GetCurrentThreadId());
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   ZeroMemory(pLoginCtx, sizeof(LOGIN_CONTEXT));

   // Copy the username BLOB. 
   // This includes making our own copy of the blob data.
   // HandleMessageForLoginThread will free our copy of the data.

   pLoginCtx->userInfo.cbSize = userInfo.cbSize;
   pLoginCtx->userInfo.pBlobData = reinterpret_cast<BYTE*>(malloc(userInfo.cbSize));

   if(NULL == pLoginCtx->userInfo.pBlobData)
   {
      hr = E_OUTOFMEMORY;
      goto cleanup;
   } 

   copyResult = memcpy_s(pLoginCtx->userInfo.pBlobData, userInfo.cbSize, userInfo.pBlobData, userInfo.cbSize);

   if(0 != copyResult)
   {
      hr = HRESULT_FROM_WIN32(copyResult);
      goto cleanup;
   }

   // Copy the password BLOB. 
   // This includes making our own copy of the blob data.
   // HandleMessageForLoginThread will free our copy of the data. 

   pLoginCtx->pwdInfo.cbSize = pwdInfo.cbSize;
   pLoginCtx->pwdInfo.pBlobData = reinterpret_cast<BYTE*>(malloc(pwdInfo.cbSize));

   if(NULL == pLoginCtx->pwdInfo.pBlobData)
   {
      hr = E_OUTOFMEMORY;
      goto cleanup;
   } 

   copyResult = memcpy_s(pLoginCtx->pwdInfo.pBlobData, pwdInfo.cbSize, pwdInfo.pBlobData, pwdInfo.cbSize);

   if(0 != copyResult)
   {
      hr = HRESULT_FROM_WIN32(copyResult);
      goto cleanup;
   }


   // If the login thread is not active, the following
   // call to PostMessageThread will fail.
   
   postResult = PostThreadMessage(
      m_loginThreadId,
      m_msgAuthenticate,
      0,
      reinterpret_cast<LPARAM>(pLoginCtx) );

   if(0 == postResult)
   {     
      hr = HRESULT_FROM_WIN32(GetLastError()); 
      ATLTRACE2("%x: Authenticate: PostThreadMessage failed. %x\n", hr);
      goto cleanup;
   }
  
   ATLTRACE2("Authenticate: PostThreadMessage succeeded.\n", GetCurrentThreadId());

   // We successfully posted the message to the login thread.
   // We have no more need for the pointer to the login context.
   pLoginCtx = NULL;

cleanup:

   if(NULL != pLoginCtx)
   {
      // We failed to post a message to the login thread.
      // The login thread will not be able to free the memory
      // pointed to by pLoginCtx.  So we free it here.  

      if(NULL != pLoginCtx->userInfo.pBlobData)
      {
         free(pLoginCtx->userInfo.pBlobData);
         pLoginCtx->userInfo.pBlobData = NULL;
      }

      if(NULL != pLoginCtx->pwdInfo.pBlobData)
      {
         free(pLoginCtx->pwdInfo.pBlobData);
         pLoginCtx->pwdInfo.pBlobData = NULL;
      }
 
      delete pLoginCtx;
      pLoginCtx = NULL;
   }

   return hr;
}


HRESULT STDMETHODCALLTYPE C[!output Safe_root]::VerifyPermission(
   BSTR bstrPermission,
   VARIANT* pContext)
{
   LOGIN_CONTEXT* pLoginCtx = NULL;
   BOOL postResult = 0;
   HRESULT hr = S_OK;

   if(NULL == bstrPermission || NULL == pContext)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   // If the log-in thread has not already been started,
   // start it now.
   
   if(0 == m_loginThreadHandle)
   {  
      hr = this->StartContentPartnerThread(ThreadTypeLogin);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: VerifyPermission: StartContentPartnerThread failed. %x\n", GetCurrentThreadId(), hr);
         goto cleanup;
      } 
      
      ATLTRACE2("%x: VerifyPermission: StartContentPartnerThread succeeded.\n", GetCurrentThreadId());     
   }

   // At this point, we know the login thread started, but
   // we don't knwo whether it is still active.

   // When we post an verify-permission message, we must provide the
   // information that was passed in the the two parameters of this method. 
   // So we copy the two parameters into two members of 
   // a LOGIN_CONTEXT structure.

   pLoginCtx = new LOGIN_CONTEXT();
   // This memory is freed in HandleMessageForLoginThread.

   if(NULL == pLoginCtx)
   {
      ATLTRACE2("%x: VerifyPermission: Failed to create new LOGIN_CONTEXT.\n", GetCurrentThreadId());
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   ZeroMemory(pLoginCtx, sizeof(LOGIN_CONTEXT));

   pLoginCtx->bstrPermission = SysAllocString(bstrPermission);
   // This memory is freed in HandleMessageForLoginThread.

   if(NULL == pLoginCtx->bstrPermission)
   {
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   VariantInit( &(pLoginCtx->permissionContext) );
   hr = VariantCopy( &(pLoginCtx->permissionContext), pContext );
   // January 2007: The only possible type for pContext is VT_BSTR.
   // VariantCopy makes a copy of the data string pointed to by pContext->bstrVal.
   // That memory is freed by a call to VariantClear in HandleMessageForLoginThread.

   if(FAILED(hr))
   {
      goto cleanup;
   }

   // The verify-permission message do not use the following members of pLoginCtx:
   //   BLOB userInfo;
   //   BLOB pwdInfo;
   //   VARIANT_BOOL fUsedCachedCreds;
   //   VARIANT_BOOL fOkToCahce;


   // If the login thread is not active, the following
   // call to PostMessageThread will fail.
   
   postResult = PostThreadMessage(
      m_loginThreadId,
      m_msgVerifyPermission,
      0,
      reinterpret_cast<LPARAM>(pLoginCtx) );

   if(0 == postResult)
   {     
      hr = HRESULT_FROM_WIN32(GetLastError()); 
      ATLTRACE2("%x: VerifyPermission: PostThreadMessage failed. %x\n", GetCurrentThreadId(), hr);
      goto cleanup;
   }
  
   ATLTRACE2("%x: VerifyPermission: PostThreadMessage succeeded.\n", GetCurrentThreadId());

   // We successfully posted the message to the login thread.
   // We have no more need for the pointer to the login context.
   pLoginCtx = NULL;

cleanup:

   if(pLoginCtx)
   {
      // We failed to post a message to the login thread.
      // The login thread will not be able to free the memory
      // pointed to by pLoginCtx.  So we free it here.  

      SysFreeString(pLoginCtx->bstrPermission); // OK to pass NULL
      VariantClear( &(pLoginCtx->permissionContext) );
      delete pLoginCtx;
      pLoginCtx = NULL;
   }

   return hr;
} // VerifyPermission



HRESULT STDMETHODCALLTYPE C[!output Safe_root]::CreateCredentialsFilePath()
{
   HRESULT hr = S_OK;
   BOOL b = FALSE;

   hr = SHGetFolderPath(
      NULL,
      CSIDL_APPDATA,
      NULL,
      SHGFP_TYPE_DEFAULT,
      g.credentialsFile);

   if(FAILED(hr))
   {
      goto cleanup;
   }

   if(S_FALSE == hr)
   {
      hr = E_FAIL;
      goto cleanup;
   }

   b = PathAppend(g.credentialsFile, L"MSSampleType1Store\\");

   if(FALSE == b)
   {
      hr = E_FAIL;
      goto cleanup;
   }

   b = CreateDirectory(g.credentialsFile, NULL);

   if( 0 == b && ERROR_ALREADY_EXISTS != GetLastError() )
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      goto cleanup;
   }

   b = PathAppend(g.credentialsFile, L"Credentials.txt");

   if(FALSE == b)
   {
      hr = E_FAIL;
      goto cleanup;
   }

cleanup:
   return hr;
}
 


// HandleMessageForLoginThread runs in the context of the login thread.
HRESULT HandleMessageForLoginThread(
   MSG* pMsg,
   CONTENT_PARTNER_THREAD_CONTEXT* pThreadCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback)
{
   LOGIN_CONTEXT* pLoginCtx = NULL;
   HRESULT hr = S_OK;
   HRESULT hrForLoginAttempt = E_FAIL;

   if(NULL == pMsg || NULL == pThreadCtx || NULL == spCallback)
   {
      hr = E_INVALIDARG;
      goto cleanup;
   }

   if(pThreadCtx->loginThreadContext.msgAuthenticate == pMsg->message)
   {
      // We must handle this message and free the message context.

      ATLTRACE2("%x: HandleMessageForLoginThread: Recieved message to authenticate.\n", GetCurrentThreadId());
  
      pLoginCtx = reinterpret_cast<LOGIN_CONTEXT*>(pMsg->lParam);

      if(NULL == pLoginCtx)
      {
         hr = E_UNEXPECTED;
         goto cleanup;
      }
  
      // ToDo: Inspect pLoginCtx.userInfo and pLoginCtx->pwdInfo
      // to make sure everything is in order.

      // Simulate a lengthy operation by sleeping two seconds.
      SleepEx(2000, FALSE);
    
      VARIANT authResult;
      VariantInit(&authResult);
      authResult.vt = VT_BOOL;
      authResult.boolVal = VARIANT_TRUE;
      hr = spCallback->Notify(wmpcnAuthResult, &authResult);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: HandleMessageForLoginThread: Unable to notify Player of successful authentication.\n", GetCurrentThreadId());
         goto cleanup;
      }
   } // authenticate message
 
   else if(pThreadCtx->loginThreadContext.msgLogin == pMsg->message)
   {
      // We must handle this message and free the message context.

      ATLTRACE2("%x: HandleMessageForLoginThread: Recieved message to log in.\n", GetCurrentThreadId());

      pLoginCtx = reinterpret_cast<LOGIN_CONTEXT*>(pMsg->lParam);

      if(NULL == pLoginCtx)
      {
         hr = E_UNEXPECTED;
         goto cleanup;
      }

      if(VARIANT_TRUE == pLoginCtx->fUsedCachedCreds)
      {
         hrForLoginAttempt = LoginUsingCachedCredentials(spCallback);
      }
      else
      {
         hrForLoginAttempt = LoginUsingSuppliedCredentials(pLoginCtx, spCallback);
      }

      VARIANT loginState;
      VariantInit(&loginState);

      if(FAILED(hrForLoginAttempt))
      {
         // We failed to log in the user.
         // Notify the Player.
         loginState.vt = VT_UI4;
         loginState.ulVal = 1;
         hr = spCallback->Notify(wmpcnLoginStateChange, &loginState); 

         if(FAILED(hr))
         {
            ATLTRACE2("%x: HandleMessageForLoginThread: Failed to notify Windows Media Player that a login attempt failed.\n", GetCurrentThreadId()); 
            goto cleanup;
         }
      }
      else
      {
         // We successfully logged in the user.
         // Notify the player
         InterlockedExchange( &(g.userLoggedIn), 1);        
         loginState.vt = VT_BOOL;  // The operation was successful.
         loginState.boolVal = VARIANT_TRUE;  // The user is logged in.
         hr = spCallback->Notify(wmpcnLoginStateChange, &loginState); 

         if(FAILED(hr))
         {
            ATLTRACE2("%x: HandleMessageForLoginThread: Failed to notify Windows Media Player that a login attempt succeeded.\n", GetCurrentThreadId()); 
            goto cleanup;
         }
      }    
   } // login message

   else if(pThreadCtx->loginThreadContext.msgLogout == pMsg->message)
   {
      // We must handle this message.
      // There is no context associated with this message, so we
      // do not have to free any memory.

      ATLTRACE2("%x: HandleMessageForLoginThread: Received message to log out.\n", GetCurrentThreadId());

      // Simulate lengthy operation by sleeping two seconds.
      SleepEx(2000, FALSE);

      InterlockedExchange( &(g.userLoggedIn), 0);

      LONG haveCreds = 0;
      HaveCachedCredentials(&haveCreds);

      if(1 == haveCreds)
      {
         hr = DeleteCachedCredentials();

         if(FAILED(hr))
         {
            ATLTRACE2("%x: HandleMessageForLoginThread: Failed to delete cached credentials.\n", GetCurrentThreadId());
            // There is no mechanism to notify the Player that we failed to log out the user.
            goto cleanup;
         }
      }
        
      // Notify the Player that the log out attempt was successful.  
      VARIANT loginState;
      VariantInit(&loginState);
      loginState.vt = VT_BOOL;  // The operation was successful.
      loginState.boolVal = VARIANT_FALSE;  // The user is logged out.
      hr = spCallback->Notify(wmpcnLoginStateChange, &loginState);

      if(FAILED(hr))
      {
         ATLTRACE2("%x: HandleMessageForLoginThread: Unable to notify Player of successful logout.\n", GetCurrentThreadId());
         goto cleanup;
      }      
   } // log-out message


   else if(pThreadCtx->loginThreadContext.msgVerifyPermission == pMsg->message)
   {
      // We must handle this message and free the message context.

      ATLTRACE2("%x: HandleMessageForLoginThread: Recieved message to verify permission.\n", GetCurrentThreadId());
  
      pLoginCtx = reinterpret_cast<LOGIN_CONTEXT*>(pMsg->lParam);

      if(NULL == pLoginCtx)
      {
         hr = E_UNEXPECTED;
         goto cleanup;
      }

      if(NULL == pLoginCtx->bstrPermission)
      {
         hr = E_UNEXPECTED;
         goto cleanup;
      }
  
      if( 0 == wcscmp(pLoginCtx->bstrPermission, g_szVerifyPermissionSync) )
      {
         // ToDo: Inspect the canonical device name in pLoginCtx->permissionContext.bstrVal
         // and determine whether permission should be granted.

         // Simulate lengthy operation by sleeping two seconds.
         SleepEx(2000, FALSE);
         hr = spCallback->VerifyPermissionComplete(pLoginCtx->bstrPermission, &(pLoginCtx->permissionContext), S_OK);

         if(FAILED(hr))
         {
            ATLTRACE2("%x: HandleMessageForLoginThread: Unable to notify Player that permission is granted.\n", GetCurrentThreadId());  
            goto cleanup;
         }
      }
      else
      {
         // We shouldn't ever get here.
         hr = spCallback->VerifyPermissionComplete(pLoginCtx->bstrPermission, &(pLoginCtx->permissionContext), E_FAIL);

         if(FAILED(hr))
         {
            ATLTRACE2("%x: HandleMessageForLoginThread: Unable to notify Player that permission is denied.\n", GetCurrentThreadId());  
            goto cleanup;
         }
      }
     
   } // verify-permission message

   else
   {
      ATLTRACE2("%x: HandleMessageForLoginThread: Received unrecognized message. %x\n", pMsg->message, GetCurrentThreadId());
   }

cleanup:

   if(NULL != pThreadCtx && NULL != pMsg)
   {

      if(pThreadCtx->loginThreadContext.msgAuthenticate == pMsg->message ||
         pThreadCtx->loginThreadContext.msgLogin == pMsg->message)
      {
         // The thread that posted this message allocated a
         // LOGIN_CONTEXT structure.
         // We must free that memory here.

         if(NULL != pLoginCtx)
         {

            if(NULL != pLoginCtx->userInfo.pBlobData)
            {
               free(pLoginCtx->userInfo.pBlobData);
               pLoginCtx->userInfo.pBlobData = NULL;
            }

            if(NULL != pLoginCtx->pwdInfo.pBlobData)
            {
               free(pLoginCtx->pwdInfo.pBlobData);
               pLoginCtx->pwdInfo.pBlobData = NULL;
            }

            delete pLoginCtx;
            pLoginCtx = NULL;
         }
      }

      if(pThreadCtx->loginThreadContext.msgVerifyPermission == pMsg->message)
      {
         // The thread that posted this message allocated a
         // LOGIN_CONTEXT structure.
         // We must free that memory here.

         if(NULL != pLoginCtx)
         {
            SysFreeString(pLoginCtx->bstrPermission);  // OK to pass NULL
            VariantClear( &(pLoginCtx->permissionContext) );

            delete pLoginCtx;
            pLoginCtx = NULL;
         }
      }

   }

   return hr;
}


HRESULT CacheCredentials(LOGIN_CONTEXT* pLoginCtx)
{
   HANDLE hCredFile = INVALID_HANDLE_VALUE;
   DWORD bytesWritten = 0;
   HRESULT hr = S_OK;
   BOOL retVal = 0;
   
   ATLTRACE2("%x: CacheCredentials\n", GetCurrentThreadId());

   hCredFile = CreateFile(
      g.credentialsFile,
      GENERIC_WRITE,
      0,
      NULL,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      NULL);

   if(INVALID_HANDLE_VALUE == hCredFile)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      goto cleanup;
   }

   // Write the size of the user info to the file.
   retVal = WriteFile(
      hCredFile,
      (VOID*)(&pLoginCtx->userInfo.cbSize),
      sizeof(pLoginCtx->userInfo.cbSize),
      &bytesWritten,
      NULL);

   if(0 == retVal)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      goto cleanup;
   }

   // Write the user info to the file.
   retVal = WriteFile(
      hCredFile,
      pLoginCtx->userInfo.pBlobData,
      pLoginCtx->userInfo.cbSize,
      &bytesWritten,
      NULL);

   if(0 == retVal)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      goto cleanup;
   }

   // Write the size of the password info to the file.
   retVal = WriteFile(
      hCredFile,
      (VOID*)(&pLoginCtx->pwdInfo.cbSize),
      sizeof(pLoginCtx->pwdInfo.cbSize),
      &bytesWritten,
      NULL);

   if(0 == retVal)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      goto cleanup;
   }

   // Write the password info to the file.
   retVal = WriteFile(
      hCredFile,
      pLoginCtx->pwdInfo.pBlobData,
      pLoginCtx->pwdInfo.cbSize,
      &bytesWritten,
      NULL);

   if(0 == retVal)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      goto cleanup;
   }

   InterlockedExchange( &(g.haveCachedCredentials), 1 );
      
cleanup:
   CloseHandle(hCredFile);
   return hr;
}


HRESULT DeleteCachedCredentials()
{   
   BOOL result = 0;
   HRESULT hr = S_OK;  

   ATLTRACE2("%x: DeleteCachedCredentials\n", GetCurrentThreadId());

   result = DeleteFile(g.credentialsFile);

   if(0 == result)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      goto cleanup;
   }

   InterlockedExchange( &(g.haveCachedCredentials), 0 );
  
cleanup:
   return hr;
}


HRESULT HaveCachedCredentials(LONG* pCached)
{
   *pCached = InterlockedCompareExchange( &(g.haveCachedCredentials), 0, 0);
   return S_OK;
}


HRESULT LoginUsingCachedCredentials(CComPtr<IWMPContentPartnerCallback> spCallback)
{
   HANDLE hCredFile = INVALID_HANDLE_VALUE;
   DWORD bytesRead = 0;
   BOOL retVal = 0;
   BLOB userInfo = {0};
   BLOB pwdInfo = {0};
   DATA_BLOB unprotectedUserInfo = {0};
   DATA_BLOB unprotectedPwdInfo = {0};
   HRESULT hr = S_OK;
   
   hCredFile = CreateFile(
      g.credentialsFile, 
      GENERIC_READ, 
      0, 
      NULL,
      OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL,
      NULL);

   if(INVALID_HANDLE_VALUE == hCredFile)
   {
      // This should not happen, because Windows Media Player
      // asks us whether we have cached credentilas before it
      // asks us to use them.

      // But for some strage reason, we don't have cached credentials.

      hr = E_FAIL;
      goto cleanup;
   }

   // Read the size of the username data.
   retVal = ReadFile(
      hCredFile, 
      &userInfo.cbSize, 
      sizeof(ULONG), 
      &bytesRead, 
      NULL);

   if(0 == retVal)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      goto cleanup;
   }

   // Allocate a buffer for the username data.
   userInfo.pBlobData = (BYTE*)malloc(userInfo.cbSize);
   
   if(NULL == userInfo.pBlobData)
   {
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   // Read the username data.
   retVal = ReadFile(
      hCredFile, 
      userInfo.pBlobData, 
      userInfo.cbSize, 
      &bytesRead, NULL);

   if(0 == retVal)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      goto cleanup;
   }

   // Read the size of the password data.
   retVal = ReadFile(
      hCredFile, 
      &pwdInfo.cbSize, 
      sizeof(ULONG), 
      &bytesRead, 
      NULL);

   if(0 == retVal)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      goto cleanup;
   }

   // Allocate a buffer for the password data.
   pwdInfo.pBlobData = (BYTE*)malloc(pwdInfo.cbSize);

   if(NULL == pwdInfo.pBlobData)
   {
      hr = E_OUTOFMEMORY;
      goto cleanup;
   }

   // Read the password data.
   retVal = ReadFile(
      hCredFile, 
      pwdInfo.pBlobData, 
      pwdInfo.cbSize,
      &bytesRead, 
      NULL);

   if(0 == retVal)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      goto cleanup;
   }

   // Decrypt the username.
   CryptUnprotectData(
      (DATA_BLOB*)&userInfo, NULL, NULL, NULL, NULL, 
      CRYPTPROTECT_UI_FORBIDDEN, &unprotectedUserInfo);

   // Decrypt the password.
   CryptUnprotectData(
      (DATA_BLOB*)&pwdInfo, NULL, NULL, NULL, NULL, 
      CRYPTPROTECT_UI_FORBIDDEN, &unprotectedPwdInfo);

   // ToDo: Communicate with the online store servers to actually
   // get the user logged in.
  
cleanup:
   CloseHandle(hCredFile);

   if(NULL != userInfo.pBlobData)
   {
      free(userInfo.pBlobData);
      userInfo.pBlobData = NULL;
   }

   if(NULL != pwdInfo.pBlobData)
   {
      free(pwdInfo.pBlobData);
      pwdInfo.pBlobData = NULL;
   }

   if(NULL != unprotectedUserInfo.pbData)
   {
      LocalFree(unprotectedUserInfo.pbData);
      unprotectedUserInfo.pbData = NULL;
   }

   if(NULL != unprotectedPwdInfo.pbData)
   {
      LocalFree(unprotectedPwdInfo.pbData);
      unprotectedPwdInfo.pbData = NULL;
   }

   if(SUCCEEDED(hr))
   {
      ATLTRACE2("%x: LoginUsingCachedCredentials succeeded.\n", GetCurrentThreadId());
   }
   else
   {
      ATLTRACE2("%x: LoginUsingCachedCredentials failed. %x\n", GetCurrentThreadId(), hr);
   }

   return hr;
}

HRESULT LoginUsingSuppliedCredentials(
   LOGIN_CONTEXT* pLoginCtx, 
   CComPtr<IWMPContentPartnerCallback> spCallback)
{
   DATA_BLOB unprotectedUserInfo = {0};
   DATA_BLOB unprotectedPwdInfo = {0};
   HRESULT hr = S_OK;

   if(VARIANT_TRUE == pLoginCtx->fOkToCahce)
   {
      hr = CacheCredentials(pLoginCtx);

      if(FAILED(hr))
      {
         ATLTRACE2("%x LoginUsingSuppliedCredentials: Failed to cache credentials. &x\n", GetCurrentThreadId(), hr);
      }
   }

   // Decrypt the username.
   CryptUnprotectData(
      (DATA_BLOB*)&pLoginCtx->userInfo, NULL, NULL, NULL, NULL, 
      CRYPTPROTECT_UI_FORBIDDEN, &unprotectedUserInfo);

   // Decrypt the password.
   CryptUnprotectData(
      (DATA_BLOB*)&pLoginCtx->pwdInfo, NULL, NULL, NULL, NULL, 
      CRYPTPROTECT_UI_FORBIDDEN, &unprotectedPwdInfo);

   // ToDo: Communicate with the online store servers to actually
   // get the user logged in.

   //cleanup

   if(NULL != unprotectedUserInfo.pbData)
   {
      LocalFree(unprotectedUserInfo.pbData);
      unprotectedUserInfo.pbData = NULL;
   }

   if(NULL != unprotectedPwdInfo.pbData)
   {
      LocalFree(unprotectedPwdInfo.pbData);
      unprotectedPwdInfo.pbData = NULL;
   }

   if(SUCCEEDED(hr))
   {
      ATLTRACE2("%x: LoginUsingSuppliedCredentials succeeded.\n", GetCurrentThreadId());
   }
   else
   {
      ATLTRACE2("%x: LoginUsingSuppliedCredentials failed.\n", GetCurrentThreadId());
   } 
 
   return hr;
}