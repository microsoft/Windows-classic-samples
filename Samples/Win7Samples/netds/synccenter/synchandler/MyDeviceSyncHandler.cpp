//////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Module Name:
//      MyDeviceSyncHandler.cpp
//
//  Abstract:
//      Implementation of the Sync Center device handler for a device.
//
//////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "MyDeviceSyncHandler.h"
#include "SetupUI.h"
#include "Guids.h"              // Declare the GUIDs used by this component.
#include "Resources.h"

#include <stdlib.h>             // Used for generating random errors
#include <time.h>

//////////////////////////////////////////////////////////////////////////////
// class CMyDeviceSyncHandler
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  Description:
//      Create an instance of the device handler class.
//
//  Parameters:
//      punkOuter       - Outer IUnknown for aggregation.
//      riid            - ID of interface to return pointer to.
//      pDeviceInfo     - Info about the device to create the handler for.
//      ppv             - Interface pointer.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating the object.
//      Other HRESULTs  - Error querying for requested interface.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceSyncHandler_CreateInstance(
    __in_opt        IUnknown         *punkOuter,
    __in            REFIID            riid,
    __in            SYNCDEVICEINFO   *pDeviceInfo,
    __deref_out     void            **ppv)
{
    *ppv = NULL;

    UNREFERENCED_PARAMETER(punkOuter);

    CMyDeviceSyncHandler *psh = new CMyDeviceSyncHandler(pDeviceInfo);
    HRESULT hr = (psh != NULL) ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = psh->QueryInterface(riid, ppv);
        psh->Release();
    }

    return hr;

} //*** CMyDeviceSyncHandler_CreateInstance

//----------------------------------------------------------------------------
//
//  Description:
//      Constructor.
//
//  Parameters:
//      pDeviceInfo
//          Structure containing information about the device this handler is for.
//
//----------------------------------------------------------------------------
CMyDeviceSyncHandler::CMyDeviceSyncHandler(__in const SYNCDEVICEINFO *pDeviceInfo)
    :
    _cRef(1),
    _cItems(0),
    _ppItems(NULL),
    _pszPartnershipName(NULL)
{
    DllAddRef();
    _nPartnerID = pDeviceInfo->nPartnerID;

    // Truncate if source string is too long.
    // Space for displaying the name is limited.
    StringCchCopyNW(_szHandlerID, ARRAYSIZE(_szHandlerID), pDeviceInfo->szHandlerID, ARRAYSIZE(pDeviceInfo->szHandlerID));
    StringCchCopyNW(_szDeviceName, ARRAYSIZE(_szDeviceName), pDeviceInfo->szName, ARRAYSIZE(pDeviceInfo->szName));

} //*** CMyDeviceSyncHandler::CMyDeviceSyncHandler

//----------------------------------------------------------------------------
//
//  Description:
//      Destructor.
//
//----------------------------------------------------------------------------
CMyDeviceSyncHandler::~CMyDeviceSyncHandler()
{
    if (_pszPartnershipName)
    {
        LocalFree(_pszPartnershipName);
    }
    if (_ppItems != NULL)
    {
        for (ULONG iItem = 0; iItem < _cItems; iItem++)
        {
            if (_ppItems[iItem] != NULL)
            {
                _ppItems[iItem]->Release();
            }
        }

        delete [] _ppItems;
    }

    DllRelease();

} //*** destructor CMyDeviceSyncHandler::~CMyDeviceSyncHandler

//----------------------------------------------------------------------------
// IUnknown (CMyDeviceSyncHandler)
//----------------------------------------------------------------------------

STDMETHODIMP CMyDeviceSyncHandler::QueryInterface(__in REFIID riid, __deref_out void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CMyDeviceSyncHandler, ISyncMgrHandler),
        QITABENT(CMyDeviceSyncHandler, ISyncMgrSyncItemContainer),
        QITABENT(CMyDeviceSyncHandler, ISyncMgrHandlerInfo),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);

} //*** CMyDeviceSyncHandler::QueryInterface

//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CMyDeviceSyncHandler::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;

} //*** CMyDeviceSyncHandler::Release

