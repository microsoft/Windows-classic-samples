//*****************************************************************************
//  Copyright (C) 2009 Microsoft Corporation
//  All rights reserved.
// 
// 1) Authorization Plugin needs to implement all following required entries
//   	WSManPluginStartup
//   	WSManPluginShutdown
//   	WSManPluginAuthzUser
//   	WSManPluginAuthzOperation
//   	WSManPluginAuthzReleaseContext
//
// 2) Authorization Plugin may optionally implement the following entry if want to support quota
//   	WSManPluginAuthzQueryQuota
//
//*****************************************************************************

#include <windows.h>
#define WSMAN_API_VERSION_1_0
#include <wsman.h>

BOOL WINAPI DllMain(IN HINSTANCE instance, IN DWORD reason, PVOID /*reserved*/)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        //
        // we disable all thread attach and detach messages to minimize our working set
        //
        if (!DisableThreadLibraryCalls(instance)) 
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------
typedef DWORD (WINAPI *WSMAN_PLUGIN_STARTUP)(
    __in DWORD flags,
    __in PCWSTR applicationIdentification,
    __in_opt PCWSTR extraInfo,
    __out PVOID *pluginContext
    );

Each plug-in needs to support the Startup callback.  A plug-in may be 
initialized more than once within the same process, but only once per 
applicationIdentification.
  ------------------------------------------------------------------------*/
extern "C" DWORD WINAPI WSManPluginStartup(__in DWORD flags,
                                           __in PCWSTR applicationIdentification,
                                           __in_opt PCWSTR extraInfo,
                                           __out PVOID * pluginContext)
{
    return NO_ERROR;
}

/*------------------------------------------------------------------------
typedef DWORD (WINAPI *WSMAN_PLUGIN_SHUTDOWN)(
    __in_opt PVOID pluginContext,
    __in DWORD flags,
    __in DWORD reason
    );

Each plug-in needs to support the Shutdown callback.  Each successful call 
to Startup will result in a call to Shutdown before the DLL is unloaded.
It is important to make sure the plug-in tracks the number of times the 
Startup entry point is called so the plug-in is not shutdown prematurely.
  ------------------------------------------------------------------------*/
extern "C" DWORD WINAPI WSManPluginShutdown(__in_opt PVOID pluginContext,
                                            __in DWORD flags,
                                            __in DWORD reason)
{
    return NO_ERROR;
}

/*------------------------------------------------------------------------
typedef VOID (WINAPI * WSMAN_PLUGIN_AUTHORIZE_USER)(
    __in PVOID pluginContext,
    __in WSMAN_SENDER_DETAILS *senderDetails,
    __in DWORD flags
    );

Plug-in needs to have a DLL export of WSManPluginAuthzUser to handle this
request.  When a user issues a request and has not made a request in
some configurable time this entry point is called to determin if the 
user is allowed to carry out any request at all.  The plug-in must call
WSManPluginAuthzUserComplete to report either the user is not authorized
with ERROR_ACCESS_DENIED or for successful authorization NO_ERROR.
ERROR_WSMAN_REDIRECT_REQUIRED should be reported if a HTTP redirect is
required for this user, in which case the new HTTP URI should be recorded
in the extendedErrorInformation.  All other errors are report a failure
to the client but no specific information is reported.
  ------------------------------------------------------------------------*/
extern "C" VOID WINAPI WSManPluginAuthzUser(__in PVOID pluginContext,
                                            __in WSMAN_SENDER_DETAILS * senderDetails,
                                            __in DWORD flags)
{
    // using original client token
    WSManPluginAuthzUserComplete(senderDetails, 
                                 0, 
                                 (PVOID)22, 
                                 senderDetails->clientToken, 
                                 TRUE, 
                                 ERROR_SUCCESS, 
                                 NULL);

    // using process token
    //WSManPluginAuthzUserComplete(senderDetails, 
    //                             0, 
    //                             (PVOID)22, 
    //                             NULL, 
    //                             FALSE, 
    //                             ERROR_SUCCESS, 
    //                             NULL);

    // user is access denied
    //WSManPluginAuthzUserComplete(senderDetails, 
    //                             0, 
    //                             NULL, 
    //                             NULL, 
    //                             FALSE, 
    //                             ERROR_ACCESS_DENIED, 
    //                             L"AuthzUser Access is denied.");

    // user needs to be redirected
    //WSManPluginAuthzUserComplete(senderDetails, 
    //                             0, 
    //                             NULL, 
    //                             NULL, 
    //                             FALSE, 
    //                             ERROR_WSMAN_REDIRECT_REQUESTED, 
    //                             L"http://redirected.com/wsman");
}

