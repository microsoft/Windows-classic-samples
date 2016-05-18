/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      ExtObj.h
//
//  Implementation File:
//      ExtObj.cpp
//
//  Description:
//      Definition of the CExtObject class, which implements the
//      extension interfaces required by a Microsoft Windows NT Cluster
//      Administrator Extension DLL.
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
#include <CluAdmEx.h>   // for CLUADMEX_OBJECT_TYPE and interfaces
#pragma warning( pop )

#include "ExtObjID.h"   // for CLSID_CoClipBookServerEx
#include "ExtObjData.h"

/////////////////////////////////////////////////////////////////////////////
// External Class Declarations
/////////////////////////////////////////////////////////////////////////////

class CBasePropertyPage;

/////////////////////////////////////////////////////////////////////////////
// CPageList
/////////////////////////////////////////////////////////////////////////////

typedef CList< CBasePropertyPage *, CBasePropertyPage * > CPageList;

/////////////////////////////////////////////////////////////////////////////
// class CExtObject
/////////////////////////////////////////////////////////////////////////////

class CExtObject
    : public IWEExtendPropertySheet
    , public IWEExtendWizard
    , public ISupportErrorInfo
    , public CComObjectRoot
    , public CComCoClass< CExtObject, &CLSID_CoClipBookServerEx >
{
public:
    CExtObject( void );
BEGIN_COM_MAP( CExtObject )
    COM_INTERFACE_ENTRY( IWEExtendPropertySheet )
    COM_INTERFACE_ENTRY( IWEExtendWizard )
    COM_INTERFACE_ENTRY( ISupportErrorInfo )
END_COM_MAP()

//DECLARE_NOT_AGGREGATABLE( CExtObject )

//
// Uncomment the line above if you don't want your object to support aggregation.
// The default is to support it.
//

#pragma warning( push, 3 )
#pragma warning( disable : 4996 ) // warning C4996: 'ATL::CComModule::UpdateRegistryClass' was declared deprecated
DECLARE_REGISTRY(
          CExtObject
        , _T("CLUADMEX.ClipBookServerEx")
        , _T("CLUADMEX.ClipBookServerEx")
        , IDS_CLUADMEX_COMOBJ_DESC
        , THREADFLAGS_APARTMENT
        );
#pragma warning( pop )

    //
    // ISupportsErrorInfo
    //

    STDMETHOD( InterfaceSupportsErrorInfo )( REFIID riid );

    //
    // IWEExtendPropertySheet
    //

public:
    STDMETHOD( CreatePropertySheetPages )(
                      IUnknown *                 piDataIn
                    , IWCPropertySheetCallback * piCallbackIn
                    );

    //
    // IWEExtendWizard
    //

public:
    STDMETHOD( CreateWizardPages )(
                      IUnknown *            piDataIn
                    , IWCWizardCallback *   piCallbackIn
                    );

    CLUADMEX_OBJECT_TYPE Cot() { return (m_podObjData != NULL) ? m_podObjData->Cot() : CLUADMEX_OT_NONE; }

    //
    // IGetClusterUIInfo data
    //

    LCID                        Lcid( void ) const                  { return m_lcid; }
    HFONT                       Hfont( void ) const                 { return m_hfont; }
    HICON                       Hicon( void ) const                 { return m_hicon; }
    IWCWizardCallback *         PiWizardCallback( void ) const      { return m_piWizardCallback; }
    BOOL                        BWizard( void ) const               { return m_bWizard; }

    //
    // IGetClusterDataInfo data
    //

    HCLUSTER                    Hcluster( void ) const              { return m_hCluster; }
    LONG                        Cobj( void ) const                  { return m_cobj; }
    CObjData *                  PodObjData( void )                  { return m_podObjData; }
    HRESULT                     HrGetObjectInfo( void );

//
// Attributes
//

protected:

    IUnknown *                  m_piData;
    IWCWizardCallback *         m_piWizardCallback;
    BOOL                        m_bWizard;

    //
    // IGetClusterUIInfo data
    //

    LCID                        m_lcid;
    HFONT                       m_hfont;
    HICON                       m_hicon;

    //
    // IGetClusterDataInfo data
    //

    HCLUSTER                    m_hCluster;
    LONG                        m_cobj;

    CObjData *                  m_podObjData;

//
// Implementation
//

    virtual ~CExtObject(void);

    HRESULT                     HrGetUIInfo( IUnknown * piDataIn );
    HRESULT                     HrSaveData( IUnknown * piDataIn );

    CPageList                   m_lpg;

}; //*** class CExtObject