//----------------------------------------------------------------------------
// ISyncMgrHandler (CMyDeviceSyncHandler)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get the name of the handler.  This name will
//      be displayed in the main Sync Center folder.
//
//  Implements: ISyncMgrHandler
//
//  Parameters:
//      ppszName
//          Name of the handler being returned.  Caller will free using
//              CoTaskMemFree().
//
//
//  Return Values:
//      S_OK        - Operation completed successfully.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::GetName(__deref_out LPWSTR *ppszName)
{
    HRESULT hr = S_OK;

    *ppszName = NULL;

    // Construct the name of the partnership from the name of the device and
    // save it for future uses.
    if (_pszPartnershipName == NULL)
    {
        hr = FormatString(g_hmodThisDll, IDS_DEVICE_FORMAT, &_pszPartnershipName, _szDeviceName);
    } // if: partnership name not constructed yet

    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(_pszPartnershipName, ppszName);
    } // if: partnership name loaded successfully

    return hr;

} //*** CMyDeviceSyncHandler::GetName

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get information about the handler.
//
//  Parameters:
//      ppHandlerInfo   - Interface for getting handler info to return to Sync Center.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::GetHandlerInfo(__deref_out ISyncMgrHandlerInfo **ppHandlerInfo)
{
    return QueryInterface(IID_PPV_ARGS(ppHandlerInfo));

} //*** CMyDeviceSyncHandler::GetHandlerInfo

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get an interface pointer for a specific
//      type of object.
//
//  Implements: ISyncMgrHandler
//
//  Parameters:
//      rguidObjectID   - GUID for the item to get the object for.
//      riid            - Interface to get.
//      ppv             - Interface returned to Sync Center
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_NOTIMPL       - Object not supported by the handler.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::GetObject(
    __in            REFGUID       rguidObjectID,
    __in            REFIID        riid,
    __deref_out     void        **ppv)
{
    HRESULT hr = E_NOTIMPL;
    *ppv = NULL;

    if (rguidObjectID == SYNCMGR_OBJECTID_QueryBeforeActivate)
    {
        hr = CMyDeviceSetupUI_CreateInstance(this, riid, ppv);
    }
    else if (rguidObjectID == SYNCMGR_OBJECTID_BrowseContent)
    {
        hr = CBrowseUI_CreateInstance(FOLDERID_Profile, riid, ppv);
    }
    else if (rguidObjectID == SYNCMGR_OBJECTID_ShowSchedule)
    {
        // Create the Schedule Wizard UI operation and initialize it.
        ISyncMgrScheduleWizardUIOperation *pUIOperation = NULL;
        hr = CoCreateInstance(CLSID_SyncMgrScheduleWizard, NULL, CLSCTX_SERVER, IID_PPV_ARGS(&pUIOperation));
        if (SUCCEEDED(hr))
        {
            hr = pUIOperation->InitWizard(_szHandlerID);
            if (SUCCEEDED(hr))
            {
                hr = pUIOperation->QueryInterface(riid, ppv);
            }

            pUIOperation->Release();
        }
    }

    return hr;

} //*** CMyDeviceSyncHandler::GetObject

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get the mask of capabilities of the handler.
//      The following capabilities are defined:
//
//      SYNCMGR_HCM_NONE
//          No capabilities are specified.
//
//      SYNCMGR_HCM_PROVIDES_ICON
//          Indicates that the handler will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_Icon is specified.
//
//      SYNCMGR_HCM_EVENT_STORE
//          Indicates that the handler will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_EventStore is specified.
//
//      SYNCMGR_HCM_CONFLICT_STORE
//          Indicates that the handler will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_ConflictStore is
//          specified.
//
//      SYNCMGR_HCM_CAN_ENABLE
//          Indicates whether the handler wants to allow the user to enable
//          the handler.  Typically a handler is enabled if at least one of
//          its items is enabled and it is disabled if all of its items are
//          disabled.  Setting this value allows a handler to implement
//          handler-wide enable functionality.  If this value is set the
//          Enable task will be shown in the main Sync Center folder when
//          this handler is selected.
//
//      SYNCMGR_HCM_CAN_DISABLE
//          Indicate whether the handler wants to allow the user to disable
//          the handler.  Typically a handler is enabled if at least on of its
//          items Is enabled and it is disabled if all of its items are
//          disabled.  Setting this value allows a handler to implement
//          handler-wide disable functionality.  It can also be used by a
//          handler to implement support for group policy that forces a
//          handler to always be able to sync.  If this value is set the
//          Disable task will be shown in the main Sync Center folder when
//          this handler is selected.
//
//      SYNCMGR_HCM_CAN_BROWSE_CONTENT
//          Indicates that the handler will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_BrowseContent is
//          specified.  If this value is set, the Browse Content task will be
//          added to the context menu for the handler.
//
//      SYNCMGR_HCM_CAN_SHOW_SCHEDULE
//          Indicates that the handler will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_ShowSchedule is
//          specified.  If this value is set, the Show Schedule task will be
//          added to the context menu for the handler.
//
//      SYNCMGR_HCM_QUERY_BEFORE_ACTIVATE
//          Indicates that the handler will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_QueryBeforeActivate is
//          specified.
//
//      SYNCMGR_HCM_QUERY_BEFORE_DEACTIVATE
//          Indicates that the handler will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_QueryBeforeDeactivate is
//          specified.
//
//      SYNCMGR_HCM_QUERY_BEFORE_ENABLE
//          Indicates that the handler will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_QueryBeforeEnable is
//          specified.
//
//      SYNCMGR_HCM_QUERY_BEFORE_DISABLE
//          Indicates that the handler will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_QueryBeforeDisable is
//          specified.
//
//
//  Implements: ISyncMgrHandler
//
//  Parameters:
//      pmCapabilities  - Capabilities mask being returned.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::GetCapabilities(__out SYNCMGR_HANDLER_CAPABILITIES *pmCapabilities)
{
    *pmCapabilities = (SYNCMGR_HANDLER_CAPABILITIES) (SYNCMGR_HCM_QUERY_BEFORE_ACTIVATE
                                                    | SYNCMGR_HCM_CAN_SHOW_SCHEDULE
                                                    | SYNCMGR_HCM_CAN_BROWSE_CONTENT);
    return S_OK;

} //*** CMyDeviceSyncHandler::GetCapabilities

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get the mask of policies of the handler.
//      The following policies are defined:
//
//      SYNCMGR_HPM_NONE
//          No policies are specified.
//
//      SYNCMGR_HPM_PREVENT_ACTIVATE
//          Indicates that activation of the handler is not supported.  This
//          can be used by a handler to implement support for group policy
//          that prevents a handler from being setup.  If this value is set,
//          the Setup task will be not shown in the Setup Sync folder when
//          this handler is selected.  Most handlers should not set this
//          value.
//
//      SYNCMGR_HPM_PREVENT_DEACTIVATE
//          Indicates that deactivation of the handler is not supported.  This
//          can be used by a handler to implement support for group policy
//          that prevents a handler from being removed from the main Sync
//          Center folder.  If this value is set, the Delete task will be not
//          shown in the main Sync Center folder when this handler is
//          selected.  Most handlers should not set this value.
//
//      SYNCMGR_HPM_PREVENT_ENABLE
//          Indicates that enabling the handler is not supported.  This can be
//          used by a handler to implement support for group policy that
//          prevents the handler from being enabled.  If this value is set the
//          Enable task will not be shown in the main Sync Center folder when
//          this handler is selected.  The handler should provide a comment
//          (returned from its implement of the GetComment() method on
//          ISyncMgrHandlerInfo) to let the user know why the Enable task is
//          not available.  Most handlers should never set this value.
//
//      SYNCMGR_HPM_PREVENT_DISABLE
//          Indicates that disabling the handler is not supported.  This can
//          be used by a handler to implement support for group policy that
//          forces the handler to always be able to sync.  If this value is
//          set the Disable task will not be shown in the main Sync Center
//          folder when this handler is selected.  The handler should provide
//          a comment (returned from its implementation of the GetComment()
//          method on ISyncMgrHandlerInfo) to let the user know why the
//          Disable task is not available.  Most handlers should never set
//          this value.
//
//      SYNCMGR_HPM_PREVENT_START_SYNC
//          Indicates that starting a sync through the user interface or
//          through the APIs is not supported.  Sync can only be started by
//          an external application that creates a session creator to report
//          progress.  If this value is set, the Start Sync task will not be
//          shown in the main Sync Center folder when the handler is selected.
//          Most handlers should not set this value.
//
//      SYNCMGR_HPM_PREVENT_STOP_SYNC
//          Indicates that stopping a sync through the user interface or
//          through the APIs is not supported.  If this value is set, the Stop
//          Sync task will not be shown in the main Sync Center folder when
//          the handler is selected.  Most handlers should not set this value.
//
//      SYNCMGR_HPM_DISABLE_ENABLE
//          Indicates that the enable task should be disabled when it is
//          shown for this handler.  With this policy set, the Enable option
//          will appear in the context menu (if SYNCMGR_HPM_PREVENT_ENABLE is
//          not set) but will be disabled.
//
//      SYNCMGR_HPM_DISABLE_DISABLE
//          Indicates that the disable task should be disabled when it is
//          shown for this handler.  With this policy set, the Disable option
//          will appear in the context menu (if SYNCMGR_HPM_PREVENT_DISABLE is
//          not set) but will be disabled.
//
//      SYNCMGR_HPM_DISABLE_START_SYNC
//          Indicates that the Start Sync task should be disabled when it is
//          shown for this handler.  With this policy set, the Start Sync
//          option will appear in the context menu (if
//          SYNCMGR_HPM_PREVENT_START_SYNC not set) but will be disabled.
//
//      SYNCMGR_HPM_DISABLE_STOP_SYNC
//          Indicates that the Stop Sync task should be disabled when it is
//          shown for this handler.  With this policy set, the Stop Sync
//          option will appear in the context menu (if
//          SYNCMGR_HPM_PREVENT_STOP_SYNC not set) but will be disabled.
//
//      SYNCMGR_HPM_DISABLE_BROWSE
//          Indicates that the Browse task should be disabled when it is shown
//          for this handler.  The Browse task will only be shown if the
//          SYNCMGR_HCM_CAN_BROWSE_CONTENT value is returned from the
//          GetCapabilities() method.
//
//      SYNCMGR_HPM_DISABLE_SCHEDULE
//          Indicates that the Schedule task should be disabled when it is
//          shown for this handler.  The Schedule task will only be shown if
//          the SYNCMGR_HCM_CAN_SHOW_SCHEDULE value is returned from the
//          GetCapabilities() method.
//
//      SYNCMGR_HPM_HIDDEN_BY_DEFAULT
//          Indicates that the handler should be hidden from the user unless
//          the Show Hidden Files option has been enabled.  This policy only
//          applies the first time the handler is loaded.  After that, the
//          hidden state is maintained by Sync Center and can be changed by
//          the user through the property sheet.
//
//      SYNCMGR_HPM_BACKGROUND_SYNC_ONLY
//          Equivalent to setting the SYNCMGR_HPM_DISABLE_START_SYNC and
//          SYNCMGR_HPM_DISABLE_STOP_SYNC values.
//
//  Implements: ISyncMgrHandler
//
//  Parameters:
//      pmPolicies  - Policy mask being returned.
//
//  Return Values:
//      S_OK        - Operation completed successfully.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::GetPolicies(__out SYNCMGR_HANDLER_POLICIES *pmPolicies)
{
    *pmPolicies = SYNCMGR_HPM_NONE;
    return S_OK;

} //*** CMyDeviceSyncHandler::GetPolicies

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center when the user chooses to setup or delete
//      the handler from the Sync Setup folder.
//
//  Implements: ISyncMgrHandler
//
//  Return Values:
//      S_OK                - Handler changed activation state successfully.
//      Failure HRESULTs    - Handler failed to change the activation state.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::Activate(BOOL fActivate)
{
    // Note that your handler will still be called on IsActive() to
    // to determine its activation state.
    UNREFERENCED_PARAMETER(fActivate);
    return S_OK;

} //*** CMyDeviceSyncHandler::Activate


