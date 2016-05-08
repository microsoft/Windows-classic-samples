//+--------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//      Declaration of plugin class
//
//----------------------------------------------------------------------------

#pragma once
#include "resource.h"

#include "PluginSampleObject.h"

class ATL_NO_VTABLE CGamutMapModelPluginSample : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CGamutMapModelPluginSample, &CLSID_GamutMapModelPluginSample>,
    public IGamutMapModelPlugIn
{
public:
    CGamutMapModelPluginSample()
    {
        m_pSourceDeviceModel = NULL;
        m_pDestinationDeviceModel = NULL;
        m_hr = E_FAIL;
    }

    ~CGamutMapModelPluginSample()
    {
        if ( m_pSourceDeviceModel )
        {
            m_pSourceDeviceModel->Release();
        }
        if ( m_pDestinationDeviceModel )
        {
            m_pDestinationDeviceModel->Release();
        }
    }


DECLARE_REGISTRY_RESOURCEID(IDR_GamutMapModelPluginSample)


BEGIN_COM_MAP(CGamutMapModelPluginSample)
    COM_INTERFACE_ENTRY(IGamutMapModelPlugIn)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

public:

    //
    // Gamut Map Model Plugin Methods
    //
    STDMETHOD(Initialize)( 
        BSTR bstrXml,
        IDeviceModelPlugIn *pDM1,
        IDeviceModelPlugIn *pDM2,
        GamutBoundaryDescription *GBD1,
        GamutBoundaryDescription *GBD2 
    );

    STDMETHOD(SourceToDestinationAppearanceColors)(UINT cColors,
                                                const JChColorF *pInputColors,
                                                JChColorF *pOutputColors
    ); 


private:
    HRESULT m_hr;
    ISampleDeviceModelPrivateInterface *m_pSourceDeviceModel;
    ISampleDeviceModelPrivateInterface *m_pDestinationDeviceModel;
};

OBJECT_ENTRY_AUTO(__uuidof(GamutMapModelPluginSample), CGamutMapModelPluginSample)
