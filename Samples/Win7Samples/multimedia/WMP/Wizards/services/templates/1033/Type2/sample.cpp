/////////////////////////////////////////////////////////////////////////////
//
// [!output root].cpp : Implementation of C[!output Safe_root]
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "[!output root].h"

//**********************************************************************
// Abstract:  allowPlay() is called before a piece of media is played.
//            The subscription service object can use this callback
//            to prevent a piece of content from playing. Note, that
//            even if you return true through pfAllowPlay() then
//            normal Digital Rights Management rules still apply
//            (i.e. if it is licensed content you will still need
//            a valid license).
// Arguments: hwnd - handle to a window under which the service may parent UI
//            pMedia - current media object that is about to be played
//            pfAllowPlay - this method should return TRUE through this
//                method to allow playback to continue. Otherwise, return
//                FALSE to prevent playback from happening.
//**********************************************************************
HRESULT C[!output Safe_root]::allowPlay(HWND hwnd, IWMPMedia *pMedia, BOOL *pfAllowPlay)
{
    *pfAllowPlay = (::MessageBox(hwnd, L"Allow user to play file?", L"Allow Play", MB_YESNO) == IDYES) ? TRUE : FALSE;
    return S_OK;
}

//**********************************************************************
// Abstract:  allowCDBurn() is called before a piece of content is
//            burned to a CD. This method will be passed a playlist of
//            media that the user is trying to burn to CD. Items that
//            the user doesn't have rights to burn should be removed
//            from pPlaylist. If the service wants to stop the burn
//            then this method should return FALSE in pfAllowBurn.
//            Again, as is with allowPlay() the user will still need
//            a valid license in order to burn the content to the CD.
//
//            This sample code will display a popup dialog that will
//            list all the items in the playlist inside a listbox control.
//            The user can then select which items should not be copied
//            to the device. Your subscription service should do something
//            similar (i.e. have some way to remove items that you don't
//            want copied to the CD).
//
// Arguments: hwnd - handle to window under which service may parent UI
//            pPlaylist - list of media objects that are associated
//                with this service and which the user is trying to
//                burn to CD.
//            pfAllowBurn - return TRUE to allow burn to continue. Otherwise,
//                return FALSE to stop burn.
//**********************************************************************
HRESULT C[!output Safe_root]::allowCDBurn(HWND hwnd, IWMPPlaylist *pPlaylist, BOOL *pfAllowBurn)
{
    // Create the dialog box where we will display our playlist.
    CAllowBurnDialog dlg;
    dlg.SetPlaylist(pPlaylist);
    if (dlg.DoModal() == IDYES)
    {
        *pfAllowBurn = TRUE;
    }
    else
    {
        *pfAllowBurn = FALSE;
    }
    return S_OK;
}

//**********************************************************************
// Abstract:  allowPDATransfer() is similar to allowCDBurn, except the user
//            is trying to copy the files to a PDA.
// Arguments: see allowCDBurn()...
//**********************************************************************
HRESULT C[!output Safe_root]::allowPDATransfer(HWND hwnd, IWMPPlaylist *pPlaylist, BOOL *pfAllowTransfer)
{
    // Create the dialog box where we will display our playlist.
    CAllowTransferDialog dlg;
    dlg.SetPlaylist(pPlaylist);
    if (dlg.DoModal() == IDYES)
    {
        *pfAllowTransfer = TRUE;
    }
    else
    {
        *pfAllowTransfer = FALSE;
    }
    return S_OK;
}

//**********************************************************************
// Abstract:  startBackgroundProcessing() indicates to the service that
//            now is an appropriate time to start background processing.
// Arguments: hwnd - window under which the service may attach UI
//**********************************************************************
HRESULT C[!output Safe_root]::startBackgroundProcessing(HWND hwnd)
{
    HRESULT hr = S_OK;
    ::MessageBox(hwnd, L"Starting background processing", L"[!output FRIENDLYNAME]", 0);
    return hr;
}

