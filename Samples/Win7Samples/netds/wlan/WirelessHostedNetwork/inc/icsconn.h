// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

// Forward Declaration
class CIcsManager;


class CIcsConnection
{

protected:

    // Not refcounted.
    // Lifetime is superset of all objects of this type.
    CIcsManager*                m_pIcsMgr;

    INetConnection*             m_pNetConnection;
    INetSharingConfiguration*   m_pNSConfig;
    LONG                        m_lIndex;
    bool                        m_bSharingEnabled;
    bool                        m_bPublic;
    bool                        m_bPrivate;
    bool                        m_bSupported;

    NETCON_PROPERTIES           m_NetConnProps;



public:
    CIcsConnection();

    ~CIcsConnection();

    HRESULT
    InitIcsConnection
    (
        CIcsManager*        pIcsMgr,
        INetConnection*     pNetConnection,
        LONG                lIndex
    );

    HRESULT
    DisableSharing
    (
    );

    HRESULT
    EnableAsPublic
    (
    );

    HRESULT
    EnableAsPrivate
    (
    );

    bool IsSupported( ) const { return m_bSupported; };
    bool IsSharingEnabled() const {return m_bSharingEnabled;};
    bool IsPublic() const {return m_bPublic;};
    bool IsPrivate() const {return m_bPrivate;};

    bool
    IsMatch
    (
        GUID*   pGuid
    );

    void GetConnectionGuid(GUID & Guid) const {Guid = m_NetConnProps.guidId;};

    void GetConnectionName(CAtlString & strName) const {strName = m_NetConnProps.pszwName;};

    HRESULT
    RefreshSharingEnabled
    (
    );
};