//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center when the user chooses to enable or disable the
//      handler from the main Sync Center folder.
//
//  Implements: ISyncMgrHandler
//
//  Return Values:
//      S_OK                - Handler changed enabled state successfully.
//      Failure HRESULTs    - Handler failed to change the enabled state.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::Enable(BOOL fEnable)
{
    // Note that your handler will still be called on IsEnabled() to
    // determine its enabled state.
    UNREFERENCED_PARAMETER(fEnable);
    return S_OK;

} //*** CMyDeviceSyncHandler::Enable

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to perform a synchronization of the specified
//      items.
//
//  Implements: ISyncMgrHandler
//
//  Parameters:
//      cItems
//          Number of items to sync.
//
//      ppszItemIDs
//          Array of items to sync.
//
//      hwndOwner
//          Owner window handle.
//
//      pCreator
//          Sync session creator interface.
//
//      punk
//          IUnknown passed to the StartHandlerSync() or StartItemSync()
//          methods on the ISyncMgrControl interface.
//
//  Return Values:
//      S_OK        - Operation completed.
//      Other HRESULT.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceSyncHandler::Synchronize(
    __in_ecount(cItems)  LPCWSTR                 *ppszItemIDs,
    __in                 ULONG                    cItems,
    __in                 HWND                     hwndOwner,
    __in                 ISyncMgrSessionCreator  *pCreator,
    __in_opt             IUnknown                *punk)
{
    // TODO: Replace this sample code with real sync code.
    // Note: If an external application performs the sync, simply signal that
    // application and return.  The external application then CoCreates
    // CLSID_SyncMgrClient specifying ISyncMgrSessionCreator and creates a
    // session to report sync progress and state.

    UNREFERENCED_PARAMETER(hwndOwner);
    UNREFERENCED_PARAMETER(punk);

    // Seed the random number generator - we will use this to generate some errors.
    // This line should be removed when implementing a real handler.
    srand((unsigned)time(0));

    // Create a sync session.  If sync occurs in an external process, we would
    // instead signal that process and have it create the sync session.
    ISyncMgrSyncCallback *pCallback = NULL;
    HRESULT hr = pCreator->CreateSession(_szHandlerID, ppszItemIDs, cItems, &pCallback);
    if (SUCCEEDED(hr))
    {
        // Loop through each item until we finish or we receive a
        // cancel request.
        SYNCMGR_CANCEL_REQUEST nCancelRequest = SYNCMGR_CR_NONE;
        HRESULT hrSync = S_OK;

        // Send the initial progress on the handler
        ULONG   uCurrentHandlerStep = 1;
        ULONG   cMaxHandlerSteps    = cItems * 50;
        _SetHandlerProgressText(pCallback, IDS_FILE_SYNC_STEP, &nCancelRequest, 0, cMaxHandlerSteps);

        for (ULONG iItem = 0; iItem < cItems; iItem++)
        {
            // Find the item.
            PCWSTR pszItemID = ppszItemIDs[iItem];
            CMyDeviceSyncItem *pItem = _FindItem(pszItemID);
            if (pItem == NULL)
            {
                _ReportItemProgress(pCallback, pszItemID, IDS_ITEM_NOT_FOUND, SYNCMGR_PS_FAILED, 0, 0, &nCancelRequest);
                continue;
            } // if: couldn't find requested item

            // If the handler was canceled, acknowledge the cancel request.
            if (nCancelRequest == SYNCMGR_CR_CANCEL_ALL)
            {
                // Report that this item has been canceled and proceed to
                // the next item. If no progress is reported for an item,
                // Sync Center will mark it as failed.
                _ReportItemProgress(pCallback, pszItemID, 0, SYNCMGR_PS_CANCELED, 0, 0, &nCancelRequest);
                continue;
            }

            // Send the initial progress report to set the max value.
            ULONG       uCurrentStep    = 1;
            ULONG       cMaxSteps       = 50;
            _ReportItemProgress(pCallback, pszItemID, pItem->GetSyncTextID(), SYNCMGR_PS_UPDATING, 0, cMaxSteps, &nCancelRequest);
            if (nCancelRequest == SYNCMGR_CR_NONE)
            {
                int nDocsCompareResult = CompareStringOrdinal(pszItemID, -1, L"Documents", -1, TRUE);

                for (; (uCurrentStep <= cMaxSteps) && (SUCCEEDED(hrSync)); uCurrentStep++, uCurrentHandlerStep++)
                {
                    // Simulate warning for sample purposes only.
                    // This will generate a warning on the 40th Documents item.
                    if ((nDocsCompareResult == CSTR_EQUAL) && (uCurrentStep == 40))
                    {
                        GUID guidEventID;
                        _ReportEvent(
                                pCallback,
                                pszItemID,
                                IDS_SYNC_WARNING_1,
                                IDS_SYNC_WARNING_DESC_1,
                                SYNCMGR_EL_WARNING,
                                SYNCMGR_EF_NONE,
                                NULL,
                                NULL,
                                NULL,
                                &guidEventID);
                    }

                    // Simulate errors for example purposes only.
                    // This will generate an error on an item
                    // synchronization with 10% probability.
                    int nRandError = (rand() % (50 * 10)) + 1;
                    if (nRandError == 1)
                    {
                        GUID guidEventID;
                        _ReportEvent(
                                pCallback,
                                pszItemID,
                                IDS_SYNC_ERROR_1,
                                IDS_SYNC_ERROR_DESC_1,
                                SYNCMGR_EL_ERROR,
                                SYNCMGR_EF_NONE,
                                NULL /*pszLinkText*/,
                                NULL /*pszLinkReference*/,
                                NULL /*pszContext*/,
                                &guidEventID);
                        hrSync = E_FAIL;
                    }

                    // Report progress.
                    _ReportItemProgress(
                            pCallback,
                            pszItemID,
                            pItem->GetSyncTextID(),
                            SYNCMGR_PS_UPDATING,
                            uCurrentStep,
                            cMaxSteps,
                            &nCancelRequest,
                            uCurrentStep,
                            cMaxSteps);
                    if (nCancelRequest != SYNCMGR_CR_NONE)
                    {
                        // The handler or item has been canceled.
                        break;
                    }

                    // Update the info text shown for the handler.
                    _SetHandlerProgressText(
                            pCallback,
                            IDS_FILE_SYNC_STEP,
                            &nCancelRequest,
                            uCurrentHandlerStep,
                            cMaxHandlerSteps);
                    if (nCancelRequest != SYNCMGR_CR_NONE)
                    {
                        // The handler has been canceled.
                        break;
                    }

                    // TODO: Implement synchronization here instead of a call to Sleep().
                    Sleep(100);

                } // for: each sync pass
            } // if: not canceled

            // We're done synchronizing, but regardless of our current state,
            // we need to send a final progress update to Sync Center so the
            // UI properly reflects the current state of the item.
            if (nCancelRequest == SYNCMGR_CR_NONE)
            {
                // Send the final progress report for this item.
                if (SUCCEEDED(hrSync))
                {
                    _ReportItemProgress(pCallback, pszItemID, IDS_ITEM_SYNC_DONE, SYNCMGR_PS_SUCCEEDED, uCurrentStep - 1, cMaxSteps, &nCancelRequest);
                }
                else
                {
                    _ReportItemProgress(pCallback, pszItemID, IDS_ITEM_FAILED, SYNCMGR_PS_FAILED, uCurrentStep - 1, cMaxSteps, &nCancelRequest, uCurrentStep - 1, cMaxSteps);
                }
            }
            else
            {
                // Report that the current item has canceled.
                _ReportItemProgress(pCallback, pszItemID, 0, SYNCMGR_PS_CANCELED, uCurrentStep, cMaxSteps, &nCancelRequest);
            }

            hrSync = S_OK;
        } // for: each item to sync

        pCallback->Release();
    } // if: sync session created successfully

    return hr;

} //*** CMyDeviceSyncHandler::Synchronize

