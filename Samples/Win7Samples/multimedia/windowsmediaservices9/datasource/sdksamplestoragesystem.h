//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            SDKSampleStorageSystem.h
//
// Abstract:
//
//*****************************************************************************

#pragma once

#ifndef __SDKSAMPLESTORAGESYSTEM_H_
#define __SDKSAMPLESTORAGESYSTEM_H_

#include "resource.h"       // main symbols

// These are part of the WMS Server SDK.
#include "dataContainer.h"
#include "WMSBasicPlugin.h"

#define URL_SCHEME_LENGTH   9
#define URL_SCHEME          L"sample://"

#define DIR_MASK_LENGTH     2
#define DIR_MASK            L"\\*"

#define MAKEQWORD(a, b)     MAKEULONGLONG((a),(b))
#define LODWORD(l)          ((DWORD)(l))
#define HIDWORD(l)          ((DWORD)(((ULONGLONG)(l) >> 32) & 0xFFFFFFFF))
#define MAKEULONGLONG(a, b) ((ULONGLONG)(((DWORD)(a)) | ((ULONGLONG)((DWORD)(b))) << 32))

class CSDKSampleStorageSystem;

/////////////////////////////////////////////////////////////////////////////
// This is a single data container.

class CSampleDataContainer : public IWMSDataContainer
{
public:
    // CSampleDataContainer
    CSampleDataContainer();
    virtual ~CSampleDataContainer();
    HRESULT Shutdown();
    HRESULT Initialize(
                IWMSContext *pUserContext,
                LPWSTR pszContainerName,
                DWORD dwFlags,
                IWMSBufferAllocator __RPC_FAR *pBufferAllocator,
                CSDKSampleStorageSystem *pOwnerContainer,
                IWMSDataSourcePluginCallback __RPC_FAR *pCallback,
                QWORD qwContext
                );
    
    void OnIoCompletion(
                        DWORD dwError,
                        DWORD dwIoType,
                        QWORD qwOffset,
                        DWORD cbTransferred,
                        BYTE *pbBuffer,
                        IWMSDataContainerCallback __RPC_FAR *pCallback,
                        QWORD qwContext
                        );

    // IUnknown
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);

    // IWMSDataContainer
    virtual HRESULT STDMETHODCALLTYPE GetContainerFormat( 
                    GUID __RPC_FAR *pFormat
                    );
    virtual HRESULT STDMETHODCALLTYPE GetDataSourcePlugin( 
                    IWMSDataSourcePlugin __RPC_FAR *__RPC_FAR *ppDataSource
                    );
    virtual HRESULT STDMETHODCALLTYPE GetInfo( 
                    DWORD dwInfoValueId,
                    IWMSDataContainerCallback __RPC_FAR *pCallback,
                    QWORD qwContext
                    );
    virtual HRESULT STDMETHODCALLTYPE Read( 
                    BYTE __RPC_FAR *pbBuffer,
                    QWORD qwOffset,
                    DWORD dwMaxDataSize,
                    DWORD dwFlags,
                    IWMSDataContainerCallback __RPC_FAR *pCallback,
                    QWORD qwContext
                    );
    virtual HRESULT STDMETHODCALLTYPE Write( 
                    BYTE __RPC_FAR *pbBuffer,
                    DWORD dwDataSize,
                    QWORD qwWritePosition,
                    IWMSDataContainerCallback __RPC_FAR *pCallback,
                    QWORD qwContext
                    );
    virtual HRESULT STDMETHODCALLTYPE GetTransferParameters( 
                    QWORD qwDesiredOffset,
                    DWORD dwDesiredMinSize,
                    DWORD dwDesiredMaxSize,
                    QWORD *pqwOffset,
                    DWORD *pdwSize,
                    DWORD *pdwBufferAlignment
                    );
    virtual HRESULT STDMETHODCALLTYPE DoDataContainerExtendedCommand( 
                    LPWSTR szCommandName,
                    IWMSCommandContext *pCommand,
                    DWORD dwCallFlags,
                    IWMSDataContainerCallback __RPC_FAR *pCallback,
                    QWORD qwContext
                    );
    virtual HRESULT STDMETHODCALLTYPE FinishParsingPacketlist( 
                    IWMSPacketList *pPacketList 
                    );

