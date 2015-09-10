
/*--

Copyright (C) Microsoft Corporation

Module Name:

    SampleProvider.h

Abstract:

    SampleProvider.h : Declaration of the CSampleProvider class, a sample VSS HW provider
    that makes use of a virtual disk driver to create snapshots.

Notes:

Revision History:

--*/

#pragma once
#include "resource.h"       // main symbols

#include "VssSampleProvider.h"


#ifdef _DISABLE_SAL8718
#pragma prefast(disable:8718 28718, "NO SAL for SDK samples")
#endif

// CSampleProvider

class ATL_NO_VTABLE CSampleProvider : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CSampleProvider, &CLSID_SampleProvider>,
    public IVssHardwareSnapshotProviderEx,
    public IVssProviderCreateSnapshotSet,
    public IVssProviderNotifications

{
public:
    CSampleProvider();
    ~CSampleProvider();
    
    DECLARE_REGISTRY_RESOURCEID(IDR_SAMPLEPROVIDER)

    DECLARE_NOT_AGGREGATABLE(CSampleProvider)

    BEGIN_COM_MAP(CSampleProvider)
        COM_INTERFACE_ENTRY(IVssHardwareSnapshotProvider)

#ifndef _PRELONGHORN_HW_PROVIDER
        COM_INTERFACE_ENTRY(IVssHardwareSnapshotProviderEx)
#endif

        COM_INTERFACE_ENTRY(IVssProviderCreateSnapshotSet)
        COM_INTERFACE_ENTRY(IVssProviderNotifications)
    END_COM_MAP()


    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease() 
    {
    }

    // IVssHardwareSnapshotProvider Methods
public:
    STDMETHOD(AreLunsSupported)( 
        IN LONG lLunCount, 
        IN LONG lContext, 
        __RPC__in_ecount_full_opt(lLunCount) VSS_PWSZ * rgwszDevices, 
        __RPC__inout_ecount_full(lLunCount)  VDS_LUN_INFORMATION* pLunInformation,
        __RPC__out BOOL* pbIsSupported 
        );
    STDMETHOD(FillInLunInfo)( 
        VSS_PWSZ wszDeviceName, 
        __RPC__inout VDS_LUN_INFORMATION * pLunInfo, 
        __RPC__out BOOL * pbIsSupported 
        );
    STDMETHOD(BeginPrepareSnapshot)( 
        VSS_ID SnapshotSetId, 
        VSS_ID SnapshotId, 
        LONG lContext, 
        LONG lLunCount, 
        __RPC__in_ecount_full_opt(lLunCount) VSS_PWSZ * rgDeviceNames, 
        __RPC__inout_ecount_full(lLunCount)  VDS_LUN_INFORMATION * rgLunInformation 
        );
    STDMETHOD(GetTargetLuns)( 
        IN LONG lLunCount, 
        __RPC__in_ecount_full_opt(lLunCount) VSS_PWSZ * rgDeviceNames, 
        __RPC__in_ecount_full_opt(lLunCount) VDS_LUN_INFORMATION * rgSourceLuns, 
        __RPC__inout_ecount_full(lLunCount)  VDS_LUN_INFORMATION * rgDestinationLuns 
        );
    STDMETHOD(LocateLuns)( 
        LONG lLunCount, 
        __RPC__in_ecount_full_opt(lLunCount) VDS_LUN_INFORMATION * rgSourceLuns
        );
    STDMETHOD(OnLunEmpty)( 
        __RPC__in_opt VSS_PWSZ wszDeviceName, 
        __RPC__in_opt VDS_LUN_INFORMATION * pInformation 
        );

    // IVssHardwareSnapshotProviderEx Methods
public:
    STDMETHOD(GetProviderCapabilities)(
        __RPC__out ULONGLONG    *pllOriginalCapabilityMask
        );

    STDMETHOD(OnLunStateChange)(
        __RPC__in_ecount_full_opt(dwCount) VDS_LUN_INFORMATION *pSnapshotLuns,
        __RPC__in_ecount_full_opt(dwCount) VDS_LUN_INFORMATION *pOriginalLuns,
        DWORD dwCount,
        DWORD dwFlags
        );

    STDMETHOD(ResyncLuns)(
        __RPC__in_ecount_full_opt(dwCount) VDS_LUN_INFORMATION *pSourceLuns,
        __RPC__in_ecount_full_opt(dwCount) VDS_LUN_INFORMATION *pTargetLuns,
        DWORD dwCount,
        __RPC__deref_out_opt IVssAsync ** ppAsync
        );

    STDMETHOD(OnReuseLuns)(
        __RPC__in_ecount_full_opt(dwCount) VDS_LUN_INFORMATION *pSnapshotLuns,
        __RPC__in_ecount_full_opt(dwCount) VDS_LUN_INFORMATION *pOriginalLuns,
        DWORD dwCount
        );
    

    // IVssProviderCreateSnapshotSet Methods
public:
    STDMETHOD(EndPrepareSnapshots)(
        VSS_ID SnapshotSetId
        );
    
    STDMETHOD(PreCommitSnapshots)(
        VSS_ID SnapshotSetId
        );
    
    STDMETHOD(CommitSnapshots)(
        VSS_ID SnapshotSetId
        );
    
    STDMETHOD(PostCommitSnapshots)(
        VSS_ID SnapshotSetId, 
        LONG lSnapshotsCount
        );
    
    STDMETHOD(PreFinalCommitSnapshots)(
        VSS_ID SnapshotSetId
        );
    
    STDMETHOD(PostFinalCommitSnapshots)(
        VSS_ID SnapshotSetId
        );
    
    STDMETHOD(AbortSnapshots)(
        VSS_ID SnapshotSetId
        );

    // IVssProviderNotifications Methods
public:
    STDMETHOD(OnLoad)(
        __RPC__in_opt IUnknown * pCallback
        );
    
    STDMETHOD(OnUnload)(
        BOOL bForceUnload
        );

    // CSampleProvider Methods
private:
    void DeleteAbortedSnapshots();
    
    std::wstring SnapshotImageFile(
        GUID snapId
        );
    
    BOOL FindSnapId(
        GUID origId, 
        GUID& snapId
        );
    
    BOOL FindOrigId(
        GUID snapId, 
        GUID& origId
        );
    
    void FreeLunInfo(
        VDS_LUN_INFORMATION& lun
        );
    
    void CopyBasicLunInfo(
        VDS_LUN_INFORMATION& lunDst, 
        VDS_LUN_INFORMATION& lunSrc
        );
    
    void DisplayLunInfo(
        VDS_LUN_INFORMATION& lun
        );
    
    HRESULT CreateVirtualDrive(
        GUID snapId,
        std::wstring fileName,
        LARGE_INTEGER fileSize,
        VDS_STORAGE_DEVICE_ID_DESCRIPTOR& vdsDesc,
        VDS_STORAGE_IDENTIFIER& vdsStorId
        );
    
    BOOL IsLunSupported(
        VDS_LUN_INFORMATION& LunInfo
        );

    // Member data
private:

    //
    // Vector of original LUN ids and associated snapshot
    //
    struct SnapshotInfo {
        GUID origLunId;
        GUID snapLunId;
    };
    typedef std::vector<SnapshotInfo> SnapshotInfoVector;
    SnapshotInfoVector m_vSnapshotInfo;

    //
    // Current snapshot set and state, used to detect new snapshot
    // sequences
    //
    VSS_ID m_setId;
    VSS_SNAPSHOT_STATE m_state;

    //
    // Member data lock, any access to member variables must be
    // protected with this lock
    //
    CRITICAL_SECTION m_cs;

    //
    // Virtual bus object
    //
    VstorInterface::VirtualBus m_vbus;
};

OBJECT_ENTRY_AUTO(__uuidof(SampleProvider), CSampleProvider)