//----------------------------------------------------------------------------
// ISyncMgrHandlerInfo (CMyDeviceSyncHandler)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get the handler type value for the handler.
//      The following types are defined:
//
//      SYNCMGR_HT_UNSPECIFIED
//          All handlers that do not specify or do not fit
//          in the rest of the options should use this value.
//
//      SYNCMGR_HT_APPLICATION
//          Handler is an application.
//
//      SYNCMGR_HT_DEVICE
//          Handler syncs with a device.
//
//      SYNCMGR_HT_FOLDER
//          Handler syncs with local or remote folders.
//
//      SYNCMGR_HT_SERVICE
//          Handler syncs with a web service.
//
//      SYNCMGR_HT_COMPUTER
//          Handler syncs with a computer.
//
//  Implements: ISyncMgrHandlerInfo
//
//  Parameters:
//      pnType  - Type to return to Sync Center.
//
//  Return Values:
//      S_OK    - Operation completed successfully
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::GetType(__out SYNCMGR_HANDLER_TYPE *pnType)
{
    *pnType = SYNCMGR_HT_DEVICE;
    return S_OK;

} //*** CMyDeviceSyncHandler::GetType

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get a label for the handler type.  This is
//      usually used to display the model of the device or some other handler-
//      specific identifying string.
//
//  Implements: ISyncMgrHandlerInfo
//
//  Parameters:
//      ppszLabel
//          Pointer to fill with a pointer to a string.  Must be allocated
//          with CoTaskMemAlloc().  Sync Center will free this using
//          CoTaskMemFree().
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating memory for the string buffer.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::GetTypeLabel(__deref_out LPWSTR *ppszLabel)
{
    *ppszLabel = NULL;

    WCHAR wszLabel[MAX_SYNCMGR_NAME];

    HRESULT hr = E_FAIL;
    // LoadString returns 0 if the string could not be found.
    if (LoadStringW(g_hmodThisDll, IDS_HANDLER_TYPE_LABEL, wszLabel, ARRAYSIZE(wszLabel)) != 0)
    {
        hr = SHStrDupW(wszLabel, ppszLabel);
    } // if: label string loaded successfully

    return hr;

} //*** CMyDeviceSyncHandler::GetTypeLabel

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get a string to display in the folder in
//      the far right column when a synchronization is not being performed.
//      If no string is provided, a blank string will be displayed.
//
//  Implements: ISyncMgrHandlerInfo
//
//  Parameters:
//      ppszComment
//          Pointer to fill with a pointer to a string.  Must be allocated
//          with CoTaskMemAlloc().  Sync Center will free this using
//          CoTaskMemFree().
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating memory for the string buffer.
//      E_NOTIMPL       - No comment is provided by the handler.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::GetComment(__deref_out LPWSTR *ppszComment)
{
    UNREFERENCED_PARAMETER(ppszComment);
    return E_NOTIMPL;

} //*** CMyDeviceSyncHandler::GetComment

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get the date and time the handler was last
//      synchronized.  If a failure is returned, the value calculated from the
//      last synchronization will be used.
//
//  Implements: ISyncMgrHandlerInfo
//
//  Parameters:
//      pftLastSync - Last sync time.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_NOTIMPL       - No last sync time is provided by the handler.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::GetLastSyncTime(__out FILETIME *pftLastSync)
{
    // If your handler does not return the last sync time,
    // Sync Center will manage this state for you.
    UNREFERENCED_PARAMETER(pftLastSync);
    return E_NOTIMPL;

} //*** CMyDeviceSyncHandler::GetLastSyncTime

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to determine whether the handler is active or
//      not.
//
//      If a handler is not active it appears in the Sync Setup folder.
//      Handlers in that folder cannot be synced.  To move a handler to the
//      main Sync Center folder, the user selects the Setup task on the
//      context menu or on the command module.
//
//      If a handler is active it appears in the main Sync Center folder.
//      A handler that is active can be synced by the user or through the
//      ISyncMgrControl interface.  To move a handler to the Sync Setup
//      folder, the user selects the Delete task on the context menu or on the
//      command module.
//
//      If a handler doesn't want to maintain its active state, it can return
//      E_NOTIMPL and Sync Center will maintain it for the handler.
//
//  Return Values:
//      S_OK        - Handler is active.
//      S_FALSE     - Handler is not active.
//      E_NOTIMPL   - Let Sync Center maintain the enabled state.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::IsActive()
{
    return E_NOTIMPL;

} //*** CMyDeviceSyncHandler::IsActive

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to determine whether the handler is enabled or
//      not.
//
//      If a handler is disabled, neither it nor any of its items will be
//      synchronized by Sync Center and many of the actions will be removed
//      or disabled in the UI.
//
//      If a handler doesn't want to maintain its enabled state, it can return
//      E_NOTIMPL and Sync Center will maintain it for the handler.
//
//  Implements: ISyncMgrHandlerInfo
//
//  Return Values:
//      S_OK        - Handler is enabled.
//      S_FALSE     - Handler is not enabled.
//      E_NOTIMPL   - Let Sync Center maintain the enabled state.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::IsEnabled()
{
    return E_NOTIMPL;

} //*** CMyDeviceSyncHandler::IsEnabled

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to determine whether the handler is in a
//      connected state or not.
//
//      If a handler is in a disconnected state, neither it nor any of its
//      items will be synchronized by Sync Center and many of the actions
//      will be removed or disabled in the UI.
//
//  Return Values:
//      S_OK        - Handler is connected.
//      S_FALSE     - Handler is disconnected.
//      E_NOTIMPL   - Handler doesn't support this state.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::IsConnected()
{
    return E_NOTIMPL;

} //*** CMyDeviceSyncHandler::IsConnected