/*------------------------------------------------------------------------
typedef VOID (WINAPI * WSMAN_PLUGIN_AUTHORIZE_OPERATION)(
    __in PVOID pluginContext,
    __in WSMAN_SENDER_DETAILS *senderDetails,
    __in DWORD flags,
    __in DWORD operation,
    __in PCWSTR action,
    __in PCWSTR resourceUri
    );

Plug-in needs to have a DLL export of WSManPluginAuthzOperation to handle this
request.  The plug-in must call WSManPluginAuthzUserComplete to report either 
the user is not authorized with ERROR_ACCESS_DENIED or for successful 
authorization NO_ERROR.  All other errors are reported as a failure
to the client as a SOAP fault with information given in the 
extendedErrorInformation parameter.
  ------------------------------------------------------------------------*/
extern "C" VOID WINAPI WSManPluginAuthzOperation(__in PVOID pluginContext,
                                                 __in WSMAN_SENDER_DETAILS * senderDetails,
                                                 __in DWORD flags,
                                                 __in DWORD operation,
                                                 __in PCWSTR action,
                                                 __in PCWSTR resourceUri)
{
    // operation is allowed
    WSManPluginAuthzOperationComplete(senderDetails, 
                                      0, 
                                      (PVOID)22, 
                                      ERROR_SUCCESS, 
                                      NULL);

    // operation is not allowed
    //WSManPluginAuthzOperationComplete(senderDetails, 
    //                                  0, 
    //                                  NULL, 
    //                                  ERROR_ACCESS_DENIED, 
    //                                  L"AuthzOperation Access is denied.");
}

/*------------------------------------------------------------------------
typedef VOID (WINAPI * WSMAN_PLUGIN_AUTHORIZE_QUERY_QUOTA)(
    __in PVOID pluginContext,
    __in WSMAN_SENDER_DETAILS *senderDetails,
    __in DWORD flags
    );

This entry point will be called after a user has been authorized to 
retrieve quota information for the user. This method will only be called 
if the configuration specifies that quotas are enabled within the 
authorization plug-in.
  ------------------------------------------------------------------------*/
extern "C" VOID WINAPI WSManPluginAuthzQueryQuota(__in PVOID pluginContext,
                                                  __in WSMAN_SENDER_DETAILS * senderDetails,
                                                  __in DWORD flags)
{
    WSMAN_AUTHZ_QUOTA m_quota;
    ZeroMemory(&m_quota, sizeof(m_quota));
    
    // hardcoded quota limits
    m_quota.maxAllowedConcurrentShells = 2;
    m_quota.maxAllowedConcurrentOperations = 2;
    m_quota.timeslotSize = 60;
    m_quota.maxAllowedOperationsPerTimeslot = 10;

    WSManPluginAuthzQueryQuotaComplete(senderDetails, 
                                       0, 
                                       &m_quota, 
                                       ERROR_SUCCESS, 
                                       NULL);
}

/*------------------------------------------------------------------------
typedef VOID (WINAPI * WSMAN_PLUGIN_AUTHORIZE_RELEASE_CONTEXT)(
    __in PVOID userAuthorizationContext
    );

This entry point is used to release the context that a plug-in reports 
via either WSManPluginAuthzUserComplete or WSManPluginAuthzOperationComplete.
  ------------------------------------------------------------------------*/
extern "C" VOID WINAPI WSManPluginAuthzReleaseContext(__in PVOID userAuthorizationContext)
{
    // release the memory if needed
}