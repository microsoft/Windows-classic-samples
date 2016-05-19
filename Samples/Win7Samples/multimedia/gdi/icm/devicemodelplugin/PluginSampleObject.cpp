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
//      Implementation of CDeviceModelPluginSample methods
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "DeviceModelPluginSample.h"
#include "PluginSampleObject.h"
#include <math.h>

//
// The sample produces 3-channel sRGB
//
#define PLUGIN_CHANNELS 3

//+---------------------------------------------------------------------------
//
//  Member:
//      CDeviceModelPluginSample::Initialize
//
//  Synopsis: 
//      Implementation of IDeviceModelPlugIn::Initialize. This method
//      does nothing in this sample.
//
//----------------------------------------------------------------------------
STDMETHODIMP CDeviceModelPluginSample::Initialize(
    BSTR bstrXML,
    UINT cNumModels,
    UINT iModelPosition )
{   
    if ( !bstrXML )
    {
        return E_INVALIDARG;
    }

    m_hr = S_OK;

    return m_hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CDeviceModelPluginSample::GetNumChannels
//
//  Synopsis: 
//      Implementation of IDeviceModelPlugIn::GetNumChannels. 
//
//----------------------------------------------------------------------------
STDMETHODIMP CDeviceModelPluginSample::GetNumChannels(
    UINT *pNumChannels )
{
    if ( FAILED( m_hr ) )
    {
        return m_hr;
    }

    if ( !pNumChannels )
    {
        return E_INVALIDARG;
    }
    
    *pNumChannels = PLUGIN_CHANNELS;

    return m_hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CDeviceModelPluginSample::DeviceToColorimetricColors
//
//  Synopsis: 
//      Implementation of IDeviceModelPlugIn::DeviceToColorimetricColors. 
//
//----------------------------------------------------------------------------
STDMETHODIMP CDeviceModelPluginSample::DeviceToColorimetricColors(
    UINT cColors,
    UINT cChannels,
    const FLOAT *pDeviceValues,
    XYZColorF       *pXYZColors )
{
    if ( FAILED( m_hr ) )
    {
        return m_hr;
    }

    if ( !pDeviceValues || !pXYZColors || ( PLUGIN_CHANNELS !=cChannels) )
    {
        return E_INVALIDARG;
    }
    
    //
    // Convert sRGB colors to CIEXYZ
    //
    for ( ULONG i=0; i<cColors; i++ )
    {
        FLOAT RGB[3];
        for ( ULONG j=0; j<PLUGIN_CHANNELS; j++ )
        {
            RGB[j] = (pDeviceValues[3*i+j] < .04045f) ? (pDeviceValues[3*i+j]/12.92f) : 
                        pow( (pDeviceValues[3*i+j]+0.055f)/(1.055f), 2.4f );
        }

        pXYZColors[i].X = 100.0f*(0.412424f * RGB[0] + 0.357579f * RGB[1] + 0.180464f * RGB[2]);
        pXYZColors[i].Y = 100.0f*(0.212656f * RGB[0] + 0.715158f * RGB[1] + 0.072186f * RGB[2]);
        pXYZColors[i].Z = 100.0f*(0.019332f * RGB[0] + 0.119193f * RGB[1] + 0.950444f * RGB[2]);

    }
    return m_hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CDeviceModelPluginSample::ColorimetricToDeviceColors
//
//  Synopsis: 
//      Implementation of IDeviceModelPlugIn::ColorimetricToDeviceColors. 
//
//----------------------------------------------------------------------------
STDMETHODIMP CDeviceModelPluginSample::ColorimetricToDeviceColors(
    UINT cColors,
    UINT cChannels,
    const  XYZColorF * pXYZColors,
    FLOAT *pDeviceValues  )
{
    if ( FAILED( m_hr ) )
    {
        return m_hr;
    }

    if ( !pDeviceValues || !pXYZColors || ( PLUGIN_CHANNELS !=cChannels) )
    {
        return E_INVALIDARG;
    }
    
    //
    // Convert CIEXYZ colors to sRGB
    //
    for ( ULONG i=0; i<cColors; i++ )
    {
        //
        // Convert XYZ to sRGB
        //
        pDeviceValues[3*i+0] = 0.01f * (3.2410f * pXYZColors[i].X - 1.5374f * pXYZColors[i].Y - .49860f * pXYZColors[i].Z);
        pDeviceValues[3*i+1] = 0.01f * (-.9692f * pXYZColors[i].X + 1.8760f * pXYZColors[i].Y + .04160f * pXYZColors[i].Z);
        pDeviceValues[3*i+2] = 0.01f * (.05560f * pXYZColors[i].X - .20400f * pXYZColors[i].Y + 1.0570f * pXYZColors[i].Z);

        for ( ULONG j=0; j<3; j++ )
        {
            pDeviceValues[3*i+j] = ((pDeviceValues[3*i+j] < .0031308)?(12.92f*pDeviceValues[3*i+j]) :
                                    pow( pDeviceValues[3*i+j], 1.0f/2.4f) * 1.055f - .055f);
        }
    }  

    return m_hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CDeviceModelPluginSample::GetPrimarySamples
//
//  Synopsis: 
//      Implementation of IDeviceModelPlugIn::GetPrimarySamples. 
//
//----------------------------------------------------------------------------
STDMETHODIMP CDeviceModelPluginSample::GetPrimarySamples(
    PrimaryXYZColors *pPrimaryColor
)
{
    if ( !pPrimaryColor )
    {
        return E_INVALIDARG;
    }
    if ( SUCCEEDED( m_hr ) )
    {
        for ( ULONG i=0; i<sizeof(PrimaryXYZColors)/sizeof(FLOAT); i++ )
        {
            ((FLOAT*) pPrimaryColor)[i] = 100.0f * ((FLOAT*) &g_sRGBPrimaries)[i];
        }
    }
    return m_hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CDeviceModelPluginSample::GetGamutBoundaryMeshSize
//
//  Synopsis: 
//      Implementation of IDeviceModelPlugIn::GetGamutBoundaryMeshSize. 
//
//----------------------------------------------------------------------------
STDMETHODIMP CDeviceModelPluginSample::GetGamutBoundaryMeshSize(
    UINT *pNumVertices,
    UINT *pNumTriangles
)
{
    if ( !pNumVertices || !pNumTriangles )
    {
        return E_INVALIDARG;
    }
    if ( SUCCEEDED( m_hr ) )
    {
        *pNumVertices = g_NumsRGBVertices;
        *pNumTriangles = g_NumsRGBTriangles;
    }
    return m_hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CDeviceModelPluginSample::GetGamutBoundaryMesh
//
//  Synopsis: 
//      Implementation of IDeviceModelPlugIn::GetGamutBoundaryMesh. 
//
//----------------------------------------------------------------------------
STDMETHODIMP CDeviceModelPluginSample::GetGamutBoundaryMesh(
    UINT cChannels,
    UINT cVertices,
    UINT cTriangles,
    FLOAT *pVertices,
    GamutShellTriangle *pTriangles
)
{
    if ( !pVertices || !pTriangles )
    {
        return E_INVALIDARG;
    }
    if ( SUCCEEDED( m_hr ) )
    {
        CopyMemory( pTriangles, g_sRGBTriangles, cTriangles*sizeof(GamutShellTriangle) );
        CopyMemory( pVertices, g_sRGBVertices, sizeof(g_sRGBVertices) );
    }
    return m_hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CDeviceModelPluginSample::GetNeutralAxisSize
//
//  Synopsis: 
//      Implementation of IDeviceModelPlugIn::GetNeutralAxisSize. 
//
//----------------------------------------------------------------------------
STDMETHODIMP CDeviceModelPluginSample::GetNeutralAxisSize(
    UINT *pcColors
)
{
    if ( !pcColors )
    {
        return E_INVALIDARG;
    }
    if ( SUCCEEDED( m_hr ) )
    {
        *pcColors = g_NumsRGBNeutralAxis;
    }
    return m_hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CDeviceModelPluginSample::GetNeutralAxis
//
//  Synopsis: 
//      Implementation of IDeviceModelPlugIn::GetNeutralAxis. 
//
//----------------------------------------------------------------------------
STDMETHODIMP CDeviceModelPluginSample::GetNeutralAxis(
    UINT cColors,
    XYZColorF *pXYZColors
)
{
    if ( !pXYZColors || cColors != g_NumsRGBNeutralAxis )
    {
        return E_INVALIDARG;
    }
    if ( SUCCEEDED( m_hr ) )
    {
        for ( ULONG i=0; i<sizeof(g_sRGBNeutralAxis)/sizeof(FLOAT); i++ )
        {
            ((FLOAT*) pXYZColors)[i] = 100.0f * ((FLOAT*) g_sRGBNeutralAxis)[i];
        }    
    }
    return m_hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CDeviceModelPluginSample::ColorimetricToDeviceColorsWithBlack
//
//  Synopsis: 
//      Implementation of IDeviceModelPlugIn::ColorimetricToDeviceColorsWithBlack. 
//
//----------------------------------------------------------------------------
STDMETHODIMP CDeviceModelPluginSample::ColorimetricToDeviceColorsWithBlack(
    UINT cColors,
    UINT cChannels,
    const  XYZColorF * pXYZColors,
    const BlackInformation *pBlackInformation,
    FLOAT *pDeviceValues  )
{
    //
    // The black information could be used for improved color control,
    // but ignored for the purposed of this sample.
    //
    return ColorimetricToDeviceColors( cColors, cChannels, pXYZColors, pDeviceValues );
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CDeviceModelPluginSample::SetTransformDeviceModelInfo
//
//  Synopsis: 
//      Implementation of IDeviceModelPlugIn::SetTransformDeviceModelInfo. 
//
//----------------------------------------------------------------------------
STDMETHODIMP CDeviceModelPluginSample::SetTransformDeviceModelInfo(
    UINT iModelPosition,
    IDeviceModelPlugIn *pIDeviceModelOther )
{
    if ( FAILED( m_hr ) )
    {
        return m_hr;
    }

    //
    // The other device model may or may not be a plugin
    //
    if ( pIDeviceModelOther )
    {
        //
        // Query the private plugin interface.
        // This will be expected to fail, if the other device model
        // is not of known type.
        //
        HRESULT hrQueryInterface = pIDeviceModelOther->QueryInterface(
                                        IID_ISampleDeviceModelPrivateInterface,
                                        (void**)&m_pOtherDeviceModel
                                        );    
        if ( SUCCEEDED( hrQueryInterface ) )
        {
            //
            // For demonstration, call private interface method
            //
            m_hr = m_pOtherDeviceModel->SamplePrivateMethod();
        }

    }

    return m_hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CDeviceModelPluginSample::SamplePrivateMethod
//
//  Synopsis: 
//      Implementation of IDeviceModelPlugIn::SamplePrivateMethod. 
//
//----------------------------------------------------------------------------
STDMETHODIMP CDeviceModelPluginSample::SamplePrivateMethod()
{
    // Empty method for demonstration
    return S_OK;
}