//----------------------------------------------------------------------------
// ISyncMgrItemContainer (CMyDeviceSyncHandler)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Return the specified sync item to Sync Center.
//
//  Implements: ISyncMgrHandlerInfo
//
//  Parameters:
//      pszItemID   - The ID of the item to return.
//      ppItem      - The item being returned.
//
//  Return Values:
//      S_OK        - Operation completed successfully.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::GetSyncItem(__in LPCWSTR pszItemID, __deref_out ISyncMgrSyncItem **ppItem)
{
    *ppItem = NULL;

    // Make sure the items have been loaded first.
    HRESULT hr = _LoadItems();
    if (SUCCEEDED(hr))
    {
        for (DWORD iItem = 0; iItem < _cItems; iItem++)
        {
            if ((_ppItems[iItem] != NULL) && (CompareSyncMgrID(_ppItems[iItem]->GetItemIDPointer(), pszItemID) == 0))
            {
                hr = _ppItems[iItem]->QueryInterface(IID_ISyncMgrSyncItem, (void **) ppItem);
                break;
            }
        } // for: each item
    }

    return hr;

} //*** CMyDeviceSyncHandler::GetSyncItem

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to return an enumeration of items managed by
//      this handler.
//
//  Implements: ISyncMgrHandlerInfo
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::GetSyncItemEnumerator(__deref_out IEnumSyncMgrSyncItems **ppenum)
{
    *ppenum = NULL;

    // Make sure the items have been loaded first.
    HRESULT hr = _LoadItems();
    if (SUCCEEDED(hr))
    {
        hr = CEnumSyncMgrItems_CreateInstance(this, IID_PPV_ARGS(ppenum));
    }

    return hr;

} //*** CMyDeviceSyncHandler::GetSyncItemEnumerator

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get the count of items managed by this handler.
//
//  Implements: ISyncMgrHandlerInfo
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncHandler::GetSyncItemCount(__out ULONG *pcItems)
{
    // Make sure the items have been loaded.
    HRESULT hr = _LoadItems();
    if (SUCCEEDED(hr))
    {
        *pcItems = _cItems;
    }

    return hr;

} //*** CMyDeviceSyncHandler::GetSyncItemCount

