/////////////////////////////////////////////////////////////////////////////
//
// [!output root]Events.cpp : Implementation of C[!output Safe_root] events
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "[!output root].h"

void C[!output Safe_root]::OpenStateChange( long NewState )
{
    switch (NewState)
    {
    case wmposUndefined:
        break;
    case wmposPlaylistChanging:
        break;
    case wmposPlaylistLocating:
        break;
    case wmposPlaylistConnecting:
        break;
    case wmposPlaylistLoading:
        break;
    case wmposPlaylistOpening:
        break;
    case wmposPlaylistOpenNoMedia:
        break;
    case wmposPlaylistChanged:
        break;
    case wmposMediaChanging:
        break;
    case wmposMediaLocating:
        break;
    case wmposMediaConnecting:
        break;
    case wmposMediaLoading:
        break;
    case wmposMediaOpening:
        break;
    case wmposMediaOpen:
        break;
    case wmposBeginCodecAcquisition:
        break;
    case wmposEndCodecAcquisition:
        break;
    case wmposBeginLicenseAcquisition:
        break;
    case wmposEndLicenseAcquisition:
        break;
    case wmposBeginIndividualization:
        break;
    case wmposEndIndividualization:
        break;
    case wmposMediaWaiting:
        break;
    case wmposOpeningUnknownURL:
        break;
    default:
        break;
    }
}

void C[!output Safe_root]::PlayStateChange( long NewState )
{
    switch (NewState)
    {
    case wmppsUndefined:
        break;
    case wmppsStopped:
        break;
    case wmppsPaused:
        break;
    case wmppsPlaying:
        break;
    case wmppsScanForward:
        break;
    case wmppsScanReverse:
        break;
    case wmppsBuffering:
        break;
    case wmppsWaiting:
        break;
    case wmppsMediaEnded:
        break;
    case wmppsTransitioning:
        break;
    case wmppsReady:
        break;
    case wmppsReconnecting:
        break;
    case wmppsLast:
        break;
    default:
        break;
    }
}

void C[!output Safe_root]::AudioLanguageChange( long LangID )
{
}

void C[!output Safe_root]::StatusChange()
{
}

void C[!output Safe_root]::ScriptCommand( BSTR scType, BSTR Param )
{
}

void C[!output Safe_root]::NewStream()
{
}

void C[!output Safe_root]::Disconnect( long Result )
{
}

void C[!output Safe_root]::Buffering( VARIANT_BOOL Start )
{
}

void C[!output Safe_root]::Error()
{
    CComPtr<IWMPError>      spError;
    CComPtr<IWMPErrorItem>  spErrorItem;
    HRESULT                 dwError = S_OK;
    HRESULT                 hr = S_OK;

    if (m_spCore)
    {
        hr = m_spCore->get_error(&spError);

        if (SUCCEEDED(hr))
        {
            hr = spError->get_item(0, &spErrorItem);
        }

        if (SUCCEEDED(hr))
        {
            hr = spErrorItem->get_errorCode( (long *) &dwError );
        }
    }
}

void C[!output Safe_root]::Warning( long WarningType, long Param, BSTR Description )
{
}

void C[!output Safe_root]::EndOfStream( long Result )
{
}

void C[!output Safe_root]::PositionChange( double oldPosition, double newPosition)
{
}

void C[!output Safe_root]::MarkerHit( long MarkerNum )
{
}

void C[!output Safe_root]::DurationUnitChange( long NewDurationUnit )
{
}

void C[!output Safe_root]::CdromMediaChange( long CdromNum )
{
}

void C[!output Safe_root]::PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change )
{
    switch (change)
    {
    case wmplcUnknown:
        break;
    case wmplcClear:
        break;
    case wmplcInfoChange:
        break;
    case wmplcMove:
        break;
    case wmplcDelete:
        break;
    case wmplcInsert:
        break;
    case wmplcAppend:
        break;
    case wmplcPrivate:
        break;
    case wmplcNameChange:
        break;
    case wmplcMorph:
        break;
    case wmplcSort:
        break;
    case wmplcLast:
        break;
    default:
        break;
    }
}

void C[!output Safe_root]::CurrentPlaylistChange( WMPPlaylistChangeEventType change )
{
    switch (change)
    {
    case wmplcUnknown:
        break;
    case wmplcClear:
        break;
    case wmplcInfoChange:
        break;
    case wmplcMove:
        break;
    case wmplcDelete:
        break;
    case wmplcInsert:
        break;
    case wmplcAppend:
        break;
    case wmplcPrivate:
        break;
    case wmplcNameChange:
        break;
    case wmplcMorph:
        break;
    case wmplcSort:
        break;
    case wmplcLast:
        break;
    default:
        break;
    }
}

void C[!output Safe_root]::CurrentPlaylistItemAvailable( BSTR bstrItemName )
{
}

void C[!output Safe_root]::MediaChange( IDispatch * Item )
{
}

void C[!output Safe_root]::CurrentMediaItemAvailable( BSTR bstrItemName )
{
}

void C[!output Safe_root]::CurrentItemChange( IDispatch *pdispMedia)
{
}

void C[!output Safe_root]::MediaCollectionChange()
{
}

void C[!output Safe_root]::MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal )
{
}

void C[!output Safe_root]::MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal )
{
}

void C[!output Safe_root]::MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal)
{
}

void C[!output Safe_root]::PlaylistCollectionChange()
{
}

void C[!output Safe_root]::PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName)
{
}

void C[!output Safe_root]::PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName)
{
}

void C[!output Safe_root]::PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted)
{
}

void C[!output Safe_root]::ModeChange( BSTR ModeName, VARIANT_BOOL NewValue)
{
}

void C[!output Safe_root]::MediaError( IDispatch * pMediaObject)
{
}

void C[!output Safe_root]::OpenPlaylistSwitch( IDispatch *pItem )
{
}

void C[!output Safe_root]::DomainChange( BSTR strDomain)
{
}

void C[!output Safe_root]::SwitchedToPlayerApplication()
{
}

void C[!output Safe_root]::SwitchedToControl()
{
}

void C[!output Safe_root]::PlayerDockedStateChange()
{
}

void C[!output Safe_root]::PlayerReconnect()
{
}

void C[!output Safe_root]::Click( short nButton, short nShiftState, long fX, long fY )
{
}

void C[!output Safe_root]::DoubleClick( short nButton, short nShiftState, long fX, long fY )
{
}

void C[!output Safe_root]::KeyDown( short nKeyCode, short nShiftState )
{
}

void C[!output Safe_root]::KeyPress( short nKeyAscii )
{
}

void C[!output Safe_root]::KeyUp( short nKeyCode, short nShiftState )
{
}

void C[!output Safe_root]::MouseDown( short nButton, short nShiftState, long fX, long fY )
{
}

void C[!output Safe_root]::MouseMove( short nButton, short nShiftState, long fX, long fY )
{
}

void C[!output Safe_root]::MouseUp( short nButton, short nShiftState, long fX, long fY )
{
}
