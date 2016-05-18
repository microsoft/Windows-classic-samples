/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      ExtObjData.h
//
//  Implementation File:
//      ExtObjData.cpp
//
//  Description:
//      Definition of the context menu, property sheet, and wizard funcitons.
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
// Include Files
/////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#include <CluAdmEx.h>   // for IWCContextMenuCallback
#pragma warning( pop )

/////////////////////////////////////////////////////////////////////////////
// Forward Class Declarations
/////////////////////////////////////////////////////////////////////////////

//class CObjData;

/////////////////////////////////////////////////////////////////////////////
// Global Function Declarations
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// class CObjData
/////////////////////////////////////////////////////////////////////////////

class CObjData
{
protected:
    CLUADMEX_OBJECT_TYPE    m_cot;
    CString                 m_strName;
    CRuntimeClass **        m_rgprtcPSPages;    // Property Sheet pages
    CRuntimeClass **        m_rgprtcWizPages;   // Wizard pages

    virtual HRESULT
    AddContextMenuItem(
                  IWCContextMenuCallback * piCallbackIn
                , UINT    nMenuItem
                , UINT    nMenuItemDesc
                , UINT    nBaseID
                , ULONG   uFlags
                );

    virtual HRESULT HrInitialize( IUnknown * piDataIn );

public:

    CObjData( void )
    {
        m_cot = CLUADMEX_OT_NONE;
        m_rgprtcPSPages = NULL;
        m_rgprtcWizPages = NULL;
    } // *** CObjData

    virtual ~CObjData( void ) {}

    static HRESULT
    S_HrCreateObject(
          IUnknown *    piDataIn
        , CObjData **   ppodOut
        );

    virtual HRESULT
    AddContextMenuItems(
        IWCContextMenuCallback * piCallbackIn
        )
    {
        UNREFERENCED_PARAMETER( piCallbackIn );
        return E_NOTIMPL;
    } // *** AddContextMenuItems

    virtual HRESULT
    InvokeCommand(
        ULONG nCommandIDIn
        )
    {
        UNREFERENCED_PARAMETER( nCommandIDIn);
        return E_NOTIMPL;
    } // *** InvokeCommand

    virtual HRESULT
    GetPropertySheetPages(
        CRuntimeClass *** ppaPropSheetsOut
        )
    {
        UNREFERENCED_PARAMETER( ppaPropSheetsOut );
        return E_NOTIMPL;
    } // *** GetPropertySheetPages

    virtual HRESULT
    GetWizardPages(
        CRuntimeClass *** ppaWizardPagesOut
        )
    {
        UNREFERENCED_PARAMETER( ppaWizardPagesOut );
        return E_NOTIMPL;
    } // *** GetWizardPages

    virtual CString
    StrName( void )
    {
        return m_strName;
    } // *** StrName

    CLUADMEX_OBJECT_TYPE
    Cot( void )
    {
        return m_cot;
    } // *** Cot

};  //*** class CObjData

/////////////////////////////////////////////////////////////////////////////
// class CClusterData
/////////////////////////////////////////////////////////////////////////////

class CClusterData
    : public CObjData
{
private:
    HCLUSTER    m_hCluster;         // This is a copy, don't free it.
    CString     m_strClusterName;

protected:
    virtual HRESULT HrInitialize( IUnknown * piDataIn );

public:
    virtual HRESULT AddContextMenuItems( IWCContextMenuCallback * piCallbackIn );
    virtual HRESULT InvokeCommand( ULONG nCommandIDIn );

    HCLUSTER GetHCluster( void ) { return m_hCluster; };

};  //*** class CClusterData

/////////////////////////////////////////////////////////////////////////////
// class CNodeData
/////////////////////////////////////////////////////////////////////////////

class CNodeData
    : public CObjData
{
private:
    HNODE   m_hNode;    // This is a copy, don't free it.

protected:
    virtual HRESULT HrInitialize( IUnknown * piDataIn );

public:

    virtual HRESULT AddContextMenuItems( IWCContextMenuCallback * piCallbackIn );
    virtual HRESULT InvokeCommand( ULONG nCommandIDIn );

    HNODE GetHNode( void ) { return m_hNode; };

};  //*** class CNodeData

/////////////////////////////////////////////////////////////////////////////
// class CGroupData
/////////////////////////////////////////////////////////////////////////////

class CGroupData
    : public CObjData
{
private:
    HGROUP  m_hGroup;   // This is a copy, don't free it.

protected:
    virtual HRESULT HrInitialize( IUnknown * piDataIn );

public:

    virtual HRESULT AddContextMenuItems( IWCContextMenuCallback * piCallbackIn );
    virtual HRESULT InvokeCommand( ULONG nCommandIDIn );

    HGROUP GetHGroup( void ) { return m_hGroup; };

};  //*** class CGroupData

/////////////////////////////////////////////////////////////////////////////
// class CResData
/////////////////////////////////////////////////////////////////////////////

class CResData
    : public CObjData
{
private:
    HRESOURCE   m_hResource;        // This is a copy, don't free it.
    CString     m_strResTypeName;
    BOOL        m_fSupportedType;   // Is this resource a type that we explicitly extend?

protected:
    virtual HRESULT HrInitialize( IUnknown * piDataIn );

public:
    virtual HRESULT AddContextMenuItems( IWCContextMenuCallback * piCallbackIn );
    virtual HRESULT InvokeCommand( ULONG nCommandIDIn );
    virtual HRESULT GetPropertySheetPages( CRuntimeClass *** ppaPropSheetsOut );
    virtual HRESULT GetWizardPages( CRuntimeClass *** ppaWizardPagesOut );

    HRESOURCE GetHResource( void ) { return m_hResource; };

}; //*** class CResData

/////////////////////////////////////////////////////////////////////////////
// class CResTypeData
/////////////////////////////////////////////////////////////////////////////

class CResTypeData
    : public CObjData
{
private:
    BOOL        m_fSupportedType;   // Is this restype one that we explicitly extend?

protected:
    virtual HRESULT HrInitialize( IUnknown * piDataIn );

public:
    virtual HRESULT AddContextMenuItems( IWCContextMenuCallback * piCallbackIn );
    virtual HRESULT InvokeCommand( ULONG nCommandIDIn );

};  //*** class CResData

/////////////////////////////////////////////////////////////////////////////
// class CNetworkData
/////////////////////////////////////////////////////////////////////////////

class CNetworkData
    : public CObjData
{
private:
    HNETWORK    m_hNetwork;     // This is a copy, don't free it.

protected:
    virtual HRESULT HrInitialize( IUnknown * piDataIn );

public:

    virtual HRESULT AddContextMenuItems( IWCContextMenuCallback * piCallbackIn );
    virtual HRESULT InvokeCommand( ULONG nCommandIDIn );

    HNETWORK GetHNetwork( void ) { return m_hNetwork; };

};  //*** class CNetworkData

/////////////////////////////////////////////////////////////////////////////
// class CNetInterfaceData
/////////////////////////////////////////////////////////////////////////////

class CNetInterfaceData
    : public CObjData
{
private:
    HNETINTERFACE   m_hNetInterface;    // This is a copy, don't free it.

protected:
    virtual HRESULT HrInitialize( IUnknown * piDataIn );

public:

    virtual HRESULT AddContextMenuItems( IWCContextMenuCallback * piCallbackIn );
    virtual HRESULT InvokeCommand( ULONG nCommandIDIn );

    HNETINTERFACE GetHNetInterface( void ) { return m_hNetInterface; };

};  //*** class CNetInterfaceData