//----------------------------------------------------------------------------
// CMyDeviceSyncHandler Private Methods
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Load items for the device handler.
//      Model 1 - Single item representing all content.
//      Model 2 - Multiple items representing each type of content.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating memory for the items.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceSyncHandler::_LoadItems()
{
    HRESULT hr = S_OK;

    if (_ppItems == NULL)
    {
        // In this case, we're providing a static array of items that each
        // handler will have.  You could also store this information in the
        // registry similar to how handler properties are stored in this sample.
        // Alternatively, if the user has control over which items appear with
        // each handler, this data could be generated dynamically.
        struct DEVICE_ITEM_INFO
        {
            PCWSTR  pszItemID;
            UINT    nItemNameStringID;
            UINT    nItemLabelStringID;
            KNOWNFOLDERID   folderID;        // Folder which should open when the user browses on the item
            UINT    uSyncTextID;             // Text to be displayed on the item during syncing ("Document x of X")
        };
        static const DEVICE_ITEM_INFO s_rgItemInfo[] =
        {
            { L"Documents",     IDS_ITEMNAME_DOCUMENTS, IDS_ITEMLABEL_DOCUMENTS, FOLDERID_Documents, IDS_DOC_SYNC_STEP },
            { L"Music",         IDS_ITEMNAME_MUSIC, IDS_ITEMLABEL_MUSIC, FOLDERID_Music, IDS_SONG_SYNC_STEP },
            { L"Pictures",      IDS_ITEMNAME_PICTURES, IDS_ITEMLABEL_PICTURES, FOLDERID_Pictures, IDS_PIC_SYNC_STEP }
        };

        // Allocate the array of sync item pointers.
        _ppItems = new CMyDeviceSyncItem*[ARRAYSIZE(s_rgItemInfo)]();
        hr = (_ppItems != NULL) ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            // Create each of the sync items.
            _cItems = 0;
            hr = S_OK;
            for (int iItemInfo = 0; (iItemInfo < ARRAYSIZE(s_rgItemInfo)) && SUCCEEDED(hr); iItemInfo++)
            {
                // Retrieve the localized name of the item out of our resource
                PWSTR pszItemName = NULL;
                hr = FormatString(g_hmodThisDll, s_rgItemInfo[iItemInfo].nItemNameStringID, &pszItemName);
                if (SUCCEEDED(hr))
                {
                    // Retrieve the localized label of the item out of our resource
                    PWSTR pszItemLabel = NULL;
                    hr = FormatString(g_hmodThisDll, s_rgItemInfo[iItemInfo].nItemLabelStringID, &pszItemLabel);
                    if (SUCCEEDED(hr))
                    {
                        // Create our item with our ID, name, known folder, and label details.
                        hr = CMyDeviceSyncItem_CreateInstance(
                            s_rgItemInfo[iItemInfo].pszItemID,
                            pszItemName,
                            pszItemLabel,
                            _szHandlerID,
                            s_rgItemInfo[iItemInfo].folderID,
                            s_rgItemInfo[iItemInfo].uSyncTextID,
                            &_ppItems[_cItems]);
                        if (SUCCEEDED(hr))
                        {
                            // Increment our total item count.
                            _cItems++;
                        }

                        LocalFree(pszItemLabel);
                    }

                    LocalFree(pszItemName);
                }
            } // for: each item to create
        } // if: array allocated successfully
    } // if: items not loaded yet

    return hr;

} //*** CMyDeviceSyncHandler::_LoadItems