protected:
    enum { IO_READ, IO_WRITE };
    LONG                        m_cRef; 
    CSDKSampleStorageSystem    *m_pOwnerStorageSystem;
    CHAR                       *m_pszPathName;
    HANDLE                      m_hFile;
    QWORD                       m_qwFileSize;
    DWORD                       m_dwAlignmentLog2;
    DWORD                       m_dwAlignment;
    CRITICAL_SECTION            m_CriticalSection;
}; // End of CSampleDataContainer





/////////////////////////////////////////////////////////////////////////////
// CSDKSampleStorageSystem

class ATL_NO_VTABLE CSDKSampleStorageSystem : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSDKSampleStorageSystem, &CLSID_SDKSampleStorageSystem>,
    public IWMSDataSourcePlugin,
    public IWMSBasicPlugin
{
public:
    CSDKSampleStorageSystem();
    virtual ~CSDKSampleStorageSystem();
    
DECLARE_REGISTRY_RESOURCEID(IDR_SDKSAMPLESTORAGESYSTEM)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSDKSampleStorageSystem)
    COM_INTERFACE_ENTRY(IWMSDataSourcePlugin)
    COM_INTERFACE_ENTRY(IWMSBasicPlugin)
END_COM_MAP()

    // IWMSDataSourcePlugin
    virtual HRESULT STDMETHODCALLTYPE OpenDataContainer( 
                    IWMSCommandContext *pCommandContext,
                    IWMSContext *pUserContext,
                    IWMSContext *pPresentationContext,
                    LPWSTR pszContainerName,
                    DWORD dwFlags,
                    IWMSBufferAllocator __RPC_FAR *pBufferAllocator,
                    IWMSDataSourcePluginCallback __RPC_FAR *pCallback,
                    QWORD qwContext
                    );
    virtual HRESULT STDMETHODCALLTYPE OpenDirectory( 
                    IWMSCommandContext *pCommandContext,
                    IWMSContext *pUserContext,
                    IWMSContext *pPresentationContext,
                    LPWSTR pszContainerName,
                    DWORD dwFlags,
                    IWMSBufferAllocator *pBufferAllocator,
                    IWMSDataSourcePluginCallback *pCallback,
                    QWORD qwContext
                    );
    virtual HRESULT STDMETHODCALLTYPE DeleteDataContainer( 
                    LPWSTR pszContainerName,
                    DWORD dwFlags,
                    IWMSDataSourcePluginCallback __RPC_FAR *pCallback,
                    QWORD qwContext
                    );
    virtual HRESULT STDMETHODCALLTYPE GetDataContainerVersion( 
                    IWMSCommandContext __RPC_FAR *pCommandContext,
                    IWMSContext __RPC_FAR *pUserContext,
                    IWMSContext __RPC_FAR *pPresContext,
                    LPWSTR pszContainerName,
                    DWORD dwFlags,
                    IWMSDataSourcePluginCallback __RPC_FAR *pCallback,
                    QWORD qwContext
                    );
    virtual HRESULT STDMETHODCALLTYPE GetRootDirectories( 
                    LPWSTR __RPC_FAR *pstrRootDirectoryList,
                    DWORD dwMaxRoots,
                    IWMSDataSourcePluginCallback __RPC_FAR *pCallback,
                    QWORD qwContext
                    );
    virtual HRESULT STDMETHODCALLTYPE GetDataSourceAttributes( 
                    DWORD __RPC_FAR *pdwFlags
                    );
    virtual HRESULT STDMETHODCALLTYPE CreateDataSourceDirectory( 
                    IWMSCommandContext *pCommandContext,
                    LPWSTR pszContainerName,
                    DWORD dwFlags,
                    IWMSDataSourcePluginCallback *pCallback,
                    QWORD qwContext
                    );
    virtual HRESULT STDMETHODCALLTYPE DeleteDirectory( 
                    LPWSTR pszContainerName,
                    DWORD dwFlags,
                    IWMSDataSourcePluginCallback *pCallback,
                    QWORD qwContext
                    );


    // IWMSBasicPlugin
    virtual HRESULT STDMETHODCALLTYPE InitializePlugin( 
                    IWMSContext __RPC_FAR *pServerContext,
                    IWMSNamedValues *pNamedValues,
                    IWMSClassObject __RPC_FAR *pClassFactory);
                    
    virtual HRESULT STDMETHODCALLTYPE GetCustomAdminInterface( 
                    IDispatch **ppValue
                    );
    virtual HRESULT STDMETHODCALLTYPE OnHeartbeat();
    virtual HRESULT STDMETHODCALLTYPE ShutdownPlugin();

    STDMETHOD( EnablePlugin )( long *pdwFlags, long *pdwHeartbeatPeriod );
    STDMETHOD( DisablePlugin )();    

