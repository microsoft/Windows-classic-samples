/////////////////////////////////////////////////////////////////////////////
//
// [!output root].h : Declaration of the C[!output Safe_root]
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __[!output SAFE_ROOT]_H_
#define __[!output SAFE_ROOT]_H_

#pragma once

#include "resource.h"
#include "wmpplug.h"
[!if HASWINDOW]
class CPluginWindow;
[!endif]

// {[!output CLASSID]}
DEFINE_GUID(CLSID_[!output Safe_root], [!output DEFINEGUID]);

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
class ATL_NO_VTABLE C[!output Safe_root] : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<C[!output Safe_root], &CLSID_[!output Safe_root]>,
[!if LISTENTOEVENTS]
    public IWMPEvents,
[!endif]
    public IWMPPluginUI
{
public:
    C[!output Safe_root]();
    ~C[!output Safe_root]();

DECLARE_REGISTRY_RESOURCEID(IDR_[!output SAFE_ROOT])

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(C[!output Safe_root])
[!if LISTENTOEVENTS]
    COM_INTERFACE_ENTRY(IWMPEvents)
[!endif]
    COM_INTERFACE_ENTRY(IWMPPluginUI)
END_COM_MAP()

    // CComCoClass methods
    HRESULT FinalConstruct();
    void    FinalRelease();

    // IWMPPluginUI methods
    STDMETHODIMP SetCore(IWMPCore *pCore);
[!if HASWINDOW]
    STDMETHODIMP Create(HWND hwndParent, HWND *phwndWindow);
    STDMETHODIMP Destroy();
    STDMETHODIMP TranslateAccelerator(LPMSG lpmsg);
[!else]
    STDMETHODIMP Create(HWND hwndParent, HWND *phwndWindow) { return E_NOTIMPL; }
    STDMETHODIMP Destroy() { return E_NOTIMPL; }
    STDMETHODIMP TranslateAccelerator(LPMSG lpmsg) { return E_NOTIMPL; }
[!endif]
[!if HASPROPERTYPAGE]
    STDMETHODIMP DisplayPropertyPage(HWND hwndParent);
[!else]
    STDMETHODIMP DisplayPropertyPage(HWND hwndParent) { return E_NOTIMPL; }
[!endif]
    STDMETHODIMP GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty);
    STDMETHODIMP SetProperty(const WCHAR *pwszName, const VARIANT *pvarProperty);

[!if LISTENTOEVENTS]
    // IWMPEvents methods
    void STDMETHODCALLTYPE OpenStateChange( long NewState );
    void STDMETHODCALLTYPE PlayStateChange( long NewState );
    void STDMETHODCALLTYPE AudioLanguageChange( long LangID );
    void STDMETHODCALLTYPE StatusChange();
    void STDMETHODCALLTYPE ScriptCommand( BSTR scType, BSTR Param );
    void STDMETHODCALLTYPE NewStream();
    void STDMETHODCALLTYPE Disconnect( long Result );
    void STDMETHODCALLTYPE Buffering( VARIANT_BOOL Start );
    void STDMETHODCALLTYPE Error();
    void STDMETHODCALLTYPE Warning( long WarningType, long Param, BSTR Description );
    void STDMETHODCALLTYPE EndOfStream( long Result );
    void STDMETHODCALLTYPE PositionChange( double oldPosition, double newPosition);
    void STDMETHODCALLTYPE MarkerHit( long MarkerNum );
    void STDMETHODCALLTYPE DurationUnitChange( long NewDurationUnit );
    void STDMETHODCALLTYPE CdromMediaChange( long CdromNum );
    void STDMETHODCALLTYPE PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change );
    void STDMETHODCALLTYPE CurrentPlaylistChange( WMPPlaylistChangeEventType change );
    void STDMETHODCALLTYPE CurrentPlaylistItemAvailable( BSTR bstrItemName );
    void STDMETHODCALLTYPE MediaChange( IDispatch * Item );
    void STDMETHODCALLTYPE CurrentMediaItemAvailable( BSTR bstrItemName );
    void STDMETHODCALLTYPE CurrentItemChange( IDispatch *pdispMedia);
    void STDMETHODCALLTYPE MediaCollectionChange();
    void STDMETHODCALLTYPE MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal );
    void STDMETHODCALLTYPE MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal );
    void STDMETHODCALLTYPE MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal);
    void STDMETHODCALLTYPE PlaylistCollectionChange();
    void STDMETHODCALLTYPE PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName);
    void STDMETHODCALLTYPE PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName);
    void STDMETHODCALLTYPE PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted);
    void STDMETHODCALLTYPE ModeChange( BSTR ModeName, VARIANT_BOOL NewValue);
    void STDMETHODCALLTYPE MediaError( IDispatch * pMediaObject);
    void STDMETHODCALLTYPE OpenPlaylistSwitch( IDispatch *pItem );
    void STDMETHODCALLTYPE DomainChange( BSTR strDomain);
    void STDMETHODCALLTYPE SwitchedToPlayerApplication();
    void STDMETHODCALLTYPE SwitchedToControl();
    void STDMETHODCALLTYPE PlayerDockedStateChange();
    void STDMETHODCALLTYPE PlayerReconnect();
    void STDMETHODCALLTYPE Click( short nButton, short nShiftState, long fX, long fY );
    void STDMETHODCALLTYPE DoubleClick( short nButton, short nShiftState, long fX, long fY );
    void STDMETHODCALLTYPE KeyDown( short nKeyCode, short nShiftState );
    void STDMETHODCALLTYPE KeyPress( short nKeyAscii );
    void STDMETHODCALLTYPE KeyUp( short nKeyCode, short nShiftState );
    void STDMETHODCALLTYPE MouseDown( short nButton, short nShiftState, long fX, long fY );
    void STDMETHODCALLTYPE MouseMove( short nButton, short nShiftState, long fX, long fY );
    void STDMETHODCALLTYPE MouseUp( short nButton, short nShiftState, long fX, long fY );

[!endif]
[!if HASPROPERTYPAGE]
    WCHAR        m_wszPluginText[MAX_PATH];

[!endif]
private:
    void         ReleaseCore();

    CComPtr<IWMPCore>           m_spCore;
[!if HASWINDOW]
    CPluginWindow               *m_pPluginWindow;
[!endif]
[!if LISTENTOEVENTS]
    CComPtr<IConnectionPoint>   m_spConnectionPoint;
    DWORD                       m_dwAdviseCookie;
[!endif]
};

#endif //__[!output SAFE_ROOT]_H_
