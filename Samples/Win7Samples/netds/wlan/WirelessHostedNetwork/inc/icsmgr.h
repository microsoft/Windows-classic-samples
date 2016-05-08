// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

HRESULT
NSModInit
(
);



VOID
NSModDeinit
(
);

class CIcsConnectionInfo : public CRefObject
{
public:
    CIcsConnectionInfo(const CIcsConnection &);

    GUID m_Guid;
    CAtlString m_Name;
    bool m_SharingEnabled;
    bool m_Public;
    bool m_Private;
    bool m_Supported;

    BOOL operator==(GUID& Guid) {return memcmp(&m_Guid, &Guid, sizeof(GUID)) == 0;};
};

class CIcsManager
{
    friend class CIcsConnection;

protected:
    INetSharingManager*     m_pNSMgr;
    CIcsConnection*         m_pList;
    bool                    m_bInstalled;
    LONG                    m_lNumConns;
    // cache the indexes of interface on which ICS is enabled
    LONG                    m_lPublicICSIntfIndex;
    LONG                    m_lPrivateICSIntfIndex;

    HRESULT
    GetIndexByGuid
    (
        GUID&       Guid,
        long*       plIndex
    );

public:
    CIcsManager();

    ~CIcsManager();

    HRESULT
    InitIcsManager
    (
    );

    HRESULT
    ResetIcsManager
    (
    );

    HRESULT
    RefreshInstalled
    (
    );

    void
    DisableIcsOnAll
    (
    );

    HRESULT
    EnableIcs
    (
        GUID&   PublicGuid,
        GUID&   PrivateGuid
    );

    void
    GetIcsConnections(CRefObjList<CIcsConnectionInfo *> &);

    // cache the index of interfaces that ICS are enabled
    void
    CacheICSIntfIndex
    (
    );

    // enable the ICS on the cached interfaces
    void
    EnableICSonCache
    (
    );
};