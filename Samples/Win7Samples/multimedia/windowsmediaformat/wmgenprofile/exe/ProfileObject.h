//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            ProfileObject.h
//
// Abstract:            The prototypes for the profile object helper
//                      classes.  These classes are used as the item data in
//                      the dialog controls, and contain the configuration of
//                      profile objects.
//
//*****************************************************************************

#if !defined(AFX_PROFILEOBJECT_H__50EC83B6_A641_43E6_8E93_C76D220CF914__INCLUDED_)
#define AFX_PROFILEOBJECT_H__50EC83B6_A641_43E6_8E93_C76D220CF914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "macros.h"
#include "GenProfile_lib.h"

enum ProfileObjectType
{
    OT_Unknown = 0,
    OT_Stream = 0x1,
    OT_Mutex = 0x2,
    OT_StreamPrioritization = 0x4,
    OT_BandwidthSharing = 0x8
};

class CProfileObject
{
public:
    CProfileObject();
    virtual ProfileObjectType Type();
    virtual ULONG AddRef();
    virtual ULONG Release();
    virtual void PrepareForDeletion() = 0;
    virtual HRESULT NotifyObjectDeleted( CProfileObject* pDeletedObject ) = 0;

protected:
    virtual ~CProfileObject();

private:
    ULONG m_cRef;
};

typedef CArray<CProfileObject*,CProfileObject*> CProfileObjectArray;

class CStream : public CProfileObject
{
public: // From CProfileObject
    virtual ProfileObjectType Type();
    virtual void PrepareForDeletion();
    virtual HRESULT NotifyObjectDeleted( CProfileObject* pDeletedObject );

public:
    DWORD GetVideoVBRQuality();
    void SetVideoVBRQuality( DWORD dwVBRQuality );

    DWORD GetStreamCodecIndex();
    void SetStreamCodecIndex( DWORD dwStreamCodecIndex );

    WORD GetStreamNumber();  // Only used during profile creation process
    void SetStreamNumber( WORD wStreamNumber );  // Only used during profile creation process

    BOOL GetIsSMPTE();
    void SetIsSMPTE( BOOL fIsSMPTE );

    BOOL GetIsUncompressed();
    void SetIsUncompressed( BOOL fIsUncompressed );

    void SetVideoMaxBufferWindow( DWORD dwMaxBufferWindow );
    DWORD GetVideoMaxBufferWindow();

    void SetVideoMaxBitrate( DWORD dwMaxBitrate );
    DWORD GetVideoMaxBitrate();

    void SetVideoVBRMode( VIDEO_VBR_MODE nVBRMode );
    VIDEO_VBR_MODE GetVideoVBRMode( );

    BOOL GetVideoIsVBR();
    void SetVideoIsVBR( BOOL fIsVBR );

    GUID GetArbitraryType();
    void SetArbitraryType( GUID guidType );

    DWORD GetVideoQuality();
    void SetVideoQuality( DWORD dwVideoQuality );

    void SetVideoSecondsPerKeyframe( DWORD dwSecondsPerKeyframe );
    DWORD GetVideoSecondsPerKeyframe();

    DWORD GetVideoFPS();
    void SetVideoFPS( DWORD dwFPS );

    DWORD GetStreamBitrate();
    void SetStreamBitrate( DWORD dwBitrate );

    DWORD GetVideoWidth();
    void SetVideoWidth( DWORD dwVideoWidth );

    DWORD GetVideoHeight();
    void SetVideoHeight( DWORD dwHeight );

    DWORD GetStreamBufferWindow();
    void SetStreamBufferWindow( DWORD dwBufferWindow );

    DWORD GetStreamFormatStringIndex();
    void SetStreamFormatStringIndex( DWORD dwFormatStringIndex );

    DWORD GetStreamFormatIndex();
    void SetStreamFormatIndex( DWORD dwIndex );

    void SetStreamType( StreamType stNewType );
    StreamType GetStreamType();

    CString GetName();
    HRESULT SetName( CString strName );

    DWORD GetPixelFormatIndex();
    void SetPixelFormatIndex( DWORD dwIndex );

    DWORD GetPixelFormatStringIndex();
    void SetPixelFormatStringIndex( DWORD dwIndex );

    DWORD GetWaveFormatIndex();
    void SetWaveFormatIndex( DWORD dwIndex );

    DWORD GetWaveFormatStringIndex();
    void SetWaveFormatStringIndex( DWORD dwIndex );

    DWORD GetLanguageLCID();
    void SetLanguageLCID( DWORD dwLCID );

    CStream();