protected:

    IWMSContext                     *m_pServerContext;
    IWMSNamedValues                 *m_pNamedValues;
    IWMSClassObject                 *m_pClassFactory;
    DWORD                            m_dwPageSize;
};


//////////////////////////////////////////////////////////////////////////////
class CSampleDirectoryInfo
{
public:
    ///////////////////////////
    CSampleDirectoryInfo() 
    { 
        m_pszwName = NULL;
        m_dwFlags = 0;
        m_qwSize = 0;
        m_pNext = NULL;
    }

    ///////////////////////////
    virtual ~CSampleDirectoryInfo() 
    {
        if( NULL != m_pszwName )
        {
            CoTaskMemFree( m_pszwName );
            m_pszwName = NULL;
        }
    }

private:
    friend class CSampleDirectory;

    LPOLESTR                m_pszwName;
    DWORD                   m_dwFlags;
    QWORD                   m_qwSize;
    CSampleDirectoryInfo    *m_pNext;
}; // CSampleDirectoryInfo

/////////////////////////////////////////////////////////////////////////////
// This is a single directory.
class CSampleDirectory :  public IWMSDirectory
{
public:
    // CSampleDirectory
    CSampleDirectory();
    virtual ~CSampleDirectory();

    HRESULT Initialize(
                IWMSContext *pUserContext,
                LPWSTR pszContainerName,
                CSDKSampleStorageSystem *pOwnerStorageSystem,
                IWMSDataSourcePluginCallback *pCallback,
                QWORD qwContext
                );
    HRESULT Shutdown();

    // IUnknown
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);

    // IWMSDirectory
    virtual HRESULT STDMETHODCALLTYPE GetDataSourcePlugin( 
                    IWMSDataSourcePlugin **ppDataSource
                    );
    virtual HRESULT STDMETHODCALLTYPE GetName( 
                    LPOLESTR *pstrValue
                    );
    virtual HRESULT STDMETHODCALLTYPE GetChildInfo( 
                    DWORD dwIndex,
                    WMSDirectoryEntryInfo *pInfo
                    );

protected:
    LONG                        m_cRef;
    CSDKSampleStorageSystem    *m_pOwnerStorageSystem;
    CRITICAL_SECTION            m_CriticalSection;

    WCHAR                      *m_pPathName;
    DWORD                       m_dwPathNameLength;
    WCHAR                      *m_pSearchDirName;
    CSampleDirectoryInfo       *m_pChildren;
    CSampleDirectoryInfo       *m_pRecentChild;
    DWORD                       m_dwItemNum;
}; // CSampleDirectory

#endif //__SDKSAMPLESTORAGESYSTEM_H_
