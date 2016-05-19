/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      RegExt.h
//
//  Implementation File:
//      RegExt.cpp
//
//  Description:
//      Definitions of routines for extension registration.
//
//  Author:
//      <name> (<e-mail name>) Mmmm DD, 2003
//
//  Revision History:
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Global Function Declarations
/////////////////////////////////////////////////////////////////////////////

//
//  Registration routines.
//

DWORD
RegisterCluAdminClusterExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
RegisterCluAdminAllNodesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
RegisterCluAdminAllGroupsExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
RegisterCluAdminAllResourcesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
RegisterCluAdminAllResourceTypesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
RegisterCluAdminAllNetworksExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
RegisterCluAdminAllNetInterfacesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
RegisterCluAdminResourceTypeExtension(
      HCLUSTER      hClusterIn
    , LPCWSTR       pwszResourceTypeIn
    , const CLSID * pClsidIn
    );

//
//  Unregistration routines.
//

DWORD
UnregisterCluAdminClusterExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
UnregisterCluAdminAllNodesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
UnregisterCluAdminAllGroupsExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
UnregisterCluAdminAllResourcesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
UnregisterCluAdminAllResourceTypesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
UnregisterCluAdminAllNetworksExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
UnregisterCluAdminAllNetInterfacesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    );

DWORD
UnregisterCluAdminResourceTypeExtension(
       HCLUSTER      hClusterIn
     , LPCWSTR       pwszResourceTypeIn
     , const CLSID * pClsid
    );