    HRESULT AddDependent( CProfileObject* pDependentObject );
    HRESULT RemoveDependent( CProfileObject* pDependentObject );
    ULONG DependentCount();
    HRESULT GetDependent( INT nIndex, CProfileObject** ppProfileObject );

protected:
    virtual ~CStream();

    CProfileObjectArray m_dependentObjects;
    CString m_strName;
    StreamType m_stStreamType;
    DWORD m_dwStreamFormatIndex;
    DWORD m_dwFormatStringIndex;
    GUID m_guidArbitraryType;
    DWORD m_dwVideoQuality;
    DWORD m_dwSecondsPerKeyframe;
    DWORD m_dwFPS;
    DWORD m_dwStreamBitrate;
    DWORD m_dwVideoWidth;
    DWORD m_dwVideoHeight;
    DWORD m_dwBufferWindow;
    DWORD m_dwStreamCodecIndex;
    WORD m_wStreamNumber; // Only used during profile creation process
    BOOL m_fHasSMPTE;
    BOOL m_fIsUncompressed;
    DWORD m_dwVideoMaxBufferWindow;
    DWORD m_dwVideoMaxBitrate;
    VIDEO_VBR_MODE m_nVideoVBRMode;
    BOOL m_fVideoIsVBR;
    DWORD m_dwVideoVBRQuality;
    DWORD m_dwPixelFormatIndex;
    DWORD m_dwPixelFormatStringIndex;
    DWORD m_dwWaveFormatIndex;
    DWORD m_dwWaveFormatStringIndex;
    LCID m_dwLanguageLCID;
};


class CMutex : public CProfileObject
{
public: // From CProfileObject
    virtual ProfileObjectType Type();
    virtual void PrepareForDeletion();
    virtual HRESULT NotifyObjectDeleted( CProfileObject* pDeletedObject );

public:
    CMutex();

    MUTEX_TYPE GetMutexType( );
    void SetMutexType( MUTEX_TYPE mtType );

    BOOL InObject( CStream* pStream ); 
    INT StreamCount();
    HRESULT AddStream( CStream* pStream );
    HRESULT RemoveStream( CStream* pStream );
    HRESULT RemoveAllStreams();
    HRESULT GetStream( INT nStreamIndex, CStream** ppStream );

protected:
    virtual ~CMutex();

    MUTEX_TYPE m_mtType;
    CProfileObjectArray m_streamsInMutex;
};


class CStreamPrioritizationObject : public CProfileObject
{
public: // From CProfileObject
    virtual ProfileObjectType Type();
    virtual void PrepareForDeletion();
    virtual HRESULT NotifyObjectDeleted( CProfileObject* pDeletedObject );

public:
    CStreamPrioritizationObject();

    HRESULT AddStream( CStream* pNewStream );
    HRESULT ClearMandatoryStreams();
    HRESULT SetStreamMandatory( CStream* pStream, BOOL fIsMandatory );
    HRESULT GetStreamMandatory( CStream* pStream, BOOL *pbIsMandatory );
    HRESULT IncreasePriority( CStream* pStream );
    HRESULT DecreasePriority( CStream* pStream );
    HRESULT GetStreamWithPriority( INT nPriorityIndex, CStream** ppStream );
    INT StreamCount();

protected:
    virtual ~CStreamPrioritizationObject();

    CProfileObjectArray m_streamPriorities;
    CProfileObjectArray m_mandatoryStreams;
};


class CBandwidthSharingObject : public CProfileObject
{
public: // From CProfileObject
    virtual ProfileObjectType Type();
    virtual void PrepareForDeletion();
    virtual HRESULT NotifyObjectDeleted( CProfileObject* pDeletedObject );

public:
	void SetBufferWindow( DWORD dwBufferWindow );
	DWORD GetBufferWindow();
    GUID GetBandwidthSharingType();
    void SetBandwidthSharingType( GUID guidType );

    DWORD GetSharedBitrate( );
    void SetSharedBitrate( DWORD dwBitrate );

    HRESULT AddStream( CStream* pStream );
    HRESULT RemoveStream( CStream* pStream );
    BOOL InObject( CStream* pStream ); 
    HRESULT RemoveAllStreams();
    INT StreamCount();
    HRESULT GetStream( INT nStreamIndex, CStream** ppStream );

    CBandwidthSharingObject();

protected:
	DWORD m_dwBufferWindow;
    virtual ~CBandwidthSharingObject();

    DWORD m_dwSharedBitrate;
    CProfileObjectArray m_streamsSharingBandwidth;
    GUID m_guidBandwidthSharingType;
};



#endif // !defined(AFX_PROFILEOBJECT_H__50EC83B6_A641_43E6_8E93_C76D220CF914__INCLUDED_)