//**********************************************************************
// Abstract:  stopBackgroundProcessing() indicates to the service that
//            now is an appropriate time to stop background processing.
// Arguments: None
//**********************************************************************
HRESULT C[!output Safe_root]::stopBackgroundProcessing(void)
{
    return E_NOTIMPL;
}

//**********************************************************************
// Abstract:  serviceEvent() provides the service with an event notification.
// Arguments: event - event code
//**********************************************************************
HRESULT C[!output Safe_root]::serviceEvent(WMPSubscriptionServiceEvent event)
{
    return E_NOTIMPL;
}

//**********************************************************************
// Abstract:  deviceAvailable() indicates to the service that
//            now is an appropriate time to access a portable device.
// Arguments: bstrDeviceName - Name of device that is available
//            pCB - callback interface to use to notify completion
//**********************************************************************
HRESULT C[!output Safe_root]::deviceAvailable(BSTR bstrDeviceName, IWMPSubscriptionServiceCallback *pCB)
{
    return E_NOTIMPL;
}

//**********************************************************************
// Abstract:  prepareForSync() allows the service to prepare a file for
//            sync to a device.
// Arguments: bstrFilename - Name of file that will by transferred.
//            bstrDeviceName - Name of device that file will be transferred to.
//            pCB - callback interface to use to notify completion
//**********************************************************************
HRESULT C[!output Safe_root]::prepareForSync(BSTR bstrFilename, BSTR bstrDeviceName, IWMPSubscriptionServiceCallback *pCB)
{
    return E_NOTIMPL;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Below we define the dialogs that are displayed when the player
// calls allowPlay(), allowCDBurn(), or allowPDATransfer().
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//**********************************************************************
template <class T>
LRESULT CAllowBaseDialog<T>::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled, int iResourceID)
{
    // Get the playlist
    if (m_spPlaylist)
    {
        HWND hwndControl = ::GetDlgItem(m_hWnd, iResourceID);
        long lNumberItems;
        HRESULT hr = m_spPlaylist->get_count(&lNumberItems);
        if (SUCCEEDED(hr))
        {
            for (long i = 0; i < lNumberItems; i++)
            {
                CComPtr<IWMPMedia> spMedia;
                hr = m_spPlaylist->get_item(i, &spMedia);
                if (SUCCEEDED(hr))
                {
                    CComBSTR cbstr;
                    hr = spMedia->get_sourceURL(&cbstr);
                    if (SUCCEEDED(hr))
                    {
                        // Add the URL to the listbox that we will display
                        int iIndex = ::SendMessage(hwndControl, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(static_cast<WCHAR*>(cbstr)));
                        
                        // Store a pointer to our media object with the listbox item (note this a weak reference
                        // to the media object since we know the playlist will live as long as the dialog lives)
                        ::SendMessage(hwndControl, LB_SETITEMDATA, iIndex, reinterpret_cast<LPARAM>(spMedia.p));
                    }
                }
            }
        }
    }
    bHandled = TRUE;
    return 0;
}

//**********************************************************************
template <class T>
LRESULT CAllowBaseDialog<T>::OnRemoveMediaFromPlaylist(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled, int iResourceID)
{
    HWND hwndControl = ::GetDlgItem(m_hWnd, iResourceID);
    if (hwndControl)
    {
        int iCurrentSelection = ::SendMessage(hwndControl, LB_GETCURSEL, 0, 0);
        if (iCurrentSelection != LB_ERR)
        {
            // Get the media object associated with the current listbox selection and delete it from out playlist
            IWMPMedia *wpMedia = reinterpret_cast<IWMPMedia*>(::SendMessage(hwndControl, LB_GETITEMDATA, iCurrentSelection, 0));
            if (m_spPlaylist && wpMedia)
            {
                m_spPlaylist->removeItem(wpMedia);
                wpMedia = NULL; // Since we have a weak reference to wpMedia we can no longer access it since it was deleted
                ::SendMessage(hwndControl, LB_DELETESTRING, iCurrentSelection, 0);
            }
        }
    }
    bHandled = TRUE;
    return 0;
}