//----------------------------------------------------------------------------
//
//  Description:
//      Find the item structure for an item ID.
//
//  Parameters:
//      pszItemID   - Item to find.
//
//  Return Values:
//      The item that was found.
//
//----------------------------------------------------------------------------
CMyDeviceSyncItem *CMyDeviceSyncHandler::_FindItem(__in LPCWSTR pszItemID)
{
    if (_cItems == 0)
    {
        _LoadItems();
    }
    CMyDeviceSyncItem *pItem = NULL;

    for (ULONG iItem = 0; iItem < _cItems; iItem++)
    {
        if (CompareSyncMgrID(_ppItems[iItem]->GetItemIDPointer(), pszItemID) == 0)
        {
            pItem = _ppItems[iItem];
            break;
        }
    } // for: each item

    return pItem;

} //*** CMyDeviceSyncHandler::_FindItem

//----------------------------------------------------------------------------
//
//  Description:
//      Report progress of synchronization.
//
//  Parameters:
//      pCallback           - Callback interface to update progress on.
//      pszItemID           - ID of item to report progress on.  Can be NULL.
//      nProgressTextID     - String resource ID of progress text.
//      nStatus             - Status being reported.
//      uCurrentStep        - Current step in the progress report.
//      uMaxStep            - Total number of steps.
//      pfCanceled          - Returns whether user canceled sync for this item.
//      ...                 - Optional parameters to format into the string.
//
//  Return Values:
//      Any values from ISyncMgrSyncCallback::ReportProgress().
//
//----------------------------------------------------------------------------
void CMyDeviceSyncHandler::_ReportItemProgress(
    __in    ISyncMgrSyncCallback        *pCallback,
    __in    LPCWSTR                      pszItemID,
    __in    UINT                         nProgressTextID,
    __in    SYNCMGR_PROGRESS_STATUS      nStatus,
    __in    ULONG                        uCurrentStep,
    __in    ULONG                        uMaxStep,
    __out   SYNCMGR_CANCEL_REQUEST      *pnCancelRequest,
    ...)
{
    HRESULT hr = S_OK;
    *pnCancelRequest = SYNCMGR_CR_NONE;
    if (pCallback != NULL)
    {
        // Default to clearing the progress if we've stopped unless we have
        // something important to convey to the user.  Otherwise, only change
        // the item progress text if we have a new string.
        PWSTR pszProgressText = (nStatus == SYNCMGR_PS_CANCELED) ? L"" : NULL;
        if (nProgressTextID != 0)
        {
            va_list vaParamList;
            va_start(vaParamList, pnCancelRequest);
            hr = FormatStringVA(g_hmodThisDll, nProgressTextID, &pszProgressText, vaParamList);
            va_end(vaParamList);
            assert(SUCCEEDED(hr));
        }

        if (SUCCEEDED(hr))
        {
            hr = pCallback->ReportProgress(pszItemID, pszProgressText, nStatus, uCurrentStep, uMaxStep, pnCancelRequest);
            assert(SUCCEEDED(hr));

            LocalFree(pszProgressText);
        }
    } // if: callback interface was specified
} //*** CMyDeviceSyncHandler::_ReportItemProgress

//----------------------------------------------------------------------------
//
//  Description:
//      Report progress of synchronization for the handler.
//
//  Parameters:
//      pCallback           - Callback interface to update progress on.
//      nProgressTextID     - String resource ID of progress text.
//      pnCancelRequest     - Returns whether user canceled sync for this item.
//      ...                 - Optional parameters to format into the string.
//
//  Return Values:
//      Any values from ISyncMgrSyncCallback::SetHandlerProgressText().
//
//----------------------------------------------------------------------------
void CMyDeviceSyncHandler::_SetHandlerProgressText(
    __in    ISyncMgrSyncCallback        *pCallback,
    __in    UINT                         nProgressTextID,
    __out   SYNCMGR_CANCEL_REQUEST      *pnCancelRequest,
    ...)
{
    HRESULT hr = S_OK;
    if (pCallback != NULL)
    {
        PWSTR pszProgressText = NULL;
        if (nProgressTextID != 0)
        {
            va_list vaParamList;
            va_start(vaParamList, pnCancelRequest);
            hr = FormatStringVA(g_hmodThisDll, nProgressTextID, &pszProgressText, vaParamList);
            va_end(vaParamList);
            assert(SUCCEEDED(hr));
        }

        if (SUCCEEDED(hr))
        {
            hr = pCallback->SetHandlerProgressText(pszProgressText, pnCancelRequest);
            assert(SUCCEEDED(hr));

            LocalFree(pszProgressText);
        }
    } // if: callback interface was specified
} //*** CMyDeviceSyncHandler::_SetHandlerProgressText

