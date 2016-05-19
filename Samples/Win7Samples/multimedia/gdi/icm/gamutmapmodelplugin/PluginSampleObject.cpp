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
//      Implementation of CGamutMapModelPluginSample methods
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "GamutMapModelPluginSample.h"
#include "PluginSampleObject.h"
#include <math.h>

#define INITGUID
#include <guiddef.h>

// TODO replace GUID while developing plugin
DEFINE_GUID(INTERFACEID_PRIVATEINTERFACE,0x94617F91,0x2C14,0x4485,0x9B,0x94,0xA7,0x85,0x79,0xB7,0x35,0x23);

//+---------------------------------------------------------------------------
//
//  Member:
//      CGamutMapModelPluginSample::Initialize
//
//  Synopsis: 
//      Implementation of IGamutMapModelPlugIn::Initialize.
//
//----------------------------------------------------------------------------
STDMETHODIMP CGamutMapModelPluginSample::Initialize(BSTR bstrXml,
                                        IDeviceModelPlugIn *pDM1,
                                        IDeviceModelPlugIn *pDM2,
                                        GamutBoundaryDescription *GBD1,
                                        GamutBoundaryDescription *GBD2 )
{
    if ( !bstrXml || !GBD1 || !GBD2 )
    {
        return E_INVALIDARG;
    }

    m_hr = S_OK;

    //
    // Query the private plugin interfaces.
    // This will be expected to fail, if the device models
    // are not of known type.
    //
    if ( pDM1 )
    {
        HRESULT hrQueryInterface = pDM1->QueryInterface(
                                    INTERFACEID_PRIVATEINTERFACE,
                                    (void**)&m_pSourceDeviceModel
                                    );    
        if ( SUCCEEDED( hrQueryInterface ) )
        {
            //
            // For demonstration, call private interface method
            //
            m_hr = m_pSourceDeviceModel->SamplePrivateMethod();
        }
    }
    if ( pDM2 && SUCCEEDED( m_hr ) )
    {
        HRESULT hrQueryInterface = pDM2->QueryInterface(
                            INTERFACEID_PRIVATEINTERFACE,
                            (void**)&m_pDestinationDeviceModel
                            );    
        if ( SUCCEEDED( hrQueryInterface ) )
        {
            //
            // For demonstration, call private interface method
            //
            m_hr = m_pDestinationDeviceModel->SamplePrivateMethod();
        }
    }
    return m_hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CGamutMapModelPluginSample::SourceToDestinationAppearanceColors
//
//  Synopsis: 
//      Implementation of 
//      IGamutMapModelPlugIn::SourceToDestinationAppearanceColors. This method
//      simply mirrors the data in the sample.
//
//----------------------------------------------------------------------------
STDMETHODIMP CGamutMapModelPluginSample::SourceToDestinationAppearanceColors(UINT cColors,
                                            const JChColorF *pInputColors,
                                            JChColorF *pOutputColors )
{
    if ( !pInputColors || !pOutputColors )
    {
        return E_INVALIDARG;
    }
    if ( SUCCEEDED( m_hr ) )
    {
        //
        // Mirror the data. Real plugins should produce output contained
        // within the destination gamut boundary
        //
        CopyMemory( pOutputColors, pInputColors, cColors*sizeof(JChColorF) );
    }
    return m_hr;
}