//----------------------------------------------------------------------------
//
//  Description:
//      Report a synchronization event
//
//  Parameters:
//      pCallback           - Callback interface to update progress on.
//      pszItemID           - ID of item to report progress on.  Can be NULL.
//      nEventNameID        - String resource ID of the Event Name.
//      nDescriptionTextID  - String resource ID of description text.
//      nLevel              - Level of the event being reported.
//      nFlags              - Additional flags for the event.
//      pszLinkText         - Link text for the event.  Can be NULL
//      pszLinkReference    - Action to shellexecute when the link is activated.  Can be NULL.
//      pszContext          - Additional data to associate with the event.  Can be NULL.
//      ...                 - Optional parameters to format into the string.
//
//  Return Values:
//      Any values from ISyncMgrSyncCallback::ReportEvent().
//
//----------------------------------------------------------------------------
void CMyDeviceSyncHandler::_ReportEvent(
    __in        ISyncMgrSyncCallback    *pCallback,
    __in        LPCWSTR                  pszItemID,
    __in        UINT                     nEventNameID,
    __in        UINT                     nDescriptionTextID,
    __in        SYNCMGR_EVENT_LEVEL      nLevel,
    __in        SYNCMGR_EVENT_FLAGS      nFlags,
    __in_opt    LPCWSTR                  pszLinkText,
    __in_opt    LPCWSTR                  pszLinkReference,
    __in_opt    LPCWSTR                  pszContext,
    __out       GUID                    *pguidEventID,
    ...)
{
    // We must have a valid name and description for all events.
    assert((nEventNameID != 0) && (nDescriptionTextID != 0));

    HRESULT hr = E_INVALIDARG;
    if (pCallback != NULL)
    {
        // Load the localized error name out of our resource.
        WCHAR wszName[MAX_SYNCMGR_NAME];
        PWSTR pszDescription = NULL;
        if (LoadStringW(g_hmodThisDll, nEventNameID, wszName, ARRAYSIZE(wszName)) != 0)
        {
            // Load and format the description string with the optional parameters.
            va_list vaParamList;
            va_start(vaParamList, pguidEventID);
            hr = FormatStringVA(g_hmodThisDll, nDescriptionTextID, &pszDescription, vaParamList);
            va_end(vaParamList);
            assert(SUCCEEDED(hr));
        }

        if (SUCCEEDED(hr))
        {
            // Report the event to Sync Center.
            hr = pCallback->ReportEvent(pszItemID, nLevel, nFlags, wszName, pszDescription, pszLinkText, pszLinkReference, pszContext, pguidEventID);
            assert(SUCCEEDED(hr));

            LocalFree(pszDescription);
        }
    } // if: callback interface was specified
} //*** CMyDeviceSyncHandler::_ReportEvent

//////////////////////////////////////////////////////////////////////////////
// class CEnumSyncMgrItems
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  Description:
//      Create an instance of the enumerator class.
//
//  Parameters:
//      pDeviceHandler  - Device handler to associate this enumerator with.
//      riid            - Interface ID to get.
//      ppv             - Interface pointer returned to caller.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating the object.
//      Other HRESULTs  - Error querying for requested interface.
//
//----------------------------------------------------------------------------
HRESULT CEnumSyncMgrItems_CreateInstance(
    __inout         CMyDeviceSyncHandler     *pDeviceHandler,
    __in            REFIID                    riid,
    __deref_out     void                    **ppv)
{
    *ppv = NULL;

    // Create an enumerator for the handler GUIDs.
    CEnumSyncMgrItems *penum = new CEnumSyncMgrItems(pDeviceHandler);
    HRESULT hr = (penum != NULL) ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = penum->QueryInterface(riid, ppv);
        penum->Release();
    }

    return hr;

} //*** CEnumSyncMgrItems_CreateInstance

//----------------------------------------------------------------------------
// IUnknown (CEnumSyncMgrItems)
//----------------------------------------------------------------------------

STDMETHODIMP CEnumSyncMgrItems::QueryInterface(__in REFIID riid, __deref_out void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CEnumSyncMgrItems, IEnumSyncMgrSyncItems),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);

} //*** CEnumSyncMgrItems::QueryInterface

//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CEnumSyncMgrItems::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;

} //*** CEnumSyncMgrItems::Release

//----------------------------------------------------------------------------
// ISyncMgrEnumItems (CEnumSyncMgrItems)
//----------------------------------------------------------------------------

STDMETHODIMP CEnumSyncMgrItems::Next(__in ULONG celt, __deref_out_ecount(celt) ISyncMgrSyncItem *rgelt[], __out_opt ULONG *pceltFetched)
{
    HRESULT hr = S_OK;
    ULONG cFetched = 0;
    while ((cFetched < celt) && (_iCur < _pDeviceHandler->_cItems))
    {
        hr = _pDeviceHandler->_ppItems[_iCur]->QueryInterface(IID_ISyncMgrSyncItem, (void **) &rgelt[cFetched]);
        if (FAILED(hr))
        {
            break;
        }
        cFetched++;
        _iCur++;
    } // while: more items

    // If we failed to QI one item, return the others we already queried.

    hr = (cFetched == celt) ? S_OK : S_FALSE;

    if (pceltFetched != NULL)
    {
        *pceltFetched = cFetched;
    }

    return hr;

} //*** CEnumSyncMgrItems::Next

